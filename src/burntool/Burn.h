/***
*Burn.h - This is the head file for Burn.
*
*       Copyright (c) 2000-2007, Anyka (GuangZhou) Software Technology Co., Ltd. All rights reserved.
*
*Purpose:
*       declare the CBurnBase class and Subclass.
*
* @author	Anyka
* @date		2007-10
* @version	1.0
*
*******************************************************************************/
#ifndef __BURN_H__
#define __BURN_H__

#include "StdAfx.h"

//////////////////////////////////////////////////////////////////////////
//Macro
#define ON_BURNFLASH_MESSAGE		((WM_USER) + (100))
#define ON_BURNFLASH_DEVICE_ARRIVE	((WM_USER) + (101))
#define ON_BURNFLASH_DEVICE_REMOVE	((WM_USER) + (102))
#define ON_MODULE_BURN_MESSAGE		((WM_USER) + (103))
#define ON_MODULE_BURN_PROGRESS		((WM_USER) + (104))
#define ON_BURNFLASH_PROCESS		((WM_USER) + (105))

#define MAX_DOWNLOAD_NAND			50
#define MAX_DOWNLOAD_MTD			50
#define MAX_DOWNLOAD_FILES			1000

//////////////////////////////////////////////////////////////////////////
//Usual enum


typedef struct
{
	char diskName;
	UINT sectorCount;
    UINT sectorSize;
    UINT PageSize;
	BOOL bWritten;
	BOOL bCompare;
	UINT nID;
}T_DISK_INFO;

typedef struct
{
	T_DISK_INFO g_disk_info[16];
	UINT nID;
}T_nID_DISK_INFO;


