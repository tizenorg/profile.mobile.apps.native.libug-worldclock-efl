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
#include "worldclock_fwk_icu.h"
#include "worldclock_types.h"
#include "worldclock_dlog.h"

char *time_pattern[2] = {
	"hh:mm a",
	"HH:mm",
};

int worldclock_icu_dst_get(const char *tz_path)
{
	retv_if(!tz_path, -1);
	int dst = 0;
	UErrorCode status = U_ZERO_ERROR;

	UChar utf16_timezone[64] = { 0 };
	if (tz_path) {
		u_uastrncpy(utf16_timezone, tz_path, 64);
	}
	UCalendar *cal =
			ucal_open(utf16_timezone, u_strlen(utf16_timezone), uloc_getDefault(),
					UCAL_TRADITIONAL, &status);
	retv_if(!cal, -1);

	UBool is_Dst = ucal_inDaylightTime(cal, &status);
	retvm_if(U_FAILURE(status), -1, "error value is %s", u_errorName(status));
	if (is_Dst) {
		dst = 1;
	}
	return dst;
}
