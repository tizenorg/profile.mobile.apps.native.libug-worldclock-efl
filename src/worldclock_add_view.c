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

#define _GNU_SOURCE
#include <string.h>

#include <stdio.h>
#include <appcore-efl.h>
#include <Elementary.h>
#include <vconf.h>
#include <unicode/utf8.h>
#include <unicode/ustring.h>
#include <efl_extension.h>
#include <notification.h>
#include <tapi_common.h>
#include <ui-gadget-module.h>
#include <system_settings.h>
#include <tapi_common.h>

#include "worldclock.h"
#include "worldclock_data.h"
#include "worldclock_dlog.h"
#include "worldclock_util.h"
#include "worldclock_add_view.h"
#include "clock_fwk_icu_label.h"

#define MAX_LEN_CITY_NAME		16
#define MAX_LEN_COUNTRY_NAME	24
#define MAX_LEN_GMT				10
#define MAX_LEN_INPUT			20
#define FIRST_UPDATE_ITEM_COUNT	14
#define MIN_COUNT_CITY			10	// the city number less than this number will hide the index
#define MAX_TITLE_NAME			128
#define NORMAL_TIMEOUT_VALUE	0.01
#define EXTENDED_TIMEOUT_VALUE	0.2

#define sncat(to, size, from) \
    ({strncat((to), (from), ((size)-strlen(to)-1));})

static struct appdata *g_ad = NULL;
static Wcl_Return_Cb g_return_cb = NULL;
static char g_index_buf[BUF_MIN_SIZE] = { '\0' };

static Eina_Bool isSearchEntryMaxReached = EINA_FALSE;

typedef enum {
	CALL_NONE = 0,
	CALL_FROM_MAIN_VIEW,
	CALL_FROM_LIVE_BOX,
	CALL_FROM_UI_GADGET,
	CALL_FROM_UNKNOW,
} ADDVIEW_CALL_FLAG;

static ADDVIEW_CALL_FLAG g_call_flag = CALL_NONE;

static void _ugview_clear_data(void *data);
static Eina_Bool _ugview_exit_cb(void *data);
static Eina_Bool _ugview_exit_prev_btn_cb(void *data);
static void _ug_reply();

static Evas_Object *_ugview_add_layout(Evas_Object *parent);

static Evas_Object *_ugview_add_genlist(Evas_Object *parent, void *data);
static Eina_Bool _ugview_genlist_check_selected_status(const void *data);

static Evas_Object *_ugview_add_index(Evas_Object *parent, void *data);
#ifdef FEATURE_SORT_ORDER
static Eina_Bool _ugview_update_index(void *data, Wcl_Addlist_Sort_Type sort, char *buf, void *item);
#else
static Eina_Bool _ugview_update_index(void *data, char *buf, void *item);
#endif

static void _check_city_count_index(void *data, UChar *u_search_text);

static char *_gl_text_get(void *data, Evas_Object *obj, const char *part);
static void _gl_load_finished_cb(void *data, Evas_Object *obj, void *event_info);
static void _gl_selected_cb(void *data, Evas_Object *obj, void *event_info);
static void _gl_del_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _gl_item_selected_cb(void *data, Evas_Object *obj, void *event_info);

static void _index_delay_changed_cb(void *data, Evas_Object *obj, void *event_info);
static void _index_changed_cb(void *data, Evas_Object *obj, void *event_info);
static void _index_selected_cb(void *data, Evas_Object *obj, void *event_info);
static void _index_del_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);

static void _searchbar_layout_del_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);

static void _searchbar_entry_changed_cb(void *data, Evas_Object *obj, void *event_info);
static void _searchbar_entry_focused_cb(void *data, Evas_Object *obj, void *event_info);
static void _searchbar_entry_max_length_reached_cb(void *data, Evas_Object *obj, void *event_info);
static void _searchbar_entry_preedit_changed_cb(void *data, Evas_Object *obj, void *event_info);
static void _searchbar_entry_lang_changed(void *data, Evas_Object *obj, void *event_info);
static void _searchbar_entry_activated_cb(void *data, Evas_Object *obj, void *event_info);
static void _searchbar_entry_del_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _searchbar_clear_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);

static Eina_Bool _entry_changed_cb(void *data);

static void _location_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info);
static void _location_btn_pressed_cb(void *data, Evas_Object *obj, void *event_info);
static void _location_btn_unpressed_cb(void *data, Evas_Object *obj, void *event_info);

static void _back_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info);
static Eina_Bool _ugview_back_button_cb(void *data, Elm_Object_Item *It);
static char *_ugview_add_tag_for_search(char *target, char *searcher);
static const UChar *_ugview_search_in_ustring(const UChar *string, UChar *substring);
static Eina_Bool _ugview_search_matched_mcc(char *string, char *city_mcc);
static Eina_Bool _set_entry_focus_idler_cb(void *data);

#ifdef FEATURE_SORT_ORDER
static Evas_Object *_sort_gl_content_get(void *data, Evas_Object *obj, const char *part);
static char *_sort_gl_text_get(void *data, Evas_Object *obj, const char *part);
static void _sort_gl_realized_cb(void *data, Evas_Object *obj, void *event_info);
static void _sort_gl_lang_changed_cb(void *data, Evas_Object *obj, void *event_info);
static void _sort_gl_clicked_cb(void *data, Evas_Object *obj, void *event_info);

static void _sort_popup_back_cb(void *data, Evas_Object *obj, void *event_info);

static void _more_ctxpopup_hide_cb(void *data, Evas_Object *obj, void *event_info);
static void _move_more_ctxpopup(void *data);

static void _navi_bar_move_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);
static void _navi_bar_delete_cb(void *data, Evas *e, Evas_Object *obj, void *event_info);

static void _more_button_clicked_cb(void *data, Evas_Object *obj, void *event_info);
static Evas_Object *_ugview_create_more_btn(Evas_Object *parent, Evas_Smart_Cb func, void *data);

static Elm_Genlist_Item_Class itc_sort = {
	.item_style = "type1",
	.func.text_get = _sort_gl_text_get,
	.func.content_get = _sort_gl_content_get,
	.func.state_get = NULL,
	.func.del = NULL,
};
#endif

static Elm_Genlist_Item_Class g_ts = {
	.item_style = "type1",
	.func.text_get = _gl_text_get,
	.func.content_get = NULL,
	.func.state_get = NULL,
	.func.del = NULL,
};


static void _ugview_clear_data(void *data)
{
	CLK_FUN_BEG();
	ret_if(!data);

	struct appdata *ad = (struct appdata *)data;

	//timer
	ECORE_TIMER_DELIF(ad->add_view_quit_timer);
	ECORE_TIMER_DELIF(ad->add_view_update_timer);
	ECORE_TIMER_DELIF(ad->search_timer);

	// remove default list used to store all appendable cities
	if (ad->default_list) {
		worldclock_util_glist_remove_all(ad->default_list, EINA_TRUE);
		ad->default_list = NULL;
	}

	int ret = system_settings_unset_changed_cb(SYSTEM_SETTINGS_KEY_FONT_SIZE);
	if (ret != SYSTEM_SETTINGS_ERROR_NONE) {
		CLK_INFO_RED("unset font callback error\n");
	}

	if (ad->add_genlist != NULL) {
		evas_object_smart_callback_del(ad->add_genlist, "loaded",
				_gl_load_finished_cb);
		evas_object_smart_callback_del(ad->add_genlist, "selected",
				_gl_selected_cb);
	}
	if (ad->searchbar_entry != NULL) {
		evas_object_smart_callback_del(ad->searchbar_entry, "changed",
				_searchbar_entry_changed_cb);
		evas_object_smart_callback_del(ad->searchbar_entry, "focused",
				_searchbar_entry_focused_cb);
		evas_object_smart_callback_del(ad->searchbar_entry, "maxlength,reached",
				_searchbar_entry_max_length_reached_cb);
		evas_object_smart_callback_del(ad->searchbar_entry, "preedit,changed",
				_searchbar_entry_preedit_changed_cb);
		evas_object_smart_callback_del(ad->searchbar_entry, "language,changed",
				_searchbar_entry_lang_changed);
		evas_object_smart_callback_del(ad->searchbar_entry, "activated",
				_searchbar_entry_activated_cb);
		EVAS_OBJECT_DELIF(ad->searchbar_entry);
	}
	if (ad->add_index) {
		evas_object_smart_callback_del(ad->add_index, "delay,changed",
				_index_delay_changed_cb);
		evas_object_smart_callback_del(ad->add_index, "changed",
				_index_changed_cb);
		evas_object_smart_callback_del(ad->add_index, "selected",
				_index_selected_cb);
	}
	g_call_flag = CALL_NONE;
	CLK_FUN_END();
}

