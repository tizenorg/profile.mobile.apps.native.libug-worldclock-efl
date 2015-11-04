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


#ifndef __CLK_FWK_UTIL_H__
#define __CLK_FWK_UTIL_H__

#include <Elementary.h>
#include "clock_fwk_define.h"

Evas_Object *load_edj(Evas_Object * parent, const char *file, const char *group);
void win_del(void *data, Evas_Object * obj, void *event);
void main_quit_cb(void *data, Evas_Object * obj, const char *emission,
		const char *source);
Evas_Object *create_win(const char *name);
Evas_Object *create_bg(Evas_Object * win);
void update_ts(Evas_Object * eo, struct text_part *tp, int size);
int update_tc(Evas_Object * eo, struct _tc *tc);

int get_string_from_time(time_t stime, char *time, int num, int time_format);

Eina_Bool clk_evas_object_color_class_set(Evas_Object * obj, Edje_Color_Class *);
#endif				//__CLK_FWK_UTIL_H__
