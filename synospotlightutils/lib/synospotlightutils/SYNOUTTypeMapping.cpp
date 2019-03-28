/* Copyright (c) 2000-2015 Synology Inc. All rights reserved. */
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <synocore/synoglobal.h>
#include <synospotlightutils/SYNOUTType.h>
#include <synospotlightutils/SYNOUTTypeMapping.h>
using namespace std;

namespace SYNO {
namespace SPOTLIGHT {
namespace UTILS {

// public functions
SYNOUTTypeMapping::SYNOUTTypeMapping() : _ok(true), _strPlistPath(SZ_DEFAULT_PLIST_PATH)
{
	init();
}

SYNOUTTypeMapping::SYNOUTTypeMapping(const string& strPlistPath) : _ok(true), _strPlistPath(strPlistPath)
{
	init();
}

string SYNOUTTypeMapping::getTypeDesc(const std::string& strType) const
{
	string strResult = "";
	map<string, SYNOUTType>::const_iterator it = _typeMapping.find(strType);
	if (_typeMapping.end() == it) {
		goto End;
	}
	strResult = it->second.getDesc();
End:
	return strResult;
}

string SYNOUTTypeMapping::getTypeExts(const std::string& strType) const
{
	string strResult = "";
	map<string, SYNOUTType>::const_iterator it = _typeMapping.find(strType);
	if (_typeMapping.end() == it) {
		goto End;
	}
	strResult = it->second.getExts();
End:
	return strResult;
}

// private init
void SYNOUTTypeMapping::init() {
	// parse plist to uttypes
	if (0 > parse(_strPlistPath)) {
		_ok = false;
		goto End;
	}
	// link children to parent
	if (0 > linkUTTypeChildren()) {
		_ok = false;
		goto End;
	}
End:
	if(!_ok) {
		SYSLOG(LOG_ERR, "some error occured while parsing type plist\n");
		_typeMapping.clear();
	}
}

// parsing utilities
bool checkNodeName(const xmlNode *node, const char* szName)
{
	bool blRet = false;
	if (NULL == node || NULL == node->name) {
		goto End;
	}
	blRet = (0 == strcmp((char*) node->name, szName));
End:
	return blRet;
}

const char* getText(const xmlNode *node)
{
	const char *text = NULL;
	const xmlNode *curChildNode = NULL;
	if (NULL == node) {
		goto End;
	}
	curChildNode = node->children;
	while (NULL != curChildNode) {
		if (XML_TEXT_NODE == curChildNode->type) {
			text = (char*) curChildNode->content;
			break;
		}
		curChildNode = curChildNode->next;
	}
End:
	return text;
}

const xmlNode* findValueNode(const xmlNode *curNode)
{
	while(NULL != curNode && XML_ELEMENT_NODE != curNode->type) {
		curNode = curNode->next;
	}
	return curNode;
}

const xmlNode* findNextValueNode(const xmlNode *curNode)
{
	if (NULL == curNode) {
		return NULL;
	}
	return findValueNode(curNode->next);
}

void SYNOUTTypeMapping::testProg() const
{
	string strTestType;
	map<string, SYNOUTType>::const_iterator it;

	cout << "input some UTType: " << endl;
	while (cin >> strTestType) {
		it = _typeMapping.find(strTestType);
		if (_typeMapping.end() == it) {
			cout << "Can't find type with id = " << strTestType << endl;
			continue;
		}
		const SYNOUTType& testType = it->second;
		cout << "======Type Summary=====" << endl;
		cout << "Type ID:       " << testType.getId() << endl;
		cout << "Type Desc:     " << testType.getDesc() << endl;
		cout << "Included exts: " << testType.getExts() << endl;
		cout << "=======================" << endl;
	}
}

// private parsing functions
int SYNOUTTypeMapping::parseConformToNode(SYNOUTType& uttype, const xmlNode* node)
{
	int iRet = -1;
	const char* szText = NULL;
	const xmlNode* curNode = NULL;

	if (NULL == node) {
		SYSLOG(LOG_ERR, "NULL tag spec node\n");
		goto End;
	}
	if (checkNodeName(node, SZ_STRING_NAME)) {
		// single string
		szText = getText(node);
		if (NULL == szText) {
			SYSLOG(LOG_ERR, "can't extract text\n");
			goto End;
		}
		uttype.addConformTo(szText);
	} else if (checkNodeName(node, SZ_ARRAY_NAME)) {
		curNode = findValueNode(node->children);
		while(curNode) {
			szText = getText(curNode);
			if (NULL == szText) {
				SYSLOG(LOG_ERR, "can't extract text\n");
				goto End;
			}
			uttype.addConformTo(szText);
			curNode = findNextValueNode(curNode);
		}
	} else {
		SYSLOG(LOG_ERR, "expect conformTo field being stirng or string array (%s)\n", node->name);
		goto End;
	}
	iRet = 0;
End:
	return iRet;
}

int SYNOUTTypeMapping::parseTagSpecNode(SYNOUTType& uttype,const xmlNode *node)
{
	int iRet = -1;
	const char* szKey = NULL;
	const char* szText = NULL;
	const xmlNode *curNode = NULL;
	const xmlNode *extArrayNode = NULL;

	if (NULL == node) {
		SYSLOG(LOG_ERR, "NULL tag spec node\n");
		goto End;
	}
	if (!checkNodeName(node, SZ_DICT_NAME)) {
		SYSLOG(LOG_ERR, "expect tag spec being DICT but %s found\n", node->name);
		goto End;
	}
	curNode = findValueNode(node->children);
	while (curNode) {
		if (!checkNodeName(curNode, SZ_KEY_NAME)) {
			SYSLOG(LOG_ERR, "expecting KEY but %s found\n", curNode->name);
			goto End;
		}
		szKey = getText(curNode);
		if (NULL == szKey) {
			SYSLOG(LOG_ERR, "failed to extract key\n");
			goto End;
		}
		curNode = findNextValueNode(curNode);
		if (NULL == curNode) {
			SYSLOG(LOG_ERR, "can't find value for key: %s\n", szKey);
			goto End;
		}
		// fill tag spec related fields
		if (0 == strcmp(szKey, SZ_PUBLIC_FILENAME_EXT)) {
			if (checkNodeName(curNode, SZ_ARRAY_NAME)) {
				extArrayNode = findValueNode(curNode->children);
				while (extArrayNode) {
					szText = getText(extArrayNode);
					if (NULL == szText) {
						SYSLOG(LOG_ERR, "cannot extract text\n");
						goto End;
					}
					uttype.addExt(szText);
					extArrayNode = findNextValueNode(extArrayNode);
				}
			} else if (checkNodeName(curNode, SZ_STRING_NAME)) {
				szText = getText(curNode);
				if (NULL == szText) {
					SYSLOG(LOG_ERR, "cannot extract text\n");
					goto End;
				}
				uttype.addExt(szText);
			} else {
				SYSLOG(LOG_ERR, "expec filename extension being string or string array\n");
				goto End;
			}
		}
		curNode = findNextValueNode(curNode);
	}
	iRet = 0;
End:
	return iRet;
}

int SYNOUTTypeMapping::parseContentTypeNode(const xmlNode *node)
{
	int iRet = -1;
	const xmlNode* curNode = NULL;
	const char *szKey = NULL;
	const char *szText = NULL;

	SYNOUTType uttype;

	if (!checkNodeName(node, SZ_DICT_NAME)) {
		SYSLOG(LOG_ERR, "expect content type declaration being DICT but %s found\n", node->name);
		goto End;
	}

	curNode = findValueNode(node->children);
	while(curNode) {
		if (!checkNodeName(curNode, SZ_KEY_NAME)) {
			SYSLOG(LOG_ERR, "expecting KEY but %s found\n", curNode->name);
			goto End;
		}
		szKey = getText(curNode);
		if (NULL == szKey) {
			SYSLOG(LOG_ERR, "failed to extract key\n");
			goto End;
		}
		curNode = findNextValueNode(curNode);
		if (NULL == curNode) {
			SYSLOG(LOG_ERR, "can't find value for key: %s\n", szKey);
			goto End;
		}
		// fill value to corresponding field
		if (0 == strcmp(szKey, SZ_UT_IDENTIFIER)) {
			if (!checkNodeName(curNode, SZ_STRING_NAME)) {
				SYSLOG(LOG_ERR, "expect id being string\n");
				goto End;
			}
			szText = getText(curNode);
			uttype.setId(szText);
		} else if (0 == strcmp(szKey, SZ_UT_DESCRIPTION)) {
			if (!checkNodeName(curNode, SZ_STRING_NAME)) {
				SYSLOG(LOG_ERR, "expect description being string\n");
				goto End;
			}
			szText = getText(curNode);
			uttype.setDesc(szText);
		} else if (0 == strcmp(szKey, SZ_UT_CONFORMS_TO)) {
			parseConformToNode(uttype, curNode);
		} else if (0 == strcmp(szKey, SZ_UT_TAG_SPEC)) {
			parseTagSpecNode(uttype, curNode);
		}
		curNode = findNextValueNode(curNode);
	}
	_typeMapping[uttype.getId()] = uttype;
	iRet = 0;
End:
	return iRet;
}

int SYNOUTTypeMapping::parse(const string& szPlistPath) {
	int iRet = -1;
	xmlDoc *doc = NULL;
	xmlNode *rootNode = NULL;
	const xmlNode *curNode = NULL;
	const xmlNode *typeNode = NULL;
	const char *szKey = NULL;

	LIBXML_TEST_VERSION
	doc = xmlReadFile(szPlistPath.c_str(), NULL, 0);
	if (NULL == doc) {
		SYSLOG(LOG_ERR, "failed to parse xml\n");
		goto End;
	}

	rootNode = xmlDocGetRootElement(doc);
	if (NULL == rootNode || XML_ELEMENT_NODE != rootNode->type
			|| !checkNodeName(rootNode, SZ_PLIST_NAME)) {
		SYSLOG(LOG_ERR, "expect plist as root\n");
		goto End;
	}
	curNode = findValueNode(rootNode->children);
	if (NULL == curNode || XML_ELEMENT_NODE != curNode->type
			|| !checkNodeName(curNode, SZ_DICT_NAME)) {
		SYSLOG(LOG_ERR, "expect dict in plist\n");
		goto End;
	}
	curNode = findValueNode(curNode->children);

	while (curNode) {
		if (!checkNodeName(curNode, SZ_KEY_NAME)) {
			SYSLOG(LOG_ERR, "expect key but %s found\n", curNode->name);
			goto End;
		}
		szKey = getText(curNode);
		if (NULL == szKey) {
			SYSLOG(LOG_ERR, "can't extract key\n");
			goto End;
		}
		curNode = findNextValueNode(curNode);
		if (NULL == curNode) {
			SYSLOG(LOG_ERR, "can't find value for key: %s\n", szKey);
			goto End;
		}
		if (0 == strcmp(szKey, SZ_UT_EXPORTED_TYPE_DECLARE)
				|| 0 == strcmp(szKey, SZ_UT_IMPORTED_TYPE_DECLARE)) {
			if (!checkNodeName(curNode, SZ_ARRAY_NAME)) {
				SYSLOG(LOG_ERR, "expect array in exported/imported type declaration but %s found\n", curNode->name);
				goto End;
			}
			typeNode = findValueNode(curNode->children);
			while (typeNode) {
				if (0 > parseContentTypeNode(typeNode)) {
					SYSLOG(LOG_ERR, "error parsing content type node\n");
					goto End;
				}
				typeNode = findNextValueNode(typeNode);
			}
		}
		curNode = findNextValueNode(curNode);
	}
	iRet = 0;
End:
	if (doc) {
		xmlFreeDoc(doc);
	}
	xmlCleanupParser();
	return iRet;
}

int SYNOUTTypeMapping::linkUTTypeChildren() {
	int conformTableSize;
	map<string, SYNOUTType>::iterator it;
	map<string, SYNOUTType>::iterator itParent;
	it = _typeMapping.begin();
	while (_typeMapping.end() != it) {
		// ref for cleaner code
		SYNOUTType& uttype = it->second;
		const vector<string>& conformTable = uttype.getConformTo();

		conformTableSize = conformTable.size();
		for (int i = 0; i < conformTableSize; i++) {
			itParent = _typeMapping.find(conformTable[i]);
			if (_typeMapping.end() == itParent) {
				SYSLOG(LOG_INFO, "Warning: some type does not exist (%s)\n", conformTable[i].c_str());
				continue;
			}
			itParent->second.addChildren(&uttype);
		}
		it++;
	}
	return 0;
}

} /* namespace UTILS */
} /* namespace SPOTLIGHT */
} /* namespace SYNO */