/**
 * Quit function called when exit event caused by timer
 *
 * @param[in]  data    data used for this function
 *
 * @return     EINA_FALSE
 */
static Eina_Bool _ugview_exit_cb(void *data)
{
	g_ad->add_view_quit_timer = NULL;

	if (g_return_cb) {
		g_return_cb(data, EINA_FALSE);
		g_return_cb = NULL;
	}
	elm_naviframe_item_pop(g_ad->navi_bar);

	return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool _ugview_exit_prev_btn_cb(void *data)
{
	g_ad->add_view_quit_timer = NULL;

	_ug_reply();

	return ECORE_CALLBACK_CANCEL;
}

static char *_ugview_add_tag_for_search(char *target, char *searcher)
{
	const char *p = NULL;
	const char *pre = NULL;
	char buf[BUF_SIZE] = { 0, };
	char str_tag_added[BUF_SIZE] = { 0, };

	pre = target;
	const char *temp = searcher;
	p = (const char *)strcasestr(pre, temp);
	if (p != NULL) {
		/* Get T105 color rgba from theme */
		char color_tag[BUF_SIZE] = { 0, };

		snprintf(color_tag, sizeof(color_tag), "<color=#FF0000FF>");

		/* append characters before matched string */
		if (p != pre) {
			buf[0] = '\0';
			strncat(buf, pre, p - pre);
			sncat(str_tag_added, BUF_SIZE, buf);
		}
		/* highlight str */
		sncat(str_tag_added, BUF_SIZE, color_tag);
		buf[0] = '\0';
		strncat(buf, p, strlen(searcher));
		sncat(str_tag_added, BUF_SIZE, buf);
		sncat(str_tag_added, BUF_SIZE, "</color>");
		/* set pointer after matched string */
		pre = p + strlen(searcher);
	}
	sncat(str_tag_added, BUF_SIZE, pre);
	return strdup(str_tag_added);
}

static char *_gl_text_get(void *data, Evas_Object *obj, const char *part)
{
	CLK_FUN_DEBUG_BEG();
	retv_if(NULL == data || NULL == g_ad, NULL);

	Wcl_CitySet *gmt = (Wcl_CitySet *) data;
	char buf[BUF_SIZE] = "";
	char *tag_city = NULL;
	char *tag_country = NULL;
	retv_if(NULL == part, NULL);

	if (!strcmp(part, "elm.text")) {
		tag_city = _ugview_add_tag_for_search(_(gmt->city), g_ad->search_text);
		tag_country =
				_ugview_add_tag_for_search(_(gmt->country), g_ad->search_text);
		snprintf(buf, BUF_SIZE, "%s, %s", tag_city, tag_country);
		FREEIF(tag_city);
		FREEIF(tag_country);
	} else if (!strcmp(part, "elm.text.sub")) {
		snprintf(buf, BUF_SIZE, "%s", gmt->timezone);
	}
	CLK_FUN_DEBUG_END();
	return strdup(buf);
}

/**
 * Check whether this city has been selected
 *
 * @param[in]  data   appointed citydata used for this function
 *
 * @return     EINA_TRUE   this city was already selected
 *             EINA_FALSE  this city was not selected
 */
static Eina_Bool _ugview_genlist_check_selected_status(const void *data)
{
	CLK_FUN_BEG();
	retv_if((!data || !g_ad), EINA_FALSE);

	Eina_Bool ret = EINA_FALSE;

	Wcl_CitySet *cs = (Wcl_CitySet *) data;
	// synchronize city status from db
	ret = worldclock_ug_data_get_city_status_from_db(cs);

	if (EINA_TRUE == ret) {
		if (1 != cs->selected) {
			ret = EINA_FALSE;
		}
	}

	CLK_FUN_END();
	return ret;
}

static void _gl_item_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	CLK_FUN_BEG();

	Elm_Object_Item *gli = NULL;
	// get selected genlist item
	gli = elm_genlist_selected_item_get(obj);
	elm_genlist_item_selected_set(gli, 0);
	CLK_FUN_END();
}

static void _gl_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	CLK_FUN_BEG();
	ret_if(!g_ad);
	const Wcl_CitySet *cs = NULL;
	Elm_Object_Item *gli = (Elm_Object_Item *) (event_info);
	cs = elm_object_item_data_get(gli);
	ret_if(!cs);

	/* replace or append operation to mainview */
	if (CALL_FROM_MAIN_VIEW == g_call_flag) {
		if (_ugview_genlist_check_selected_status(cs)) {
			CLK_INFO("this city has been selected, show notify\n");
			notification_status_message_post(_("IDS_COM_POP_ALREDY_EXISTS"));
			return;
		} else {
			// create new structure to store new city
			Wcl_CitySet *t_cs = calloc(1, sizeof(Wcl_CitySet));
			ret_if(!t_cs);
			memcpy(t_cs, cs, sizeof(Wcl_CitySet));
			t_cs->selected = 1;
			g_ad->return_data = t_cs;
			CLK_ERR("Selected city = %s", _(t_cs->city));
		}
	} else if (CALL_FROM_LIVE_BOX == g_call_flag) {
		// create new structure to store new city
		Wcl_CitySet *t_cs = calloc(1, sizeof(Wcl_CitySet));
		ret_if(!t_cs);
		memcpy(t_cs, cs, sizeof(Wcl_CitySet));
		t_cs->selected = 0;
		g_ad->return_data = t_cs;
		CLK_ERR("Selected city = %s", _(t_cs->city));
	} else if (CALL_FROM_UI_GADGET == g_call_flag) {
		// create new structure to store new city
		Wcl_CitySet *t_cs = calloc(1, sizeof(Wcl_CitySet));
		if (t_cs) {
			// copy selected data
			memcpy(t_cs, cs, sizeof(Wcl_CitySet));
			// get current dst value, and set it into result data
			t_cs->dst = worldclock_dst_get(cs);
			// save return data for UG
			g_ad->return_data = t_cs;
		} else {
			CLK_ERR("CALLOC ERROR!!!");
			// set NULL for UG
			g_ad->return_data = NULL;
		}
	}
	// Select done, Exit.
	ECORE_TIMER_DELIF(g_ad->add_view_quit_timer);

	Ecore_IMF_Context *imf_context = elm_entry_imf_context_get(g_ad->searchbar_entry);
	if (imf_context
			&& ECORE_IMF_INPUT_PANEL_STATE_HIDE !=
			ecore_imf_context_input_panel_state_get(imf_context)) {
		ecore_imf_context_input_panel_hide(imf_context);
		g_ad->add_view_quit_timer = ecore_timer_add(EXTENDED_TIMEOUT_VALUE, _ugview_exit_cb, NULL);
	} else {
		g_ad->add_view_quit_timer = ecore_timer_add(NORMAL_TIMEOUT_VALUE, _ugview_exit_cb, NULL);
	}
	CLK_FUN_END();
}

/**
 * Search substring in string by UChar type
 *
 * @param[in]  string		The string to search
 * @param[in]  substring	The substring to find
 *
 * @return    A pointer to the first occurrence of substring in string
 */
const UChar *_ugview_search_in_ustring(const UChar *string, UChar *substring)
{
	//CLK_FUN_BEG();
	int i = 0;
	const UChar *pstr = string;
	int substr_len = u_strlen(substring);
	int str_len = u_strlen(string);

	if (0 == substr_len) {
		// NULL string
		return pstr;
	} else if (substr_len > str_len) {
		// substring to long
		return NULL;
	}

	for (i = 0; i < str_len - substr_len + 1; i++) {
		if (!u_strncasecmp(substring, &pstr[i], substr_len, 0)) {
			// match
			return &pstr[i];

		}
	}
	//CLK_FUN_END();
	// no match
	return NULL;
}

