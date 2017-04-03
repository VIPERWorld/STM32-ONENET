/**
	************************************************************
	************************************************************
	************************************************************
	*	�ļ����� 	onenet.c
	*
	*	���ߣ� 		MDZZ
	*
	*	���ڣ� 		2016-12-07
	*
	*	�汾�� 		V1.1
	*
	*	˵���� 		��onenetƽ̨�����ݽ�����Э���
	*
	*	�޸ļ�¼��	V1.1�����˿�������������������д---��غ�����OneNet_SendData��OneNet_Load_DataStream
	*					  ���������ӷ�ʽ2---��غ�����OneNet_DevLink
	*					  ������PUSHDATA���ܣ��豸���豸֮���ͨ��---��غ�����OneNet_PushData
	************************************************************
	************************************************************
	************************************************************
**/

//��Ƭ��ͷ�ļ�
#include "stm32f10x.h"

//�����豸
#include "net_device.h"

//Э���ļ�
#include "onenet.h"
#include "fault.h"

//Ӳ������
#include "usart.h"
#include "delay.h"
#include "led.h"
#include "beep.h"
#include "iwdg.h"
#include "hwtimer.h"
#include "selfcheck.h"
#include "relay.h"

//ͼƬ�����ļ�
#include "image_2k.h"

//C��
#include <string.h>
#include <stdlib.h>
#include <stdio.h>




EdpPacket *send_pkg;	//Э���

ONETNET_INFO oneNetInfo = {"5041895", "IljqkD9W0fjzNS87iXIaLXQxQ1M=", "183.230.40.39", "876", 0, 0, 0, 0};//
extern DATA_STREAM dataStream[];


/*
************************************************************
*	�������ƣ�	OneNet_DevLink
*
*	�������ܣ�	��onenet��������
*
*	��ڲ�����	devid�������豸��devid
*				auth_key�������豸��masterKey��apiKey
*
*	���ز�����	��
*
*	˵����		��onenetƽ̨�������ӣ��ɹ������oneNetInfo.netWork����״̬��־
************************************************************
*/
void OneNet_DevLink(const char* devid, const char* auth_key)
{

	unsigned char *dataPtr;
	
	UsartPrintf(USART_DEBUG, "OneNet_DevLink\r\n"
                        "DEVID: %s,     APIKEY: %s\r\n"
                        , devid, auth_key);

#if 1
	send_pkg = PacketConnect1(devid, auth_key);						//����devid �� apikey��װЭ���
#else
	send_pkg = PacketConnect2(devid, auth_key);						//���ݲ�Ʒid �� ��Ȩ��Ϣ��װЭ���
#endif
	
	NET_DEVICE_SendData(send_pkg->_data, send_pkg->_write_pos);		//�ϴ�ƽ̨
	
	dataPtr = NET_DEVICE_GetIPD(250);								//�ȴ�ƽ̨��Ӧ
	if(dataPtr != NULL)
	{
		oneNetInfo.netWork = OneNet_EDPKitCmd(dataPtr);				//��������
	}
	
	DeleteBuffer(&send_pkg);										//ɾ��
	
	if(oneNetInfo.netWork)											//�������ɹ�
	{
		UsartPrintf(USART_DEBUG, "Tips:	NetWork OK\r\n");
		
		oneNetInfo.errCount = 0;
		
		NET_DEVICE_Set_DataMode(DEVICE_DATA_MODE);					//����Ϊ�����շ�ģʽ
	}
	else
	{
		UsartPrintf(USART_DEBUG, "Tips:	NetWork Fail\r\n");
		
		if(++oneNetInfo.errCount >= 5)								//��������趨�����󣬻�δ����ƽ̨
		{
			oneNetInfo.netWork = 0;
			faultType = faultTypeReport = FAULT_NODEVICE;			//���ΪӲ������
		}
	}
	
}

/*
************************************************************
*	�������ƣ�	OneNet_PushData
*
*	�������ܣ�	PUSHDATA
*
*	��ڲ�����	dst_devid�������豸��devid
*				data����������
*				data_len�����ݳ���
*
*	���ز�����	��
*
*	˵����		�豸���豸֮���ͨ��
************************************************************
*/
_Bool OneNet_PushData(const char* dst_devid, const char* data, unsigned int data_len)
{
	
	if(!oneNetInfo.netWork || NET_DEVICE_Get_DataMode() != DEVICE_DATA_MODE)		//�������δ���� �� ��Ϊ�����շ�ģʽ
		return 1;
	
	NET_DEVICE_Set_DataMode(DEVICE_CMD_MODE);										//����Ϊ�����շ�ģʽ

	send_pkg = PacketPushdata(dst_devid, data, data_len);
	
	NET_DEVICE_SendData(send_pkg->_data, send_pkg->_write_pos);						//�ϴ�ƽ̨
	
	DeleteBuffer(&send_pkg);														//ɾ��
	
	NET_DEVICE_Set_DataMode(DEVICE_DATA_MODE);
	
	return 0;

}

