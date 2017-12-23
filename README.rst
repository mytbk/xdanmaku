xdanmaku
========

xdanmaku is a danmaku client for `tuna/gdanmaku-server <https://github.com/tuna/gdanmaku-server>`_ written in C, using libX11. It aims to be simple and able to run on any X11 based desktop environment.

Usage
-----

To use it, just run `xdanmaku <server> <channel>`, for example `xdanmaku https://dm.tuna.moe:8443 demo` and open https://dm.tuna.moe:8443/channel/demo to post danmakus.

Install
-------

If you use Arch, you can use the AUR package `xdanmaku <https://aur.archlinux.org/packages/xdanmaku/>`_.

To build yourself, you need to install the following dependencies:

- libxft
- libcurl
- json-c
- libuuid

In Debian::

  apt install libxft-dev libcurl4-openssl-dev libjson-c-dev uuid-dev
