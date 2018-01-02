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

#include <curl/curl.h>
#include <uuid/uuid.h>
#include <json-c/json_tokener.h>
#include <string.h>
#include <stdlib.h>

/*
 * demo server: https://dm.tuna.moe:8443/channel/demo
 * api: GET "/api/v1.1/channels/<channel>/danmaku"
 * we need to create a subscriber uuid first
 *
 * headers:
 *   - "X-GDANMAKU-SUBSCRIBER-ID": uuid
 *   - "X-GDANMAKU-AUTH-KEY": password
 *
 * example response: [{"text": "tttt", "style": "blue", "position": "fly"}]
 */

static void uuidgen(char *u)
{
	uuid_t uu;
	uuid_generate(uu);
	uuid_unparse(uu, u);
}

static size_t get_json_callback(void *contents, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	struct MemoryStruct *mem = (struct MemoryStruct *)userp;

	json_tokener *tok = json_tokener_new();
	*((json_object **)userp) = json_tokener_parse_ex(tok, contents, realsize);
	json_tokener_free(tok);

	return realsize;
}

typedef struct
{
	CURL *curl;
	char *fullurl;
	struct curl_slist *chunk;
} curl_subscriber;

static const char *subid_s = "X-GDANMAKU-SUBSCRIBER-ID: ";
static const char *authkey_s = "X-GDANMAKU-AUTH-KEY: ";

void new_subscriber(curl_subscriber *cs, const char *url, const char *chn, const char *passwd)
{
	CURL *curl = curl_easy_init();

	char id_header[64];
	strcpy(id_header, subid_s);
	uuidgen(id_header + strlen(subid_s));
	struct curl_slist *chunk = NULL;
	chunk = curl_slist_append(chunk, id_header);

	if (passwd != NULL) {
		int L = strlen(authkey_s);
		char *authkey_header = (char*)malloc(L + strlen(passwd) + 1);
		strcpy(authkey_header, authkey_s);
		strcpy(authkey_header + L, passwd);
		chunk = curl_slist_append(chunk, authkey_header);
	}

	const char *api = "/api/v1.1/channels/%s/danmaku";
	int ulen = strlen(url);
	char *u = malloc(ulen + strlen(chn) + strlen(api));
	memcpy(u, url, ulen);
	if (u[ulen-1] == '/')
		ulen--;
	sprintf(u+ulen, api, chn);

	if (curl) {
		curl_easy_setopt(curl, CURLOPT_URL, u);
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, get_json_callback);
	}

	cs->curl = curl;
	cs->fullurl = u;
	cs->chunk = chunk;
}

json_object *do_subscribe(curl_subscriber *cs)
{
	CURLcode res;
	json_object *jobj;

	curl_easy_setopt(cs->curl, CURLOPT_WRITEDATA, (void *)&jobj);

	/* Perform the request, res will get the return code */
	res = curl_easy_perform(cs->curl);
	/* Check for errors */
	if(res != CURLE_OK)
		fprintf(stderr, "curl_easy_perform() failed: %s\n",
				  curl_easy_strerror(res));
	return jobj;
}

void free_subscriber(curl_subscriber *cs)
{
	curl_easy_cleanup(cs->curl);
	curl_slist_free_all(cs->chunk);
	free(cs->fullurl);
}
