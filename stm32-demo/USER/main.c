#include "stm32f10x.h"
#include "delay.h"
#include "Reg_RW.h"
#include "LDChip.h"
#include "usart.h"
#include "sys.h"
#include "syn6288.h"
#include "stdio.h"


u8 SYN_StopCom[] = {0xFD, 0X00, 0X02, 0X02, 0XFD}; //ֹͣ�ϳ�
u8 SYN_SuspendCom[] = {0XFD, 0X00, 0X02, 0X03, 0XFC}; //��ͣ�ϳ�
u8 SYN_RecoverCom[] = {0XFD, 0X00, 0X02, 0X04, 0XFB}; //�ָ��ϳ�
u8 SYN_ChackCom[] = {0XFD, 0X00, 0X02, 0X21, 0XDE}; //״̬��ѯ
u8 SYN_PowerDownCom[] = {0XFD, 0X00, 0X02, 0X88, 0X77}; //����POWER DOWN ״̬����



void User_Modification(u8 dat);

u8 nAsrStatus=0;
u8 nAsrRes=0;
u8 flag=0;


int main(void)
{	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	delay_init();
	LD3320_Init();
	EXTIX_Init();
	LD_Reset();
	uart_init(9600);
	USART3_Init(9600);
	nAsrStatus = LD_ASR_NONE;		//	��ʼ״̬��û������ASR
	SCS=0;
	printf("���г���\r\n");
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
				if (RunASR()==0)	/*	����һ��ASRʶ�����̣�ASR��ʼ����ASR��ӹؼ��������ASR����*/
				{
					nAsrStatus = LD_ASR_ERROR;
				}
				break;
			}

			case LD_ASR_FOUNDOK: /*	һ��ASRʶ�����̽�����ȥȡASRʶ����*/
			{
				nAsrRes = LD_GetResult();		/*��ȡ���*/												
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
* ��    �ƣ��û�ִ�к��� 
* ��    �ܣ�ʶ��ɹ���ִ�ж������ڴ˽����޸� 
* ��ڲ����� �� 
* ���ڲ�������
* ˵    ���� 					 
**********************************************************/
void User_Modification(u8 dat)
{
	if(dat ==0)
	{
		flag=1;
		printf("�յ�\r\n");
	}
	else if(flag)
	{
		flag=0;
		switch(nAsrRes)		   /*�Խ��ִ����ز���,�ͻ��޸�*/
		{
			case CODE_DMCS:			/*���������ԡ�*/
					printf("\"�������\"ʶ��ɹ�\r\n"); /*text.....*/
					SYN_FrameInfo(2,"[v7][m1][t5]�������ʶ��ɹ�");
					delay_ms(1500);
												break;
			case CODE_CSWB:			/*���������ϡ�*/
					printf("\"�������\"ʶ��ɹ�\r\n"); /*text.....*/
												break;
			
			case CODE_1KL1:	 /*���������*/
					printf("\"����\"ʶ��ɹ�\r\n"); /*text.....*/
					SYN_FrameInfo(2,"[v7][m1][t5]����");
					delay_ms(1500);
												break;
			case CODE_1KL2:		/*����Ϻ���*/
		
					printf("\"�Ϻ�\"ʶ��ɹ�\r\n"); /*text.....*/
												break;
			case CODE_1KL3:	 /*������ơ�*/
					printf("\"����\"ʶ��ɹ�\r\n"); /*text.....*/
												break;
			case CODE_1KL4:		/*����صơ�*/				
					printf("\"�ص�\"ʶ��ɹ�\r\n"); /*text.....*/
												break;
			
			case CODE_2KL1:	 /*���....��*/
					printf("\"����\"ʶ��ɹ�\r\n"); /*text.....*/
												break;
			case CODE_2KL2:	 /*���....��*/
					printf("\"����\"ʶ��ɹ�\r\n"); /*text.....*/
												break;
			case CODE_2KL3:	 /*���....��*/
					printf("\"����ת\"ʶ��ɹ�\r\n"); /*text.....*/
												break;
			case CODE_2KL4:	 /*���....��*/
					printf("\"����ת\"ʶ��ɹ�\r\n"); /*text.....*/
															break;
						
			case CODE_3KL1:	 /*���....��*/
					printf("\"�򿪿յ�\"ʶ��ɹ�\r\n"); /*text.....*/
												break;
			case CODE_3KL2:	 /*���....��*/
					printf("\"�رտյ�\"ʶ��ɹ�\r\n"); /*text.....*/
												break;
			case CODE_5KL1:	 /*���....��*/
					printf("\"����\"ʶ��ɹ�"); /*text.....*/
												break;
			
			default:break;
		}
	}
	else 	
	{
		printf("��˵��һ������\r\n"); /*text.....*/	
	}
	
}
