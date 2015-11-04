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

#include "clock_fwk_util.h"
#include <vconf.h>
#include <stdlib.h>
#include <unicode/ustring.h>
#include <unicode/ucol.h>

/**
* send
* This function is  used to load edc
* @param           parent   pointer to evas_object, as the parent evas object
* @param           file     pointer to char, as the edc file path
* @param           group    pointer to char, as the group name in edc
* @return          return a Evas_Object, is the child of @parent or NULL if error
* @exception       if error happen, return NULL
*/
Evas_Object *load_edj(Evas_Object * parent, const char *file, const char *group)
{
	Evas_Object *eo = NULL;
	int error_code;
	eo = elm_layout_add(parent);
	if (eo) {
		error_code = elm_layout_file_set(eo, file, group);
		if (!error_code) {
			evas_object_del(eo);
			return NULL;
		}
		evas_object_size_hint_weight_set(eo, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	}
	return eo;
}

/**
* send
* This function is  used to del a window
* @param           data    pointer to data
* @param           obj     pointer to current evas_object
* @param           event   pointer to event
* @return          void
* @exception
*/
void win_del(void *data, Evas_Object * obj, void *event)
{
	elm_exit();
}

/**
* send
* This function is  used to handle main quit
* @param           data[in]         pointer to data
* @param           obj[in]          pointer to current evas object
* @param           emission[in]     string for path
* @param           source[in]       string for source
* @return          void
* @exception
*/
void main_quit_cb(void *data, Evas_Object * obj, const char *emission, const char *source)
{
	elm_exit();
}

/**
* send
* This function is  used to create elm_win
* @param           name   pointer to char, as the win's title
* @return          return a Evas_Object, is a win main evas object or NULL if failed
* @exception       if error happen, return NULL
*/
Evas_Object *create_win(const char *name)
{
	Evas_Object *eo = NULL;
	eo = elm_win_add(NULL, name, ELM_WIN_BASIC);
	if (eo) {
		elm_win_title_set(eo, name);
		evas_object_smart_callback_add(eo, "delete,request", win_del, NULL);
	}
	return eo;
}

/**
* send
* This function is  used to create bg
* @param           name   pointer to a win main evas object
* @return          return a Evas_Object, is a bg evas object or NULL if failed
* @exception       if error happen, return NULL
*/
Evas_Object *create_bg(Evas_Object * win)
{
	Evas_Object *bg = elm_bg_add(win);
	retvm_if(!bg, NULL, "bg null");
	evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	//elm_win_resize_object_add(win, bg);
	evas_object_show(bg);
	return bg;
}

/**
* send
* This function is  used to update language
* @param           eo[in]         pointer to Evas object
* @param           tp[in]         pointer to struct text_part
* @param           size[in]       @tp number
* @return          void
* @exception
*/
void update_ts(Evas_Object * eo, struct text_part *tp, int size)
{
	int i;
	if (eo == NULL || tp == NULL || size < 0) {
		return;
	}
	for (i = 0; i < size; i++) {
		if (tp[i].part && tp[i].msgid) {
			edje_object_part_text_set(eo, tp[i].part, _(tp[i].msgid));
		}
	}
}

int get_string_from_time(time_t stime, char *time, int num, int time_format)
{
	struct tm tm;
	localtime_r(&stime, &tm);
	switch (time_format) {
	case VCONFKEY_TIME_FORMAT_12:
		strftime(time, sizeof(char) * num, "%I:%M", &tm);
		break;
	case VCONFKEY_TIME_FORMAT_24:
		strftime(time, sizeof(char) * num, "%H:%M", &tm);
		break;
	default:
		break;
	}
	return SUCCESS;
}

Eina_Bool clk_evas_object_color_class_set(Evas_Object * obj,
		Edje_Color_Class * color_class)
{
	retvm_if(NULL_CHECK(obj), EINA_FALSE, "obj null");
	retvm_if(NULL_CHECK(color_class), EINA_FALSE, "color_class null");
	return edje_object_color_class_set(_EDJ(obj), color_class->name,
			color_class->r, color_class->g,
			color_class->b, color_class->a,
			color_class->r2, color_class->g2,
			color_class->b2, color_class->a2,
			color_class->r3, color_class->g3, color_class->b3, color_class->a3);
}
