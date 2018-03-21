#include "stm32f10x.h"
#include "./SysTick/bsp_SysTick.h"
#include "./usart/bsp_usart.h"
#include "./led/bsp_led.h"
#include "./key/bsp_key.h"
#include "./NRF24L01/24l01.h"
/*
 * 函数名：main
 * 描述  ：主函数
 * 输入  ：无
 * 输出  ：无
 */
int main(void)
{
	uint8_t key, mode;
	uint16_t t = 0;
	uint8_t tmp_buf[33];
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // 设置中断优先级分组2
	/* 配置串口为：115200 8-N-1 */
	USART_Config();
	/* 初始化与LED连接的硬件接口 */
	LED_GPIO_Config();
	/* 按键初始化 */										
	Key_GPIO_Config();
	/* 无线模块初始化 */
	NRF24L01_Config();

	/* 检查无线模块是否在位 */
	printf("\r\n开始检查无线模块是否在位...\r\n");
	while (NRF24L01_Check()) 
	{
		printf("\rNRF24L01 Error");
		Delay_ms(200);
		printf("\r               ");
		Delay_ms(200);
	}
	printf("\rNRF24L01 Check - OK");

	/* 依据按键确定进入哪个模式!*/
	while (1) 
	{
		key = Key_Scan(0);
		if (key == KEY0_PRES)
		{
			mode = 0;	// RX模式
			break;
		}
		else if (key == KEY1_PRES)
		{
			mode = 1;	// TX模式
			break;
		}
		/* 闪烁提示按键信息 */
		t++;
		if (t == 100)
			printf("\r\n> KEY0:RX_Mode  KEY1:TX_Mode"); 
		if (t == 200)
		{
			printf("\r                               ");
			t = 0;
		}
		Delay_ms(5);
	}
	printf("\r                               "); //清空按键提示
	
	/* 若为RX模式 */
	if (mode == 0)
	{
		printf("\rNRF24L01 RX_Mode\r\n");
		printf("Received DATA:\r\n");
		NRF24L01_RX_Mode();
		while (1)
		{
			/* 一旦接收到信息,则显示出来. */
			if (NRF24L01_RxPacket(tmp_buf) == 0) 
			{
				tmp_buf[32] = 0; //加入字符串结束符
				printf("%s", tmp_buf);
			}
			else
				delay_us(100);
			t++;
			if (t == 10000) //大约1s钟改变一次LED0状态
			{
				t = 0;
				LED0_TOGGLE;
			}
		}
	}
	/* 反之，若为TX模式 */
	else 
	{
		printf("\rNRF24L01 TX_Mode\r\n");
		NRF24L01_TX_Mode();
		//所发送的字符串从空格键开始
		mode = ' '; 
		while (1)
		{
			if (NRF24L01_TxPacket(tmp_buf) == TX_OK)
			{
				printf("Sended DATA:");
				printf("%s", tmp_buf);
				key = mode;
				for (t = 0; t < 32; t++)
				{
					key++;
					if (key > ('~'))
						key = ' ';
					tmp_buf[t] = key;
				}
				mode++;
				if (mode > '~')
					mode = ' ';
				tmp_buf[32] = 0; //加入结束符
			}
			else
			{
				printf("Send Failed ");
			}
			LED0_TOGGLE;
			Delay_ms(1500);
		}
	}
}