# Xkb-switch-i3

This i3-flavored version of [xkb-switch](https://github.com/ierton/xkb-switch)
is capable of switching layout groups automatically in
[i3wm](https://i3wm.org). It subscribes to i3wm events and, when a window is
focused, switches to the last active layout for the window.

NOTE: Starting from version `1.6.1` xkb-switch-i3 has it's own versioning.

## Installation

For Arch Linux
[xkb-switch-i3](https://aur.archlinux.org/packages/xkb-switch-i3/) and
[xkb-switch-i3-git](https://aur.archlinux.org/packages/xkb-switch-i3-git/) packages
are available in [AUR](https://aur.archlinux.org/).

Dependencies for building from source:

- i3-wm
- jsoncpp
- libsigc++
- libx11
- libxkbfile

```sh
# Clone this repo
git clone https://github.com/zebradil/xkb-switch-i3
# Init submodules
git submodule update --init
# Prepare
mkdir build && cd build
cmake ..
# Build
make
# Install
make install
```

## Usage

Run it in terminal and try changing windows and layouts to see how it works.

```sh
xkb-switch --i3
```

To have it started with i3, add the following to the configuration:

```
exec_always --no-startup-id xkb-switch --i3
```

# Original README

by Jay Bromley, Sergei Mironov, Alexei Rad'kov

xkb-switch is a C++ program that allows to query and change the XKB layout state.
Originally ruby-based code written by Jay Bromley.

- XKeyboard.cpp Implementation for XKB query/set class
- XKbSwitch.cpp Main program
- XKbSwitchApi.cpp The Vim API bindings

The C++ class has no special dependencies on anything outside of
X-related libraries, so it can be easily used with other software.

Older versions of Xkb-switch were licensed under the GNU GPLv3, the current
version is licensed under the MIT license. See COPYING for details.

<!-- vim-markdown-toc GFM -->

- [Installing](#installing)
- [Usage](#usage)
- [VIM integration](#vim-integration)
- [Layout groups](#layout-groups)
- [Bugs or Problems](#bugs-or-problems)
- [Licensing notice](#licensing-notice)

<!-- vim-markdown-toc -->

## Installing

Package _libxkbfile-dev_ (or _libxkbfile-devel_ for Fedora) needs to be
installed to build the program.

To build the program manually, unpack the tarball and cd to source directory.
[Nix](http://nixos.org/nix) users may use `nix-shell` to enter the minimally
sufficient development shell or `nix-build` to build the sources. Other
distributions typically require the following commands to build and install the
program:

```sh
$ mkdir build && cd build
$ cmake ..
$ make
```

Optionally, test the basic functions by running `./test.sh` script. The script
should print OK in the last line and return exit code of zero.

```sh
$ ../test.sh 2>&1 | tee test.log
$ tail -n 1 test.log | grep OK || echo "Test failed!"
```

In order to do a system-wide install, use your system's package manager or
default to the following:

```sh
$ sudo make install                    # System-wide installation
$ make DESTDIR=$HOME/.local install    # User installation
```

On some distributions, you may need to update the program cache if it's the
first time you're installing this program

```sh
$ sudo ldconfig
```

## Usage

```
$ xkb-switch --help

Usage: xkb-switch -s ARG            Sets current layout group to ARG
       xkb-switch -l|--list         Displays all layout groups
       xkb-switch -h|--help         Displays this message
       xkb-switch -v|--version      Shows version number
       xkb-switch -w|--wait [-p]    Waits for group change and exits
       xkb-switch -W                Infinitely waits for group change
       xkb-switch -n|--next         Switch to the next layout group
       xkb-switch [-p]              Displays current layout group
```

_A note on `xkb-switch -x`_
Command line option `xkb-switch -x` has been removed recently. Please, use `setxkbmap
-query` or `setxkbmap -print` to obtain debug information.

## VIM integration

Xkb-switch goes with a library libxkbswitch.so which can be called from
within Vim scripts like this:

```vim
let g:XkbSwitchLib = "/path/to/libxkbswitch.so"
echo libcall(g:XkbSwitchLib, 'Xkb_Switch_getXkbLayout', '')
call libcall(g:XkbSwitchLib, 'Xkb_Switch_setXkbLayout', 'us')
```

See also [article in Russian](http://lin-techdet.blogspot.ru/2012/12/vim-xkb-switch-libcall.html)
describing complex solution.

## Layout groups

xkb-group.sh can help you to manage layout groups. Just run it and send some
input at it's stdin every time you want to trigger layouts from primary to
secondary and back. For example:

```sh
$ xkb-group.sh us ru
switch # switch from us to ru or from current layout to us
switch # switch from ru to us or from us to ru

# from another terminal
$ xkb-switch -s de # switch to 'de' layout, change secondary layout to 'de'

# back to terminal running `xkb-group.sh'
switch # switch from de to us
switch # switch from us to de
```

## Bugs or Problems

Admittedly, I only tested with a few different layouts that I used. If you find
any bugs let me know by submitting an issue or via grrwlf@gmail.com.

References:

- <https://www.x.org/releases/X11R7.5/doc/input/XKB-Config.html>
  - XKB configuration
- <https://www.x.org/releases/current/doc/xorg-docs/input/XKB-Enhancing.html>
  - How to further enhance XKB configuration
- <https://0x64616c.livejournal.com/914.html>
  - Old LJ post by Mitya describing minimalistic X11 kb test application

## Licensing notice

In response to the [request](https://github.com/grwlf/xkb-switch/issues/69) I
decided to re-license the project under the more permissive MIT license. Older
versions of the software remain under the GPL license we used previously. This
[StackExchange
question](https://softwareengineering.stackexchange.com/questions/105912/can-you-change-code-distributed-under-the-mit-license-and-re-distribute-it-unde)
includes some information regarding this kind of situations.
I included the top-3 contributors into the MIT license text. Please contact me
either directly or via the mentioned Github issue if you have any questions or
suggestions regarding this decision.