static Eina_Bool _ugview_search_matched_mcc(char *string, char *city_mcc)
{
	Eina_Bool ret = EINA_TRUE;
	if (!strlen(string)) {
		return ret;
	}

	if (strlen(city_mcc) == 3) {
		if (IS_STR_NOT_EQUAL(string, city_mcc)) {
			ret = EINA_FALSE;
		}
		return ret;
	}

	ret = EINA_FALSE;
	char *tmp_mcc = strdup(city_mcc);
	char *save_ptr = NULL;
	char *ptr = NULL;
	ptr = strtok_r(tmp_mcc, " ", &save_ptr);
	if (ptr) {
		if (IS_STR_EQUAL(ptr, string)) {
			ret = EINA_TRUE;
			FREEIF(tmp_mcc);
			return ret;
		}
	}
	while ((ptr = strtok_r(NULL, " ", &save_ptr))) {
		if (IS_STR_EQUAL(ptr, string)) {
			ret = EINA_TRUE;
			break;
		}
	}
	FREEIF(tmp_mcc);
	return ret;
}

static Eina_Bool _set_entry_focus_idler_cb(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	elm_object_focus_set(ad->searchbar_entry, EINA_TRUE);
	return ECORE_CALLBACK_CANCEL;
}

static void _gl_load_finished_cb(void *data, Evas_Object *obj,
		void *event_info)
{
	CLK_INFO("genlist load finished");
	if (g_ad->city_index) {
		if (g_ad->item_selected != NULL) {
			elm_genlist_item_show(g_ad->item_selected, ELM_GENLIST_ITEM_SCROLLTO_TOP);
		}
		g_ad->city_index = FAILED;
	}
}

/**
 * Update genlist which displaying in add view
 *
 * @param[in]  data   data used for this function
 *
 * @return     EINA_FALSE
 */
Eina_Bool _ugview_genlist_update(void *data)
{
	CLK_FUN_BEG();
	retv_if(!data, ECORE_CALLBACK_CANCEL);
	retv_if(!g_ad, ECORE_CALLBACK_CANCEL);
	struct appdata *ad = (struct appdata *)data;
	Elm_Object_Item *gli = NULL;
	Eina_List *el = NULL;
	Wcl_CitySet *cs = NULL;
	int count = 0;
	ad->flag_index = EINA_TRUE;
	if (g_ad->is_Empty) {
		if (ad->nocontent
				&& ad->nocontent == elm_object_part_content_get(ad->add_fs_layout,
						"elm.swallow.content")) {
			EVAS_OBJECT_DELIF(ad->nocontent);
			ad->add_genlist = _ugview_add_genlist(ad->add_fs_layout, data);
			elm_object_part_content_set(ad->add_fs_layout,
					"elm.swallow.content", ad->add_genlist);
		}
	}
	g_ad->is_Empty = EINA_FALSE;
	ad->flag_first = EINA_TRUE;
	ad->flag_special_character = EINA_TRUE;
	UChar u_search_text[CITY_BUF_SIZE] = { 0 };

#ifdef FEATURE_SORT_ORDER
	EVAS_OBJECT_DELIF(ad->sort_popup_genlist);
	EVAS_OBJECT_DELIF(ad->sort_popup);
#endif

	elm_layout_sizing_eval(ad->add_fs_layout);

	// init data list
	memset(g_index_buf, 0X0, BUF_MIN_SIZE * sizeof(char));

	// set data list
	if (!ad->default_list) {
		// get all appendable cities
		ad->default_list =
				worldclock_ug_data_get_default_city_list(ad->selectionFlag);
	}
	// reset timer
	g_ad->add_view_update_timer = NULL;

	retv_if(NULL == ad->search_text, EINA_FALSE);
	if (!strcmp(ad->search_text, "")) {
		// if searchbar is been clicked, be filled in ""
		CLK_INFO("ad->search_text=%s\n", ad->search_text);
	} else {
		CLK_INFO("ad->search_text=%s\n", ad->search_text);
		u_uastrncpy(u_search_text, ad->search_text, CITY_BUF_SIZE);
	}

#ifdef FEATURE_SORT_ORDER
	if (WCL_SORT_BY_NAME == ad->sort) {
		// sort by name
		snprintf(g_index_buf, BUF_MIN_SIZE, "%c", 'Z');
		// sort default list
		ad->default_list =
				eina_list_sort(ad->default_list,
						eina_list_count(ad->default_list), worldclock_city_compare_cb);
	} else if (WCL_SORT_BY_TIMEZONE == ad->sort)
#endif
	{
		// sort by time
		snprintf(g_index_buf, BUF_MIN_SIZE, "%s", "-18");
		// sort default list
		ad->default_list =
				eina_list_sort(ad->default_list,
						eina_list_count(ad->default_list), worldclock_time_compare_cb);
	}

	if (ad->add_genlist) {
		elm_genlist_clear(ad->add_genlist);
	}
	if (ad->add_index) {
		elm_index_item_clear(ad->add_index);
	}
	//if city number less than 10 ,hide the index layout
	_check_city_count_index(ad, u_search_text);

	el = ad->default_list;
	while (el) {
		// get data
		cs = (Wcl_CitySet *) el->data;
		if (cs) {
			UChar u_city_name[CITY_BUF_SIZE] = { 0 };
			UChar u_country_name[COUNTRY_BUF_SIZE] = { 0 };
			Eina_Bool match = EINA_FALSE;
			u_uastrncpy(u_city_name, _(cs->city), CITY_BUF_SIZE);
			u_uastrncpy(u_country_name, _(cs->country), COUNTRY_BUF_SIZE);
			if (strlen(ad->current_mcc)) {
				if (_ugview_search_matched_mcc(ad->current_mcc, cs->mcc)) {
					CLK_INFO("Success to find current city");
					match = EINA_TRUE;
				}
			} else if (strlen(ad->current_mcc) < 1 && (_ugview_search_in_ustring(u_city_name, u_search_text)
					|| _ugview_search_in_ustring(u_country_name,
							u_search_text))) {
				match = EINA_TRUE;
			}
			if (match) {
				// append data to genlist
				gli = elm_genlist_item_append(ad->add_genlist, &g_ts, cs,	/* item data */
						NULL,	/* parent */
						ELM_GENLIST_ITEM_NONE, _gl_item_selected_cb,	/* func */
						cs	/* func data */
							     );

				//if enter ug by selected item,city_index>0,if not city_index ==0 and show the first item
				if ((ad->city_index == cs->index) && (ad->city_index > 0)) {
					ad->item_selected = gli;
				} else if ((el == ad->default_list)
						&& (ad->city_index == 0)) {
					elm_genlist_item_show(gli,
							ELM_GENLIST_ITEM_SCROLLTO_TOP);
				}
				if (ad->flag_index) {
					// update index with genlist item
#ifdef FEATURE_SORT_ORDER
					_ugview_update_index(ad, ad->sort, g_index_buf, gli);
#else
					_ugview_update_index(ad, g_index_buf, gli);
#endif
				}
				ad->flag_first = EINA_FALSE;
				count++;
			} else {
				// get next data
				el = el->next;
				continue;
			}
		} else {
			break;
		}
		// get next data
		el = el->next;
	}
	ad->searched_num = count;

	if (count > 0) {

		elm_index_autohide_disabled_set(g_ad->add_index, EINA_TRUE);
		elm_index_level_go(g_ad->add_index, 0);
		evas_object_show(g_ad->add_index);
	} else if (count == 0) {
		g_ad->is_Empty = EINA_TRUE;
		ad->searched_text_length = strlen(ad->search_text);
		if (ad->add_genlist
				&& ad->add_genlist ==
				elm_object_part_content_get(ad->add_fs_layout,
						"elm.swallow.content")) {
			EVAS_OBJECT_DELIF(ad->add_genlist);
			ad->nocontent = elm_layout_add(ad->add_fs_layout);
			elm_layout_theme_set(ad->nocontent, "layout", "nocontents",
					"search");
			elm_object_domain_translatable_part_text_set(ad->nocontent,
					"elm.text", PACKAGE,
					"IDS_COM_BODY_NO_SEARCH_RESULTS");
			elm_object_part_content_set(ad->add_fs_layout,
					"elm.swallow.content", ad->nocontent);
		}
	}

	CLK_FUN_END();
	return ECORE_CALLBACK_CANCEL;
}

/**
 * Add new genlist which used for displaying all cities which could be append
 * into worldclock.
 *
 * @param[in]  parent   The parent object of the new genlist object
 * @param[in]  data     Data used in this function
 *
 * @return     NULL if create genlist failed.
 *             Pointer to the new genlist object if create genlist successfully.
 */
