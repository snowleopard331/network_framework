/**
* author    :   yunfei
* email     :   snowleopard331@163.com
* date      :   2016/01/30
*/

#include "PosixDaemon.h"

//pid_t parent_pid, sid = 0;

//void daemonSignal(int s)
//{
//    if(getpid() != parent_pid)
//    {
//        return;
//    }
//
//    if(s == SIGUSR1)
//    {
//        exit(EXIT_SUCCESS);
//    }
//
//    if(sid)
//    {
//        kill(sid, s);
//    }
//
//    exit(EXIT_FAILURE);
//}

void startDaemon(uint32 timeout /* = 10 */)
{
    // parent_pid = getpid();
    pid_t pid;

    //signal(SIGUSR1, daemonSignal);
    //signal(SIGINT, daemonSignal);
    //signal(SIGTERM, daemonSignal);
    //signal(SIGALRM, daemonSignal);

    pid = fork();

    if(pid < 0)
    {
        exit(EXIT_FAILURE);
    }

    if(pid > 0)
    {
        //alarm(timeout);
        //pause();
        exit(EXIT_FAILURE);
    }

    umask(0);

    pid_t sid = setsid();

    if(sid < 0)
    {
        exit(EXIT_FAILURE);
    }

    if((chdir("./")) < 0)
    {
        exit(EXIT_FAILURE);
    }
}
