/*
 * Copyright 2017 Iru Cai <mytbk920423@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <X11/Xft/Xft.h>
#include <X11/Xlib.h>

typedef struct {
	Display *dpy;
	Window root_window;
	Visual *vis;
	int depth;
	Colormap colormap;
	XftFont *font;
	int height, width;
} display_info;

typedef enum { DAN_L2R, DAN_R2L, DAN_TOP, DAN_BOTTOM, OTHER } danmaku_type;

typedef struct {
	display_info *di;
	Window w;
	int width, height;
	int x, y, dx, dy;
	int ttl;
	danmaku_type type;
} danmaku_info;

void danmaku_init(display_info *d, const char *fontname);

/* danmaku init functions: set x, y, dx, dy, type */
typedef void (*dm_init_func)(danmaku_info *, int, int, int);
void dm_init_fly_l2r(danmaku_info *dan, int w, int h, int ttl);
void dm_init_fly_r2l(danmaku_info *dan, int w, int h, int ttl);
void dm_init_top(danmaku_info *dan, int w, int h, int ttl);
void dm_init_bottom(danmaku_info *dan, int w, int h, int ttl);

XftColor *find_color(const char *color);
danmaku_info *create_danmaku(display_info *d, const char *s, XftColor *xcolor,
									  dm_init_func dm_init, int ttl);
void issue_danmaku(danmaku_info *dan);
void move_danmaku(danmaku_info *dan);
void free_danmaku(danmaku_info *dan);