static Evas_Object *_ugview_add_genlist(Evas_Object *parent, void *data)
{
	CLK_FUN_BEG();
	retv_if(!data, NULL);
	Evas_Object *genlist = NULL;
	// add genlist
	genlist = elm_genlist_add(parent);
	elm_scroller_policy_set(genlist, ELM_SCROLLER_POLICY_OFF,
			ELM_SCROLLER_POLICY_OFF);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	elm_genlist_homogeneous_set(genlist, EINA_TRUE);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
	// swallow
	elm_object_part_content_set(parent, "elm.swallow.content", genlist);
	// show
	evas_object_show(genlist);
	// register callback function for select event on genlist
	evas_object_smart_callback_add(genlist, "selected",
			_gl_selected_cb, data);

	CLK_FUN_END();
	return genlist;
}

static void _index_delay_changed_cb(void *data, Evas_Object *obj,
		void *event_info)
{
	elm_genlist_item_show(elm_object_item_data_get(event_info),
			ELM_GENLIST_ITEM_SCROLLTO_TOP);
}

static void _index_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_show(elm_object_item_data_get(event_info),
			ELM_GENLIST_ITEM_SCROLLTO_TOP);
}

static void _index_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_item_show(elm_object_item_data_get(event_info),
			ELM_GENLIST_ITEM_SCROLLTO_TOP);
	elm_index_item_selected_set(event_info, EINA_FALSE);
}

#ifdef FEATURE_SORT_ORDER
static Eina_Bool _ugview_update_index(void *data, Wcl_Addlist_Sort_Type sort, char *buf, void *item)
#else
static Eina_Bool _ugview_update_index(void *data, char *buf, void *item)
#endif
{
	//CLK_FUN_BEG();
	retv_if(!data, EINA_FALSE);
	struct appdata *ad = (struct appdata *)data;
	Evas_Object *index = ad->add_index;
	char buf_name[BUF_MIN_SIZE] = { 0, };
	char buf_zone[BUF_MIN_SIZE] = { 0, };
	char *pbegin = NULL;
	Elm_Object_Item *gli = (Elm_Object_Item *) item;
	Wcl_CitySet *cs = elm_object_item_data_get(gli);
	retv_if(!index, EINA_FALSE);

	memset(buf_name, 0x0, BUF_MIN_SIZE * sizeof(char));
#ifdef FEATURE_SORT_ORDER
	if (WCL_SORT_BY_NAME == sort) {
		static char g_alphabetic_buf[BUF_MIN_SIZE] = { '\0' };
		// append index to elm_index
		char *bufalphabetic =
				get_alphabetic_index_name(get_alphabetic_index(_(cs->city)));
		if (bufalphabetic == NULL) {
			char *temp = vconf_get_str(VCONFKEY_LANGSET);
			if ( temp != NULL && (!strcmp(temp, "ja_JP.UTF-8")) && ad->flag_special_character) {
				Elm_Object_Item *it =
						elm_index_item_append(index, "漢", NULL, NULL);
				elm_object_item_data_set(it, gli);
				ad->flag_special_character = EINA_FALSE;
			}
			FREEIF(temp);
			return EINA_FALSE;
		}
		if (g_alphabetic_buf[0] == '\0') {
			snprintf(g_alphabetic_buf, sizeof(g_alphabetic_buf), "%s",
					bufalphabetic);
			Elm_Object_Item *it = elm_index_item_append(index, bufalphabetic,
					NULL, NULL);
			elm_object_item_data_set(it, gli);
		} else if (g_alphabetic_buf[0] != '\0') {
			if (IS_STR_NOT_EQUAL(g_alphabetic_buf, bufalphabetic)
					|| ad->flag_first == EINA_TRUE) {
				snprintf(g_alphabetic_buf, sizeof(g_alphabetic_buf), "%s",
						bufalphabetic);
				Elm_Object_Item *it =
						elm_index_item_append(index, bufalphabetic,
								NULL, NULL);
				elm_object_item_data_set(it, gli);
			}
		}
	} else if (WCL_SORT_BY_TIMEZONE == sort)
#endif
	{
		// get timezone
		if (0 == strcmp(cs->timezone, "GMT+0")
				|| 0 == strcmp(cs->timezone, "GMT+0:30")) {
			pbegin = cs->timezone + 4;
		} else {
			pbegin = cs->timezone + 3;
		}

		strncpy(buf_zone, pbegin, BUF_MIN_SIZE);
		buf_zone[strlen(cs->timezone)] = '\0';

		if (strcmp(buf, buf_zone)) {
			// append timezone into index list if it is not exist
			snprintf(buf, BUF_MIN_SIZE, "%s", buf_zone);
			Elm_Object_Item *it =
					elm_index_item_append(index, buf, NULL, NULL);
			elm_object_item_data_set(it, gli);
		}
	}
	//CLK_FUN_END();
	return EINA_TRUE;
}

/**
 * Create new elm_index used in add view
 *
 * @param[in]  parent     Evas_Object which is the parent of new elm_index
 * @param[in]  date       data which used int this function
 *
 * @return     NULL if create failed
 *             Pointer to new elm_index if create successflly
 */
static Evas_Object *_ugview_add_index(Evas_Object *parent, void *data)
{
	CLK_FUN_BEG();
	retv_if((!parent || !data), NULL);

	Evas_Object *index = NULL;
	// add index
	index = elm_index_add(parent);
	evas_object_size_hint_weight_set(index, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	evas_object_smart_callback_add(index, "delay,changed",
			_index_delay_changed_cb, NULL);
	evas_object_smart_callback_add(index, "changed", _index_changed_cb,
			NULL);
	evas_object_smart_callback_add(index, "selected", _index_selected_cb,
			NULL);
	CLK_FUN_END();
	return index;
}

/**
 * check the city number to decide if or not keep index layout
 *
 * @param[in]  data     struct appdata *
 * @param[in]   u_search_text Uchar*
 *
 */
static void _check_city_count_index(void *data, UChar *u_search_text)
{
	struct appdata *ad = (struct appdata *)data;
	ret_if(NULL == ad);
	Eina_List *el = NULL;
	Wcl_CitySet *cs = NULL;
	int count = 0;

	/*get the number of city */
	el = ad->default_list;
	while (el) {
		/* get data */
		cs = (Wcl_CitySet *) el->data;
		if (cs) {
			UChar u_city_name[CITY_BUF_SIZE] = { 0 };
			UChar u_country_name[COUNTRY_BUF_SIZE] = { 0 };
			Eina_Bool match = EINA_FALSE;
			u_uastrncpy(u_city_name, _(cs->city), CITY_BUF_SIZE);
			u_uastrncpy(u_country_name, _(cs->country), COUNTRY_BUF_SIZE);
			if (strlen(ad->current_mcc)) {
				if (_ugview_search_matched_mcc(ad->current_mcc, cs->mcc)) {
					match = EINA_TRUE;
				}
			} else if (_ugview_search_in_ustring(u_city_name, u_search_text)
					|| _ugview_search_in_ustring(u_country_name,
							u_search_text)) {
				match = EINA_TRUE;
			}
			if (match) {
				count++;
				if (count > MIN_COUNT_CITY) {
					break;
				}
			} else {
				/* get next data */
				el = el->next;
				continue;
			}
		} else {
			break;
		}
		/* get next data */
		el = el->next;
	}

	if (count > MIN_COUNT_CITY) {
		ad->flag_index = EINA_TRUE;
		elm_object_signal_emit(ad->add_fs_layout, "elm,state,fastscroll,show",
				"");
		if (ad->add_index == NULL) {
			ad->add_index = _ugview_add_index(ad->add_layout, ad);
			elm_object_part_content_set(ad->add_fs_layout,
					"elm.swallow.fastscroll", ad->add_index);
		}
	} else {
		ad->flag_index = EINA_FALSE;
		ad->add_index =
				elm_object_part_content_unset(ad->add_fs_layout,
						"elm.swallow.fastscroll");
		EVAS_OBJECT_DELIF(ad->add_index);
		elm_object_signal_emit(ad->add_fs_layout, "elm,state,fastscroll,hide",
				"");
	}
}

static void _searchbar_entry_activated_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_object_focus_set(obj, EINA_FALSE);
}

static void _searchbar_entry_focused_cb(void *data, Evas_Object *obj, void *event_info)
{
	CLK_FUN_BEG();
	ret_if(!data);

	struct appdata *ad = (struct appdata *)data;

	if (elm_object_focus_get(ad->searchbar_entry)) {
		elm_object_signal_emit(ad->searchbar_rect, "elm,state,focused", "");
	} else{
		elm_object_signal_emit(ad->searchbar_rect, "elm,state,unfocused", "");
	}

	CLK_FUN_END();
}

