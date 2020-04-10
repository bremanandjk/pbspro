/*
 * Copyright (C) 1994-2020 Altair Engineering, Inc.
 * For more information, contact Altair at www.altair.com.
 *
 * This file is part of the PBS Professional ("PBS Pro") software.
 *
 * Open Source License Information:
 *
 * PBS Pro is free software. You can redistribute it and/or modify it under the
 * terms of the GNU Affero General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option) any
 * later version.
 *
 * PBS Pro is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.
 * See the GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Commercial License Information:
 *
 * For a copy of the commercial license terms and conditions,
 * go to: (http://www.pbspro.com/UserArea/agreement.html)
 * or contact the Altair Legal Department.
 *
 * Altair’s dual-license business model allows companies, individuals, and
 * organizations to create proprietary derivative works of PBS Pro and
 * distribute them - whether embedded or bundled with other software -
 * under a commercial license agreement.
 *
 * Use of Altair’s trademarks, including but not limited to "PBS™",
 * "PBS Professional®", and "PBS Pro™" and Altair’s logos is subject to Altair's
 * trademark licensing policies.
 *
 */


/**
 * @file    db_postgres_job.c
 *
 * @brief
 *      Implementation of the job data access functions for postgres
 */

#include <pbs_config.h>   /* the master config generated by configure */
#include "pbs_db.h"
#include "db_postgres.h"

