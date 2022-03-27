/*
 * Plain launcher program for Jay Bromley's Xkeyboard library.
 *
 * Copyright (C) 2010 by Sergey Mironov <ierton@gmail.com>
 *
 * This file is part of Xkb-switch.
 * Xkb-switch is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Xkb-switch is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Xkb-switch. If not, see <http://www.gnu.org/licenses/>.
 */
#include <X11/XKBlib.h>

#include <iostream>
#include <algorithm>
#include <sstream>
#include <getopt.h>

#include <map>
#include <ctime>

#include <i3ipc++/ipc.hpp>

#include "XKeyboard.hpp"
#include "Utils.hpp"

using namespace std;
using namespace kb;

void usage()
{
  cerr << "Usage: xkb-switch -s ARG         Sets current layout group to ARG" << endl;
  cerr << "       xkb-switch -l|--list      Displays all layout groups" << endl;
  cerr << "       xkb-switch -h|--help      Displays this message" << endl;
  cerr << "       xkb-switch -v|--version   Shows version number" << endl;
  cerr << "       xkb-switch -w|--wait [-p] Waits for group change" << endl;
  cerr << "       xkb-switch -W|--longwait  Infinitely waits for group change, prints group names to stdout" << endl;
  cerr << "       xkb-switch -n|--next      Switch to the next layout group" << endl;
  cerr << "       xkb-switch -d|--debug     Print debug information" << endl;
  cerr << "       xkb-switch [-p]           Displays current layout group" << endl;
  cerr << "       xkb-switch --i3           Monitor and automatically switch layouts in i3wm" << endl;
}

string print_layouts(const string_vector& sv)
{
  ostringstream oss;
  bool fst = true;

  oss << "[";
  for(string_vector::const_iterator i=sv.begin(); i!=sv.end(); i++) {
    if(!fst) oss << " ";
    oss << *i;
    fst = false;
  }
  oss << "]";
  return oss.str();
}

void i3_watch(XKeyboard xkb, const string_vector& syms) {
  map<int, int> window_group_map;
  int previous_container_id = 0;
  int default_group = xkb.get_group();

  i3ipc::connection conn;

  conn.subscribe(i3ipc::ET_WINDOW);

  conn.signal_window_event.connect(
    [
      &default_group,
      &previous_container_id,
      &syms,
      &window_group_map,
      &xkb
    ]
    (const i3ipc::window_event_t&  ev) {

      int new_group;
      map<int, int>::iterator it;

      if (ev.type == i3ipc::WindowEventType::FOCUS) {
        std::time_t t = std::time(nullptr);
        char mbstr[100];
        std::strftime(mbstr, sizeof(mbstr), "%Y-%m-%dT%X", std::localtime(&t));
        std::cout << "[" << mbstr << "] Switched to #" << ev.container->id << " - \"" << ev.container->name << '"' << std::endl;

        if (previous_container_id != 0) {
          window_group_map[previous_container_id] = xkb.get_group();
        }

        it = window_group_map.find(ev.container->id);
        new_group = it != window_group_map.end() ? it->second : default_group;
        xkb.set_group(new_group);
        previous_container_id = ev.container->id;

        std::cout << "\tWindow layout: " << syms.at(new_group) << std::endl;
      }
    }
  );

  while (true) {
    conn.handle_event();
  }
}

