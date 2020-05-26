/*
 * Copyright (C) 1994-2020 Altair Engineering, Inc.
 * For more information, contact Altair at www.altair.com.
 *
 * This file is part of both the OpenPBS software ("OpenPBS")
 * and the PBS Professional ("PBS Pro") software.
 *
 * Open Source License Information:
 *
 * OpenPBS is free software. You can redistribute it and/or modify it under
 * the terms of the GNU Affero General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * OpenPBS is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Affero General Public
 * License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Commercial License Information:
 *
 * PBS Pro is commercially licensed software that shares a common core with
 * the OpenPBS software.  For a copy of the commercial license terms and
 * conditions, go to: (http://www.pbspro.com/agreement.html) or contact the
 * Altair Legal Department.
 *
 * Altair's dual-license business model allows companies, individuals, and
 * organizations to create proprietary derivative works of OpenPBS and
 * distribute them - whether embedded or bundled with other software -
 * under a commercial license agreement.
 *
 * Use of Altair's trademarks, including but not limited to "PBS™",
 * "OpenPBS®", "PBS Professional®", and "PBS Pro™" and Altair's logos is
 * subject to Altair's trademark licensing policies.
 */



/**
 * @file    db_postgres_resv.c
 *
 * @brief
 *      Implementation of the resv data access functions for postgres
 */
#include <pbs_config.h>   /* the master config generated by configure */
#include "pbs_db.h"
#include "db_postgres.h"

/**
 * @brief
 *	Prepare all the reservation related sqls. Typically called after connect
 *	and before any other sql exeuction
 *
 * @param[in]	conn - Database connection handle
 *
 * @return      Error code
 * @retval	-1 - Failure
 * @retval	 0 - Success
 *
 */
