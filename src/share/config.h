/************************************************************************/
/*
auth:   yunfei
date:   2016/01/15
email:  snowleopard331@163.com
des:    read config file
*/
/************************************************************************/

#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "Common.h"
#include "policy/Singleton.h"

#include <boost/property_tree/ptree.hpp>
//#include <boost/property_tree/ptree_fwd.hpp>
#include <boost/property_tree/ini_parser.hpp>

class Config
{
public:
    friend Evil::OperatorNew<Config>;

private:
    Config();
    ~Config();

public:
    bool setSource(const char* file);

    bool reload();

    std::string getStringDefault(const std::string& sectionName, const std::string& key, const std::string& def);

    std::string getStringDefault(const char* sectionName, const char* key, const std::string& def);

    bool getBoolDefault(const std::string& sectionName, const std::string& key, const bool def = false);

    bool getBoolDefault(const char* sectionName, const char* key, const bool def = false);

    int getIntDefault(const std::string& sectionName, const std::string& key, const int def);

    int getIntDefault(const char* sectionName, const char* key, const int def);

    float getFloatDefault(const std::string& sectionName, const std::string& key, const float def);
    
    float getFloatDefault(const char* sectionName, const char* key, const float def);

    std::string getFileName() const
    {
        return m_FileName;
    }

private:

    std::string                     m_FileName;
    boost::property_tree::ptree*    m_Config;
};

#define sConfig Evil::Singleton<Config>::Instance()

#endif//_CONFIG_H_