/*
************************************************************
*	�������ƣ�	OneNet_toString
*
*	�������ܣ�	����ֵתΪ�ַ���
*
*	��ڲ�����	dataStream��������
*				buf��ת����Ļ���
*				pos���������е��ĸ�����
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void OneNet_toString(DATA_STREAM *dataStream, char *buf, unsigned short pos)
{
	
	memset(buf, 0, 48);

	switch((unsigned char)dataStream[pos].dataType)
	{
		case TYPE_BOOL:
			snprintf(buf, 48, "%d", *(_Bool *)dataStream[pos].data);
		break;
		
		case TYPE_CHAR:
			snprintf(buf, 48, "%d", *(char *)dataStream[pos].data);
		break;
		
		case TYPE_UCHAR:
			snprintf(buf, 48, "%d", *(unsigned char *)dataStream[pos].data);
		break;
		
		case TYPE_SHORT:
			snprintf(buf, 48, "%d", *(short *)dataStream[pos].data);
		break;
		
		case TYPE_USHORT:
			snprintf(buf, 48, "%d", *(unsigned short *)dataStream[pos].data);
		break;
		
		case TYPE_INT:
			snprintf(buf, 48, "%d", *(int *)dataStream[pos].data);
		break;
		
		case TYPE_UINT:
			snprintf(buf, 48, "%d", *(unsigned int *)dataStream[pos].data);
		break;
		
		case TYPE_LONG:
			snprintf(buf, 48, "%ld", *(long *)dataStream[pos].data);
		break;
		
		case TYPE_ULONG:
			snprintf(buf, 48, "%ld", *(unsigned long *)dataStream[pos].data);
		break;
			
		case TYPE_FLOAT:
			snprintf(buf, 48, "%f", *(float *)dataStream[pos].data);
		break;
		
		case TYPE_DOUBLE:
			snprintf(buf, 48, "%f", *(double *)dataStream[pos].data);
		break;
		
		case TYPE_GPS:
			snprintf(buf, 48, "{\"lon\":%s,\"lat\":%s}", (char *)dataStream[pos].data, (char *)dataStream[pos].data + 16);
		break;
	}

}

/*
************************************************************
*	�������ƣ�	OneNet_Load_DataStream
*
*	�������ܣ�	��������װ
*
*	��ڲ�����	type���������ݵĸ�ʽ
*				send_buf�����ͻ���ָ��
*				len�������������ĸ���
*
*	���ز�����	��
*
*	˵����		��װ��������ʽ
************************************************************
*/
void OneNet_Load_DataStream(unsigned char type, char *send_buf, unsigned char len)
{
	
	unsigned char count = 0;
	char stream_buf[96];
	char data_buf[48];
	char *ptr = send_buf;

	switch(type)
	{
		case kTypeFullJson:
			
			UsartPrintf(USART_DEBUG, "Tips:	OneNet_SendData-kTypeFullJson\r\n");
		
			strncpy(send_buf, "{\"datastreams\":[", strlen("{\"datastreams\":["));
			for(; count < len; count++)
			{
				if(dataStream[count].flag) //���ʹ�ܷ��ͱ�־λ
				{
					memset(stream_buf, 0, sizeof(stream_buf));

					OneNet_toString(dataStream, data_buf, count);
					snprintf(stream_buf, sizeof(stream_buf), "{\"id\":\"%s\",\"datapoints\":[{\"value\":%s}]},", dataStream[count].name, data_buf);
					
					strncat(send_buf, stream_buf, strlen(stream_buf));
				}
			}
			
			while(*ptr != '\0')					//�ҵ�������
				ptr++;
			*(--ptr) = '\0';					//������','�滻Ϊ������
			
			strncat(send_buf, "]}", 2);
			
			send_pkg = PacketSaveJson(NULL, send_buf, kTypeFullJson);				//���
		
		break;
			
		case kTypeSimpleJsonWithoutTime:
			
			UsartPrintf(USART_DEBUG, "Tips:	OneNet_SendData-kTypeSimpleJsonWithoutTime\r\n");
		
			strncpy(send_buf, "{", 1);
			for(; count < len; count++)
			{
				if(dataStream[count].flag) //���ʹ�ܷ��ͱ�־λ
				{
					memset(stream_buf, 0, sizeof(stream_buf));

					OneNet_toString(dataStream, data_buf, count);
					snprintf(stream_buf, sizeof(stream_buf), "\"%s\":%s,", dataStream[count].name, data_buf);

					strncat(send_buf, stream_buf, strlen(stream_buf));
				}
			}
			
			while(*ptr != '\0')					//�ҵ�������
				ptr++;
			*(--ptr) = '\0';					//������','�滻Ϊ������
			
			strncat(send_buf, "}", 1);
			
			send_pkg = PacketSaveJson(NULL, send_buf, kTypeSimpleJsonWithoutTime);			//���
		
		break;
		
		case kTypeSimpleJsonWithTime:
			
			UsartPrintf(USART_DEBUG, "Tips:	OneNet_SendData-kTypeSimpleJsonWithTime\r\n");
		
			strncpy(send_buf, "{", 1);
			for(; count < len; count++)
			{
				if(dataStream[count].flag) //���ʹ�ܷ��ͱ�־λ
				{
					memset(stream_buf, 0, sizeof(stream_buf));
					
					OneNet_toString(dataStream, data_buf, count);
					snprintf(stream_buf, sizeof(stream_buf), "\"%s\":{\"2016-08-10T12:31:17\":%s},", dataStream[count].name, data_buf);
					
					strncat(send_buf, stream_buf, strlen(stream_buf));
				}
			}
			
			while(*ptr != '\0')					//�ҵ�������
				ptr++;
			*(--ptr) = '\0';					//������','�滻Ϊ������
			
			strncat(send_buf, "}", 1);
			
			send_pkg = PacketSaveJson(NULL, send_buf, kTypeSimpleJsonWithTime);				//���
		
		break;
		
		case kTypeString:
			
			UsartPrintf(USART_DEBUG, "Tips:	OneNet_SendData-kTypeString\r\n");
		
			strncpy(send_buf, ",;", 2);
			for(; count < len; count++)
			{
				if(dataStream[count].flag) //���ʹ�ܷ��ͱ�־λ
				{
					memset(stream_buf, 0, sizeof(stream_buf));
					
					OneNet_toString(dataStream, data_buf, count);
					snprintf(stream_buf, sizeof(stream_buf), "%s,%s;", dataStream[count].name, data_buf);
				}
				
				strncat(send_buf, stream_buf, strlen(stream_buf));
			}
			
			send_pkg = PacketSavedataSimpleString(NULL, send_buf);							//���
		
		break;
	}

}

