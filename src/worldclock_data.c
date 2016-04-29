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

#include <stdio.h>
#include <Elementary.h>
#include <sqlite3.h>
#include <fcntl.h>

#include <vconf.h>

#include "worldclock.h"
#include "worldclock_data.h"
#include "worldclock_dlog.h"
#include "worldclock_types.h"
#include "worldclock_util.h"

//TODO: remove hardcode when correct path will be acquired
#define WORLDCLOCK_DB               "worldclock.db"
#define WORLDCLOCK_DB_QUERY_LEN     512
#define WORLDCLOCK_DB_TABLE_CITY    "city_table"

#define WORLDCLOCK_SQLITE_SCHEMA "create table '%q' (\
			idex int primary key,\
				 city varchar(%d), \
				 country varchar(%d), \
				 timezone varchar(%d), \
				 dst_type int, \
				 dst_enabled int, \
				 selected int, \
				 sequence int, \
				 tz_path varchar(%d) \
				 flag varchar(%d) \
				 mcc varchar(%d) \
			);"

typedef struct _add_cs {
	char *city;
	char *country;
	char *timezone;
	char *tz_path;
} Add_CS;

static sqlite3 *g_hDB = NULL;
static Add_CS g_cs_main[] = {
#include "worldclock_timezone.h"
};

/**
 * Extract data from sqlite3 db
 * index, city, country, timezone, dst_type, selected, sequence, tz_path
 *
 * @param[in]  p_record  Pointer to such city information structure,
 *                       which used for store the data,
 *                       which extracted from sqlite3 db
 *
 * @return
 */
static void __data_extract_record(Wcl_CitySet * p_record, sqlite3_stmt * p_sqStmt)
{
	char *pCity = NULL;
	char *pCountry = NULL;
	char *pTimezone = NULL;
	char *pTzPath = NULL;
	char *pFlag = NULL;
	char *pMCC = NULL;

	/*  Parameter Check */
	ret_if(!p_sqStmt);
	ret_if(!p_record);

	/* Extract record */
	// 1. index
	p_record->index = sqlite3_column_int(p_sqStmt, 0);

	// 2. city
	pCity = (char *)sqlite3_column_text(p_sqStmt, 1);
	snprintf(p_record->city, CITY_BUF_SIZE, "%s", pCity);

	// 3. country
	pCountry = (char *)sqlite3_column_text(p_sqStmt, 2);
	snprintf(p_record->country, COUNTRY_BUF_SIZE, "%s", pCountry);

	// 4. timezone
	pTimezone = (char *)sqlite3_column_text(p_sqStmt, 3);
	snprintf(p_record->timezone, TIMEZONE_BUF_SIZE, "%s", pTimezone);

	// 5. dst_type
	p_record->dst_type = sqlite3_column_int(p_sqStmt, 4);

	// 6. dst_enabled
	p_record->dst_enabled = sqlite3_column_int(p_sqStmt, 5);

	// 7. selected
	p_record->selected = sqlite3_column_int(p_sqStmt, 6);

	// 8. sequence
	p_record->sequence = sqlite3_column_int(p_sqStmt, 7);

	// 9. tz_path
	pTzPath = (char *)sqlite3_column_text(p_sqStmt, 8);
	snprintf(p_record->tz_path, TZPATH_BUF_SIZE, "%s", pTzPath);

	// 10. flag
	pFlag = (char *)sqlite3_column_text(p_sqStmt, 9);
	snprintf(p_record->flag, FLAG_BUF_SIZE, "%s", pFlag);

	// 11. mcc
	pMCC = (char *)sqlite3_column_text(p_sqStmt, 10);
	snprintf(p_record->mcc, MCC_BUF_SIZE, "%s", pMCC);

	// 12. nowtime
	p_record->now_time = 0;
}

/**
 * Append given record into sqlite3 db
 *
 * @param[in]  p_record   Pointer to such city data which need to be append
 * @param[in]  index      The index of place which you want to instert the data in.
 *
 * @return     EINA_FALSE if append data failed.
 *             EINA_TRUE if append successfully.
 */
inline static Eina_Bool __data_add_record(Add_CS * p_record, int index)
{
	CLK_FUN_DEBUG_BEG();
	int rc = 0;
	int selected = 0;
	char szQuery[WORLDCLOCK_DB_QUERY_LEN] = { 0, };
	sqlite3_stmt *p_sqStmt = NULL;

	retv_if(!p_record, EINA_FALSE);

	if (!strcmp(p_record->city, "Seoul")
			&& !strcmp(p_record->timezone, "GMT+9")) {
		// set Seoul as default selected city
		selected = 1;
	}

	CLK_DEBUG_INFO("\n\tcity: %s\tcountry: %s\ttimezone: %s\ttz_path: %s\n",
			p_record->city, p_record->country, p_record->timezone, p_record->tz_path);
	//  Set SQL Statement
	snprintf(szQuery, WORLDCLOCK_DB_QUERY_LEN,
			"insert into %s (idex, city, country, timezone, dst_type, dst_enabled, selected, sequence, tz_path)\
            values(%d,?,?,?,%d,%d,%d,%d,?);", WORLDCLOCK_DB_TABLE_CITY,
			index,
			//p_record->city,
			//p_record->country,
			//p_record->timezone,
			0,		//p_record->dst_type,
			1,		//p_record->dst_enabled,
			selected,	//p_record->selected,
			0		//p_record->sequence
			//p_record->tz_path
		);
	rc = sqlite3_prepare_v2(g_hDB, szQuery, strlen(szQuery), &p_sqStmt, NULL);
	GOTO_ERROR_IF(SQLITE_OK != rc);

	//  Bind Text Field
	rc = sqlite3_bind_text(p_sqStmt, 1, p_record->city,
			strlen(p_record->city), SQLITE_STATIC);
	GOTO_ERROR_IF(SQLITE_OK != rc);
	rc = sqlite3_bind_text(p_sqStmt, 2, p_record->country,
			strlen(p_record->country), SQLITE_STATIC);
	GOTO_ERROR_IF(SQLITE_OK != rc);
	rc = sqlite3_bind_text(p_sqStmt, 3, p_record->timezone,
			strlen(p_record->timezone), SQLITE_STATIC);
	GOTO_ERROR_IF(SQLITE_OK != rc);
	rc = sqlite3_bind_text(p_sqStmt, 4, p_record->tz_path,
			strlen(p_record->tz_path), SQLITE_STATIC);
	GOTO_ERROR_IF(SQLITE_OK != rc);

	//  Execute
	rc = sqlite3_step(p_sqStmt);
	GOTO_ERROR_IF(SQLITE_DONE != rc);

	//  Finalize
	rc = sqlite3_finalize(p_sqStmt);
	GOTO_ERROR_IF(SQLITE_OK != rc);
	p_sqStmt = NULL;
	CLK_FUN_DEBUG_END();
	return EINA_TRUE;

error:
	CLK_ERR("\n[ERROR] szQuery : %s", szQuery);
	CLK_ERR("\n[ERROR] SQL error msg : %s", (char *)sqlite3_errmsg(g_hDB));
	if (p_sqStmt) {
		sqlite3_finalize(p_sqStmt);
		p_sqStmt = NULL;
	}
	return EINA_FALSE;
}

