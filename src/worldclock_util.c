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

#include <stdio.h>
#include <string.h>
#include <appcore-efl.h>
#include <Elementary.h>
#include <vconf.h>
#include <unicode/ustring.h>
#include <unicode/ucol.h>
#include "worldclock_util.h"
#include "worldclock_dlog.h"
#include "worldclock_fwk_icu.h"
#include "worldclock_data.h"

#ifndef UG_MODULE_API
#define UG_MODULE_API __attribute__ ((visibility("default")))
#endif

/**
 * Compare the city name of two cities
 *
 * @param[in]  data1   record data of city1
 * @param[in]  data2   record data of city2
 *
 * @return     -1 if city name of city2 is bigger than city1's
 *              1 if city name of city1 is bigger than city2's
 *              0 if city name of city2 is equal to city1, or meet error
 */
int worldclock_city_compare_cb(const void *data1, const void *data2)
{
	retv_if(!data1 || !data2, 0);

	UChar bufcity1[CITY_BUF_SIZE] = { 0 };
	UChar bufcity2[CITY_BUF_SIZE] = { 0 };

	const Wcl_CitySet *cs1, *cs2;

	cs1 = (const Wcl_CitySet *)data1;
	cs2 = (const Wcl_CitySet *)data2;

	char *city1 = _(cs1->city);
	char *city2 = _(cs2->city);

	u_uastrcpy(bufcity1, city1);
	u_uastrcpy(bufcity2, city2);

	UErrorCode status = U_ZERO_ERROR;
	UCollator *coll = ucol_open(getenv("LANG"), &status);
	retv_if (!coll, 0);

	UCollationResult ret = ucol_strcoll(coll, bufcity1, -1, bufcity2, -1);

	ucol_close(coll);

	switch (ret) {
	case UCOL_EQUAL:
		return 0;
	case UCOL_GREATER:
		return 1;
	case UCOL_LESS:
		return -1;
	default:
		return 0;
	}
}

/**
 * Compare timezone of two cities
 *
 * @param[in]  data1   recorder of city1
 * @param[in]  data2   recorder of city2
 *
 * @return      1 if timezone of city1 is bigger than city2's
 *              0 if timezone of city2 is equal to city1, or meet error
 *             -1 if timezone of city2 is bigger than city1's
 */
int worldclock_time_compare_cb(const void *data1, const void *data2)
{
	retv_if(!data1 || !data2, 0);

	float diff_val = 0.0;
	int ret;
	// get record data
	const Wcl_CitySet *cs1 = (const Wcl_CitySet *)data1;
	const Wcl_CitySet *cs2 = (const Wcl_CitySet *)data2;

	char buf_tz1[TIMEZONE_BUF_SIZE] = { 0, };
	char buf_tz2[TIMEZONE_BUF_SIZE] = { 0, };

	const char *pbegin = NULL;
	const char *ppos = NULL;

	strncpy(buf_tz1, cs1->timezone + 3, TIMEZONE_BUF_SIZE);
	pbegin = buf_tz1;
	ppos = strstr(buf_tz1, ":");
	if (ppos) {
		buf_tz1[ppos - pbegin] = '.';
	}

	strncpy(buf_tz2, cs2->timezone + 3, TIMEZONE_BUF_SIZE);
	pbegin = buf_tz2;
	ppos = strstr(buf_tz2, ":");
	if (ppos) {
		buf_tz2[ppos - pbegin] = '.';
	}
	// get timezone
	float d1 = atof(buf_tz1);
	float d2 = atof(buf_tz2);

	// get diff between of two cities
	diff_val = d1 - d2;
	// get result
	if (diff_val > 0) {
		ret = 1;
	} else if (diff_val < 0) {
		ret = -1;
	} else {
		ret = 0;
	}
	return ret;
}

/**
 * Compare sequence of two cities
 *
 * @param[in]  data1   recorder of city1
 * @param[in]  data2   recorder of city2
 *
 * @return      1 if sequence of city1 is bigger than city2's
 *              0 if sequence of city2 is equal to city1, or meet error
 *             -1 if sequence of city2 is bigger than city1's
 */
int worldclock_sequence_compare_cb(const void *data1, const void *data2)
{
	retv_if(!data1 || !data2, 0);
	int ret;

	const Wcl_CitySet *cs1 = (const Wcl_CitySet *)data1;
	const Wcl_CitySet *cs2 = (const Wcl_CitySet *)data2;

	// get result
	if (cs1->sequence > cs2->sequence) {
		ret = 1;
	} else if (cs1->sequence < cs2->sequence) {
		ret = -1;
	} else {
		ret = 0;
	}
	return ret;
}

/**
 * Checking whether the count of city list meet the max number
 *
 * @param[in]  ap  given data used in this function
 * @param[in]  cs  given city
 *
 * @return     EINA_TRUE if list full
 *             EINA_FALSE if not
 */