static Eina_Bool _entry_changed_cb(void *data)
{
	struct appdata *ad = (struct appdata *)data;;
	retv_if(NULL == ad, ECORE_CALLBACK_CANCEL);

	//reset current city
	if (ad->search_text[0] != '\0') {
		ad->current_mcc[0] = '\0';
	}

	ad->search_timer = NULL;
	ECORE_TIMER_DELIF(ad->add_view_update_timer);
	ad->add_view_update_timer = ecore_timer_add(NORMAL_TIMEOUT_VALUE, _ugview_genlist_update, ad);
	return ECORE_CALLBACK_CANCEL;
}

static void _searchbar_entry_preedit_changed_cb(void *data,
		Evas_Object *obj, void *event_info)
{
	retm_if(!data, "data null");
	CLK_FUN_BEG();
	struct appdata *ad = (struct appdata *)data;
	char tmp[BUF_SIZE] = { 0, };
	char buf[BUF_SIZE] = { 0, };
	wchar_t wbuf[BUF_SIZE] = { 0, };
	Eina_Bool paste_flag = EINA_FALSE;
	int i;
	int idx = 0;
	const char *entry_str = NULL;

	entry_str = elm_entry_entry_get(obj);
	if (entry_str != NULL) {
		strncpy(tmp, entry_str, BUF_SIZE-1);
	}

	//reset current city
	if(tmp[0] != '\0')
	    ad->current_mcc[0] = '\0';

	for (i = 0; i < strlen(tmp); i++) {
//              CLK_INFO_GREEN("tmp[%d] = %c\n", i, tmp[i]);
		if (tmp[i] == '<') {
			paste_flag = EINA_TRUE;
		} else if (tmp[i] == '>') {
			paste_flag = EINA_FALSE;
		}
		if (!paste_flag) {
			if (tmp[i] != '>') {
				buf[idx] = tmp[i];
				idx++;
			}
		}
	}

	int length = 0;
	mbstowcs(wbuf, buf, 255);
	length = wcslen(wbuf);

	if (length > MAX_LEN_INPUT) {
		if (isSearchEntryMaxReached) {
			CLK_INFO("Name entry already reached max length");
			return;
		} else {
			isSearchEntryMaxReached = EINA_TRUE;
		}
	} else {
		isSearchEntryMaxReached = EINA_FALSE;
		char *str = NULL;
		int strLength = 0;
		Eina_Bool isNeedUpdate = EINA_TRUE;
		str = (char *)elm_object_text_get(obj);
		char *input_text = NULL;
		input_text = elm_entry_markup_to_utf8(str);
		retm_if(!input_text, "input_text is NULL");

		CLK_INFO_YELLOW("input_text = %s\n", input_text);

		if (!strcmp(input_text, ad->search_text)) {
			FREEIF(input_text);
			return;
		}

		CLK_INFO("ad->search_text: %s\n", ad->search_text);

		if ((!strcmp(ad->search_text, "")
				&& !strcmp(input_text, _("IDS_COM_SK_SEARCH")))) {
			CLK_INFO("Not Update\n");
			isNeedUpdate = EINA_FALSE;
		}

		UChar ustr[BUF_SIZE] = { 0 };
		u_uastrncpy(ustr, input_text, BUF_SIZE);
		strLength = u_strlen(ustr);
		CLK_INFO("string length : %d\n", strLength);
		if (strLength > MAX_LEN_INPUT) {
			// the content of entry won't change if enter too many characters
			elm_object_text_set(obj, ad->search_text);
			// set cursor to the end of entry
			elm_entry_cursor_end_set(obj);
			return;
		}

		if (input_text[0] != '\0') {
			// reset search text
			memset(ad->search_text, '\0', BUF_SIZE);
			if (strcmp(input_text, _("IDS_COM_SK_SEARCH"))
					&& strcmp(input_text, "")) {
				/*strlcpy(ad->search_text, input_text,
						strlen(input_text) + 1);*/
			}
			CLK_INFO_GREEN("ad->search_text:%s\n", ad->search_text);
		} else {
			ad->search_text[0] = '\0';
		}

		CLK_INFO("ad->search_text:%s\n", ad->search_text);
		// if search text in search_bar is not "Search", update genlist
		if (EINA_TRUE == isNeedUpdate) {
			CLK_INFO("Update the search view text, search_text=%s\n",
					ad->search_text);

			ECORE_TIMER_DELIF(g_ad->add_view_update_timer);
			// update genlist
			g_ad->add_view_update_timer =
					ecore_timer_add(NORMAL_TIMEOUT_VALUE, _ugview_genlist_update, ad);

		}
		FREEIF(input_text);
	}
	CLK_FUN_END();
}

static void _searchbar_entry_max_length_reached_cb(void *data, Evas_Object *obj,
		void *event_info)
{
	retm_if(!data, "data null");
	isSearchEntryMaxReached = EINA_TRUE;
	notification_status_message_post(_("IDS_COM_POP_MAXIMUM_NUMBER_OF_CHARACTERS_REACHED"));
}

static void _searchbar_entry_lang_changed(void *data, Evas_Object *obj, void *event_info)
{
#ifdef FEATURE_SORT_ORDER
	//Reset the Entry text when language is changed
	struct appdata *ad = (struct appdata *)data;
	if (ad->sort_popup) {
		elm_object_tree_focus_allow_set(ad->add_search_bar, EINA_TRUE);
		elm_object_part_text_set(ad->searchbar_entry, "elm.guide",
				_("IDS_COM_BODY_SEARCH"));
		elm_object_tree_focus_allow_set(ad->add_search_bar, EINA_FALSE);
	}
#endif
}

static void _searchbar_entry_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	CLK_FUN_BEG();

	if (!data) {
		return;
	}

	struct appdata *ad = (struct appdata *)data;
	if (!elm_entry_is_empty(ad->searchbar_entry) && elm_object_focus_get(ad->searchbar_entry)) {
		elm_object_signal_emit(ad->searchbar_rect, "elm,action,show,button", "");
	} else{
		elm_object_signal_emit(ad->searchbar_rect, "elm,action,hide,button", "");
	}

	if (isSearchEntryMaxReached) {
		int char_len = evas_string_char_len_get(elm_entry_entry_get(obj));
		if (char_len < MAX_LEN_INPUT) {
			isSearchEntryMaxReached = EINA_FALSE;
		}
	}
	char *search_str = elm_entry_markup_to_utf8(elm_entry_entry_get(obj));
	ret_if(NULL == search_str);
	if (search_str[0] != '\0') {
		memset(ad->search_text, '\0', BUF_SIZE);
		if (strcmp(search_str, "")) {
			strncpy(ad->search_text, search_str, strlen(search_str) + 1);
		}
	} else {
		ad->search_text[0] = '\0';
	}
	FREEIF(search_str);
	ECORE_TIMER_DELIF(ad->search_timer);
	ad->search_timer = ecore_timer_add(NORMAL_TIMEOUT_VALUE, _entry_changed_cb, ad);
}

static void _searchbar_clear_button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	CLK_FUN_BEG();

	Evas_Object *entry = (Evas_Object *)data;

	elm_entry_entry_set(entry, "");
}

static void _location_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	CLK_FUN_BEG();

	ret_if(!data);
	struct appdata *wa = (struct appdata *)data;

	int state = 0;
	char **cp_list = NULL;
	char *plmn = NULL;
	TapiHandle *handle1 = NULL;
	TapiHandle *handle2 = NULL;
	vconf_get_bool(VCONFKEY_TELEPHONY_READY, &state);

	if (state) { /* Telephony State - READY */
		cp_list = tel_get_cp_name_list();
		if (cp_list != NULL) {
			handle1 = tel_init(cp_list[0]);
			if (cp_list[1]) {
				handle2 = tel_init(cp_list[1]);
			}
			//strfreev(cp_list);
		}
	} else { /* Telephony State - not READY */
		CLK_ERR("tapi state is not ready. just init tapi with NULL parameter");
		handle1 = tel_init(NULL);
	}

	if (handle1 != NULL) {
		tel_get_property_string(handle1, TAPI_PROP_NETWORK_PLMN, &plmn);
	} else {
		CLK_ERR("tapi handler1 is null");
	}

	if (plmn == NULL && handle2 != NULL) {
		tel_get_property_string(handle2, TAPI_PROP_NETWORK_PLMN, &plmn);
	} else if(!handle2) {
		CLK_ERR("tapi handler2 is null");
	}
	CLK_ERR("Current network's PLMN = %s.", plmn);

	tel_deinit(handle1);
	tel_deinit(handle2);

	if (plmn) {
		strncpy(wa->current_mcc, plmn, 3);
		FREEIF(plmn);
		CLK_INFO("MCC from networ PLMN = %s, length = %d", wa->current_mcc, strlen(wa->current_mcc));

		if (strlen(wa->current_mcc) && strncmp(wa->current_mcc, "11111", 3) != 0) {
			CLK_ERR("find");
			elm_entry_entry_set(wa->searchbar_entry, "");
		} else {
			CLK_ERR("MCC from Network PLMN is not correct. Show error popup!");
			notification_status_message_post(_("IDS_CLOCK_POP_LOCATION_SERVICES_NOT_AVAILABLE"));
		}
	} else {
		CLK_ERR("Failed to get network PLMN. Show error popup!");
		notification_status_message_post(_("IDS_CLOCK_POP_LOCATION_SERVICES_NOT_AVAILABLE"));
	}
}

