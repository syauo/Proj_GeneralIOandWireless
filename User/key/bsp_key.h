/**
  * @Title  	bsp_key.h
  * @author 	X.W.
  * @date		2018年3月1日
  * @version	V1.0
  * @Description: 定义按键头文件
  */
#ifndef KEY_BSP_KEY_H_
#define KEY_BSP_KEY_H_

#include "stm32f10x.h"

// 按键引脚定义
// 启明MINI，KEY0-PF2; KEY1-PF3; KEY2-PF4; KEY3-PF5(接高)
// KEY0
#define KEY0_GPIO_CLK		RCC_APB2Periph_GPIOF
#define KEY0_GPIO_PORT		GPIOF
#define KEY0_GPIO_PIN 		GPIO_Pin_2
// KEY1
#define KEY1_GPIO_CLK		RCC_APB2Periph_GPIOF
#define KEY1_GPIO_PORT 	GPIOF
#define KEY1_GPIO_PIN		GPIO_Pin_3
// KEY2
#define KEY2_GPIO_CLK		RCC_APB2Periph_GPIOF
#define KEY2_GPIO_PORT		GPIOF
#define KEY2_GPIO_PIN 		GPIO_Pin_4
// KEY3
#define KEY3_GPIO_CLK		RCC_APB2Periph_GPIOF
#define KEY3_GPIO_PORT		GPIOF
#define KEY3_GPIO_PIN 		GPIO_Pin_5


/**
 * 按键按下标志宏
 */
#define KEY0_PRES	0x01
#define KEY1_PRES 	0x02
#define KEY2_PRES 	0x04
#define KEY3_PRES	0x08

/**
 * 读取按键宏
 */
#define KEY0  	GPIO_ReadInputDataBit(KEY0_GPIO_PORT,KEY0_GPIO_PIN)
#define KEY1  	GPIO_ReadInputDataBit(KEY1_GPIO_PORT,KEY1_GPIO_PIN)
#define KEY2   GPIO_ReadInputDataBit(KEY2_GPIO_PORT,KEY2_GPIO_PIN)
#define KEY3   GPIO_ReadInputDataBit(KEY3_GPIO_PORT,KEY3_GPIO_PIN)

/**
 * 函数声明
 */
void Key_GPIO_Config(void);
uint8_t Key_Scan(uint8_t mode);

#endif /* KEY_BSP_KEY_H_ */
