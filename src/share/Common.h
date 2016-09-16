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
#include <boost/lexical_cast.hpp>

#include "Define.h"
#include "log/glogHelper.h"
#include "GameDef.h"
#include "config.h"


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

template<typename NumType>
std::string numToStr(NumType num)
{
    std::stringstream ss;
    ss<<num;
    
    std::string str;
    ss>>str;
    return str;
}

template<typename NumType = int>
bool numToStr(NumType num, std::string& str)
{
    bool ret = true;

    try
    {
        str = boost::lexical_cast<std::string>(num);
    }
    catch (boost::bad_lexical_cast& e)
    {
        LOG(ERROR) << e.what();
        ret = false;
    }

    return ret;
}

template<typename NumType = int>
bool strToNum(std::string& str, NumType& num)
{
    bool ret = true;

    try
    {
        num = boost::lexical_cast<NumType>(str);
    }
    catch (boost::bad_lexical_cast& e)
    {
        LOG(ERROR) << e.what();
        ret = false;
    }

    return ret;
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