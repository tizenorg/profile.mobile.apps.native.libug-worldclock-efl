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


#ifndef UG_MODULE_API
#define UG_MODULE_API __attribute__ ((visibility("default")))
#endif

#include <vconf.h>
#include <Elementary.h>
#include <ui-gadget-module.h>

#include "worldclock.h"
#include "worldclock_dlog.h"
#include "ug_worldclock_efl.h"
#include "worldclock_add_view.h"
#include "efl_extension.h"
#include "clock_fwk_icu_label.h"
#include "clock_fwk_util.h"

#ifndef _
#define _(s)  dgettext(PACKAGE, s)
#endif

struct ug_data {
	ui_gadget_h ug;
	//Elm_Theme *th;

	struct appdata *ad;
};

static struct ug_data *g_ugd;

/**
 * Callback func which should be called when exit from this ug.
 *
 * @param[in]  data         The data which used in this function
 *
 * @return
 */
static void __ug_return_cb(void *data, Eina_Bool isReload)
{
	CLK_FUN_BEG();
	struct ug_data *ugd = NULL;
	Wcl_CitySet *cs = NULL;
	app_control_h app_control = NULL;

	ret_if(!g_ugd);

	ugd = g_ugd;
	cs = ugd->ad->return_data;

	if (cs) {
		app_control_create(&app_control);
		app_control_add_extra_data(app_control, "city", cs->city);
		app_control_add_extra_data(app_control, "city_name", _(cs->city));
		app_control_add_extra_data(app_control, "country", cs->country);
		app_control_add_extra_data(app_control, "country_name", _(cs->country));

		if (ugd->ad->caller != WCL_CALLER_IS_APP_IT_SELF) {
			const char *timezone = cs->timezone;
			if (timezone == strstr(timezone, "GMT")) {
				timezone += 3;
			}

			app_control_add_extra_data(app_control, "timezone", timezone);
			app_control_add_extra_data(app_control, "tzpath", cs->tz_path);
			CLK_INFO("[Result] city: %s, city_name: %s, country: %s, timezone: %s, tzpath: %s\n", cs->city, _(cs->city), cs->country, timezone, cs->tz_path);
		}

		ug_send_result(ugd->ug, app_control);
		app_control_destroy(app_control);

		FREEIF(ugd->ad->return_data);
#ifdef FEATURE_SORT_ORDER
		EVAS_OBJECT_DELIF(ugd->ad->more_popup);
#endif
	} else {
		CLK_ERR("No return data selected!");
	}
	CLK_FUN_END();
}

/**
 * Create main layout
 *
 * @param[in]  parent  The parent Evas_Object which main layout belong to
 *
 * @return             NULL if an error occurred
 *                     pointer to new main layout otherwise
 */
static Evas_Object *__ug_create_main_layout(Evas_Object * parent)
{
	CLK_FUN_BEG();
	Evas_Object *layout = NULL;

	retv_if(parent == NULL, NULL);
	// add layout
	layout = elm_layout_add(parent);
	retvm_if(layout == NULL, NULL, "Failed elm_layout_add.\n");
	// theme
	elm_layout_theme_set(layout, "layout", "application", "default");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	evas_object_show(layout);

	CLK_FUN_END();
	return layout;
}

/**
 * Create navigation layout
 *
 * @param[in]  parent  The parent Evas_Object which navigation layout belong to
 *
 * @return             NULL if an error occurred
 *                     pointer to new navigation layout otherwise
 */
static Evas_Object *__ug_create_navigation_layout(Evas_Object * parent)
{
	CLK_FUN_BEG();
	Evas_Object *navi_bar = NULL;

	retv_if(parent == NULL, NULL);
	// add navigationbar
	navi_bar = elm_naviframe_add(parent);
	elm_naviframe_prev_btn_auto_pushed_set(navi_bar, EINA_FALSE);
	eext_object_event_callback_add(navi_bar, EEXT_CALLBACK_BACK, eext_naviframe_back_cb,
			NULL);
	eext_object_event_callback_add(navi_bar, EEXT_CALLBACK_MORE, eext_naviframe_more_cb,
			NULL);
	// set content
	elm_object_part_content_set(parent, "elm.swallow.content", navi_bar);
	// show navigationbar
	evas_object_show(navi_bar);

	CLK_FUN_END();
	return navi_bar;
}

