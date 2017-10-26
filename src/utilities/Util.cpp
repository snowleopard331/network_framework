/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2015/09/14
*/

#include "Util.h"

Tokens StrSplit(const std::string& src, const std::string& separator)
{
    Tokens r;
    std::string s;
    for(std::string::const_iterator iter = src.begin(); iter != src.end(); ++iter)
    {
        if(separator.find(*iter) != std::string::npos)
        {
            if(s.length())
            {
                r.push_back(s);
            }

            s = "";
        }
        else
        {
            s += *iter;
        }
    }

    if(s.length())
    {
        r.push_back(s);
    }

    return r;
}