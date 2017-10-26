/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2015/09/14
*/

#ifndef _UTIL_H_
#define _UTIL_H_

#include "Common.h"

typedef std::vector<std::string> Tokens;

Tokens StrSplit(const std::string& src, const std::string& sep);

#endif//_UTIL_H_