static void __ug_lang_update(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	Elm_Object_Item *navi_it = elm_naviframe_top_item_get(ad->navi_bar);
	if (!navi_it) {
		return;
	}
	switch (ad->caller) {
	case WCL_CALLER_IS_APP_IT_SELF:
	case WCL_CALLER_IS_LIVEBOX:
		elm_object_item_domain_translatable_text_set(navi_it,PACKAGE,"IDS_WCL_OPT_ADDCITY");
		break;
	case WCL_CALLER_IS_SHARED_LIBRARY:
		elm_object_item_domain_translatable_text_set(navi_it,PACKAGE,"IDS_WCL_HEADER_SELECT_TIME_ZONE");
		break;
	case WCL_CALLER_IS_UI_GADGET:
		elm_object_item_domain_translatable_text_set(navi_it,PACKAGE,"IDS_WCL_BODY_TIME_ZONE");
		break;
	default:
		break;
	}
}

static void _hide_title(void *data, Evas_Object * obj, void *event_info)
{
	CLK_FUN_BEG();
	struct appdata *ad = (struct appdata *)data;
	Evas_Object *nf = ad->navi_bar;

	int rotation = elm_win_rotation_get(ad->win);
	if (rotation == 0 || rotation == 180) {
		CLK_INFO("hide title return :: App is protrate");
		return;
	}

	if (!nf) {
		return;
	}

	Elm_Object_Item *navi_it = elm_naviframe_top_item_get(nf);
	if (!navi_it) {
		return;
	}
	elm_naviframe_item_title_enabled_set(navi_it, EINA_FALSE, EINA_FALSE);
	CLK_FUN_END();
}

static void _show_title(void *data, Evas_Object * obj, void *event_info)
{
	CLK_FUN_BEG();
	struct appdata *ad = (struct appdata *)data;
	Evas_Object *nf = ad->navi_bar;
	if (!nf) {
		return;
	}

	Elm_Object_Item *navi_it = elm_naviframe_top_item_get(nf);
	if (!navi_it) {
		return;
	}
	elm_naviframe_item_title_enabled_set(navi_it, EINA_TRUE, EINA_FALSE);
	CLK_FUN_END();
}

static void *on_create(ui_gadget_h ug, enum ug_mode mode, app_control_h data, void *priv)
{
	CLK_FUN_BEG();
	Evas_Object *win = NULL;
	struct ug_data *ugd = NULL;
	struct appdata *ad = NULL;
	char *caller_name = NULL;
	char *city_index = NULL;
	char *text_id = NULL;

	retv_if(ug == NULL || priv == NULL, NULL);

	ugd = priv;
	ugd->ug = ug;
	// get ug window
	win = (Evas_Object *) ug_get_window();
	retv_if(win == NULL, NULL);
	// allocate data
	ad = (struct appdata *)calloc(1, sizeof(struct appdata));
	retv_if(ad == NULL, NULL);
	/*disable rotate */
	ad->win = win;
	ad->conform = (Evas_Object *) ug_get_conformant();
	ad->ug = ug;
	ad->parent = ug_get_parent_layout(ug);
	GOTO_ERROR_IF(!ad->parent);

	if (data) {
		app_control_get_extra_data(data, "caller", &caller_name);
		app_control_get_extra_data(data, "city_index", &city_index);

		app_control_get_extra_data(data, "translation_request", &text_id);
	}
	if (city_index) {
		ad->city_index = atoi(city_index);
	} else {
		ad->city_index = -1;
	}
	FREEIF(city_index);

	// set caller flag
	if (caller_name && !strcmp("clock", caller_name)) {
		ad->caller = WCL_CALLER_IS_APP_IT_SELF;
	} else if (caller_name && !strcmp("dual_clock", caller_name)) {
		ad->caller = WCL_CALLER_IS_LIVEBOX;
	} else {
		ad->caller = WCL_CALLER_IS_UI_GADGET;
	}
	CLK_INFO("ad->caller = %d", ad->caller);
	FREEIF(caller_name);
	/* language setting */
	bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);

	if (text_id) {
		CLK_INFO("text_id = %d", text_id);
		app_control_h app_control = NULL;
		app_control_create(&app_control);
		app_control_add_extra_data(app_control, "city_name", _(text_id));
		ug_send_result(ug, app_control);
		app_control_destroy(app_control);

		FREEIF(text_id);
		ug_destroy_me(ug);
		GOTO_ERROR_IF(true);
		return NULL;
	}

	/* main layout */
	ad->ly_main = __ug_create_main_layout(ad->parent);
	GOTO_ERROR_IF(ad->ly_main == NULL);
	ad->bg = create_bg(ad->ly_main);
	elm_object_part_content_set(ad->ly_main, "elm.swallow.bg", ad->bg);

	/* navigation bar */
	ad->navi_bar = __ug_create_navigation_layout(ad->ly_main);
	GOTO_ERROR_IF(ad->navi_bar == NULL);

	// set selection flag
	ad->selectionFlag = WCL_SELECT_IF_HAS_TZPATH;
	// create add view for ug
	worldclock_ugview_add(ad->navi_bar, ad, __ug_return_cb);

	evas_object_smart_callback_add(ad->conform, "virtualkeypad,state,on", _hide_title,
			ad);
	evas_object_smart_callback_add(ad->conform, "virtualkeypad,state,off",
			_show_title, ad);
	evas_object_smart_callback_add(ad->conform, "clipboard,state,on", _hide_title,
			ad);
	evas_object_smart_callback_add(ad->conform, "clipboard,state,off", _show_title,
			ad);

	ugd->ad = ad;
	g_ugd = ugd;

	CLK_FUN_END();
	return ad->ly_main;

