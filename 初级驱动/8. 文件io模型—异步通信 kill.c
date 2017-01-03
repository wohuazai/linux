###客户端
 - **要点三**（ 设置信号SIGIO的处理）
      1. 设置信号处理方法（当中断型号来之后自动跳到catch_signal( ) 函数执行
          signal(SIGIO, catch_signal);
      2. 设置信号的属主进程
         fcntl(fd, F_SETOWN, getpid());
      3. 将读写模式设置成异步模式
        unsigned int flag = fcntl(fd, F_GETFL, 0);
        fcntl(fd, F_SETFL, flag | FASYNC);

-----------
```
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/input.h>
#include <poll.h>
#include <signal.h>


struct key_event{
    int  code ; // 按键的键值， 比如KEY_DOWN,  KEY_POWER
    int value; // 按键的状态，按下为1， 抬起为0
};

static int fd;

void catch_signal(int signo)
{
	int ret;

    struct key_event event;
    if(signo == SIGIO)
    {
		printf("we got a SIGIO\n");
		ret = read(fd, &event, sizeof(struct key_event));
		if(ret < 0)
		{
		    perror("read");
		    exit(1);
		}

		if(event.code == KEY_POWER)
		{
		    if(event.value)
		    {
				printf("__APP__ key power pressed\n");
		    }else
		    {
				printf("__APP__ key power up\n");
		    }
		}
		if(event.code == KEY_ENTER)
		{
		    if(event.value)
		    {
				printf("__APP__ key enter pressed\n");
		    }else
		    {
				printf("__APP__ key enter up\n");
		    }
		}
    }	

	printf("-----------------signal handled end\n");
}


int main(int argc, char *argv[])
{

   // int fd;

    int ret;
    char kbbuf[128];

    fd = open("/dev/key1", O_RDWR);
    if(fd < 0)
    {
		perror("open");
		exit(1);
    }

    //  需要设置信号SIGIO的处理
    // 1,设置信号处理方法
    signal(SIGIO, catch_signal);
    // 2,设置信号的属主进程
    fcntl(fd, F_SETOWN, getpid());
    //3， 将读写模式设置成异步模式
    unsigned int flag = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flag | FASYNC);


    //做其他的事情
    while(1){
		sleep(1);
		printf("I am very bored\n");
    }

	
    close(fd);

    return 0;
}
```
---------------------------------------------------
---------------------------------------------------
###驱动端
--------------------
  - 要点3条
