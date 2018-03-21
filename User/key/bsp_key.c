/**
  * @Title  	bsp_key.c
  * @author 	X.W.
  * @date		2018年3月1日
  * @version	V1.0
  * @Description: 实现按键输入
  */
#include "./key/bsp_key.h"
#include "./SysTick/bsp_SysTick.h"

/**
 * @brief  配置按键用到的I/O口
 * @param  无
 * @retval 无
 */
void Key_GPIO_Config(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	/*开启按键端口时钟*/
	RCC_APB2PeriphClockCmd(KEY0_GPIO_CLK|KEY1_GPIO_CLK|KEY2_GPIO_CLK|KEY3_GPIO_CLK, ENABLE);

	// 选择按键引脚
	GPIO_InitStructure.GPIO_Pin = KEY0_GPIO_PIN;
	// 设置按键引脚为上拉输入
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	// 使用结构体初始化按键
	GPIO_Init(KEY0_GPIO_PORT, &GPIO_InitStructure);

	// 选择按键引脚
	GPIO_InitStructure.GPIO_Pin = KEY1_GPIO_PIN;
	// 设置按键引脚为浮空输入
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	// 使用结构体初始化按键
	GPIO_Init(KEY1_GPIO_PORT, &GPIO_InitStructure);

	// 选择按键引脚
	GPIO_InitStructure.GPIO_Pin = KEY2_GPIO_PIN;
	// 设置按键引脚为浮空输入
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	// 使用结构体初始化按键
	GPIO_Init(KEY2_GPIO_PORT, &GPIO_InitStructure);

	// 选择按键引脚
	GPIO_InitStructure.GPIO_Pin = KEY3_GPIO_PIN;
	// 设置按键引脚为浮空输入
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
	// 使用结构体初始化按键
	GPIO_Init(KEY3_GPIO_PORT, &GPIO_InitStructure);
}

/**
 * 函数名：Key_Scan
 * 描述  ：检查是否有按键按下
 * 输入  ：uint8_t mode : 0-不支持连续按；1-支持连续按
 * 输出  ：KEY0_PRES - KEY0按下
 * 	  	   KEY1_PRES - KEY1按下
 * 	  	   KEY2_PRES - KEY2按下
 * 	  	   KEY3_PRES - KEY3按下
 * 注意  ：此函数有响应优先级，KEY0>KEY1>KEY2>KEY3!!!
 */
uint8_t Key_Scan(uint8_t mode)
{
	// 按键松开标志
	static uint8_t keyUp=1;
	// 如果mode=1，则支持连按
	if(mode) keyUp=1;
	// 检测是否有按键按下
	if(keyUp && (KEY0==0||KEY1==0||KEY2==0||KEY3==1)){
		Delay_ms(10);	//！< 去抖动
		keyUp = 0;
		if(     KEY0 == 0)	return KEY0_PRES;
		else if(KEY1 == 0)	return KEY1_PRES;
		else if(KEY2 == 0) return KEY2_PRES;
		else if(KEY3 == 1) return	KEY3_PRES;
	}
	else if(KEY0==1&&KEY1==1&&KEY2==1&&KEY3==0){
		keyUp = 1;
	}

	// 无按键按下
	return 0;
}

/**
 * END OF FILE
 */