error:
	if (ad) {
		// reset navigation bar
		if (ad->navi_bar) {
			evas_object_del(ad->navi_bar);
			ad->navi_bar = NULL;
		}
		// reset layout
		if (ad->ly_main) {
			evas_object_del(ad->ly_main);
			ad->ly_main = NULL;
		}
		if (ad->bg) {
			evas_object_del(ad->bg);
			ad->bg = NULL;
		}
		if (ad->conform) {
			evas_object_smart_callback_del(ad->conform,
					"virtualkeypad,state,on", _hide_title);
			evas_object_smart_callback_del(ad->conform,
					"virtualkeypad,state,off", _show_title);
			evas_object_smart_callback_del(ad->conform, "clipboard,state,on",
					_hide_title);
			evas_object_smart_callback_del(ad->conform, "clipboard,state,off",
					_show_title);
			ad->conform = NULL;
		}
		free(ad);
		ad = NULL;
	}
	return NULL;
}

static void on_start(ui_gadget_h ug, app_control_h data, void *priv)
{
	CLK_FUN_BEG();
	struct appdata *ad = NULL;

	ad = g_ugd->ad;

	if (ad->conform == NULL) {
		CLK_INFO("conformant get failed in on_create");
		CLK_INFO("retry to get conformant");
		ad->conform = (Evas_Object *) ug_get_conformant();
		evas_object_smart_callback_add(ad->conform, "virtualkeypad,state,on",
				_hide_title, ad);
		evas_object_smart_callback_add(ad->conform, "virtualkeypad,state,off",
				_show_title, ad);
		evas_object_smart_callback_add(ad->conform, "clipboard,state,on",
				_hide_title, ad);
		evas_object_smart_callback_add(ad->conform, "clipboard,state,off",
				_show_title, ad);
	}
	CLK_FUN_END();
}

static void on_pause(ui_gadget_h ug, app_control_h data, void *priv)
{
	CLK_FUN_BEG();
}

static void on_resume(ui_gadget_h ug, app_control_h data, void *priv)
{
	CLK_FUN_BEG();
}

static void on_destroy(ui_gadget_h ug, app_control_h data, void *priv)
{
	CLK_FUN_BEG();
	struct ug_data *ugd;

	ret_if(!ug || !priv);

	ugd = priv;
	if (ugd->ad) {
		struct appdata *ad = ugd->ad;
		worldclock_ugview_free(ad);

#ifdef FEATURE_SORT_ORDER
		/*reset layout */
		EVAS_OBJECT_DELIF(ad->more_popup);
		EVAS_OBJECT_DELIF(ad->sort_popup);
#endif

		EVAS_OBJECT_DELIF(ad->add_index);
		EVAS_OBJECT_DELIF(ad->add_genlist);
		EVAS_OBJECT_DELIF(ad->navi_bar);
		EVAS_OBJECT_DELIF(ad->ly_main);
		EVAS_OBJECT_DELIF(ad->bg);
		FREEIF(ad->return_data);

		ECORE_TIMER_DELIF(ad->add_view_quit_timer);
		ECORE_TIMER_DELIF(ad->add_view_update_timer);
		ECORE_TIMER_DELIF(ad->search_timer);

		if (ad->conform) {
			evas_object_smart_callback_del(ad->conform,
					"virtualkeypad,state,on", _hide_title);
			evas_object_smart_callback_del(ad->conform,
					"virtualkeypad,state,off", _show_title);
			evas_object_smart_callback_del(ad->conform, "clipboard,state,on",
					_hide_title);
			evas_object_smart_callback_del(ad->conform, "clipboard,state,off",
					_show_title);
			ad->conform = NULL;
		}
		/*enable rotate */
		free(ugd->ad);
		ugd->ad = NULL;
	}

	CLK_FUN_END();
}

