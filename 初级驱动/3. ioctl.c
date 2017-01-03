- 概念
```
1. ioctl接口的概念和实现---给驱动发送命令
    实际是给驱动发送指令, 命令一般都是会自定义
    led1  ---  on/off
    led2  ---  on/off
    all   ---  on /off
```
		
---------------------
```
1.
用户端：
	  int ioctl(int d, int cmd, ...);
	  实例：
	  	 1. ioctl(fd, IOC_LED_ON);
		 2. ioctl(fd, IOC_LED_NUM_ON, 3);
	----------------------------------
内核： vfs :  sys_ioctl()
					|
			 filp->f_op->unlocked_ioctl(filp, cmd, arg);
	------------------------------------
驱动端：unlocked_ioctl（ fd,  cmd,）
		{
				switch(cmd){
						case 命令1：
							break;
						case 命令2：
							break;
				}
		}
	实例：
		long led_drv_ioctl(struct file *filp, unsigned int cmd, unsigned long args)
		{
			int num = args;
			switch(cmd){
				case IOC_LED_NUM_ON:
				gpio_request(S5PV210_GPC0(num),"ledx");
				gpio_direction_output(S5PV210_GPC0(num),1);
				gpio_free(S5PV210_GPC0(num));
				break;
			case IOC_LED_NUM_OFF:
				gpio_request(S5PV210_GPC0(num),"ledx");
				gpio_direction_output(S5PV210_GPC0(num),0);
				gpio_free(S5PV210_GPC0(num));
				break;
		}
------------------------------------------------------------------------------
2. 最关键的就是命令：自定义成一个整数
   2.1 直接指定一个值--有可能会和系统中自带的命令有冲突
	   #define   IOC_LED_ON   0x1234
	   #define   IOC_LED_OFF  0x1235
   2.2 用系统提供的接口去定义命令--建议
	   #define xxx _IO(type,nr)   //生成一个唯一的整数
	   #define xxx _IOR(type,nr,size)
	   #define xxx _IOW(type,nr,size)
	   #define xxx _IOWR(type,nr,size)
				参数1：魔幻数－一个字符(随便一个（都相同）)
				参数2：命令区分
				参数３：如果需要发送命令的同时，需要传递数据，就将数据类型填在这里
3. 实例：
	#define  IOC_LED_ON      _IO('L',  0x1234)    //全亮 ，不需要第三个参数
	#define  IOC_LED_OFF     _IO('L',  0x1235)     //全灭
	#define  IOC_LED_NUM_ON  _IOW('L', 0x1236, int)  //需要带最后一个参数，类型是int
	#define  IOC_LED_NUM_OFF _IOW('L', 0x1237, int)  
					
```
