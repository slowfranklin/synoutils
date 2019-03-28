//Copyright (c) 2000-2015 Synology Inc. All rights reserved.
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <signal.h>
#include <json/value.h>
#include <algorithm>
#include <sstream>
#include <synocontentsearchutils/connection.h>
#include <synodaemon/io_utils.h>
#include "error_int.h"

using namespace std;

namespace SYNO {
namespace IR {
namespace DAEMON {

static const std::string SOCK_PATH = "/var/run/synoelasticd.sock";

/*
 * Connection: communication with search daemon class
 */
Connection::Connection() : _fd(-1), _err(CONNECTION_ERR_NONE)
{
}

Connection::Connection(int fd) : _fd(fd), _err(CONNECTION_ERR_NONE)
{
}

Connection::~Connection()
{
	Close();
}

bool Connection::Connect(time_t connectionTimeout)
{
	bool ret = false;
	sockaddr_un address;
	struct timeval timeout;

	signal(SIGPIPE, SIG_IGN);
	_fd = socket(PF_UNIX, SOCK_STREAM, 0);
	if (_fd < 0) {
		DLOG("socket() failed");
		SetError(CONNECTION_ERR_SOCKET);
		goto Error;
	}

	/* start with a clean address structure */
	memset(&address, 0, sizeof(struct sockaddr_un));
	address.sun_family = AF_UNIX;
	snprintf(address.sun_path, SOCK_PATH.length() + 1, "%s", SOCK_PATH.c_str());

	if (connect(_fd, (struct sockaddr *) &address, sizeof(struct sockaddr_un)) != 0) {
		DLOG("Connect domain socket failed, %m");
		SetError(CONNECTION_ERR_SOCKET);
		goto Error;
	}
	//set sokcet timeout info
	timeout.tv_sec = connectionTimeout;
	timeout.tv_usec = 0;
	if (0 > setsockopt(_fd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout))) {
		DLOG("Failed to set socket recv timeout, %m");
		SetError(CONNECTION_ERR_SOCKET);
		goto Error;
	}
	if (0 > setsockopt(_fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(timeout))) {
		DLOG("Failed to set socket send timeout, %m");
		SetError(CONNECTION_ERR_SOCKET);
		goto Error;
	}

	ret = true;
Error:
	return ret;
}

void Connection::Close()
{
	if (_fd != -1) {
		close(_fd);
		_fd = -1;
	}
}
ConnectionErr Connection::GetError() const
{
	return _err;
}
void Connection::SetError(ConnectionErr err)
{
	_err = err;
}

bool Connection::SendQuery(const Json::Value &jsQuery, SYNOSearchCallback pFuncSearchCallback,
		void *cbData, const string &strAgent, uid_t uid)
{
	bool ret = false;
	Json::Value jsRewriteQuery, jsFields(Json::arrayValue);

	if (!jsQuery.isObject() || -1 == _fd) {
		SetError(CONNECTION_ERR_BAD_PARAMETER);
		goto Error;
	}

	jsRewriteQuery = jsQuery;
	jsFields.append("SYNOMDPath");
	jsFields.append("SYNOMDFSName");
	jsFields.append("SYNOMDExtension");
	jsFields.append("SYNOMDIsDir");
	jsRewriteQuery["data"]["fields"] = jsFields;
	jsRewriteQuery["data"]["collector"]["agent"] = strAgent;
	jsRewriteQuery["data"]["collector"]["uid"] = uid;
	//send query request
	if (!synodaemon::io::PacketWrite(_fd, jsRewriteQuery.toString())) {
		SetError(CONNECTION_ERR_SOCKET_WRITE);
		goto Error;
	}
	//get response
	if (!ProcessRepsonse(pFuncSearchCallback, cbData)) {
		goto Error;
	}

	ret = true;
Error:
	return ret;
}

//private function
bool Connection::ProcessRepsonse(SYNOSearchCallback pFuncSearchCallback, void *cbData)
{
	bool ret = false;
	int total = 0, count = 0;
	string strResp = "";
	Json::Value jsItem;
	SYNOSearchRecord rec;

	//fisrt packet: {"total":100}
	if (!synodaemon::io::PacketRead(_fd, strResp)) {
		SYSLOG(LOG_ERR, "Failed to read socket, errno(%d): %m", errno);
		SetError(CONNECTION_ERR_SOCKET_READ);
		goto Exit;
	}
	if (!jsItem.fromString(strResp) && !jsItem.isMember("total")) {
		DLOG("response format is wrong: %s", strResp.c_str());
		SetError(CONNECTION_ERR_RESP_FORMAT);
		goto Exit;
	}
	total = jsItem["total"].asInt();

	while (count++ < total) {
		//initial variable
		strResp = "";
		if (!synodaemon::io::PacketRead(_fd, strResp)) {
			SYSLOG(LOG_ERR, "Failed to read socket, errno(%d): %m", errno);
			SetError(CONNECTION_ERR_SOCKET_READ);
			goto Exit;
		}
		jsItem.clear();
		if (!jsItem.fromString(strResp)) {
			DLOG("response format is wrong: %s", strResp.c_str());
			SetError(CONNECTION_ERR_RESP_FORMAT);
			goto Exit;
		}
		//last packet: {"success": true}
		if (jsItem.isMember("success")) {
			break;
		}
		//hit record
		memset(&rec, 0, sizeof(rec));
		snprintf(rec.szPath, sizeof(rec.szPath), "%s", jsItem.get("SYNOMDPath", "").asCString());
		snprintf(rec.szName, sizeof(rec.szName), "%s", jsItem.get("SYNOMDFSName", "").asCString());
		snprintf(rec.szExt, sizeof(rec.szExt), "%s", jsItem.get("SYNOMDExtension", "").asCString());
		rec.blIsDir = "y" == jsItem.get("SYNOMDIsDir", "n").asString() ? 1 : 0;
		if (!pFuncSearchCallback(&rec, cbData)) {
			break;
		}
	}

	ret = true;
Exit:
	return ret;
}

} //end of DAEMON
} //end of IR
} //end of SYNO
