```
 1. Դ������
 2. ��ѹԴ��
 3.  ����Դ��Ŀ¼���޸�Makefile  
 4. ����Դ��
 5. ����
 6. ��װ 
 7. ���밲װĿ¼��
 8. ����������Ҫ��Ŀ¼
 9. ��Ӷ�̬��
 10. ���ϵͳ�����ļ�
 11. ��_install/etc������ļ�fstab
 12. ��_install/etc�´���init.dĿ¼������init.d�´���rcS�ļ�
 13. ��_install/etc�����profile�ļ�
 14. �豸�ļ�����
 15. ```

```
1��  Դ������

����ѡ��İ汾��busybox-1.17.3.tar.bz2����·��Ϊ��
http://busybox.net/downloads/
```

```
2��  ��ѹԴ��

$ tar  xvf  busybox-1.17.3.tar.bz2
```

```
3��  ����Դ��Ŀ¼���޸�Makefile

$ cd  busybox-1.17.3
$ vim Makefile
  ARCH = arm
  CROSS_COMPILE = arm-none-linux-gnueabi-

```

```
4��  ����Դ��

$ make menuconfig
Busybox Settings ---> 
       Build Options --->
              [*] Build BusyBox as a static binary (no shared libs)
              [ ] Force NOMMU build
              [ ] Build with Large File Support (for accessing files > 2 GB)
              ()Cross Compiler prefix
              () Additional CFLAGS
              
              Installation Options  --->
                        [*] Don't use /usr
                                    Applets links (as soft-links)  --->
                        (./_install) BusyBox installation prefix (NEW)
```

```
5��  ����

$ make

```

```
6��  ��װ 

$ make  install 
  busyboxĬ�ϰ�װ·��ΪԴ��Ŀ¼�µ�_install

```

```
7��  ���밲װĿ¼��

$ cd  _install
$ ls
bin  linuxrc  sbin  
```

```
8��  ����������Ҫ��Ŀ¼

$ mkdir  dev  etc  mnt  proc  var  tmp  sys  root  lib home usr
```

```
9��  ��Ӷ�̬��

?  ���������еĿ⿽����_installĿ¼�µ�libĿ¼
$cp -raf /usr/local/toolchain-4.5.1-farsight/arm-none-linux-gnueabi/lib/*  lib*/
?  ɾ�����ļ��еķ��ű�Ҳ������
$ rm -rf lib/*.a
$ arm-none-linux-gnueabi-strip  lib/*

```

```
10�� ���ϵͳ�����ļ�

��_install/etc������ļ�inittab���ļ��������£�

::sysinit:/etc/init.d/rcS
::askfirst:-/bin/ash
::ctrlaltdel:/sbin/reboot
::shutdown:/sbin/swapoff -a
::shutdown:/bin/umount -a -r
::restart:/sbin/init

��ʽ˵��:
<id>:<runlevels>:<action>:<process> : ��ʲôʱ������ʲô����

id:		   ������ʹ�õ��ն���
runlevels: ������д
<action>:  Valid actions include: sysinit, respawn, askfirst, wait, once, restart, ctrlaltdel, and shutdown.

sysinit��  ϵͳ��ʼ����ʱ��,������ǰ��
respawn��askfirst��һ���ģ� ���process������ֹ��,ϵͳ�ὲ������
askfirst�� ������ʾPlease press Enter to activate this console
wait��     �ý���һ��Ҫִ����,����ִ����һ��
once��     �������������ֹ,��ô��������
```

```
11����_install/etc������ļ�fstab���ļ��������£�

#device     mount-point     type        options         dump     fsck order
proc          /proc         proc        defaults        0        0
tmpfs         /tmp          tmpfs       defaults        0        0
sysfs         /sys          sysfs       defaults        0        0
tmpfs         /dev          tmpfs       defaults        0        0

�������ǹ��ص��ļ�ϵͳ������proc��sysfs��tmpfs�����ں���proc��sysfsĬ�϶�֧�֣���tmpfs��û��֧�ֵģ�������Ҫ���tmpfs��֧��

�޸��ں����ã�
File systems --->
       Pseudo filesystems ---> 
             [*] /proc file system support (NEW) 
              [*] sysfs file system support (NEW) 
              [*] Virtual memory file system support (former shm fs) 
              [*]   Tmpfs POSIX Access Control Lists 
���±����ں�
	$ make zImage -j2

```

```
12�� ��_install/etc�´���init.dĿ¼������init.d�´���rcS�ļ���rcS�ļ�����Ϊ��
          
#!/bin/sh
echo ">>>>>>>> in /etc/init.d/rcS<<<<<<<<<<<<<<<<<"
/bin/mount -a
echo /sbin/mdev > /proc/sys/kernel/hotplug
/sbin/mdev -s
-----------------------------------------------------------
mdev�������Զ������豸�ڵ�    
    ΪrcS��ӿ�ִ��Ȩ�ޣ�
       $ chmod   a+x  init.d/rcS
```

```
13����_install/etc�����profile�ļ����ļ�����Ϊ��

#!/bin/sh
export HOSTNAME=farsight
export USER=root
export HOME=root
export PS1="[$USER@$HOSTNAME \W]\# "
PATH=/bin:/sbin:/usr/bin:/usr/sbin
LD_LIBRARY_PATH=/lib:/usr/lib:$LD_LIBRARY_PATH
export PATH  LD_LIBRARY_PATH

```

```
14�� �豸�ļ�����

���ļ�ϵͳ����һ���豸�ڵ��Ǳ���ģ���dev�´���console�ڵ�
$ mknod   dev/console  c  5  1
���ϵĲ���������linux��Ŀ¼����Ҫ�������ļ�������ֱ��ʹ��

$ sudo  cp   -raf  _install  /opt/myrootfs
```

####NFS����
```
1. �����õĸ�Ŀ¼���Ƴ�/opt/rootfs(������Ѿ������Ŀ¼,��Ҫע����)
   $ sudo  cp   -raf  _install  /opt/rootfs
   1.2 ��ubuntu�н�/opt/rootfs���ó�nfs�ļ�ϵͳ
	   �޸�ubuntu��/etc/exports: ��/opt/rootfs���ó�nfs�ļ�ϵͳ��ʽ
	   /opt/rootfs            *(subtree_check,rw,no_root_squash,async)
	   
	   ������������ sudo service nfs-kernel-server restart
2. ����uboot��������
   # setenv  bootcmd  tftp  20800000  zImage \;  go  20800000
   # setenv  bootargs  root=nfs  nfsroot=192.168.7.100:/opt/rootfs  init=/linuxrc console=ttySAC0,115200  ip=192.168.7.200
   # saveenv

```

###����뿪��������һ���Լ���ִ�г��򣬿��������´���
```
12. �����һ����ִ�г���һ��Ҫ����binĿ¼��  /bin/hello_arm
 
    /etc/init.d/rcS��������ִ�г���(hello_arm Ϊ��ִ�г���)
	����1��ֱ����/etc/init.d/rcS���һ����
          /bin/hello_arm
           
	����2��
		1.��/etc Ŀ¼�д���һ���ļ���Ϊlocal,Ȼ����local�д���mysrv.sh�ű��ļ���
		2.��mysrv.sh�ű���д��
			#!/bin/ash
		    /bin/hello_arm 
		3.��/etc/init.d/rcS���
		   /etc/local/mysrv.sh

	��������������
```

