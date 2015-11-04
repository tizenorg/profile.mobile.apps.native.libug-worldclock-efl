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


#define __CLK_FWK_ICU_C__
#include "clock_fwk_icu.h"
#include <vconf.h>
#include <unicode/udatpg.h>
#include <unicode/ucnv.h>
#include <unistd.h>
//
CLK_DATE_FORMAT clk_fwk_icu_date_format_get()
{
	CLK_DATE_FORMAT dateStyle = CLK_DATE_FORMAT_DD_MM_YYYY;
	int value = 0;
	int ret = -1;
	ret = vconf_get_int(VCONFKEY_SETAPPL_DATE_FORMAT_INT, &value);
	if (-1 == ret) {
		value = SETTING_DATE_FORMAT_DD_MM_YYYY;
	}
	switch (value) {
	case SETTING_DATE_FORMAT_DD_MM_YYYY:
		dateStyle = CLK_DATE_FORMAT_DD_MM_YYYY;
		break;
	case SETTING_DATE_FORMAT_MM_DD_YYYY:
		dateStyle = CLK_DATE_FORMAT_MM_DD_YYYY;
		break;
	case SETTING_DATE_FORMAT_YYYY_MM_DD:
		dateStyle = CLK_DATE_FORMAT_YYYY_MM_DD;
		break;
	case SETTING_DATE_FORMAT_YYYY_DD_MM:
		dateStyle = CLK_DATE_FORMAT_YYYY_DD_MM;
		break;
	default:
		break;
	}
	return dateStyle;
}

//
CLK_TIME_FORMAT clk_fwk_icu_time_format_get()
{
	CLK_TIME_FORMAT time_format = CLK_TIME_FORMAT_12HOUR;
	int ret = 0;
	int value = 0;
	ret = vconf_get_int(VCONFKEY_REGIONFORMAT_TIME1224, &value);
	// if failed, set default time format
	if (-1 == ret) {
		value = VCONFKEY_TIME_FORMAT_12;
	}
	switch (value) {
	case VCONFKEY_TIME_FORMAT_12:
		time_format = CLK_TIME_FORMAT_12HOUR;
		break;
	case VCONFKEY_TIME_FORMAT_24:
		time_format = CLK_TIME_FORMAT_24HOUR;
		break;
	default:
		break;
	}
	return time_format;
}

int clk_timezone_get(char *dst, size_t number)
{
	retv_if(!dst || number < 1, FAILED);
	UChar timezoneID[MAX_TIMEZONE_LENGHT] = { 0 };
	UErrorCode status = U_ZERO_ERROR;
	ucal_getDefaultTimeZone(timezoneID, MAX_TIMEZONE_LENGHT, &status);
	retv_if(status != SUCCESS, FAILED);

	dst = u_austrncpy(dst, timezoneID, number);
	CLK_INFO_PURPLE("default timezone is %s: ", dst);
	return SUCCESS;
}

void clk_timezone_update()
{
	UErrorCode status = U_ZERO_ERROR;
	UChar uOlsonID[BUF_SIZE_64] = { 0 };

	char *timezone = vconf_get_str(VCONFKEY_SETAPPL_TIMEZONE_ID);
	ret_if(!timezone);
	CLK_INFO_PURPLE("current timezone is :  %s", timezone);
	u_uastrcpy(uOlsonID, timezone);
	ucal_setDefaultTimeZone(uOlsonID, &status);

	free(timezone);
	retm_if(U_FAILURE(status), "error value is %s", u_errorName(status));
}