int main( int argc, char* argv[] )
{
  size_t verbose = 1;
  string_vector syms;
  bool syms_collected = false;

  try {
    int m_cnt = 0;
    int m_wait = 0;
    int m_lwait = 0;
    int m_print = 0;
    int m_next = 0;
    int m_list = 0;
    int m_i3 = 0;
    int opt;
    int option_index = 0;
    string newgrp;

    static struct option long_options[] = {
            {"i3", no_argument, NULL, '3'},
            {"set", required_argument, NULL, 's'},
            {"list", no_argument, NULL, 'l'},
            {"version", no_argument, NULL, 'v'},
            {"wait", no_argument, NULL, 'w'},
            {"longwait", no_argument, NULL, 'W'},
            {"print", no_argument, NULL, 'p'},
            {"next", no_argument, NULL, 'n'},
            {"help", no_argument, NULL, 'h'},
            {"debug", no_argument, NULL, 'd'},
            {NULL, 0, NULL, 0},
    };
    while ((opt = getopt_long(argc, argv, "s:lvwWpnhd",
                              long_options, &option_index))!=-1) {
      switch (opt) {
      case '3':
        m_i3 = 1;
        m_cnt++;
        break;
      case 's':
        if (!optarg || string(optarg).empty())
          CHECK_MSG(verbose, 0, "Argument expected");
        newgrp=optarg;
        m_cnt++;
        break;
      case 'l':
        m_list = 1;
        m_cnt++;
        break;
      case 'v':
        cerr << "xkb-switch " << XKBSWITCH_VERSION << endl;
        break;
      case 'w':
        m_wait = 1;
        m_cnt++;
        break;
      case 'W':
        m_lwait = 1;
        m_cnt++;
        break;
      case 'p':
        m_print = 1;
        m_cnt++;
        break;
      case 'n':
        m_next = 1;
        m_cnt++;
        break;
      case 'h':
        usage();
        break;
      case 'd':
        verbose++;
        break;
      case '?':
        THROW_MSG(verbose, "Invalid arguments. Check --help.");
        break;
      default:
        THROW_MSG(verbose, "Invalid argument: '" << (char)opt << "'. Check --help.");
        break;
      }
    }

    if (verbose > 1) {
      cerr << "[DEBUG] xkb-switch version " << XKBSWITCH_VERSION << endl;
    }

    if(m_list || m_lwait || m_i3 || !newgrp.empty()) {
      CHECK_MSG(verbose, m_cnt==1, "Invalid flag combination. Try --help.");
    }

    // Default action
    if(m_cnt==0)
      m_print = 1;

    XKeyboard xkb(verbose);
    xkb.open_display();

    if(m_wait) {
      xkb.wait_event();
    }

    if(m_lwait) {
      while(true) {
        xkb.wait_event();
        xkb.build_layout(syms);
        syms_collected = true;
        cout << syms.at(xkb.get_group()) << endl;
      }
    }

    layout_variant_strings lv = xkb.get_layout_variant();
    if(verbose >= 2) {
      cerr << "[DEBUG] layout: " << (lv.first.length() > 0 ? lv.first : "<empty>") << endl;
      cerr << "[DEBUG] variant: " << (lv.second.length() > 0 ? lv.second : "<empty>") << endl;
    }
    xkb.build_layout_from(syms, lv);
    syms_collected = true;

    if (m_next) {
      CHECK_MSG(verbose, !syms.empty(), "No layout groups configured");
      const string nextgrp = syms.at(xkb.get_group());
      string_vector::iterator i = find(syms.begin(), syms.end(), nextgrp);
      if (++i == syms.end())i = syms.begin();
      xkb.set_group(i - syms.begin());
    }
    else if(!newgrp.empty()) {
      string_vector::iterator i = find(syms.begin(), syms.end(), newgrp);
      CHECK_MSG(verbose, i!=syms.end(),
        "Group '" << newgrp << "' is not supported by current layout. Try xkb-switch -l.");
      xkb.set_group(i-syms.begin());
    }

    if(m_print) {
      cout << syms.at(xkb.get_group()) << endl;
    }

    if(m_list) {
      for(int i=0; i<syms.size(); i++) {
        cout << syms[i] << endl;
      }
    }

    if(m_i3) {
      i3_watch(xkb, syms);
    }

    return 0;
  }
  catch(std::exception & err) {
    if ( verbose >= 2) {
      cerr << "xkb-switch: ";
    }
    cerr << err.what() << endl;
    if (verbose >= 2) {
      if (syms_collected) {
        cerr << "xkb-switch: layouts: " << print_layouts(syms) << endl;
      }
    }
    return 2;
  }
}
