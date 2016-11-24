/**********************************************************************************
 * Ӳ������˵��
 
	 STM32      GPRSģ��
	 PA2  ->  GPRS RXD
	 PA3  ->  GPRS TXD
	 GND	   ->GND
	 
				OLEDģ��
	 PB6  ->  OLED SCL
	 PB7  ->  OLED SDA
	 
				Ѫ����
	 PA9  ->  Glucose Meter RXD
	 PA10 ->  Glucose Meter TXD
	 
	 PA1  ->  ��ص�ѹ������0-3.3V��
	  
**********************************************************************************/

#include "stm32f10x.h"
#include "usart.h"
#include "Led.h"
#include "SysTick.h"
#include "timer.h"
#include "string.h"
#include "OLED_I2C.h"
#include "delay.h"
#include "sys.h"
#include "stdio.h"
#include "adc.h"
#include "stm32f10x_adc.h"

#define Buf2_Max 200 					  //����2���泤��

/*************  ���ر�������	*********************************************/
char Uart2_Buf[Buf2_Max]; //����2���ջ���
u8 Times=0,First_Int = 0,shijian=0;
vu8 Timer0_start;	//��ʱ��0��ʱ����������

u8 content[200];//TCP��������

u8 display1[16];	
u8 display2[16];
u8 display3[16];	
u8 display4[16];

u16 adcx;
float temp;
	
u8 reclen=0;  
int i;

/*************	���غ�������	**************/
void CLR_Buf2(void);
u8 Find(char *a);
void Second_AT_Command(char *b,char *a,u8 wait_time);
void Wait_CREG(void);
void Init_TCP(void);
void Send_TCP(void);

/*************  �ⲿ�����ͱ�������*****************/
extern u8  USART1_RX_BUF[200]; //���ջ���
extern u16 USART1_RX_STA;      //����״̬���	

/*******************************************************************************
* ������ : main 
* ����   : ������
* ����   : 
* ���   : 
* ����   : 
* ע��   : 
*******************************************************************************/
int main(void)
{

	SysTick_Init_Config();
	GPIO_Config();	
	
	Adc_Init();		  					//ADC��ʼ��	    
	
	USART1_Init_Config(9600);
	USART2_Init_Config(115200);
	Timer2_Init_Config();
	
	I2C_Configuration();				//I2C��ʼ��
	OLED_Init();						//OLED��ʼ��
	OLED_Fill(0x00);					//OLEDȫ����
	
	Wait_CREG();   					    //��ѯ�ȴ�ģ��ע��ɹ�
	
	Second_AT_Command("AT+CSQ","+CSQ",3);
	sprintf((char*)display1,"RSSI:%d",(Uart2_Buf[5]-48)*10+Uart2_Buf[6]-48);
	OLED_ShowStr(0,0,display1,2);		//��ʾ�ź�ǿ��
	
	adcx=Get_Adc_Average(ADC_Channel_1,10);
	temp=(float)adcx*(3.3/4096)/0.21;
	adcx=temp;
	temp-=adcx;
	temp*=100;
	sprintf((char*)display3,"Voltage:%02d.%02d V",adcx,(int)temp);
	OLED_ShowStr(64,0,display1,2);		//��ʾ��ص�ѹ��0-3.3V��
	
	Init_TCP();
	
	while(1)
	{	
		if(USART1_RX_STA&0X8000)				//���յ�һ��������
		{
			reclen=USART1_RX_STA&0X7FFF;		//�õ����ݳ���
			USART1_RX_BUF[reclen]=0;	 		//���������
			USART1_RX_STA=0;
			for(i=0;i<reclen;i++)
			{
				content[i]=USART1_RX_BUF[i];		
			}
			OLED_ShowStr(0,2,USART1_RX_BUF,2);  //��ʾ��������
		}	
		
		Send_TCP();								//TCPЭ�鷢�����ݵ��������˿�
		
	}
}


void USART2_IRQHandler(void)                	
{
			u8 Res=0;
			Res =USART_ReceiveData(USART2);
			Uart2_Buf[First_Int] = Res;  	  //�����յ����ַ����浽������
			First_Int++;                			//����ָ������ƶ�
			if(First_Int > Buf2_Max)       		//���������,������ָ��ָ�򻺴���׵�ַ
			{
				First_Int = 0;
			}    
} 	


