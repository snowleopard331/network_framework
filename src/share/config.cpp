/************************************************************************/
/*
auth:   yunfei
date:   2016/01/15
email:  snowleopard331@163.com
des:    read config file
*/
/************************************************************************/

#include "config.h"

#include <unistd.h>

Config::Config()
    : m_Config(NULL)
{

}

Config::~Config()
{
    SafeDelete(m_Config);
}

bool Config::setSource(const char* file)
{
    m_FileName = file;

    return reload();
}

bool Config::reload()
{
    if(m_Config)
    {
        delete m_Config;
    }

    m_Config = new boost::property_tree::ptree;

    if(0 == access(m_FileName.c_str(), 0))
    {
        boost::property_tree::ini_parser::read_ini(m_FileName, *m_Config);
        return true;
    }

    SafeDelete(m_Config);
    return false;
}

std::string Config::getStringDefault(const std::string& sectionName, const std::string& key, const std::string& def)
{
    std::string name = sectionName + "." + key;
    return m_Config->get<std::string>(name, def);
}

std::string Config::getStringDefault(const char* sectionName, const char* key, const std::string& def)
{
    std::string name = sectionName;
    name += ".";
    name += key;
    return m_Config->get<std::string>(name, def);
}

bool Config::getBoolDefault(const std::string& sectionName, const std::string& key, const bool def /* = false */)
{
    std::string name = sectionName + "." + key;
    return m_Config->get<bool>(name, def);
}

bool Config::getBoolDefault(const char* sectionName, const char* key, const bool def /* = false */)
{
    std::string name = sectionName;
    name += ".";
    name += key;
    return m_Config->get<bool>(name, def);
}

int Config::getIntDefault(const std::string& sectionName, const std::string& key, const int def)
{
    std::string name = sectionName + "." + key;
    return m_Config->get<int>(name, def);
}

int Config::getIntDefault(const char* sectionName, const char* key, const int def)
{
    std::string name = sectionName;
    name += ".";
    name += key;
    return m_Config->get<int>(name, def);
}

float Config::getFloatDefault(const std::string& sectionName, const std::string& key, const float def)
{
    std::string name = sectionName + "." + key;
    return m_Config->get<float>(name, def);
}

float Config::getFloatDefault(const char* sectionName, const char* key, const float def)
{
    std::string name = sectionName;
    name += ".";
    name += key;
    return m_Config->get<float>(name, def);
}

