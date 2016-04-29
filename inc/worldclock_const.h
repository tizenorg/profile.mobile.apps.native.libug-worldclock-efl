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

#ifndef __DEF_WORLDCLOCK_CONST_H_
#define __DEF_WORLDCLOCK_CONST_H_

#include <limits.h>

#ifndef _
#define _(s)  (dgettext(PACKAGE, s))
#endif
#ifndef _EDJ
#define _EDJ(o)         (elm_layout_edje_get(o))
#endif

#define BUF_SIZE                256
#define BUF_MIN_SIZE            32
#define BUF_LARGE_SIZE          512
#define BUF_PATH                PATH_MAX

#define CITY_BUF_SIZE           128
#define COUNTRY_BUF_SIZE        128
#define TIMEZONE_BUF_SIZE       32
#define TZPATH_BUF_SIZE         128
#define FLAG_BUF_SIZE         128
#define MCC_BUF_SIZE	20

#define PACKAGE                 "libug-worldclock-efl"

/* group name */
#define GRP_ADD                "ug_add_layout"

#ifndef SUCCESS
#define SUCCESS 0
#endif
#ifndef FAILED
#define FAILED -1
#endif

#endif				/* __DEF_WORLDCLOCK_CONST_H_ */
