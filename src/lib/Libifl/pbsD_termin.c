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
 * @file	pbs_terminate.c
 */

#include <pbs_config.h>   /* the master config generated by configure */

#include <string.h>
#include <stdio.h>
#include "libpbs.h"
#include "dis.h"
#include "pbs_ecl.h"


/**
 * @brief
 *	-send termination batch_request to the given server.
 *
 * @param[in] c - communication handle
 * @param[in] manner - manner in which server to be terminated
 * @param[in] extend - extension string for request
 *
 * @return	int
 * @retval	0		success
 * @retval	pbs_error	error
 *
 */

int
send_terminate(int c, int sock, int manner, char *extend)
{
	struct batch_reply *reply;
	int rc = 0;
	
	/* setup DIS support routines for following DIS calls */

	DIS_tcp_funcs();

	if ((rc=encode_DIS_ReqHdr(sock, PBS_BATCH_Shutdown, pbs_current_user)) ||
		(rc = encode_DIS_ShutDown(sock, manner)) ||
		(rc = encode_DIS_ReqExtend(sock, extend))) {
		return PBSE_PROTOCOL;
	}
	if (dis_flush(sock)) {
		pbs_errno = PBSE_PROTOCOL;
		return pbs_errno;
	}

	/* read in reply */

	reply = PBSD_rdrpy_sock(sock, &rc);

	if (set_conn_errno(c, reply->brp_code) != 0) {
		pbs_errno = reply->brp_code;
		return pbs_errno;
	}
	rc = reply->brp_code;

	if (reply->brp_choice == BATCH_REPLY_CHOICE_Text) {
		if (reply->brp_un.brp_txt.brp_str != NULL) {
			if (set_conn_errtxt(c, reply->brp_un.brp_txt.brp_str) != 0) {
				pbs_errno = PBSE_SYSTEM;
				return pbs_errno;
			}
		}
	}

	PBSD_FreeReply(reply);

	return rc;
}


/**
 * @brief
 *	-send termination batch_request to server.
 *
 * @param[in] c - communication handle
 * @param[in] manner - manner in which server to be terminated
 * @param[in] extend - extension string for request
 *
 * @return	int
 * @retval	0		success
 * @retval	pbs_error	error
 *
 */
int
__pbs_terminate(int c, int manner, char *extend)
{
	int rc = 0;
	int i;
	int sock;
	int count = 0;
	shard_conn_t **shard_connection = NULL;

	/* initialize the thread context data, if not already initialized */
	if (pbs_client_thread_init_thread_context() != 0)
		return pbs_errno;

	/* lock pthread mutex here for this connection */
	/* blocking call, waits for mutex release */
	if (pbs_client_thread_lock_connection(c) != 0)
		return pbs_errno;


	/* send request to all servers that are up */
	if (get_max_servers() > 1) {
		int errd = 0;
		int rc_errd = 0;
		shard_connection = (shard_conn_t **)get_conn_shards(c);
		if (!shard_connection)
			return PBSE_NOSERVER;
		for (i = 0; i < get_current_servers(); i++) {
			if (shard_connection[i]->state == SHARD_CONN_STATE_CONNECTED)
				sock = shard_connection[i]->sd;
			else {
				/* attempt connection to instance */
				sock = tcp_connect(c, pbs_conf.psi[i]->name, pbs_conf.psi[i]->port, NULL);
			}
			if (sock != -1) {
				if ((rc = send_terminate(c, sock, manner, extend)) != 0) {
					errd++;
					rc_errd = rc;
				} else
					count++;
			}
		}
		if (count == 0) {
			/* could not send to any servers, all down */
			rc = PBSE_NOCONNECTION;
		}
		if (errd > 0) {
			rc = rc_errd;
		}
	} else {
		rc = send_terminate(c, c, manner, extend);
	}

	if (rc == PBSE_PROTOCOL) {
		if (set_conn_errtxt(c, dis_emsg[rc]) != 0) {
			pbs_errno = PBSE_SYSTEM;
		} else {
			pbs_errno = PBSE_PROTOCOL;
		}
	}

	/* unlock the thread lock and update the thread context data */
	if (pbs_client_thread_unlock_connection(c) != 0)
		return pbs_errno;

	return rc;
}
