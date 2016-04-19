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

#include <dlog.h>
#include "ug_worldclock_efl.h"

//tag
#ifdef  LOG_TAG
#undef  LOG_TAG
#endif
#define LOG_TAG "WORLDCLOCK_UG"

#define FUNCTION_LOG ENABLE
#ifdef FUNCTION_LOG
#define CLK_FUN_BEG()   CLK_INFO("====>>>>fun=%s, BEGIN====>>>>", __FUNCTION__);
#define CLK_FUN_END()   CLK_INFO("====>>>>fun=%s, END====>>>>",  __FUNCTION__);
#else
#define CLK_FUN_BEG()
#define CLK_FUN_END()
#endif
#define CLK_INFO(fmt, arg...) (LOGD("[%s:%d] "fmt,__FILE__, __LINE__, ##arg))
#define CLK_SINFO(fmt, arg...) (SECURE_LOGD("[%s:%d] "fmt,__FILE__, __LINE__, ##arg))
#define CLK_ERR(fmt,arg...) (LOGE(FONT_COLOR_RED"[%s:%d] "fmt FONT_COLOR_RESET, __FILE__,  __LINE__, ##arg))

#define CLK_FUN_DEBUG_BEG()	//CLK_INFO("====>>>>fun=%s, BEGIN====>>>>",__FUNCTION__);
#define CLK_FUN_DEBUG_END()	//CLK_INFO("====>>>>fun=%s, END====>>>>",  __FUNCTION__);
#define CLK_DEBUG_INFO(fmt, arg...)	//CLK_INFO(fmt, ##arg)
#define CLK_DEBUG_INFO(fmt, arg...)	//CLK_ERR(fmt,##arg)

/* anci c color type */
#define FONT_COLOR_RESET    "\033[0m"
#define FONT_COLOR_RED      "\033[31m"
#define FONT_COLOR_GREEN    "\033[32m"
#define FONT_COLOR_YELLOW   "\033[33m"
#define FONT_COLOR_CYAN     "\033[36m"

#define CLK_INFO_RED(fmt, arg...) (CLK_INFO(FONT_COLOR_RED fmt FONT_COLOR_RESET, ##arg))
#define CLK_INFO_GREEN(fmt, arg...) (CLK_INFO(FONT_COLOR_GREEN fmt FONT_COLOR_RESET, ##arg))
#define CLK_INFO_YELLOW(fmt, arg...) (CLK_INFO(FONT_COLOR_YELLOW fmt FONT_COLOR_RESET, ##arg))
#define CLK_INFO_CYAN(fmt, arg...) (CLK_INFO(FONT_COLOR_CYAN fmt FONT_COLOR_RESET, ##arg))

#define ret_if(expr) ({do { \
        if(expr) { \
            CLK_ERR("(%s) -> %s() return", #expr, __FUNCTION__); \
            return; \
        } \
    } while (0);})
#define retv_if(expr, val) ({do { \
        if(expr) { \
            CLK_ERR("(%s) -> %s() return", #expr, __FUNCTION__); \
            return (val); \
        } \
    } while (0);})
#define retm_if(expr, fmt, arg...) ({do { \
        if(expr) { \
            CLK_ERR(fmt, ##arg); \
            CLK_ERR("(%s) -> %s() return", #expr, __FUNCTION__); \
            return; \
        } \
    } while (0);})
#define retvm_if(expr, val, fmt, arg...) ({do { \
        if(expr) { \
            CLK_ERR(fmt, ##arg); \
            CLK_ERR("(%s) -> %s() return", #expr, __FUNCTION__); \
            return (val); \
        } \
    } while (0);})

#define GOTO_ERROR_IF(expr) \
    ({do { \
        if ((expr)) { \
            CLK_ERR("(%s) goto error", #expr); \
            goto error; \
        } \
    } while (0);})

#endif				// __DEF_WORLDCLOCK_DLOG_H__
