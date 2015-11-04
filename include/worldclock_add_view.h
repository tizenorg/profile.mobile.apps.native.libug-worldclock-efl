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

#ifndef __DEF_WORLDCLOCK_ADDVIEW_H_
#define __DEF_WORLDCLOCK_ADDVIEW_H_

#include <Elementary.h>
#include "worldclock.h"

/**
 * Create add view for append new city into the selection list of worldclock
 *
 * @param[in]  parent    Evas_Object which is the parent of add view
 * @param[in]  data      data which used in this function
 * @param[in]  func      Callback function which used for invoking when exit from add view
 *
 * @return    FAILED if create add view failed.
 *            SUCCESS if create add view successfully.
 */
int worldclock_ugview_add(Evas_Object * parent, void *data, Wcl_Return_Cb func);

/**
 * Update worldclock view
 *
 * @param[in]  data   data used for this function
 *
 * @return    FAILED if update add view failed.
 *            SUCCESS if update add view successfully.
 */
int worldclock_ugview_update(void *data);

/**
 * Release all resources which used in add view when exit from add view
 *
 * @param[in]  data   data which used in this function
 *
 * @return
 */
void worldclock_ugview_free(void *data);

#endif				/* __DEF_WORLDCLOCK_ADDVIEW_H_ */