int
pg_db_prepare_resv_sqls(pbs_db_conn_t *conn)
{
	snprintf(conn->conn_sql, MAX_SQL_LENGTH, "insert into pbs.resv ("
		"ri_resvID, "
		"ri_queue, "
		"ri_state, "
		"ri_substate, "
		"ri_stime, "
		"ri_etime, "
		"ri_duration, "
		"ri_tactive, "
		"ri_svrflags, "
		"ri_numattr, "
		"ri_resvTag, "
		"ri_un_type, "
		"ri_fromsock, "
		"ri_fromaddr, "
		"ri_savetm, "
		"ri_creattm, "
		"attributes "
		") "
		"values "
		"($1, $2, $3, $4, $5, $6, $7, $8, $9, $10, $11, $12, $13, "
		"$14, localtimestamp, localtimestamp, hstore($15::text[])) "
		"returning to_char(ri_savetm, 'YYYY-MM-DD HH24:MI:SS.US') as ri_savetm");
	if (pg_prepare_stmt(conn, STMT_INSERT_RESV, conn->conn_sql, 15) != 0)
		return -1;

	snprintf(conn->conn_sql, MAX_SQL_LENGTH, "update pbs.resv set "
		"ri_queue = $2, "
		"ri_state = $3, "
		"ri_substate = $4, "
		"ri_stime = $5, "
		"ri_etime = $6, "
		"ri_duration = $7, "
		"ri_tactive = $8, "
		"ri_svrflags = $9, "
		"ri_numattr = $10, "
		"ri_resvTag = $11, "
		"ri_un_type = $12, "
		"ri_fromsock = $13, "
		"ri_fromaddr = $14, "
		"ri_savetm = localtimestamp, "
		"attributes = attributes || hstore($15::text[]) "
		"where ri_resvID = $1 "
		"returning to_char(ri_savetm, 'YYYY-MM-DD HH24:MI:SS.US') as ri_savetm");
	if (pg_prepare_stmt(conn, STMT_UPDATE_RESV, conn->conn_sql, 15) != 0)
		return -1;

	snprintf(conn->conn_sql, MAX_SQL_LENGTH, "update pbs.resv set "
		"ri_queue = $2, "
		"ri_state = $3, "
		"ri_substate = $4, "
		"ri_stime = $5, "
		"ri_etime = $6, "
		"ri_duration = $7, "
		"ri_tactive = $8, "
		"ri_svrflags = $9, "
		"ri_numattr = $10, "
		"ri_resvTag = $11, "
		"ri_un_type = $12, "
		"ri_fromsock = $13, "
		"ri_fromaddr = $14, "
		"ri_savetm = localtimestamp "
		"where ri_resvID = $1 "
		"returning to_char(ri_savetm, 'YYYY-MM-DD HH24:MI:SS.US') as ri_savetm");
	if (pg_prepare_stmt(conn, STMT_UPDATE_RESV_QUICK, conn->conn_sql, 14) != 0)
		return -1;

	snprintf(conn->conn_sql, MAX_SQL_LENGTH, "update pbs.resv set "
		"ri_savetm = localtimestamp, "
		"attributes = attributes || hstore($2::text[]) "
		"where ri_resvID = $1 "
		"returning to_char(ri_savetm, 'YYYY-MM-DD HH24:MI:SS.US') as ri_savetm");
	if (pg_prepare_stmt(conn, STMT_UPDATE_RESV_ATTRSONLY, conn->conn_sql, 2) != 0)
		return -1;

	snprintf(conn->conn_sql, MAX_SQL_LENGTH, "update pbs.resv set "
		"ri_savetm = localtimestamp,"
		"attributes = attributes - $2::text[] "
		"where ri_resvID = $1 "
		"returning to_char(ri_savetm, 'YYYY-MM-DD HH24:MI:SS.US') as ri_savetm");
	if (pg_prepare_stmt(conn, STMT_REMOVE_RESVATTRS, conn->conn_sql, 2) != 0)
		return -1;

	snprintf(conn->conn_sql, MAX_SQL_LENGTH, "select "
		"ri_resvID, "
		"ri_queue, "
		"ri_state, "
		"ri_substate, "
		"ri_stime, "
		"ri_etime, "
		"ri_duration, "
		"ri_tactive, "
		"ri_svrflags, "
		"ri_numattr, "
		"ri_resvTag, "
		"ri_un_type, "
		"ri_fromsock, "
		"ri_fromaddr, "
		"to_char(ri_savetm, 'YYYY-MM-DD HH24:MI:SS.US') as ri_savetm, "
		"hstore_to_array(attributes) as attributes "
		"from pbs.resv where ri_resvid = $1");
	if (pg_prepare_stmt(conn, STMT_SELECT_RESV, conn->conn_sql, 1) != 0)
		return -1;

	snprintf(conn->conn_sql, MAX_SQL_LENGTH, "select "
		"ri_resvID, "
		"ri_queue, "
		"ri_state, "
		"ri_substate, "
		"ri_stime, "
		"ri_etime, "
		"ri_duration, "
		"ri_tactive, "
		"ri_svrflags, "
		"ri_numattr, "
		"ri_resvTag, "
		"ri_un_type, "
		"ri_fromsock, "
		"ri_fromaddr, "
		"to_char(ri_savetm, 'YYYY-MM-DD HH24:MI:SS.US') as ri_savetm, "
		"hstore_to_array(attributes) as attributes "
		"from pbs.resv where ri_savetm > to_timestamp($1, 'YYYY-MM-DD HH24:MI:SS:US') "
		"order by ri_savetm ");
	if (pg_prepare_stmt(conn, STMT_FINDRESVS_FROM_TIME_ORDBY_SAVETM, conn->conn_sql, 1) != 0)
		return -1;

	snprintf(conn->conn_sql, MAX_SQL_LENGTH, "select "
		"ri_resvID, "
		"ri_queue, "
		"ri_state, "
		"ri_substate, "
		"ri_type, "
		"ri_stime, "
		"ri_etime, "
		"ri_duration, "
		"ri_tactive, "
		"ri_svrflags, "
		"ri_numattr, "
		"ri_resvTag, "
		"ri_un_type, "
		"ri_fromsock, "
		"ri_fromaddr, "
		"to_char(ri_savetm, 'YYYY-MM-DD HH24:MI:SS.US') as ri_savetm, "
		"hstore_to_array(attributes) as attributes "
		"from pbs.resv "
		"order by ri_creattm");
	if (pg_prepare_stmt(conn, STMT_FINDRESVS_ORDBY_CREATTM,
		conn->conn_sql, 0) != 0)
		return -1;

	snprintf(conn->conn_sql, MAX_SQL_LENGTH, "delete from pbs.resv where ri_resvid = $1");
	if (pg_prepare_stmt(conn, STMT_DELETE_RESV, conn->conn_sql, 1) != 0)
		return -1;

	return 0;
}

