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
 * @file    pbs_ds_password.c
 *
 * @brief
 *      This is a tool to allow the admin to change the database password.
 *	This file uses the Libaes (AES) encryption to encrypt the chosen
 *	password to file $PBS_HOME/server_priv/db_password.
 *
 * @par	This tool has two modes.
 *	-r - No password is asked from the user. A random password is generated
 *	and set to the database, then the password is encrypted using AES
 *	encryption and stored in the above mentioned location. This option
 *	is used by the PBS installer to generate and set an initial password
 *	for the database.
 *
 *	-C <username>- Change the data-service account name that PBS uses to access
 *	the data-service. If the user name specified is different from what is listed
 *	in pbs.conf file, then pbs_ds_password asks the user to confirm whether
 *	he/she really intends to change the data-service user. On Unix, the user-name
 *	supplied must be an existing non-root system user. pbs_ds_password will
 *	check to ensure that the user is non-root.
 *	If the admin wishes to change the data-service user, then pbs_ds_password
 *	will also prompt the user to enter the password to be set for this new user.
 *	pbs_ds_password then creates the new user as a superuser in the database,
 *	and sets the chosen password. It then updates the db_usr file in
 *	server_priv with the new data service user name. On Unix,
 *	pbs_ds_password displays a reminder to the user to run "pbs_probe -f "
 *	command to "fix" the change in ownership of the files related to the data-service.
 *
 *	No options: This is the interactive mode. In this mode, the tool asks the
 *	user to enter a password twice. If both the passwords match then the tool
 *	sets the password to the database and stores the encrypted password
 *	in the above mentioned location.
 *
 *	Changes can be made only when the pbs data-service is running. This
 *	can be done when pbs_server is running (which means data-service is also
 *	running), or if pbs_server is down, the admin can start the data-service and
 *	then run this command.
 *
 *	This tool uses the usual way to connect to the database, which means to
 *	change the database it has to first authenticate with the database with the
 *	currently set password. The connect_db function it calls, automatically
 *	uses the current password from $PBS_HOME/server_priv/db_password
 *	to connect to the database.
 *
 *	The tool attempts to connect to the data-service running on the localhost
 *	only. Thus this tool can be used only from the same host that is running the
 *	pbs_dataservice. (For example, in failover scenario, this tool needs to be
 *	invoked from the same host which is currently running the data-service)
 *
 */

#include <pbs_config.h>   /* the master config generated by configure */
#include <pbs_version.h>
#include <assert.h>
#include <pwd.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libgen.h>
#include <dirent.h>
#include <errno.h>

#include "libpbs.h"
#include "portability.h"

#include "ticket.h"

#include "server_limits.h"
#include "pbs_db.h"

#ifndef LOGIN_NAME_MAX
#define LOGIN_NAME_MAX 256
#endif


int	cred_type;
size_t	cred_len;
char	*cred_buf = NULL;

int started_db = 0;
pbs_db_conn_t *conn = NULL;
struct passwd	*pwent;
char pwd_file_new[MAXPATHLEN+1];

extern unsigned char pbs_aes_key[][16];
extern unsigned char pbs_aes_iv[][16];

/**
 * @brief
 *	At exit handler to close database connection,
 *	stop database if this program had started it,
 *	and to remove the temp password file, if it
 *	was created
 *
 * @return	Void
 *
 */
static void
cleanup()
{
	char *db_err = NULL;

	if (pwd_file_new[0] != 0)
		unlink(pwd_file_new);

	if (conn != NULL) {
		pbs_db_disconnect(conn);
		pbs_db_destroy_connection(conn);
		conn = NULL;
	}
	if (started_db  == 1) {
		if (pbs_shutdown_db(&db_err) != 0) {
			fprintf(stderr, "Failed to stop PBS Data Service");
			if (db_err) {
				fprintf(stderr, ":[%s]", db_err);
				free(db_err);
			}
			fprintf(stderr, "\n");
		}
		started_db = 0;
	}
}

#define MAX_PASSWORD_LEN 256

/**
 * @brief
 *	Accepts a password string without echoing characters
 *	on the screen
 *
 * @param[out]	passwd - password read from user
 *
 * @return - Error code
 * @retval  -1 - Failure
 * @retval   0 - Success
 *
 */