static void _location_btn_pressed_cb(void *data, Evas_Object *obj, void *event_info)
{
	CLK_FUN_BEG();
	ret_if(!data);
	Evas_Object *location_icon = (Evas_Object *)data;
	elm_object_signal_emit(location_icon, "location_btn.press", "elm");
}

static void _location_btn_unpressed_cb(void *data, Evas_Object *obj, void *event_info)
{
	CLK_FUN_BEG()
	ret_if(!data);
	Evas_Object *location_icon = (Evas_Object *)data;
	elm_object_signal_emit(location_icon, "location_btn.unpress", "elm");
}

static Evas_Object *__ugview_searchbar_add(Evas_Object *parent, void *data)
{
	CLK_FUN_BEG();
	retv_if((!parent || !data), NULL);
	struct appdata *ad = (struct appdata *)data;;

	Evas_Object *searchbar_layout = worldclock_load_edj(parent, WCL_EDJ_NAME, "editfield_layout");
	retv_if(!searchbar_layout, NULL);

	/* editfield layout */
	ad->searchbar_rect = elm_layout_add(searchbar_layout);
	elm_layout_theme_set(ad->searchbar_rect, "layout", "searchfield", "singleline");
	evas_object_size_hint_align_set(ad->searchbar_rect, EVAS_HINT_FILL, 0.0);
	evas_object_size_hint_weight_set(ad->searchbar_rect, EVAS_HINT_EXPAND, 0.0);
	evas_object_event_callback_add(searchbar_layout, EVAS_CALLBACK_DEL, _searchbar_layout_del_cb, NULL);

	/* entry */
	ad->searchbar_entry = elm_entry_add(ad->searchbar_rect);
	elm_entry_cnp_mode_set(ad->searchbar_entry, ELM_CNP_MODE_PLAINTEXT);
	elm_entry_single_line_set(ad->searchbar_entry, EINA_TRUE);
	elm_entry_scrollable_set(ad->searchbar_entry, EINA_TRUE);
	elm_entry_autocapital_type_set(ad->searchbar_entry, ELM_AUTOCAPITAL_TYPE_NONE);
	elm_entry_prediction_allow_set(ad->searchbar_entry, EINA_TRUE);
	elm_entry_input_panel_layout_set(ad->searchbar_entry, ELM_INPUT_PANEL_LAYOUT_NORMAL);
	elm_entry_input_panel_return_key_type_set(ad->searchbar_entry, ELM_INPUT_PANEL_RETURN_KEY_TYPE_SEARCH);
	elm_object_domain_translatable_part_text_set(ad->searchbar_entry, "elm.guide", PACKAGE, "IDS_COM_BODY_SEARCH");

	evas_object_event_callback_add(ad->searchbar_entry, EVAS_CALLBACK_DEL, _searchbar_entry_del_cb, NULL);

	static Elm_Entry_Filter_Limit_Size limit_filter_data = { MAX_LEN_INPUT, 0 };
	elm_entry_markup_filter_append(ad->searchbar_entry, elm_entry_filter_limit_size, &limit_filter_data);

	evas_object_smart_callback_add(ad->searchbar_entry, "changed", _searchbar_entry_changed_cb, data);
	evas_object_smart_callback_add(ad->searchbar_entry, "focused", _searchbar_entry_focused_cb, data);
	evas_object_smart_callback_add(ad->searchbar_entry, "maxlength,reached", _searchbar_entry_max_length_reached_cb, data);
	evas_object_smart_callback_add(ad->searchbar_entry,	"preedit,changed", _searchbar_entry_preedit_changed_cb, data);
	evas_object_smart_callback_add(ad->searchbar_entry, "language,changed",	_searchbar_entry_lang_changed, data);
	evas_object_smart_callback_add(ad->searchbar_entry, "activated", _searchbar_entry_activated_cb, NULL);

	elm_object_part_content_set(ad->searchbar_rect, "elm.swallow.content", ad->searchbar_entry);
	elm_object_part_content_set(searchbar_layout, "entry_part", ad->searchbar_rect);

	/* clear button */
	ad->searchbar_button = elm_button_add(ad->searchbar_rect);
	elm_object_style_set(ad->searchbar_button, "editfield_clear");
	evas_object_smart_callback_add(ad->searchbar_button, "clicked", _searchbar_clear_button_clicked_cb, ad->searchbar_entry);
	elm_object_part_content_set(ad->searchbar_rect, "elm.swallow.button", ad->searchbar_button);

	/* current city button */
	Evas_Object *location_btn = elm_button_add(searchbar_layout);
	elm_object_style_set(location_btn, "transparent");
	Evas_Object *location_icon = elm_layout_add(location_btn);
	elm_layout_file_set(location_icon, WCL_EDJ_NAME, "location_icon");
	elm_object_part_content_set(location_btn, "elm.swallow.content", location_icon);
	elm_object_part_content_set(searchbar_layout, "location_sw", location_btn);
	evas_object_smart_callback_add(location_btn, "clicked", _location_btn_clicked_cb, ad);
	evas_object_smart_callback_add(location_btn, "pressed", _location_btn_pressed_cb, location_icon);
	evas_object_smart_callback_add(location_btn, "unpressed", _location_btn_unpressed_cb, location_icon);
	evas_object_show(location_btn);

	return searchbar_layout;
}

static Evas_Object *_ugview_add_layout(Evas_Object *parent)
{
	CLK_FUN_BEG();
	retv_if(!parent, NULL);

	// create window layout
	Evas_Object *layout = worldclock_load_edj(parent, WCL_EDJ_NAME, "searchbar_base");
	retv_if(!layout, NULL);

	elm_object_signal_emit(layout, "elm,state,show,searchbar", "elm");

	return layout;
}

static void _ug_reply()
{
	CLK_FUN_BEG();
	app_control_h app_control = NULL;
	app_control_create(&app_control);
	app_control_add_extra_data(app_control, "view", "destroy");
	ug_send_result(g_ad->ug, app_control);
	app_control_destroy(app_control);

	ug_destroy_me(g_ad->ug);
}

static void _back_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	ret_if(!data);
	struct appdata *ad = (struct appdata *)data;

	ECORE_TIMER_DELIF(ad->add_view_quit_timer);
	g_return_cb = NULL;
	Ecore_IMF_Context *imf_context = elm_entry_imf_context_get(ad->searchbar_entry);
	if (imf_context
			&& ECORE_IMF_INPUT_PANEL_STATE_HIDE !=
			ecore_imf_context_input_panel_state_get(imf_context)) {
		ecore_imf_context_input_panel_hide(imf_context);
		ad->add_view_quit_timer = ecore_timer_add(EXTENDED_TIMEOUT_VALUE, _ugview_exit_prev_btn_cb, NULL);
	} else {
		ad->add_view_quit_timer = ecore_timer_add(NORMAL_TIMEOUT_VALUE, _ugview_exit_prev_btn_cb, NULL);
	}
}

static Eina_Bool _ugview_back_button_cb(void *data, Elm_Object_Item *It)
{
	retv_if(!data, EINA_FALSE);
	struct appdata *ad = (struct appdata *)data;

	Ecore_IMF_Context *imf_context = elm_entry_imf_context_get(ad->searchbar_entry);
	if (imf_context
			&& ECORE_IMF_INPUT_PANEL_STATE_HIDE !=
			ecore_imf_context_input_panel_state_get(imf_context)) {
		ecore_imf_context_input_panel_hide(imf_context);
		return EINA_FALSE;
	} else {
		ecore_job_add(_ug_reply, NULL);
		return EINA_FALSE;
	}
}

