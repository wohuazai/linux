- gpio�⺯����ʹ��----�ǳ������gpio������ʽ
	- �ں��ṩ��һ�麯��������ֱ��ȥ����gpio�ڣ�������Ҫȥ�Ե�ַ����

```
����gpio��ʱ����ǲ���gpio�ĺ��룺S5PV210_GPC0(3)
	
1. gpio_request(unsigned gpio,const char * label)//����gpio��ʹ��Ȩ

2. gpio_free(unsigned gpio) //�ͷ�gpio	

3. gpio_direction_input(unsigned gpio) //��ĳ��gpio�����ó����빦��
   �� (s3c_gpio_cfgpin(unsigned gpio,  S3C_GPIO_INPUT);)

4. gpio_direction_output(unsigned gpio,int value) //��ĳ��gpio���� 
   �ó�������ܣ� ����ֱ������ߵ͵�ƽ
   �� (s3c_gpio_cfgpin(unsigned gpio,  S3C_GPIO_OUTPUT);)
   �� (gpio_set_value(unsigned gpio�� int value);)
									
5. gpio_get_value(unsigned gpio); // ��ȡ��gpio�ڵ�ƽ
		
6. gpio_set_value(unsigned gpio�� int value); //����gpio�ڵ�ƽ


7. int gpio_to_irq(unsigned gpio); //����gpio����ת�����жϵĺ���

8. int irq_to_gpio(unsigned irq);
  
9. s3c_gpio_cfgpin(unsigned int pin,unsigned int config) //����  
   gpio�ڵ��������ܣ�nand, uart, i2c����
							|
10. s3c_gpio_cfgpin(S5PV210_MP01(1), S3C_GPIO_SFN(2));


```
