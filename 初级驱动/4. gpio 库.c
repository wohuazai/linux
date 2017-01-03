- gpio库函数的使用----非常方便的gpio操作方式
	- 内核提供了一组函数，可以直接去操作gpio口，并不需要去对地址操作

```
操作gpio的时候就是操作gpio的号码：S5PV210_GPC0(3)
	
1. gpio_request(unsigned gpio,const char * label)//申请gpio的使用权

2. gpio_free(unsigned gpio) //释放gpio	

3. gpio_direction_input(unsigned gpio) //将某个gpio口设置成输入功能
   或 (s3c_gpio_cfgpin(unsigned gpio,  S3C_GPIO_INPUT);)

4. gpio_direction_output(unsigned gpio,int value) //将某个gpio口设 
   置成输出功能， 并且直接输出高低电平
   或 (s3c_gpio_cfgpin(unsigned gpio,  S3C_GPIO_OUTPUT);)
   或 (gpio_set_value(unsigned gpio， int value);)
									
5. gpio_get_value(unsigned gpio); // 获取到gpio口电平
		
6. gpio_set_value(unsigned gpio， int value); //设置gpio口电平


7. int gpio_to_irq(unsigned gpio); //根据gpio号码转换成中断的号码

8. int irq_to_gpio(unsigned irq);
  
9. s3c_gpio_cfgpin(unsigned int pin,unsigned int config) //设置  
   gpio口的其他功能，nand, uart, i2c功能
							|
10. s3c_gpio_cfgpin(S5PV210_MP01(1), S3C_GPIO_SFN(2));


```
