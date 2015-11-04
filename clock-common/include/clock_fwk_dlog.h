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


#ifndef __CLK_FWK_DLOG_H__
#define __CLK_FWK_DLOG_H__

#include <stdio.h>
#include <assert.h>
#include <dlog.h>
/**********************************************************************
******************define, struct ,typedef, union, enum, global val *************************************
***********************************************************************/
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
//#define CLK_INFO(fmt, arg...) printf("[%s:%d] "fmt"\n",__FILE__, __LINE__,##arg);
//#define CLK_ERR(fmt,arg...) printf(FONT_COLOR_RED"[%s:%d] "fmt FONT_COLOR_RESET"\n",__FILE__, __LINE__,##arg);

#define CLK_FUN_DEBUG_BEG()	//CLK_INFO("====>>>>fun=%s, BEGIN====>>>>",__FUNCTION__);
#define CLK_FUN_DEBUG_END()	//CLK_INFO("====>>>>fun=%s, END====>>>>",  __FUNCTION__);
#define CLK_DEBUG_INFO(fmt, arg...)	//CLK_INFO(fmt, ##arg)
#define CLK_DEBUG_INFO(fmt, arg...)	//CLK_ERR(fmt,##arg)

#define CLK_RET_IF(expr) \
    ({do { \
            if(expr) { \
                    nErr = FAILED;\
                    CLK_INFO_RED("!!!!!!!!!!!!!!!!!!!! CHECK nErr=%d!!!!!!! \n(%s)return!!!!!!!!!!!\n\n",nErr, #expr); \
                    goto End; \
                }\
    }while(0);})
#define CLK_RETV_IF(expr, val) \
        ({do { \
                if(expr) { \
                    ret = val;\
                    nErr = FAILED;\
                    CLK_INFO_RED("!!!!!!!!!!!!!!!!!!!! CHECK nErr=%d!!!!!!! \n(%s)return!!!!!!!!!!!\n\n",nErr, #expr); \
                    goto End; \
                } \
        }while(0);})
#define CLK_RETE_IF(expr, errorId) \
    ({do{ \
                if(expr) { \
                    nErr = errorId;\
                    CLK_INFO_RED("!!!!!!!!!!!!!!!!!!!! CHECK nErr=%d!!!!!!! \n(%s)return!!!!!!!!!!!\n\n",nErr, #expr); \
                    goto End; \
        } \
    }while(0);})
#define CLK_RETEM_IF(expr, errorId,fmt, arg...) \
    ({do{ \
                if(expr) { \
                    nErr = errorId;\
                CLK_INFO_RED(fmt, ##arg); \
                    CLK_INFO_RED("!!!!!!!!!!!!!!!!!!!! CHECK nErr=%d!!!!!!! \n(%s)return!!!!!!!!!!!\n\n",nErr, #expr); \
                    goto End; \
                } \
    }while(0);})
#define CLK_RETVE_IF(expr, val, errorId) \
    ({do{ \
                if(expr) { \
                    ret = val;\
                    nErr = errorId;\
                    CLK_INFO_RED("!!!!!!!!!!!!!!!!!!!! CHECK nErr=%d!!!!!!! \n(%s)return!!!!!!!!!!!\n\n",nErr, #expr); \
                    goto End; \
                } \
    }while(0);})
#define CLK_RETM_IF(expr, fmt, arg...) \
    ({do { \
                if(expr) { \
                    nErr = FAILED;\
                    CLK_INFO_RED(fmt, ##arg); \
                    CLK_INFO_RED("!!!!!!!!!!!!!!!!!!!! CHECK nErr=%d!!!!!!! \n(%s)return!!!!!!!!!!!\n\n",nErr, #expr); \
                    goto End; \
                } \
    }while(0);})
#define CLK_RETVM_IF(expr, val, fmt, arg...) \
    ({do { \
                if(expr) { \
                    ret = val;\
                    nErr = FAILED;\
                    CLK_INFO_RED(fmt, ##arg); \
                    CLK_ERR("!!!!!!!!!!!!!!!!!!!! CHECK nErr=%d!!!!!!! \n(%s)return!!!!!!!!!!!\n\n",nErr, #expr); \
                    goto End; \
                } \
    }while(0);})
#define CLK_RETVME_IF(expr, val, errorId, fmt, arg...) \
   ({do{ \
                if(expr) { \
                    ret = val;\
                    nErr = errorId;\
                    CLK_INFO_RED(fmt, ##arg); \
                    CLK_INFO_RED("!!!!!!!!!!!!!!!!!!!! CHECK nErr=%d!!!!!!! \n(%s)return!!!!!!!!!!!\n\n",nErr, #expr); \
                    goto End; \
                } \
    }while(0);})
#define CLK_GOTO_IF_RUN(con, pos, expr, fmt, arg...)\
   ({ do{\
            if(con) {\
                    expr;\
                    CLK_INFO_RED(fmt, ##arg); \
                    goto pos;\
            }\
    }while (0);})

/* anci c color type */
#define FONT_COLOR_RESET    "\033[0m"
#define FONT_COLOR_RED      "\033[31m"
#define FONT_COLOR_GREEN    "\033[32m"
#define FONT_COLOR_YELLOW   "\033[33m"
#define FONT_COLOR_BLUE     "\033[34m"
#define FONT_COLOR_PURPLE   "\033[35m"
#define FONT_COLOR_CYAN     "\033[36m"
#define FONT_COLOR_GRAY     "\033[37m"

#define CLK_INFO_RED(fmt, arg...) (CLK_INFO(FONT_COLOR_RED fmt FONT_COLOR_RESET, ##arg))
#define CLK_INFO_GREEN(fmt, arg...) (CLK_INFO(FONT_COLOR_GREEN fmt FONT_COLOR_RESET, ##arg))
#define CLK_INFO_YELLOW(fmt, arg...) (CLK_INFO(FONT_COLOR_YELLOW fmt FONT_COLOR_RESET, ##arg))
#define CLK_INFO_BLUE(fmt, arg...) (CLK_INFO(FONT_COLOR_BLUE fmt FONT_COLOR_RESET, ##arg))
#define CLK_INFO_PURPLE(fmt, arg...) (CLK_INFO(FONT_COLOR_PURPLE fmt FONT_COLOR_RESET, ##arg))
#define CLK_INFO_CYAN(fmt, arg...) (CLK_INFO(FONT_COLOR_CYAN fmt FONT_COLOR_RESET, ##arg))
#define CLK_INFO_GRAY(fmt, arg...) (CLK_INFO(FONT_COLOR_GRAY fmt FONT_COLOR_RESET, ##arg))

#define CLK_INFO_WITH_COLOR(color, fmt, arg...) (CLK_INFO(color fmt FONT_COLOR_RESET, ##arg))

//
#define warn_if(expr, fmt, arg...) ({do { \
        if(expr) { \
            CLK_INFO("(%s) -> "fmt, #expr, ##arg); \
        } \
    } while (0);})
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
#endif				//__CLK_FWK_DLOG_H__
