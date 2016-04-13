MySQL-devel-5.6.30-1.linux_glibc2.5.x86_64          文件为mysql开发头文件rpm包
MySQL-shared-compat-5.6.30-1.linux_glibc2.5.i386    文件为mysql开发库rpm包

这两个安装包不能重定向安装路径

安装 拷贝(root用户)



安装命令
    rpm -ivha MySQL-devel-5.6.30-1.linux_glibc2.5.x86_64.rpm            头文件
    rpm -ivh -a MySQL-shared-compat-5.6.30-1.linux_glibc2.5.i386.rpm    库文件

卸载命令
    rpm -e MySQL-devel 

cp -vr libmysqlclient* /home/yunfei/myworld/network_ramework/dep/mysql/lib
cp -vr mysql /home/yunfei/myworld/network_ramework/dep/mysql/lib/
cp -vr mysql /home/yunfei/myworld/network_ramework/dep/mysql/include/




///////////////////////////////////////////////////////////////////////////////

依赖库问题  参考一下答案

打印机驱动程序是两个rpm安装包：
cndrvcups-common-2.60-1.x86_64.rpm和cndrvcups-capt-2.60-1.x86_64.rpm。
执行安装命令rpm -ivh cndrvcups-common-2.60-1.x86_64.rpm，出现依赖项错误，错误代码如下：
[root@cSlave00 RPM]# rpm -ivh cndrvcups-common-2.60-1.x86_64.rpm
error: Failed dependencies:
libc.so.6 is needed by cndrvcups-common-2.60-1.x86_64
libc.so.6(GLIBC_2.0) is needed by cndrvcups-common-2.60-1.x86_64
libc.so.6(GLIBC_2.1) is needed by cndrvcups-common-2.60-1.x86_64
libc.so.6(GLIBC_2.1.3) is needed by cndrvcups-common-2.60-1.x86_64
libc.so.6(GLIBC_2.3) is needed by cndrvcups-common-2.60-1.x86_64
libdl.so.2 is needed by cndrvcups-common-2.60-1.x86_64
libdl.so.2(GLIBC_2.0) is needed by cndrvcups-common-2.60-1.x86_64
libdl.so.2(GLIBC_2.1) is needed by cndrvcups-common-2.60-1.x86_64
libm.so.6 is needed by cndrvcups-common-2.60-1.x86_64
libm.so.6(GLIBC_2.0) is needed by cndrvcups-common-2.60-1.x86_64
libpthread.so.0 is needed by cndrvcups-common-2.60-1.x86_64
libpthread.so.0(GLIBC_2.0) is needed by cndrvcups-common-2.60-1.x86_64
libpthread.so.0(GLIBC_2.1) is needed by cndrvcups-common-2.60-1.x86_64
libpthread.so.0(GLIBC_2.3.2) is needed by cndrvcups-common-2.60-1.x86_64
librt.so.1 is needed by cndrvcups-common-2.60-1.x86_64
libstdc++.so.6 is needed by cndrvcups-common-2.60-1.x86_64
libstdc++.so.6(CXXABI_1.3) is needed by cndrvcups-common-2.60-1.x86_64
在网上进行一番搜索，解决方法都是安装缺少的依赖库即可。但是搜索后发现，本机上有些库已经安装了。
比如libc.so.6，该库对应的软件包名称为glibc。
[root@cSlave00 RPM]# yum list glibc*
已加载插件：fastestmirror, refresh-packagekit, security
Loading mirror speeds from cached hostfile
* base: mirrors.cug.edu.cn
* extras: mirrors.cug.edu.cn
* updates: mirrors.skyshe.cn
已安装的软件包 glibc.x86_64 2.12-1.149.el6
@anaconda-CentOS-201410241409.x86_64/6.6
glibc-common.x86_64 2.12-1.149.el6
@anaconda-CentOS-201410241409.x86_64/6.6
glibc-devel.x86_64 2.12-1.149.el6
@anaconda-CentOS-201410241409.x86_64/6.6
glibc-headers.x86_64 2.12-1.149.el6
@anaconda-CentOS-201410241409.x86_64/6.6
可安装的软件包
glibc.i686 2.12-1.149.el6 base
glibc-devel.i686 2.12-1.149.el6 base
glibc-static.i686 2.12-1.149.el6 base
glibc-static.x86_64 2.12-1.149.el6 base
glibc-utils.x86_64 2.12-1.149.el6 base
那么问题来了，既然已经安装了libc.so.6，为什么还是提示缺少该库呢？继续找啊找，百度搜索基本上无果，只能寄希望于谷歌。终于，搞定科学上网，谷歌能上了，继续搜索，重点关注英文结果。最后，总算在Stack Overflow上找到了类似的问题与解答，点击这里查看原文。
这里只摘录关键的几句话：
In Red Hat Enterprise Linux 5, if a package was available for both the main and the compatibility architectures, both architectures of the package were installed by default.In Red Hat Enterprise Linux 6, only the package for the primary architecture is installed by default.To avoid problems during the backup-archive client and API installation on a 64-bit machine, be sure to install libstdc++ and compat-libstdc++.
大意是说，从Red Hat Enterprise Linux 6开始，默认只安装主架构所需要的包，而不安装兼容架构的包，也就是说，64位系统默认只安装64位的软件包。为避免这种问题，在64位系统中，要同时安装64位的包和32位的兼容包。CentOS和从Red Hat Enterprise Linux 6的关系就不用我说了，因此我感觉这个回答比较靠谱，赶紧试试吧。
[root@cSlave00 RPM]# yum install glibc.i686
已加载插件：fastestmirror, refresh-packagekit, security
设置安装进程
……
已安装:
glibc.i686 0:2.12-1.149.el6
作为依赖被安装: nss-softokn-freebl.i686 0:3.14.3-18.el6_6
作为依赖被升级: nss-softokn-freebl.x86_64 0:3.14.3-18.el6_6 完毕！
[root@cSlave00 RPM]# rpm -ivh cndrvcups-common-2.60-1.x86_64.rpm error: Failed
dependencies:
libstdc++.so.6 is needed by cndrvcups-common-2.60-1.x86_64
libstdc++.so.6(CXXABI_1.3) is needed by
cndrvcups-common-2.60-1.x86_64
结果令人满意，果然不再提示libc.so.6了。继续解决libstdc++.so.6！
[root@cSlave00 RPM]# yum list libstdc++*
已加载插件：fastestmirror, refresh-packagekit, security
Loading mirror speeds from cached hostfile
* base: ftp.sjtu.edu.cn
* extras: mirrors.163.com
* updates: ftp.sjtu.edu.cn
已安装的软件包
libstdc++.x86_64 4.4.7-11.el6
@anaconda-CentOS-201410241409.x86_64/6.6
可安装的软件包
libstdc++.i686 4.4.7-11.el6 base libstdc++-devel.i686 4.4.7-11.el6 base
libstdc++-devel.x86_64 4.4.7-11.el6 base
libstdc++-docs.x86_64 4.4.7-11.el6 base
[root@cSlave00 RPM]# yum install libstdc++.i686
已加载插件：fastestmirror, refresh-packagekit, security
设置安装进程 ……
已安装:
libstdc++.i686 0:4.4.7-11.el6
作为依赖被安装: libgcc.i686 0:4.4.7-11.el6
完毕！
[root@cSlave00 RPM]# rpm -ivh cndrvcups-common-2.60-1.x86_64.rpm
Preparing… ########################################### [100%]
1:cndrvcups-common ########################################### [100%]
至此，终于解决了依赖问题，cndrvcups-common-2.60-1.x86_64.rpm终于安装成功。用同样的方法，也顺利地把cndrvcups-capt-2.60-1.x86_64.rpm安装成功。
总结：在安装rpm包的时候，如果出现类似libc.so.6 is needed by XXX的依赖问题，首先检查一下本机是否安装了相应的依赖库；如果本机确实已经安装了所需的依赖库而问题依旧，那就试试把相应的兼容包安装一下，问题应该就能迎刃而解了！


参考地址：http://raksmart.idcspy.com/781