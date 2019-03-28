//Copyright (c) 2000-2015 Synology Inc. All rights reserved.
#include <json/value.h>
#include <synocontentsearchutils/connection.h>
#include <synocontentsearchutils/connection_cwrap.h>
#include <synocontentsearchutils/fileindex_cwrap.h>

bool SYNOSearchQuerySend(const char *szShareName, const char *szQuery, int pageSize, int pageNum,
		SYNOSearchCallback pFuncSearchCallback, void *cbData, const char *szAgent, uid_t uid, ConnectionErr *err)
{
	bool ret = false;
	Json::Value jsQuery, jsData, jsIndexIDs, jsFields;
	char szIndexID[PATH_MAX + 1] = {0};
	SYNO::IR::DAEMON::Connection connection;

	if (!err) {
		goto Error;
	}
	if (!szShareName || !szQuery || 0 > pageSize || 0 > pageNum ||
		!pFuncSearchCallback) {
		*err = CONNECTION_ERR_BAD_PARAMETER;
		goto Error;
	}
	if (0 >= FILEIDXGetIndexID(szShareName, szIndexID, sizeof(szIndexID))) {
		ret = CONNECTION_ERR_PATH_NOT_FOUND;
		goto Error;
	}
	if (!connection.Connect()) {
		*err = connection.GetError();
		goto Error;
	}

	jsIndexIDs.append(szIndexID);
	jsFields.append("SYNOMDPath");
	jsFields.append("SYNOMDFSName");
	jsFields.append("SYNOMDExtension");
	jsFields.append("SYNOMDIsDir");

	jsQuery["command"] = "search";
	jsData["from"] = pageSize * pageNum;
	jsData["size"] = pageSize;
	jsData["indice"] = jsIndexIDs;
	jsData["fields"] = jsFields;
	jsData["query_string"]["query"] = szQuery;
	jsQuery["data"] = jsData;
	if (!connection.SendQuery(jsQuery, pFuncSearchCallback, cbData, szAgent, uid)) {
		*err = connection.GetError();
		goto Error;
	}

	ret = true;
Error:
	connection.Close();
	return ret;
}
