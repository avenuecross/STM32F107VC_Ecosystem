#include "yeelink.h"
#include "delay.h"
#include "usart.h"
#include "led.h"

extern unsigned char U8T_data_H;
extern unsigned char U8RH_data_H;

unsigned char STRU8T_data_H;
unsigned char STRU8RH_data_H;
unsigned char flag;

unsigned char senAT1[40] = "AT+CWMODE=3\r\n";
unsigned char senAT2[40] = "AT+RST\r\n";
unsigned char senAT3[40] = "AT+CWJAP=\"goushishuai\",\"19941204\"\r\n";
unsigned char senAT4[40] = "AT+CIPMUX=0\r\n";
unsigned char senAT5[40] = "AT+CIPMODE=1\r\n";
//unsigned char senAT6[40] = "AT+CIPSTART=\"TCP\",\"42.96.164.52\",80\r\n";
unsigned char senAT6[40] = "AT+CIPSTART=\"TCP\",\"118.190.25.51\",80\r\n";
unsigned char senAT7[40] = "AT+CIPSEND\r\n";

unsigned char senDHT_T[300] = "POST /v1.1/device/352882/sensor/397594/datapoints HTTP/1.1\r\n\
Host:api.yeelink.net\r\n\
Accept: */*\r\n\
U-ApiKey:e133bfb9f71b42374cc66682cdb9f5b8\r\n\
Content-Length:15\r\n\
Content-Type: application/x-www-form-urlencoded\r\n\
Connection:close\r\n\
\r\n\
{\"value\":  0}\r\n";
unsigned char senDHT_RH[300] = "POST /v1.1/device/352882/sensor/397598/datapoints HTTP/1.1\r\n\
Host:api.yeelink.net\r\n\
Accept: */*\r\n\
U-ApiKey:e133bfb9f71b42374cc66682cdb9f5b8\r\n\
Content-Length:15\r\n\
Content-Type: application/x-www-form-urlencoded\r\n\
Connection:close\r\n\
\r\n\
{\"value\":  0}\r\n";

unsigned char str_T[3]="   ";
unsigned char str_RH[3]="   ";

void transmit2yeelink(void)
{
    delay_ms(3000);
    led_on(LED_3);

    getSTR();
    HAL_UART_Transmit(&huart1, &U8T_data_H, 1, 0xffff);
    HAL_UART_Transmit(&huart1, str_T, sizeof(str_T)/sizeof(unsigned char), 0xffff);
    adddata();
    //uart4ready
    putATinstruction();
    
}

void putATinstruction(void)
{

//    while(uart_get_cmd()!=0x79);
    delay_ms(4000);
    HAL_UART_Transmit(&huart1, senAT1, sizeof(senAT1)/sizeof(unsigned char), 0xffff);
    delay_ms(4000);
    HAL_UART_Transmit(&huart1, senAT2, sizeof(senAT2)/sizeof(unsigned char), 0xffff);
    delay_ms(4000);
    HAL_UART_Transmit(&huart1, senAT3, sizeof(senAT3)/sizeof(unsigned char), 0xffff);
    delay_ms(6000);
    HAL_UART_Transmit(&huart1, senAT4, sizeof(senAT4)/sizeof(unsigned char), 0xffff);
    delay_ms(6000);
    HAL_UART_Transmit(&huart1, senAT5, sizeof(senAT5)/sizeof(unsigned char), 0xffff);
    delay_ms(6000);
    HAL_UART_Transmit(&huart1, senAT6, sizeof(senAT6)/sizeof(unsigned char), 0xffff);
    delay_ms(4000);
    HAL_UART_Transmit(&huart1, senAT7, sizeof(senAT7)/sizeof(unsigned char), 0xffff);

    //transmitdata
    putDHT11_T();//10s interval
    //putDHT11_RH();
    led_off(LED_3);
}

void putDHT11_T(void)
{
    delay_ms(4000);
    HAL_UART_Transmit(&huart1, senDHT_T, sizeof(senDHT_T)/sizeof(unsigned char), 0xffff);
    delay_ms(2000);
    HAL_UART_Transmit(&huart1, senDHT_RH, sizeof(senDHT_RH)/sizeof(unsigned char), 0xffff);
}

void adddata(void)
{
    senDHT_T[235] = str_T[0];
    senDHT_T[236] = str_T[1];
    senDHT_T[237] = str_T[2];
    
    senDHT_RH[235] = str_RH[0];
    senDHT_RH[236] = str_RH[1];
    senDHT_RH[237] = str_RH[2];
}

void getSTR(void)
{
//    change(STRU8T_data_H,str_T);
//    change(STRU8RH_data_H,str_RH);
    
    int i=0;
    
    for(i = 2; i >= 0; i--)
        str_T[i] = ' ';
    for(i = 2; i >= 0; i--)
        str_RH[i] = ' ';
    
    for(i = 2; i >= 0; i--)
	{
        if(U8T_data_H > 0)
        str_T[i] = U8T_data_H%((unsigned char)10) + 48;
        U8T_data_H = U8T_data_H/10;
    }
    for(i = 2; i > 0; i--)
	{
        if(U8RH_data_H > 0)
        str_RH[i] = U8RH_data_H%((unsigned char)10) + 48;
        U8RH_data_H = U8RH_data_H/10;
    }
}

//void change(unsigned char n,unsigned char *str)
//{
//	int i = 0;
////    int l = 0;
////    char m = 0;
////	m = n;
////	while(m != 0)
////	{
////		m = m/10;
////		l++;
////	}

//	for(i = 0; i < 3; i++)
//	{
//        str[i] = n%((unsigned char)10) + 48;
//        n = n/10;
//	}
//}













