/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2015/09/07
*/

#ifndef _COMMON_H_
#define _COMMON_H_

#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <string.h>
#include <sstream>

#include "Define.h"
#include "log/glogHelper.h"
//#include "glogHelper.h"

/**
 * @brief operator new[] based version of strdup() function! Release memory by using operator delete[] !
 *
 * @param source
 * @return char
 */
inline char* common_strdup(const char* src)
{
    char* dest = new char[strlen(src) + 1];
    strcpy(dest, src);

    return dest;
}

enum TimeConstants
{
    MINUTE = 60,
    HOUR   = MINUTE * 60,
    DAY    = HOUR * 24,
    WEEK   = DAY * 7,
    MONTH  = DAY * 30,
    YEAR   = MONTH * 12,
    IN_MILLISECONDS = 1000
};

#define ATTR_PRINTF(F,V) __attribute__ ((format (printf, F, V)))

#endif//_COMMON_H_