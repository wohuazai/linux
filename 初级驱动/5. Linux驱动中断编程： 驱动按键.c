``` 
（有点像线程申请）
1. 编写处理中断的函数---申请中断
   int request_irq(unsigned int irq,irq_handler_t handler,unsigned long flags,const char * name,void * dev)
	
	参数1：中断的号码，按键
		  a. 外部中断--IRQ_EINT(号码)--irqs.h
		  b. 内部中断 mach/irqs.h   plat/irqs.h
			 #define IRQ_UART0		S5P_IRQ_VIC1(10)
			 #define IRQ_UART1		S5P_IRQ_VIC1(11)
		  c. gpio_to_irq();如果某个gpio口有中断功能，就用这个获取到

	参数２：中断的处理方法
		  irqreturn_t (*irq_handler_t)(int, void *);
		  参数1：表示当前的中断号码
		  参数２：request_irq第五个传参
		  实例：
			 irqreturn_t   key_irq_handle(int irqno, void *devid)
			 {
				return IRQ_HANDLED;
			 }　

	参数3：触发方式：内部触发，上升下降沿触发，高低电平触发
				#define IRQF_TRIGGER_NONE	  0x00000000
				#define IRQF_TRIGGER_RISING	  0x00000001
				#define IRQF_TRIGGER_FALLING  0x00000002
				#define IRQF_TRIGGER_HIGH	  0x00000004
				#define IRQF_TRIGGER_LOW	  0x00000008
	参数4： 字符串，自定义， 给用户查看 /proc/interrupts
	参数5：传递给参数２的数据，用于区分共享同一中断处理函数的不同的中断
	返回值： 非0表示出错


		释放中断：
				free_irq(unsigned int irq,void * dev_id)
```

----------------------------------
