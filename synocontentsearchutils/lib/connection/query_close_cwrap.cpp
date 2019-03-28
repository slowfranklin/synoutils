//Copyright (c) 2000-2015 Synology Inc. All rights reserved.
#include <stdlib.h>
#include <sys/socket.h>
#include <synocontentsearchutils/connection_cwrap.h>
#include <synodaemon/io_utils.h>

void SYNOQueryConnectionClose(SYNOQueryConnection *connection)
{
	if (connection) {
		if (0 <= connection->sockfd){
			close(connection->sockfd);
		}
		free(connection);
	}
}