/**
 * @brief
 *	Load resv data from the row into the resv object
 *
 * @param[in]	res - Resultset from a earlier query
 * @param[in]	presv  - resv object to load data into
 * @param[in]	row - The current row to load within the resultset
 *
 * @return      Error code
 * @retval	-1 - On Error
 * @retval	 0 - On Success
 * @retval	>1 - Number of attributes
 */
static int
load_resv(PGresult *res, pbs_db_resv_info_t *presv, int row)
{
	char *raw_array;
	static int ri_resvid_fnum, ri_queue_fnum, ri_state_fnum, ri_substate_fnum, ri_stime_fnum,
	ri_etime_fnum, ri_duration_fnum, ri_tactive_fnum, ri_svrflags_fnum, ri_numattr_fnum, ri_resvTag_fnum,
	ri_un_type_fnum, ri_fromsock_fnum, ri_fromaddr_fnum, ri_savetm_fnum,
	attributes_fnum;

	static int fnums_inited = 0;

	if (fnums_inited == 0) {
		ri_resvid_fnum = PQfnumber(res, "ri_resvID");
		ri_queue_fnum = PQfnumber(res, "ri_queue");
		ri_state_fnum = PQfnumber(res, "ri_state");
		ri_substate_fnum = PQfnumber(res, "ri_substate");
		ri_stime_fnum = PQfnumber(res, "ri_stime");
		ri_etime_fnum = PQfnumber(res, "ri_etime");
		ri_duration_fnum = PQfnumber(res, "ri_duration");
		ri_tactive_fnum = PQfnumber(res, "ri_tactive");
		ri_svrflags_fnum = PQfnumber(res, "ri_svrflags");
		ri_numattr_fnum = PQfnumber(res, "ri_numattr");
		ri_resvTag_fnum = PQfnumber(res, "ri_resvTag");
		ri_un_type_fnum = PQfnumber(res, "ri_un_type");
		ri_fromsock_fnum = PQfnumber(res, "ri_fromsock");
		ri_fromaddr_fnum = PQfnumber(res, "ri_fromaddr");
		ri_savetm_fnum = PQfnumber(res, "ri_savetm");
		attributes_fnum = PQfnumber(res, "attributes");
		fnums_inited = 1;
	}

	GET_PARAM_STR(res, row, presv->ri_resvid, ri_resvid_fnum);
	GET_PARAM_STR(res, row, presv->ri_queue, ri_queue_fnum);
	GET_PARAM_INTEGER(res, row, presv->ri_state, ri_state_fnum);
	GET_PARAM_INTEGER(res, row, presv->ri_substate, ri_substate_fnum);
	GET_PARAM_BIGINT(res, row, presv->ri_stime, ri_stime_fnum);
	GET_PARAM_BIGINT(res, row, presv->ri_etime, ri_etime_fnum);
	GET_PARAM_BIGINT(res, row, presv->ri_duration, ri_duration_fnum);
	GET_PARAM_INTEGER(res, row, presv->ri_tactive, ri_tactive_fnum);
	GET_PARAM_INTEGER(res, row, presv->ri_svrflags, ri_svrflags_fnum);
	GET_PARAM_INTEGER(res, row, presv->ri_numattr, ri_numattr_fnum);
	GET_PARAM_INTEGER(res, row, presv->ri_resvTag, ri_resvTag_fnum);
	GET_PARAM_INTEGER(res, row, presv->ri_un_type, ri_un_type_fnum);
	GET_PARAM_INTEGER(res, row, presv->ri_fromsock, ri_fromsock_fnum);
	GET_PARAM_BIGINT(res, row, presv->ri_fromaddr, ri_fromaddr_fnum);
	GET_PARAM_STR(res, row, presv->ri_savetm, ri_savetm_fnum);
	GET_PARAM_BIN(res, row, raw_array, attributes_fnum);

	/* convert attributes from postgres raw array format */
	return (dbarray_2_attrlist(raw_array, &presv->db_attr_list));
}

/**
 * @brief
 *	Insert resv data into the database
 *
 * @param[in]	conn - Connection handle
 * @param[in]	obj  - Information of resv to be inserted
 *
 * @return      Error code
 * @retval	-1 - Failure
 * @retval	 0 - Success
 *
 */
