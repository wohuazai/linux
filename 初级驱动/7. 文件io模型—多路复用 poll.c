###�ͻ��� app

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
	int  code ; // �����ļ�ֵ�� ����KEY_DOWN,  KEY_POWER
	int value; // ������״̬������Ϊ1�� ̧��Ϊ0
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

	pfd[0].fd = 0;  //��ر�׼����          
	pfd[0].events = POLLIN;        
	
	pfd[1].fd = fd; //��ذ���                          //1.��ض���
	pfd[1].events = POLLIN;                            //2.���ģʽ

	while(1)
	{
			ret = poll(pfd, 2, -1);
			if(ret > 0)
			{
				if(pfd[0].revents & POLLIN)           //3.����ֵ(����POLLINʱΪ��)
				{
					fgets(kbbuf, 128, stdin);
					printf("kbbuf = %s\n", kbbuf);
				}
				
				if(pfd[1].revents & POLLIN)          //3.����ֵ(����POLLINʱΪ��)
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
###����


 ˼�룺
![�����ݾͷ���POLLIN��û���ݷ���VFS��](http://img.blog.csdn.net/20160624194038145)
�ھ����������Ƿ���POLLIN��û���ݷ���0;
```
unsigned int key_drv_poll(struct file *filp, struct poll_table_struct *pts)
{
	printk("-------^_^   %s-----------\n", __FUNCTION__);
	unsigned int  mask = 0;
	
	// ����ǰ�ĵȴ�����ע�ᵽvfs��
	poll_wait(filp, &key_dev->wq_head, pts);


	//��������ݵ�ʱ�򣬷���һ��POLLIN
	if(key_dev->have_data)
			mask |= POLLIN;

	return mask;        //�������Ƿ���POLLIN��û���ݷ���0;
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


// ���һ����ʾ�������ݵĶ���
struct key_event{
	int  code ; // �����ļ�ֵ�� ����KEY_DOWN,  KEY_POWER
	int value; // ������״̬������Ϊ1�� ̧��Ϊ0
};

// �������---���κζ����ɶ��� struct ���Ƕ���
//���һ���������ͣ�������ǰ���豸��Ϣ��ͬʱ��һ��ȫ�ֵ��豸����
struct s5pv210_key{
	int dev_major; //���ϵ�ע���豸�ŵķ�ʽ
	struct class *cls;
	struct device *dev;
	int irqno; //��ʾ�豸���жϺ���
	int testdata;
	struct key_event event;//��Ű���������
	wait_queue_head_t wq_head;
	int have_data; //��ʾ��־λ����ʾ�Ƿ�������
};

//���һ�����������ǰ�������Ϣ
/*
1�� �жϺ�
	2�� gpio����
	3�� ����
	4�� ����������
*/
struct key_info{
	char *name;
	int irqno;
	int gpionum;
	int code;
	int flags;  //������ʽ
};
//�������а�������Ϣ
struct key_info  allkeys[] = {
	[0] = {
		.name = "key1_eint0",        // �ж�����
		.irqno = IRQ_EINT(0),         //  �жϺ�
		.gpionum = S5PV210_GPH0(0),     //gpio��
		.flags = IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,  //�����ش�����������ʽ��
		.code = KEY_UP,    //��ǰ�İ������������ң�
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




//���ö���
struct  s5pv210_key  *key_dev;



int key_drv_open(struct inode *inode, struct file *filp)
{
	//һ��Ҳ������ʼ������
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

	//�����������Ƿ�����
	if((filp->f_flags & O_NONBLOCK ) &&  !key_dev->have_data)
		return -EAGAIN;

	
	// û�����ݾ�����--have_data���0��ʾû������,����Ҫ��
	wait_event_interruptible(key_dev->wq_head, key_dev->have_data);

	ret = copy_to_user(buf, &key_dev->event, count);
	if(ret > 0)
	{
		printk("copy_to_user error\n");
		return -EFAULT;
	}

	// ��պ󣬽���ȥ������������
	memset(&key_dev->event,0, sizeof(struct key_event));
	key_dev->have_data = 0; // �������ó�û������

	
	return count;

}


unsigned int key_drv_poll(struct file *filp, struct poll_table_struct *pts)
{
	printk("-------^_^   %s-----------\n", __FUNCTION__);
	unsigned int  mask = 0;
	
	// ����ǰ�ĵȴ�����ע�ᵽvfs��
	poll_wait(filp, &key_dev->wq_head, pts);


	//��������ݵ�ʱ�򣬷���һ��POLLIN
	if(key_dev->have_data)
			mask |= POLLIN;

	return mask;        //�������Ƿ���POLLIN��û���ݷ���0;
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

	//���ֲ�ͬ���ж�
	struct key_info  *p = (struct key_info  *)dev_id;

	//�����ǰ��»���̧��
	int value;


	value = gpio_get_value(p->gpionum);
	
	if(value){
		//̧��
		printk("%s  up \n", p->name);
		key_dev->event.code = p->code;
		key_dev->event.value = 0;

	}else{
		//����	
		printk("%s  pressed \n", p->name);
		key_dev->event.code =  p->code;
		key_dev->event.value = 1;
	}

	wake_up_interruptible(&key_dev->wq_head);//�ỽ��
	key_dev->have_data = 1; //��ʾ������

	
	return IRQ_HANDLED;
}



static  int __init key_drv_init(void)
{
	int ret;
	//������Դ
	// ʵ�����ö���
	//  GFP_KERNEL�����ǰû���ڴ�ɷ��䣬�ú�����һֱ��
	key_dev = kzalloc(sizeof(struct s5pv210_key), GFP_KERNEL);
	if(key_dev == NULL)
	{
		printk(KERN_ERR "kzalloc error\n");
		return -ENOMEM;
	}
	// 1,  �����豸��
	key_dev->dev_major = register_chrdev(0, "s5pv210_key_drv", &key_fops);
	if(key_dev->dev_major < 0)
	{
		printk(KERN_ERR "kzalloc error\n");
		ret = -ENODEV;
		goto err_0;
	}

	
	// 2, �Զ������ļ�
	//  /sys/class/key_cls �ļ���
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


	// 4, Ӳ���ĳ�ʼ��---�ж�
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

               //    1.   �жϺ�  �жϴ�����  ������ʽ  �ж����� ���ݸ���������ֵ                        
		ret = request_irq(irqno, key_irq_handler, flags,name, &allkeys[i]);   //�ж�����
		if(ret != 0)
		{
			printk(KERN_ERR "request_irq error  : i=%d\n", i);
			ret = -EBUSY;
			goto err_4;
		}
		
	}
	
	//��ʼ���ȴ�����ͷ
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
	//�ͷ���Դ
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
