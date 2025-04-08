#include "stm32f10x.h"
#include "delay.h"
#include "Reg_RW.h"
#include "LDChip.h"
#include "usart.h"
#include "sys.h"
#include "syn6288.h"
#include "stdio.h"


u8 SYN_StopCom[] = {0xFD, 0X00, 0X02, 0X02, 0XFD}; //停止合成
u8 SYN_SuspendCom[] = {0XFD, 0X00, 0X02, 0X03, 0XFC}; //暂停合成
u8 SYN_RecoverCom[] = {0XFD, 0X00, 0X02, 0X04, 0XFB}; //恢复合成
u8 SYN_ChackCom[] = {0XFD, 0X00, 0X02, 0X21, 0XDE}; //状态查询
u8 SYN_PowerDownCom[] = {0XFD, 0X00, 0X02, 0X88, 0X77}; //进入POWER DOWN 状态命令



void User_Modification(u8 dat);

u8 nAsrStatus=0;
u8 nAsrRes=0;
u8 flag=0;


int main(void)
{	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	delay_init();
	LD3320_Init();
	EXTIX_Init();
	LD_Reset();
	uart_init(9600);
	USART3_Init(9600);
	nAsrStatus = LD_ASR_NONE;		//	初始状态：没有在作ASR
	SCS=0;
	printf("运行程序\r\n");
	while(1)
	{
		switch(nAsrStatus)
		{
			case LD_ASR_RUNING:
			case LD_ASR_ERROR:	
					 break;
			case LD_ASR_NONE:
			{
				nAsrStatus=LD_ASR_RUNING;
				if (RunASR()==0)	/*	启动一次ASR识别流程：ASR初始化，ASR添加关键词语，启动ASR运算*/
				{
					nAsrStatus = LD_ASR_ERROR;
				}
				break;
			}

			case LD_ASR_FOUNDOK: /*	一次ASR识别流程结束，去取ASR识别结果*/
			{
				nAsrRes = LD_GetResult();		/*获取结果*/												
				User_Modification(nAsrRes);
				nAsrStatus = LD_ASR_NONE;
				break;
			}
			case LD_ASR_FOUNDZERO:
			default:
			{
				nAsrStatus = LD_ASR_NONE;
				break;
			}
		} 
	}
}

/***********************************************************
* 名    称：用户执行函数 
* 功    能：识别成功后，执行动作可在此进行修改 
* 入口参数： 无 
* 出口参数：无
* 说    明： 					 
**********************************************************/
void User_Modification(u8 dat)
{
	if(dat ==0)
	{
		flag=1;
		printf("收到\r\n");
	}
	else if(flag)
	{
		flag=0;
		switch(nAsrRes)		   /*对结果执行相关操作,客户修改*/
		{
			case CODE_DMCS:			/*命令“代码测试”*/
					printf("\"代码测试\"识别成功\r\n"); /*text.....*/
					SYN_FrameInfo(2,"[v7][m1][t5]代码测试识别成功");
					delay_ms(1500);
												break;
			case CODE_CSWB:			/*命令“测试完毕”*/
					printf("\"测试完毕\"识别成功\r\n"); /*text.....*/
												break;
			
			case CODE_1KL1:	 /*命令“北京”*/
					printf("\"北京\"识别成功\r\n"); /*text.....*/
					SYN_FrameInfo(2,"[v7][m1][t5]北京");
					delay_ms(1500);
												break;
			case CODE_1KL2:		/*命令“上海”*/
		
					printf("\"上海\"识别成功\r\n"); /*text.....*/
												break;
			case CODE_1KL3:	 /*命令“开灯”*/
					printf("\"开灯\"识别成功\r\n"); /*text.....*/
												break;
			case CODE_1KL4:		/*命令“关灯”*/				
					printf("\"关灯\"识别成功\r\n"); /*text.....*/
												break;
			
			case CODE_2KL1:	 /*命令“....”*/
					printf("\"广州\"识别成功\r\n"); /*text.....*/
												break;
			case CODE_2KL2:	 /*命令“....”*/
					printf("\"深圳\"识别成功\r\n"); /*text.....*/
												break;
			case CODE_2KL3:	 /*命令“....”*/
					printf("\"向左转\"识别成功\r\n"); /*text.....*/
												break;
			case CODE_2KL4:	 /*命令“....”*/
					printf("\"向右转\"识别成功\r\n"); /*text.....*/
															break;
						
			case CODE_3KL1:	 /*命令“....”*/
					printf("\"打开空调\"识别成功\r\n"); /*text.....*/
												break;
			case CODE_3KL2:	 /*命令“....”*/
					printf("\"关闭空调\"识别成功\r\n"); /*text.....*/
												break;
			case CODE_5KL1:	 /*命令“....”*/
					printf("\"后退\"识别成功"); /*text.....*/
												break;
			
			default:break;
		}
	}
	else 	
	{
		printf("请说出一级口令\r\n"); /*text.....*/	
	}
	
}