#ifdef FEATURE_SORT_ORDER
static Evas_Object *_sort_gl_content_get(void *data, Evas_Object *obj, const char *part)
{
	struct appdata *ad = g_ad;
	if (NULL == ad) {
		return NULL;
	}
	int index = (Wcl_Addlist_Sort_Type) data;
	if (!strcmp(part, "elm.swallow.icon.1")) {
		Evas_Object *radio = elm_radio_add(obj);
		elm_radio_state_value_set(radio, index);
		elm_radio_group_add(radio, ad->sort_radio);
		return radio;
	}
	return NULL;
}

static char *_sort_gl_text_get(void *data, Evas_Object *obj, const char *part)
{
	Wcl_Addlist_Sort_Type index = (Wcl_Addlist_Sort_Type) (data);
	if (strcmp(part, "elm.text") == 0) {
		switch (index) {
		case WCL_SORT_BY_NAME:
			return strdup(_("IDS_COM_BODY_DETAILS_NAME"));
		case WCL_SORT_BY_TIMEZONE:
			return strdup(_("IDS_WCL_TAB_TIME"));
		default:
			break;
		}
	}
	return NULL;
}

static void _sort_gl_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	Elm_Object_Item *item = (Elm_Object_Item *) event_info;
	elm_genlist_item_selected_set(item, EINA_FALSE);

	ad->sort_text = (Wcl_Addlist_Sort_Type) (elm_genlist_item_index_get(item));
	CLK_INFO_CYAN("sort text : %d\n", ad->sort_text);

	if (ad->sort == ad->sort_text && !strlen(ad->current_mcc)) {
		CLK_INFO("Same sort option selected. Just remove popup menu");
		goto End;
	}

	elm_radio_value_set(ad->sort_radio, ad->sort_text);

	// set sort flag, and update genlist
	if (ad->sort_text == WCL_SORT_BY_TIMEZONE) {
		ad->sort = WCL_SORT_BY_TIMEZONE;
	} else if (ad->sort_text == WCL_SORT_BY_NAME) {
		ad->sort = WCL_SORT_BY_NAME;
	}
	// update genlist
	ad->current_mcc[0] = '\0';
	ECORE_TIMER_DELIF(ad->add_view_update_timer);
	ad->add_view_update_timer = ecore_timer_add(NORMAL_TIMEOUT_VALUE, _ugview_genlist_update, ad);

End:
	//_alarm_view_genlist_update_layout(ad);
	EVAS_OBJECT_DELIF(ad->sort_popup_genlist);
	EVAS_OBJECT_DELIF(ad->sort_popup);
	if (!elm_object_tree_focus_allow_get(ad->add_search_bar)) {
		elm_object_tree_focus_allow_set(ad->add_search_bar, EINA_TRUE);
	}
}

static void _sort_popup_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	EVAS_OBJECT_DELIF(ad->sort_popup_genlist);
	evas_object_del(ad->sort_popup);
	ad->sort_popup = NULL;
	if (!elm_object_tree_focus_allow_get(ad->add_search_bar)) {
		elm_object_tree_focus_allow_set(ad->add_search_bar, EINA_TRUE);
	}
}

static void _sort_gl_realized_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!event_info) {
		return;
	}
	Elm_Object_Item *gli = (Elm_Object_Item *) (event_info);
	int index = (int)elm_object_item_data_get(gli);
	// genlist last item
	if (index == WCL_SORT_BY_TIMEZONE) {
		elm_object_item_signal_emit(gli, "elm,state,bottomline,hide", "");	// send this signal
	}
}

static void _sort_gl_lang_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	// Update genlist items. The Item texts will be translated in the gl_text_get().
	elm_genlist_realized_items_update(obj);
}

static void __ctx_popup_sort_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	retm_if(!data, "data null");
	struct appdata *ad = (struct appdata *)data;

	if (ad->more_popup) {
		evas_object_del(ad->more_popup);
		ad->more_popup = NULL;
	}
	if (elm_object_tree_focus_allow_get(ad->add_search_bar)) {
		elm_object_tree_focus_allow_set(ad->add_search_bar, EINA_FALSE);
	}

	ad->sort_popup = elm_popup_add(ad->ly_main);
	eext_object_event_callback_add(ad->sort_popup, EEXT_CALLBACK_BACK, _sort_popup_back_cb,
			ad);

	evas_object_smart_callback_add(ad->sort_popup, "block,clicked",
			_sort_popup_back_cb, ad);
	elm_object_domain_translatable_part_text_set(ad->sort_popup, "title,text",
			PACKAGE, "IDS_COM_OPT_SORT_BY");
	evas_object_size_hint_weight_set(ad->sort_popup, EVAS_HINT_EXPAND,
			EVAS_HINT_EXPAND);

	Evas_Object *genlist = elm_genlist_add(ad->sort_popup);
	elm_object_style_set(genlist, "popup");
	elm_genlist_homogeneous_set(genlist, EINA_TRUE);
	elm_scroller_policy_set(genlist, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	elm_scroller_content_min_limit(genlist, EINA_FALSE, EINA_TRUE);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);

	evas_object_smart_callback_add(genlist, "realized", _sort_gl_realized_cb, NULL);
	evas_object_smart_callback_add(genlist, "language,changed", _sort_gl_lang_changed_cb, ad);

	ad->sort_radio = elm_radio_add(genlist);
	if (ad->sort == WCL_SORT_BY_TIMEZONE) {
		elm_radio_value_set(ad->sort_radio, WCL_SORT_BY_TIMEZONE);
	} else {
		elm_radio_value_set(ad->sort_radio, WCL_SORT_BY_NAME);
	}
	ad->ex_sort_text = ad->sort_text;
	Wcl_Addlist_Sort_Type index = WCL_SORT_BY_NAME;

	for (index = WCL_SORT_BY_NAME; index < WCL_UNKNOW_SORT_TYPE; index++) {
		elm_genlist_item_append(genlist, &itc_sort, (void *)index, NULL,
				ELM_GENLIST_ITEM_NONE, _sort_gl_clicked_cb, (void *)ad);
	}
	elm_object_content_set(ad->sort_popup, genlist);

	evas_object_show(ad->sort_popup);
}

static void _more_ctxpopup_hide_cb(void *data, Evas_Object *obj, void *event_info)
{
	CLK_FUN_BEG();
	struct appdata *ad = (struct appdata *)data;
	if (ad->more_popup) {
		evas_object_del(ad->more_popup);
		ad->more_popup = NULL;
		elm_object_tree_focus_allow_set(ad->add_search_bar, EINA_TRUE);
	}
	CLK_FUN_END();
}

static void _move_more_ctxpopup(void *data)
{
	CLK_FUN_BEG();
	struct appdata *ad = (struct appdata *)data;
	if (ad->more_btn) {
		int w1, h1;
		int rotate;
		elm_win_screen_size_get(ad->win, NULL, NULL, &w1, &h1);
		rotate = elm_win_rotation_get(ad->win);
		CLK_INFO("rotate = %d w1 = %d, h1 = %d", rotate, w1, h1);

		if (rotate == 90) {
			evas_object_move(ad->more_popup, 0, w1);
		} else if (rotate == 270) {
			evas_object_move(ad->more_popup, h1, w1);
		} else {
			evas_object_move(ad->more_popup, 0, h1);
		}
	}
}

static void _navi_bar_move_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	CLK_FUN_BEG();
	_move_more_ctxpopup(data);
}

static void _navi_bar_delete_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = (struct appdata *)data;
	evas_object_event_callback_del(ad->navi_bar, EVAS_CALLBACK_MOVE,
			_navi_bar_move_cb);
	evas_object_smart_callback_del(ad->more_popup, "dismissed",
			_more_ctxpopup_hide_cb);
}

