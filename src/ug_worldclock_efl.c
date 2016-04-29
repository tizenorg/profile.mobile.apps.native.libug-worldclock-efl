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

#include "worldclock.h"
#include "worldclock_dlog.h"
#include "ug_worldclock_efl.h"
#include "worldclock_add_view.h"
#include "worldclock_util.h"
#include "efl_extension.h"
#include "clock_fwk_icu_label.h"

#ifndef _
#define _(s)  dgettext(PACKAGE, s)
#endif

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
	struct appdata *ad = NULL;
	Wcl_CitySet *cs = NULL;
	app_control_h reply = NULL;


	ad = data;
	cs = ad->return_data;

	if (cs) {
		app_control_create(&reply);
		app_control_add_extra_data(reply, "city", cs->city);
		app_control_add_extra_data(reply, "city_name", _(cs->city));
		app_control_add_extra_data(reply, "country", cs->country);
		app_control_add_extra_data(reply, "country_name", _(cs->country));

		if (ad->caller != WCL_CALLER_IS_APP_IT_SELF) {
			const char *timezone = cs->timezone;
			if (timezone == strstr(timezone, "GMT")) {
				timezone += 3;
			}

			app_control_add_extra_data(reply, "timezone", timezone);
			app_control_add_extra_data(reply, "tzpath", cs->tz_path);
			CLK_INFO("[Result] city: %s, city_name: %s, country: %s, timezone: %s, tzpath: %s\n", cs->city, _(cs->city), cs->country, timezone, cs->tz_path);
		}

		app_control_reply_to_launch_request(reply, ad->app_caller, APP_CONTROL_RESULT_SUCCEEDED);
		app_control_destroy(reply);

		FREEIF(ad->return_data);
#ifdef FEATURE_SORT_ORDER
		EVAS_OBJECT_DELIF(ad->more_popup);
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
* send
* This function is  used to create bg
* @param           name   pointer to a win main evas object
* @return          return a Evas_Object, is a bg evas object or NULL if failed
* @exception       if error happen, return NULL
*/
Evas_Object *__ug_create_bg(Evas_Object * win)
{
	Evas_Object *bg = elm_bg_add(win);
	retvm_if(!bg, NULL, "bg null");
	evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(bg);
	return bg;
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

static bool on_create(void *priv)
{
	CLK_FUN_BEG();
	Evas_Object *win = NULL;
	struct appdata *ad = NULL;

	retv_if(priv == NULL, false);

	ad = priv;

	// get ug window
	win = elm_win_util_standard_add(PACKAGE, PACKAGE);
	retv_if(win == NULL, false);
	// allocate data
	ad = (struct appdata *)calloc(1, sizeof(struct appdata));
	retv_if(ad == NULL, false);
	/*disable rotate */
	ad->win = win;
    elm_win_conformant_set(win, EINA_TRUE);
    elm_win_autodel_set(win, EINA_TRUE);
    evas_object_show(win);
	ad->conform = elm_conformant_add(win);
    evas_object_size_hint_weight_set(ad->conform, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_win_resize_object_add(win, ad->conform);
    evas_object_show(ad->conform);

	/* language setting */
	bindtextdomain(PACKAGE, worldclock_get_locale_path());
	textdomain(PACKAGE);

	/* main layout */
	ad->ly_main = __ug_create_main_layout(ad->conform);
	GOTO_ERROR_IF(ad->ly_main == NULL);
	ad->bg = __ug_create_bg(ad->ly_main);
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
	elm_object_content_set(ad->conform, ad->ly_main);
	CLK_FUN_END();
	return true;

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

static void on_pause(void *priv)
{
	CLK_FUN_BEG();
}

static void on_resume(void *priv)
{
	CLK_FUN_BEG();
}

static void on_destroy(void *priv)
{
	CLK_FUN_BEG();

	if (priv) {
		struct appdata *ad = priv;
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
		if(ad->app_caller)
		    app_control_destroy(ad->app_caller);

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
		free(ad);
	}
	worldclock_path_util_free();
	CLK_FUN_END();
}

static void on_app_control(app_control_h app_control, void *priv)
{
    struct appdata *ad = NULL;
    char *caller_name = NULL;
    char *city_index = NULL;
    char *text_id = NULL;
    ret_if(!priv);
    ad = priv;

    if (app_control) {
        app_control_clone(&ad->app_caller, app_control);
        app_control_get_extra_data(app_control, "caller", &caller_name);
        app_control_get_extra_data(app_control, "city_index", &city_index);

        app_control_get_extra_data(app_control, "translation_request", &text_id);
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

    if (text_id) {
        CLK_INFO("text_id = %d", text_id);
        app_control_h reply = NULL;
        app_control_create(&reply);
        app_control_add_extra_data(reply, "city_name", _(text_id));
        app_control_reply_to_launch_request(reply, app_control, APP_CONTROL_RESULT_SUCCEEDED);
        app_control_destroy(reply);

        FREEIF(text_id);
        ui_app_exit(); //check if it necessary
    }
}

static void on_lang_changed(app_event_info_h event_info, void *priv)
{
    ret_if(!priv);

    struct appdata *ad;
    ad = priv;
    __ug_lang_update(ad);

    uninit_alphabetic_index();
    char *lang = vconf_get_str(VCONFKEY_LANGSET);
    init_alphabetic_index(lang);
    FREEIF(lang);

    worldclock_ugview_update(ad);
}

static void on_orient_changed(app_event_info_h event_info, void *priv)
{
    ret_if(!priv);

    struct appdata *ad;
    Elm_Index_Item *it = NULL;
    Ecore_IMF_Context *imf_context = NULL;
    app_device_orientation_e orient = APP_DEVICE_ORIENTATION_0;
    ad = priv;
    app_event_get_device_orientation(event_info, &orient);

    switch (orient)
    {
        case APP_DEVICE_ORIENTATION_0:
        case APP_DEVICE_ORIENTATION_180:
            _show_title(ad, NULL, NULL);
            it = elm_index_selected_item_get(ad->add_index, 0);
            if (it != NULL) {
                elm_index_item_selected_set(it, EINA_FALSE);
            }
            break;
        case APP_DEVICE_ORIENTATION_90:
        case APP_DEVICE_ORIENTATION_270:
            imf_context = elm_entry_imf_context_get(ad->searchbar_entry);
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
            break;
    }
}

int main(int argc, char *argv[])
{
    struct appdata *ad;

    ad = calloc(1, sizeof(struct appdata));
    retv_if(!ad, -1);

    ui_app_lifecycle_callback_s cbs = {};

    cbs.app_control = on_app_control;
    cbs.create = on_create;
    cbs.pause = on_pause;
    cbs.resume = on_resume;
    cbs.terminate = on_destroy;

    app_event_handler_h handlers[3] = {};
    ui_app_add_event_handler(&handlers[0], APP_EVENT_LANGUAGE_CHANGED, on_lang_changed, ad);
    ui_app_add_event_handler(&handlers[1], APP_EVENT_REGION_FORMAT_CHANGED, on_lang_changed, ad);
    ui_app_add_event_handler(&handlers[2], APP_EVENT_DEVICE_ORIENTATION_CHANGED, on_orient_changed, &ad);

    return ui_app_main(argc, argv, &cbs, ad);
}

