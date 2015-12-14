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
