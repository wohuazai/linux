``` 
���е����߳����룩
1. ��д�����жϵĺ���---�����ж�
   int request_irq(unsigned int irq,irq_handler_t handler,unsigned long flags,const char * name,void * dev)
	
	����1���жϵĺ��룬����
		  a. �ⲿ�ж�--IRQ_EINT(����)--irqs.h
		  b. �ڲ��ж� mach/irqs.h   plat/irqs.h
			 #define IRQ_UART0		S5P_IRQ_VIC1(10)
			 #define IRQ_UART1		S5P_IRQ_VIC1(11)
		  c. gpio_to_irq();���ĳ��gpio�����жϹ��ܣ����������ȡ��

	���������жϵĴ�����
		  irqreturn_t (*irq_handler_t)(int, void *);
		  ����1����ʾ��ǰ���жϺ���
		  ��������request_irq���������
		  ʵ����
			 irqreturn_t   key_irq_handle(int irqno, void *devid)
			 {
				return IRQ_HANDLED;
			 }��

	����3��������ʽ���ڲ������������½��ش������ߵ͵�ƽ����
				#define IRQF_TRIGGER_NONE	  0x00000000
				#define IRQF_TRIGGER_RISING	  0x00000001
				#define IRQF_TRIGGER_FALLING  0x00000002
				#define IRQF_TRIGGER_HIGH	  0x00000004
				#define IRQF_TRIGGER_LOW	  0x00000008
	����4�� �ַ������Զ��壬 ���û��鿴 /proc/interrupts
	����5�����ݸ������������ݣ��������ֹ���ͬһ�жϴ������Ĳ�ͬ���ж�
	����ֵ�� ��0��ʾ����


		�ͷ��жϣ�
				free_irq(unsigned int irq,void * dev_id)
```

----------------------------------
