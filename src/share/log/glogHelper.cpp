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

#include "glogHelper.h"

#define LOGDIR  "log"
#define MKDIR   "mkdir "LOGDIR

//����Ϣ������������ļ��� LOG(ERROR
void signalHandle(const char* data, int size)
{
    std::string str = std::string(data, size);

    /*
    ofstream fs("glog_test.log", ios::app);
    fs<<str;
    fs.close();
    */

    LOG(ERROR)<<str;
}

// GLOG����
GLogHelper::GLogHelper(char* program)
{
    system(MKDIR);
    google::InitGoogleLogging(program);

    //���ü������ google::INFO ����־ͬʱ�������Ļ
    google::SetStderrLogging(google::INFO);
    
    /*
        ��־�ļ�����ʽ:basename+ʱ�����������-HH:MM:SS.���߳�ID��
        ��־�Ƿּ����ŵģ��ͼ������־�ļ������߼������־��Ϣ, 
        ��INFO�������־���������߼����������־��ERROR�������־ֻ����ERROR��FATAL��������
    */
    //���� google::ERROR �������־�洢·�����ļ���ǰ׺(basename)
    google::SetLogDestination(google::ERROR, "log/log_error_");
    //���� google::INFO �������־�洢·�����ļ���ǰ׺(basename)
    google::SetLogDestination(google::INFO, "log/log_info_");
    google::SetLogDestination(google::WARNING, "log/log_warning_");
    google::SetLogDestination(google::FATAL, "log/log_fatal_");

    //������־�����Ĭ��Ϊ30�룬�˴���Ϊ�������
    FLAGS_logbufsecs = 0;
    //�����־��СΪ 1024MB
    FLAGS_max_log_size = 1024;
    //�����̱�д��ʱ��ֹͣ��־���
    FLAGS_stop_logging_if_full_disk = true;
    
    /*
        Linux
    */
    //��׽ core dumped
    //google::InstallFailureSignalHandler();
    //Ĭ�ϲ�׽ SIGSEGV �ź���Ϣ���������� stderr������ͨ������ķ����Զ������>��ʽ��
    //google::InstallFailureWriter(&signalHandle);
}

//GLOG�ڴ�����
GLogHelper::~GLogHelper()
{
    google::ShutdownGoogleLogging();
}