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

#ifndef __helloUG_efl_H__
#define __helloUG_efl_H__

#include <Elementary.h>

#define IS_STR_EQUAL(a, b) (!strcmp(a, b))
#define IS_STR_NOT_EQUAL(a, b) (strcmp(a, b))

//******************define:free *************************************
#define FREEIF(p) ({if (p) {free(p); p = NULL; }})
#define EVAS_OBJECT_DELIF(p) ({if (p) {evas_object_del(p); p = NULL; }})
#define ECORE_TIMER_DELIF(p) ({if (p) {ecore_timer_del(p); p = NULL; }})

#endif				/* __helloUG_efl_H__ */