/**
 * Append data of all cities list in the top of city list into db file
 *
 * @return     EINA_FALSE if append failed.
 *             EINA_TRUE if append successfully.
 */
static Eina_Bool __data_add_all_country()
{
	CLK_FUN_DEBUG_BEG();
	int i = 0;
	Eina_Bool ret = EINA_TRUE;

	// the last item in  g_cs_main[] must be NULL, so that this loop can stop
	while (g_cs_main[i].city) {
		// append record one by one
		ret = __data_add_record(&g_cs_main[i], i);
		i++;
		if (EINA_FALSE == ret) {
			ret = EINA_FALSE;
			break;
		}
	}

	CLK_FUN_DEBUG_END();
	return ret;
}

/**
 * Check whether such sqlite3 db file exist
 * If the db file exist, read it into memory.
 *
 * @param[in]  pszTableName  The path about sqlite3 db file
 *
 * @return     EINA_FALSE if file not exist
 *             EINA_TRUE if file exist.
 */
static Eina_Bool __data_is_table_exist(char *pszTableName)
{
	CLK_FUN_DEBUG_BEG();
	retv_if(!pszTableName, EINA_FALSE);

	int rc = 0;
	char *pszGetTableName = NULL;
	char szQuery[WORLDCLOCK_DB_QUERY_LEN] = { 0, };
	sqlite3_stmt *p_sqStmt = NULL;
	Eina_Bool is_success = EINA_FALSE;

	// set sqlite search text
	snprintf(szQuery, WORLDCLOCK_DB_QUERY_LEN,
			"SELECT name FROM sqlite_master WHERE name = '%s'", pszTableName);
	int ref = sqlite3_prepare_v2(g_hDB, szQuery, strlen(szQuery), &p_sqStmt,
			NULL);
	GOTO_ERROR_IF(SQLITE_OK != ref);
	CLK_DEBUG_INFO("\n[SUCCESS] szQuery : %s\n", szQuery);
	rc = sqlite3_step(p_sqStmt);
	while (SQLITE_ROW == rc) {
		pszGetTableName = (char *)sqlite3_column_text(p_sqStmt, 0);
		//CLK_DEBUG_INFO("pszGetTableName = %s", pszGetTableName);
		if (!strcmp(pszGetTableName, pszTableName)) {
			is_success = EINA_TRUE;
			break;
		}
		rc = sqlite3_step(p_sqStmt);
	}
	sqlite3_finalize(p_sqStmt);
	p_sqStmt = NULL;
	CLK_FUN_DEBUG_END();
	return is_success;

error:
	CLK_ERR("\n[ERROR] szQuery : %s", szQuery);
	CLK_ERR("\n[ERROR] SQL error msg : %s", (char *)sqlite3_errmsg(g_hDB));
	if (p_sqStmt) {
		sqlite3_finalize(p_sqStmt);
		p_sqStmt = NULL;
	}
	return is_success;
}

/**
 * Create new db file from default data list.
 *
 * @return     EINA_FALSE if create failed
 *             EINA_TRUE if create successfully.
 */
static Eina_Bool __data_create_country_table()
{
	CLK_FUN_DEBUG_BEG();
	char *pszErrorMsg = NULL;
	char *szQuery = NULL;
	Eina_Bool b_result = EINA_TRUE;

	// if tabel do not exist, create it
	if (EINA_FALSE == __data_is_table_exist(WORLDCLOCK_DB_TABLE_CITY)) {
		// create tabel
		szQuery =
				sqlite3_mprintf(WORLDCLOCK_SQLITE_SCHEMA,
						WORLDCLOCK_DB_TABLE_CITY, CITY_BUF_SIZE, COUNTRY_BUF_SIZE,
						TIMEZONE_BUF_SIZE, TZPATH_BUF_SIZE, FLAG_BUF_SIZE, MCC_BUF_SIZE);
		GOTO_ERROR_IF(SQLITE_OK != sqlite3_exec(g_hDB, szQuery, NULL, NULL,
				&pszErrorMsg));
		sqlite3_free(szQuery);
		// append all cities
		b_result = __data_add_all_country();
	}
	CLK_FUN_DEBUG_END();
	return b_result;

error:
	CLK_ERR("\n[ERROR] szQuery : %s", szQuery);
	CLK_ERR("\n[ERROR] error msg : %s\n", pszErrorMsg);
	sqlite3_free(pszErrorMsg);
	return EINA_FALSE;
}