/*
************************************************************
*	�������ƣ�	OneNet_SendData
*
*	�������ܣ�	�ϴ����ݵ�ƽ̨
*
*	��ڲ�����	type���������ݵĸ�ʽ
*				len�������������ĸ���
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
_Bool OneNet_SendData(SaveDataType type, unsigned char len)
{
	
	char send_buf[SEND_BUF_SIZE];
	
	if(!oneNetInfo.netWork || NET_DEVICE_Get_DataMode() != DEVICE_DATA_MODE)		//�������δ���� �� ��Ϊ�����շ�ģʽ
		return 1;
	
	NET_DEVICE_Set_DataMode(DEVICE_CMD_MODE);										//����Ϊ�����շ�ģʽ
	memset(send_buf, 0, SEND_BUF_SIZE);
	
	if(type != kTypeBin)															//�������ļ���ȫ���������ã�����Ҫִ����Щ
	{
		OneNet_Load_DataStream(type, send_buf, len);								//����������
		NET_DEVICE_SendData(send_pkg->_data, send_pkg->_write_pos);					//�ϴ����ݵ�ƽ̨
		
		DeleteBuffer(&send_pkg);													//ɾ��
		
		faultTypeReport = FAULT_NONE;												//����֮��������
	}
	else
	{
		UsartPrintf(USART_DEBUG, "Tips:	OneNet_SendData-kTypeBin\r\n");
		
		OneNet_SendData_EDPType2();
	}
	
	NET_DEVICE_Set_DataMode(DEVICE_DATA_MODE);
	
	return 0;
	
}

/*
************************************************************
*	�������ƣ�	OneNet_SendData_EDPType2
*
*	�������ܣ�	�ϴ����������ݵ�ƽ̨
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		���ǵ����豸����������ʱ������ʹ�������豸��͸��ģʽ
************************************************************
*/
#define PKT_SIZE 1024
void OneNet_SendData_EDPType2(void)
{

	char text[] = "{\"ds_id\":\"pic\"}";								//ͼƬ����ͷ
	unsigned int len = sizeof(Array);
	unsigned char *pImage = (unsigned char *)Array;					
	
	UsartPrintf(USART_DEBUG, "image len = %d", len);
	
	send_pkg = PacketSaveBin(NULL, text, len);							//һ����ͼƬ���ݾ�HardFault_Handler��
										//ԭ����������ʱ��д��devid��ƽ̨����ϴ���ͼƬ�������·�һ�Σ����´���buf������Ӷ�����HardFault_Handler
	//send_pkg = PacketSaveBin(oneNetInfo.devID, text, len);//���ڴ��ڽ���������˷�ֹ�������Խ��Ĵ������²����ˡ�
	
	NET_DEVICE_SendData(send_pkg->_data, send_pkg->_write_pos);			//��ƽ̨�ϴ����ݵ�
	
	DeleteBuffer(&send_pkg);											//ɾ��
	
	while(len > 0)
	{
		DelayXms(50);													//��ͼʱ��ʱ�������һ�㣬�����������һ����ʱ
		
		if(len >= PKT_SIZE)
		{
			NET_DEVICE_SendData(pImage, PKT_SIZE);						//���ڷ��ͷ�Ƭ
			
			pImage += PKT_SIZE;
			len -= PKT_SIZE;
		}
		else
		{
			NET_DEVICE_SendData(pImage, (unsigned short)len);			//���ڷ������һ����Ƭ
			len = 0;
		}
	}

}

