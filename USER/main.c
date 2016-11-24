/**********************************************************************************
 * 硬件连接说明
 
	 STM32      GPRS模块
	 PA2  ->  GPRS RXD
	 PA3  ->  GPRS TXD
	 GND	   ->GND
	 
				OLED模块
	 PB6  ->  OLED SCL
	 PB7  ->  OLED SDA
	 
				血糖仪
	 PA9  ->  Glucose Meter RXD
	 PA10 ->  Glucose Meter TXD
	 
	 PA1  ->  电池电压采样（0-3.3V）
	  
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

#define Buf2_Max 200 					  //串口2缓存长度

/*************  本地变量声明	*********************************************/
char Uart2_Buf[Buf2_Max]; //串口2接收缓存
u8 Times=0,First_Int = 0,shijian=0;
vu8 Timer0_start;	//定时器0延时启动计数器

u8 content[200];//TCP发送内容

u8 display1[16];	
u8 display2[16];
u8 display3[16];	
u8 display4[16];

u16 adcx;
float temp;
	
u8 reclen=0;  
int i;

/*************	本地函数声明	**************/
void CLR_Buf2(void);
u8 Find(char *a);
void Second_AT_Command(char *b,char *a,u8 wait_time);
void Wait_CREG(void);
void Init_TCP(void);
void Send_TCP(void);

/*************  外部函数和变量声明*****************/
extern u8  USART1_RX_BUF[200]; //接收缓冲
extern u16 USART1_RX_STA;      //接收状态标记	

/*******************************************************************************
* 函数名 : main 
* 描述   : 主函数
* 输入   : 
* 输出   : 
* 返回   : 
* 注意   : 
*******************************************************************************/
int main(void)
{

	SysTick_Init_Config();
	GPIO_Config();	
	
	Adc_Init();		  					//ADC初始化	    
	
	USART1_Init_Config(9600);
	USART2_Init_Config(115200);
	Timer2_Init_Config();
	
	I2C_Configuration();				//I2C初始化
	OLED_Init();						//OLED初始化
	OLED_Fill(0x00);					//OLED全屏灭
	
	Wait_CREG();   					    //查询等待模块注册成功
	
	Second_AT_Command("AT+CSQ","+CSQ",3);
	sprintf((char*)display1,"RSSI:%d",(Uart2_Buf[5]-48)*10+Uart2_Buf[6]-48);
	OLED_ShowStr(0,0,display1,2);		//显示信号强度
	
	adcx=Get_Adc_Average(ADC_Channel_1,10);
	temp=(float)adcx*(3.3/4096)/0.21;
	adcx=temp;
	temp-=adcx;
	temp*=100;
	sprintf((char*)display3,"Voltage:%02d.%02d V",adcx,(int)temp);
	OLED_ShowStr(64,0,display1,2);		//显示电池电压（0-3.3V）
	
	Init_TCP();
	
	while(1)
	{	
		if(USART1_RX_STA&0X8000)				//接收到一次数据了
		{
			reclen=USART1_RX_STA&0X7FFF;		//得到数据长度
			USART1_RX_BUF[reclen]=0;	 		//加入结束符
			USART1_RX_STA=0;
			for(i=0;i<reclen;i++)
			{
				content[i]=USART1_RX_BUF[i];		
			}
			OLED_ShowStr(0,2,USART1_RX_BUF,2);  //显示接收数据
		}	
		
		Send_TCP();								//TCP协议发送数据到服务器端口
		
	}
}


void USART2_IRQHandler(void)                	
{
			u8 Res=0;
			Res =USART_ReceiveData(USART2);
			Uart2_Buf[First_Int] = Res;  	  //将接收到的字符串存到缓存中
			First_Int++;                			//缓存指针向后移动
			if(First_Int > Buf2_Max)       		//如果缓存满,将缓存指针指向缓存的首地址
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
* 函数名 : CLR_Buf2
* 描述   : 清除串口2缓存数据
* 输入   : 
* 输出   : 
* 返回   : 
* 注意   : 
*******************************************************************************/
void CLR_Buf2(void)
{
	u16 k;
	for(k=0;k<Buf2_Max;k++)      //将缓存内容清零
	{
		Uart2_Buf[k] = 0x00;
	}
    First_Int = 0;              //接收字符串的起始存储位置
}

/*******************************************************************************
* 函数名 : Find
* 描述   : 判断缓存中是否含有指定的字符串
* 输入   : 
* 输出   : 
* 返回   : unsigned char:1 找到指定字符，0 未找到指定字符 
* 注意   : 
*******************************************************************************/

u8 Find(char *a)
{ 
  if(strstr(Uart2_Buf,a)!=NULL)
	    return 1;
	else
			return 0;
}

/*******************************************************************************
* 函数名 : Second_AT_Command
* 描述   : 发送AT指令函数
* 输入   : 发送数据的指针、希望接收到的应答、发送等待时间(单位：S)
* 输出   : 
* 返回   : 
* 注意   : 
*******************************************************************************/

void Second_AT_Command(char *b,char *a,u8 wait_time)         
{
	u8 i;
	char *c;
	c = b;										//保存字符串地址到c
	CLR_Buf2(); 
  i = 0;
	while(i == 0)                    
	{
		if(!Find(a)) 
		{
			if(Timer0_start == 0)
			{
				b = c;							//将字符串地址给b
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
* 函数名 : Init_TCP
* 描述   : 设置TCP连接
* 输入   : 
* 输出   : 
* 返回   : 
* 注意   : 
*******************************************************************************/
void Init_TCP(void)
{
	Second_AT_Command("AT+CGATT=1","OK",3);								 //GPRS 附着								
	Second_AT_Command("AT+CGACT=1,1","OK",3);						 	 //PDP上下文激活
	Second_AT_Command("AT+CIPSTART=\"TCP\",\"112.74.55.52\",8000","OK",3);	
}

/*******************************************************************************
* 函数名 : Send_TCP
* 描述   : 发送TCP数据
* 输入   : 
* 输出   : 
* 返回   : 
* 注意   : 
*******************************************************************************/
void Send_TCP(void)
{

	Second_AT_Command("AT+CIPSEND",">",3); //发送数据长度：25（具体的计算方法看串口调试比较）接收到“>”才发送短信内容
	UART2_SendString((char*)content);     //发送短信内容
	USART_SendData(USART2 ,0X1A);  //发送结束符
	UART2_SendLR();
}

/*******************************************************************************
* 函数名 : Wait_CREG
* 描述   : 等待模块注册成功
* 输入   : 
* 输出   : 
* 返回   : 
* 注意   : 
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
		UART2_SendString("AT+CREG?");   //查找模块是否注册成功
		UART2_SendLR();
		Delay_nMs(5000);  						
	    for(k=0;k<Buf2_Max;k++)      			
    	{
			if(Uart2_Buf[k] == ':')
			{
				if(((Uart2_Buf[k+4] == '1')&&(Uart2_Buf[k+5] != '3'))||(Uart2_Buf[k+4] == '5'))  //说明模块已经注册成功
				{
					i = 1;
				  break;
				}
			}
		}
	}
}
