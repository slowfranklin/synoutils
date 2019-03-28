//Copyright (c) 2000-2015 Synology Inc. All rights reserved.
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <error_int.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <synocontentsearchutils/connection_cwrap.h>
#include <synodaemon/io_utils.h>

#ifndef SOCK_PATH
#define SOCK_PATH "/var/run/synoelasticd.sock"
#endif

#ifndef CONN_TIMEOUT_SEC
#define CONN_TIMEOUT_SEC 20
#endif

int SYNOQueryConnectionInit(SYNOQueryConnection **connection)
{
	int ret = CONNECTION_ERR_NONE;
	sockaddr_un address;
	struct timeval timeout;

	signal(SIGPIPE, SIG_IGN);
	/* Alloc connection */
	*connection = (SYNOQueryConnection *)malloc(sizeof(SYNOQueryConnection));
	if (NULL == *connection) {
		SYSLOG(LOG_ERR, "malloc() failed");
		ret = CONNECTION_ERR_MALLOC_CONNECTION;
		goto Error;
	}
	/* Create domain socket*/
	(*connection)->sockfd = socket(PF_UNIX, SOCK_STREAM, 0);
	if ((*connection)->sockfd < 0) {
		SYSLOG(LOG_ERR, "socket() failed");
		ret = CONNECTION_ERR_SOCKET;
		goto Error;
	}
	/* Connection to synocontentsearch daemon */
	memset(&address, 0, sizeof(struct sockaddr_un));
	address.sun_family = AF_UNIX;
	snprintf(address.sun_path, strlen(SOCK_PATH) + 1, SOCK_PATH);
	if (connect((*connection)->sockfd, (struct sockaddr *) &address,
				sizeof(struct sockaddr_un)) != 0) {
		SYSLOG(LOG_ERR,"Connect domain socket failed, %m");
		ret = CONNECTION_ERR_SOCKET;
		goto Error;
	}
	/* Set sokcet timeout info */
	timeout.tv_sec = CONN_TIMEOUT_SEC;
	timeout.tv_usec = 0;
	if (0 > setsockopt((*connection)->sockfd, SOL_SOCKET,
						SO_RCVTIMEO, (char *)&timeout, sizeof(timeout))) {
		SYSLOG(LOG_ERR,"Failed to set socket recv timeout, %m");
		ret = CONNECTION_ERR_SOCKET;
		goto Error;
	}
	if (0 > setsockopt((*connection)->sockfd, SOL_SOCKET,
						SO_SNDTIMEO, (char *)&timeout, sizeof(timeout))) {
		SYSLOG(LOG_ERR,"Failed to set socket send timeout, %m");
		ret = CONNECTION_ERR_SOCKET;
		goto Error;
	}
	/* Set FD_SET */
	FD_ZERO(&((*connection)->readFdSet));
	FD_SET((*connection)->sockfd, (&(*connection)->readFdSet));
Error:
	return ret;
}
