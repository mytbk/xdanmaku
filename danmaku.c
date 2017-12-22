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

#include <stdio.h>
#include <stdlib.h>

#include "danmaku.h"

#define NCOLORS (sizeof(colors) / sizeof(colors[0]))
static const char *colors[] = {"blue", "white", "red",    "yellow",
			       "cyan", "green", "purple", "black"};
static XftColor xcolors[NCOLORS];

static void color_init(display_info *d)
{
	int i;
	for (i = 0; i < NCOLORS; i++)
		XftColorAllocName(d->dpy, d->vis, d->colormap, colors[i],
				  &xcolors[i]);
}

XftColor *find_color(const char *color)
{
	int i;
	for (i = 0; i < NCOLORS; i++) {
		if (strcmp(color, colors[i]) == 0)
			return &xcolors[i];
	}
	return NULL;
}

void danmaku_init(display_info *d, const char *fontname)
{
	Display *dpy = XOpenDisplay(NULL);
	int scr = DefaultScreen(dpy);
	Window root_window = DefaultRootWindow(dpy);

	d->dpy = dpy;
	d->root_window = root_window;
	d->height = DisplayHeight(dpy, scr);
	d->width = DisplayWidth(dpy, scr);
	d->font = XftFontOpenName(dpy, scr, fontname);
	if (d->font == NULL)
		puts("open font error");

	XVisualInfo vinfo;
	if (XMatchVisualInfo(dpy, scr, 32, TrueColor, &vinfo)) {
		d->vis = vinfo.visual;
		d->depth = vinfo.depth;
	} else {
		puts("no such visual");
		d->vis = DefaultVisual(dpy, scr);
		d->depth = 0;
	}

	d->colormap = XCreateColormap(dpy, root_window, d->vis, AllocNone);
	color_init(d);
}

void dm_init_fly_l2r(danmaku_info *dan, int w, int h, int ttl)
{
	dan->x = -w;
	/* y can be any position between [h, height-h*2) */
	dan->y = h + (rand() % (dan->di->height - h*3));
	dan->dx = (dan->di->width + w - 1) / ttl + 1;
	dan->dy = 0;
	dan->type = DAN_L2R;
}

void dm_init_fly_r2l(danmaku_info *dan, int w, int h, int ttl)
{
	dan->x = dan->di->width;
	dan->y = h + (rand() % (dan->di->height - h*3));
	dan->dx = -((dan->di->width + w - 1) / ttl + 1);
	dan->dy = 0;
	dan->type = DAN_R2L;
}

void dm_init_top(danmaku_info *dan, int w, int h, int ttl)
{
	dan->x = (dan->di->width - w) / 2;
	dan->y = rand() % h;
	dan->dx = 0;
	dan->dy = 0;
	dan->type = DAN_TOP;
}

void dm_init_bottom(danmaku_info *dan, int w, int h, int ttl)
{
	dan->x = (dan->di->width - w) / 2;
	dan->y = dan->di->height - h * 2 + (rand() % h);
	dan->dx = 0;
	dan->dy = 0;
	dan->type = DAN_BOTTOM;
}

danmaku_info *create_danmaku(display_info *d, const char *s, XftColor *xcolor,
									  dm_init_func dm_init, int ttl)
{
	XGlyphInfo extents;
	int wt, ht;

	Display *dpy = d->dpy;

	danmaku_info *dan = (danmaku_info*)malloc(sizeof(danmaku_info));

	XftTextExtentsUtf8(dpy, d->font, s, strlen(s), &extents);
	wt = extents.width;
	ht = extents.height;

	XSetWindowAttributes attr = {.colormap = d->colormap,
				     .background_pixel = 0x00000000, /* ARGB */
				     .border_pixel = 0,
				     .override_redirect = 1};
	Window w = XCreateWindow(
	    dpy, d->root_window,
	    -wt, /* put the window out of the screen */
	    0, wt, ht, 0, d->depth, InputOutput, d->vis,
	    CWColormap | CWBackPixel | CWBorderPixel | CWOverrideRedirect,
	    &attr);

	XftDraw *drw = XftDrawCreate(dpy, w, d->vis, d->colormap);

	dan->di = d;
	dan->w = w;
	dan->ttl = ttl;
	dan->width = wt;
	dan->height = ht;
	dm_init(dan, wt, ht, ttl);

	XSelectInput(dpy, w,
		     ExposureMask | PropertyChangeMask | StructureNotifyMask);
	XMapWindow(dpy, w);
	XClearWindow(dpy, w);
	XftDrawStringUtf8(drw, xcolor, d->font, extents.x, extents.y, s,
			  strlen(s));

	return dan;
}

void issue_danmaku(danmaku_info *dan)
{
	Display *dpy = dan->di->dpy;
	Window w = dan->w;

	XMoveWindow(dpy, w, dan->x, dan->y);
	XSync(dpy, False);
}

void move_danmaku(danmaku_info *dan)
{
	dan->x += dan->dx;
	dan->y += dan->dy;
	dan->ttl--;
	XMoveWindow(dan->di->dpy, dan->w, dan->x, dan->y);
}

void free_danmaku(danmaku_info *dan)
{
	XDestroyWindow(dan->di->dpy, dan->w);
	free(dan);
}