static int
read_password(char *passwd)
{
	int len;
	char *p;

	if (system("stty -echo") != 0)
		return -1;

	fgets(passwd, MAX_PASSWORD_LEN, stdin);

	if (system("stty echo") != 0)
		return -1;

	len = strlen(passwd);
	p = passwd + len - 1;
	while (*p == '\r' || *p == '\n')
		p--;
	*(p + 1) = 0;

	return 0;
}

/**
 * @brief
 *	Generates a random password for the database
 *	The allowed_chars array contains a list of
 *	characters acceptable for the password. This
 *	function uses srand to randomize the seed on the
 *	current timestamp. Then it uses rand to select
 *	a random character from the array to add to the
 *	password string.
 *
 * @param[out]	passwd - password generated
 * @param[in] len - Length of password to be generated
 *
 * @return - Error code
 * @retval  -1 - Failure
 * @retval   0 - Success
 *
 */
static int
gen_password(char *passwd, int len)
{
	int chrs = 0;
	char allowed_chars[] = "0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ!@#$%^&*()_+";
	int arr_len = strlen(allowed_chars);

	sleep(1); /* sleep 1 second to ensure the srand on time(0) truely randomizes the seed */
	srand(time(0));
	while (chrs < len) {
		int c;
		c = (char)(rand() % arr_len);
		passwd[chrs++] = allowed_chars[c];
	}
	passwd[chrs] = '\0';
	return 0;
}

/**
 * @brief
 *	Updates the db_usr file in server_priv with
 *	the new data service user name.
 *
 * @param[in] file - The db_usr file
 * @param[in] userid - The new data service user to be set
 *
 * @return - Error code
 * @retval  -1 - Failure
 * @retval   0 - Success
 *
 */
int
update_db_usr(char *file, char *userid)
{
	int fd;
	int rc = 0;

	if ((fd = open(file, O_CREAT | O_TRUNC | O_WRONLY, 0600)) == -1) {
		fprintf(stderr, "%s: open failed, errno=%d\n", file, errno);
		return -1;
	}
	if (write(fd, userid, strlen(userid)) == -1) {
		fprintf(stderr, "%s: write failed, errno=%d\n", file, errno);
		rc = -1;
	}
	close(fd);
	return rc;
}

/**
 * @brief
 *	Checks whether given user exists on the system.
 *	On Unix, it additionally checks that the user is not
 *	the root user (id 0).
 *
 * @param[in] userid - The id to check
 *
 * @return - user suitable to use?
 * @retval  -1 - Userid not suitable for use
 * @retval   0 - Userid suitable to use
 *
 */
int
check_user(char *userid)
{
	pwent = getpwnam(userid);
	if (pwent == NULL)
		return (-1);
	if (pwent->pw_uid == 0)
		return (-1);

	/* in unix make sure that the user home dir is accessible */
	if (access(pwent->pw_dir, R_OK | W_OK | X_OK) != 0)
		return (-1);
	return 0;
}

/**
 * @brief
 *	This function changes the ownership of
 *	the whole directory tree (and files) under the
 *	pbs datastore directory to the new data service
 *	user account. This is required only in Unix.
 *	On Windows, acess is given to the admin
 *	group anyway, which allows any service_account
 *	(part of the admin group) to be able to access
 *	these directories.
 *
 * @param[out] path - Path of the datastore directory
 * @param[in] userid - The new data service user
 *				to change ownership to
 *
 * @return - Error code
 * @retval  -1 - Failure
 * @retval   0 - Success
 *
 */
int
change_ownership(char *path, char *userid)
{
	DIR	*dir;
	struct	dirent *pdirent;
	char    dirfile[MAXPATHLEN+1];
	struct stat stbuf;

	chown(path, pwent->pw_uid, (gid_t)-1);
	dir = opendir(path);
	if (dir == NULL) {
		return -1;
	}

	while (errno = 0, (pdirent = readdir(dir)) != NULL) {
		if (strcmp(pdirent->d_name, ".") == 0 ||
			strcmp(pdirent->d_name, "..") == 0)
			continue;

		sprintf(dirfile, "%s/%s", path, pdirent->d_name);
		chown(dirfile, pwent->pw_uid, (gid_t)-1);
		stat(dirfile, &stbuf);
		if (stbuf.st_mode & S_IFDIR) {
			change_ownership(dirfile, userid);
			continue;
		}
	}
	if (errno != 0 && errno != ENOENT) {
		(void)closedir(dir);
		return -1;
	}
	(void)closedir(dir);
	return 0;
}
/**
 * @brief
 *	The main function in C - entry point
 *
 * @param[in]  argc - argument count
 * @param[in]  argv - pointer to argument array
 *
 * @return  int
 * @retval  0 - success
 * @retval  !0 - error
 */
