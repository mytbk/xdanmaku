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

#include "danmaku.h"
#include <unistd.h>

int main(void)
{
	display_info di;
	danmaku_info *dan, *dan2;

	char ustr[100];
	// const char *fontname = "Source Code Pro Light";
	const char *fontname = "思源黑体 CN:size=40";
	danmaku_init(&di, fontname);

	scanf("%s", ustr);

	dan = create_danmaku(&di, ustr, find_color("red"), dm_init_fly_l2r, 1000);
	dan2 = create_danmaku(&di, ustr, find_color("blue"), dm_init_fly_r2l, 1000);

	issue_danmaku(dan);
	issue_danmaku(dan2);

	while (1) {
		// XEvent e;
		// XNextEvent(di.dpy, &e);
		XFlush(di.dpy);

		move_danmaku(dan);
		move_danmaku(dan2);

		usleep(10000);
	}

	XCloseDisplay(di.dpy);
}