void TIM2_IRQHandler(void) 
{
	static u8 flag =1;

	if (TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET) 
	{
		
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update  );  
		
		if(Timer0_start)
		Times++;
		if(Times > shijian)
		{
			Timer0_start = 0;
			Times = 0;
		}
	
		if(flag)
		{
			flag=0;
		}
		else
		{
			flag=1;
		}
	}	
}

/*******************************************************************************
* ������ : CLR_Buf2
* ����   : �������2��������
* ����   : 
* ���   : 
* ����   : 
* ע��   : 
*******************************************************************************/
void CLR_Buf2(void)
{
	u16 k;
	for(k=0;k<Buf2_Max;k++)      //��������������
	{
		Uart2_Buf[k] = 0x00;
	}
    First_Int = 0;              //�����ַ�������ʼ�洢λ��
}

/*******************************************************************************
* ������ : Find
* ����   : �жϻ������Ƿ���ָ�����ַ���
* ����   : 
* ���   : 
* ����   : unsigned char:1 �ҵ�ָ���ַ���0 δ�ҵ�ָ���ַ� 
* ע��   : 
*******************************************************************************/

u8 Find(char *a)
{ 
  if(strstr(Uart2_Buf,a)!=NULL)
	    return 1;
	else
			return 0;
}

/*******************************************************************************
* ������ : Second_AT_Command
* ����   : ����ATָ���
* ����   : �������ݵ�ָ�롢ϣ�����յ���Ӧ�𡢷��͵ȴ�ʱ��(��λ��S)
* ���   : 
* ����   : 
* ע��   : 
*******************************************************************************/

void Second_AT_Command(char *b,char *a,u8 wait_time)         
{
	u8 i;
	char *c;
	c = b;										//�����ַ�����ַ��c
	CLR_Buf2(); 
  i = 0;
	while(i == 0)                    
	{
		if(!Find(a)) 
		{
			if(Timer0_start == 0)
			{
				b = c;							//���ַ�����ַ��b
				for (; *b!='\0';b++)
				{
					while(USART_GetFlagStatus(USART2, USART_FLAG_TC)==RESET);
					USART_SendData(USART2,*b);
				}
				UART2_SendLR();	
				Times = 0;
				shijian = wait_time;
				Timer0_start = 1;
		   }
    }
 	  else
		{
			i = 1;
			Timer0_start = 0;
		}
	}
	CLR_Buf2(); 
}

/*******************************************************************************
* ������ : Init_TCP
* ����   : ����TCP����
* ����   : 
* ���   : 
* ����   : 
* ע��   : 
*******************************************************************************/
void Init_TCP(void)
{
	Second_AT_Command("AT+CGATT=1","OK",3);								 //GPRS ����								
	Second_AT_Command("AT+CGACT=1,1","OK",3);						 	 //PDP�����ļ���
	Second_AT_Command("AT+CIPSTART=\"TCP\",\"112.74.55.52\",8000","OK",3);	
}

/*******************************************************************************
* ������ : Send_TCP
* ����   : ����TCP����
* ����   : 
* ���   : 
* ����   : 
* ע��   : 
*******************************************************************************/
void Send_TCP(void)
{

	Second_AT_Command("AT+CIPSEND",">",3); //�������ݳ��ȣ�25������ļ��㷽�������ڵ��ԱȽϣ����յ���>���ŷ��Ͷ�������
	UART2_SendString((char*)content);     //���Ͷ�������
	USART_SendData(USART2 ,0X1A);  //���ͽ�����
	UART2_SendLR();
}

/*******************************************************************************
* ������ : Wait_CREG
* ����   : �ȴ�ģ��ע��ɹ�
* ����   : 
* ���   : 
* ����   : 
* ע��   : 
*******************************************************************************/
void Wait_CREG(void)
{
	u8 i;
	u8 k;
	i = 0;
	CLR_Buf2();
  while(i == 0)        			
	{
		CLR_Buf2();        
		UART2_SendString("AT+CREG?");   //����ģ���Ƿ�ע��ɹ�
		UART2_SendLR();
		Delay_nMs(5000);  						
	    for(k=0;k<Buf2_Max;k++)      			
    	{
			if(Uart2_Buf[k] == ':')
			{
				if(((Uart2_Buf[k+4] == '1')&&(Uart2_Buf[k+5] != '3'))||(Uart2_Buf[k+4] == '5'))  //˵��ģ���Ѿ�ע��ɹ�
				{
					i = 1;
				  break;
				}
			}
		}
	}
}