/**
 * @brief
 *	Prepare all the job related sqls. Typically called after connect
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
pg_db_prepare_job_sqls(pbs_db_conn_t *conn)
{
	snprintf(conn->conn_sql, MAX_SQL_LENGTH, "insert into pbs.job ("
		"ji_jobid,"
		"ji_state,"
		"ji_substate,"
		"ji_svrflags,"
		"ji_numattr,"
		"ji_ordering,"
		"ji_priority,"
		"ji_stime,"
		"ji_endtBdry,"
		"ji_queue,"
		"ji_destin,"
		"ji_un_type,"
		"ji_momaddr,"
		"ji_momport,"
		"ji_exitstat,"
		"ji_quetime,"
		"ji_rteretry,"
		"ji_fromsock,"
		"ji_fromaddr,"
		"ji_4jid,"
		"ji_4ash,"
		"ji_credtype,"
		"ji_qrank,"
		"ji_savetm,"
		"ji_creattm,"
		"attributes"
		") "
		"values ($1, $2, $3, $4, $5, $6, $7, $8, $9, "
		"$10, $11, $12, $13,"
		"$14, $15, $16, $17, $18, $19, $20, $21, $22, $23,"
		 "localtimestamp, localtimestamp,"
		"hstore($24::text[]))");

	if (pg_prepare_stmt(conn, STMT_INSERT_JOB, conn->conn_sql, 24) != 0)
		return -1;

	snprintf(conn->conn_sql, MAX_SQL_LENGTH, "update pbs.job set "
		"ji_state = $2,"
		"ji_substate = $3,"
		"ji_svrflags = $4,"
		"ji_numattr = $5,"
		"ji_ordering = $6,"
		"ji_priority = $7,"
		"ji_stime = $8,"
		"ji_endtBdry = $9,"
		"ji_queue  = $10,"
		"ji_destin = $11,"
		"ji_un_type = $12,"
		"ji_momaddr = $13,"
		"ji_momport  = $14,"
		"ji_exitstat = $15,"
		"ji_quetime = $16,"
		"ji_rteretry = $17,"
		"ji_fromsock = $18,"
		"ji_fromaddr = $19,"
		"ji_4jid = $20,"
		"ji_4ash = $21,"
		"ji_credtype = $22,"
		"ji_qrank = $23,"
		"ji_savetm = localtimestamp,"
		"attributes = attributes || hstore($24::text[]) "
		"where ji_jobid = $1");
	if (pg_prepare_stmt(conn, STMT_UPDATE_JOB, conn->conn_sql, 24) != 0)
		return -1;

	snprintf(conn->conn_sql, MAX_SQL_LENGTH, "update pbs.job set "
		"ji_savetm = localtimestamp,"
		"attributes = attributes - hstore($2::text[]) "
		"where ji_jobid = $1");
	if (pg_prepare_stmt(conn, STMT_REMOVE_JOBATTRS, conn->conn_sql, 2) != 0)
		return -1;

	snprintf(conn->conn_sql, MAX_SQL_LENGTH, "update pbs.job set "
		"ji_state = $2,"
		"ji_substate = $3,"
		"ji_svrflags = $4,"
		"ji_numattr = $5,"
		"ji_ordering = $6,"
		"ji_priority = $7,"
		"ji_stime = $8,"
		"ji_endtBdry = $9,"
		"ji_queue  = $10,"
		"ji_destin = $11,"
		"ji_un_type = $12,"
		"ji_momaddr = $13,"
		"ji_momport  = $14,"
		"ji_exitstat = $15,"
		"ji_quetime = $16,"
		"ji_rteretry = $17,"
		"ji_fromsock = $18,"
		"ji_fromaddr = $19,"
		"ji_4jid = $20,"
		"ji_4ash = $21,"
		"ji_credtype = $22,"
		"ji_qrank = $23,"
		"ji_savetm = localtimestamp "
		"where ji_jobid = $1");
	if (pg_prepare_stmt(conn, STMT_UPDATE_JOB_QUICK, conn->conn_sql, 23) != 0)
		return -1;

	snprintf(conn->conn_sql, MAX_SQL_LENGTH, "select "
		"ji_jobid,"
		"ji_state,"
		"ji_substate,"
		"ji_svrflags,"
		"ji_numattr,"
		"ji_ordering,"
		"ji_priority,"
		"ji_stime,"
		"ji_endtBdry,"
		"ji_queue,"
		"ji_destin,"
		"ji_un_type,"
		"ji_momaddr,"
		"ji_momport,"
		"ji_exitstat,"
		"ji_quetime,"
		"ji_rteretry,"
		"ji_fromsock,"
		"ji_fromaddr,"
		"ji_4jid,"
		"ji_4ash,"
		"ji_credtype,"
		"ji_qrank,"
		"extract(epoch from ji_savetm)::bigint as ji_savetm, "
		"extract(epoch from ji_creattm)::bigint as ji_creattm, "
		"hstore_to_array(attributes) as attributes "
		"from pbs.job where ji_jobid = $1");
	if (pg_prepare_stmt(conn, STMT_SELECT_JOB, conn->conn_sql, 1) != 0)
		return -1;

	/*
	 * Use the sql encode function to encode the $2 parameter. Encode using
	 * 'escape' mode. Encode considers $2 as a bytea and returns a escaped
	 * string using 'escape' syntax. Refer to the following postgres link
	 * for details:
	 * http://www.postgresql.org/docs/8.3/static/functions-string.html
	 */
	snprintf(conn->conn_sql, MAX_SQL_LENGTH, "insert into "
		"pbs.job_scr (ji_jobid, script) "
		"values "
		"($1, encode($2, 'escape'))");
	if (pg_prepare_stmt(conn, STMT_INSERT_JOBSCR, conn->conn_sql, 2) != 0)
		return -1;

	/*
	 * Use the sql decode function to decode the script parameter. Decode
	 * using 'escape' mode. Decode considers script as encoded TEXT and
	 * decodes it using 'escape' syntax, returning a bytea. The :: is used
	 * to "typecast" the output to a bytea.
	 * Refer to the following postgres link for details:
	 * http://www.postgresql.org/docs/8.3/static/functions-string.html
	 */
	snprintf(conn->conn_sql, MAX_SQL_LENGTH, "select decode(script, 'escape')::bytea as script "
		"from pbs.job_scr "
		"where ji_jobid = $1");
	if (pg_prepare_stmt(conn, STMT_SELECT_JOBSCR, conn->conn_sql, 1) != 0)
		return -1;

	snprintf(conn->conn_sql, MAX_SQL_LENGTH, "select "
		"ji_jobid,"
		"ji_state,"
		"ji_substate,"
		"ji_svrflags,"
		"ji_numattr,"
		"ji_ordering,"
		"ji_priority,"
		"ji_stime,"
		"ji_endtBdry,"
		"ji_queue,"
		"ji_destin,"
		"ji_un_type,"
		"ji_momaddr,"
		"ji_momport,"
		"ji_exitstat,"
		"ji_quetime,"
		"ji_rteretry,"
		"ji_fromsock,"
		"ji_fromaddr,"
		"ji_4jid,"
		"ji_4ash,"
		"ji_credtype,"
		"ji_qrank,"
		"extract(epoch from ji_savetm)::bigint as ji_savetm, "
		"extract(epoch from ji_creattm)::bigint as ji_creattm, "
		"hstore_to_array(attributes) as attributes "
		"from pbs.job order by ji_qrank");
	if (pg_prepare_stmt(conn, STMT_FINDJOBS_ORDBY_QRANK, conn->conn_sql, 0) != 0)
		return -1;

	snprintf(conn->conn_sql, MAX_SQL_LENGTH, "select "
		"ji_jobid,"
		"ji_state,"
		"ji_substate,"
		"ji_svrflags,"
		"ji_numattr,"
		"ji_ordering,"
		"ji_priority,"
		"ji_stime,"
		"ji_endtBdry,"
		"ji_queue,"
		"ji_destin,"
		"ji_un_type,"
		"ji_momaddr,"
		"ji_momport,"
		"ji_exitstat,"
		"ji_quetime,"
		"ji_rteretry,"
		"ji_fromsock,"
		"ji_fromaddr,"
		"ji_4jid,"
		"ji_4ash,"
		"ji_credtype,"
		"ji_qrank,"
		"extract(epoch from ji_savetm)::bigint as ji_savetm, "
		"extract(epoch from ji_creattm)::bigint as ji_creattm, "
		"hstore_to_array(attributes) as attributes "
		"from pbs.job where ji_queue = $1"
		" order by ji_qrank");
	if (pg_prepare_stmt(conn, STMT_FINDJOBS_BYQUE_ORDBY_QRANK,
		conn->conn_sql, 1) != 0)
		return -1;

	snprintf(conn->conn_sql, MAX_SQL_LENGTH, "delete from pbs.job where ji_jobid = $1");
	if (pg_prepare_stmt(conn, STMT_DELETE_JOB, conn->conn_sql, 1) != 0)
		return -1;

	snprintf(conn->conn_sql, MAX_SQL_LENGTH, "delete from pbs.job_scr where ji_jobid = $1");
	if (pg_prepare_stmt(conn, STMT_DELETE_JOBSCR, conn->conn_sql, 1) != 0)
		return -1;

	return 0;
}

