```
����
   1. ����������Ҫ��ȡĳ����Դ��ʱ�������Դ���ɴ��ô���̾ͻ�����       
     ���ó�����Ȩ���� ������ݿɴ���̾ͻᱻ���� 
  
   2. linux�󲿷ֵĺ���Ĭ�϶�������, ʵ������Ҫ����֧��(����CPUռ��)
      �磺 scanf(); accept(),  read(); readfrom...	 	 
```

-----------------

###a. ʵ������
 
```
wait_queue_head_t��wq_head;   //����һ��������

1. ��ʼ���ȴ�����ͷ( __init )
	init_waitqueue_head(wait_queue_head_t *q); //ԭ��
	
2. ��ĳ���ض��ĵط� �� xxx_irq( )����������(�жϲ���ʱ)
   //�ڲ��лṹ��һ���Դ�������(�ڵ�--wait_queue_t),���뵽�ȴ�����ͷ��
	wake_up_interruptible(wait_queue_head_t *q )

	��ʹ	condition = 1 ;

3. ��ĳ���ض��ĵط���xxx_read ()������

       wait_event_interruptible(wait_queue_head_t wq,  int condition)
	   ����1���ȴ�����ͷ����
	   ����2����ʾ����������Ϊ�٣��ͻ����ߣ�Ϊ�治������


   

```
-------------------

		
###b. ������:  

```		
˼�룺������Դ��ʱ�򣬸���Դ��û����Դ��������
```
```
Ӧ�����ó�������
	open("/dev/key1", O_RDWR|O_NONBLOCK);
	read();
-------------------------------------------------------
���������������ͷ�����
	 if((filp->f_flags & O_NONBLOCK ) &&  !key_dev->have_data)
	 {
		return -EAGAIN;
	 }

```
----------------
```
�޸Ļ���������

	Ӧ�ó������÷�������
	fd = open("/dev/key1", O_RDWR);
	fd = socket(AF_INET, SOCK_STREAM, 0);
					
	//��֮����Ҫ�޸�ԭ��flag
	int flags = fcntl(fd, F_GETFL, NULL);
	flags |= O_NONBLOCK;
	fcntl(fd, F_SETFL, flags);
```
