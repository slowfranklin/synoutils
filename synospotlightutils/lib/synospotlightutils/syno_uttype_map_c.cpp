/* Copyright (c) 2000-2015 Synology Inc. All rights reserved. */
#include <string>
#include <cstring>
#include <cstdlib>
#include <syslog.h>
#include <synospotlightutils/syno_uttype_map_c.h>
#include <synospotlightutils/SYNOUTTypeMapping.h>

using std::string;

using namespace SYNO::SPOTLIGHT::UTILS;

static const SYNOUTTypeMapping* pUTTypeMapping = NULL;

/*
 * Retrieve included extensions with a UTType
 *
 * @param szUTType
 *          [IN] UTType name string
 * @return
 *          included extensions separated with ' ' or
 *          NULL if UTType or include extensions not found.
 *          It's up to the caller to free the memory.
 *
 *  NOTE: this function is NOT thread safe
 *
 *  example:
 *
 *  char *szExts = NULL;
 *  szExts = SYNOUTTypeGetExts("public.jpeg");
 *  if (NULL != szExts) {
 *		printf("%s\n", szExts);
 *		free(szExts);
 *		szExts = NULL;
 *  } else {
 *		printf("no UTType or no extensions found\n");
 *  }
 *
 */
char *SYNOUTTypeGetExts(const char *szUTType)
{
	char *szResult = NULL;
	string strExts = "";
	if (NULL == pUTTypeMapping) {
		pUTTypeMapping = new SYNOUTTypeMapping();
	}
	if (NULL == szUTType) {
		goto End;
	}
	strExts = pUTTypeMapping->getTypeExts(string(szUTType));
	if (0 == strExts.size()) {
		goto End;
	}
	szResult = strdup(strExts.c_str());
	if (NULL == szResult) {
		syslog(LOG_ERR, "failed to alloc memory\n");
		goto End;
	}
End:
	return szResult;
}
