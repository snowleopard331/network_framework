MySQL-devel-5.6.30-1.linux_glibc2.5.x86_64          �ļ�Ϊmysql����ͷ�ļ�rpm��
MySQL-shared-compat-5.6.30-1.linux_glibc2.5.i386    �ļ�Ϊmysql������rpm��

��������װ�������ض���װ·��

��װ ����(root�û�)



��װ����
    rpm -ivha MySQL-devel-5.6.30-1.linux_glibc2.5.x86_64.rpm            ͷ�ļ�
    rpm -ivh -a MySQL-shared-compat-5.6.30-1.linux_glibc2.5.i386.rpm    ���ļ�

ж������
    rpm -e MySQL-devel 

cp -vr libmysqlclient* /home/yunfei/myworld/network_ramework/dep/mysql/lib
cp -vr mysql /home/yunfei/myworld/network_ramework/dep/mysql/lib/
cp -vr mysql /home/yunfei/myworld/network_ramework/dep/mysql/include/




///////////////////////////////////////////////////////////////////////////////

����������  �ο�һ�´�

��ӡ����������������rpm��װ����
cndrvcups-common-2.60-1.x86_64.rpm��cndrvcups-capt-2.60-1.x86_64.rpm��
ִ�а�װ����rpm -ivh cndrvcups-common-2.60-1.x86_64.rpm��������������󣬴���������£�
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
�����Ͻ���һ������������������ǰ�װȱ�ٵ������⼴�ɡ������������֣���������Щ���Ѿ���װ�ˡ�
����libc.so.6���ÿ��Ӧ�����������Ϊglibc��
[root@cSlave00 RPM]# yum list glibc*
�Ѽ��ز����fastestmirror, refresh-packagekit, security
Loading mirror speeds from cached hostfile
* base: mirrors.cug.edu.cn
* extras: mirrors.cug.edu.cn
* updates: mirrors.skyshe.cn
�Ѱ�װ������� glibc.x86_64 2.12-1.149.el6
@anaconda-CentOS-201410241409.x86_64/6.6
glibc-common.x86_64 2.12-1.149.el6
@anaconda-CentOS-201410241409.x86_64/6.6
glibc-devel.x86_64 2.12-1.149.el6
@anaconda-CentOS-201410241409.x86_64/6.6
glibc-headers.x86_64 2.12-1.149.el6
@anaconda-CentOS-201410241409.x86_64/6.6
�ɰ�װ�������
glibc.i686 2.12-1.149.el6 base
glibc-devel.i686 2.12-1.149.el6 base
glibc-static.i686 2.12-1.149.el6 base
glibc-static.x86_64 2.12-1.149.el6 base
glibc-utils.x86_64 2.12-1.149.el6 base
��ô�������ˣ���Ȼ�Ѿ���װ��libc.so.6��Ϊʲô������ʾȱ�ٸÿ��أ������Ұ��ң��ٶ������������޹���ֻ�ܼ�ϣ���ڹȸ衣���ڣ��㶨��ѧ�������ȸ������ˣ������������ص��עӢ�Ľ�������������Stack Overflow���ҵ������Ƶ��������𣬵������鿴ԭ�ġ�
����ֻժ¼�ؼ��ļ��仰��
In Red Hat Enterprise Linux 5, if a package was available for both the main and the compatibility architectures, both architectures of the package were installed by default.In Red Hat Enterprise Linux 6, only the package for the primary architecture is installed by default.To avoid problems during the backup-archive client and API installation on a 64-bit machine, be sure to install libstdc++ and compat-libstdc++.
������˵����Red Hat Enterprise Linux 6��ʼ��Ĭ��ֻ��װ���ܹ�����Ҫ�İ���������װ���ݼܹ��İ���Ҳ����˵��64λϵͳĬ��ֻ��װ64λ���������Ϊ�����������⣬��64λϵͳ�У�Ҫͬʱ��װ64λ�İ���32λ�ļ��ݰ���CentOS�ʹ�Red Hat Enterprise Linux 6�Ĺ�ϵ�Ͳ�����˵�ˣ�����Ҹо�����ش�ȽϿ��ף��Ͻ����԰ɡ�
[root@cSlave00 RPM]# yum install glibc.i686
�Ѽ��ز����fastestmirror, refresh-packagekit, security
���ð�װ����
����
�Ѱ�װ:
glibc.i686 0:2.12-1.149.el6
��Ϊ��������װ: nss-softokn-freebl.i686 0:3.14.3-18.el6_6
��Ϊ����������: nss-softokn-freebl.x86_64 0:3.14.3-18.el6_6 ��ϣ�
[root@cSlave00 RPM]# rpm -ivh cndrvcups-common-2.60-1.x86_64.rpm error: Failed
dependencies:
libstdc++.so.6 is needed by cndrvcups-common-2.60-1.x86_64
libstdc++.so.6(CXXABI_1.3) is needed by
cndrvcups-common-2.60-1.x86_64
����������⣬��Ȼ������ʾlibc.so.6�ˡ��������libstdc++.so.6��
[root@cSlave00 RPM]# yum list libstdc++*
�Ѽ��ز����fastestmirror, refresh-packagekit, security
Loading mirror speeds from cached hostfile
* base: ftp.sjtu.edu.cn
* extras: mirrors.163.com
* updates: ftp.sjtu.edu.cn
�Ѱ�װ�������
libstdc++.x86_64 4.4.7-11.el6
@anaconda-CentOS-201410241409.x86_64/6.6
�ɰ�װ�������
libstdc++.i686 4.4.7-11.el6 base libstdc++-devel.i686 4.4.7-11.el6 base
libstdc++-devel.x86_64 4.4.7-11.el6 base
libstdc++-docs.x86_64 4.4.7-11.el6 base
[root@cSlave00 RPM]# yum install libstdc++.i686
�Ѽ��ز����fastestmirror, refresh-packagekit, security
���ð�װ���� ����
�Ѱ�װ:
libstdc++.i686 0:4.4.7-11.el6
��Ϊ��������װ: libgcc.i686 0:4.4.7-11.el6
��ϣ�
[root@cSlave00 RPM]# rpm -ivh cndrvcups-common-2.60-1.x86_64.rpm
Preparing�� ########################################### [100%]
1:cndrvcups-common ########################################### [100%]
���ˣ����ڽ�����������⣬cndrvcups-common-2.60-1.x86_64.rpm���ڰ�װ�ɹ�����ͬ���ķ�����Ҳ˳���ذ�cndrvcups-capt-2.60-1.x86_64.rpm��װ�ɹ���
�ܽ᣺�ڰ�װrpm����ʱ�������������libc.so.6 is needed by XXX���������⣬���ȼ��һ�±����Ƿ�װ����Ӧ�������⣻�������ȷʵ�Ѿ���װ���������������������ɣ��Ǿ����԰���Ӧ�ļ��ݰ���װһ�£�����Ӧ�þ���ӭ�ж����ˣ�


�ο���ַ��http://raksmart.idcspy.com/781