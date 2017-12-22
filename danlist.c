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

#include <stdlib.h>
#include <assert.h>
#include "danlist.h"

pNode newnode(danmaku_info *dan)
{
	pNode n = (pNode)malloc(sizeof(struct Node));
	if (n==NULL)
		return NULL;
	n->dan = dan;

	return n;
}

void list_insert(pNode s, pNode t)
{
	t->prev = s;
	t->next = s->next;
	s->next->prev = t;
	s->next = t;
}

pNode list_remove(pNode s)
{
	pNode prev = s->prev;
	s->prev->next = s->next;
	s->next->prev = s->prev;
	free(s);
	return prev;
}

pNode list_new(void)
{
	pNode n = (pNode)malloc(sizeof(struct Node));
	if (n==NULL)
		return NULL;

	n->prev = n->next = n;
	n->dan = NULL;
	return n;
}

void list_free(pNode s)
{
	assert(s->dan == NULL);
	pNode t = s->next;
	while (t != s) {
		pNode t_ = t->next;
		free(t);
		t = t_;
	}
	free(s);
}

void list_append(pNode s, pNode n)
{
	list_insert(s->prev, n);
}

void list_append_danmaku(pNode s, danmaku_info *dan)
{
	list_insert(s->prev, newnode(dan));
}

danmaku_state *new_danstat(void)
{
	danmaku_state *ds = (danmaku_state *)malloc(sizeof(danmaku_state));
	ds->issued = list_new();
	ds->waiting = list_new();
	pthread_mutex_init(&ds->mutex, NULL);
	return ds;
}

void queue_danmaku(danmaku_state *ds, danmaku_info *dan)
{
	pthread_mutex_lock(&ds->mutex);
	list_append_danmaku(ds->waiting, dan);
	pthread_mutex_unlock(&ds->mutex);
}

void schedule_danmaku(danmaku_state *dstat)
{
	pNode waiting = dstat->waiting;
	pNode issued = dstat->issued;
	pNode s, t;

	/* remove danmakus with ttl is 0 */
	FOR_ALL_DANMAKU(issued, s) {
		if (s->dan->ttl == 0) {
			free_danmaku(s->dan);
			s = list_remove(s);
		}
	}

	/* move waiting danmakus that can be issued */
	FOR_ALL_DANMAKU(waiting, t) {
		danmaku_info *dt = t->dan;
		int can_issue = 1;
		if (dt->type == DAN_TOP || dt->type == DAN_BOTTOM) {
			FOR_ALL_DANMAKU(issued, s) {
				if (s->dan->type == dt->type) {
					can_issue = 0;
					break;
				}
			}
		} else if (dt->type == DAN_L2R || dt->type == DAN_R2L) {
			FOR_ALL_DANMAKU(issued, s) {
				danmaku_info *ds = s->dan;
				if (ds->type != dt->type)
					continue;
				if ((ds->type == DAN_L2R && ds->x > 0) ||
					 (ds->type == DAN_R2L && ds->x < ds->di->width - ds->width))
					continue;
				if (!(dt->y > ds->y + ds->height || dt->y + dt->height < ds->y)) {
					can_issue = 0;
					break;
				}
			}
		}
		if (can_issue) {
			list_append_danmaku(issued, dt);
			issue_danmaku(dt);
			pthread_mutex_lock(&dstat->mutex);
			t = list_remove(t);
			pthread_mutex_unlock(&dstat->mutex);
		}
	}

	/* move the issued danmakus */
	FOR_ALL_DANMAKU(issued, s)
		move_danmaku(s->dan);
}
