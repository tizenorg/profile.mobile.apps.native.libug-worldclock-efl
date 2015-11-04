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

#ifndef __WCL_FWK_ICU_H__
#define __WCL_FWK_ICU_H__
#include "clock_fwk_icu.h"

/*BEG: Gong Xingsheng<x536.gong@samsung.com> add in Mon Nov 21 01:04:37 PST 2011
 * this struct is saved hour, min, sec and current time format
 * which is used in genlist item date
 */

typedef struct _worldclock_time_s worldclock_time_s;
struct _worldclock_time_s {
	int hour;
	int min;
	int sec;
	bool bAm;
};
/*END: Gong Xingsheng<x536.gong@samsung.com> add in Mon Nov 21 01:04:37 PST 2011 */

int worldclock_icu_dst_get(const char *tz_path);

#endif				//__WCL_FWK_ICU_H__