```
	1.  需要一个struct fasync_struct 指针,不需要自己分配
		struct fasync_struct  *key_fasync //描述了信号是给谁, 信息的载体(包括了SIGIO发送给哪个进程的信息)
	
	2. 思想：
	   2.1 初始化struct fasync_struct指针, 以后就可以知道信号发送给谁
		   帮你去构建struct fasync_struct
	   2.3 将前两个参数给到struct fasync_struct ，其中filp中的f_owner就记录了用户空间设置的属主进程的pid
		
		实例：
			int key_irq_fcntl(int fd, struct file *file, int no)
	        {
			   //1.调用一个fasync_helper,  构建struct fasync_struct，描述信号的信息(信号是给谁的)
			   return fasync_helper(fd, filp,	on, &key_dev->key_fasync);
			}
	3.  在（中断的最后）发送信号（ SIGIO）
				kill_fasync(struct fasync_struct * * fp,int sig,int band);
		实例：
				kill_fasync(&key_dev->key_fasync, SIGIO, POLLIN);
        
```
------------------------
```
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/input.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/poll.h>

#include <asm/uaccess.h>
#include <asm/io.h>


// 设计一个表示按键数据的对象
struct key_event{
	int  code ; // 按键的键值， 比如KEY_DOWN,  KEY_POWER
	int value; // 按键的状态，按下为1， 抬起为0
};

// 面向对象---将任何都看成对象， struct 就是对象
//设计一个对象类型，描述当前的设备信息，同时是一个全局的设备对象
struct s5pv210_key{
	int dev_major; //用老的注册设备号的方式
	struct class *cls;
	struct device *dev;
	int irqno; //表示设备的中断号码
	int testdata;
	struct key_event event;//存放按键的数据
	wait_queue_head_t wq_head;
	int have_data; //表示标志位，表示是否有数据
	struct fasync_struct  *key_fasync; //指向一个被分配之后的struct fasync_struct空间
	struct tasklet_struct mytasklet;
};





//设计一个对象，描述是按键的信息
/*
1， 中断号
	2， gpio号码
	3， 名字
	4， 按键的类型
*/
struct key_info{
	char *name;
	int irqno;
	int gpionum;
	int code;
	int flags;  //触发方式
};
//设置所有按键的信息
struct key_info  allkeys[] = {
	[0] = {
		.name = "key1_eint0",
		.irqno = IRQ_EINT(0),
		.gpionum = S5PV210_GPH0(0),
		.flags = IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
		.code = KEY_UP,
	},

	[1] = {
		.name = "key2_eint1",
		.irqno = IRQ_EINT(1),
		.gpionum = S5PV210_GPH0(1),
		.flags = IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
		.code = KEY_DOWN,
	},
	[2] = {
		.name = "key3_eint2",
		.irqno = IRQ_EINT(2),
		.gpionum = S5PV210_GPH0(2),
		.flags = IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
		.code = KEY_LEFT,
                },
	[6] = {
		.name = "key7_eint22",
		.irqno = IRQ_EINT16_31,
		.gpionum = S5PV210_GPH2(6),
		.flags = IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
		.code = KEY_HOME,
	},
	[7] = {
		.name = "key8_eint23",
		.irqno = IRQ_EINT16_31,
		.gpionum = S5PV210_GPH2(7),
		.flags = IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
		.code = KEY_POWER,
	},
};


//设置对象
struct  s5pv210_key  *key_dev;

int key_drv_open(struct inode *inode, struct file *filp)
{
	//一般也是做初始化动作
	printk("-------^_^   %s-----------\n", __FUNCTION__);

	memset(&key_dev->event,0, sizeof(struct key_event));
	key_dev->have_data = 0;

	return 0;

}

// write(fd, buf, size);
ssize_t key_drv_write(struct file *filp, const char __user *buf, size_t count, loff_t *fpos)
{


	return 0;

}
int key_drv_close(struct inode *inode, struct file *filp)
{
	printk("-------^_^   %s-----------\n", __FUNCTION__);

	fasync_helper(-1, filp, 0, &key_dev->key_fasync);//释放key_dev->key_fasync

	return 0;

}


long key_drv_ioctl(struct file *filp, unsigned int cmd, unsigned long args)
{

	
	return 0;

}


ssize_t key_drv_read (struct file *filp, char __user *buf, size_t count, loff_t *fpos)
{
	printk("-------^_^   %s------have= %d-----\n", __FUNCTION__, key_dev->have_data);
	int ret;
	
	//区分阻塞还是非阻塞
	if((filp->f_flags & O_NONBLOCK ) &&  !key_dev->have_data)
		return -EAGAIN;
	
	// 没有数据就休眠--have_data如果0表示没有数据,就需要等
	wait_event_interruptible(key_dev->wq_head, key_dev->have_data);

	ret = copy_to_user(buf, &key_dev->event, count);
	if(ret > 0)
	{
		printk("copy_to_user error\n");
		return -EFAULT;
	}

	// 清空后，接着去收其他的数据
	memset(&key_dev->event,0, sizeof(struct key_event));
	key_dev->have_data = 0; // 重新设置成没有数据

	
	return count;

}


unsigned int key_drv_poll(struct file *filp, struct poll_table_struct *pts)
{
	printk("-------^_^   %s-----------\n", __FUNCTION__);
	unsigned int  mask = 0;
	
	// 将当前的等待队列注册到vfs中
	poll_wait(filp, &key_dev->wq_head, pts);


	//如果有数据的时候，返回一个POLLIN
	if(key_dev->have_data)
			mask |= POLLIN;

	return mask;
}

int key_drv_fasync (int fd, struct file *filp, int on)
{
	//调用一个fasync_helper,  构建struct fasync_struct，描述信号的信息(信号是给谁的)

	return fasync_helper(fd, filp,  on, &key_dev->key_fasync);

}


const struct file_operations  key_fops = {
	.open = key_drv_open,
	.write = key_drv_write,
	.read = key_drv_read,
	.release = key_drv_close,
	.unlocked_ioctl = key_drv_ioctl,
	.poll = key_drv_poll,
	.fasync = key_drv_fasync,
};


void tasklet_half_irq_handle(unsigned long data)
{
	printk("-------^_^   %s-----------\n", __FUNCTION__);
	wake_up_interruptible(&key_dev->wq_head);//会唤醒
	key_dev->have_data = 1; //表示有数据

	///发送信号SIGIO信号给fasync_struct 结构体所描述的PID，触发应用程序的SIGIO信号处理函数
	kill_fasync(&key_dev->key_fasync, SIGIO, POLLIN);
	
}



irqreturn_t key_irq_handler(int irqno, void *dev_id)
{

	printk("-------^_^   %s-----------\n", __FUNCTION__);

	//区分不同的中断
	struct key_info  *p = (struct key_info  *)dev_id;

	//区分是按下还是抬起
	int value;


	value = gpio_get_value(p->gpionum);
	
	if(value){
		//抬起
		printk("%s  up \n", p->name);
		key_dev->event.code = p->code;
		key_dev->event.value = 0;

	}else{
		//按下	
		printk("%s  pressed \n", p->name);
		key_dev->event.code =  p->code;
		key_dev->event.value = 1;
	}

	//启动下半部
	tasklet_schedule(&key_dev->mytasklet);
	
	return IRQ_HANDLED;
}



static  int __init key_drv_init(void)
{
	int ret;
	//申请资源
	// 实例化该对象
	//  GFP_KERNEL如果当前没有内存可分配，该函数会一直等
	key_dev = kzalloc(sizeof(struct s5pv210_key), GFP_KERNEL);
	if(key_dev == NULL)
	{
		printk(KERN_ERR "kzalloc error\n");
		return -ENOMEM;
	}
	// 1,  申请设备号
	key_dev->dev_major = register_chrdev(0, "s5pv210_key_drv", &key_fops);
	if(key_dev->dev_major < 0)
	{
		printk(KERN_ERR "kzalloc error\n");
		ret = -ENODEV;
		goto err_0;
	}

	
	// 2, 自动创建文件
	//  /sys/class/key_cls 文件夹
	key_dev->cls = class_create(THIS_MODULE, "key_cls");
	if(IS_ERR(key_dev->cls))
	{
		printk(KERN_ERR "class_create error\n");
		ret = PTR_ERR(key_dev->cls);
		goto err_1;
	}

	//  /sys/class/key_cls/key/
	//  /dev/key1
	key_dev->dev = device_create(key_dev->cls, NULL, MKDEV(key_dev->dev_major,1),NULL, "key1");
	if(IS_ERR(key_dev->dev))
	{
		printk(KERN_ERR "device_create error\n");
		ret = PTR_ERR(key_dev->dev);
		goto err_2;
	}


	// 4, 硬件的初始化---中断
	//
	//key_dev->irqno = IRQ_EINT(1);

	int i;
	int irqno;
	int flags;
	char *name;
	for(i=0; i<ARRAY_SIZE(allkeys); i++)
	{
		irqno = allkeys[i].irqno;
		if(irqno == IRQ_EINT16_31)
				irqno = gpio_to_irq(allkeys[i].gpionum);

		flags = allkeys[i].flags;
		name = allkeys[i].name;
		ret = request_irq(irqno, key_irq_handler, flags,name, &allkeys[i]);
		if(ret != 0)
		{
			printk(KERN_ERR "request_irq error  : i=%d\n", i);
			ret = -EBUSY;
			goto err_4;
		}
		
	}
	
	//初始化等待队列头
	init_waitqueue_head(&key_dev->wq_head);

	// 初始化tasklet，为下半部做准备
	tasklet_init(&key_dev->mytasklet,tasklet_half_irq_handle, 34);
	
	return 0;

err_4:
	i--;
	for(; i>0; i--)
	{
		irqno = gpio_to_irq(allkeys[i].gpionum);
		free_irq(irqno, &allkeys[i]);
	}
	
err_3:
	device_destroy(key_dev->cls, MKDEV(key_dev->dev_major,1));
err_2:
	class_destroy(key_dev->cls);

err_1:
	unregister_chrdev(key_dev->dev_major, "s5pv210_key_drv");
	
err_0:
	kfree(key_dev);
	return ret;

}


static void __exit  key_drv_exit(void)
{
	//释放资源
	int i;
	int irqno;
	for(i=0; i<ARRAY_SIZE(allkeys); i++)
	{	
		irqno = gpio_to_irq(allkeys[i].gpionum);
		free_irq(irqno, &allkeys[i]);
	}
	device_destroy(key_dev->cls, MKDEV(key_dev->dev_major,1));
	class_destroy(key_dev->cls);
	unregister_chrdev(key_dev->dev_major, "s5pv210_key_drv");
	
	kfree(key_dev);
}



module_init(key_drv_init);
module_exit(key_drv_exit);
MODULE_LICENSE("GPL");

```