/**
 * @brief
 *	Load job data from the row into the job object
 *
 * @param[in]	res - Resultset from an earlier query
 * @param[out]  pj  - Job object to load data into
 * @param[in]	row - The current row to load within the resultset
 *
 * @return error code
 * @retval 0 Success
 * @retval -1 Error
 *
 */
static int
load_job(const  PGresult *res, pbs_db_job_info_t *pj, int row)
{
	char *raw_array;
	static int ji_jobid_fnum, ji_state_fnum, ji_substate_fnum, ji_svrflags_fnum, ji_numattr_fnum,
	ji_ordering_fnum, ji_priority_fnum, ji_stime_fnum, ji_endtBdry_fnum, ji_queue_fnum, ji_destin_fnum,
	ji_un_type_fnum, ji_momaddr_fnum, ji_momport_fnum, ji_exitstat_fnum, ji_quetime_fnum, ji_rteretry_fnum,
	ji_fromsock_fnum, ji_fromaddr_fnum, ji_4jid_fnum, ji_4ash_fnum, ji_credtype_fnum, ji_qrank_fnum,
	ji_savetm_fnum, ji_creattm_fnum, attributes_fnum;

	static int fnums_inited = 0;

	if (fnums_inited == 0) {
		/* cache the column numbers of various job table fields */
		ji_jobid_fnum = PQfnumber(res, "ji_jobid");
		ji_state_fnum = PQfnumber(res, "ji_state");
		ji_substate_fnum = PQfnumber(res, "ji_substate");
		ji_svrflags_fnum = PQfnumber(res, "ji_svrflags");
		ji_numattr_fnum = PQfnumber(res, "ji_numattr");
		ji_ordering_fnum = PQfnumber(res, "ji_ordering");
		ji_priority_fnum = PQfnumber(res, "ji_priority");
		ji_stime_fnum = PQfnumber(res, "ji_stime");
		ji_endtBdry_fnum = PQfnumber(res, "ji_endtBdry");
		ji_queue_fnum = PQfnumber(res, "ji_queue");
		ji_destin_fnum = PQfnumber(res, "ji_destin");
		ji_un_type_fnum = PQfnumber(res, "ji_un_type");
		ji_momaddr_fnum = PQfnumber(res, "ji_momaddr");
		ji_momport_fnum = PQfnumber(res, "ji_momport");
		ji_exitstat_fnum = PQfnumber(res, "ji_exitstat");
		ji_quetime_fnum = PQfnumber(res, "ji_quetime");
		ji_rteretry_fnum = PQfnumber(res, "ji_rteretry");
		ji_fromsock_fnum = PQfnumber(res, "ji_fromsock");
		ji_fromaddr_fnum = PQfnumber(res, "ji_fromaddr");
		ji_4jid_fnum = PQfnumber(res, "ji_4jid");
		ji_4ash_fnum = PQfnumber(res, "ji_4ash");
		ji_qrank_fnum = PQfnumber(res, "ji_qrank");
		ji_credtype_fnum = PQfnumber(res, "ji_credtype");
		ji_savetm_fnum = PQfnumber(res, "ji_savetm");
		ji_creattm_fnum = PQfnumber(res, "ji_creattm");
		attributes_fnum = PQfnumber(res, "attributes");
		fnums_inited = 1;
	}

	GET_PARAM_STR(res, row, pj->ji_jobid, ji_jobid_fnum);
	GET_PARAM_INTEGER(res, row, pj->ji_state, ji_state_fnum);
	GET_PARAM_INTEGER(res, row, pj->ji_substate, ji_substate_fnum);
	GET_PARAM_INTEGER(res, row, pj->ji_svrflags, ji_svrflags_fnum);
	GET_PARAM_INTEGER(res, row, pj->ji_numattr, ji_numattr_fnum);
	GET_PARAM_INTEGER(res, row, pj->ji_ordering, ji_ordering_fnum);
	GET_PARAM_INTEGER(res, row, pj->ji_priority, ji_priority_fnum);
	GET_PARAM_BIGINT(res, row, pj->ji_stime, ji_stime_fnum);
	GET_PARAM_BIGINT(res, row, pj->ji_endtBdry, ji_endtBdry_fnum);
	GET_PARAM_STR(res, row, pj->ji_queue, ji_queue_fnum);
	GET_PARAM_STR(res, row, pj->ji_destin, ji_destin_fnum);
	GET_PARAM_INTEGER(res, row, pj->ji_un_type, ji_un_type_fnum);
	GET_PARAM_BIGINT(res, row, pj->ji_momaddr, ji_momaddr_fnum);
	GET_PARAM_INTEGER(res, row, pj->ji_momport, ji_momport_fnum);
	GET_PARAM_INTEGER(res, row, pj->ji_exitstat, ji_exitstat_fnum);
	GET_PARAM_BIGINT(res, row, pj->ji_quetime, ji_quetime_fnum);
	GET_PARAM_BIGINT(res, row, pj->ji_rteretry, ji_rteretry_fnum);
	GET_PARAM_INTEGER(res, row, pj->ji_fromsock, ji_fromsock_fnum);
	GET_PARAM_BIGINT(res, row, pj->ji_fromaddr, ji_fromaddr_fnum);
	GET_PARAM_STR(res, row, pj->ji_4jid, ji_4jid_fnum);
	GET_PARAM_STR(res, row, pj->ji_4ash, ji_4ash_fnum);
	GET_PARAM_INTEGER(res, row, pj->ji_credtype, ji_credtype_fnum);
	GET_PARAM_INTEGER(res, row, pj->ji_qrank, ji_qrank_fnum);
	GET_PARAM_BIGINT(res, row, pj->ji_savetm, ji_savetm_fnum);
	GET_PARAM_BIGINT(res, row, pj->ji_creattm, ji_creattm_fnum);
	GET_PARAM_BIN(res, row, raw_array, attributes_fnum);

	/* convert attributes from postgres raw array format */
	return (convert_array_to_db_attr_list(raw_array, &pj->attr_list));
}

