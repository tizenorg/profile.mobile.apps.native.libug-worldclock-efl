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

#ifndef __DEF_WORLDCLOCK_DLOG_H__
#define __DEF_WORLDCLOCK_DLOG_H__

#include "clock_fwk_define.h"

//tag
#ifdef  LOG_TAG
#undef  LOG_TAG
#endif
#define LOG_TAG "WORLDCLOCK_UG"

//
#define GOTO_ERROR_IF(expr) \
    ({do { \
        if ((expr)) { \
            CLK_ERR("(%s) goto error", #expr); \
            goto error; \
        } \
    } while (0);})

#endif				// __DEF_WORLDCLOCK_DLOG_H__
