//Copyright (c) 2000-2015 Synology Inc. All rights reserved.
#include <json/value.h>
#include <string.h>
#include <synocontentsearchutils/connection_cwrap.h>
#include <string>
#include <error_int.h>
#include <synodaemon/io_utils.h>

/*
 * Receive one search record after calling "SYNOQueryConnectionOpen"
 *
 * @param connection
 *          [IN] connection to daemon(domain socket server)
 * @param rec
 *          [OUT] search record to be filled
 * @return error code
 *
 * usage example:
 *
 * SYNOSearchRecord rec;
 *
 * for (int i=0;i<10;i++) {
 *     if(0 == SYNOQueryGetRecord(connection, rec)) {
 *         printf("%s\n",rec.szPath);
 *     } else {
 *         printf("error occured\n");
 *         break;
 *     }
 * }
 *
 */
int SYNOQueryGetNextRecord(SYNOQueryConnection *connection, SYNOSearchRecord *rec)
{
	Json::Value jsItem;
	std::string strResp = "";
	ConnectionErr err = CONNECTION_ERR_NONE;

	/* Init Value */
	memset(rec, 0, sizeof(SYNOSearchRecord));

	/* Check Parameters */
	if (NULL == connection) {
		SYSLOG(LOG_ERR, "connection is NULL. %m");
		err = CONNECTION_ERR_BAD_PARAMETER;
		goto Error;
	}

	strResp = "";
	// read packet content
	if (!synodaemon::io::PacketRead(connection->sockfd, strResp)) {
		SYSLOG(LOG_DEBUG, "Failed to read socket, errno(%d): %m", errno);
		err = CONNECTION_ERR_SOCKET_READ;
		goto Error;
	}
	jsItem.clear();
	// check string which is correctly
	if (!jsItem.fromString(strResp)) {
		SYSLOG(LOG_ERR, "response format is wrong: %s", strResp.c_str());
		err = CONNECTION_ERR_RESP_FORMAT;
		goto Error;
	}
	// first packet is total infor
	if (jsItem.isMember("total")) {
		return SYNOQueryGetNextRecord(connection, rec);
	}
	// last packet is success info
	if (jsItem.isMember("success")) {
		// end of query, not an error
		// return with record being empty
		goto Error;
	}
	snprintf(rec->szPath, sizeof(rec->szPath), "%s", jsItem.get("SYNOMDPath", "").asCString());
	snprintf(rec->szName, sizeof(rec->szName), "%s", jsItem.get("SYNOMDFSName", "").asCString());
	snprintf(rec->szExt, sizeof(rec->szExt), "%s", jsItem.get("SYNOMDExtension", "").asCString());
	rec->blIsDir = "y" == jsItem.get("SYNOMDIsDir", "n").asString() ? 1 : 0;
Error:
	return err;
}
