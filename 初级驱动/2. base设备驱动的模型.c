��һ
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

// �������---���κζ����ɶ��� struct ���Ƕ���
//���һ���������ͣ�������ǰ���豸��Ϣ��ͬʱ��һ��ȫ�ֵ��豸����
struct led_desc{
	dev_t  devno;
	struct cdev *cdev;
	struct class *cls;
	struct device *dev;
};

//���ö���
struct  led_desc  *led_dev;


volatile unsigned long *gpc0conf; //��ʾ�����ַ
volatile unsigned long *gpc0data;


int led_drv_open(struct inode *inode, struct file *filp)
{
	//һ��Ҳ������ʼ������
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
	if(val) //���
	{
		*gpc0data |= (0x3<<3);
	}else{ //���
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
	//������Դ
	// ʵ�����ö���
	//  GFP_KERNEL�����ǰû���ڴ�ɷ��䣬�ú�����һֱ��
	led_dev = kzalloc(sizeof(struct led_desc), GFP_KERNEL);
	if(led_dev == NULL)
	{
		printk(KERN_ERR "kzalloc error\n");
		return -ENOMEM;
	}
	// 1,  �����豸��
	// ��̬����	register_chrdev_region(dev_t from,unsigned count,const char * name)
	//��̬����
	ret = alloc_chrdev_region(&led_dev->devno, 0, 1, "led_drv"); //  /proc/devices
	if(ret != 0)
	{
		printk(KERN_ERR "alloc_chrdev_region error\n");
		goto err_0;
	}
	printk("major number is %d\n", MAJOR(led_dev->devno));
	
	//  ����cdev
	led_dev->cdev = cdev_alloc();
	cdev_init(led_dev->cdev, &led_fops);
	cdev_add(led_dev->cdev, led_dev->devno, 1);

	// 2, �Զ������ļ�
	//  /sys/class/led_cls �ļ���
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
	// 4, Ӳ���ĳ�ʼ��---��ַӳ��
	//��Ҫ�������ַװ���������ַ
	gpc0conf = ioremap(GPC0_PHY_BASE, GPC0_SIZE);
	gpc0data = gpc0conf + 1;

	//�ȳ�ʼ���������
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
	//�ͷ���Դ
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

������
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

// �������---���κζ����ɶ��� struct ���Ƕ���
//���һ���������ͣ�������ǰ���豸��Ϣ��ͬʱ��һ��ȫ�ֵ��豸����
struct led_desc{
	dev_t  devno;
	struct cdev *cdev;
	struct class *cls;
	struct device *dev;
	void *reg_base; // ��ʾ�Ĵ����Ļ�׼��ַ
};

//���ö���
struct  led_desc  *led_dev;



int led_drv_open(struct inode *inode, struct file *filp)
{
	//һ��Ҳ������ʼ������
	printk("-------^_^   %s-----------\n", __FUNCTION__);

	// inode�����þ�������ȥ���ֲ�ͬ���豸
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

	// ͨ��filp��ȡ��inode
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
	if(val) //���
	{
		// led_dev->reg_base + 4��ʾ�������ݼĴ���
		
		__raw_writel(__raw_readl(led_dev->reg_base + 4)|(0x3<<3),   led_dev->reg_base + 4);
		
	}else{ //���
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
	//������Դ
	// ʵ�����ö���
	//  GFP_KERNEL�����ǰû���ڴ�ɷ��䣬�ú�����һֱ��
	led_dev = kzalloc(sizeof(struct led_desc), GFP_KERNEL);
	if(led_dev == NULL)
	{
		printk(KERN_ERR "kzalloc error\n");
		return -ENOMEM;
	}
	// 1,  �����豸��
	// ��̬����	register_chrdev_region(dev_t from,unsigned count,const char * name)
	//��̬����
	ret = alloc_chrdev_region(&led_dev->devno, 0, 1, "led_drv"); //  /proc/devices
	if(ret != 0)
	{
		printk(KERN_ERR "alloc_chrdev_region error\n");
		goto err_0;
	}
	printk("major number is %d\n", MAJOR(led_dev->devno));
	
	//  ����cdev
	led_dev->cdev = cdev_alloc();
	cdev_init(led_dev->cdev, &led_fops);
	cdev_add(led_dev->cdev, led_dev->devno, 1);

	// 2, �Զ������ļ�
	//  /sys/class/led_cls �ļ���
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
	// 4, Ӳ���ĳ�ʼ��---��ַӳ��
	//��Ҫ�������ַװ���������ַ
	led_dev->reg_base = ioremap(GPC0_PHY_BASE, GPC0_SIZE);

	//��gpio���ó��������
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
	//�ͷ���Դ
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
