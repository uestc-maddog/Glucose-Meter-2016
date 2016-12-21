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
#include "meter.h"
#include "delay.h"

int j,con;
int data0[5],data1[5],data2[5],data3[5],data4[5],data5[5];
long int time[6];
int value[6];

u8 content[100];

u8 display3[16];	
u8 display4[16];

/*************  血糖仪数据指令声明	*********************************************/
unsigned char init_str[]={0x02,0x12,0x00,0x05,0x0B,0x02,0x00,0x00,0x00,0x00,0x84,0x6A,0xE8,0x73,0x00,0x03,0x9B,0xEA };
unsigned char getnumber_str[]={0x02,0x0A,0x00,0x05,0x1F,0xF5,0x01,0x03,0x38,0xAA };
unsigned char get0_str[]={0x02,0x0A,0x03,0x05,0x1F,0x00,0x00,0x03,0x4B,0x5F};
unsigned char get1_str[]={0x02,0x0A,0x03,0x05,0x1F,0x01,0x00,0x03,0x7B,0x68};
unsigned char get2_str[]={0x02,0x0A,0x03,0x05,0x1F,0x02,0x00,0x03,0x2B,0x31};
unsigned char get3_str[]={0x02,0x0A,0x03,0x05,0x1F,0x03,0x00,0x03,0x1B,0x06};
unsigned char get4_str[]={0x02,0x0A,0x03,0x05,0x1F,0x04,0x00,0x03,0x8B,0x83};
unsigned char get5_str[]={0x02,0x0A,0x03,0x05,0x1F,0x05,0x00,0x03,0xBB,0xB4};

/*************  外部函数和变量声明*****************/
extern u8  USART1_RX_BUF[200]; //接收缓冲
extern u16 USART1_RX_STA;      //接收状态标记	
extern u8 reclen; 

