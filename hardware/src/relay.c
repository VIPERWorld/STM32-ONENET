/**
	************************************************************
	************************************************************
	************************************************************
	*	�ļ����� 	led.c
	*
	*	���ߣ� 		MDZZ
	*
	*	���ڣ� 		2017-3-28
	*
	*	�汾�� 		V1.0
	*
	*	˵���� 		LED��ʼ�������ؼ̵���
	*
	*	�޸ļ�¼��	
	************************************************************
	************************************************************
	************************************************************
**/
//��Ƭ��ͷ�ļ�
#include "stm32f10x.h"

//�̵���ͷ�ļ�
#include "relay.h"


RELAY_STATUS relayStatus;


/*
************************************************************
*	�������ƣ�	Relay_Init
*
*	�������ܣ�	Relay��ʼ��
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		RELAY4-PA6	RELAY5-PA8	
				�ߵ�ƽ��		�͵�ƽ��
************************************************************
*/
void Relay_Init(void)
{
	
	GPIO_InitTypeDef gpioInitStrcut;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	gpioInitStrcut.GPIO_Mode = GPIO_Mode_Out_PP;
	gpioInitStrcut.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_8 ;
	gpioInitStrcut.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &gpioInitStrcut);

    
    J1_Set(RELAY_OFF);
    J2_Set(RELAY_OFF);


}

/*
************************************************************
*	�������ƣ�	J1_Set
*
*	�������ܣ�	�̵���1����
*
*	��ڲ�����	status��RELAY_ON-��	RELAY_OFF-��
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void J1_Set(RELAY_ENUM status)
{

	GPIO_WriteBit(GPIOA, GPIO_Pin_6, status != RELAY_ON ? Bit_SET : Bit_RESET);
	relayStatus.J1Sta = status;

}

/*
************************************************************
*	�������ƣ�	J2_Set
*
*	�������ܣ�	�̵���2����
*
*	��ڲ�����	status��RELAY_ON-��	RELAY_OFF-��
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void J2_Set(RELAY_ENUM status)
{

	GPIO_WriteBit(GPIOA, GPIO_Pin_8, status != RELAY_ON ? Bit_SET : Bit_RESET);
	relayStatus.J2Sta = status;

}