/**
 * Get city list search city by given search text
 *
 * @param[in]   search_txt    given search text
 * @param[out]  p_recordList  Result list which used to store result list
 * @param[in]   selectFlag    selection flag which used for judge selection type
 *
 * @return    -1 if meet error
 *            count of the items in result list
 */
static int __data_get_city_list_by_search_text(const char *search_txt,
		Wcl_CitySet ** p_recordList, Wcl_Selection_Flag selectFlag)
{
	CLK_FUN_DEBUG_BEG();
	int rc = -1;
	int nRecCount = 0;
	Wcl_CitySet *p_record = NULL;
	sqlite3_stmt *p_sqStmt = NULL;
	char szQuery[WORLDCLOCK_DB_QUERY_LEN] = { 0, };

	char selFlag[BUF_SIZE] = { 0, };
	switch (selectFlag) {
	case WCL_SELECT_IN_ALL:
		break;
	case WCL_SELECT_IF_HAS_TZPATH:
		CLK_DEBUG_INFO
		("\n\tselect cities which has tz_path for Setting module!!!\n");
		// only find those cities which has tz_path.
		snprintf(selFlag, BUF_SIZE, " and tz_path<>''");
		break;
	case WCL_SELECT_IN_UNSELECTED:
	default:
		// only find those cities which is unselected.
		snprintf(selFlag, BUF_SIZE, " and selected=0");
		break;
	}
	// get item number
	snprintf(szQuery, WORLDCLOCK_DB_QUERY_LEN,
			"select count(idex) from %s where city like '%%%s%%' or country like '%%%s%%' or timezone like 'GMT%%%s%%'%s\
            order by city;", WORLDCLOCK_DB_TABLE_CITY, search_txt, search_txt, search_txt,
			selFlag);
	CLK_DEBUG_INFO("szQuery=%s\n", szQuery);
	rc = sqlite3_prepare_v2(g_hDB, szQuery, strlen(szQuery), &p_sqStmt, NULL);
	GOTO_ERROR_IF(SQLITE_OK != rc);
	rc = sqlite3_step(p_sqStmt);
	nRecCount = sqlite3_column_int(p_sqStmt, 0);
	rc = sqlite3_finalize(p_sqStmt);
	p_sqStmt = NULL;

	// read data from db
	*p_recordList = (Wcl_CitySet *) calloc(nRecCount, sizeof(Wcl_CitySet));
	GOTO_ERROR_IF(NULL == *p_recordList);
	p_record = *p_recordList;
	snprintf(szQuery, WORLDCLOCK_DB_QUERY_LEN,
			"select * from %s where city like '%%%s%%' or country like '%%%s%%' or timezone like 'GMT%%%s%%'%s order by city;",
			WORLDCLOCK_DB_TABLE_CITY, search_txt, search_txt, search_txt, selFlag);
	CLK_DEBUG_INFO("szQuery=%s\n", szQuery);
	rc = sqlite3_prepare_v2(g_hDB, szQuery, strlen(szQuery), &p_sqStmt, NULL);
	rc = sqlite3_step(p_sqStmt);
	while (rc == SQLITE_ROW) {
		__data_extract_record(p_record++, p_sqStmt);
		rc = sqlite3_step(p_sqStmt);
	}
	rc = sqlite3_finalize(p_sqStmt);
	GOTO_ERROR_IF(SQLITE_OK != rc);
	p_sqStmt = NULL;
	CLK_FUN_DEBUG_END();
	return nRecCount;

error:
	CLK_ERR("\n[ERROR] szQuery : %s", szQuery);
	CLK_ERR("\n[ERROR] SQL error msg : %s", (char *)sqlite3_errmsg(g_hDB));
	if (p_sqStmt) {
		sqlite3_finalize(p_sqStmt);
		p_sqStmt = NULL;
	}
	if (*p_recordList) {
		free(*p_recordList);
	}
	return -1;
}

/**
 * Get city list search city by given city name
 *
 * @param[in]   city_name     given city name
 * @param[out]  p_recordList  Result list which used to store result list
 * @param[in]   selectFlag    selection flag which used for judge selection type
 *
 * @return    -1 if meet error
 *            count of the items in result list
 */
