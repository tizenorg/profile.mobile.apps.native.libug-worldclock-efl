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

#ifndef __DEF_WORLDCLOCK_DATA_H_
#define __DEF_WORLDCLOCK_DATA_H_

#include <Elementary.h>
#include "worldclock_types.h"

Wcl_CitySet *worldclock_ug_data_get_local_city();

Eina_List *worldclock_ug_data_get_all_added_city();
Eina_List *worldclock_ug_data_get_default_city_list(Wcl_Selection_Flag selectFlag);
Eina_List *worldclock_ug_data_get_search_city_list(const char *search_txt,
		Wcl_Search_Type search_type, Wcl_Selection_Flag selectFlag);

Eina_Bool worldclock_ug_data_update_db_record(Wcl_CitySet * p_record);
Eina_Bool worldclock_ug_data_get_city_status_from_db(Wcl_CitySet * p_record);
Eina_Bool worldclock_ug_data_open_database();
Eina_Bool worldclock_ug_data_close_database();

int worldclock_ug_data_get_selected_city_num();

void worldclock_ug_data_cityset_copy(Wcl_CitySet * dst_city,
		const Wcl_CitySet * src_city);

#endif				/* __DEF_WORLDCLOCK_DATA_H_ */
