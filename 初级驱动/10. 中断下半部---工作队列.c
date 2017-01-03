

--------------------------------------------------
---------------------------------
##����һ<footnote>
----
- �Ĳ���
```
0.  strunt work_queue{            //���幤�����ж���
		struct workqueue_struct  *mywq;
		struct work_struct mywork;
	}
	strunt worK_queue *key_dev;
	 
1. ����һ���������У�Ӳ����ʼ��ʱ��
	key_dev->mywq = create_workqueue("my_workqueue");

2. ��ʼ��work��Ӳ����ʼ��ʱ��
	INIT_WORK(&key_dev->mywork,work_half_irq_handle);
	
3. �����°벿���жϴ�����ִ�У�
	queue_work(key_dev->mywq, &key_dev->mywork);

4. ִ���°벿
	void work_half_irq_handle(struct work_struct *work)
	{
		printk("-------^_^   %s-----------\n", __FUNCTION__);
	}	
```

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
	struct fasync_struct  *key_fasync; //ָ��һ��������֮���struct fasync_struct�ռ�
	struct workqueue_struct  *mywq;
	struct work_struct mywork;
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

	fasync_helper(-1, filp, 0, &key_dev->key_fasync);//�ͷ�key_dev->key_fasync

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

	return mask;
}

int key_drv_fasync (int fd, struct file *filp, int on)
{
	//����һ��fasync_helper,  ����struct fasync_struct�������źŵ���Ϣ(�ź��Ǹ�˭��)

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

void work_half_irq_handle(struct work_struct *work)
{
	printk("-------^_^   %s-----------\n", __FUNCTION__);
	wake_up_interruptible(&key_dev->wq_head);//�ỽ��
	key_dev->have_data = 1; //��ʾ������

	//�����ź�
	kill_fasync(&key_dev->key_fasync, SIGIO, POLLIN);
	
}



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

	//�����°벿
	queue_work(key_dev->mywq, &key_dev->mywork);
	
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
		ret = request_irq(irqno, key_irq_handler, flags,name, &allkeys[i]);
		if(ret != 0)
		{
			printk(KERN_ERR "request_irq error  : i=%d\n", i);
			ret = -EBUSY;
			goto err_4;
		}
		
	}
	
	//��ʼ���ȴ�����ͷ
	init_waitqueue_head(&key_dev->wq_head);


	// ����һ����������
	key_dev->mywq = create_workqueue("my_workqueue");

	// ��ʼ��work
	INIT_WORK(&key_dev->mywork,work_half_irq_handle);
	
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
	//����������ɾ��
	
	destroy_workqueue(key_dev->mywq);
	
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


---------------------------------------------------
-----------------
##������
-----

- ������
```
0.  strunt work_queue{            //���幤�����ж���
		struct workqueue_struct  *mywq;
		struct work_struct mywork;
	}
	strunt worK_queue *key_dev;
	 
1. ��ʼ��work��Ӳ����ʼ��ʱ��
	INIT_WORK(&key_dev->mywork,work_half_irq_handle);
	
2. �����°벿���жϴ�������ִ�У�
	schedule_work( &key_dev->mywork);

3. ִ���°벿
	void work_half_irq_handle(struct work_struct *work)
	{
		printk("-------^_^   %s-----------\n", __FUNCTION__);
	}	
```
----------------
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
	struct fasync_struct  *key_fasync; //ָ��һ��������֮���struct fasync_struct�ռ�
	struct work_struct mywork;
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

	fasync_helper(-1, filp, 0, &key_dev->key_fasync);//�ͷ�key_dev->key_fasync

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

	return mask;
}

int key_drv_fasync (int fd, struct file *filp, int on)
{
	//����һ��fasync_helper,  ����struct fasync_struct�������źŵ���Ϣ(�ź��Ǹ�˭��)

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

void work_half_irq_handle(struct work_struct *work)
{
	printk("-------^_^   %s-----------\n", __FUNCTION__);
	wake_up_interruptible(&key_dev->wq_head);//�ỽ��
	key_dev->have_data = 1; //��ʾ������

	//�����ź�
	kill_fasync(&key_dev->key_fasync, SIGIO, POLLIN);
	
}



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

	//�����°벿
	schedule_work( &key_dev->mywork);
	
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
		ret = request_irq(irqno, key_irq_handler, flags,name, &allkeys[i]);
		if(ret != 0)
		{
			printk(KERN_ERR "request_irq error  : i=%d\n", i);
			ret = -EBUSY;
			goto err_4;
		}
		
	}
	
	//��ʼ���ȴ�����ͷ
	init_waitqueue_head(&key_dev->wq_head);


	// ��ʼ��work
	INIT_WORK(&key_dev->mywork,work_half_irq_handle);
	
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
	//����������ɾ��	
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

