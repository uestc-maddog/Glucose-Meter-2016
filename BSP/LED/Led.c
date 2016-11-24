/**********************************************************************************
 * �ļ���  ��led.c
 * ����    ��led Ӧ�ú�����         
 * ʵ��ƽ̨��NiRen_TwoHeartϵͳ��
 * Ӳ�����ӣ�  PB5 -> LED1     
 *             PB6 -> LED2     
 *             PB7 -> LED3    
 *             PB8 -> LED3    
 * ��汾  ��ST_v3.5
**********************************************************************************/

#include "Led.h"
	   
/*******************************************************************************
* ������  : GPIO_Config
* ����    : LED ��PWR_MG323 IO����
* ����    : ��
* ���    : ��
* ����    : �� 
* ˵��    : LED(1~4)��IO�ڷֱ���:PB5,PB6,PB7,PB8  PWR_MG323:PB9
*******************************************************************************/
void GPIO_Config(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;				//����һ��GPIO_InitTypeDef���͵�GPIO��ʼ���ṹ��
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB , ENABLE);			//ʹ��GPIOB������ʱ��	
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5| GPIO_Pin_6|GPIO_Pin_7|GPIO_Pin_8|GPIO_Pin_9;	//ѡ��Ҫ��ʼ����GPIOB����(PB5,PB6,PB7,PB8,PB9)
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	//�������Ź���ģʽΪͨ��������� 		
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;	//������������������Ϊ50MHz
	GPIO_Init(GPIOB, &GPIO_InitStructure);			//���ÿ⺯���е�GPIO��ʼ����������ʼ��GPIOB�е�PB5,PB6,PB7,PB8,PB9����			 
}

