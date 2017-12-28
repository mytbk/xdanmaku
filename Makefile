LDFLAGS=-lXft -lX11 -lXinerama -lcurl -luuid -ljson-c -pthread
CFLAGS=$(shell pkg-config --cflags xft) -pthread

xdanmaku_main: xdanmaku_main.o subscribe.o danmaku.o danlist.o

danmaku_main: danmaku_main.o danmaku.o

subscribe_main: subscribe_main.o subscribe.o

clean:
	rm -f *.o xdanmaku_main danmaku_main subscribe_main

.PHONY: clean
