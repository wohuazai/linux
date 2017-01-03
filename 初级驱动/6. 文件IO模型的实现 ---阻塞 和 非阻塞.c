```
概念
   1. 阻塞：当需要获取某个资源的时候，如果资源不可达，那么进程就会休眠       
     （让出调度权）， 如果数据可达，进程就会被唤醒 
  
   2. linux大部分的函数默认都是休眠, 实际是需要驱动支持(降低CPU占用)
      如： scanf(); accept(),  read(); readfrom...	 	 
```

-----------------

###a. 实现阻塞
 
```
wait_queue_head_t　wq_head;   //声明一个变量；

1. 初始化等待队列头( __init )
	init_waitqueue_head(wait_queue_head_t *q); //原型
	
2. 在某个特定的地方 （ xxx_irq( )）进行休眠(中断产生时)
   //内部中会构建一个对待队列项(节点--wait_queue_t),加入到等待队里头中
	wake_up_interruptible(wait_queue_head_t *q )

	并使	condition = 1 ;

3. 在某个特定的地方（xxx_read ()）唤醒

       wait_event_interruptible(wait_queue_head_t wq,  int condition)
	   参数1：等待队列头对象
	   参数2：表示条件，条件为假，就会休眠，为真不会休眠


   

```
-------------------

		
###b. 非阻塞:  

```		
思想：当有资源的时候，给资源，没有资源给错误码
```
```
应用设置成阻塞：
	open("/dev/key1", O_RDWR|O_NONBLOCK);
	read();
-------------------------------------------------------
驱动：区分阻塞和非阻塞
	 if((filp->f_flags & O_NONBLOCK ) &&  !key_dev->have_data)
	 {
		return -EAGAIN;
	 }

```
----------------
```
修改环境变量：

	应用程序设置非阻塞：
	fd = open("/dev/key1", O_RDWR);
	fd = socket(AF_INET, SOCK_STREAM, 0);
					
	//打开之后需要修改原来flag
	int flags = fcntl(fd, F_GETFL, NULL);
	flags |= O_NONBLOCK;
	fcntl(fd, F_SETFL, flags);
```
