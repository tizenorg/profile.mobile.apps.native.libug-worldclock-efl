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

#ifndef __DEF_WORLDCLOCK_UTIL_H_
#define __DEF_WORLDCLOCK_UTIL_H_

#include <appcore-efl.h>
#include <Elementary.h>
#include <unicode/umachine.h>
#include "worldclock.h"

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
int worldclock_city_compare_cb(const void *data1, const void *data2);

/**
 * Compare timezone of two cities
 *
 * @param[in]  data1   recorder of city1
 * @param[in]  data2   recorder of city2
 *
 * @return     -1 if timezone of city2 is bigger than city1's
 *              1 if timezone of city1 is bigger than city2's
 *              0 if timezone of city2 is equal to city1, or meet error
 */
int worldclock_time_compare_cb(const void *data1, const void *data2);

/**
 * Compare sequence of two cities
 *
 * @param[in]  data1   recorder of city1
 * @param[in]  data2   recorder of city2
 *
 * @return     -1 if sequence of city2 is bigger than city1's
 *              1 if sequence of city1 is bigger than city2's
 *              0 if sequence of city2 is equal to city1, or meet error
 */
int worldclock_sequence_compare_cb(const void *data1, const void *data2);

/**
 * Checking whether the count of city list meet the max number
 *
 * @param[in]  city list
 *
 * @return     EINA_TRUE if list full
 *             EINA_FALSE if not
 */
Eina_Bool worldclock_is_city_list_full(Eina_List * eina_list);

/**
 * Insert given city into given eina list
 *
 * @param[in]  city list
 * @param[in]  cs  given city
 *
 * @return     -1 if not exist
 *             0 if list full
 *             list-count if success,
 */
int worldclock_append_city_to_list(Eina_List ** p_eina_list, Wcl_CitySet * cs);

/**
 * Insert given city into given eina list
 *
 * @param[in]  city list
 * @param[in]  cs  given city
 * @param[in]  position to insert
 *
 * @return     -1 if not exist
 *             0 if list full
 *             list-count if success,
 */
int worldclock_insert_city_to_list(Eina_List ** p_eina_list, Wcl_CitySet * cs,
		int position);

/**
 * Remove all items in eina_list
 *
 * @param[in]  glist            given eina list
 * @param[in]  is_free_element  flag to define if free data in every item is needed
 *
 * @return     EINA_TRUE if remove successfully
 */
Eina_Bool worldclock_util_glist_remove_all(Eina_List * glist, Eina_Bool is_free_element);
#if 0
/**
 * Creat new popup window, and show it
 *
 * @param[in]  parent Parent for such popup
 * @param[in]  data   Data used in this function
 * @param[in]  info   String displayed on popup
 *
 * @return
 */
void worldclock_show_popup(Evas_Object * parent, Evas_Object ** p_popup, char *info);
#endif
/**
 * Check whether given city exist in given eina_list
 *
 * @param[in]  eina_list   given eina list
 * @param[in]  cs          record of given city
 * @return     EINA_FALSE if not exist
 *             EINA_TRUE if exist
 */
Eina_Bool worldclock_whether_city_exist_in_eina_list(Eina_List * eina_list,
		Wcl_CitySet * cs);

/**
 * Reset now time of genlist item data
 *
 * @param[in]  eina_list   given eina list
 *
 * @return
 */
void worldclock_reset_now_time(Eina_List * eina_list);

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
		const char *group);

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
		char *searchword, int max_len);

/**
 * This function is used to compute the length of string which displaying on entry
 * The html flag which could change the actual length of string should be ignored.
 *
 * @param[in]  str    source string which got from entry
 *
 * @return  The length of the displaying part about str
 */
int worldclock_str_get_displaying_length(const char *str);

/**
 * This function is used to justify whethet html flag exist in string
 *
 * @param[in]  str    source string which got from entry
 *
 * @return  EINA_TRUE if contain html
 */
Eina_Bool worldclock_str_is_contain_html(const char *str);

/**
 * This function is used to convert string type from Unicode to UTF8
 *
 * @param[in]  unichars    source string whose type is Unicode
 *
 * @return  The result string whose type is UTF8
 */
char *worldclock_strToUTF8(const UChar * unichars);

int worldclock_dst_get(const Wcl_CitySet * cs);
time_t worldclock_genlist_time_get(Wcl_CitySet * cs, void *data);

Evas_Object *widget_create_controlbar(Evas_Object * parent, const char *style);

#endif				/* __DEF_WORLDCLOCK_UTIL_H_ */