static int __data_get_city_list_by_city_name(const char *city_name,
		Wcl_CitySet ** p_recordList, Wcl_Selection_Flag selectFlag)
{
	CLK_FUN_DEBUG_BEG();
	int rc = -1;
	int nRecCount = 0;
	Wcl_CitySet *p_record = NULL;
	sqlite3_stmt *p_sqStmt = NULL;
	char szQuery[WORLDCLOCK_DB_QUERY_LEN] = { 0, };

	char selFlag[BUF_SIZE] = { 0, };
	switch (selectFlag) {
	case WCL_SELECT_IN_ALL:
		break;
	case WCL_SELECT_IF_HAS_TZPATH:
		CLK_DEBUG_INFO
		("\n\tselect cities which has tz_path for Setting module!!!\n");
		// only find those cities which has tz_path.
		snprintf(selFlag, BUF_SIZE, " and tz_path<>''");
		break;
	case WCL_SELECT_IN_UNSELECTED:
	default:
		// only find those cities which is unselected.
		snprintf(selFlag, BUF_SIZE, " and selected=0");
		break;
	}
	// get item number
	snprintf(szQuery, WORLDCLOCK_DB_QUERY_LEN,
			"select count(idex) from %s where city like 'IDS_WCL_BODY_%%%s%%' or city like '%%%s%%'%s\
            order by city;", WORLDCLOCK_DB_TABLE_CITY, city_name, city_name,
			selFlag);
	//CLK_DEBUG_INFO("szQuery=%s\n",szQuery);
	rc = sqlite3_prepare_v2(g_hDB, szQuery, strlen(szQuery), &p_sqStmt, NULL);
	GOTO_ERROR_IF(SQLITE_OK != rc);
	rc = sqlite3_step(p_sqStmt);
	nRecCount = sqlite3_column_int(p_sqStmt, 0);
	rc = sqlite3_finalize(p_sqStmt);
	p_sqStmt = NULL;

	// read data from db
	*p_recordList = (Wcl_CitySet *) calloc(nRecCount, sizeof(Wcl_CitySet));
	GOTO_ERROR_IF(NULL == *p_recordList);
	p_record = *p_recordList;
	snprintf(szQuery, WORLDCLOCK_DB_QUERY_LEN,
			"select * from %s where city like 'IDS_WCL_BODY_%%%s%%' or city like '%%%s%%'%s order by city;",
			WORLDCLOCK_DB_TABLE_CITY, city_name, city_name, selFlag);
	rc = sqlite3_prepare_v2(g_hDB, szQuery, strlen(szQuery), &p_sqStmt, NULL);
	rc = sqlite3_step(p_sqStmt);
	while (rc == SQLITE_ROW) {
		__data_extract_record(p_record++, p_sqStmt);
		rc = sqlite3_step(p_sqStmt);
	}
	rc = sqlite3_finalize(p_sqStmt);
	GOTO_ERROR_IF(SQLITE_OK != rc);
	p_sqStmt = NULL;
	CLK_FUN_DEBUG_END();
	return nRecCount;

error:
	CLK_ERR("\n[ERROR] szQuery : %s", szQuery);
	if (p_sqStmt) {
		sqlite3_finalize(p_sqStmt);
		p_sqStmt = NULL;
	}
	if (*p_recordList) {
		free(*p_recordList);
	}
	return -1;
}

/**
 * Get city list search country by given country name
 *
 * @param[in]   country_name  given country name
 * @param[out]  p_recordList      Result list which used to store result list
 * @param[in]   selectFlag    selection flag which used for judge selection type
 *
 * @return    -1 if meet error
 *            count of the items in result list
 */
static int __data_get_city_list_by_country_name(const char *country_name,
		Wcl_CitySet ** p_recordList, Wcl_Selection_Flag selectFlag)
{
	CLK_FUN_DEBUG_BEG();
	int rc = -1;
	int nRecCount = 0;
	Wcl_CitySet *p_record = NULL;
	sqlite3_stmt *p_sqStmt = NULL;
	char selFlag[BUF_SIZE] = { 0, };
	char szQuery[WORLDCLOCK_DB_QUERY_LEN] = { 0, };

	switch (selectFlag) {
	case WCL_SELECT_IN_ALL:
		break;
	case WCL_SELECT_IF_HAS_TZPATH:
		CLK_DEBUG_INFO
		("\n\tselect cities which has tz_path for Setting module!!!\n");
		// only find those cities which has tz_path.
		snprintf(selFlag, BUF_SIZE, " and tz_path<>''");
		break;
	case WCL_SELECT_IN_UNSELECTED:
	default:
		// only find those cities which is unselected.
		snprintf(selFlag, BUF_SIZE, " and selected=0");
		break;
	}
	// get item number
	snprintf(szQuery, WORLDCLOCK_DB_QUERY_LEN,
			"select count(idex) from %s where country like 'IDS_WCL_BODY_%%%s%%' or country like '%%%s%%'%s\
            order by country;", WORLDCLOCK_DB_TABLE_CITY, country_name, country_name,
			selFlag);
	//CLK_DEBUG_INFO("szQuery=%s\n",szQuery);
	rc = sqlite3_prepare_v2(g_hDB, szQuery, strlen(szQuery), &p_sqStmt, NULL);
	GOTO_ERROR_IF(SQLITE_OK != rc);
	rc = sqlite3_step(p_sqStmt);
	nRecCount = sqlite3_column_int(p_sqStmt, 0);
	rc = sqlite3_finalize(p_sqStmt);
	p_sqStmt = NULL;

	// read data from db
	*p_recordList = (Wcl_CitySet *) calloc(nRecCount, sizeof(Wcl_CitySet));
	GOTO_ERROR_IF(NULL == *p_recordList);
	p_record = *p_recordList;
	snprintf(szQuery, WORLDCLOCK_DB_QUERY_LEN,
			"select * from %s where country like 'IDS_WCL_BODY_%%%s%%' or country like '%%%s%%'%s\
            order by country;", WORLDCLOCK_DB_TABLE_CITY, country_name, country_name,
			selFlag);
	rc = sqlite3_prepare_v2(g_hDB, szQuery, strlen(szQuery), &p_sqStmt, NULL);
	rc = sqlite3_step(p_sqStmt);
	while (rc == SQLITE_ROW) {
		__data_extract_record(p_record++, p_sqStmt);
		rc = sqlite3_step(p_sqStmt);
	}
	rc = sqlite3_finalize(p_sqStmt);
	GOTO_ERROR_IF(SQLITE_OK != rc);
	p_sqStmt = NULL;

	CLK_FUN_DEBUG_END();
	return nRecCount;

error:
	CLK_ERR("\n[ERROR] szQuery : %s", szQuery);
	CLK_ERR("\n[ERROR] SQL error msg : %s", (char *)sqlite3_errmsg(g_hDB));
	if (p_sqStmt) {
		sqlite3_finalize(p_sqStmt);
		p_sqStmt = NULL;
	}
	if (*p_recordList) {
		free(*p_recordList);
	}
	return -1;
}