/*
************************************************************
*	�������ƣ�	OneNet_HeartBeat
*
*	�������ܣ�	�������
*
*	��ڲ�����	��
*
*	���ز�����	��
*
*	˵����		
************************************************************
*/
void OneNet_HeartBeat(void)
{

	unsigned char heartBeat[2] = {PINGREQ, 0}, sCount = 5;
	unsigned char errType = 0;
	unsigned char *dataPtr;
	
	if(!oneNetInfo.netWork || NET_DEVICE_Get_DataMode() != DEVICE_DATA_MODE)	//�������Ϊ���� �� ��Ϊ�����շ�ģʽ
		return;
	
	NET_DEVICE_Set_DataMode(DEVICE_HEART_MODE);									//����Ϊ�����շ�ģʽ
	
	while(--sCount)																//ѭ��������
	{
		NET_DEVICE_SendData(heartBeat, sizeof(heartBeat));						//��ƽ̨�ϴ���������

		dataPtr = NET_DEVICE_GetIPD(200);										//��ȡ���ݣ��ȵ�2s
		if(dataPtr != NULL)														//�������ָ�벻Ϊ��
		{
			if(dataPtr[0] == PINGRESP)											//��һ������Ϊ������Ӧ
			{
				errType = CHECK_NO_ERR;											//����޴���
				UsartPrintf(USART_DEBUG, "Tips:	HeartBeat OK\r\n");
				break;
			}
		}
		else
		{
			if(dataPtr[0] != PINGRESP)											//����δ��Ӧ
			{
				UsartPrintf(USART_DEBUG, "Check Device\r\n");
					
				errType = NET_DEVICE_Check();									//�����豸״̬���
			}
		}
			
		DelayXms(10);															//��ʱ�ȴ�
	}
	
	if(sCount == 0)																//����
	{
		UsartPrintf(USART_DEBUG, "HeartBeat TimeOut\r\n");
		
		errType = NET_DEVICE_Check();											//�����豸״̬���
	}
	
	if(errType == CHECK_CONNECTED || errType == CHECK_CLOSED || errType == CHECK_GOT_IP)
		faultTypeReport = faultType = FAULT_EDP;								//���ΪЭ�����
	else if(errType == CHECK_NO_DEVICE)
		faultTypeReport = faultType = FAULT_NODEVICE;							//���Ϊ�豸����
	else
		faultTypeReport = faultType = FAULT_NONE;								//�޴���
	
	NET_DEVICE_ClrData();														//�������
	
	NET_DEVICE_Set_DataMode(DEVICE_DATA_MODE);									//����Ϊ�����շ�ģʽ

}

