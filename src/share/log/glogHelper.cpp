/************************************************************************/
/*
auth:   yunfei
date:   2015/02/27
email:  snowleopard331@163.com
des:    this file function is packaging and pratice glog libiary
*/
/************************************************************************/

#include <stdlib.h>
#include <string>
#include <fstream>
#include "glogHelper.h"

#define LOGDIR  "log"
#define MKDIR   "mkdir " LOGDIR

//将信息输出到单独的文件和 LOG(ERROR
void signalHandle(const char* data, int size)
{
    std::string str = std::string(data, size);

    LOG(ERROR)<<str;
}

// GLOG配置
GLogHelper::GLogHelper()
{
    
}

//GLOG内存清理
GLogHelper::~GLogHelper()
{
    google::ShutdownGoogleLogging();
}

void GLogHelper::initialize(char* programName)
{
    // check if the folder exist 
    std::fstream logFolder;
    logFolder.open("./log", std::ios::in);
    if(!logFolder)
    {
        system(MKDIR);
    }

    google::InitGoogleLogging(programName);

    //设置级别高于 google::INFO 的日志同时输出到屏幕
    google::SetStderrLogging(google::INFO); 

    /*
        日志文件名格式:basename+时间戳（年月日-HH:MM:SS.主线程ID）
        日志是分级别存放的，低级别的日志文件包含高级别的日志信息, 
        如INFO级别的日志包含其他高级别的所有日志，ERROR级别的日志只包含ERROR和FATAL两个级别
    */
    //设置 google::ERROR 级别的日志存储路径和文件名前缀(basename)
    google::SetLogDestination(google::ERROR, "log/log_error_");
    //设置 google::INFO 级别的日志存储路径和文件名前缀(basename)
    google::SetLogDestination(google::INFO, "log/log_info_");
    google::SetLogDestination(google::WARNING, "log/log_warning_");
    google::SetLogDestination(google::FATAL, "log/log_fatal_");

    //缓冲日志输出，默认为30秒，此处改为立即输出
    FLAGS_logbufsecs = 0;
    //最大日志大小为 1024MB
    FLAGS_max_log_size = 1024;
    //当磁盘被写满时，停止日志输出
    FLAGS_stop_logging_if_full_disk = true;
    //设置输出到屏幕的日志
    FLAGS_colorlogtostderr = true;

    /*
        Linux
    */
    //捕捉 core dumped
    google::InstallFailureSignalHandler();
    //默认捕捉 SIGSEGV 信号信息输出会输出到 stderr，可以通过下面的方法自定义输出>方式：
    google::InstallFailureWriter(&signalHandle);
}