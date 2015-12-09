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

#ifndef __CLK_FWK_DEFINE_H__
#define __CLK_FWK_DEFINE_H__

#include "clock_fwk_dlog.h"
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <libintl.h>
#include <time.h>
//tag
#ifdef  LOG_TAG
#undef  LOG_TAG
#endif
#define LOG_TAG "CLK_COM"
//get str
#ifdef _
#undef _
#endif
#define _(s)  (dgettext(PACKAGE, s))
//equal
#define IS_EQUAL(a, b) ((a) == (b))
#define NULL_CHECK(p) (IS_EQUAL(p, NULL))
#define NOT_NULL(p) (!NULL_CHECK(p))
#define IS_STR_EQUAL(a, b) (!strcmp(a, b))
#define IS_STR_NOT_EQUAL(a, b) (strcmp(a, b))
#define ASSERT(p) (assert(p))
//
#define CALLOC(num, type) ((type *)calloc(num, sizeof(type)))
//******************define:free *************************************
#define FREEIF(p) ({if (p) {free(p); p = NULL; }})
#define FREERIF_FN(p, fn) ({if (p) {fn(p); p = NULL; }})
#define EVAS_OBJECT_DELIF(p) ({if (p) {evas_object_del(p); p = NULL; }})
#define ELM_GENLIST_ITEM_DELIF(p) ({if (p) {elm_object_item_del(p); p = NULL; }})
#define EVAS_OBJECT_SHOWIF(p) ({if (p) {evas_object_show(p); })
#define ECORE_TIMER_DELIF(p) ({if (p) {ecore_timer_del(p); p = NULL; }})
#define ECORE_FILE_MONITOR_DELIF(p) ({if (p) {ecore_file_monitor_del(p); p = NULL; }})
#define ECORE_EVENT_HANDLER_DELIF(p) ({if (p) {ecore_event_handler_del(p); p = NULL; }})
#define ELM_SLIDER_VALUE_GET(p) (elm_slider_value_get(p)+0.5)
//edj
#ifndef _EDJ
#define _EDJ(obj) (elm_layout_edje_get(obj))
#endif
//******************set time *************************************
#define SET_TIME_DATA_T(pTime_data, YEAR, MONTH, DAY, HOUR, MIN, SEC) \
({do {\
    (pTime_data)->year = (YEAR);\
    (pTime_data)->month = (MONTH);\
    (pTime_data)->day = (DAY);\
    (pTime_data)->hour = (HOUR);\
    (pTime_data)->min = (MIN);\
    (pTime_data)->sec = (SEC);\
} while (0);\
})

//count
#define BUF_SIZE_32 (32)
#define BUF_SIZE_64 (64)
#define BUF_SIZE_128 (128)
#define BUF_SIZE_256 (256)
//
#ifndef SUCCESS
#define SUCCESS 0
#endif
#ifndef FAILED
#define FAILED -1
#endif
//
struct _tc {			/* text class */
	char *name;		/* text class name */
	int size;		/* font size */
};
//
struct _ts {			/* text set */
	char *name;		/* part name */
	char *msgid;		/* message ID */
};
//
struct text_part {
	char *part;
	char *msgid;
};
//
typedef struct _clock_color_class {
	const char *name;
	int r, g, b, a;
	int r2, g2, b2, a2;
	int r3, g3, b3, a3;
} clock_color_class;

#endif				//__CLK_FWK_DEFINE_H__
