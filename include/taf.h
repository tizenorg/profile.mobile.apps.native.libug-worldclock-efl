/*
 * Copyright (c) 2012-2015 Samsung Electronics Co., Ltd All Rights Reserved
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
 *
 */

#ifndef __TAF_LYOUT_H__
#define __TAF_LYOUT_H__

#include <Elementary.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef TAF_API
#define TAF_API __attribute__ ((visibility ("default")))
#endif

#define TAF_BUF_SIZE_TINY          	(32)
#define TAF_BUF_SIZE_SMALL          (64)
#define TAF_BUF_SIZE_MEDIUM         (128)
#define TAF_BUF_SIZE_LARGE          (256)
#define TAF_BUF_SIZE_SUPER          (512)
#define TAF_BUF_SIZE_GRAND          (1024)

Evas_Object *taf_layout_create(Evas_Object * parent, const char *file,
		const char *group);
Evas_Object *taf_swallowed_layout_create(Evas_Object * parent, const char *part,
		const char *file, const char *group);
void taf_init(Evas_Object * parent);
void taf_beep();
void taf_destroy();

#ifdef __cplusplus
}
#endif
#endif