/*
************************************************************
*	�������ƣ�	OneNet_EDPKitCmd
*
*	�������ܣ�	EDPЭ���������
*
*	��ڲ�����	data��ƽ̨�·�������
*
*	���ز�����	��
*
*	˵����		��ʱֻ�������ӽ���
************************************************************
*/
_Bool OneNet_EDPKitCmd(unsigned char *data)
{

	if(data[0] == CONNRESP) //������Ӧ
	{
		UsartPrintf(USART_DEBUG, "DevLink: %d\r\n", data[3]);
		
		//0		���ӳɹ�
		//1		��֤ʧ�ܣ�Э�����
		//2		��֤ʧ�ܣ��豸ID��Ȩʧ��
		//3		��֤ʧ�ܣ�������ʧ��
		//4		��֤ʧ�ܣ��û�ID��Ȩʧ��
		//5		��֤ʧ�ܣ�δ��Ȩ
		//6		��֤ʧ�ܣ�������δ����
		//7		��֤ʧ�ܣ����豸�ѱ�����
		//8		��֤ʧ�ܣ��ظ��������������
		//9		��֤ʧ�ܣ��ظ��������������
		
		if(data[3] == 0)
			return 1;
		else
			return 0;
	}
	
	return 0;

}

/*
************************************************************
*	�������ƣ�	OneNet_Replace
*
*	�������ܣ�	�滻ƽ̨�·������еĽ�����
*
*	��ڲ�����	res��ƽ̨�·�������
*
*	���ز�����	��
*
*	˵����		ƽ̨���ء���ť�·���������й̶�4�����������滻Ϊ�����������ַ����Է���string�����
************************************************************
*/
void OneNet_Replace(unsigned char *res, unsigned char num)
{
	
	unsigned char count = 0;

	while(count < num)
	{
		if(*res == '\0')		//�ҵ�������
		{
			*res = '~';			//�滻
			count++;
		}
		
		res++;
	}

}

/*
************************************************************
*	�������ƣ�	OneNet_App
*
*	�������ܣ�	ƽ̨�·��������������
*
*	��ڲ�����	cmd��ƽ̨�·�������
*
*	���ز�����	��
*
*	˵����		��ȡ�������Ӧ����
************************************************************
*/
void OneNet_App(unsigned char *cmd)
{

	char *dataPtr;
	char numBuf[10];
	int num = 0;
	
	if(cmd[0] == PUSHDATA)								//pushdata���ܽ��յ�����
		OneNet_Replace(cmd, 1);
	else
		OneNet_Replace(cmd, 4);
	
	dataPtr = strstr((const char *)cmd, "}");			//����'}'

	if(dataPtr != NULL)									//����ҵ���
	{
		dataPtr++;
		
		while(*dataPtr >= '0' && *dataPtr <= '9')		//�ж��Ƿ����·��������������
		{
			numBuf[num++] = *dataPtr++;
		}
		
		num = atoi((const char *)numBuf);				//תΪ��ֵ��ʽ
		
		if(strstr((char *)cmd, "redled"))				//����"redled"
		{
			if(num == 1)								//�����������Ϊ1������
			{
				Led4_Set(LED_ON);
			}
			else if(num == 0)							//�����������Ϊ0�������
			{
				Led4_Set(LED_OFF);
			}
			
			oneNetInfo.sendData = 1;					//������ݷ���
		}
														//��ͬ
		else if(strstr((char *)cmd, "greenled"))
		{
			if(num == 1)
			{
				Led5_Set(LED_ON);
			}
			else if(num == 0)
			{
				Led5_Set(LED_OFF);
			}
			
			oneNetInfo.sendData = 1;
		}
		else if(strstr((char *)cmd, "yellowled"))
		{
			if(num == 1)
			{
				Led6_Set(LED_ON);
			}
			else if(num == 0)
			{
				Led6_Set(LED_OFF);
			}
			
			oneNetInfo.sendData = 1;
		}
		else if(strstr((char *)cmd, "blueled"))
		{
			if(num == 1)
			{
				Led7_Set(LED_ON);
			}
			else if(num == 0)
			{
				Led7_Set(LED_OFF);
			}
			
			oneNetInfo.sendData = 1;
		}
		else if(strstr((char *)cmd, "beep"))
		{
			if(num == 1)
			{
				Beep_Set(BEEP_ON);
			}
			else if(num == 0)
			{
				Beep_Set(BEEP_OFF);
			}
			
			oneNetInfo.sendData = 1;
		}
		
		
		else if(strstr((char *)cmd, "relaya"))		//�̵���1
		{
			if(num == 1)
			{
				J1_Set(RELAY_ON);
			}
			else if(num == 0)
			{
				J1_Set(RELAY_OFF);
			}
			
			oneNetInfo.sendData = 1;
		}
		
		
				
		else if(strstr((char *)cmd, "relayb"))		//�̵���2
		{
			if(num == 1)
			{
				J2_Set(RELAY_ON);
			}
			else if(num == 0)
			{
				J2_Set(RELAY_OFF);
			}
			
			oneNetInfo.sendData = 1;
		}
	}
	
	NET_DEVICE_ClrData();								//��ջ���

}
