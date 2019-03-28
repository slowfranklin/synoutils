//Copyright (c) 2000-2015 Synology Inc. All rights reserved.
#include <json/value.h>
#include <synocontentsearchutils/connection_cwrap.h>
#include <synocontentsearchutils/fileindex_cwrap.h>
#include <string>
#include <synodaemon/io_utils.h>

/*
 * Open(send) a "query" to synocontentsearch daemon.
 *
 * @param *connection
 *          [IN] a pointer to SYNOQueryConnection(domain socket server)
 * @param szShareName
 *          [IN] share name
 * @param szQuery
 *          [IN] lucene query string
 * @param pageNum
 *          [IN] page number
 * @param pageSize
 *          [IN] max page size in page number
 * @return CONNECTION_ERR_NONE: on success
 *         others             : on failed
 *
 * usage example:
 *
 * #define PAGE_NUM 1
 * #define MAX_SIZE 10
 * char *szShareName = "Share";
 * char *szQuery = "Hello World";
 *
 * if (CONNECTION_ERR_NONE != SYNOQueryConnectionOpen(
 *                              connection, szShareName,
 *                              szQuery, PAGE_NUM, MAX_SIZE){
 *     SYSLOG(LOG_ERR, "SYNOQueryConnectionOpen Error");
 *     goto Error;
 * }
 *
 */
int SYNOQueryConnectionOpen(SYNOQueryConnection *connection,
							const char *szShareName, const char *szQuery,
							int pageNum, int pageSize, const char *szAgent, uid_t uid)
{
	int ret = CONNECTION_ERR_NONE;
	Json::Value jsonQuery, jsonData, jsIndexIDs(Json::arrayValue), jsFields(Json::arrayValue);
	char szIndexID[PATH_MAX + 1] = {0};

	/* Check input are correctlly*/
	if ((NULL == connection) || (NULL == szShareName) || (NULL == szQuery) || ( 0 > pageNum) || (1 > pageSize)) {
		ret = CONNECTION_ERR_BAD_PARAMETER;
		goto Error;
	}

	if (0 >= FILEIDXGetIndexID(szShareName, szIndexID, sizeof(szIndexID))) {
		ret = CONNECTION_ERR_PATH_NOT_FOUND;
		goto Error;
	}
	jsIndexIDs.append(szIndexID);
	jsFields.append("SYNOMDPath");
	jsFields.append("SYNOMDFSName");
	jsFields.append("SYNOMDExtension");
	jsFields.append("SYNOMDIsDir");

	/* Prepare json format string*/
	jsonQuery["command"] = "search";
	jsonData["from"] = pageSize * pageNum;
	jsonData["size"] = pageSize;
	jsonData["indice"] = jsIndexIDs;
	jsonData["fields"] = jsFields;
	jsonData["query_string"]["query"] = szQuery;
	jsonData["collector"]["agent"] = szAgent;
	jsonData["collector"]["uid"] = uid;
	jsonQuery["data"] = jsonData;

	/* send query length to daemon */
	if (!synodaemon::io::PacketWrite(connection->sockfd, jsonQuery.toString())) {
		ret = CONNECTION_ERR_SOCKET_WRITE;
		goto Error;
	}
Error:
	return ret;
}