Eina_Bool worldclock_is_city_list_full(Eina_List * eina_list)
{
	CLK_FUN_DEBUG_BEG();
	retv_if(!eina_list, EINA_FALSE);
	Eina_Bool ret = EINA_FALSE;

	int count = eina_list_count(eina_list);
	if (count >= WORLDCLOCK_MAX_CITY_COUNT) {
		CLK_INFO("list full, please delete some\n");
		ret = EINA_TRUE;
	}
	CLK_FUN_DEBUG_END();
	return ret;
}

/**
 * Insert given city into given eina list
 *
 * @param[in]  p_eina_list  given data used in this function
 * @param[in]  cs           given city
 * @param[in]  position     position where the city is insert into(-1: append to list)
 *
 * @return     -1 if not exist
 *             0 if list full
 *             list-count if success,
 */
int worldclock_insert_city_to_list(Eina_List ** p_eina_list, Wcl_CitySet * cs,
		int position)
{
	CLK_FUN_DEBUG_BEG();
	retv_if(!p_eina_list || !cs, -1);

	int count = 0;
	count = eina_list_count(*p_eina_list);
	if (count >= WORLDCLOCK_MAX_CITY_COUNT) {
		CLK_INFO("list full, please delete some\n");
		return 0;
	}
	// set selected flag
	cs->selected = 1;

	if (0 > position) {
		*p_eina_list = eina_list_append(*p_eina_list, cs);

		// set sequence
		cs->sequence = count;
	} else if (0 == position) {
		*p_eina_list = eina_list_prepend(*p_eina_list, cs);

		// set sequence
		Eina_List *el = *p_eina_list;
		int index = 0;
		while (el) {
			cs = (Wcl_CitySet *) el->data;
			cs->sequence = index;

			// get next item
			el = el->next;
			index++;
		}
	} else {
		//  T.B.D.
	}

	count = eina_list_count(*p_eina_list);

	CLK_FUN_DEBUG_END();
	return count;
}

/**
 * Remove all items in eina_list
 *
 * @param[in]  glist            given eina list
 * @param[in]  is_free_element  flag to define if free data in every item is needed
 *
 * @return     EINA_TRUE if remove successfully
 */
Eina_Bool worldclock_util_glist_remove_all(Eina_List * glist, Eina_Bool is_free_element)
{
	CLK_FUN_DEBUG_BEG();
	if (glist != NULL) {
		if (EINA_TRUE == is_free_element) {
			void *user_data = NULL;
			Eina_List *tmp_list = glist;
			while (tmp_list) {
				user_data = tmp_list->data;
				if (user_data) {
					free(user_data);
					user_data = NULL;
				}
				// get next data
				tmp_list = tmp_list->next;
			}
		}
		// free eina list
		eina_list_free(glist);
		glist = NULL;
	}

	CLK_FUN_DEBUG_END();
	return EINA_TRUE;
}

// if exist, return EINA_TRUE, else return EINA_FALSE
/**
 * Check whether given city exist in given eina_list
 *
 * @param[in]  eina_list   given eina list
 * @param[in]  cs          record of given city
 * @return     EINA_FALSE if not exist
 *             EINA_TRUE if exist
 */
Eina_Bool worldclock_whether_city_exist_in_eina_list(Eina_List * eina_list,
		Wcl_CitySet * cs)
{
	CLK_FUN_DEBUG_BEG();
	Wcl_CitySet *cs_tmp = NULL;
	Eina_List *el = NULL;
	// get data list
	el = eina_list;
	while (el) {
		// get data
		cs_tmp = (Wcl_CitySet *) el->data;
		if (cs_tmp->index == cs->index) {
			CLK_FUN_DEBUG_END();
			return EINA_TRUE;
		}
		// get next data
		el = el->next;
	}

	CLK_FUN_DEBUG_END();
	return EINA_FALSE;
}

/**
 * Reset now time of genlist item data
 *
 * @param[in]  data      Data used in this function
 *
 * @return
 */
void worldclock_reset_now_time(Eina_List * eina_list)
{
	CLK_FUN_DEBUG_BEG();
	ret_if(!eina_list);

	Wcl_CitySet *cs = NULL;
	// get data list
	Eina_List *el = eina_list;
	while (el) {
		cs = (Wcl_CitySet *) el->data;
		cs->now_time = 0;
		// get next item
		el = el->next;
	}

	CLK_FUN_DEBUG_END();
}

/**
 * Create layout by load edj according group name from edj file
 *
 * @param[in]  parent   Parent of new layout
 * @param[in]  file     EDJE file
 * @param[in]  group    name of group
 *
 * @return     NULL if meet error
 *             Pointer to new layout
 */
