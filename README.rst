xdanmaku
========

xdanmaku is a danmaku client for `tuna/gdanmaku-server <https://github.com/tuna/gdanmaku-server>`_ written in C, using libX11. It aims to be simple and able to run on any X11 based desktop environment.

Usage
-----

::

  xdanmaku [-fn font] [-s screen] <-u url> <-c channel> [-p passwd]
    url: danmaku server url <default: https://dm.tuna.moe:8443>
    channel: danmaku channel <default: demo>
    passwd: password for the channel <default: NULL>
    font: <default: Source Han Sans CN Medium:size=40>
    screen: <default all screens>


Install
-------

If you use Arch, you can use the AUR package `xdanmaku <https://aur.archlinux.org/packages/xdanmaku/>`_.

To build yourself, you need to install the following dependencies:

- libxft
- libxinerama
- libcurl
- json-c
- libuuid

In Debian::

  apt install libxft-dev libxinerama-dev libcurl4-openssl-dev libjson-c-dev uuid-dev
