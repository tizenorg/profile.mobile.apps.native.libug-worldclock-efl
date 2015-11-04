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

#ifndef __DEF_WORLDCLOCK_TYPES_H__
#define __DEF_WORLDCLOCK_TYPES_H__

#include <Elementary.h>
#include "worldclock_const.h"

typedef void (*Wcl_Return_Cb) (void *, Eina_Bool);

typedef enum {
	WCL_CALLER_IS_APP_IT_SELF = 0,
	WCL_CALLER_IS_SHARED_LIBRARY,
	WCL_CALLER_IS_UI_GADGET,
	WCL_CALLER_IS_LIVEBOX,
	WCL_CALLER_IS_UNKNOW,
} Wcl_Caller;

#ifdef FEATURE_SORT_ORDER
typedef enum {
	WCL_SORT_BY_NAME = 1,
	WCL_SORT_BY_TIMEZONE,

	WCL_UNKNOW_SORT_TYPE,
} Wcl_Addlist_Sort_Type;
#endif

typedef enum {
	WCL_SELECT_IN_UNSELECTED = 0,
	WCL_SELECT_IN_ALL,
	WCL_SELECT_IF_HAS_TZPATH,

	WCL_SELECTION_UNKNOW_FLAG,
} Wcl_Selection_Flag;

typedef enum {
	WCL_SEARCH_BY_ALL_KEYWORDS = 0,
	WCL_SEARCH_BY_CITY_NAME,
	WCL_SEARCH_BY_COUNTRY_NAME,
	WCL_SEARCH_BY_TIMEZONE,

	WCL_UNKNOWN_SEARCH_TYPE,
} Wcl_Search_Type;

typedef enum {
	WCL_DST_OFF = -1,
	WCL_DST_AUTO = 0,
	WCL_DST_1_HOUR,
	WCL_DST_2_HOURS,

	WCL_DST_TYPES_COUNT,

	WCL_DST_UNKNOWN_TYPE,
} Wcl_DST_Type;

typedef struct _cs {		/* city set */
	int index;		/* record index in db */
	char city[CITY_BUF_SIZE];	/* city name */
	char country[COUNTRY_BUF_SIZE];	/* country name */
	char timezone[TIMEZONE_BUF_SIZE];	/* timezone */
	int dst_type;		/* dst type */
	int dst_enabled;	/* dst disable flag */
	int selected;		/* selected or not */
	int sequence;		/* sequence number in mainlist */
	char tz_path[TZPATH_BUF_SIZE];	/* TZ path of city */

	int dst;		/* dst value */
	volatile time_t now_time;	/* the current time of city */

	char flag[COUNTRY_BUF_SIZE];	/* flag image name */
	char mcc[MCC_BUF_SIZE];
} Wcl_CitySet;

#endif				// __DEF_WORLDCLOCK_TYPES_H__
