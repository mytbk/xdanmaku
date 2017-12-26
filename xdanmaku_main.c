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

int main(int argc, char *argv[])
{
	pthread_t sub_thread;

	const char *fontname = "Source Han Sans CN Medium:size=40";
	const char *url = "https://dm.tuna.moe:8443";
	const char *channel = "demo";

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-fn")==0) {
			fontname = argv[++i];
		} else if (strcmp(argv[i], "-u")==0) {
			url = argv[++i];
		} else if (strcmp(argv[i], "-c")==0) {
			channel = argv[++i];
		} else {
			fprintf(stderr, "usage: %s [-fn font] <-u url> <-c channel>\n", argv[0]);
			fprintf(stderr, "  url: danmaku server url <default: https://dm.tuna.moe:8443>\n");
			fprintf(stderr, "  channel: danmaku channel <default: demo>\n");
			fprintf(stderr, "  font: <default: %s>\n", fontname);
			return 1;
		}
	}

	danmaku_init(&di, fontname);

	curl_global_init(CURL_GLOBAL_DEFAULT);
	new_subscriber(&cs, url, channel);

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