int
main(int argc, char *argv[])
{
	int i, rc;
	char passwd[MAX_PASSWORD_LEN + 1] = {'\0'};
	char passwd2[MAX_PASSWORD_LEN + 1];
	char *pquoted;
	char pwd_file[MAXPATHLEN + 1];
	char userid[LOGIN_NAME_MAX + 1];
	int fd, errflg = 0;
	int gen_pwd = 0;
	char sqlbuff[1024];
	int db_conn_error=0;
	char *db_errmsg = NULL;
	int pmode;
	int change_user = 0;
	char *olduser;
	int update_db = 0;
	char getopt_format[5];
	char prog[]="pbs_ds_password";
	char errmsg[PBS_MAX_DB_CONN_INIT_ERR + 1];

	conn = NULL;
	pwd_file_new[0]=0;

	/*test for real deal or just version and exit*/
	PRINT_VERSION_AND_EXIT(argc, argv);

	/* read configuration file */
	if (pbs_loadconf(0) == 0) {
		fprintf(stderr, "%s: Could not load pbs configuration\n", prog);
		return (-1);
	}

	/* backup old user name */
	if ((olduser = pbs_get_dataservice_usr(errmsg, PBS_MAX_DB_CONN_INIT_ERR)) == NULL) {
		fprintf(stderr, "%s: Could not retrieve current data service user\n", prog);
		if (strlen(errmsg) > 0)
			fprintf(stderr, "%s\n", errmsg);
		return (-1);
	}

	if (pbs_conf.pbs_data_service_host == NULL)
		update_db = 1;

	userid[0]=0; /* empty user id */

	strcpy(getopt_format, "rC:");

	while ((i = getopt(argc, argv, getopt_format)) != EOF) {
		switch (i) {
			case 'r':
				gen_pwd = 1;
				break;
			case 'C':
				strcpy(userid, optarg);
				break;
			case '?':
			default:
				errflg++;
		}
	}

	if (errflg) {
		fprintf(stderr, "\nusage:\t%s [-r] [-C username]\n", prog);
		fprintf(stderr, "      \t%s --version\n", prog);
		return (-1);
	}

    /* NOTE : This functionality is added just for the automation testing purpose.
     * Usage: pbs_ds_password <password>
     */
	if (argv[optind] != NULL) {
		gen_pwd = 0;
		strncpy(passwd, argv[optind], sizeof(passwd));
		passwd[sizeof(passwd) - 1] = '\0';
	}

	/* check admin privileges */
	if ((getuid() != 0) || (geteuid() != 0)) {
		fprintf(stderr, "%s: Must be run by root\n", prog);
		return (1);
	}

	change_user = 0;
	/* if the -C option was specified read the user from pbs.conf */
	if (userid[0] != 0) {
		if (strcmp(olduser, userid) != 0) {
			change_user = 1;
		}
	}

	if (change_user == 1) {
		/* check that the supplied user-id exists (and is non-root on unix) */
		if (check_user(userid) != 0) {
			fprintf(stderr, "\n%s: User-id %s does not exist/is root user/home dir is not accessible\n", prog, userid);
			return (-1);
		}
	}

	atexit(cleanup);

	if (update_db == 1) {
		/* then connect to database */
		conn = pbs_db_init_connection(NULL, PBS_DB_CNT_TIMEOUT_NORMAL, 1, &db_conn_error, errmsg, PBS_MAX_DB_CONN_INIT_ERR);
		if (!conn) {
			get_db_errmsg(db_conn_error, &db_errmsg);
			fprintf(stderr, "%s: %s\n", prog, db_errmsg);
			if (strlen(errmsg) > 0)
				fprintf(stderr, "%s\n", errmsg);
			return -1;
		}
		db_conn_error = pbs_db_connect(conn);
		if (db_conn_error == PBS_DB_SUCCESS && change_user == 1) {
			/* able to connect ? Thats bad, PBS or dataservice is running */
			fprintf(stderr, "%s: PBS Services and/or PBS Data Service is running\n", prog);
			fprintf(stderr, "                 Stop PBS and Data Services before changing Data Service user\n");
			return (-1);
		}

		if (db_conn_error != PBS_DB_SUCCESS) {
			if (db_conn_error == PBS_DB_CONNREFUSED) {
				/* start db only if it was not already running */
				if (pbs_startup_db(&db_errmsg) != 0) {
					if (db_errmsg)
						fprintf(stderr, "%s: Failed to start PBS dataservice:[%s]\n", prog, db_errmsg);
					else
						fprintf(stderr, "%s: Failed to start PBS dataservice\n", prog);
					return (-1);
				}
				started_db = 1;
			}
			db_conn_error = pbs_db_connect(conn);
			if (db_conn_error != PBS_DB_SUCCESS) {
				get_db_errmsg(db_conn_error, &db_errmsg);
				if (conn->conn_db_err)
					fprintf(stderr, "%s: Could not connect to PBS data service:%s:[%s]\n", prog,
						db_errmsg, (char*)conn->conn_db_err);
				else
					fprintf(stderr, "%s: Could not connect to PBS data service:%s\n", prog, db_errmsg);
				return (-1);
			}
		}
	}

	if (gen_pwd == 0 && passwd[0] == '\0') {
		/* ask user to enter password twice */
		printf("Enter the password:");
		read_password(passwd);

		printf("\nRe-enter the password:");
		read_password(passwd2);
		printf("\n\n");
		if (strcmp(passwd, passwd2) != 0) {
			fprintf(stderr, "Entered passwords do not match\n");
			return (-2);
		}
		if (strlen(passwd) == 0) {
			fprintf(stderr, "Blank password is not allowed\n");
			return (-2);
		}
	} else if (gen_pwd == 1) {
		gen_password(passwd, 16);
	}

	rc = pbs_encrypt_pwd(passwd, &cred_type, &cred_buf, &cred_len, (const unsigned char *) pbs_aes_key, (const unsigned char *) pbs_aes_iv);
	if (rc != 0) {
		fprintf(stderr, "%s: Failed to encrypt password\n", prog);
		return (-1);
	}

	/* escape password to use in sql strings later */
	if ((pquoted = pbs_db_escape_str(conn, passwd)) == NULL) {
		fprintf(stderr, "%s: Out of memory\n", prog);
		return -1;
	}

	sprintf(pwd_file_new, "%s/server_priv/db_password.new", pbs_conf.pbs_home_path);
	sprintf(pwd_file, "%s/server_priv/db_password", pbs_conf.pbs_home_path);

	/* write encrypted password to the password file */
	pmode = 0600;
	if ((fd = open(pwd_file_new, O_WRONLY | O_TRUNC | O_CREAT | O_Sync,
		pmode)) == -1) {
		perror("open/create failed");
		fprintf(stderr, "%s: Unable to create file %s\n", prog, pwd_file_new);
		return (-1);
	}

	if (update_db == 1) {
		/* change password only if this config option is not set */
		if (change_user == 1) {
			/* check whether user exists */
			snprintf(sqlbuff, sizeof(sqlbuff),
				"select usename from pg_user where usename = '%s'",
				userid);
			if (pbs_db_execute_str(conn, sqlbuff) == 1) {
				/* now attempt to create new user & set the database passwd to the un-encrypted password */
				snprintf(sqlbuff, sizeof(sqlbuff),
					"create user \"%s\" SUPERUSER ENCRYPTED PASSWORD '%s'",
					userid, pquoted);
			} else {
				/* attempt to alter new user & set the database passwd to the un-encrypted password */
				snprintf(sqlbuff, sizeof(sqlbuff),
					"alter user \"%s\" SUPERUSER ENCRYPTED PASSWORD '%s'",
					userid, pquoted);
			}
			memset(passwd, 0, sizeof(passwd));
			memset(passwd2, 0, sizeof(passwd2));
			memset(pquoted, 0, (sizeof(char) * strlen(pquoted)));
			if (pbs_db_execute_str(conn, sqlbuff) == -1) {
				fprintf(stderr, "%s: Failed to create/alter user id %s\n", prog, userid);
				return -1;
			}
		} else {
			/* now attempt to set the database passwd to the un-encrypted password */
			/* alter user ${user} SUPERUSER ENCRYPTED PASSWORD '${passwd}' */
			sprintf(sqlbuff, "alter user \"%s\" SUPERUSER ENCRYPTED PASSWORD '%s'",
				olduser, pquoted);
			memset(passwd, 0, sizeof(passwd));
			memset(passwd2, 0, sizeof(passwd2));
			memset(pquoted, 0, (sizeof(char) * strlen(pquoted)));
			if (pbs_db_execute_str(conn, sqlbuff) == -1) {
				fprintf(stderr, "%s: Failed to create/alter user id %s\n", prog, userid);
				return -1;
			}
		}
	}

	if (write(fd, cred_buf, cred_len) != cred_len) {
		perror("write failed");
		fprintf(stderr, "%s: Unable to write to file %s\n", prog, pwd_file_new);
		return -1;
	}
	close(fd);
	free(cred_buf);

	if (rename(pwd_file_new, pwd_file) != 0) {
		return (-1);
	}


	if (update_db == 1) {
		cleanup(); /* cleanup will disconnect and delete tmp file too */
	}

	printf("---> Updated user password\n");
	if (update_db == 1 && change_user == 1) {
		printf("---> Updated user in datastore\n");
		printf("---> Stored user password in datastore\n");
	}

	if (change_user == 1) {
		char usr_file[MAXPATHLEN + 1];
		sprintf(usr_file, "%s/server_priv/db_user", pbs_conf.pbs_home_path);

		/* update PBS_HOME/server_priv/db_user file with the new user name */
		if (update_db_usr(usr_file, userid) != 0) {
			fprintf(stderr, "Unable to update file %s\n", usr_file);
			return -1;
		}
		printf("---> Updated new user\n");
	}

	if (update_db == 1 && change_user == 1) {
		char datastore[MAXPATHLEN + 1];

		/* ownership is changed only for Unix users
		 * On windows, these files are allways owned by the user who installed the database
		 * and writable by administrators anyway
		 */
		sprintf(datastore, "%s/datastore", pbs_conf.pbs_home_path);
		/* change ownership of the datastore directories to the new user, so that db can be started again */
		if (change_ownership(datastore, userid) != 0) {
			fprintf(stderr, "%s: Failed to change ownership on path %s\n", prog, datastore);
			return -1;
		}
		printf("---> Changed ownership of %s to user %s\n", datastore, userid);

		/* reload configuration file */
		if (pbs_loadconf(1) == 0) {
			fprintf(stderr, "%s: Could not load pbs configuration\n", prog);
			return (-1);
		}

		if (pbs_startup_db(&db_errmsg) != 0) {
			if (db_errmsg)
				fprintf(stderr, "%s: Failed to start PBS dataservice as new user:[%s]\n", prog, db_errmsg);
			else
				fprintf(stderr, "%s: Failed to start PBS dataservice as new user\n", prog);
			return (-1);
		}
		started_db = 1;

		/* connect again to drop the old user */
		conn = pbs_db_init_connection(NULL, PBS_DB_CNT_TIMEOUT_NORMAL, 1, &db_conn_error, errmsg, PBS_MAX_DB_CONN_INIT_ERR);
		if (!conn) {
			get_db_errmsg(db_conn_error, &db_errmsg);
			fprintf(stderr, "%s: %s\n", prog, db_errmsg);
			if (strlen(errmsg) > 0)
				fprintf(stderr, "%s\n", errmsg);
			return -1;
		}
		db_conn_error = pbs_db_connect(conn);
		if (db_conn_error != PBS_DB_SUCCESS) {
			get_db_errmsg(db_conn_error, &db_errmsg);
			if (conn->conn_db_err)
				fprintf(stderr, "%s: Could not connect to PBS data service as new user:%s[%s]\n", prog,
					db_errmsg, (char*)conn->conn_db_err);
			else
				fprintf(stderr, "%s: Could not connect to PBS data service as new user:%s\n", prog, db_errmsg);
			return (-1);
		}
		/* delete the old user from the database */
		sprintf(sqlbuff, "drop user \"%s\"", olduser);
		pbs_db_execute_str(conn, sqlbuff);
	}
	printf("---> Success\n");

	return (0);
}