/**
 * Get city list search timezone by given number
 *
 * @param[in]   timezone      given number for searching timezone
 * @param[out]  p_recordList      Result list which used to store result list
 * @param[in]   selectFlag    selection flag which used for judge selection type
 *
 * @return    -1 if meet error
 *            count of the items in result list
 */
static int __data_get_city_list_by_timezone(const char *timezone,
		Wcl_CitySet ** p_recordList, Wcl_Selection_Flag selectFlag)
{
	CLK_FUN_DEBUG_BEG();
	int rc = -1;
	int nRecCount = 0;
	Wcl_CitySet *p_record = NULL;
	sqlite3_stmt *p_sqStmt = NULL;
	char selFlag[BUF_SIZE] = { 0, };
	char szQuery[WORLDCLOCK_DB_QUERY_LEN] = { 0, };

	switch (selectFlag) {
	case WCL_SELECT_IN_ALL:
		break;
	case WCL_SELECT_IF_HAS_TZPATH:
		CLK_DEBUG_INFO
		("\n\tselect cities which has tz_path for Setting module!!!\n");
		// only find those cities which has tz_path.
		snprintf(selFlag, BUF_SIZE, " and tz_path<>''");
		break;
	case WCL_SELECT_IN_UNSELECTED:
	default:
		// only find those cities which is unselected.
		snprintf(selFlag, BUF_SIZE, " and selected=0");
		break;
	}
	// get item number
	snprintf(szQuery, WORLDCLOCK_DB_QUERY_LEN,
			"select count(idex) from %s where timezone like 'IDS_WCL_BODY_%%%s%%'\
            or timezone like '%%%s%%'%s order by timezone;", WORLDCLOCK_DB_TABLE_CITY, timezone, timezone, selFlag);
	rc = sqlite3_prepare_v2(g_hDB, szQuery, strlen(szQuery), &p_sqStmt, NULL);
	GOTO_ERROR_IF(SQLITE_OK != rc);
	rc = sqlite3_step(p_sqStmt);
	nRecCount = sqlite3_column_int(p_sqStmt, 0);
	rc = sqlite3_finalize(p_sqStmt);
	p_sqStmt = NULL;

	// read data from db
	*p_recordList = (Wcl_CitySet *) calloc(nRecCount, sizeof(Wcl_CitySet));
	GOTO_ERROR_IF(NULL == *p_recordList);
	p_record = *p_recordList;
	snprintf(szQuery, WORLDCLOCK_DB_QUERY_LEN,
			"select * from %s where timezone like 'IDS_WCL_BODY_%%%s%%'\
            or timezone like '%%%s%%'%s order by timezone;", WORLDCLOCK_DB_TABLE_CITY, timezone, timezone, selFlag);
	rc = sqlite3_prepare_v2(g_hDB, szQuery, strlen(szQuery), &p_sqStmt, NULL);
	rc = sqlite3_step(p_sqStmt);
	while (rc == SQLITE_ROW) {
		__data_extract_record(p_record++, p_sqStmt);
		rc = sqlite3_step(p_sqStmt);
	}
	rc = sqlite3_finalize(p_sqStmt);
	GOTO_ERROR_IF(SQLITE_OK != rc);
	p_sqStmt = NULL;

	CLK_FUN_DEBUG_END();
	return nRecCount;

error:
	CLK_ERR("\n[ERROR] szQuery : %s", szQuery);
	CLK_ERR("\n[ERROR] SQL error msg : %s", (char *)sqlite3_errmsg(g_hDB));
	if (p_sqStmt) {
		sqlite3_finalize(p_sqStmt);
		p_sqStmt = NULL;
	}
	if (*p_recordList) {
		free(*p_recordList);
	}
	return -1;
}

/**
 * Get all cities into given city list
 *
 * @param[out]  p_recordList      Result list which used to store result list
 * @param[in]   selectFlag    selection flag which used for judge selection type
 *
 * @return    -1 if meet error
 *            count of the items in result list
 */
static int __data_get_all_default_cities(Wcl_CitySet ** p_recordList,
		Wcl_Selection_Flag selectFlag)
{
	CLK_FUN_DEBUG_BEG();
	int rc = -1;
	int nRecCount = 0;
	Wcl_CitySet *p_record = NULL;
	sqlite3_stmt *p_sqStmt = NULL;
	char selFlag[BUF_SIZE] = { 0, };
	char szQuery[WORLDCLOCK_DB_QUERY_LEN] = { 0, };
	retv_if(!p_recordList, -1);

	switch (selectFlag) {
	case WCL_SELECT_IN_ALL:
		break;
	case WCL_SELECT_IF_HAS_TZPATH:
		CLK_DEBUG_INFO
		("\n\tselect cities which has tz_path for Setting module!!!\n");
		// only find those cities which has tz_path.
		snprintf(selFlag, BUF_SIZE, " where tz_path<>''");
		break;
	case WCL_SELECT_IN_UNSELECTED:
	default:
		// only find those cities which is unselected.
		snprintf(selFlag, BUF_SIZE, " where selected=0");
		break;
	}
	// get item number
	snprintf(szQuery, WORLDCLOCK_DB_QUERY_LEN,
			"select count(idex) from %s%s;", WORLDCLOCK_DB_TABLE_CITY, selFlag);
	rc = sqlite3_prepare_v2(g_hDB, szQuery, strlen(szQuery), &p_sqStmt, NULL);
	GOTO_ERROR_IF(SQLITE_OK != rc);
	rc = sqlite3_step(p_sqStmt);
	nRecCount = sqlite3_column_int(p_sqStmt, 0);
	rc = sqlite3_finalize(p_sqStmt);
	p_sqStmt = NULL;

	// read data from db
	*p_recordList = (Wcl_CitySet *) calloc(nRecCount, sizeof(Wcl_CitySet));
	GOTO_ERROR_IF(NULL == *p_recordList);
	p_record = *p_recordList;
	snprintf(szQuery, WORLDCLOCK_DB_QUERY_LEN, "select * from %s%s;",
			WORLDCLOCK_DB_TABLE_CITY, selFlag);
	rc = sqlite3_prepare_v2(g_hDB, szQuery, strlen(szQuery), &p_sqStmt, NULL);
	rc = sqlite3_step(p_sqStmt);
	while (rc == SQLITE_ROW) {
		__data_extract_record(p_record++, p_sqStmt);
		rc = sqlite3_step(p_sqStmt);
	}
	rc = sqlite3_finalize(p_sqStmt);
	GOTO_ERROR_IF(SQLITE_OK != rc);
	p_sqStmt = NULL;
	CLK_FUN_DEBUG_END();
	return nRecCount;

error:
	CLK_ERR("\n[ERROR] szQuery : %s", szQuery);
	CLK_ERR("\n[ERROR] SQL error msg : %s", (char *)sqlite3_errmsg(g_hDB));
	if (p_sqStmt) {
		sqlite3_finalize(p_sqStmt);
		p_sqStmt = NULL;
	}
	if (*p_recordList) {
		free(*p_recordList);
	}
	return -1;
}

