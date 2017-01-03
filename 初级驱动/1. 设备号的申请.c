```
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

```
----------------
### ����һ����̬�����豸�ţ���������256����

```
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

	// �������ڵ㣩 /sys/class/key_cls/key/
	//  /dev/key1
	key_dev->dev = device_create(key_dev->cls, NULL, MKDEV(key_dev->dev_major,1),NULL, "key1");
	if(IS_ERR(key_dev->dev))
	{
		printk(KERN_ERR "device_create error\n");
		ret = PTR_ERR(key_dev->dev);
		goto err_2;
	}


	// 4, Ӳ���ĳ�ʼ��


err_2:
	class_destroy(key_dev->cls);

err_1:
	unregister_chrdev(key_dev->dev_major, "s5pv210_key_drv");
	
err_0:
	kfree(key_dev);
	return ret;

}
```


### ����������̬�����豸�ţ���������4096����

```
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


	// 4, Ӳ���ĳ�ʼ��



err_2:
	class_destroy(led_dev->cls);

err_1:
	cdev_del(led_dev->cdev);
	unregister_chrdev_region(led_dev->devno,1);
	
err_0:
	kfree(led_dev);
	return ret;

}

```

- **�˳���ע��**

```
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

```
