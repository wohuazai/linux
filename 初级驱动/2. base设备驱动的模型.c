法一
```
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/device.h>

#include <asm/uaccess.h>
#include <asm/io.h>

#define GPC0_PHY_BASE 0xE0200060
#define GPC0_SIZE 8

// 面向对象---将任何都看成对象， struct 就是对象
//设计一个对象类型，描述当前的设备信息，同时是一个全局的设备对象
struct led_desc{
	dev_t  devno;
	struct cdev *cdev;
	struct class *cls;
	struct device *dev;
};

//设置对象
struct  led_desc  *led_dev;


volatile unsigned long *gpc0conf; //表示虚拟地址
volatile unsigned long *gpc0data;


int led_drv_open(struct inode *inode, struct file *filp)
{
	//一般也是做初始化动作
	printk("-------^_^   %s-----------\n", __FUNCTION__);

	return 0;

}

// write(fd, buf, size);
ssize_t led_drv_write(struct file *filp, const char __user *buf, size_t count, loff_t *fpos)
{
	int val;
	int ret;
	
	ret = copy_from_user(&val, buf, count);
	if(ret > 0)
	{
		printk(KERN_ERR "copy_from_user error\n");
		return -EFAULT;
	}
```
```
	if(val) //点灯
	{
		*gpc0data |= (0x3<<3);
	}else{ //灭灯
		*gpc0data &= ~(0x3<<3);
	}
```
```
	return count;

}
int led_drv_close(struct inode *inode, struct file *filp)
{
	printk("-------^_^   %s-----------\n", __FUNCTION__);

	*gpc0data &= ~(0x3<<3);

	return 0;

}

const struct file_operations  led_fops = {
	.open = led_drv_open,
	.write = led_drv_write,
	.release = led_drv_close,
};


static  int __init led_drv_init(void)
{
	int ret;
	//申请资源
	// 实例化该对象
	//  GFP_KERNEL如果当前没有内存可分配，该函数会一直等
	led_dev = kzalloc(sizeof(struct led_desc), GFP_KERNEL);
	if(led_dev == NULL)
	{
		printk(KERN_ERR "kzalloc error\n");
		return -ENOMEM;
	}
	// 1,  申请设备号
	// 静态申请	register_chrdev_region(dev_t from,unsigned count,const char * name)
	//动态申请
	ret = alloc_chrdev_region(&led_dev->devno, 0, 1, "led_drv"); //  /proc/devices
	if(ret != 0)
	{
		printk(KERN_ERR "alloc_chrdev_region error\n");
		goto err_0;
	}
	printk("major number is %d\n", MAJOR(led_dev->devno));
	
	//  创建cdev
	led_dev->cdev = cdev_alloc();
	cdev_init(led_dev->cdev, &led_fops);
	cdev_add(led_dev->cdev, led_dev->devno, 1);

	// 2, 自动创建文件
	//  /sys/class/led_cls 文件夹
	led_dev->cls = class_create(THIS_MODULE, "led_cls");
	if(IS_ERR(led_dev->cls))
	{
		printk(KERN_ERR "class_create error\n");
		ret = PTR_ERR(led_dev->cls);
		goto err_1;
	}

	//  /sys/class/led_cls/led0/
	//  /dev/led0
	led_dev->dev = device_create(led_dev->cls, NULL, led_dev->devno,NULL, "led%d", 0);
	if(IS_ERR(led_dev->dev))
	{
		printk(KERN_ERR "device_create error\n");
		ret = PTR_ERR(led_dev->dev);
		goto err_2;
	}

```
```
	// 4, 硬件的初始化---地址映射
	//先要将物理地址装换成虚拟地址
	gpc0conf = ioremap(GPC0_PHY_BASE, GPC0_SIZE);
	gpc0data = gpc0conf + 1;

	//先初始化输出功能
	*gpc0conf &= ~(0xff<<12);
	*gpc0conf |= (0x11<<12);

```
```
	return 0;

err_2:
	class_destroy(led_dev->cls);

err_1:
	cdev_del(led_dev->cdev);
	unregister_chrdev_region(led_dev->devno,1);
	
err_0:
	kfree(led_dev);
	return ret;

}


static void __exit  led_drv_exit(void)
{
	//释放资源
	iounmap(gpc0conf);
	device_destroy(led_dev->cls,led_dev->devno);
	class_destroy(led_dev->cls);
	cdev_del(led_dev->cdev);
	unregister_chrdev_region(led_dev->devno,1);
	kfree(led_dev);
}



module_init(led_drv_init);
module_exit(led_drv_exit);
MODULE_LICENSE("GPL");
```
-------------------------------