static void on_message(ui_gadget_h ug, app_control_h msg, app_control_h data, void *priv)
{
}

static void on_event(ui_gadget_h ug, enum ug_event event, app_control_h data, void *priv)
{
	CLK_FUN_BEG();
	ret_if(!ug || !priv);

	struct ug_data *ugd;
	struct appdata *ad;
	Elm_Object_Item *it = NULL;

	ugd = priv;
	if (ugd->ad) {
		ad = ugd->ad;
	} else {
		return;
	}

	switch (event) {
	case UG_EVENT_LOW_MEMORY:
		CLK_INFO("Event : UG_EVENT_LOW_MEMORY");
		break;
	case UG_EVENT_LOW_BATTERY:
		CLK_INFO("Event : UG_EVENT_LOW_BATTERY");
		break;
	case UG_EVENT_LANG_CHANGE:
		CLK_INFO("Event : UG_EVENT_LANG_CHANGE");
		__ug_lang_update(ad);
		uninit_alphabetic_index();
		char *lang = vconf_get_str(VCONFKEY_LANGSET);
		init_alphabetic_index(lang);
		FREEIF(lang);
		worldclock_ugview_update(ad);
		break;
	case UG_EVENT_ROTATE_PORTRAIT:
	case UG_EVENT_ROTATE_PORTRAIT_UPSIDEDOWN:
		CLK_INFO("Event : UG_EVENT_ROTATE_PORTRAIT");
		_show_title(ad, NULL, NULL);
		it = elm_index_selected_item_get(ad->add_index, 0);
		if (it != NULL) {
			elm_index_item_selected_set(it, EINA_FALSE);
		}
		break;
	case UG_EVENT_ROTATE_LANDSCAPE:
	case UG_EVENT_ROTATE_LANDSCAPE_UPSIDEDOWN:
		CLK_INFO("Event : UG_EVENT_ROTATE_LANDSCAPE");
		Ecore_IMF_Context *imf_context = elm_entry_imf_context_get(ad->searchbar_entry);
		if (imf_context) {
			if (ECORE_IMF_INPUT_PANEL_STATE_HIDE !=
					ecore_imf_context_input_panel_state_get(imf_context)) {
				_hide_title(ad, NULL, NULL);
			}
		}

		it = elm_index_selected_item_get(ad->add_index, 0);
		if (it != NULL) {
			elm_index_item_selected_set(it, EINA_FALSE);
		}
		//CLK_INFO("current level = %d", level);
		break;
	default:
		CLK_INFO("Event : %d", event);
		break;
	}
	CLK_FUN_END();
}

static void on_key_event(ui_gadget_h ug, enum ug_key_event event,
		app_control_h data, void *priv)
{
	CLK_FUN_BEG();
	ret_if(!ug);

	switch (event) {
	case UG_KEY_EVENT_END:
		ug_destroy_me(ug);
		break;
	default:
		break;
	}
	CLK_FUN_END();
}

UG_MODULE_API int UG_MODULE_INIT(struct ug_module_ops *ops)
{
	CLK_FUN_BEG();
	struct ug_data *ugd;

	retv_if(!ops, -1);

	ugd = calloc(1, sizeof(struct ug_data));
	retv_if(!ugd, -1);

	ops->create = on_create;
	ops->start = on_start;
	ops->pause = on_pause;
	ops->resume = on_resume;
	ops->destroy = on_destroy;
	ops->message = on_message;
	ops->event = on_event;
	ops->key_event = on_key_event;
	ops->priv = ugd;
	ops->opt = UG_OPT_INDICATOR_ENABLE;

	CLK_FUN_END();
	return 0;
}

UG_MODULE_API void UG_MODULE_EXIT(struct ug_module_ops *ops)
{
	CLK_FUN_BEG();
	struct ug_data *ugd;

	ret_if(!ops);

	ugd = ops->priv;

	FREEIF(ugd);
	g_ugd = NULL;

	CLK_FUN_END();
}