int
pg_db_save_resv(pbs_db_conn_t *conn, pbs_db_obj_info_t *obj, int savetype)
{
	pbs_db_resv_info_t *presv = obj->pbs_db_un.pbs_db_resv;
	char *stmt = NULL;
	int params;
	char *raw_array = NULL;
	static int ri_savetm_fnum;
	static int fnums_inited = 0;

	SET_PARAM_STR(conn, presv->ri_resvid, 0);

	if (savetype & OBJ_SAVE_QS) {
		SET_PARAM_STR(conn, presv->ri_queue, 1);
		SET_PARAM_INTEGER(conn, presv->ri_state, 2);
		SET_PARAM_INTEGER(conn, presv->ri_substate, 3);
		SET_PARAM_BIGINT(conn, presv->ri_stime, 4);
		SET_PARAM_BIGINT(conn, presv->ri_etime, 5);
		SET_PARAM_BIGINT(conn, presv->ri_duration, 6);
		SET_PARAM_INTEGER(conn, presv->ri_tactive, 7);
		SET_PARAM_INTEGER(conn, presv->ri_svrflags, 8);
		SET_PARAM_INTEGER(conn, presv->ri_numattr, 9);
		SET_PARAM_INTEGER(conn, presv->ri_resvTag, 10);
		SET_PARAM_INTEGER(conn, presv->ri_un_type, 11);
		SET_PARAM_INTEGER(conn, presv->ri_fromsock, 12);
		SET_PARAM_BIGINT(conn, presv->ri_fromaddr, 13);
		stmt = STMT_UPDATE_RESV_QUICK;
		params = 14;
	}

	/* are there attributes to save to memory or local cache? */
	if (presv->cache_attr_list.attr_count > 0) {
		dist_cache_save_attrs(presv->ri_resvid, &presv->cache_attr_list);
	}

	if ((presv->db_attr_list.attr_count > 0) || (savetype & OBJ_SAVE_NEW)) {
		int len = 0;
		/* convert attributes to postgres raw array format */
		if ((len = attrlist_2_dbarray(&raw_array, &presv->db_attr_list)) <= 0)
			return -1;

		if (savetype & OBJ_SAVE_QS) {
			SET_PARAM_BIN(conn, raw_array, len, 14);
			stmt = STMT_UPDATE_RESV;
			params = 15;
		} else {
			SET_PARAM_BIN(conn, raw_array, len, 1);
			params = 2;
			stmt = STMT_UPDATE_RESV_ATTRSONLY;
		}
	}

	if (savetype & OBJ_SAVE_NEW)
		stmt = STMT_INSERT_RESV;

	if (stmt != NULL) {
		if (pg_db_cmd(conn, stmt, params) != 0) {
			free(raw_array);
			return -1;
		}
		if (fnums_inited == 0) {
			ri_savetm_fnum = PQfnumber(conn->conn_resultset, "ri_savetm");
			fnums_inited = 1;
		}
		GET_PARAM_STR(conn->conn_resultset, 0, presv->ri_savetm, ri_savetm_fnum);
		PQclear(conn->conn_resultset);
		free(raw_array);
	}

	return 0;
}

/**
 * @brief
 *	Load resv data from the database
 *
 * @param[in]	conn - Connection handle
 * @param[in]	obj  - Load resv information into this object
 *
 * @return      Error code
 * @retval	-1 - Failure
 * @retval	 0 - Success
 * @retval	 1 -  Success but no rows loaded
 *
 */
int
pg_db_load_resv(pbs_db_conn_t *conn, pbs_db_obj_info_t *obj)
{
	pbs_db_resv_info_t *presv = obj->pbs_db_un.pbs_db_resv;
	PGresult *res;
	int rc;

	SET_PARAM_STR(conn, presv->ri_resvid, 0);

	if ((rc = pg_db_query(conn, STMT_SELECT_RESV, 1, &res)) != 0)
		return rc;

	rc = load_resv(res, presv, 0);

	PQclear(res);

	if (rc == 0) {
		/* in case of multi-server, also read NOSAVM attributes from distributed cache */
		/* call in this functions since all call paths lead to this before decode */
		//if (use_dist_cache) {
		//	dist_cache_recov_attrs(presv->ri_resvid, &presv->ri_savetm, &presv->cache_attr_list);
		//}
	}

	return rc;
}

