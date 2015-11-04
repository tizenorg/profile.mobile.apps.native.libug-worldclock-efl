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


#include "clock_fwk_define.h"
#include "taf-button.h"
#include "taf.h"
#include "worldclock_const.h"
#include "clock_fwk_dlog.h"

typedef enum {
	STATE_NORMAL,
	STATE_PRESS,
	STATE_DIM,
} btn_state_e;

typedef struct {
	Evas_Object *self;
	Evas_Object *customized_layout;
	btn_info_s *info;
	/* event callback */
	taf_event_callback_f event_cb;
	void *event_data;

	/* status */
	Eina_Bool longpress_enabled;
	Eina_Bool longpress_emited;
	btn_state_e state;
	char txt[TAF_BUF_SIZE_GRAND];
} taf_button_s;

static void __on_pressed(void *data, Evas_Object * obj, void *event_info);
static void __on_unpressed(void *data, Evas_Object * obj, void *event_info);
static void __on_clicked(void *data, Evas_Object * obj, void *event_info);
static void __on_repeated(void *data, Evas_Object * obj, void *event_info);

static void __on_delete(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
	taf_button_s *btn = (taf_button_s *) evas_object_data_del(obj,
			"__image_button_private");
	if (btn != NULL) {
		/* why we do this?
		   when we fast tap the button, the "unpressed" may still remain in the message queue,
		   that will cause crash, so we should delete the callback
		   ex: fast tap "add page" button
		 */
		evas_object_smart_callback_del(btn->self, "pressed", __on_pressed);
		evas_object_smart_callback_del(btn->self, "unpressed", __on_unpressed);
		evas_object_smart_callback_del(btn->self, "clicked", __on_clicked);
		evas_object_smart_callback_del(btn->self, "repeated", __on_repeated);
		free(btn);
	}
}

static void __btn_state_update(taf_button_s * btn)
{
	const char *bg = btn->info->bg.normal;
	const char *fg = btn->info->fg.normal;
	const char *text = btn->info->text.normal;

	if (btn->state == STATE_PRESS) {
		bg = btn->info->bg.pressed;
		fg = btn->info->fg.pressed;
		text = btn->info->text.pressed;
	} else if (btn->state == STATE_DIM) {
		bg = btn->info->bg.disabled;
		fg = btn->info->fg.disabled;
		text = btn->info->text.disabled;
	}

	taf_swallowed_layout_create(btn->customized_layout, "elm.swallow.bg",
			btn->info->edje_file, bg);
	taf_swallowed_layout_create(btn->customized_layout, "elm.swallow.fg",
			btn->info->edje_file, fg);
	taf_swallowed_layout_create(btn->customized_layout, "elm.swallow.text",
			btn->info->edje_file, text);
	Evas_Object *text_obj = elm_object_part_content_get(btn->customized_layout,
			"elm.swallow.text");
	if (strncmp(btn->txt, "IDS_", 4) == 0) {
		elm_object_domain_translatable_part_text_set(text_obj, "elm.text",PACKAGE,btn->txt);
	} else {
		elm_object_part_text_set(text_obj, "elm.text", btn->txt);
	}
}

static void __on_pressed(void *data, Evas_Object * obj, void *event_info)
{
	taf_button_s *btn = (taf_button_s *) data;
	ret_if(btn->state == STATE_DIM);

	btn->longpress_emited = EINA_FALSE;
	btn->state = STATE_PRESS;
	__btn_state_update(btn);

	/*play sound*/
	taf_beep();
}

static void __on_unpressed(void *data, Evas_Object * obj, void *event_info)
{
	taf_button_s *btn = (taf_button_s *) data;
	ret_if(btn->state == STATE_DIM);
	btn->state = STATE_NORMAL;
	__btn_state_update(btn);
}

static void __on_clicked(void *data, Evas_Object * obj, void *event_info)
{
	taf_button_s *btn = (taf_button_s *) data;
	if (!btn->longpress_emited) {
		if (btn->event_cb != NULL) {
			btn->event_cb(btn->event_data, "clicked", NULL);
		}
	}
}

static void __on_repeated(void *data, Evas_Object * obj, void *event_info)
{
	taf_button_s *btn = (taf_button_s *) data;
	if (btn->longpress_enabled) {
		if (!btn->longpress_emited) {
			btn->longpress_emited = EINA_TRUE;
			if (btn->event_cb != NULL) {
				btn->event_cb(btn->event_data, "longpress", NULL);
			}
		}
	}
}

