```
 1. 三个重要脚本文件
 	1. /etc/inittab 
		#系统最先执行,结束后才继续其它动作		
		::sysinit:/etc/init.d/rcS    		
		#和repsawn（重生） 一样，结束后系统让它再跑起来		
		::askfirst:-/bin/ash
		#按下Ctrl+Alt+Del组合键		
		::ctrlaltdel:/sbin/reboot
		#系统关机时执行
		::shutdown:/sbin/swapoff -a
		::shutdown:/bin/umount -a -r
		#先重新读取、解析/etc/inittab 文件，再执行restart程序		
		::restart:/sbin/init
		
	2. /etc/ini.d/rcS
	
	3. /etc/fstab
```


```
 1. 源码下载
 2. 解压源码
 3.  进入源码目录，修改Makefile  
 4. 配置源码
 5. 编译
 6. 安装 
 7. 进入安装目录下
 8. 创建其他需要的目录
 9. 添加动态库
 10. 添加系统启动文件
 11. 在_install/etc下添加文件fstab
 12. 在_install/etc下创建init.d目录，并在init.d下创建rcS文件
 13. 在_install/etc下添加profile文件
 14. 设备文件创建
 15. ```

```
1、  源码下载

我们选择的版本是busybox-1.17.3.tar.bz2下载路径为：
http://busybox.net/downloads/
```

```
2、  解压源码

$ tar  xvf  busybox-1.17.3.tar.bz2
```

```
3、  进入源码目录，修改Makefile

$ cd  busybox-1.17.3
$ vim Makefile
  ARCH = arm
  CROSS_COMPILE = arm-none-linux-gnueabi-

```

```
4、  配置源码

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
5、  编译

$ make

```

```
6、  安装 

$ make  install 
  busybox默认安装路径为源码目录下的_install

```

```
7、  进入安装目录下

$ cd  _install
$ ls
bin  linuxrc  sbin  
```

```
8、  创建其他需要的目录

$ mkdir  dev  etc  mnt  proc  var  tmp  sys  root  lib home usr
```

```
9、  添加动态库

?  将工具链中的库拷贝到_install目录下的lib目录
$cp -raf /usr/local/toolchain-4.5.1-farsight/arm-none-linux-gnueabi/lib/*  lib/   	*/
?  删除库文件中的符号表，也叫瘦身
$ rm -rf lib/*.a   									*/
$ arm-none-linux-gnueabi-strip  lib/*                     				*/

```

```
10、 添加系统启动文件

在_install/etc下添加文件inittab，文件内容如下：

::sysinit:/etc/init.d/rcS
::askfirst:-/bin/ash
::ctrlaltdel:/sbin/reboot
::shutdown:/sbin/swapoff -a
::shutdown:/bin/umount -a -r
::restart:/sbin/init

格式说明:
<id>:<runlevels>:<action>:<process> : 在什么时间启动什么进程

id:		   进程所使用的终端名
runlevels: 不用填写
<action>:  Valid actions include: sysinit, respawn, askfirst, wait, once, restart, ctrlaltdel, and shutdown.

sysinit：  系统初始化的时候,放在最前面
respawn、askfirst是一样的： 如果process意外终止啦,系统会讲其重启
askfirst： 会有提示Please press Enter to activate this console
wait：     该进程一定要执行完,才能执行下一个
once：     进程如果意外终止,那么不会重启
```

```
11，在_install/etc下添加文件fstab，文件内容如下：

#device     mount-point     type        options         dump     fsck order
proc          /proc         proc        defaults        0        0
tmpfs         /tmp          tmpfs       defaults        0        0
sysfs         /sys          sysfs       defaults        0        0
tmpfs         /dev          tmpfs       defaults        0        0

这里我们挂载的文件系统有三个proc、sysfs和tmpfs。在内核中proc和sysfs默认都支持，而tmpfs是没有支持的，我们需要添加tmpfs的支持

修改内核配置：
File systems --->
       Pseudo filesystems ---> 
             [*] /proc file system support (NEW) 
              [*] sysfs file system support (NEW) 
              [*] Virtual memory file system support (former shm fs) 
              [*]   Tmpfs POSIX Access Control Lists 
重新编译内核
	$ make zImage -j2

```

```
12， 在_install/etc下创建init.d目录，并在init.d下创建rcS文件，rcS文件内容为：
          
#!/bin/sh
echo ">>>>>>>> in /etc/init.d/rcS<<<<<<<<<<<<<<<<<"
#执行 etc/fstab 里面的挂载命令
/bin/mount -a
echo /sbin/mdev > /proc/sys/kernel/hotplug

#挂载/sys/mdev 目录下的设备节点
/sbin/mdev -s
-----------------------------------------------------------
mdev是用于自动创建设备节点    

为rcS添加可执行权限：
	$ chmod   a+x  init.d/rcS
```

```
13，在_install/etc下添加profile文件，文件内容为：

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
14、 设备文件创建

根文件系统中有一个设备节点是必须的，在dev下手动创建console节点
$ mknod   dev/console  c  5  1
以上的步骤就完成了linux根目录所需要的所有文件，可以直接使用

$ sudo  cp   -raf  _install  /opt/myrootfs
```

####NFS测试
```
1. 将做好的根目录复制成/opt/rootfs(如果你已经有这个目录,就要注意啦)
   $ sudo  cp   -raf  _install  /opt/rootfs
   1.2 在ubuntu中将/opt/rootfs设置成nfs文件系统
	   修改ubuntu的/etc/exports: 将/opt/rootfs设置成nfs文件系统格式
	   /opt/rootfs            *(subtree_check,rw,no_root_squash,async)
	   
	   重启服务器： sudo service nfs-kernel-server restart
2. 设置uboot环境变量
   # setenv  bootcmd  tftp  20800000  zImage \;  go  20800000
   # setenv  bootargs  root=nfs  nfsroot=192.168.7.100:/opt/rootfs  init=/linuxrc console=ttySAC0,115200  ip=192.168.7.200
   # saveenv

```

###如果想开机就运行一个自己可执行程序，可以做如下处理
```
12. 编译出一个可执行程序，一定要放在bin目录下  /bin/hello_arm
 
    /etc/init.d/rcS中启动可执行程序(hello_arm 为可执行程序)
	方法1：直接在/etc/init.d/rcS添加一条：
          /bin/hello_arm
           
	方法2：
		1.在/etc 目录中创建一个文件夹为local,然后在local中创建mysrv.sh脚本文件。
		2.在mysrv.sh脚本中写入
			#!/bin/ash
		    /bin/hello_arm 
		3.在/etc/init.d/rcS添加
		   /etc/local/mysrv.sh

	重新启动开发板
```