/**
 * Get all selected cities into given city list
 *
 * @param[out]  p_recordList      Result list which used to store result list
 *
 * @return    -1 if meet error
 *            count of the items in result list
 */
static int __data_get_all_selected_cities(Wcl_CitySet ** p_recordList)
{
	CLK_FUN_DEBUG_BEG();
	int rc = -1;
	int nRecCount = 0;
	Wcl_CitySet *p_record = NULL;
	sqlite3_stmt *p_sqStmt = NULL;
	char szQuery[WORLDCLOCK_DB_QUERY_LEN] = { 0, };

	retv_if(!p_recordList, -1);
	// get item number
	snprintf(szQuery, WORLDCLOCK_DB_QUERY_LEN,
			"select count(idex) from %s where selected=1 order by sequence;",
			WORLDCLOCK_DB_TABLE_CITY);
	rc = sqlite3_prepare_v2(g_hDB, szQuery, strlen(szQuery), &p_sqStmt, NULL);
	GOTO_ERROR_IF(SQLITE_OK != rc);
	rc = sqlite3_step(p_sqStmt);
	nRecCount = sqlite3_column_int(p_sqStmt, 0);
	rc = sqlite3_finalize(p_sqStmt);
	p_sqStmt = NULL;

	// read data from db
	*p_recordList = (Wcl_CitySet *) calloc(nRecCount, sizeof(Wcl_CitySet));
	GOTO_ERROR_IF(NULL == *p_recordList);
	p_record = *p_recordList;
	snprintf(szQuery, WORLDCLOCK_DB_QUERY_LEN,
			"select * from %s where selected=1 order by sequence;",
			WORLDCLOCK_DB_TABLE_CITY);
	rc = sqlite3_prepare_v2(g_hDB, szQuery, strlen(szQuery), &p_sqStmt, NULL);
	rc = sqlite3_step(p_sqStmt);
	while (rc == SQLITE_ROW) {
		__data_extract_record(p_record++, p_sqStmt);
		rc = sqlite3_step(p_sqStmt);
	}
	rc = sqlite3_finalize(p_sqStmt);
	GOTO_ERROR_IF(SQLITE_OK != rc);
	p_sqStmt = NULL;
	CLK_FUN_DEBUG_END();
	return nRecCount;

error:
	CLK_ERR("\n[ERROR] szQuery : %s", szQuery);
	CLK_ERR("\n[ERROR] SQL error msg : %s", (char *)sqlite3_errmsg(g_hDB));
	if (p_sqStmt) {
		sqlite3_finalize(p_sqStmt);
		p_sqStmt = NULL;
	}
	if (*p_recordList) {
		free(*p_recordList);
	}
	return -1;
}

/**
 * Get home city from vconf module
 *
 * @return    NULL if meet error
 *            Pointer to the home city
 */
Wcl_CitySet *worldclock_ug_data_get_local_city()
{
	CLK_FUN_DEBUG_BEG();
	Wcl_CitySet *cs = NULL;
	char *city = NULL;
	char *timezone = NULL;

	// get timezone from vconf
	timezone = vconf_get_str(VCONFKEY_SETAPPL_TIMEZONE_INT);
	// get home city from vconf
	city = vconf_get_str(VCONFKEY_SETAPPL_CITYNAME_INDEX_INT);

	if (timezone && city) {
		int count = __data_get_city_list_by_city_name(city, &cs,
				WCL_SELECT_IN_ALL);
		retv_if(count <= 0, NULL);
	} else {
		// if can not get home city, set default as Seoul
		cs = (Wcl_CitySet *) calloc(1, sizeof(Wcl_CitySet));
		retv_if(!cs, NULL);

		// timezone
		snprintf(cs->timezone, TIMEZONE_BUF_SIZE, "%s", ("GMT+9"));
		// city name
		snprintf(cs->city, CITY_BUF_SIZE, "%s", ("Seoul"));
		// sequence
		cs->sequence = 0;
		// tz_path
		snprintf(cs->tz_path, TZPATH_BUF_SIZE, "%s", ("Asia/Seoul"));
	}
	worldclock_ug_data_get_city_status_from_db(cs);
	// dst
	cs->dst = worldclock_dst_get(cs);
	cs->now_time = time(NULL);

	if (timezone) {
		free(timezone);
		timezone = NULL;
	}
	if (city) {
		free(city);
		city = NULL;
	}
	//CLK_INFO("homezone>>> city: %s, timezone: %s, dst: %d\n", cs->city, cs->timezone, cs->dst);
	CLK_FUN_DEBUG_END();
	return cs;
}

