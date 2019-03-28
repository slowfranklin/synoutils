//Copyright (c) 2000-2015 Synology Inc. All rights reserved.
#include <json/value.h>
#include <string.h>
#include <synocontentsearchutils/connection_cwrap.h>
#include <string>
#include <error_int.h>
#include <synodaemon/io_utils.h>

SYNOSearchRecord SYNOQueryFetchNext(SYNOQueryConnection *connection, struct timeval timeout)
{
	SYNOSearchRecord rec;

	/* Init Value */
	memset(&rec, 0, sizeof(rec));

	/* Check Parameters */
	if (NULL == connection) {
		SYSLOG(LOG_ERR, "connection is NULL. %m");
		goto Error;
	}

	/* Set FD_SET */
	FD_ZERO(&(connection->readFdSet));
	FD_SET(connection->sockfd, &(connection->readFdSet));

	/* Polling */
	if (0 > select(FD_SETSIZE, &(connection->readFdSet), NULL, NULL, &timeout)) {
		SYSLOG(LOG_ERR, "select get error %m");
		goto Error;
	}

	if (FD_ISSET(connection->sockfd, &(connection->readFdSet))) {
		SYNOQueryGetNextRecord(connection, &rec);
	}
Error:
	return rec;
}
