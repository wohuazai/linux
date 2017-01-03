###客户端 app

-------------------
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


struct key_event{
	int  code ; // 按键的键值， 比如KEY_DOWN,  KEY_POWER
	int value; // 按键的状态，按下为1， 抬起为0
};


int main(int argc, char *argv[])
{

	int fd;

	int ret;
	char kbbuf[128];
	struct key_event event;
	
	fd = open("/dev/key1", O_RDWR);
	if(fd < 0)
	{
		perror("open");
		exit(1);
	}

	struct pollfd pfd[2];

	pfd[0].fd = 0;  //监控标准输入          
	pfd[0].events = POLLIN;        
	
	pfd[1].fd = fd; //监控按键                          //1.监控对象
	pfd[1].events = POLLIN;                            //2.监控模式

	while(1)
	{
			ret = poll(pfd, 2, -1);
			if(ret > 0)
			{
				if(pfd[0].revents & POLLIN)           //3.返回值(返回POLLIN时为真)
				{
					fgets(kbbuf, 128, stdin);
					printf("kbbuf = %s\n", kbbuf);
				}
				
				if(pfd[1].revents & POLLIN)          //3.返回值(返回POLLIN时为真)
				{
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
			}
	
		}

	close(fd);

	return 0;
}
```

---------------------------------------------------
------------------------------------------ 
###程序


 思想：
![有数据就返回POLLIN，没数据返回VFS区](http://img.blog.csdn.net/20160624194038145)
口诀：有数据是返回POLLIN，没数据返回0;
```
unsigned int key_drv_poll(struct file *filp, struct poll_table_struct *pts)
{
	printk("-------^_^   %s-----------\n", __FUNCTION__);
	unsigned int  mask = 0;
	
	// 将当前的等待队列注册到vfs中
	poll_wait(filp, &key_dev->wq_head, pts);


	//如果有数据的时候，返回一个POLLIN
	if(key_dev->have_data)
			mask |= POLLIN;

	return mask;        //有数据是返回POLLIN，没数据返回0;
}
```
----------------------
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
		.name = "key1_eint0",        // 中断名字
		.irqno = IRQ_EINT(0),         //  中断号
		.gpionum = S5PV210_GPH0(0),     //gpio脚
		.flags = IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,  //上下沿触发（触发方式）
		.code = KEY_UP,    //当前的按键（上下左右）
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
	[3] = {
		.name = "key4_eint3",
		.irqno = IRQ_EINT(3),
		.gpionum = S5PV210_GPH0(3),
		.flags = IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
		.code = KEY_RIGHT,

	},

	[4] = {
		.name = "key5_eint4",
		.irqno = IRQ_EINT(4),
		.gpionum = S5PV210_GPH0(4),
		.flags = IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
		.code = KEY_ENTER,

	},
	[5] = {
		.name = "key6_eint5",
		.irqno = IRQ_EINT(5),
		.gpionum = S5PV210_GPH0(5),
		.flags = IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
		.code = KEY_ESC,

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


	return 0;

}


long key_drv_ioctl(struct file *filp, unsigned int cmd, unsigned long args)
{

	
	return 0;

}


ssize_t key_drv_read (struct file *filp, char __user *buf, size_t count, loff_t *fpos)
{
	//printk("-------^_^   %s-----------\n", __FUNCTION__);
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

	return mask;        //有数据是返回POLLIN，没数据返回0;
}


const struct file_operations  key_fops = {
	.open = key_drv_open,
	.write = key_drv_write,
	.read = key_drv_read,
	.release = key_drv_close,
	.unlocked_ioctl = key_drv_ioctl,
	.poll = key_drv_poll,
};



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

	wake_up_interruptible(&key_dev->wq_head);//会唤醒
	key_dev->have_data = 1; //表示有数据

	
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

               //    1.   中断号  中断处理函数  触发方式  中断名字 传递给处理函数的值                        
		ret = request_irq(irqno, key_irq_handler, flags,name, &allkeys[i]);   //中断申请
		if(ret != 0)
		{
			printk(KERN_ERR "request_irq error  : i=%d\n", i);
			ret = -EBUSY;
			goto err_4;
		}
		
	}
	
	//初始化等待队列头
	init_waitqueue_head(&key_dev->wq_head);
	
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
