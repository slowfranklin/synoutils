/* Copyright (c) 2000-2015 Synology Inc. All rights reserved. */
#include "synospotlightutils/SYNOUTType.h"

using namespace std;

namespace SYNO {
namespace SPOTLIGHT {
namespace UTILS {

void SYNOUTType::setId(const string& strId)
{
	_strId = strId;
}

void SYNOUTType::setId(const char* szId)
{
	_strId = string(szId);
}

void SYNOUTType::setDesc(const string &strDesc)
{
	_strDesc = strDesc;
}

void SYNOUTType::setDesc(const char* szDesc)
{
	_strDesc = string(szDesc);
}

void SYNOUTType::addExt(const string& strExt)
{
	_exts.push_back(strExt);
}

void SYNOUTType::addExt(const char* szExt)
{
	if (NULL != szExt) {
		_exts.push_back(string(szExt));
	}
}

void SYNOUTType::addConformTo(const string& strConform)
{
	_conformTo.push_back(strConform);
}

void SYNOUTType::addConformTo(const char* szConform)
{
	if (NULL != szConform) {
		_conformTo.push_back(string(szConform));
	}
}

void SYNOUTType::addChildren(const SYNOUTType* child)
{
	if (NULL != child) {
		_children.push_back(child);
	}
}

string SYNOUTType::getId() const
{
	return _strId;
}

string SYNOUTType::getDesc() const
{
	return _strDesc;
}

const vector<string>& SYNOUTType::getConformTo() const
{
	return _conformTo;
}

string SYNOUTType::getExts() const
{
	string strExts = "";
	string strTmp = "";
	int iVecSize = _exts.size();
	for (int i = 0; i < iVecSize; i++) {
		if (strExts.size() == 0) {
			strExts += _exts[i];
		} else {
			strExts += string(" ") + _exts[i];
		}
	}
	iVecSize = _children.size();
	for (int i = 0; i < iVecSize; i++) {
		strTmp = _children[i]->getExts();
		if (strExts.size() == 0) {
			strExts = strTmp;
		} else if (strTmp != "") {
			strExts += string(" ") + strTmp;
		}
	}
	return strExts;
}

} /* namespace UTILS */
} /* namespace SPOTLIGHT */
} /* namespace SYNO */
