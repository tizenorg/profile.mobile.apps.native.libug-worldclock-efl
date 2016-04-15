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

#ifndef __DEF_WORLDCLOCK_H__
#define __DEF_WORLDCLOCK_H__

#include <Elementary.h>
#include <Ecore_IMF.h>
#include <app.h>

#include "worldclock_const.h"
#include "worldclock_types.h"

struct appdata {
	int root_w;
	int root_h;
	int root_x;
	int root_y;

	//the caller
	Wcl_Caller caller;
	//the selection flag
	Wcl_Selection_Flag selectionFlag;

	//flag for if or not enter the loop update
	bool flag_first;	//as the flag for the first time to enter __ugview_index_update
	bool flag_special_character;	//as the flag for if or not have special character in index

	//For store the default country list
	Eina_List *default_list;
	//flag for if or not hide the index
	bool flag_index;

	app_control_h app_caller;

	/* Add city List */
	//For save current search content
	char search_text[BUF_SIZE];
	char current_mcc[MCC_BUF_SIZE];

	Elm_Object_Item *item_selected;	//for store the selected  city item
	int city_index;		//for store the selected city index

	Ecore_IMF_Context *imf_context;

	/* public used widget */
	Evas_Object *win;
	Evas_Object *conform;
	Evas_Object *bg;

	// samsung window layout
	Evas_Object *ly_main;
	Evas_Object *more_btn;
	Evas_Object *back_btn;

	/* add layout */
	Evas_Object *add_ly;
	Evas_Object *add_layout;
	Evas_Object *add_search_bar;
	Evas_Object *add_fs_layout;
	Evas_Object *add_genlist;
	Evas_Object *add_index;
	Evas_Object *nocontent;

	Evas_Object *searchbar_rect;
	Evas_Object *searchbar_entry;
	Evas_Object *searchbar_button;

	/* timer */
	Ecore_Timer *add_view_quit_timer;
	Ecore_Timer *add_view_update_timer;
	Ecore_Timer *search_timer;

	/* Home City */
	Wcl_CitySet *home_cs;

	Evas_Object *navi_bar;

	// return a Wcl_CitySet, used in ug
	Wcl_CitySet *return_data;

#ifdef FEATURE_SORT_ORDER
	//mark sort by name or gmt
	Wcl_Addlist_Sort_Type sort;
	Evas_Object *more_popup;

	//sort popup box
	Evas_Object *sort_popup;	//sort popup
	Evas_Object *sort_popup_genlist;	//sort popup genlist
	Evas_Object *sort_radio;	//sort radio
	Wcl_Addlist_Sort_Type sort_text;	//sort text
	Wcl_Addlist_Sort_Type ex_sort_text;	//sort text
#endif

	Eina_Bool is_Empty;
	int searched_num;
	int searched_text_length;
};

#endif				/* __DEF_WORLDCLOCK_H_ */

