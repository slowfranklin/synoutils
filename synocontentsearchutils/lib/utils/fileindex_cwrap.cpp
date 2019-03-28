//Copyright (c) 2000-2016 Synology Inc. All rights reserved.
#include <string>
#include <unordered_map>
#include <algorithm>
#include <json/value.h>
#include <synoshare/utils.h>
#include <synofileop/ea.h>
#include <synocontentsearchutils/fileindex_cwrap.h>

#define SZ_FILE_INDEX_DB_NAME           SYNO_HIDDEN_EA_PREFIX_EX".fileindexdb"
#define SZ_FILE_INDEX_QUEUE_NAME        SYNO_HIDDEN_EA_PREFIX_EX"file_index_queue"
#define SZF_FILEINDEX_CONF_FOLDER_PATH  "/var/packages/SynoFinder/etc/fileindex.folders"

using namespace std;

namespace {

bool
StringStartWith(const string &str, const string &substr)
{
	if (str.size() >= substr.size() &&
			0 == strncmp(str.c_str(), substr.c_str(), substr.size())) {
		return true;
	}
	return false;
}

bool
IsFileExist(const string &strFullPath)
{
	struct stat s;
	BZERO_STRUCT(s);
	if (0 == lstat(strFullPath.c_str(), &s) &&
			S_ISREG(s.st_mode)) {
		return true;
	} else {
		return false;
	}
}

bool
GetShareNameByPath(string &strShareName, const string &strPath)
{
	bool   blRet = false;
	size_t pos;

	if (strPath.empty()) {
		goto END;
	}

	pos = strPath.substr(1).find("/");
	strShareName = string::npos == pos ? strPath.substr(1) : strPath.substr(1, pos);

	blRet = true;
END:
	return blRet;
}

bool
GetFullPath(string &strFullPath, const string &strPath, BOOL blRealTime)
{
	static     unordered_map<string, string> mapShVolPath;
	bool       blRet        = false;
	PSYNOSHARE pShare       = NULL;
	string     strShareName;
	string     strTmp;
	string     strVol;
	size_t     pos;

	if (!GetShareNameByPath(strShareName, strPath)) {
		SYSLOG(LOG_ERR, "GetShareNameByPath failed, path=%s", strPath.c_str());
		goto ERROR;
	}

	if (FALSE == blRealTime) {
		if (mapShVolPath.end() != mapShVolPath.find(strShareName)) {
			strVol = mapShVolPath[strShareName];
			goto END;
		}
	}

	if (0 > SYNOShareGet(strShareName.c_str(), &pShare)) {
		SYSLOG(LOG_ERR, "SYNOShareGet failed, share=%s", strShareName.c_str());
		goto ERROR;
	}

	strTmp = pShare->szPath;
	pos = strTmp.find_last_of("/");
	strVol = strTmp.substr(0, pos);
	mapShVolPath[strShareName] = strVol;

END:
	strFullPath = strVol + strPath;
	blRet = true;

ERROR:
	if (pShare) {
		SYNOShareFree(pShare);
	}
	return blRet;
}

bool
GetConfig(Json::Value &jsCfg, BOOL blRealTime)
{
	static bool        blLoaded = false;
	static Json::Value inJsCfg  = Json::arrayValue;
	bool               blRet;

	blRet = false;

	if ((!blLoaded || TRUE == blRealTime)) {
		if (!IsFileExist(SZF_FILEINDEX_CONF_FOLDER_PATH)) {
			inJsCfg = Json::arrayValue;
			goto END;
		}
		if (inJsCfg.fromFile(SZF_FILEINDEX_CONF_FOLDER_PATH)) {
			blLoaded = true;
		} else {
			goto ERROR;
		}
	}

END:
	jsCfg = inJsCfg;
	blRet = true;

ERROR:
	return blRet;
}

} // Anonymous namespace

int
FILEIDXDBPathGet(const char *szName, char *szOutDBPath, const int cbOutLen)
{
	int        ret    = -1;
	PSYNOSHARE pShare = NULL;

	SYNOSDK_CHECK_ARGS(3, -1, NULL != szName, NULL != szOutDBPath, cbOutLen > 0);

	if (0 > SYNOShareGet(szName, &pShare)) {
		SYSLOG(LOG_ERR, "SYNOShareGet failed, name=%s, code=%d", szName, SLIBCErrGet());
		goto END;
	}

	ret = FILEIDXDBPathCompose(pShare->szPath, szOutDBPath, cbOutLen);
END:
	if (pShare) {
		SYNOShareFree(pShare);
		pShare = NULL;
	}
	return ret;
}

int
FILEIDXDBPathCompose(const char *szSharePath, char *szOutDBPath, const int cbOutLen)
{
	char szBuf[PATH_MAX] = {0};

	SYNOSDK_CHECK_ARGS(3, -1, NULL != szSharePath, NULL != szOutDBPath, cbOutLen > 0);

	// /volumex/{share name}/@eaDir
	snprintf(szBuf, sizeof(szBuf), "%s/%s", szSharePath, SYNO_EA_DIR);
	SYNOEAMKDir(0, szBuf);

	// /volumex/{share name}/@eaDir/{db dir}
	snprintf(szOutDBPath, cbOutLen, "%s/%s/%s", szSharePath, SYNO_EA_DIR, SZ_FILE_INDEX_DB_NAME);
	mkdir(szOutDBPath, 0777);

	return strlen(szOutDBPath);
}