/**
 * Get all select cities into given city list
 *
 * @return    NULL if meet error
 *            Pointer to the result selection list
 */
Eina_List *worldclock_ug_data_get_all_added_city()
{
	CLK_FUN_DEBUG_BEG();
	Wcl_CitySet *cs = NULL;
	Eina_List *el = NULL;

	// get all selected cities into city list
	int count = __data_get_all_selected_cities(&cs);
	GOTO_ERROR_IF(!cs);
	int i = 0;
	//CLK_DEBUG_INFO("count=%d\n",count);
	// extract all cities in city list to eina_list
	for (i = 0; i < count; i++) {
		GOTO_ERROR_IF(NULL == (cs + i));
		// malloc new memory
		Wcl_CitySet *a_cs = (Wcl_CitySet *) calloc(1, sizeof(Wcl_CitySet));
		GOTO_ERROR_IF(!a_cs);
		// copy city data
		worldclock_ug_data_cityset_copy(a_cs, cs + i);
		// append data
		el = eina_list_append(el, a_cs);
	}
	free(cs);
	cs = NULL;
	CLK_FUN_DEBUG_END();
	return el;
error:
	if (cs) {
		free(cs);
	}
	if (el) {
		worldclock_util_glist_remove_all(el, EINA_TRUE);
//		eina_list_free(el);
	}
	return NULL;
}

/**
 * Get all appendable cities into given city list
 *
 * @param[in]   selectFlag    selection flag which used for judge selection type
 *
 * @return    NULL if meet error
 *            Pointer to the result selection list
 */
Eina_List *worldclock_ug_data_get_default_city_list(Wcl_Selection_Flag selectFlag)
{
	CLK_FUN_DEBUG_BEG();
	int i = 0;
	int count = 0;
	Wcl_CitySet *cs = NULL;
	Eina_List *el = NULL;

	// get all appendable cities
	count = __data_get_all_default_cities(&cs, selectFlag);
	GOTO_ERROR_IF(!cs);
	//CLK_DEBUG_INFO("count=%d\n",count);
	for (i = 0; i < count; i++) {
		GOTO_ERROR_IF(NULL == (cs + i));
		// malloc new memory
		Wcl_CitySet *a_cs = (Wcl_CitySet *) calloc(1, sizeof(Wcl_CitySet));
		GOTO_ERROR_IF(!a_cs);
		// copy city data
		worldclock_ug_data_cityset_copy(a_cs, cs + i);
		// append data
		el = eina_list_append(el, a_cs);
	}
	free(cs);
	cs = NULL;
	CLK_FUN_DEBUG_END();
	return el;
error:
	if (cs) {
		free(cs);
	}
	if (el) {
		worldclock_util_glist_remove_all(el, EINA_TRUE);
//		eina_list_free(el);
	}
	return NULL;
}

/**
 * Search cities according to search_txt, and return search result
 *
 * @param[in]  data
 * @param[in]  search_type   the search type which will search in this function
 *                           search_type: 1->by city name, 2->by country name, 3->by timezone
 * @param[in]  selectFlag    selection flag which used for judge selection type
 *
 * @return    NULL if meet error
 *            Pointer to the result selection list
 */
Eina_List *worldclock_ug_data_get_search_city_list(const char *search_txt,
		Wcl_Search_Type search_type, Wcl_Selection_Flag selectFlag)
{
	CLK_FUN_BEG();
	int i = 0;
	int count = 0;
	Wcl_CitySet *cs = NULL;
	Eina_List *el = NULL;

	// get search result
	switch (search_type) {
	case WCL_SEARCH_BY_ALL_KEYWORDS:
		// search by all issues
		count = __data_get_city_list_by_search_text(search_txt, &cs, selectFlag);
		break;
	case WCL_SEARCH_BY_CITY_NAME:
		// search by city name
		count = __data_get_city_list_by_city_name(search_txt, &cs, selectFlag);
		break;
	case WCL_SEARCH_BY_COUNTRY_NAME:
		// search by country name
		count = __data_get_city_list_by_country_name(search_txt, &cs, selectFlag);
		break;
	case WCL_SEARCH_BY_TIMEZONE:
		// search by timezone
		count = __data_get_city_list_by_timezone(search_txt, &cs, selectFlag);
		break;
	default:
		CLK_ERR("no such searching type!\n");
		return NULL;
	}

	GOTO_ERROR_IF(!cs);

	// extract search result into eina_list
	for (i = 0; i < count; i++) {
		// malloc new memory
		Wcl_CitySet *a_cs = (Wcl_CitySet *) calloc(1, sizeof(Wcl_CitySet));
		GOTO_ERROR_IF(!a_cs);
		// copy city data
		worldclock_ug_data_cityset_copy(a_cs, cs + i);
		// append record
		el = eina_list_append(el, a_cs);
	}
	if (cs) {
		free(cs);
	}
	CLK_FUN_END();
	return el;
error:
	if (cs) {
		free(cs);
	}
	if (el) {
		worldclock_util_glist_remove_all(el, EINA_TRUE);
//		eina_list_free(el);
	}
	return NULL;
}

/**
 * Update data of given city to database
 *
 * @param[in]  p_record      data of given city
 *
 * @return    EINA_FALSE if meet error
 *            EINA_TRUE if update successfully
 */