/**
 *@brief
 *	Save (insert/update) a new/existing job
 *
 * @param[in]	conn - The connnection handle
 * @param[in]	obj  - The job object to save
 * @param[in]	savetype - PBS_UPDATE_DB_QUICK/PBS_UPDATE_DB_FULL/PBS_INSERT_DB
 *
 * @return      Error code
 * @retval	-1 - Execution of prepared statement failed
 * @retval	 0 - Success and > 0 rows were affected
 * @retval	 1 - Execution succeeded but statement did not affect any rows
 *
 */
int
pg_db_save_job(pbs_db_conn_t *conn, pbs_db_obj_info_t *obj, int savetype)
{
	char *stmt;
	pbs_db_job_info_t *pjob = obj->pbs_db_un.pbs_db_job;
	int params;
	int rc;
	char *raw_array = NULL;

	SET_PARAM_STR(conn, pjob->ji_jobid, 0);
	SET_PARAM_INTEGER(conn, pjob->ji_state, 1);
	SET_PARAM_INTEGER(conn, pjob->ji_substate, 2);
	SET_PARAM_INTEGER(conn, pjob->ji_svrflags, 3);
	SET_PARAM_INTEGER(conn, pjob->ji_numattr, 4);
	SET_PARAM_INTEGER(conn, pjob->ji_ordering, 5);
	SET_PARAM_INTEGER(conn, pjob->ji_priority, 6);
	SET_PARAM_BIGINT(conn, pjob->ji_stime, 7);
	SET_PARAM_BIGINT(conn, pjob->ji_endtBdry, 8);
	SET_PARAM_STR(conn, pjob->ji_queue, 9);
	SET_PARAM_STR(conn, pjob->ji_destin, 10);
	SET_PARAM_INTEGER(conn, pjob->ji_un_type, 11);
	SET_PARAM_BIGINT(conn, pjob->ji_momaddr, 12);
	SET_PARAM_INTEGER(conn, pjob->ji_momport, 13);
	SET_PARAM_INTEGER(conn, pjob->ji_exitstat, 14);
	SET_PARAM_BIGINT(conn, pjob->ji_quetime, 15);
	SET_PARAM_BIGINT(conn, pjob->ji_rteretry, 16);
	SET_PARAM_INTEGER(conn, pjob->ji_fromsock, 17);
	SET_PARAM_BIGINT(conn, pjob->ji_fromaddr, 18);
	SET_PARAM_STR(conn, pjob->ji_4jid, 19);
	SET_PARAM_STR(conn, pjob->ji_4ash, 20);
	SET_PARAM_INTEGER(conn, pjob->ji_credtype, 21);
	SET_PARAM_INTEGER(conn, pjob->ji_qrank, 22);

	if (savetype == PBS_UPDATE_DB_QUICK) {
		params = 23;
	} else {
		int len = 0;
		/* convert attributes to postgres raw array format */
		if ((len = convert_db_attr_list_to_array(&raw_array, &pjob->attr_list)) <= 0)
			return -1;

		SET_PARAM_BIN(conn, raw_array, len, 23);
		params = 24;
	}

	if (savetype == PBS_UPDATE_DB_FULL)
		stmt = STMT_UPDATE_JOB;
	else if (savetype == PBS_UPDATE_DB_QUICK)
		stmt = STMT_UPDATE_JOB_QUICK;
	else
		stmt = STMT_INSERT_JOB;

	rc = pg_db_cmd(conn, stmt, params);

	free(raw_array);

	return rc;
}

