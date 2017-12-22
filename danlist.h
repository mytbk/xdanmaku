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

#include "danmaku.h"
#include <pthread.h>

struct Node
{
	danmaku_info *dan;
	struct Node *prev;
	struct Node *next;
};

typedef struct Node *pNode;

typedef struct
{
	pNode issued;
	pNode waiting;
	pthread_mutex_t mutex;
} danmaku_state;

danmaku_state *new_danstat(void);
void queue_danmaku(danmaku_state *ds, danmaku_info *dan);
void schedule_danmaku(danmaku_state *ds);

#define FOR_ALL_DANMAKU(l, i) for (i = l->next; i != l; i = i->next)
