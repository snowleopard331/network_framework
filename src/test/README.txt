# using libs (as boost), it is necessary to add the library in the path 
# to the Shared config file /etc/ld.so.conf

Excute:
 cat /etc/ld.so.conf
 echo "/home/yunfei/myworld/network_ramework/dep/boost/lib" >> /etc/ld.so.conf
 ldconfig