typedef enum
{
	MESSAGE_SET_STANDBY  = 0,				//����Ϊ׼��״̬
	MESSAGE_SET_UDISK_UPDATE_STANDBY,		//����Ϊ׼��״̬
	MESSAGE_START_BURN,                     //��ʼ��¼�߳�
	MESSAGE_RESET_STANDBY,					//����ΪReset״̬
	MESSAGE_START_SET_REGISTER,				//���üĴ�����Ϣ

	MESSAGE_START_SET_CHANNELID,		    //����ͨ���ĵ�ַ
	MESSAGE_START_CHANNELID_FAIL,		    //ͨ���ĵ�ַ����

	MESSAGE_START_SET_RAMPARAM,             //��ʼ����ram����
	MESSAGE_SET_RAMPARAM_FAIL,				//����ram����ʧ��
	MESSAGE_SET_REGISTER_FAIL,				//���üĴ���ʧ��
	MESSAGE_GET_REGVALUE_FAIL,				//��ȡ�Ĵ���ֵʧ��
	MESSAGE_DOWNLOAD_CHANNELID_FAIL,        //����channel��ַʧ��
	MESSAGE_SET_COMMODE_FAIL,               //���ô�������ʧ��
	MESSAGE_DOWNLOAD_COMADDR_FAIL,			//���ش�������ʧ��
	MESSAGE_SET_REGISTER_SUCCESS,			//���üĴ����ɹ�
	MESSAGE_START_DOWNLOAD_LOADFILE,		//��ʼDownLoad�ļ�
	MESSAGE_DOWNLOAD_LOADFILE_FAIL,			//DownLoad�ļ�ʧ��
	MESSAGE_DOWNLOAD_LOADFILE_SUCCESS,		//DownLoad�ļ��ɹ�
	MESSAGE_START_INIT_USB,					//��ʼ��ʼ��USB
	MESSAGE_INIT_USB_FAIL,					//��ʼ��USBʧ��
	MESSAGE_INIT_USB_SUCCESS,				//��ʼ��USB�ɹ�
	MESSAGE_START_TEST_TRANSC,				//��ʼ����ͨ��
	MESSAGE_TEST_TRANSC_FAIL,				//����ͨ��ʧ��
	MESSAGE_START_ERASE_TRANSC,				//��ʼ����
	MESSAGE_ERASE_TRANSC_FAIL,				//����ʧ��
	MESSAGE_START_FORMAT_TRANSC,			//��ʼ��ʽ��
	MESSAGE_FORMAT_TRANSC_FAIAL,			//��ʽ��ʧ��
	MESSAGE_START_DOWNLOAD_FILE,			//��ʼDownLoad�ļ�
	MESSAGE_DOWNLOAD_FILE_FAIL,				//DownLoad�ļ�ʧ��
	MESSAGE_DOWNLOAD_FILE_SUCCESS,			//DownLoad�ļ��ɹ�
	MESSAGE_TASK_COMPLETE,					//�����������
	MESSAGE_DOWNLOAD_FILE_LEN,				//
	MESSAGE_DOWNLOADING_FILE,				//
	MESSAGE_START_DOWNLOAD_FAT_IMAGE,		//
	MESSAGE_DOWNLOAD_FAT_IMAGE_FAIL,		//	
	MESSAGE_DOWNLOADING_FAT_IMAGE,			//
	MESSAGE_DOWNLOAD_FAT_IMAGE_SUCCESS,		//
	MESSAGE_START_DOWNLOAD_RESOURCE,		//��ʼ������Դ�ļ�
	MESSAGE_DOWNLOAD_RESOURCE_SUCCESS,		//������Դ�ļ�ʧ��
	MESSAGE_DOWNLOAD_RESOURCE_FAIL,			//������Դ�ļ��ɹ�
	MESSAGE_COMPARE_FILE,					//��ʼ�Ƚ��ļ�
	MESSAGE_COMPARE_FILE_FAIL,				//�Ƚ��ļ�ʧ��
	MESSAGE_COMPARE_FILE_SUCCESS,			//�Ƚ��ļ��ɹ�
    MESSAGE_GET_CHIP_PARA,					//��ȡNandFlashоƬ����
    MESSAGE_GET_CHIP_PARA_FAIL,				//��ȡ����ʧ��
    MESSAGE_GET_CHIP_PARA_SUCCESS,			//��ȡ�����ɹ�
    MESSAGE_SET_CHIP_PARA,					//����NandFlashоƬ����
    MESSAGE_SET_CHIP_PARA_FAIL,				//���ò���ʧ��
    MESSAGE_SET_CHIP_PARA_SUCCESS,			//���ò����ɹ�
	MESSAGE_CHECK_FORMAT_DATA_FAIL,			//
	MESSAGE_BEGIN_GET_AID,                  //
	MESSAGE_GET_AID_FAIL,                   //
	MESSAGE_START_IMAGE_CREATE,             //������������
	MESSAGE_IMAGE_CREATE_SUCCESS,           //���������ɹ�
	MESSAGE_IMAGE_CREATE_FAIL,              //��������ʧ��
	MESSAGE_IMAGE_CREATE_RESET,             //��ɾ������������ѡ��
	MESSAGE_START_MODULE_BURN,              //
	MESSAGE_MODULE_BURN_FAIL,               //
	MESSAGE_BASEBAND_GPIO_SETTING,
	MESSAGE_BASEBAND_GPIO_SETTING_FAIL,
	MESSAGE_SET_PARAM_TO_PRODUCER_FAIL,     //���ò�����producerʧ��
	MESSAGE_DOWNLOAD_PRODUCER_START,        //��ʼ����producer
	MESSAGE_DOWNLOAD_PRODUCER_FAIL,         //����producerʧ��
	MESSAGE_DOWNLOAD_AND_TESTRAM_START,        //��ʼ���ز���RAM
	MESSAGE_DOWNLOAD_AND_TESTRAM_FAIL,         //�������ز���RAMʧ��
	MESSAGE_DOWNLOAD_PRODUCER_USB_FAIL,     //����producer��usb����ʧ��
	MESSAGE_DOWNLOAD_PRODUCER_TIMEOUT_FAIL, //����producer��ʱ
	MESSAGE_DOWNLOAD_PRODUCER_SUCCESS,      //����producer�ɹ�
	MESSAGE_DOWNLOAD_BIN_START,             //��ʼ����bin
	MESSAGE_DOWNLOAD_BIN_FAIL,              //����binʧ��
	MESSAGE_DOWNLOAD_BIN_SUCCESS,           //����bin�ɹ�
	MESSAGE_DOWNLOAD_IMG_START,             //��ʼ����img
	MESSAGE_DOWNLOAD_IMG_FAIL,              //����imgʧ��
	MESSAGE_DOWNLOAD_IMG_SUCCESS,           //����img�ɹ�
	MESSAGE_DOWNLOAD_BOOT_START,            //��ʼ����BOOT
	MESSAGE_DOWNLOAD_BOOT_FAIL,             //����BOOTʧ��
	MESSAGE_DOWNLOAD_BOOT_SUCCESS,          //����BOOT�ɹ�
	MESSAGE_PARTTION_INFORMATION_IS_NULL,   //������ϢΪ��
	MESSAGE_GET_MEDIUM_DATAINFO_FAIL,       //��ȡý��������Ϣʧ��
	MESSAGE_GET_FREE_BLOCK_FAIL,            //��ȡ���п���Ϣʧ��
	MESSAGE_LOW_FORMAT_START,				//��ʼ�͸�
	MESSAGE_LOW_FORMAT_FAIL,				//�͸�ʧ��
	MESSAGE_MALLOC_MEDIUM_FAIL,				//����ý��ʧ��

	MESSAGE_WRITE_MAC_ADDR_ASA_START,       //��ʼдmac��ַ�밲ȫ��
	MESSAGE_WRITE_MAC_ADDR_ASA_FAIL,        //дmac��ַ�밲ȫ��ʧ��
	MESSAGE_WRITE_SERIAL_ADDR_ASA_START,    //��ʼд���кŵ�ַ�밲ȫ��
	MESSAGE_WRITE_SERIAL_ADDR_ASA_FAIL,     //д���кŵ�ַ�밲ȫ��ʧ��


    MESSAGE_SET_MODE_START,                 //������¼ģʽ
	MESSAGE_SET_MODE_FAIL,                  //������¼ģʽʧ��
	MESSAGE_SET_ERASEMODE_START,            //���ò���ģʽ
	MESSAGE_SET_ERASEMODE_FAIL,             //���ò���ģʽʧ��
	MESSAGE_SET_NAND_GPIOCE_FAIL,           //����nand gpioce ʧ��
	MESSAGE_SET_NAND_PARA_START,            //����nand����
	MESSAGE_SET_NAND_PARA_FAIL,             //����nand����ʧ��
	MESSAGE_SET_SEC_AREA_START,             //���ð�ȫ��
	MESSAGE_SET_SEC_AREA_FAIL,              //���ð�ȫ��ʧ��
	MESSAGE_BIN_UPLOAD_FAIL,                //bin�ض�ʧ��
	MESSAGE_GET_BAD_BLOCK_FAIL,             //��ȡ������Ϣʧ��

	MESSAGE_BURN_MAC_ADDR_READ,              //����ȫ���е�mac��ַ
	MESSAGE_BURN_MAC_ADDR_READ_ERROR,        //����ȫ���е�mac��ַ����Ч��
	MESSAGE_BURN_SERIAL_ADDR_READ,           //����ȫ���е����кŵ�ַ
	MESSAGE_BURN_SERIAL_ADDR_READ_ERROR,     //����ȫ���е����кŵ�ַ����Ч��

	MESSAGE_BURN_MAC_ADDR_COMPARE,          //��¼mac��ַ�Ƚ�
	MESSAGE_BURN_MAC_ADDR_FAIL,             //��¼mac��ַ�������ֵ
	MESSAGE_BURN_MAC_ADDR_SHOW,             //��ʾ��ǰ��¼��mac��ַ	
	MESSAGE_BURN_SERIAL_ADDR_COMPARE,       //��¼���кŵ�ַ�Ƚ�
	MESSAGE_BURN_SERIAL_ADDR_FAIL,          //��¼���кŵ�ַ�������ֵ
	MESSAGE_BURN_SERIAL_ADDR_SHOW,          //��ʾ��ǰ��¼�����кŵ�ַ
	
	MESSAGE_SET_RESV_AREA_START,            //���ñ�����
	MESSAGE_SET_RESV_AREA_FAIL,             //���ñ�����ʧ��
	MESSAGE_CREATE_PARTITION_START,         //����������
	MESSAGE_CREATE_PARTITION_FAIL,          //����������ʧ��
	
	MESSAGE_DOWNLOAD_CHANGE_CLK_START,      //���ر�ƵС����
	MESSAGE_DOWNLOAD_CHANGE_CLK_FAIL,       //���ر�ƵС����ʧ��
	MESSAGE_DOWNLOAD_CHANGECLK_TIMEOUT_FAIL,    //���ر�ƵС����ʱ
	MESSAGE_DOWNLOAD_CHANGE_CLK_USB_FAIL,   //���ر�ƵС����usb����ʧ��
	MESSAGE_CLOSE_START,                    //��ֹ��¼�߳�
	MESSAGE_CLOSE_FAIL,                     //��ֹ��¼�߳�ʧ��


	MESSAGE_SEND_CMD_ANYKA,                 //����anyka���С���ˣ�����MASS boot
	MESSAGE_GET_HIGHID_FAIL,				//��ȡhighIDʧ��
	MESSAGE_GET_ALL_FREE_BLOCK,             //ͳ�����Կ��п�
	MESSAGE_MEDIUM_CAPACITY_FAIL,           //ÿ��nand��sd��������һ��
	MMESSAGE_MEDIUM_CAPACITY_CHECK,         //���ÿ��nand��sd������
	MESSAGE_UPLAOD_SPIFLASH_FAIL,           //spiflash�ض�ʧ��
	MESSAGE_UPLOAD_SPIFLASH_START,          //���ڻض�spiflash������

}MESSAGE_TYPE;


typedef enum
{
	E_IMG_INIT,
	E_IMG_CREATING,
	E_IMG_SUCCESS,
	E_IMG_FAIL
}E_IMAGE_STATUS;

#endif