void getdata()
{
	while(1)
	{
		UART1_SendString(init_str);			//初始化血糖仪连接
		if(USART1_RX_STA&0X8000)				
			break;
		delay_ms(100);
	}
	
	while(1)
	{
		UART1_SendString(getnumber_str);	//获取血糖仪数据条数
		if(USART1_RX_STA&0X8000)				
		{
			reclen=USART1_RX_STA&0X7FFF;	
			USART1_RX_BUF[reclen]=0;	 		
			USART1_RX_STA=0;
			for(j=0;j<reclen;j++)
			{
				con=USART1_RX_BUF[j];
				if(con==15)
				{
					con=USART1_RX_BUF[j+1];
					sprintf((char*)display3,"Data Num:   %03d",con);
					OLED_ShowStr(0,3,display3,1);
					break;
				}
			}
			break;
			
		}	
		delay_ms(100);
	}
	
	while(1)
	{
		UART1_SendString(get5_str);			//获取血糖仪5号数据
		if(USART1_RX_STA&0X8000)				
		{
			reclen=USART1_RX_STA&0X7FFF;	
			USART1_RX_BUF[reclen]=0;	 		
			USART1_RX_STA=0;
			for(j=0;j<reclen;j++)
			{
				con=USART1_RX_BUF[j];
				if(con==180)
				{
					data0[0]=USART1_RX_BUF[j+1];	//5号数据赋给0号数组
					data0[1]=USART1_RX_BUF[j+2];
					data0[2]=USART1_RX_BUF[j+3];
					data0[3]=USART1_RX_BUF[j+4];
					data0[4]=USART1_RX_BUF[j+5];
					sprintf((char*)display3,"Reading:     1/6");
					OLED_ShowStr(0,4,display3,1);
					break;
				}
			}
			break;
		}
		delay_ms(100);		
	}
	
	while(1)
	{
		UART1_SendString(get4_str);			//获取血糖仪4号数据
		if(USART1_RX_STA&0X8000)				
		{
			reclen=USART1_RX_STA&0X7FFF;	
			USART1_RX_BUF[reclen]=0;	 		
			USART1_RX_STA=0;
			for(j=0;j<reclen;j++)
			{
				con=USART1_RX_BUF[j];
				if(con==131)
				{
					data1[0]=USART1_RX_BUF[j+1];
					data1[1]=USART1_RX_BUF[j+2];
					data1[2]=USART1_RX_BUF[j+3];
					data1[3]=USART1_RX_BUF[j+4];
					data1[4]=USART1_RX_BUF[j+5];
					sprintf((char*)display3,"Reading:     2/6");
					OLED_ShowStr(0,4,display3,1);
					break;
				}
			}
			break;
		}
		delay_ms(100);		
	}
	
	while(1)
	{	
		UART1_SendString(get3_str);			//获取血糖仪3号数据
		if(USART1_RX_STA&0X8000)				
		{
			reclen=USART1_RX_STA&0X7FFF;	
			USART1_RX_BUF[reclen]=0;	 		
			USART1_RX_STA=0;
			for(j=0;j<reclen;j++)
			{
				con=USART1_RX_BUF[j];
				if(con==6)
				{
					data2[0]=USART1_RX_BUF[j+1];
					data2[1]=USART1_RX_BUF[j+2];
					data2[2]=USART1_RX_BUF[j+3];
					data2[3]=USART1_RX_BUF[j+4];
					data2[4]=USART1_RX_BUF[j+5];
					sprintf((char*)display3,"Reading:     3/6");
					OLED_ShowStr(0,4,display3,1);
					break;
				}
			}
			break;
		}
		delay_ms(100);
	}
	
	while(1)
	{
		UART1_SendString(get2_str);			//获取血糖仪2号数据
		if(USART1_RX_STA&0X8000)				
		{
			reclen=USART1_RX_STA&0X7FFF;	
			USART1_RX_BUF[reclen]=0;	 		
			USART1_RX_STA=0;
			for(j=0;j<reclen;j++)
			{
				con=USART1_RX_BUF[j];
				if(con==49)
				{
					data3[0]=USART1_RX_BUF[j+1];
					data3[1]=USART1_RX_BUF[j+2];
					data3[2]=USART1_RX_BUF[j+3];
					data3[3]=USART1_RX_BUF[j+4];
					data3[4]=USART1_RX_BUF[j+5];
					sprintf((char*)display3,"Reading:     4/6");
					OLED_ShowStr(0,4,display3,1);
					break;
				}
			}
			break;
		}
		delay_ms(100);
	}
	
	while(1)
	{
		UART1_SendString(get1_str);			//获取血糖仪1号数据
		if(USART1_RX_STA&0X8000)				
		{
			reclen=USART1_RX_STA&0X7FFF;	
			USART1_RX_BUF[reclen]=0;	 		
			USART1_RX_STA=0;
			for(j=0;j<reclen;j++)
			{
				con=USART1_RX_BUF[j];
				if(con==104)
				{
					data4[0]=USART1_RX_BUF[j+1];
					data4[1]=USART1_RX_BUF[j+2];
					data4[2]=USART1_RX_BUF[j+3];
					data4[3]=USART1_RX_BUF[j+4];
					data4[4]=USART1_RX_BUF[j+5];
					sprintf((char*)display3,"Reading:     5/6");
					OLED_ShowStr(0,4,display3,1);
					break;
				}
			}
			break;
		}
		delay_ms(100);
	}
	
	while(1)
	{
		UART1_SendString(get0_str);			//获取血糖仪0号数据
		if(USART1_RX_STA&0X8000)				
		{
			reclen=USART1_RX_STA&0X7FFF;	
			USART1_RX_BUF[reclen]=0;	 		
			USART1_RX_STA=0;
			for(j=0;j<reclen;j++)
			{
				con=USART1_RX_BUF[j];
				if(con==95)
				{
					data5[0]=USART1_RX_BUF[j+1];
					data5[1]=USART1_RX_BUF[j+2];
					data5[2]=USART1_RX_BUF[j+3];
					data5[3]=USART1_RX_BUF[j+4];
					data5[4]=USART1_RX_BUF[j+5];
					sprintf((char*)display3,"Reading:     6/6");
					OLED_ShowStr(0,4,display3,1);
					break;
				}
			}
			break;
		}
		delay_ms(100);
	}
}

void transform()
{
	time[0]=data0[0]+data0[1]*256+data0[2]*256*256+data0[3]*256*256*256;
	time[1]=data1[0]+data1[1]*256+data1[2]*256*256+data1[3]*256*256*256;
	time[2]=data2[0]+data2[1]*256+data2[2]*256*256+data2[3]*256*256*256;
	time[3]=data3[0]+data3[1]*256+data3[2]*256*256+data3[3]*256*256*256;
	time[4]=data4[0]+data4[1]*256+data4[2]*256*256+data4[3]*256*256*256;
	time[5]=data5[0]+data5[1]*256+data5[2]*256*256+data5[3]*256*256*256;
	
	value[0]=data0[4];
	value[1]=data1[4];
	value[2]=data2[4];
	value[3]=data3[4];
	value[4]=data4[4];
	value[5]=data5[4];
	
	for(j=0;j<6;j++)
	{
		sprintf((char*)content,"M1003/15528093697/%d/%ld",value[j],time[j]);
		sprintf((char*)display3,"Transform:   %d/6",j+1);
		OLED_ShowStr(0,5,display3,1);
	}
}