法二：
```
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/cdev.h>
#include <linux/device.h>

#include <asm/uaccess.h>
#include <asm/io.h>

#define GPC0_PHY_BASE 0xE0200060
#define GPC0_SIZE 8

// 面向对象---将任何都看成对象， struct 就是对象
//设计一个对象类型，描述当前的设备信息，同时是一个全局的设备对象
struct led_desc{
	dev_t  devno;
	struct cdev *cdev;
	struct class *cls;
	struct device *dev;
	void *reg_base; // 表示寄存器的基准地址
};

//设置对象
struct  led_desc  *led_dev;



int led_drv_open(struct inode *inode, struct file *filp)
{
	//一般也是做初始化动作
	printk("-------^_^   %s-----------\n", __FUNCTION__);

	// inode的作用就是用于去区分不同次设备
	int major = imajor(inode);
	int minor = iminor(inode);

	printk("led_drv_open : major = %d, minor = %d\n", major, minor);

	static int a = 999;
	filp->private_data = &a;
	

	return 0;

}

// write(fd, buf, size);
ssize_t led_drv_write(struct file *filp, const char __user *buf, size_t count, loff_t *fpos)
{

	// 通过filp获取到inode
	struct inode *inode = filp->f_path.dentry->d_inode;
	int major = imajor(inode);
	int minor = iminor(inode);

	int a_value = *((int *)filp->private_data);

	printk("led_drv_write : major = %d, minor = %d, a = %d\n", major, minor, a_value);

	
	
	int val;
	int ret;

	
	ret = copy_from_user(&val, buf, count);
	if(ret > 0)
	{
		printk(KERN_ERR "copy_from_user error\n");
		return -EFAULT;
	}
```
```
	if(val) //点灯
	{
		// led_dev->reg_base + 4表示的是数据寄存器
		
		__raw_writel(__raw_readl(led_dev->reg_base + 4)|(0x3<<3),   led_dev->reg_base + 4);
		
	}else{ //灭灯
		__raw_writel(__raw_readl(led_dev->reg_base + 4)& ~(0x3<<3),   led_dev->reg_base + 4);
	}
```
```

	return count;

}
int led_drv_close(struct inode *inode, struct file *filp)
{
	printk("-------^_^   %s-----------\n", __FUNCTION__);

	__raw_writel(__raw_readl(led_dev->reg_base + 4)& ~(0x3<<3),   led_dev->reg_base + 4);

	return 0;

}

const struct file_operations  led_fops = {
	.open = led_drv_open,
	.write = led_drv_write,
	.release = led_drv_close,
};


static  int __init led_drv_init(void)
{
	int ret;
	//申请资源
	// 实例化该对象
	//  GFP_KERNEL如果当前没有内存可分配，该函数会一直等
	led_dev = kzalloc(sizeof(struct led_desc), GFP_KERNEL);
	if(led_dev == NULL)
	{
		printk(KERN_ERR "kzalloc error\n");
		return -ENOMEM;
	}
	// 1,  申请设备号
	// 静态申请	register_chrdev_region(dev_t from,unsigned count,const char * name)
	//动态申请
	ret = alloc_chrdev_region(&led_dev->devno, 0, 1, "led_drv"); //  /proc/devices
	if(ret != 0)
	{
		printk(KERN_ERR "alloc_chrdev_region error\n");
		goto err_0;
	}
	printk("major number is %d\n", MAJOR(led_dev->devno));
	
	//  创建cdev
	led_dev->cdev = cdev_alloc();
	cdev_init(led_dev->cdev, &led_fops);
	cdev_add(led_dev->cdev, led_dev->devno, 1);

	// 2, 自动创建文件
	//  /sys/class/led_cls 文件夹
	led_dev->cls = class_create(THIS_MODULE, "led_cls");
	if(IS_ERR(led_dev->cls))
	{
		printk(KERN_ERR "class_create error\n");
		ret = PTR_ERR(led_dev->cls);
		goto err_1;
	}

	//  /sys/class/led_cls/led0/
	//  /dev/led0
	led_dev->dev = device_create(led_dev->cls, NULL, led_dev->devno,NULL, "led%d", 0);
	if(IS_ERR(led_dev->dev))
	{
		printk(KERN_ERR "device_create error\n");
		ret = PTR_ERR(led_dev->dev);
		goto err_2;
	}

```
```
	// 4, 硬件的初始化---地址映射
	//先要将物理地址装换成虚拟地址
	led_dev->reg_base = ioremap(GPC0_PHY_BASE, GPC0_SIZE);

	//将gpio设置成输出功能
	unsigned long temp = readl(led_dev->reg_base);
	temp &= ~(0xff<<12);
	temp |= (0x11<<12);
	writel(temp, led_dev->reg_base);
	
```
```
	return 0;

err_2:
	class_destroy(led_dev->cls);

err_1:
	cdev_del(led_dev->cdev);
	unregister_chrdev_region(led_dev->devno,1);
	
err_0:
	kfree(led_dev);
	return ret;

}


static void __exit  led_drv_exit(void)
{
	//释放资源
	iounmap(led_dev->reg_base);
	device_destroy(led_dev->cls,led_dev->devno);
	class_destroy(led_dev->cls);
	cdev_del(led_dev->cdev);
	unregister_chrdev_region(led_dev->devno,1);
	kfree(led_dev);
}



module_init(led_drv_init);
module_exit(led_drv_exit);
MODULE_LICENSE("GPL");
```