/**
 * @brief
 *	Load job data from the database
 *
 * @param[in]	conn - Connection handle
 * @param[in/out]obj  - Load job information into this object where
 *			jobid = obj->pbs_db_un.pbs_db_job->ji_jobid
 *
 * @return      Error code
 * @retval	-1 - Failure
 * @retval	 0 - Success
 * @retval	 1 -  Success but no rows loaded
 *
 */
int
pg_db_load_job(pbs_db_conn_t *conn, pbs_db_obj_info_t *obj)
{
	PGresult *res;
	int rc;
	pbs_db_job_info_t *pj = obj->pbs_db_un.pbs_db_job;

	SET_PARAM_STR(conn, pj->ji_jobid, 0);

	if ((rc = pg_db_query(conn, STMT_SELECT_JOB, 1, &res)) != 0)
		return rc;

	rc = load_job(res, pj, 0);

	PQclear(res);
	return rc;
}

/**
 * @brief
 *	Find jobs
 *
 * @param[in]	conn - Connection handle
 * @param[out]  st   - The cursor state variable updated by this query
 * @param[in]	obj  - Information of job to be found
 * @param[in]	opts - Any other options (like flags, timestamp)
 *
 * @return      Error code
 * @retval	-1 - Failure
 * @retval	 0 - Success
 * @retval	 1 -  Success but no rows found
 *
 */
