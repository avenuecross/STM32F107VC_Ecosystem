#include "DHT11.h"
#include "usart.h"
#include "delay.h"
#include "led.h"

typedef unsigned char  U8;       /* defined for unsigned 8-bits integer variable 	  无符号8位整型变量  */
typedef signed   char  S8;       /* defined for signed 8-bits integer variable		  有符号8位整型变量  */
typedef unsigned int   U16;      /* defined for unsigned 16-bits integer variable 	  无符号16位整型变量 */
typedef signed   int   S16;      /* defined for signed 16-bits integer variable 	  有符号16位整型变量 */
typedef unsigned long  U32;      /* defined for unsigned 32-bits integer variable 	  无符号32位整型变量 */
typedef signed   long  S32;      /* defined for signed 32-bits integer variable 	  有符号32位整型变量 */
typedef float          F32;      /* single precision floating point variable (32bits) 单精度浮点数（32位长度） */
typedef double         F64;      /* double precision floating point variable (64bits) 双精度浮点数（64位长度） */

#define uchar unsigned char
#define uint unsigned int
#define   Data_0_time    4

U8  U8FLAG,k;
U8  U8count,U8temp;
U8  U8T_data_H,U8T_data_L,U8RH_data_H,U8RH_data_L,U8checkdata;
U8  U8T_data_H_temp,U8T_data_L_temp,U8RH_data_H_temp,U8RH_data_L_temp,U8checkdata_temp;
U8  U8comdata;
U8  outdata[5];
U8  indata[5];
U8  count, count_r=0;
U8  str[5]={"RS232"};
U16 U16temp1,U16temp2;

void DHT_Set_Output(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = GPIO_PIN_14;				 
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;		 
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);					 					
}	

void DHT_Set_Input(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.Pin = GPIO_PIN_14;			 
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH; 
	HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);					 
}


void SendData(U8 *a)
{
	outdata[0] = a[0]; 
	outdata[1] = a[1];
	outdata[2] = a[2];
	outdata[3] = a[3];
	outdata[4] = a[4];
	count = 1;
// 	uart_send_cmd(outdata);
}

void COM(void)
{
    U8 i;

    for(i=0;i<8;i++)	   
    {
        U8FLAG=2;	
        while((!(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_14))) && U8FLAG++);
        delay_10us(3);
        U8temp=0;
        if(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_14))
        {
            U8temp=1;
        }
        U8FLAG=2;
        while((HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_14))&&U8FLAG++);
	  
        if(U8FLAG==1)
            break;
        //check data bit logic
        U8comdata<<=1;
        U8comdata|=U8temp;
    }
}

	//----以下变量均为全局变量--------
	//----温度高8位== U8T_data_H------
	//----温度低8位== U8T_data_L------
	//----湿度高8位== U8RH_data_H-----
	//----湿度低8位== U8RH_data_L-----
	//----校验 8位 == U8checkdata-----

void RH(void)
{
   DHT_Set_Output();
  //主机拉低18ms 
   HAL_GPIO_WritePin(GPIOE, GPIO_PIN_14,GPIO_PIN_RESET);
   delay_ms(18);
   HAL_GPIO_WritePin(GPIOE, GPIO_PIN_14,GPIO_PIN_SET);
 //总线由上拉电阻拉高 主机延时20us
   delay_10us(2);
 //主机设为输入 判断从机响应信号
   DHT_Set_Input();
 //判断从机是否有低电平响应信号 如不响应则跳出，响应则向下运行	  
   if(!(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_14)))	  
   {
       U8FLAG=2;
     //判断从机是否发出 80us 的低电平响应信号是否结束	 
       while((!(HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_14))) && U8FLAG++);
       U8FLAG=2;
     //判断从机是否发出 80us 的高电平，如发出则进入数据接收状态
       while((HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_14)) && U8FLAG++);
     //数据接收状态		 
       COM();
       U8RH_data_H_temp = U8comdata;
       COM();
       U8RH_data_L_temp = U8comdata;
       COM();
       U8T_data_H_temp = U8comdata;
       COM();
       U8T_data_L_temp = U8comdata;
       COM();
       U8checkdata_temp = U8comdata;
       HAL_GPIO_WritePin(GPIOE, GPIO_PIN_14,GPIO_PIN_SET);
     //数据校验 
     
       U8temp=(U8T_data_H_temp + U8T_data_L_temp + U8RH_data_H_temp + U8RH_data_L_temp);
       if(U8temp == U8checkdata_temp)
       {
          U8RH_data_H = U8RH_data_H_temp;
          U8RH_data_L = U8RH_data_L_temp;
          U8T_data_H = U8T_data_H_temp;
          U8T_data_L = U8T_data_L_temp;
          U8checkdata = U8checkdata_temp;
       }
   }
}
	
//----------------------------------------------
//main()功能描述:  AT89C51  11.0592MHz 	串口发 
//送温湿度数据,波特率 9600 
//----------------------------------------------
void data_receive(void)
{

    delay_10us(10);

//	while(1)
//	{  
	   //调用温湿度读取子程序 
	   RH();

	   str[0]=U8RH_data_H;
	   str[1]=U8RH_data_L;
	   str[2]=U8T_data_H;
	   str[3]=U8T_data_L;
	   str[4]=U8checkdata;
//	   SendData(str);  //send to port
	   //time of read data from module is perferably less than 2s
	   delay_ms(2000);
//	}
	
}

