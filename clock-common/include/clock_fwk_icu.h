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


#ifndef __CLK_FWK_ICU_H__
#define __CLK_FWK_ICU_H__

#include "clock_fwk_define.h"
#include <unicode/uloc.h>
#include <unicode/udat.h>
#include <unicode/ustring.h>
#include <time.h>

#define MAX_PATTERN_LENGHT  (32)
#define MAX_TIMEZONE_LENGHT (64)

typedef enum {
	CLK_DATE_FORMAT_DD_MM_YYYY = 0,
	CLK_DATE_FORMAT_MM_DD_YYYY,
	CLK_DATE_FORMAT_YYYY_MM_DD,
	CLK_DATE_FORMAT_YYYY_DD_MM,
} CLK_DATE_FORMAT;

typedef enum {
	CLK_TIME_FORMAT_12HOUR = 0,
	CLK_TIME_FORMAT_24HOUR,
} CLK_TIME_FORMAT;

CLK_DATE_FORMAT clk_fwk_icu_date_format_get();
CLK_TIME_FORMAT clk_fwk_icu_time_format_get();

int clk_fwk_icu_get_data(time_t time, char *dstStr, int dstStr_num, char *loc,
		char *pattern, int pattern_num, const char *timezone);
int clk_timezone_get(char *dst, size_t number);
void clk_timezone_update();

#endif				//__CLK_FWK_ICU_H__