static void _more_button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (!data) {
		return;
	}
	CLK_FUN_BEG();
	struct appdata *ad = (struct appdata *)data;
	if (ad->is_Empty == EINA_TRUE) {
		return;
	}

	elm_object_focus_set(entry, EINA_FALSE);
	Ecore_IMF_Context *imf_context = elm_entry_imf_context_get(ad->searchbar_entry);
	if (imf_context
			&& ECORE_IMF_INPUT_PANEL_STATE_HIDE !=
			ecore_imf_context_input_panel_state_get(imf_context)) {
		ecore_imf_context_input_panel_hide(imf_context);
	}

	if (ad->searched_num == 0) {
		return;
	}

	elm_object_tree_focus_allow_set(ad->add_search_bar, EINA_FALSE);

	ad->more_popup = elm_ctxpopup_add(ad->ly_main);
	elm_object_style_set(ad->more_popup, "more/default");
	elm_ctxpopup_auto_hide_disabled_set(ad->more_popup, EINA_TRUE);
	eext_object_event_callback_add(ad->more_popup, EEXT_CALLBACK_BACK,
			eext_ctxpopup_back_cb, NULL);
	eext_object_event_callback_add(ad->more_popup, EEXT_CALLBACK_MORE,
			eext_ctxpopup_back_cb, NULL);

	Elm_Object_Item *sort_by =
			elm_ctxpopup_item_append(ad->more_popup, "IDS_COM_OPT_SORT_BY",
					NULL, __ctx_popup_sort_clicked_cb, ad);
	elm_object_item_domain_text_translatable_set(sort_by, PACKAGE, TRUE);

	evas_object_smart_callback_add(ad->more_popup, "dismissed",
			_more_ctxpopup_hide_cb, ad);
	evas_object_event_callback_add(ad->navi_bar, EVAS_CALLBACK_MOVE,
			_navi_bar_move_cb, ad);
	evas_object_event_callback_add(ad->navi_bar, EVAS_CALLBACK_DEL,
			_navi_bar_delete_cb, ad);

	_move_more_ctxpopup(ad);
	evas_object_show(ad->more_popup);
	CLK_FUN_END();
}

static Evas_Object *_ugview_create_more_btn(Evas_Object *parent, Evas_Smart_Cb func,
		void *data)
{
	Evas_Object *btn = elm_button_add(parent);
	if (!btn) {
		return NULL;
	}
	elm_object_style_set(btn, "naviframe/more/default");
	if (func) {
		evas_object_smart_callback_add(btn, "clicked", func, data);
	}
	return btn;
}
#endif

static void _gl_del_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	g_ad->add_genlist = NULL;
}

static void _searchbar_entry_del_cb(void *data, Evas *e,
		Evas_Object *obj, void *event_info)
{
	g_ad->searchbar_entry = NULL;
}

static void _searchbar_layout_del_cb(void *data, Evas *e,
		Evas_Object *obj, void *event_info)
{
	g_ad->add_search_bar = NULL;
}

static void _index_del_cb(void *data, Evas *e, Evas_Object *obj,
		void *event_info)
{
	g_ad->add_index = NULL;
}

int worldclock_ugview_add(Evas_Object *parent, void *data, Wcl_Return_Cb func)
{
	CLK_FUN_BEG();
	retv_if(!data, FAILED);
	int ret = SUCCESS;
	struct appdata *ad = (struct appdata *)data;
	char *lang = vconf_get_str(VCONFKEY_LANGSET);
	init_alphabetic_index(lang);
	FREEIF(lang);
	g_ad = ad;
	g_return_cb = func;
	const char *group_name = NULL;
	char title_name[MAX_TITLE_NAME] = { 0, };

	/*select call_flag & group name by caller */
	switch (ad->caller) {
	case WCL_CALLER_IS_APP_IT_SELF:
	case WCL_CALLER_IS_SHARED_LIBRARY:
		g_call_flag = CALL_FROM_MAIN_VIEW;
		group_name = GRP_ADD;
		if (ad->city_index == -1) {
			snprintf(title_name, sizeof(title_name), "%s",
					"IDS_WCL_OPT_ADDCITY");
		} else {
			snprintf(title_name, sizeof(title_name), "%s",
					"IDS_CLOCK_HEADER_EDIT_CITY");
		}
		break;
	case WCL_CALLER_IS_LIVEBOX:
		g_call_flag = CALL_FROM_LIVE_BOX;
		group_name = GRP_ADD;
		snprintf(title_name, sizeof(title_name), "%s",
				"IDS_WCL_OPT_ADDCITY");
		break;
	case WCL_CALLER_IS_UI_GADGET:
		g_call_flag = CALL_FROM_UI_GADGET;
		group_name = GRP_ADD;
		snprintf(title_name, sizeof(title_name), "%s",
				"IDS_WCL_HEADER_SELECT_TIME_ZONE");
		break;
	default:
		break;
	}

	/* open data base */
	retv_if(EINA_FALSE == worldclock_ug_data_open_database(), FAILED);

	/* add layout */
	ad->add_ly = _ugview_add_layout(parent);
	retv_if(ad->add_ly == NULL, FAILED);

	/* init search data */
	memset(ad->search_text, 0x0, BUF_SIZE * sizeof(char));
	memset(ad->current_mcc, 0x0, MCC_BUF_SIZE * sizeof(char));

	/* load edje */
	ad->add_layout = worldclock_load_edj(ad->add_ly, WCL_EDJ_NAME, group_name);
	retv_if(ad->add_layout == NULL, FAILED);
	elm_object_part_content_set(ad->add_ly, "elm.swallow.content", ad->add_layout);

	/*fast scroll layout */
	ad->add_fs_layout = elm_layout_add(ad->add_layout);
	elm_layout_theme_set(ad->add_fs_layout, "layout", "application", "fastscroll");
	elm_object_part_content_set(ad->add_layout, "add/genlist", ad->add_fs_layout);

	/*genlist */
#ifdef FEATURE_SORT_ORDER
	ad->sort = WCL_SORT_BY_NAME;
#endif

	ad->add_genlist = _ugview_add_genlist(ad->add_fs_layout, data);
	retv_if(ad->add_genlist == NULL, FAILED);
	/*add callback for fixed the selected item location */
	evas_object_event_callback_add(ad->add_genlist, EVAS_CALLBACK_DEL,
			_gl_del_cb, NULL);
	evas_object_smart_callback_add(g_ad->add_genlist, "loaded",
			_gl_load_finished_cb, ad);

	/*reset index */
	ad->add_index = _ugview_add_index(ad->add_layout, ad);
	evas_object_event_callback_add(ad->add_index, EVAS_CALLBACK_DEL, _index_del_cb, NULL);
	retv_if(!ad->add_index, EINA_FALSE);
	elm_object_part_content_set(ad->add_fs_layout, "elm.swallow.fastscroll", ad->add_index);

	/*create left button */
	ad->back_btn = elm_button_add(ad->navi_bar);
	elm_object_style_set(ad->back_btn, "naviframe/back_btn/default");
	evas_object_smart_callback_add(ad->back_btn, "clicked", _back_btn_clicked_cb, ad);

	/*update genlist */
	ret = _ugview_genlist_update(ad);

	/*searchbar */
	ad->add_search_bar = __ugview_searchbar_add(ad->add_ly, data);
	retv_if(ad->add_search_bar == NULL, FAILED);
	elm_object_part_content_set(ad->add_ly, "searchbar", ad->add_search_bar);

	/* set segment control as title about layout */
	Elm_Object_Item *elm_item = elm_naviframe_item_push(ad->navi_bar, title_name, ad->back_btn, NULL, ad->add_ly, NULL);
	elm_object_item_domain_text_translatable_set(elm_item,PACKAGE,EINA_TRUE);
	elm_naviframe_item_pop_cb_set(elm_item, _ugview_back_button_cb, ad);
	elm_naviframe_item_title_enabled_set(elm_item, EINA_TRUE, EINA_FALSE);

#ifdef FEATURE_SORT_ORDER
	ad->more_btn = _ugview_create_more_btn(ad->navi_bar, _more_button_clicked_cb, ad);
	elm_object_item_part_content_set(elm_item, "toolbar_more_btn", ad->more_btn);
#endif

	ecore_idler_add(_set_entry_focus_idler_cb, data);

	CLK_FUN_END();
	return ret;
}

int worldclock_ugview_update(void *data)
{
	CLK_FUN_BEG();

	int ret = FAILED;
	retvm_if(!data, ret, "data is NULL");

	ret = _ugview_genlist_update(data);

	CLK_FUN_END();
	return ret;
}

void worldclock_ugview_free(void *data)
{
	CLK_FUN_BEG();
	ret_if(!data);

	struct appdata *ad = (struct appdata *)data;
	uninit_alphabetic_index();
	worldclock_ug_data_close_database();

	if (ad->add_ly) {
		_ugview_clear_data(data);
	}
	CLK_FUN_END();
}