int
pg_db_find_job(pbs_db_conn_t *conn, void *st, pbs_db_obj_info_t *obj,
	pbs_db_query_options_t *opts)
{
	PGresult *res;
	pg_query_state_t *state = (pg_query_state_t *) st;
	pbs_db_job_info_t *pdjob = obj->pbs_db_un.pbs_db_job;
	int rc;
	int params;

	if (!state)
		return -1;

	if (opts != NULL && opts->flags == FIND_JOBS_BY_QUE) {
		SET_PARAM_STR(conn, pdjob->ji_queue, 0);
		params=1;
		strcpy(conn->conn_sql, STMT_FINDJOBS_BYQUE_ORDBY_QRANK);
	} else {
		strcpy(conn->conn_sql, STMT_FINDJOBS_ORDBY_QRANK);
		params=0;
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
 *	Get the next job from the cursor
 *
 * @param[in]	conn - Connection handle
 * @param[in]	st   - The cursor state
 * @param[out]  obj  - Job information is loaded into this object
 *
 * @return      Error code
 * @retval	-1 - Failure
 * @retval	 0 - Success
 *
 */
int
pg_db_next_job(pbs_db_conn_t *conn, void *st, pbs_db_obj_info_t *obj)
{
	pg_query_state_t *state = (pg_query_state_t *) st;

	return load_job(state->res, obj->pbs_db_un.pbs_db_job, state->row);
}

/**
 * @brief
 *	Delete the job from the database
 *
 * @param[in]	conn - Connection handle
 * @param[in]	obj  - Job information
 *
 * @return      Error code
 * @retval	-1 - Failure
 * @retval	 0 - Success
 * @retval	 1 -  Success but no rows deleted
 *
 */
int
pg_db_delete_job(pbs_db_conn_t *conn, pbs_db_obj_info_t *obj)
{
	pbs_db_job_info_t *pj = obj->pbs_db_un.pbs_db_job;
	int rc = 0;

	SET_PARAM_STR(conn, pj->ji_jobid, 0);

	if (pbs_db_begin_trx(conn, 0, 0) != 0)
		goto err;

	if ((rc = pg_db_cmd(conn, STMT_DELETE_JOB, 1)) == -1)
		goto err;

	if (pg_db_cmd(conn, STMT_DELETE_JOBSCR, 1) == -1)
		goto err;


	if (pbs_db_end_trx(conn, PBS_DB_COMMIT) != 0)
		goto err;

	return rc;
err:
	(void) pbs_db_end_trx(conn, PBS_DB_ROLLBACK);
	return -1;
}

/**
 * @brief
 *	Insert job script
 *
 * @param[in]	conn - Connection handle
 * @param[in]	obj  - Job script object
 * @param[in]	savetype - Just a place holder here. Maintained the same prototype as with
 * 		           the other database save functions since this is called through function pointer.
 *
 * @return      Error code
 * @retval	-1 - Failure
 * @retval	 0 - Success
 *
 */
int
pg_db_save_jobscr(pbs_db_conn_t *conn, pbs_db_obj_info_t *obj, int savetype)
{
	pbs_db_jobscr_info_t *pscr = obj->pbs_db_un.pbs_db_jobscr;

	SET_PARAM_STR(conn, pscr->ji_jobid, 0);

	/*
	 * The script data could contain non-UTF8 characters. We therefore
	 * consider it binary and encode it into TEXT by using the "encode"
	 * sql function. The input data to load, therefore, is binary data
	 * and so we use the function "LOAD_BIN" to load the parameter to
	 * the prepared statement
	 */
	SET_PARAM_BIN(conn, pscr->script, (pscr->script)?strlen(pscr->script):0, 1);

	if ((pg_db_cmd(conn, STMT_INSERT_JOBSCR, 2)) != 0)
		return -1;

	return 0;
}

/**
 * @brief
 *	load job script
 *
 * @param[in]	  conn - Connection handle
 * @param[in/out] obj  - Job script is loaded into this object
 *
 * @return      Error code
 * @retval	-1 - Failure
 * @retval	 0 - Success
 *
 */
int
pg_db_load_jobscr(pbs_db_conn_t *conn, pbs_db_obj_info_t *obj)
{
	PGresult *res;
	pbs_db_jobscr_info_t *pscr = obj->pbs_db_un.pbs_db_jobscr;
	static int script_fnum = -1;

	SET_PARAM_STR(conn, pscr->ji_jobid, 0);

	/*
	 * The data (script) we stored was a "encoded" binary. We "decode" it
	 * back while reading, giving us "binary" data. Since we want the
	 * result data to be returned in binary, we set conn_result_format
	 * to 1 to indicate binary result. This setting is a one-time,
	 * auto-reset switch which resets to 0 (TEXT) mode after each execution
	 * of pg_db_query.
	 *
	 */
	if (pg_db_query(conn, STMT_SELECT_JOBSCR, 1, &res) != 0)
		return -1;

	if (script_fnum == -1)
		script_fnum = PQfnumber(res, "script");

	GET_PARAM_BIN(res, 0, pscr->script, script_fnum);

	return 0;
}



/**
 * @brief
 *	Deletes attributes of a job
 *
 * @param[in]	conn - Connection handle
 * @param[in]	obj  - Job information
 * @param[in]	obj_id  - Job id
 * @param[in]	attr_list - List of attributes
 *
 * @return      Error code
 * @retval	 0 - Success
 * @retval	-1 - On Failure
 *
 */
int
pg_db_del_attr_job(pbs_db_conn_t *conn, pbs_db_obj_info_t *obj, void *obj_id, pbs_db_attr_list_t *attr_list)
{
	char *raw_array = NULL;
	int len = 0;

	if ((len = convert_db_attr_list_to_array(&raw_array, attr_list)) <= 0)
		return -1;
	SET_PARAM_STR(conn, obj_id, 0);

	SET_PARAM_BIN(conn, raw_array, len, 1);

	if (pg_db_cmd(conn, STMT_REMOVE_JOBATTRS, 2) != 0)
		return -1;

	free(raw_array);

	return 0;
}

/**
 * @brief
 *	Frees allocate memory of an Object
 *
 * @param[in]	obj - pbs_db_obj_info_t containing the DB object
 *
 * @return      None
 *
 */
void
pg_db_reset_job(pbs_db_obj_info_t *obj)
{
	free_db_attr_list(&(obj->pbs_db_un.pbs_db_job->attr_list));
}

