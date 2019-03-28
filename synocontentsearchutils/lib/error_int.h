//Copyright (c) 2000-2015 Synology Inc. All rights reserved.
#ifndef __CONTENT_SEARCH_UTILS_ERR_INT_H__
#define __CONTENT_SEARCH_UTILS_ERR_INT_H__
#include <synocore/synoglobal.h>
#include <syslog.h>
#include <errno.h>
#include <json/value.h>

namespace SYNO {
namespace IR {
namespace DAEMON{

#define DLOG(fmt...) SYSLOG(LOG_ERR, fmt)
#define err(fmt...)  SYSLOG(LOG_ERR, fmt)

#define errJson(jsonA)  do { \
	Json::FastWriter tmpWrite; \
	std::string sOut; \
	sOut = tmpWrite.write(jsonA); \
	SYSLOG(LOG_ERR, "(%u) [%s]=%s", getpid(), #jsonA, sOut.c_str()); \
}   while(0)

#ifdef SYNO_DEBUG
#define dbg(fmt, args...)   err(fmt, ##args)
#define dbgJson(a)   errJson(a)
#else
#define dbg(fmt, args...)
#define dbgJson(a)
#endif

#ifdef SYNO_DEBUG_FLOW
#define dbgflow(fmt, args...)   err(fmt, ##args)
#else
#define dbgflow(fmt, args...)
#endif

#define errret(x, fmt, args...)   do { \
	err("(%s:%d)(%m)" fmt "\n", __FILE__, __LINE__, ##args); \
	return x; \
} while(0)

#define errend(fmt, args...)   do { \
		err("(%s:%d)(%m)" fmt "\n", __FILE__, __LINE__, ##args); \
		goto End; \
} while(0)
#define errbreak(fmt, args...) \
	err("(%s:%d)(%m)" fmt "\n", __FILE__, __LINE__, ##args); \
	break;

#define errcontinue(fmt, args...)   do { \
		err("(%s:%d)(%m)" fmt "\n", __FILE__, __LINE__, ##args); \
		continue; \
} while(0)

#define ifend(x) \
	dbgflow(#x); \
	if((x)) { \
		goto End; \
	}

#define iflog(x) \
	dbgflow(#x); \
	if((x)) { \
		errend("Failed [%s], err=%m", #x); \
	}

#define ifbreak(x) \
	dbgflow(#x); \
	if((x)) { \
		break; \
	}

#define ifcontinue(x) \
	dbgflow(#x); \
	if((x)) { \
		continue; \
	}

}
}
}

#endif
