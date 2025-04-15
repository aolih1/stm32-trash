#include "stm32f10x.h"
#include "delay.h"
#include "Reg_RW.h"
#include "LDChip.h"
#include "usart.h"
#include "sys.h"
#include "syn6288.h"
#include "stdio.h"
#include "PWM.h"
#include "Sensor.h"

u8 SYN_StopCom[] = {0xFD, 0X00, 0X02, 0X02, 0XFD}; //停止合成
u8 SYN_SuspendCom[] = {0XFD, 0X00, 0X02, 0X03, 0XFC}; //暂停合成
u8 SYN_RecoverCom[] = {0XFD, 0X00, 0X02, 0X04, 0XFB}; //恢复合成
u8 SYN_ChackCom[] = {0XFD, 0X00, 0X02, 0X21, 0XDE}; //状态查询
u8 SYN_PowerDownCom[] = {0XFD, 0X00, 0X02, 0X88, 0X77}; //进入POWER DOWN 状态命令

static int flag1,flag2,flag3; //舵机状态

void User_Modification(u8 dat);

u8 nAsrStatus=0;
u8 nAsrRes=0;
u8 flag=0;


int main(void)
{	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	delay_init();
	PWM_Init();
	LD3320_Init();
	EXTIX_Init();
	LD_Reset();
	uart_init(9600);
	USART3_Init(9600);
	Sensor_Init();
	TIM_SetCompare1(TIM2, 500);
  TIM_SetCompare2(TIM2, 500);
  TIM_SetCompare3(TIM2, 500);
	nAsrStatus = LD_ASR_NONE;		//	初始状态：没有在作ASR
	SCS=0;
	printf("运行程序\r\n");
	while(1)
	{
		// GPIO_Pin_12 | GPIO_Pin_13 |  GPIO_Pin_14
		if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_12))
		{
			SYN_FrameInfo(2,"[v7][m1][t5]可回收垃圾已满");
		};
			
		if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_13))
		{
			SYN_FrameInfo(2,"[v7][m1][t5]不可回收垃圾已满");
		};
		
		if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_14))
		{
			SYN_FrameInfo(2,"[v7][m1][t5]有害垃圾已满");
		};
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
			
			case CODE_1KL1:	 /*命令“可回收垃圾”*/
					printf("\"可回收垃圾\"识别成功\r\n"); /*text.....*/
					SYN_FrameInfo(2,"[v7][m1][t5]可回收垃圾");
					flag1 = ~flag1;
				  if(flag1)
						TIM_SetCompare1(TIM2, 1500);
					else
						TIM_SetCompare1(TIM2, 500);
					delay_ms(1500);
												break;
			case CODE_1KL2:		/*命令“不可回收垃圾”*/
		
					printf("\"不可回收垃圾\"识别成功\r\n"); /*text.....*/
					SYN_FrameInfo(2,"[v7][m1][t5]不可回收垃圾");
					flag2 = ~flag2;
				  if(flag2)
						TIM_SetCompare2(TIM2, 1500);
					else
						TIM_SetCompare2(TIM2, 500);
					delay_ms(1500);
												break;
			case CODE_1KL3:	 /*命令“有害垃圾”*/
					printf("\"有害垃圾\"识别成功\r\n"); /*text.....*/
					flag3 = ~flag3;
				  if(flag3)
						TIM_SetCompare4(TIM2, 1500);
					else
						TIM_SetCompare4(TIM2, 500);
					delay_ms(1500);
												break;
			
			default:break;
		}
	}
	else 	
	{
		printf("请说出一级口令\r\n"); /*text.....*/	
	}
	
}



void EXTI0_IRQHandler()
{
	if (EXTI_GetITStatus(EXTI_Line0) == SET)		//判断是否是外部中断0号线触发的中断
	{
		/*如果出现数据乱跳的现象，可再次判断引脚电平，以避免抖动*/
		if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_0) == 0)
		{
			    flag1 = ~flag1;
				  if(flag1)
						TIM_SetCompare1(TIM2, 1500);
					else
						TIM_SetCompare1(TIM2, 500);
		}
		EXTI_ClearITPendingBit(EXTI_Line0);		//清除外部中断14号线的中断标志位
													//中断标志位必须清除
													//否则中断将连续不断地触发，导致主程序卡死
	}
};
void EXTI1_IRQHandler()
{
		if (EXTI_GetITStatus(EXTI_Line1) == SET)		//判断是否是外部中断1号线触发的中断
	{
		/*如果出现数据乱跳的现象，可再次判断引脚电平，以避免抖动*/
		if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_1) == 0)
		{
			    flag2 = ~flag2;
				  if(flag1)
						TIM_SetCompare2(TIM2, 1500);
					else
						TIM_SetCompare2(TIM2, 500);
		}
		EXTI_ClearITPendingBit(EXTI_Line1);		
													//中断标志位必须清除
													//否则中断将连续不断地触发，导致主程序卡死
	}
}

void EXTI9_5_IRQHandler(void)
{
	if(EXTI_GetITStatus(LD3320_IRQEXITLINE)!= RESET ) 
	{
		ProcessInt(); 
 		//printf("进入中断\r\n");	
		EXTI_ClearFlag(LD3320_IRQEXITLINE);
		EXTI_ClearITPendingBit(LD3320_IRQEXITLINE);//清除LINE上的中断标志位  
	} 
	// 按键PA7
	if (EXTI_GetITStatus(EXTI_Line7) == SET)		
	{
		/*如果出现数据乱跳的现象，可再次判断引脚电平，以避免抖动*/
		if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_7) == 0)
		{
			// TIM_SetCompare4(TIM2, 1500);
			   flag3 = ~flag3;
				  if(flag3)
						TIM_SetCompare4(TIM2, 1500);
					else
						TIM_SetCompare4(TIM2, 500);
			
		}
		EXTI_ClearITPendingBit(EXTI_Line7);		
													//中断标志位必须清除
													//否则中断将连续不断地触发，导致主程序卡死
	}
}
