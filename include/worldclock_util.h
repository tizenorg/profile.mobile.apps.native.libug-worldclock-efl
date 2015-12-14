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

int worldclock_dst_get(const Wcl_CitySet * cs);

#endif				/* __DEF_WORLDCLOCK_UTIL_H_ */