Evas_Object *taf_button_create(Evas_Object * parent, btn_info_s * info)
{

	if (parent == NULL) {
		return NULL;
	}

	taf_button_s *btn = CALLOC(1, taf_button_s);
	if (btn == NULL) {
		return NULL;
	}
	btn->info = info;
	/* elm_button */
	btn->self = elm_button_add(parent);
	elm_object_style_set(btn->self, "taf/transparent");
	evas_object_smart_callback_add(btn->self, "pressed", __on_pressed, btn);
	evas_object_smart_callback_add(btn->self, "unpressed", __on_unpressed, btn);
	evas_object_smart_callback_add(btn->self, "clicked", __on_clicked, btn);
	evas_object_smart_callback_add(btn->self, "repeated", __on_repeated, btn);
	evas_object_data_set(btn->self, "__image_button_private", btn);
	evas_object_event_callback_add(btn->self, EVAS_CALLBACK_DEL, __on_delete, NULL);

	/* customized layout */
	btn->customized_layout =
			taf_layout_create(parent, WCL_EDJ_NAME, "taf.button");
	elm_object_part_content_set(btn->self, "taf.swallow.content",
			btn->customized_layout);

	/* Set bg, fg, text */
	btn->state = STATE_NORMAL;
	__btn_state_update(btn);

	/* done */
	return btn->self;
}

void taf_button_event_callback_add(Evas_Object * obj, taf_event_callback_f cb,
		void *data)
{
	taf_button_s *btn = (taf_button_s *) evas_object_data_get(obj,
			"__image_button_private");
	ret_if(btn == NULL || (cb == NULL));

	btn->event_cb = cb;
	btn->event_data = data;
}

void taf_button_disabled_set(Evas_Object * obj, Eina_Bool disabled)
{
	taf_button_s *btn = (taf_button_s *) evas_object_data_get(obj,
			"__image_button_private");
	ret_if(btn == NULL);

	elm_object_disabled_set(obj, disabled);
	btn->state = disabled ? STATE_DIM : STATE_NORMAL;
	__btn_state_update(btn);
}

void taf_button_enable_longpress(Evas_Object * obj)
{
	taf_button_s *btn = (taf_button_s *) evas_object_data_get(obj,
			"__image_button_private");
	ret_if(btn == NULL);
	btn->longpress_enabled = EINA_TRUE;
	elm_button_autorepeat_set(obj, EINA_TRUE);
	elm_button_autorepeat_initial_timeout_set(obj, 1.0);
	elm_button_autorepeat_gap_timeout_set(obj, 1.0);
}

void taf_button_text_set(Evas_Object * obj, const char *txt)
{

	taf_button_s *btn = (taf_button_s *) evas_object_data_get(obj,
			"__image_button_private");
	ret_if(btn == NULL);

	Evas_Object *text =
			elm_object_part_content_get(btn->customized_layout, "elm.swallow.text");
	if (strncmp(txt, "IDS_", strlen("IDS_")) == 0) {
		elm_object_domain_translatable_part_text_set(text, "elm.text",
				PACKAGE, txt);
	} else {
		elm_object_part_text_set(text, "elm.text", txt);
	}
	snprintf(btn->txt, TAF_BUF_SIZE_GRAND, "%s", txt);
}

void taf_button_style_set(Evas_Object * obj, btn_info_s *info)
{
	ret_if(obj == NULL);
	taf_button_s *btn = (taf_button_s *) evas_object_data_get(obj,
			"__image_button_private");
	ret_if(btn == NULL);
	btn->info = info;
	taf_swallowed_layout_create
	(btn->customized_layout, "elm.swallow.bg",
	 info->edje_file, btn->info->bg.normal);
	taf_swallowed_layout_create
	(btn->customized_layout, "elm.swallow.fg",
	 info->edje_file, btn->info->fg.normal);
}

void taf_button_press(Evas_Object *obj)
{
	taf_button_s *btn = (taf_button_s *) evas_object_data_get(obj,
			"__image_button_private");
	ret_if(!btn);

	__on_pressed(btn,obj,NULL);
}

void taf_button_unpress(Evas_Object *obj)
{
	taf_button_s *btn = (taf_button_s *) evas_object_data_get(obj,
			"__image_button_private");
	ret_if(!btn);

	__on_unpressed(btn,obj,NULL);
}

