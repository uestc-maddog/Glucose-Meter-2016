/**********************************************************************************
 * 文件名  ：led.c
 * 描述    ：led 应用函数库         
 * 实验平台：NiRen_TwoHeart系统板
 * 硬件连接：  PB5 -> LED1     
 *             PB6 -> LED2     
 *             PB7 -> LED3    
 *             PB8 -> LED3    
 * 库版本  ：ST_v3.5
**********************************************************************************/

#include "Led.h"
	   
/*******************************************************************************
* 函数名  : GPIO_Config
* 描述    : LED 和PWR_MG323 IO配置
* 输入    : 无
* 输出    : 无
* 返回    : 无 
* 说明    : LED(1~4)的IO口分别是:PB5,PB6,PB7,PB8  PWR_MG323:PB9
*******************************************************************************/
void GPIO_Config(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;				//定义一个GPIO_InitTypeDef类型的GPIO初始化结构体
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB , ENABLE);			//使能GPIOB的外设时钟	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5| GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9;	//选择要初始化的GPIOB引脚(PB5,PB6,PB7,PB8,PB9)
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//设置引脚工作模式为通用推挽输出 		
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//设置引脚输出最大速率为50MHz
	GPIO_Init(GPIOB, &GPIO_InitStructure);			//调用库函数中的GPIO初始化函数，初始化GPIOB中的PB5,PB6,PB7,PB8,PB9引脚			 
}