Evas_Object *worldclock_load_edj(Evas_Object * parent, const char *file,
		const char *group)
{
	CLK_FUN_DEBUG_BEG();
	Evas_Object *eo = NULL;
	int r = 0;
	// add new layout
	eo = elm_layout_add(parent);
	if (eo) {
		// set edje file & name of group
		r = elm_layout_file_set(eo, file, group);
		if (!r) {
			evas_object_del(eo);
			return NULL;
		}
		// set hint
		evas_object_size_hint_weight_set(eo, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	}

	CLK_FUN_DEBUG_END();
	return eo;
}

/**
 * Search word in string
 * * Set color on the word if found
 * * Set max length to aimed string, replace tail with ...
 *
 * @param[in]  string      source string for search
 * @param[in]  searchword  word which used for search
 * @param[in]  max_len     Max length of string which to display
 *
 * @return  string which for displaying
 */
const char *worldclock_searchword_in_string(const char *string,
		char *searchword, int max_len)
{
	//SEARCH_FUNC_START;
	char pstr[BUF_LARGE_SIZE] = { 0, };
	char result_str[BUF_LARGE_SIZE] = { 0, }, start_str[BUF_LARGE_SIZE] = {
		0,
	};
	static char return_string[BUF_LARGE_SIZE] = { 0, };
	int word_len = 0, search_len = 0, i = 0;	//, hilight_len = 0, hilight_len_2 = 0;
	Eina_Bool found = EINA_FALSE;
	strncpy(pstr, string, BUF_LARGE_SIZE);
	word_len = strlen(pstr);
	search_len = strlen(searchword);
	if (word_len > max_len) {
		snprintf(&pstr[max_len], BUF_LARGE_SIZE - max_len - 1, "%s", "...");
	}
	for (i = 0; i < word_len; i++) {
		if (!strncasecmp(searchword, &pstr[i], search_len)) {
			found = EINA_TRUE;
			break;
		}
	}
	if (EINA_TRUE == found) {
		if (i == 0) {
			strncpy(result_str, &pstr[i], search_len);
			result_str[search_len] = '\0';
			snprintf(return_string, BUF_LARGE_SIZE,
					"<color=#e58616>%s</color>%s", &result_str[0],
					&pstr[search_len]);
		} else if (i > 0) {
			strncpy(start_str, &pstr[0], i);
			start_str[i + 1] = '\0';
			strncpy(result_str, &pstr[i], search_len);
			result_str[search_len] = '\0';
			snprintf(return_string, BUF_LARGE_SIZE,
					"%s<color=#e58616>%s</color>%s", &start_str[0],
					&result_str[0], &pstr[i + search_len]);
		}
	} else {
		snprintf(return_string, BUF_LARGE_SIZE, "%s", pstr);
	}

	return return_string;
}

/**
 * This function is used to compute the length of string which displaying on entry
 * The html flag which could change the actual length of string should be ignored.
 *
 * @param[in]  str    source string which got from entry
 *
 * @return  The length of the displaying part about str
 */
int worldclock_str_get_displaying_length(const char *str)
{
	retv_if(NULL == str, -1);
	int len = strlen(str);

	char *pre_flag = strchr(str, '<');

	while (pre_flag) {
		int sub_num = 1;
		char *end_flag = strchr(pre_flag, '>');
		if (end_flag) {
			sub_num += end_flag - pre_flag;
			pre_flag = strchr(end_flag, '<');
		}
		len -= sub_num;
	}

	return len;
}

/**
 * This function is used to justify whethet html flag exist in string
 *
 * @param[in]  str    source string which got from entry
 *
 * @return  EINA_TRUE if contain html
 */
Eina_Bool worldclock_str_is_contain_html(const char *str)
{
	retv_if(NULL == str, EINA_FALSE);
	Eina_Bool ret = EINA_FALSE;

	char *pre_flag = strchr(str, '<');

	if (pre_flag) {
		ret = EINA_TRUE;
	}

	return ret;
}

/**
 * This function is used to convert string type from Unicode to UTF8
 *
 * @param[in]  unichars    source string whose type is Unicode
 *
 * @return  The result string whose type is UTF8
 */
char *worldclock_strToUTF8(const UChar * unichars)
{
	retv_if(unichars == NULL, NULL);
	int len = 0;
	int len_str = 0;
	int len_utf8 = 0;
	char *str = NULL;
	UErrorCode status = U_ZERO_ERROR;

	len = u_strlen(unichars);
	len_str = sizeof(char) * 4 * (len + 1);
	str = (char *)calloc(1, len_str);
	retv_if(NULL == str, NULL);

	u_strToUTF8(str, len_str, &len_utf8, unichars, len, &status);
	return str;
}

int worldclock_dst_get(const Wcl_CitySet * cs)
{
	int dst = 0;
	if (cs->dst_enabled) {
		switch (cs->dst_type) {
		case WCL_DST_AUTO:
			dst = worldclock_icu_dst_get(cs->tz_path);
			break;
		case WCL_DST_1_HOUR:
			dst = 1;
			break;
		case WCL_DST_2_HOURS:
			dst = 2;
			break;
		default:
			dst = 0;
			break;
		}
	}
	return dst;
}

Evas_Object *widget_create_controlbar(Evas_Object * parent, const char *style)
{
	retv_if(!parent, NULL);
	Evas_Object *ret = elm_toolbar_add(parent);
	elm_toolbar_shrink_mode_set(ret, ELM_TOOLBAR_SHRINK_EXPAND);
	if (style) {
		elm_object_style_set(ret, style);
		if (0 == strcmp(style, "tabbar")) {
			elm_toolbar_select_mode_set(ret, ELM_OBJECT_SELECT_MODE_ALWAYS);
		}
	}
	return ret;
}
