- ����
```
1. ioctl�ӿڵĸ����ʵ��---��������������
    ʵ���Ǹ���������ָ��, ����һ�㶼�ǻ��Զ���
    led1  ---  on/off
    led2  ---  on/off
    all   ---  on /off
```
		
---------------------
```
1.
�û��ˣ�
	  int ioctl(int d, int cmd, ...);
	  ʵ����
	  	 1. ioctl(fd, IOC_LED_ON);
		 2. ioctl(fd, IOC_LED_NUM_ON, 3);
	----------------------------------
�ںˣ� vfs :  sys_ioctl()
					|
			 filp->f_op->unlocked_ioctl(filp, cmd, arg);
	------------------------------------
�����ˣ�unlocked_ioctl�� fd,  cmd,��
		{
				switch(cmd){
						case ����1��
							break;
						case ����2��
							break;
				}
		}
	ʵ����
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
2. ��ؼ��ľ�������Զ����һ������
   2.1 ֱ��ָ��һ��ֵ--�п��ܻ��ϵͳ���Դ��������г�ͻ
	   #define   IOC_LED_ON   0x1234
	   #define   IOC_LED_OFF  0x1235
   2.2 ��ϵͳ�ṩ�Ľӿ�ȥ��������--����
	   #define xxx _IO(type,nr)   //����һ��Ψһ������
	   #define xxx _IOR(type,nr,size)
	   #define xxx _IOW(type,nr,size)
	   #define xxx _IOWR(type,nr,size)
				����1��ħ������һ���ַ�(���һ��������ͬ��)
				����2����������
				�������������Ҫ���������ͬʱ����Ҫ�������ݣ��ͽ�����������������
3. ʵ����
	#define  IOC_LED_ON      _IO('L',  0x1234)    //ȫ�� ������Ҫ����������
	#define  IOC_LED_OFF     _IO('L',  0x1235)     //ȫ��
	#define  IOC_LED_NUM_ON  _IOW('L', 0x1236, int)  //��Ҫ�����һ��������������int
	#define  IOC_LED_NUM_OFF _IOW('L', 0x1237, int)  
					
```
