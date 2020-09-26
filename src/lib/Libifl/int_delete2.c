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
 * @file	int_manager.c
 *
 * @brief
 * The function that underlies most of the job manipulation
 * routines...
 */

#include <pbs_config.h>   /* the master config generated by configure */

#include <stdio.h>
#include "libpbs.h"
#include "pbs_ecl.h"


/**
 * @brief
 *	-send manager request and read reply.
 *
 * @param[in] c - communication handle
 * @param[in] function - req type
 * @param[in] command - command
 * @param[in] objtype - object type
 * @param[in] objname - object name
 * @param[in] aoplp - attribute list
 * @param[in] extend - extend string for req
 *
 * @return	int
 * @retval	0	success
 * @retval	!0	error
 *
 */
struct batch_deljob_status *
PBSD_delete2(int c, int function, int command, int objtype, char *objname, struct attropl *aoplp, char *extend)
{
	int i;
	struct batch_reply *reply;
	struct batch_deljob_status *rbsp = NULL;

	/* initialize the thread context data, if not initialized */
	if (pbs_client_thread_init_thread_context() != 0)
		return NULL;


	/* now verify the attributes, if verification is enabled */
	if ((pbs_verify_attributes(c, function, objtype,
		command, aoplp)) != 0)
		return NULL;

	/* lock pthread mutex here for this connection */
	/* blocking call, waits for mutex release */
	if (pbs_client_thread_lock_connection(c) != 0)
		return NULL;

	/* send the manage request */
	i = PBSD_mgr_put(c, function, command, objtype, objname, aoplp, extend, PROT_TCP, NULL);
	if (i) {
		(void)pbs_client_thread_unlock_connection(c);
		return NULL;
	}

	/* read reply from stream into presentation element */
	reply = PBSD_rdrpy(c);

	if (reply == NULL) {
		pbs_errno = PBSE_PROTOCOL;
	} else if (reply->brp_choice != BATCH_REPLY_CHOICE_NULL  &&
		reply->brp_choice != BATCH_REPLY_CHOICE_Text &&
		reply->brp_choice != BATCH_REPLY_CHOICE_Delete) {
		pbs_errno = PBSE_PROTOCOL;
	} 
	
	if ((reply != NULL) && (reply->brp_un.brp_delstatc != NULL)) {
		rbsp = reply->brp_un.brp_delstatc;
		reply->brp_un.brp_delstatc = NULL;
	}
	
	PBSD_FreeReply(reply);

	/* unlock the thread lock and update the thread context data */
	if (pbs_client_thread_unlock_connection(c) != 0)
		return NULL;

	return rbsp;
}