BOOL
FILEIDXIsShareIndexed(const char *szShareName)
{
	return FILEIDXIsShareIndexedEx(szShareName, TRUE);
}

BOOL
FILEIDXIsShareIndexedEx(const char *szShareName, BOOL blRealTime)
{
	bool        blRet  = false;
	Json::Value jsData;

	if (!GetConfig(jsData, blRealTime)) {
		SYSLOG(LOG_ERR, "Failed to load json from %s", SZF_FILEINDEX_CONF_FOLDER_PATH);
		goto END;
	}

	blRet = std::any_of(jsData.begin(), jsData.end(), [szShareName](const Json::Value &it) {
		string strShareName;

		if (!GetShareNameByPath(strShareName, it.get("path", "").asString())) {
			SYSLOG(LOG_ERR, "Failed to share name from path: %s", it.get("path", "").asCString());
			return false;
		}
		return strShareName == szShareName;
	});

END:
	return blRet ? TRUE : FALSE;
}

BOOL
FILEIDXIsPathContainIndexed(const char *szPath)
{
	return FILEIDXIsPathContainIndexedEx(szPath, TRUE);
}

BOOL
FILEIDXIsPathContainIndexedEx(const char *szPath, BOOL blRealTime)
{
	bool        blRet  = false;
	Json::Value jsData;
	string      strPath(szPath);

	if (!GetConfig(jsData, blRealTime)) {
		SYSLOG(LOG_ERR, "Failed to load json from %s", SZF_FILEINDEX_CONF_FOLDER_PATH);
		goto END;
	}

	blRet = std::any_of(jsData.begin(), jsData.end(), [&strPath](const Json::Value &it) {
		return StringStartWith(it.get("path", "").asString() + "/", strPath + "/");
	});

END:
	return blRet ? TRUE : FALSE;
}

BOOL
FILEIDXIsPathIndexed(const char *szPath)
{
	return FILEIDXIsPathIndexedEx(szPath, TRUE);
}

BOOL
FILEIDXIsPathIndexedEx(const char *szPath, BOOL blRealTime)
{
	string strFullPath;

	if (!GetFullPath(strFullPath, szPath, blRealTime)) {
		SYSLOG(LOG_ERR, "Failed to get full path: %s", szPath);
		return FALSE;
	}

	return FILEIDXIsFullPathIndexedEx(strFullPath.c_str(), blRealTime);
}

BOOL
FILEIDXIsFullPathIndexed(const char *szFullPath)
{
	return FILEIDXIsFullPathIndexedEx(szFullPath, TRUE);
}

BOOL
FILEIDXIsFullPathIndexedEx(const char *szFullPath, BOOL blRealTime)
{
	bool        blRet  = false;
	Json::Value jsData;
	string      strFullPath(szFullPath);

	if (!GetConfig(jsData, blRealTime)) {
		SYSLOG(LOG_ERR, "Failed to load json from %s", SZF_FILEINDEX_CONF_FOLDER_PATH);
		goto END;
	}

	blRet = std::any_of(jsData.begin(), jsData.end(), [&strFullPath, blRealTime](const Json::Value &it) {
		string strRuleFullPath;

		if (!GetFullPath(strRuleFullPath, it.get("path", "").asString(), blRealTime)) {
			SYSLOG(LOG_ERR, "Failed to get full path: %s", it.get("path", "").asCString());
			return false;
		}
		return StringStartWith(strFullPath + "/", strRuleFullPath + "/");
	});

END:
	return blRet ? TRUE : FALSE;
}

int
FILEIDXQueuePathGet(const char *szName, char *szOutQueuePath, const int cbOutLen)
{
	int        ret    = -1;
	PSYNOSHARE pShare = NULL;

	SYNOSDK_CHECK_ARGS(3, -1, NULL != szName, NULL != szOutQueuePath, cbOutLen > 0);

	if (0 > SYNOShareGet(szName, &pShare)) {
		SYSLOG(LOG_ERR, "SYNOShareGet failed, name=%s, code=%d", szName, SLIBCErrGet());
		goto END;
	}

	ret = FILEIDXQueuePathCompose(pShare->szPath, szOutQueuePath, cbOutLen);
END:
	if (pShare) {
		SYNOShareFree(pShare);
		pShare = NULL;
	}
	return ret;
}

int
FILEIDXQueuePathCompose(const char *szSharePath, char *szOutQueuePath, const int cbOutLen)
{
	char szBuf[PATH_MAX] = {0};

	SYNOSDK_CHECK_ARGS(3, -1, NULL != szSharePath, NULL != szOutQueuePath, cbOutLen > 0);

	// /volumex/{share name}/@eaDir
	snprintf(szBuf, sizeof(szBuf), "%s/%s", szSharePath, SYNO_EA_DIR);
	SYNOEAMKDir(0, szBuf);

	// /volumex/{share name}/@eaDir/{queue file}
	snprintf(szOutQueuePath, cbOutLen, "%s/%s/%s", szSharePath, SYNO_EA_DIR, SZ_FILE_INDEX_QUEUE_NAME);

	return strlen(szOutQueuePath);
}

BOOL
FILEIDXGetIndexID(const char *szShareName, char* szIndexID, const int cbOutLen)
{
	SYNOSDK_CHECK_ARGS(3, FALSE, NULL != szShareName, NULL != szIndexID, cbOutLen > 0);

	snprintf(szIndexID, cbOutLen, "fileindex_%s", szShareName);
	return TRUE;
}
