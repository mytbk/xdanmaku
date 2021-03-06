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

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "subscribe.h"
#include "danmaku.h"
#include "danlist.h"
#include <X11/extensions/Xinerama.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int count = 1;

curl_subscriber cs;
display_info di;
danmaku_state *ds;

static inline
const char *json_gets(json_object *j, const char *key)
{
	json_object *val;

	if (json_object_object_get_ex(j, key, &val)) {
		if (json_object_is_type(val, json_type_string))
			return json_object_get_string(val);
	}
	return NULL;
}

void *_subscribe(void* arg)
{
	while (1) {
		json_object *jobj = do_subscribe(&cs);

		if (json_object_is_type(jobj, json_type_array)) {
			int n = json_object_array_length(jobj);
			for (int i=0; i<n; i++) {
				json_object *jdan = json_object_array_get_idx(jobj, i);

				const char *txt, *style, *pos;

				txt = json_gets(jdan, "text");
				if (!txt)
					continue;

				style = json_gets(jdan, "style");
				if (!style)
					continue;
				XftColor *color = find_color(style);
				if (!color)
					continue;

				dm_init_func f;
				pos = json_gets(jdan, "position");
				if (!pos)
					continue;
				if (strcmp(pos, "top") == 0)
					f = dm_init_top;
				else if (strcmp(pos, "bottom") == 0)
					f = dm_init_bottom;
				else if (strcmp(pos, "fly") == 0)
					f = (rand()&1)?dm_init_fly_l2r:dm_init_fly_r2l;

				danmaku_info *dan = create_danmaku(&di, txt, color, f, 1000);
				queue_danmaku(ds, dan);
			}
		}
		/* free the object */
		json_object_put(jobj);
	}
}

static void print_scrinfo(XineramaScreenInfo *si)
{
	fprintf(stderr, "  number: %d, x: %d, y: %d, width: %d, height: %d\n",
			si->screen_number, si->x_org, si->y_org, si->width, si->height);
}

int main(int argc, char *argv[])
{
	pthread_t sub_thread;

	const char *fontname = "Source Han Sans CN Medium:size=40";
	const char *url = "https://dm.tuna.moe:8443";
	const char *channel = "demo";
	const char *passwd = NULL;
	int scr = -1;

	const char *usage =
		"usage: %s [-fn font] [-s screen] <-u url> <-c channel> [-p passwd]\n"
		"  url: danmaku server url <default: https://dm.tuna.moe:8443>\n"
		"  channel: danmaku channel <default: demo>\n"
		"  passwd: password for the channel <default: NULL>\n"
		"  font: <default: %s>\n"
		"  screen: <default all screens>\n";

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-fn") == 0) {
			fontname = argv[++i];
		} else if (strcmp(argv[i], "-u") == 0) {
			url = argv[++i];
		} else if (strcmp(argv[i], "-c") == 0) {
			channel = argv[++i];
		} else if (strcmp(argv[i], "-p") == 0) {
			passwd = argv[++i];
		} else if (strcmp(argv[i], "-s") == 0) {
			scr = atoi(argv[++i]);
		} else {
			fprintf(stderr, usage, argv[0], fontname);
			return 1;
		}
	}

	danmaku_init(&di, fontname);
	if (scr >= 0) {
		int num;
		XineramaScreenInfo *scrinfo = XineramaQueryScreens(di.dpy, &num);
		if (scr < num) {
			XineramaScreenInfo *si = &scrinfo[scr];
			di.x_org = si->x_org;
			di.y_org = si->y_org;
			di.width = si->width;
			di.height = si->height;
			fprintf(stderr, "Xinerama info of the selected screen:\n");
			print_scrinfo(si);
		} else {
			fprintf(stderr, "Screen %d not found!\n"
					"Xinerama info of all screens:\n", scr);
			for (int i = 0; i < num; i++)
				print_scrinfo(&scrinfo[i]);

			return 1;
		}
	}

	curl_global_init(CURL_GLOBAL_DEFAULT);
	new_subscriber(&cs, url, channel, passwd);

	if (cs.curl) {
		if (pthread_create(&sub_thread, NULL, _subscribe, NULL) != 0) {
			fprintf(stderr, "Fail to create subscriber thread!\n");
			return 1;
		}
	} else {
		fprintf(stderr, "Fail to create cURL subscriber!\n");
		return 1;
	}

	ds = new_danstat();

	while (1) {
		XFlush(di.dpy);
		schedule_danmaku(ds);
		usleep(10000);
	}

	pthread_join(sub_thread, NULL);
	free_subscriber(&cs);
	curl_global_cleanup();
}