/**
 * @brief
 *	Find resv
 *
 * @param[in]	conn - Connection handle
 * @param[in]	st   - The cursor state variable updated by this query
 * @param[in]	obj  - Information of resv to be found
 * @param[in]	opts - Any other options (like flags, timestamp)
 *
 * @return      Error code
 * @retval	-1 - Failure
 * @retval	 0 - Success
 * @retval	 1 - Success, but no rows found
 *
 */
int
pg_db_find_resv(pbs_db_conn_t *conn, void *st, pbs_db_obj_info_t *obj,
	pbs_db_query_options_t *opts)
{
	PGresult *res;
	int rc;
	int params;
	pg_query_state_t *state = (pg_query_state_t *) st;

	if (!state)
		return -1;

	if (opts != NULL && opts->timestamp) {
		SET_PARAM_STR(conn, opts->timestamp, 0);
		params = 1;
		strcpy(conn->conn_sql, STMT_FINDRESVS_FROM_TIME_ORDBY_SAVETM);
	} else {
		params = 0;
		strcpy(conn->conn_sql, STMT_FINDRESVS_ORDBY_CREATTM);
	}

	if ((rc = pg_db_query(conn, conn->conn_sql, params, &res)) != 0)
		return rc;

	state->row = 0;
	state->res = res;
	state->count = PQntuples(res);
	return 0;
}

/**
 * @brief
 *	Get the next resv from the cursor
 *
 * @param[in]	conn - Connection handle
 * @param[in]	st   - The cursor state
 * @param[in]	obj  - Resv information is loaded into this object
 *
 * @return      Error code
 *		(Even though this returns only 0 now, keeping it as int
 *			to support future change to return a failure)
 * @retval	 0 - Success
 *
 */
int
pg_db_next_resv(pbs_db_conn_t* conn, void* st, pbs_db_obj_info_t* obj)
{
	pg_query_state_t *state = (pg_query_state_t *) st;
	obj->pbs_db_un.pbs_db_resv->ri_savetm[0] = '\0';
	
	return (load_resv(state->res, obj->pbs_db_un.pbs_db_resv, state->row));
}

/**
 * @brief
 *	Delete the resv from the database
 *
 * @param[in]	conn - Connection handle
 * @param[in]	obj  - Resv information
 *
 * @return      Error code
 * @retval	-1 - Failure
 * @retval	 0 - Success
 * @retval	 1 - Success but no rows deleted
 *
 */
int
pg_db_delete_resv(pbs_db_conn_t *conn, pbs_db_obj_info_t* obj)
{
	pbs_db_resv_info_t *presv = obj->pbs_db_un.pbs_db_resv;
	SET_PARAM_STR(conn, presv->ri_resvid, 0);
	return (pg_db_cmd(conn, STMT_DELETE_RESV, 1));
}


/**
 * @brief
 *	Deletes attributes of a Resv
 *
 * @param[in]	conn - Connection handle
 * @param[in]	obj  - Resv information
 * @param[in]	obj_id  - Resv id
 * @param[in]	attr_list - List of attributes
 *
 * @return      Error code
 * @retval	 0 - Success
 * @retval	-1 - On Failure
 *
 */
int
pg_db_del_attr_resv(pbs_db_conn_t *conn, void *obj_id, char *sv_time, pbs_db_attr_list_t *attr_list)
{
	char *raw_array = NULL;
	int len = 0;
	static int ri_savetm_fnum;
	static int fnums_inited = 0;

	if ((len = attrlist_2_dbarray_ex(&raw_array, attr_list, 1)) <= 0)
		return -1;
	SET_PARAM_STR(conn, obj_id, 0);

	SET_PARAM_BIN(conn, raw_array, len, 1);

	if (pg_db_cmd(conn, STMT_REMOVE_RESVATTRS, 2) != 0) {
		free(raw_array);
		return -1;
	}

	if (fnums_inited == 0) {
		ri_savetm_fnum = PQfnumber(conn->conn_resultset, "ri_savetm");
		fnums_inited = 1;
	}
	GET_PARAM_STR(conn->conn_resultset, 0, sv_time, ri_savetm_fnum);
	PQclear(conn->conn_resultset);
	free(raw_array);

	return 0;
}