Eina_Bool worldclock_ug_data_update_db_record(Wcl_CitySet * p_record)
{
	CLK_FUN_DEBUG_BEG();
	int rc = 0;
	Wcl_CitySet *cs = p_record;
	char *pszErrorMsg = NULL;
	char *szQuery = NULL;

	retv_if(!cs, EINA_FALSE);
	/*  Update Inserted Record  */
	//CLK_DEBUG_INFO("cs->city=%s, cs->country=%s, dst_type=%d, selected=%d, sequence=%d\n",
	//          cs->city, cs->country, cs->dst_type, cs->selected, cs->sequence);
	szQuery =
			sqlite3_mprintf
			("update '%q' set dst_type = %d, dst_enabled = %d, selected = %d, sequence = %d where idex = %d;",
					WORLDCLOCK_DB_TABLE_CITY, cs->dst_type, cs->dst_enabled, cs->selected,
					cs->sequence, cs->index);
	//CLK_DEBUG_INFO("szQuery: %s\n", szQuery);

	rc = sqlite3_exec(g_hDB, "BEGIN  TRANSACTION", NULL, NULL, &pszErrorMsg);
	rc = sqlite3_exec(g_hDB, szQuery, NULL, NULL, &pszErrorMsg);
	GOTO_ERROR_IF(SQLITE_OK != rc);
	rc = sqlite3_exec(g_hDB, "COMMIT TRANSACTION", NULL, NULL, &pszErrorMsg);

	sqlite3_free(szQuery);

	//CLK_DEBUG_INFO("\n\t++++++++++++++++++++++  update successfully !!!!! \n");
	CLK_FUN_DEBUG_END();
	return EINA_TRUE;

error:
	CLK_ERR("\n[ERROR] szQuery : %s", szQuery);
	CLK_ERR("\n[ERROR] error index : %d\n", rc);
	CLK_ERR("\n[ERROR] error msg : %s\n", pszErrorMsg);
	return EINA_FALSE;
}

/**
 * Get data of given city to database
 *
 * @param[in]  p_record      data of given city
 *
 * @return    EINA_FALSE if meet error
 *            EINA_TRUE if load successfully
 */
Eina_Bool worldclock_ug_data_get_city_status_from_db(Wcl_CitySet * p_record)
{
	CLK_FUN_DEBUG_BEG();
	int rc = -1;
	Wcl_CitySet *cs = p_record;
	sqlite3_stmt *p_sqStmt = NULL;
	char szQuery[WORLDCLOCK_DB_QUERY_LEN] = { 0, };

	retv_if(!cs, EINA_FALSE);
	// get item number
	//snprintf(szQuery, WORLDCLOCK_DB_QUERY_LEN, "select * from %s where idex = %d;", WORLDCLOCK_DB_TABLE_CITY, cs->index);
	snprintf(szQuery, WORLDCLOCK_DB_QUERY_LEN,
			"select * from %s where city = '%s' and country = '%s';",
			WORLDCLOCK_DB_TABLE_CITY, cs->city, cs->country);
	//CLK_INFO_YELLOW("szQuery=%s\n",szQuery);
	rc = sqlite3_prepare_v2(g_hDB, szQuery, strlen(szQuery), &p_sqStmt, NULL);
	GOTO_ERROR_IF(SQLITE_OK != rc);
	rc = sqlite3_step(p_sqStmt);
	GOTO_ERROR_IF(SQLITE_OK != rc && SQLITE_ROW != rc);
	__data_extract_record(cs, p_sqStmt);
	//CLK_INFO_YELLOW("cs->city = %s , cs->country = %s\n", cs->city, cs->country);
	rc = sqlite3_finalize(p_sqStmt);
	GOTO_ERROR_IF(SQLITE_OK != rc);
	p_sqStmt = NULL;
	CLK_FUN_DEBUG_END();
	return EINA_TRUE;

error:
	CLK_ERR("\n[ERROR] szQuery : %s", szQuery);
	CLK_ERR("\n[ERROR] SQL error msg : %s", (char *)sqlite3_errmsg(g_hDB));
	if (p_sqStmt) {
		sqlite3_finalize(p_sqStmt);
		p_sqStmt = NULL;
	}
	return EINA_FALSE;
}

/**
 * Try to open data base
 *
 * @return    EINA_FALSE if open database failed
 *            EINA_TRUE if open database successfully
 */
Eina_Bool worldclock_ug_data_open_database()
{
	CLK_FUN_DEBUG_BEG();
	//int rc = 0;
	//int* error_code = SQLITE_OP_OK;

	// check whether file exists, if not, create it
	if (!g_hDB) {
		// open it, create it if not exist
	    char *res = NULL;
	    res = app_get_shared_resource_path();
	    if(res)
	    {
	        char db_path[BUF_SIZE];
	        snprintf(db_path, BUF_SIZE, "%s/%s", res, WORLDCLOCK_DB);
	        if (SQLITE_OK != sqlite3_open(db_path, &g_hDB)) {
	            free(res);
	            return EINA_FALSE;
	        }
	    }
	    else
	    {
	        return EINA_FALSE;
	    }
	    free(res);

	}

	CLK_FUN_DEBUG_END();
	return __data_create_country_table();
}

/**
 * Try to close data base
 *
 * @return    EINA_FALSE if open database failed
 *            EINA_TRUE if open database successfully
 */
Eina_Bool worldclock_ug_data_close_database()
{
	CLK_FUN_DEBUG_BEG();
	sqlite3_close(g_hDB);
	g_hDB = NULL;
	CLK_FUN_DEBUG_END();
	return EINA_TRUE;
}

/**
 * Copy given city data to aimed city
 *
 * @param[in]   src_city     the source city record
 * @param[out]  dst_city     the aimed city record
 *
 * @return
 */
void worldclock_ug_data_cityset_copy(Wcl_CitySet * dst_city, const Wcl_CitySet * src_city)
{
	ret_if(!dst_city || !src_city);

	//copy data of city set
	memcpy(dst_city, src_city, sizeof(Wcl_CitySet));
}
