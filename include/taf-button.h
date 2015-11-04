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

#ifndef __TAF_BUTTON_H__
#define __TAF_BUTTON_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <Elementary.h>
#include "taf.h"

	typedef struct {
		const char *normal;
		const char *pressed;
		const char *disabled;
	} element_s;

	typedef struct {
		element_s bg;
		element_s fg;
		element_s text;
		const char * edje_file;
	} btn_info_s;

	typedef void (*taf_event_callback_f) (void *data, const char *event, void *event_info);

	TAF_API Evas_Object *taf_button_create(Evas_Object * parent, btn_info_s * info);
	TAF_API void taf_button_event_callback_add(Evas_Object * obj, taf_event_callback_f cb,
			void *data);
	TAF_API void taf_button_disabled_set(Evas_Object * obj, Eina_Bool disabled);
	TAF_API void taf_button_enable_longpress(Evas_Object * obj);
	TAF_API void taf_button_text_set(Evas_Object * obj, const char *text);
	TAF_API void taf_button_style_set(Evas_Object * obj, btn_info_s * info);
	TAF_API void taf_button_press(Evas_Object *obj);
	TAF_API void taf_button_unpress(Evas_Object *obj);
#ifdef __cplusplus
}
#endif

#endif
