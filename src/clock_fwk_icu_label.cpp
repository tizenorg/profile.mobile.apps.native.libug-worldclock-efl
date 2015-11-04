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

#include "clock_fwk_icu_label.h"
#include <unicode/ustring.h>

#include <unicode/ucol.h>

#include <unicode/alphaindex.h>
#include <unicode/locid.h>
#include <unicode/bytestream.h>
#include <unicode/utypes.h>
#include <unicode/putil.h>
#include <unicode/uiter.h>

char *g_alphabetic_index[128] = { NULL };

static AlphabeticIndex *Alph_index = NULL;

void init_alphabetic_index(char *lang)
{
	int32_t i = 0;
	UErrorCode status = U_ZERO_ERROR;
	//Locale locale(getenv("LANG"));
	Locale locale(lang);
	AlphabeticIndex *index = new AlphabeticIndex(locale, status);
	if (NULL == index) {
		return;
	}
	for (i = 0; index->nextBucket(status); i++) {
		UnicodeString label = index->getBucketLabel();
		UAlphabeticIndexLabelType type = index->getBucketLabelType();
		if ((type != U_ALPHAINDEX_UNDERFLOW) && (type != U_ALPHAINDEX_OVERFLOW)) {
			int len = 0;
			int len_str = 0;
			int len_utf8 = 0;
			char *str = NULL;
			UErrorCode status = U_ZERO_ERROR;
			const UChar *unichars = label.getBuffer();
			if (unichars != NULL) {
				len = u_strlen(unichars);
				len_str = sizeof(char) * 4 * (len + 1);
				str = (char *)calloc(1, len_str);
				u_strToUTF8(str, len_str, &len_utf8, unichars, len, &status);
			}

			if (NULL == g_alphabetic_index[i]) {
				g_alphabetic_index[i] = str;
			} else {
				free(g_alphabetic_index[i]);
				g_alphabetic_index[i] = str;
			}

		} else {
			free(g_alphabetic_index[i]);
			g_alphabetic_index[i] = NULL;
		}
	}
	delete index;
}

void uninit_alphabetic_index()
{
	int i = 0;
	while (g_alphabetic_index[i] != NULL) {
		free(g_alphabetic_index[i]);
		g_alphabetic_index[i] = NULL;
		i++;
	}
	if (NULL != Alph_index) {
		delete Alph_index;
		Alph_index = NULL;
	}
}

int32_t get_alphabetic_index(char *cityname)
{
	int32_t alphabetic_index = 0;
	UErrorCode status = U_ZERO_ERROR;
	Locale locale(getenv("LANG"));
	if (Alph_index == NULL) {
		Alph_index = new AlphabeticIndex(locale, status);
	}
	if (NULL == Alph_index) {
		return 0;
	}
	alphabetic_index = Alph_index->getBucketIndex(UnicodeString(cityname), status);
	return alphabetic_index;
}

char *get_alphabetic_index_name(int32_t alphabetic_index)
{
	return g_alphabetic_index[alphabetic_index];
}
