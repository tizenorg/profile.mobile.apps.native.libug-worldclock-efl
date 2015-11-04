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

#include <taf.h>
#include "worldclock_const.h"

static Evas_Object *beeper = NULL;

TAF_API Evas_Object *taf_layout_create(Evas_Object * parent, const char *file,
		const char *group)
{
	Evas_Object *layout = elm_layout_add(parent);
	if (elm_layout_file_set(layout, file, group)) {
		evas_object_show(layout);
	} else {
		evas_object_del(layout);
		layout = NULL;
	}
	return layout;
}

TAF_API Evas_Object *taf_swallowed_layout_create(Evas_Object * parent, const char *part,
		const char *file, const char *group)
{
	Evas_Object *layout = taf_layout_create(parent, file, group);
	if (layout != NULL) {
		elm_object_part_content_set(parent, part, layout);
	}
	return layout;
}

TAF_API void taf_init(Evas_Object * parent)
{
	if (beeper == NULL) {
		beeper = taf_layout_create(parent, WCL_EDJ_PATH, "beeper");
		evas_object_move(beeper, -1000, -1000);
	}
}

TAF_API void taf_beep()
{
	if (beeper != NULL) {
		elm_object_signal_emit(beeper, "beep", "elm");
	}
}

TAF_API void taf_destroy()
{
	if (beeper != NULL) {
		evas_object_del(beeper);
	}
}

