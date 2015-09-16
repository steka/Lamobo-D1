/***
*Burn.cpp - This is the cpp file for Burn.
*
*       Copyright (c) 2000-2007, Anyka (GuangZhou) Software Technology Co., Ltd. All rights reserved.
*
*Purpose:
*       define the CBurnBase class and Subclass.
*
* @author	Anyka
* @date		2007-10
* @version	2.0
*
*******************************************************************************/

#include "StdAfx.h"
#include "BurnTool.h"
#include "Burn.h"
#include "Config.h"
#include "MainFrm.h"
#include "logFile.h"
#include "AKFS.h"
#include <stdio.h>
#include <sys/timeb.h>
#include <time.h>


extern "C"
{
#include "fsa.h"
//#include "fha_test.h"
#include "fha.h"
}

extern UINT m_worknum;   //u����¼ʱ�����̵߳ĸ���
extern UINT g_workTotalnum; //������¼ʱ��ȡ�����̵߳ĸ���
extern UINT g_workThreadnum;//
extern CConfig theConfig;   //config
extern CBurnToolApp theApp;  //mainfrm

extern UINT g_download_nand_count; // ����nand������ļ�����
extern T_DOWNLOAD_NAND g_download_nand[];//nand����Ľṹ��

extern UINT g_download_udisk_count;// ����u�̵��ļ�����
extern T_DOWNLOAD_UDISK g_download_udisk[];

extern UINT g_download_mtd_count;// ���ؾ�����ļ�����
extern T_DOWNLOAD_MTD g_download_mtd[];
extern UINT g_timer_counter;    //��ʱ
extern BOOL g_bEraseMode;   //��ģʽ
//UINT g_udisk_burn_index = 0;
//BOOL g_udisk_flag = FALSE;

//extern HANDLE image_event[MAX_DEVICE_NUM];
extern HANDLE image_event;    //���������¼�
extern HANDLE udiskburn_event;   //u����¼���¼�
extern HANDLE capacity_event;   //���������С���¼�
extern HANDLE ResetDevice_event;  //�����¼�
//extern UINT g_img_stat[MAX_DEVICE_NUM];
extern UINT g_img_stat;    //�����״̬
extern UINT g_disk_count;    //�����ĸ���
extern BOOL m_budisk_burn;   //���ƶ���¼��U����¼
extern BOOL  USB_attachflag; //����ʼ����usbʱ����ô��ʱ�Ͳ��ٽ��в���usb�豸��ֱ����¼���
//extern T_DISK_INFO g_disk_info[16];
extern T_nID_DISK_INFO g_nID_disk_info[MAX_DEVICE_NUM];  //������Ϣ
extern char g_download_mtd_flag[MAX_DEVICE_NUM];
extern CHAR burn_detel_usb_flag[MAX_DEVICE_NUM]; //
UCHAR *m_pBuf_BadBlk[MAX_DEVICE_NUM] = {0};  //����
UCHAR *m_pBuf_freeBlk[MAX_DEVICE_NUM] = {0}; //���п�
//UINT badbufnum = 0;
extern volatile UINT g_udisk_burnnum;  //����usb�豸�ĸ���
extern BOOL  m_budisk_getUSBnum; //ֻ�е���U����¼��ʽ�²���ture
extern BOOL g_bUploadbinMode;    //bin�ض���־
extern g_bUpload_spialldata;
BOOL g_capacity_flag = AK_TRUE;    //nand �� sd���������Ƿ����
UINT g_capacity_size[MAX_DEVICE_NUM] = {0};  //��nand �� sd��������
UINT g_capacity_burnnum = 0;
extern HANDLE g_handle;   //�ź���

#define ERASE_NAND_MODE 1

#define OOB_READTURE 1    //oob ���ɹ�
#define OOB_READFAIL 0    //oob ��ʧ��   

#define FHA_SUCCESS 1
#define FHA_FAIL    0

#define  passY               _T("Y")    //mac�����кųɹ��ı���
#define  passN               _T("N")    //mac�����кųɹ��ı���
#define  COMMODE_LEN         20

#define BLOCK_PAGE0_FLAG_INVALID 0x00   //������
#define BLOCK_PAGE0_FLAG_VALID   0xFF   //����
#define MTD_OOB_LEN              8      //oob�ĳ���
#define USB_ONE_BUF_SIZE    8192        //usb��buf����
//#define DELAY_UDSIK_BURN_TIMER    150

#define IMG_BUF_SIZE_NAND    (128 * 1024)   //���ؾ���ʱnand�Ĵ�С
#define IMG_BUF_SIZE_SD    (64 * 1024)      //���ؾ����sd��С

//oob�����ݣ�������oob��Чʱʹ��
static m_mtd_oob_invalid[MTD_OOB_LEN]  = {BLOCK_PAGE0_FLAG_INVALID};

//�����ļ�����ʱʹ��
static PTCHAR m_ImgTable[] =
{
    _T("512"),
    _T("1k"),
    _T("2k"),
    _T("4k"),
    _T("8k"),
};

typedef struct
{
	T_PMEDIUM medium;
	T_U32 ID;
}T_PC_MEDIUM;

//sd���Ľṹ��
typedef struct
{
    UINT total_block;
    UINT block_size;
}T_EMMC_INFO;

//�����Ǻ���������

//��ȡmac��ַ��buf
BOOL Get_Mac_Addr(TCHAR *buf, UINT channelID);
//��ȡ���кŵ�buf
BOOL Get_serial_Addr(TCHAR *buf, UINT channelID);
//дmac�����кŵ��ļ���
BOOL write_config(LPCTSTR file_path, TCHAR *pBufHigh, TCHAR *pBufLow);
//��¼ʧ�ܺ󣬼�¼mac�����к�
void Burn_Fail(TCHAR *serialtempbuf, TCHAR *mactempbuf, UINT nID, BOOL readSerialflag, BOOL readmacflag);
//�ж�mac�Ƿ����
BOOL Macaddr_isunuse(char *buf, int len);
//�ж����к��Ƿ����
BOOL Serialaddr_isunuse(char *buf, int len);
//���÷�����Ϣ
void SetPartInfo(UINT nID, T_DRIVER_INFO *pPartInfo, UINT num, UINT SectorPerPage, UINT SectorSize, UINT nDISKB_Enlarge);
//��¼u���ļ����ڵķ���
void Save_udiskfile_drivernum(void);
//����nand�Ļص�
VOID Burn_ResetNandBase(UINT nID, T_PNANDFLASH nand);
//��ȡ���鵽BUF
T_BOOL fNand_Get_BadBlockBuf(UINT nID, T_U32 BlockNum, T_U32 nChipCnt, T_MODE_CONTROL *ModeCtrl);
//�ͷ����п��п�
void fNand_Free_BadBlockBuf(UINT nID);
//��ȡ���п��п�
T_BOOL fNand_Get_FreeBlockBuf(UINT nID, T_U32 chip, T_U32 BlockNum, T_U32 nChipCnt, UINT StartBlock);
//�������bin��λ��
T_BOOL browser_for_binUpload(TCHAR *folderPath);
//����sd��
VOID Burn_ResetMedium(UINT nID, T_PMEDIUM medium);
//�ͷ�sd���ʵ��ڴ�
T_VOID Burn_Free_Medium(T_PMEDIUM medium);
//����sd���ʵ��ڴ�
T_PMEDIUM Burn_Malloc_Medium(UINT ID, UINT secsize, UINT capacity, UINT SecPerPg);
//��ȡ�����·��
BOOL GetDownloadImgFromPCPath(PTCHAR pDst, PTCHAR pSrc, USHORT  page_size);
//��������
BOOL Burn_CreatePartion(UINT nID, HWND hWnd, PTCHAR file_name, T_PARTION_INFO *partInfo);
//����spi����
BOOL Burn_CreatePartion_SpiFlash(UINT nID, HWND hWnd, PTCHAR file_name);
//�����ļ���u����
BOOL Burn_DownloadFile(UINT nID, HWND hWnd, PTCHAR file_name);
BOOL Burn_Get_spiAlldata(UINT nID, HWND hWnd, PTCHAR file_name, UINT spiflash_len);

//�����ݹ�����ʱû��ʹ��
UINT GetDISKBSizeEnlarge()
{
	UINT diskB_enlarge = 0;

	/*��չ���ݵĺ����� ��������ʹ��*/

	return diskB_enlarge;
}

//�ж��Ƿ�ȫ0ֵ
static BOOL is_zero_ether_addr(TCHAR *addr)
{
	BOOL flag = FALSE;
	
	//0��ʾ48
	if ((addr[0] == 48 && addr[1] == 48 && addr[3] == 48 
		&& addr[4] == 48 && addr[6] == 48 && addr[7] == 48))
	{
		flag = TRUE;
	}
	
	return flag;
	//return !(addr[0] | addr[1] | addr[3] | addr[4] | addr[6] | addr[7]);
}

//�Ӱ�ȫ����ȡMAC��ַ
BOOL Burn_GetMACInfo(UINT nID, HWND hWnd, PTCHAR file_name, 
					 PTCHAR maclowbuf, PTCHAR mactempbuf, BOOL *readmacflag, UINT  *macmode)
{
	BOOL macaddrflag = TRUE;
	UINT maclen = MAX_MAC_SEQU_ADDR_COUNT;
	TCHAR tempbuf_mac[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};
    TCHAR MacBuf[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};

	USES_CONVERSION;

	//���39оƬ�ϵ�spi��¼�����������¼ʱ��Ҳ���Խ�������mac��ַ�����к�
	//��Ϊspi��mac�����к�����bin�ļ���ʽ��ŵģ�����ֻ�������ݡ�
	if(theConfig.burn_mode != E_CONFIG_SFLASH)//��spi��¼����������֧��mac�����к�
	{
		if(theConfig.bUpdate)
		{
			return TRUE;
		}
	}

	//�ж��Ƿ���¼MAC��ַ
	if(theConfig.macaddr_flag) 
	{
		BYTE buf[MAX_MAC_SEQU_ADDR_COUNT*2+1] = {0};
		CLogFile  burnFile(file_name);
		
		//�ж��Ƿ�ǿ����¼MAC��ַ
		if (theConfig.fore_write_mac_addr)
		{
			*macmode = ASA_MODE_CREATE;

			//��ȡmac��ַ
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_BURN_MAC_ADDR_COMPARE));
			macaddrflag = Get_Mac_Addr(maclowbuf,  nID);
		}
		else
		{
			*macmode = ASA_MODE_OPEN;
			//��asa����mac��ַ
			//if����ǿ��д��
			burnFile.WriteLogFile(LOG_LINE_TIME,  "++read asa file++\r\n" );
			if (BT_ReadASAFile(nID, "MACADDR", buf, &maclen)  == BT_SUCCESS)
			{
				*readmacflag = TRUE;
				
				//�ж϶�������mac��ַ�Ƿ���Ч
				PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_BURN_MAC_ADDR_READ));
				if (!Macaddr_isunuse((char *)buf, maclen))
				{
					burnFile.WriteLogFile(0, "read MAC ADDR is error from asafile!\r\n");
					PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_BURN_MAC_ADDR_READ_ERROR));
					return FALSE;
				}

				memset(tempbuf_mac, 0, MAX_MAC_SEQU_ADDR_COUNT+1);
				theConfig.lower_to_upper(A2T((char *)buf), tempbuf_mac);
				_tcscpy(MacBuf, tempbuf_mac);
				
			}
			else
			{
				*readmacflag = FALSE;
				//���»�ȡmac��ַ
				PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_BURN_MAC_ADDR_COMPARE));
				macaddrflag = Get_Mac_Addr(maclowbuf,  nID);
			}
		}

		if (!macaddrflag)
		{
			//�ж�mac��ַ�Ƿ�������ֵ
			burnFile.WriteLogFile(0, "MAC ADDR beyond the most addr!\r\n");
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_BURN_MAC_ADDR_FAIL));
			return FALSE;
		}

		//�ѻ�ȡ���ĵ�ַ�ŵ�һ����ʱ��buf��
		if (*readmacflag)
		{
			_tcscpy(mactempbuf, MacBuf);
		} 
		else
		{
			_tcscpy(mactempbuf, theConfig.mac_current_high);
			_tcscat(mactempbuf, _T(":"));
			_tcscat(mactempbuf, maclowbuf);
			//memcpy(mactempbuf, buf, MAX_MAC_SEQU_ADDR_COUNT);
		}
		_tcscpy(maclowbuf, &mactempbuf[9]); //��ʾ��8λ��ֵ

		//��ʾ��ǰͨ����mac��ַ
		_tcscpy(theConfig.g_mac_show_current_low[nID-1], maclowbuf);
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_BURN_MAC_ADDR_SHOW));
		
	}

	return TRUE;
}
//�Ӱ�ȫ����ȡ���к�
BOOL Burn_GetSERIALInfo(UINT nID, HWND hWnd, PTCHAR file_name, 
					 PTCHAR seriallowbuf, PTCHAR serialtempbuf, BOOL *readSerialflag, UINT  *Serialmode)
{
	BOOL serialaddrflag = TRUE;
	UINT Seriallen = MAX_MAC_SEQU_ADDR_COUNT;
	TCHAR tempbuf_serial[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};
    TCHAR SerialBuf[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};

	USES_CONVERSION;

	//���39оƬ�ϵ�spi��¼�����������¼ʱ��Ҳ���Խ�������mac��ַ�����к�
	//��Ϊspi��mac�����к�����bin�ļ���ʽ��ŵģ�����ֻ�������ݡ�
	if(theConfig.burn_mode != E_CONFIG_SFLASH)//��spi��¼����������֧��mac�����к�
	{
		if(theConfig.bUpdate)
		{
			return TRUE;
		}
	}

	if(theConfig.sequenceaddr_flag) 
	{
		BYTE buf[MAX_MAC_SEQU_ADDR_COUNT*2+1] = {0};
		CLogFile  burnFile(file_name);

		//�ж��Ƿ�ǿ����¼���к�
		if (theConfig.fore_write_serial_addr)
		{
			*Serialmode = TRUE;
			//��ȡ���кŵ�ַ
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_BURN_SERIAL_ADDR_COMPARE));
			serialaddrflag = Get_serial_Addr(seriallowbuf,  nID);
		}
		else
		{
			*Serialmode = FALSE;
			//if����ǿ��д��
			burnFile.WriteLogFile(LOG_LINE_TIME,  "++read asa file++\r\n" );
			if (BT_ReadASAFile(nID, "SERADDR", buf, &Seriallen)  == BT_SUCCESS)
			{
				*readSerialflag = TRUE;

				//�ж϶����������кŵ�ַ�Ƿ���Ч
				PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_BURN_SERIAL_ADDR_READ));
				if (!Serialaddr_isunuse((char *)buf, Seriallen))
				{
					burnFile.WriteLogFile(0, "read serial ADDR is error from asafile!\r\n");
					PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_BURN_SERIAL_ADDR_READ_ERROR));
					return FALSE;
				}

				memset(tempbuf_serial, 0, MAX_MAC_SEQU_ADDR_COUNT+1);
				theConfig.lower_to_upper(A2T((char *)buf), tempbuf_serial);
				_tcscpy(SerialBuf, tempbuf_serial);
				
				
			}
			else
			{
				PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_BURN_SERIAL_ADDR_COMPARE));
				*readSerialflag = FALSE;	
				//��ȡ���кŵ�ַ
				serialaddrflag = Get_serial_Addr(seriallowbuf,  nID);
			}
		}
		
		
		if (!serialaddrflag)
		{	
			//�ж����кŵ�ֵ�Ƿ�������ֵ
			burnFile.WriteLogFile(0, "serial ADDR beyond the most addr!\r\n");
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_BURN_SERIAL_ADDR_FAIL));
			return FALSE;
		}
		
		//������ɹ��ˣ���ô���ü�¼�����������к�
		if (*readSerialflag)
		{
			_tcscpy(serialtempbuf, SerialBuf);
		}
		else
		{
			//�����¼��ǰ�����к�
			_tcscpy(serialtempbuf, theConfig.sequence_current_high);
			_tcscat(serialtempbuf, seriallowbuf);
		}
		_tcscpy(seriallowbuf, &serialtempbuf[10]); //��ʾ��6λ��ֵ

		//��ʾ��ǰͨ�������кŵ�ַ
		_tcscpy(theConfig.g_sequence_show_current_low[nID-1], seriallowbuf);
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_BURN_SERIAL_ADDR_SHOW));
	}
	
	return TRUE;
}

//дmac��ַ����ȫ��
BOOL Burn_WriteMACInfo(UINT nID, HWND hWnd, PTCHAR file_name, PTCHAR maclowbuf, PTCHAR mactempbuf, 
					   BOOL readmacflag, UINT  macmode)
{
	USES_CONVERSION;
	/***********************************************/

	//���39оƬ�ϵ�spi��¼�����������¼ʱ��Ҳ���Խ�������mac��ַ�����к�
	//��Ϊspi��mac�����к�����bin�ļ���ʽ��ŵģ�����ֻ�������ݡ�
	if(theConfig.burn_mode != E_CONFIG_SFLASH)//��spi��¼����������֧��mac�����к�
	{
		if(theConfig.bUpdate)
		{
			return TRUE;
		}
	}

	//�ж��Ƿ���¼MAC��ַ
	if(theConfig.macaddr_flag) 
	{
		TCHAR buf[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};
		BYTE tbuf[MAX_MAC_SEQU_ADDR_COUNT*2+1];
		UINT *ptr = (UINT *)tbuf; 
		CLogFile  burnFile(file_name);
		
		_tcscpy(buf, mactempbuf);
		ptr[0] = wcslen(buf);
		memcpy(&tbuf[4], T2A(buf), 56);
		
		
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_WRITE_MAC_ADDR_ASA_START));
		burnFile.WriteLogFile(LOG_LINE_TIME, "++write mac add into asa file++\r\n");
		
		//дmac��ַ����ȫ����
		if (BT_WriteASAFile(nID, "MACADDR", tbuf, 30, macmode) != BT_SUCCESS)
		{
			//Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
			burnFile.WriteLogFile(LOG_LINE_TIME, "->fail!\r\n");
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_WRITE_MAC_ADDR_ASA_FAIL));
			return FALSE;
		}
		//����Ƕ�ʧ�ܵĻ����Ͱѵ�ǰ��MAC��ַд���ĵ�������Ͳ�д
		if (!readmacflag)
		{
			theConfig.write_config_addr(CONFIG_ADDR_FILE_NAME, maccurrentlow, passY, maclowbuf, nID-1);
		}
	}
	
	return TRUE;
}
//д���кŵ���ȫ��
BOOL Burn_WriteSERIALInfo(UINT nID, HWND hWnd, PTCHAR file_name, PTCHAR seriallowbuf, PTCHAR serialtempbuf, 
						  BOOL readSerialflag, UINT  Serialmode)
{
	USES_CONVERSION;

	//���39оƬ�ϵ�spi��¼�����������¼ʱ��Ҳ���Խ�������mac��ַ�����к�
	//��Ϊspi��mac�����к�����bin�ļ���ʽ��ŵģ�����ֻ�������ݡ�
	if(theConfig.burn_mode != E_CONFIG_SFLASH)//��spi��¼����������֧��mac�����к�
	{
		if(theConfig.bUpdate)
		{
			return TRUE;
		}
	}

	if(theConfig.sequenceaddr_flag) 
	{
		TCHAR buf[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};
		BYTE tbuf[MAX_MAC_SEQU_ADDR_COUNT*2+1];
		UINT *ptr = (UINT *)tbuf; 
		CLogFile  burnFile(file_name);
		
		_tcscpy(buf, serialtempbuf);
		ptr[0] = wcslen(buf);
		memcpy(&tbuf[4], T2A(buf), MAX_MAC_SEQU_ADDR_COUNT*2-4);
		
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_WRITE_SERIAL_ADDR_ASA_START));
		burnFile.WriteLogFile(LOG_LINE_TIME, "++write serial add into asa file++\r\n");
		
		if (BT_WriteASAFile(nID, "SERADDR", tbuf, 30, Serialmode) != BT_SUCCESS)
		{
			//Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
			burnFile.WriteLogFile(LOG_LINE_TIME, "->fail!\r\n");
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_WRITE_SERIAL_ADDR_ASA_FAIL));
			return FALSE;
		}
		
		//����Ƕ�ʧ�ܵĻ����Ͱѵ�ǰ�����к�д���ĵ�������Ͳ�д
		if (!readSerialflag)
		{
			theConfig.write_config_addr(CONFIG_ADDR_FILE_NAME, sequencecurrentlow, passY, seriallowbuf, nID-1);
		}
		//MessageBox(NULL, _T("2!"), NULL,MB_OK);
	}
	return TRUE;
}

BOOL Burn_upload_spiflash_Data(UINT nID, HWND hWnd, PTCHAR file_name, T_SFLASH_PHY_INFO *sFlashPhyInfo)
{
	BOOL ret = TRUE;

	//�ض���һ��spi������
	if (g_bUpload_spialldata)
	{
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_UPLOAD_SPIFLASH_START));
		if (BT_SetSPIParam(nID, sFlashPhyInfo) == BT_FAIL)
		{
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_SET_CHIP_PARA_FAIL));
			return FALSE;
		}

		if (!Burn_Get_spiAlldata(nID, hWnd, file_name,  sFlashPhyInfo->total_size))
		{
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_UPLAOD_SPIFLASH_FAIL));
			ret = FALSE;
		}
		else
		{
			ret = TRUE;
		}
		//�ر�
		if (BT_Close(nID) != BT_SUCCESS)
		{
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_CLOSE_FAIL));
			return FALSE;
		}
		
	}
	
	return ret;

}

//spi����¼
BOOL burn_spiflash(UINT nID, HWND hWnd, T_MODE_CONTROL ModeCtrl,  PTCHAR file_name)
{
	T_DOWNLOAD_BIN download_bin;
	UINT i;
	BOOL readmacflag_spi = FALSE;//д��־
	TCHAR mactempbuf_spi[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};//��¼��ʱ
	TCHAR maclowbuf_spi[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};//��¼��λ
	UINT  macmode_spi = 0;//�Ǵ������Ǵ�
	BOOL readSerialflag_spi = FALSE;//д��־
	TCHAR serialtempbuf_spi[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};//��¼��ʱ
	TCHAR seriallowbuf_spi[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};//��¼��λ
	UINT  Serialmode_spi = 0;//�Ǵ������Ǵ�
	BOOL ret = TRUE;

	USES_CONVERSION;

	UINT sflashID, sflashChipCnt;
	PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_GET_CHIP_PARA));
	//ͨ��produce��ȡ��Ӧ��spiflash
	if (BT_GetFlashID(nID, 254, 254, &sflashID, &sflashChipCnt) != BT_SUCCESS)
	{
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_GET_CHIP_PARA_FAIL));
		return FALSE;
	}

	//ͨ������񵽵�spiflash��������¼���������ȡ��Ӧ��spiflash
	T_SFLASH_PHY_INFO_TRANSC sFlashPhyInfo;
	for(i = 0; i < theConfig.spiflash_parameter_count; i++)
	{
		//spi vailed in (bit24-0)
		if((sflashID&0xffffff) == (theConfig.spiflash_parameter[i].chip_id&0xffffff))
		{
			memcpy(&sFlashPhyInfo, &theConfig.spiflash_parameter[i], sizeof(T_SFLASH_PHY_INFO_TRANSC));
			break;
		}
	}
	 //��ȡspiflash
	//������һ��
	if (i == theConfig.spiflash_parameter_count || BT_SetSPIParam(nID, &sFlashPhyInfo) == BT_FAIL)
	{
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_SET_CHIP_PARA_FAIL));
		return FALSE;
	}

	//����ʱ��bin�ض�ʱ����ô��ֱ�ӷ���
	if (g_bUploadbinMode)
	{
		return TRUE;
	}


	//�ж��Ƿ���¼MAC��ַ
    if (!Burn_GetMACInfo(nID, hWnd, file_name, maclowbuf_spi, mactempbuf_spi, &readmacflag_spi, &macmode_spi))
	{
		return FALSE;
	}
	
	//д���к�
	if (!Burn_GetSERIALInfo(nID, hWnd, file_name, seriallowbuf_spi, serialtempbuf_spi, &readSerialflag_spi, &Serialmode_spi))
	{
		Burn_Fail(seriallowbuf_spi, maclowbuf_spi, nID, readSerialflag_spi, readmacflag_spi);
		return FALSE;
	}

	PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_BIN_START));
	for(i = 0; i < g_download_nand_count; i++)
	{
		download_bin.bCompare = g_download_nand[i].bCompare;
		_tcscpy(download_bin.pc_path , theApp.ConvertAbsolutePath(g_download_nand[i].pc_path));
		download_bin.ld_addr = g_download_nand[i].ld_addr;
		memcpy(download_bin.file_name, T2A(g_download_nand[i].file_name), MAX_PATH);
		
		download_bin.bBackup = g_download_nand[i].bBackup;
		download_bin.bUpdateSelf = theConfig.bUpdateself;

		//����ÿһ��bin�ļ������չ��С
		if (g_download_nand[i].bin_revs_size != 0)
		{
			UINT bin_len = 0;

			if (theConfig.planform_tpye == E_LINUX_PLANFORM)
			{
				bin_len = (UINT)(g_download_nand[i].bin_revs_size*1024*1024 + 1023)/1024;//��KΪ��λ����
			}
			else
			{
				bin_len = (UINT)g_download_nand[i].bin_revs_size;
			}

			if (BT_Set_BinResvSize(nID, bin_len) != BT_SUCCESS)
			{
				Burn_Fail(seriallowbuf_spi, maclowbuf_spi, nID, readSerialflag_spi, readmacflag_spi);
				PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_SET_RESV_AREA_FAIL));
				return FALSE;
			}
		}

		
		if (BT_DownloadBin(nID, &download_bin) != BT_SUCCESS)
		{
			Burn_Fail(seriallowbuf_spi, maclowbuf_spi, nID, readSerialflag_spi, readmacflag_spi);
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_BIN_FAIL));
			return FALSE;
		}
	}

	//��¼��д��MAC��ַ
	if (!Burn_WriteMACInfo(nID, hWnd, file_name, maclowbuf_spi, mactempbuf_spi, readmacflag_spi, macmode_spi))
	{
		Burn_Fail(seriallowbuf_spi, maclowbuf_spi, nID, readSerialflag_spi, readmacflag_spi);
		return FALSE;
	}
	
	//��¼���к�
	if (!Burn_WriteSERIALInfo(nID, hWnd, file_name, seriallowbuf_spi, serialtempbuf_spi, readSerialflag_spi, Serialmode_spi))
	{
		Burn_Fail(seriallowbuf_spi, maclowbuf_spi, nID, readSerialflag_spi, readmacflag_spi);
		return FALSE;
	}


	//���������ؾ���ֻ֧��linuxƽ̨
	if ((theConfig.planform_tpye == E_LINUX_PLANFORM) && (theConfig.format_count != 0))
	{
		UINT pagesize = sFlashPhyInfo.page_size;
		T_DOWNLOAD_IMG download_img;

		//��spi���з���
		if (!Burn_CreatePartion_SpiFlash(nID, hWnd, file_name))
		{
			return FALSE;
		}

		//���ؾ����ļ�		
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_IMG_START));
		for(i = 0; i < theConfig.download_mtd_count; i++)
		{
			download_img.bCompare = theConfig.download_mtd_data[i].bCompare;
			_tcscpy(download_img.pc_path, theApp.ConvertAbsolutePath(theConfig.download_mtd_data[i].pc_path));
	
			download_img.driver_name = (char)theConfig.download_mtd_data[i].disk_name[0];
			//linuxƽ̨����Ҫ����ʲôʱ���ؾ��񣬾����ļ���linux�Ǳ߿���
			//if (ModeCtrl.burn_mode != MODE_UPDATE)// || (partInfo[driverNo].bOpenZone == FALSE))
			{
				if (BT_DownloadImg(nID, &download_img) != BT_SUCCESS)
				{
					PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_IMG_FAIL));
					return FALSE;
				}
			}
		}

		if (!Burn_DownloadFile(nID, hWnd, file_name))
		{
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_FILE_FAIL));
			return FALSE;
		}
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_BOOT_START));
		//��������ģʽ��������bin���ļ��ĸ�����0,�Ͳ�����boot
		if (ModeCtrl.burn_mode != MODE_UPDATE && theConfig.download_nand_count != 0)
		{
			if (CHIP_37XX_L == theConfig.chip_type)
			{
				if (BT_DownloadBoot(nID, theApp.ConvertAbsolutePath(theConfig.path_nandboot_new), theConfig.chip_type) != BT_SUCCESS)
				{
					PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_BOOT_FAIL));
					return FALSE;
				}
			}
			else
			{
				if (BT_DownloadBoot(nID, theApp.ConvertAbsolutePath(theConfig.path_nandboot), theConfig.chip_type) != BT_SUCCESS)
				{
					PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_BOOT_FAIL));
					return FALSE;
				}
			}
		}
		
		//�ر�
		if (BT_Close(nID) != BT_SUCCESS)
		{
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_CLOSE_FAIL));
			return FALSE;
		}

		//spi��������
		if (!Burn_upload_spiflash_Data(nID,  hWnd,  file_name, &sFlashPhyInfo))
		{
			return FALSE;
		}

		//��¼���
		return TRUE;
	}

	PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_BOOT_START));

	if (CHIP_37XX_L == theConfig.chip_type)
	{
		if (BT_DownloadBoot(nID, theApp.ConvertAbsolutePath(theConfig.path_nandboot_new), theConfig.chip_type) != BT_SUCCESS)
		{
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_BOOT_FAIL));
			return FALSE;
		}
	}
	else
	{
		if (BT_DownloadBoot(nID, theApp.ConvertAbsolutePath(theConfig.path_nandboot), theConfig.chip_type) != BT_SUCCESS)
		{
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_BOOT_FAIL));
			return FALSE;
		}
	}

	if (BT_Close(nID) != BT_SUCCESS)
	{
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_CLOSE_FAIL));
		return FALSE;
	}

	//spi��������
	if (!Burn_upload_spiflash_Data(nID,  hWnd,  file_name, &sFlashPhyInfo))
	{
		return FALSE;
	}

	return TRUE;

}

//�Ѵ�дת����Сд
void MakeLower(LPTSTR str)
{
	TCHAR *pStr;
	
	if(!str || !str[0])
	{
		return;
	}

	pStr = str;
	while(*pStr++)
	{
		if(*pStr >= 'A' && *pStr <= 'Z')
		{
			*pStr += 'a' - 'A';
		}
	}
}

/*
static BOOL CheckImgFileName(PTCHAR str)
{
    int i, j;

    j = sizeof(m_ImgTable) / sizeof(PTCHAR);
    
    for (i=0; i<j; i++)
    {
        if (_tcsstr(str, m_ImgTable[i]) != NULL)
        {
            return FALSE;
        }
    }
    
    return TRUE;
}
*/

//���ļ����в��Ҿ����ļ�
BOOL FindImgFileInDir(LPTSTR pathImg, LPTSTR pathPC, int ImgID)
{
	WIN32_FIND_DATA fd;
	HANDLE hSearch;
	TCHAR searchPath[MAX_PATH+1];
    TCHAR tmpPCPath[MAX_PATH+1];
    int tabID = sizeof(m_ImgTable) / sizeof(PTCHAR);

    if (pathImg == NULL || pathPC == NULL || ImgID >= tabID)
    {
        return FALSE;
    }
    
    _tcsncpy(searchPath, pathPC, MAX_PATH);
	_tcscat(searchPath, _T("\\*"));

	hSearch = FindFirstFile(searchPath, &fd);
	if(INVALID_HANDLE_VALUE == hSearch)
	{
		return FALSE;
	}
	
	USES_CONVERSION;
	
	do
	{
		if((0 != _tcscmp(fd.cFileName, _T("."))) && (0 != _tcscmp(fd.cFileName, _T(".."))))
		{
			if((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
			{
                _tcscpy(tmpPCPath, fd.cFileName);
                MakeLower(tmpPCPath);//��дתСд
                if (_tcsstr(tmpPCPath, m_ImgTable[ImgID]) != NULL)
                {
			        _tcscpy(pathImg, pathPC);
                    _tcscat(pathImg, _T("\\"));
			        _tcscat(pathImg, fd.cFileName);
                    FindClose(hSearch);

                    return TRUE;
                }
			}
		}
	}
	while(FindNextFile(hSearch, &fd));

	FindClose(hSearch);
    
	return FALSE;	
}

//ͨ��pc·�����ؾ����ļ�
BOOL GetDownloadImgFromPCPath(PTCHAR pDst, PTCHAR pSrc, USHORT  page_size)
{
    DWORD dwAttr;
    USHORT SecSize = page_size;
    int SecBit = 0;

    if (page_size % 512)
        return FALSE;

    SecSize >>= 9;

	//��������λ��
    while (SecSize > 1)
    {
        SecBit++;
        SecSize >>= 1;
    }
    
	//��ȡԴ�ļ�������
	dwAttr = GetFileAttributes(pSrc);
	if(0xFFFFFFFF == dwAttr)
	{
		return FALSE;
	}    

	if((dwAttr & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
    {
        //��Ҫ�����ļ����ڲ�����Ӧimg
        if (!FindImgFileInDir(pDst , pSrc, SecBit))
            return FALSE;
    }
    else
    {
		//���ļ���
		_tcscpy(pDst , pSrc);
    }

    return TRUE;
}

//����comAddr���ڴ�
BOOL Download_comAddr_ToRam(UINT nID, HWND hWnd, PTCHAR file_name, UINT com)
{
	//UINT com = 0;
	UINT cmddata = 0x30700000;
	
	CLogFile  burnFile(file_name);

	if (BT_DownloadDataToRam(nID, cmddata,(PBYTE)&com, 4) != BT_SUCCESS)
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "->BT_DownloadDataToRam fail.\r\n" );
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_SET_REGISTER_FAIL));
		return FALSE;
	}
	return TRUE;
}

//��37��98��������һЩֵ���ڴ���
BOOL Download_Channel_Addranddata_ToRam(UINT nID, HWND hWnd, TCHAR *file_name)
{
	UINT channel_addr = 0x80100000; // ��ʼ��
	UINT cmddata      = 0x81a00000; // ��ʼ��

	CLogFile  burnFile(file_name);
	
	PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_START_SET_CHANNELID));
	
	//ak98оƬ
	if(theConfig.chip_type == CHIP_980X)
	{
		burnFile.WriteLogFile(0,  "channel ID=0x80100000\n");
		channel_addr = 0x80100000;
		
	}
	else if(theConfig.chip_type == CHIP_37XX || theConfig.chip_type == CHIP_37XX_L)//ak37оƬ
	{
		burnFile.WriteLogFile(0,  "channel ID=0x30100000\\n");
		channel_addr = 0x30100000;
	}
	else if(theConfig.chip_type == CHIP_39XX)//ak39оƬ
	{
		burnFile.WriteLogFile(0,  "channel ID=0x80100000\\n");
		channel_addr = 0x80100000;
	}
	else
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "->chip_type  error.\r\n");
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_START_CHANNELID_FAIL));
		return FALSE;
	}
	
	//���ص�ַ
	if (BT_DownloadDataToRam(nID, channel_addr, NULL, 0) != BT_SUCCESS)
	{
		burnFile.WriteLogFile(LOG_LINE_TIME,  "->SetRamParam fail.\r\n");
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_SET_REGISTER_FAIL));
		return FALSE;
	}
	
	if (theConfig.cmdLine.bCmdLine)
	{
		burnFile.WriteLogFile(0, "data{%s}\n", theConfig.cmdLine.CmdData );
		if(theConfig.chip_type == CHIP_980X)//ak98оƬ
		{
			burnFile.WriteLogFile(0,  "cmddata =0x81a00100\n");
			cmddata = 0x81a00100;
			
		}
		else if(theConfig.chip_type == CHIP_37XX || theConfig.chip_type == CHIP_37XX_L)//ak37оƬ
		{
			burnFile.WriteLogFile(0,  "cmddata =0x30400100\\n");
			cmddata = 0x30400100;
		}
		else if(theConfig.chip_type == CHIP_39XX)//ak39оƬ
		{
			burnFile.WriteLogFile(0,  "cmddata =0x80100100\\n");
			cmddata = 0x80100100;
		}
		else
		{
			burnFile.WriteLogFile(LOG_LINE_TIME, "->chip_type error1.\r\n");
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_START_CHANNELID_FAIL));
			return FALSE;
		}
		//����ֵ
		if (BT_DownloadDataToRam(nID, cmddata,(PBYTE)theConfig.cmdLine.CmdData, 128) != BT_SUCCESS)
		{
			burnFile.WriteLogFile(LOG_LINE_TIME, "->SetRamParam fail.\r\n" );
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_SET_REGISTER_FAIL));
			return FALSE;
		}
	}
	return TRUE;
}

//��ȡchip�ļ��еĴ�������
UINT Read_commode_fromfile(void)
{
	TCHAR commode[COMMODE_LEN] = {0};
	TCHAR tempfilename[COMMODE_LEN] = {0};
	TCHAR filename[COMMODE_LEN] = {0};
	UINT i = 0;
	UINT com_mode = 0;
	//T_DOWNLOAD_BIN download_bin;
	TCHAR pc_path[MAX_PATH+1] = {0};

	USES_CONVERSION;

	for(i = 0; i < g_download_nand_count; i++)
	{

		//�����Сд��,��ôת��Ϊ��д
        _tcsncpy(tempfilename, g_download_nand[i].file_name, 5);
		theConfig.lower_to_upper(tempfilename, filename);


		if (_tcscmp(A2T("CHIP"), filename) == 0)
		{
			_tcscpy(pc_path , theApp.ConvertAbsolutePath(g_download_nand[i].pc_path));
		}
	}
	
	GetPrivateProfileString(_T("chip"), _T("uart_ID"), NULL, commode, COMMODE_LEN, pc_path);	

	com_mode = atoi(T2A(commode));//���ַ���ת��������

	return com_mode;
}
//���ر�ƵС����
BOOL Burn_DownloadChangClock(UINT nID, HWND hWnd, PTCHAR file_name)
{
	if (CHIP_980X == theConfig.chip_type) 
	{
		CLogFile  burnFile(file_name);

		burnFile.WriteLogFile(0, "SUPPORT_LINUX\r\n");
		if (!theConfig.bUDiskUpdate)
		{
			burnFile.WriteLogFile(0,  "Not Udiskburn!\n" );
			burnFile.WriteLogFile(LOG_LINE_TIME, "++Begin donwload change_clk++\r\n");
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_CHANGE_CLK_START));
			if (BT_DownloadProducer(nID, theApp.ConvertAbsolutePath(_T("CHANG_CLK.bin")), 
				0x48000000) != BT_SUCCESS)
			{
				burnFile.WriteLogFile(0, "->donwload change_clk fail!\r\n");
				PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_CHANGE_CLK_FAIL));
				return FALSE;
			} 
			/*��һ�����ر�ƵС�������Ҫ���¶Ͽ�����usb*/
	//		if (BT_WaitUSBAttach(nID) == BT_FAIL)
	//		{
	//			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_CHANGE_CLK_USB_FAIL));
	//            return FALSE;
	//		}
		}
		else
		{
			burnFile.WriteLogFile(0, "Is Udiskburn!\r\n" );
			burnFile.WriteLogFile(LOG_LINE_TIME, "++USB Attach++\r\n" );
		}
    
		g_workThreadnum++;
		burnFile.WriteLogFile(0, "Work Thread num = %d\n", g_workThreadnum);

		/*�ȴ����е��߳�*/
		burnFile.WriteLogFile(LOG_LINE_TIME, "...waiting...\r\n" );
		g_timer_counter = 0;
		while ((g_workThreadnum < g_workTotalnum) && (g_timer_counter <= 60))
		{
			Sleep(10);
		}

		burnFile.WriteLogFile(LOG_LINE_TIME, "counter:%d\n",g_timer_counter);

		//���ʱ�䳬��1���ӣ���ô�ͳ���
		if (g_timer_counter > 60)
		{
			burnFile.WriteLogFile(LOG_LINE_TIME, "time out!\r\n");
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_CHANGECLK_TIMEOUT_FAIL));
			return FALSE;
		}
		//�˴�sleep����������Ϊ�±�Ƶ����Ҫһ���ȴ�usb���ӵ�ʱ��
		Sleep(500);
	}
	else
	{
		g_workThreadnum++;
	}
	
	return TRUE;
}
//�����ڴ����
BOOL Burn_SetRamParam(UINT nID, HWND hWnd, PTCHAR file_name)
{
    T_RAM_REG ram_reg[64];
    UINT  ram_cnt;
	UINT i;
    
	//����10/11оƬ�ģ�������Ҫ�����ڴ����
	if ((CHIP_11XX != theConfig.chip_type) && (CHIP_10X6 != theConfig.chip_type) 
		&& (CHIP_1080A != theConfig.chip_type) && (CHIP_1080L != theConfig.chip_type)
		 && (CHIP_10XXC != theConfig.chip_type))
	{
		CLogFile  burnFile(file_name);
		burnFile.WriteLogFile(LOG_LINE_TIME, "++SetRamParam++\r\n" );
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_START_SET_RAMPARAM));

		//ͨ���ڴ�������ж�ȡ�ڴ��ַ��ֵ
		ram_cnt = config_ram_param(ram_reg);
		burnFile.WriteLogFile(0, "ram_count = %d\n", ram_cnt);
		for (i=0; i<ram_cnt; i++)
		{
			burnFile.WriteLogFile(0, "ram[%02d]: addr=%08x, value=%08x\n", i, ram_reg[i].reg_addr, ram_reg[i].reg_value);

		}
        //�Ѷ������ڴ�ֵ�͵�ַ����ȥ����д��boot����		
		if (BT_SetRamParam(nID, ram_reg, ram_cnt) != BT_SUCCESS)
		{
			burnFile.WriteLogFile(LOG_LINE_TIME, "->SetRamParam fail.\r\n");
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_SET_RAMPARAM_FAIL));
			return FALSE;
		}	
	}
	return TRUE;
}
//��rtosƽ̨�£���Ҫд�봮������
BOOL Burn_RTOSDownComInfo(UINT nID, HWND hWnd, PTCHAR file_name)
{
	if ((theConfig.planform_tpye == E_ROST_PLANFORM) && ((CHIP_37XX == theConfig.chip_type)  || (CHIP_37XX_L == theConfig.chip_type)))
	{
		CLogFile  burnFile(file_name);

		//���ļ��ж�ȡ��theConfig.com_mode
		burnFile.WriteLogFile(LOG_LINE_TIME,  "++read com mode++\r\n" );

		//��ȡcom
		theConfig.com_mode = Read_commode_fromfile();
		if (theConfig.com_mode != 0 && theConfig.com_mode != 1 )
		{
			burnFile.WriteLogFile(LOG_LINE_TIME, "->Download_comAddr_ToRam fail.\r\n");
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_SET_COMMODE_FAIL));
			return FALSE;
		}
		
		//��COM����д��ram��
		if (FALSE == Download_comAddr_ToRam( nID,  hWnd,  file_name,  theConfig.com_mode))
		{
			burnFile.WriteLogFile(LOG_LINE_TIME, "->Download_comAddr_ToRam fail.\r\n");
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_COMADDR_FAIL));
			return FALSE;
		}
	}	

	return TRUE;
}
//��linuxƽ̨����Ҫ�ȴ�produce��������
BOOL Burn_WaitProducerInitFinish(UINT nID, HWND hWnd, PTCHAR file_name)
{
    //#ifdef SUPPORT_LINUX
	if (theConfig.planform_tpye == E_LINUX_PLANFORM)
	{
		CLogFile  burnFile(file_name);
		/*����producer����Ҫ���¶Ͽ�����usb*/
		burnFile.WriteLogFile(LOG_LINE_TIME, "++Attach USB++\r\n" );
		if (BT_WaitUSBAttach(nID) == BT_FAIL)
		{
			burnFile.WriteLogFile(LOG_LINE_TIME,  "->Attach USB fail.\r\n");
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_PRODUCER_USB_FAIL));
			return FALSE;
		}
		
		g_workThreadnum++;
		/*�ȴ����е��߳�*/
		g_timer_counter = 0;
		while ((g_workThreadnum < (g_workTotalnum *2)) && (g_timer_counter <= 60)) 
		{
			Sleep(10);
		}
		//��60�ͳ���
		if (g_timer_counter > 60)
		{
			burnFile.WriteLogFile(LOG_LINE_TIME,  "time out.\r\n" );
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_PRODUCER_TIMEOUT_FAIL));
			return FALSE;
		}
	}

	Sleep(1500);	

	return TRUE;
}

//��usb1.1ת��Ϊusb2.0
BOOL Burn_SwitchUSB11To20(UINT nID, HWND hWnd, PTCHAR file_name)
{
	//����1080LоƬ�������2.0�����Բ���Ҫ����ת����2.0��
	//����������Ƶ������,10L��10CҲ��Ҫ����usb�л��ͱ�Ƶ
	if ((theConfig.planform_tpye == E_ROST_PLANFORM) && (!g_bEraseMode))
	{
		if (((theConfig.bUsb2) && (CHIP_1080L != theConfig.chip_type) && (CHIP_10XXC != theConfig.chip_type) && (CHIP_37XX_L != theConfig.chip_type))
			|| ((theConfig.bPLL_Frep_change) && ((CHIP_1080L == theConfig.chip_type) || (CHIP_10XXC == theConfig.chip_type) || (CHIP_37XX_L == theConfig.chip_type))))
		{
			CLogFile  burnFile(file_name);

			burnFile.WriteLogFile(LOG_LINE_TIME,  "++Switch USB to high speed, bUsb2: %d, g_bEraseMode: %d++\r\n", theConfig.bUsb2, g_bEraseMode);
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_START_INIT_USB));
			//�˽ӿڲ���Ҫ�ٽ��еȴ�һ��һ����ɣ�����ͬʱִ��
			if (BT_SwitchUSBHighSpeedAsyn(nID) != BT_SUCCESS)
			{
				burnFile.WriteLogFile(LOG_LINE_TIME, "->Switch USB to high speed fail.\r\n" );
				PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_INIT_USB_FAIL));
				return FALSE;
			}
		}
	}

	return TRUE;
}

//����producerg ,���ж�usb���������ӵĲ���
BOOL Burn_TestPCAndProducerConnectOK(UINT nID, HWND hWnd, PTCHAR file_name)
{
	CLogFile  burnFile(file_name);
	burnFile.WriteLogFile(LOG_LINE_TIME, "++test connetion++\r\n" );
    PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_START_TEST_TRANSC));
    //����B��T����produce�Ƿ�����յ�
	if (BT_TestConnection(nID) != BT_SUCCESS)
    {
		burnFile.WriteLogFile(LOG_LINE_TIME, "->connetion fail!\r\n" );
        PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_TEST_TRANSC_FAIL));
        return FALSE;
    }

	return TRUE;
}
//����gpio
BOOL Burn_SetGPIO(UINT nID, HWND hWnd, PTCHAR file_name)
{
    if (theConfig.gpio_pin_select)
    {
        T_GPIO_PARAM GpioParam = {0};
		CLogFile  burnFile(file_name);

        GpioParam.num = theConfig.gpio_pin;
		

        burnFile.WriteLogFile(0,  "GpioParam.num = %d\n", GpioParam.num);
    	PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_BASEBAND_GPIO_SETTING));
        burnFile.WriteLogFile(LOG_LINE_TIME,  "++SetGPIO++\r\n");
        //������Ӧ��gpio
		if (BT_SetGPIO(nID, &GpioParam) == BT_FAIL)
        {
			burnFile.WriteLogFile(LOG_LINE_TIME, "->setGPIO fail!\r\n" );
	    	PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_BASEBAND_GPIO_SETTING_FAIL));
            return FALSE;
        }
    }

	return TRUE;
}

/************************************************************************/
/* ���ò�����¼�е���Ҫ���ƵĶ�����producer                             */
/************************************************************************/
BOOL Burn_SetBurnPara(UINT nID, HWND hWnd, PTCHAR file_name)
{
	if (theConfig.planform_tpye == E_LINUX_PLANFORM)
	{
		CLogFile  burnFile(file_name);

		burnFile.WriteLogFile(LOG_LINE_TIME,  "++Set Burned Param++\r\n");
		burnFile.WriteLogFile(0,  "mode: %d\n",theConfig.burned_param.burned_mode);
		if (BT_SetBurnedParam(nID, &theConfig.burned_param) == BT_FAIL)
		{
			burnFile.WriteLogFile(LOG_LINE_TIME, "->Set Burned Param fail\r\n" );
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_SET_PARAM_TO_PRODUCER_FAIL));
			return FALSE;
		}
	}

	return TRUE;
}

//���ò���ģʽ
BOOL Burn_SetEraseMode(UINT nID, HWND hWnd, PTCHAR file_name, T_MODE_CONTROL ModeCtrl)
{
	UINT erase_mode = ERASE_NAND_MODE;  //��Ϊ��blockģʽ

	CLogFile  burnFile(file_name);
	PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_SET_ERASEMODE_START));
	burnFile.WriteLogFile(LOG_LINE_TIME, "++set erase nand++\r\n" );
	burnFile.WriteLogFile(0, "erase_mode = %d, eMedium = %d\n", erase_mode, ModeCtrl.eMedium );

	if (BT_SetEraseMode(nID, erase_mode) == BT_FAIL)
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "->set erase nand mode fail!\r\n" );
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_SET_ERASEMODE_FAIL));
		return FALSE;
	}
	return TRUE;
}
//������¼ģʽ
BOOL Burn_SetBurnMode(UINT nID, HWND hWnd, PTCHAR file_name, T_MODE_CONTROL ModeCtrl)
{
	CLogFile  burnFile(file_name);
    PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_SET_MODE_START));
    burnFile.WriteLogFile(LOG_LINE_TIME, "++set mode++\r\n" );
    burnFile.WriteLogFile(0, "burn_mode = %d, eMedium = %d\n", ModeCtrl.burn_mode, ModeCtrl.eMedium );
    if (BT_SetMode(nID, ModeCtrl) == BT_FAIL)
    {
		burnFile.WriteLogFile(LOG_LINE_TIME, "->set mode fail!\r\n" );
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_SET_MODE_FAIL));
        return FALSE;
    }

	return TRUE;
}
//ֻ֧��linuxƽ̨
BOOL Burn_SetGPIOToNandChip(UINT nID, HWND hWnd, PTCHAR file_name, T_MODE_CONTROL ModeCtrl)
{
	if (theConfig.planform_tpye == E_LINUX_PLANFORM)
	{
		if (TRANSC_MEDIUM_NAND == ModeCtrl.eMedium || TRANSC_MEDIUM_SPI_NAND == ModeCtrl.eMedium)
		{
			CLogFile  burnFile(file_name);

			burnFile.WriteLogFile(LOG_LINE_TIME, "nandflash chip num=%d, ce2:%d, ce3=%d\n", 
				theConfig.chip_select_ctrl.chip_sel_num,
				theConfig.chip_select_ctrl.gpio_chip_2,
				theConfig.chip_select_ctrl.gpio_chip_3);
			if (BT_FAIL == BT_SetNandGpioCe(nID, &theConfig.chip_select_ctrl))
			{
				PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_SET_NAND_GPIOCE_FAIL));
				return FALSE;
			}
		}
	}

	return TRUE;
}
//ͨ��nand��ID��������NAND�Ĳ���
BOOL Burn_GetFlashIDAndSetNandParam(UINT nID, HWND hWnd, PTCHAR file_name, 
					 UINT *sector_per_page, UINT *SectorSize, T_NAND_PHY_INFO *NandPhyInfo, UINT *flash_ChipCnt)
{
	UINT flashID, flashChipCnt;
	UINT HighID;
	UINT i;
	UINT parameter_count = 0;
	UINT chip_id = 0;
	UINT flag = 0;
	UINT cmd_len = 0;

	CLogFile  burnFile(file_name);
	if (E_CONFIG_NAND == theConfig.burn_mode || E_CONFIG_SPI_NAND == theConfig.burn_mode)
	{
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_GET_CHIP_PARA));
		burnFile.WriteLogFile(LOG_LINE_TIME, "++BT_GetFlashID++\r\n" );
		if (BT_GetFlashID(nID, theConfig.chip_select_ctrl.gpio_chip_2, theConfig.chip_select_ctrl.gpio_chip_3, &flashID, &flashChipCnt) != BT_SUCCESS)
		{
			burnFile.WriteLogFile(LOG_LINE_TIME,  "->GetFlashID fail!\r\n");
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_GET_CHIP_PARA_FAIL));
			return FALSE;
		}

		burnFile.WriteLogFile(0,  "FlashID = 0x%x, chip count = %d\n", flashID,flashChipCnt );
		*flash_ChipCnt = flashChipCnt; //(��λ��chip_cnt, ��λhighID)

		//T_NAND_PHY_INFO NandPhyInfo;
		if (theConfig.burn_mode == E_CONFIG_SPI_NAND)
		{
			parameter_count = theConfig.spi_nandflash_parameter_count;
		}
		else
		{
			parameter_count = theConfig.nandflash_parameter_count;
		}

		for(i = 0; i < parameter_count; i++)
		{
			if (theConfig.burn_mode == E_CONFIG_SPI_NAND)
			{
				chip_id = theConfig.spi_nandflash_parameter[i].chip_id;
			}
			else
			{
				chip_id = theConfig.nandflash_parameter[i].chip_id;
				flag = theConfig.nandflash_parameter[i].flag;
				cmd_len = theConfig.nandflash_parameter[i].cmd_len;
			}

			if(flashID == chip_id)
			{
				if ((theConfig.burn_mode != E_CONFIG_SPI_NAND && flag & (1 << 25)) != 0)
				{
					if (BT_GetFlash_HighID(nID, theConfig.chip_select_ctrl.gpio_chip_2, theConfig.chip_select_ctrl.gpio_chip_3, &HighID) != BT_SUCCESS)
					{
						burnFile.WriteLogFile(LOG_LINE_TIME,  "->BT_GetFlash_HighID fail!\r\n");
						PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_GET_HIGHID_FAIL));
						return FALSE;
					}
					

					//�Ƚ�highID�Ƿ���ͬ
					if((HighID & 0xFFFF) != (cmd_len & 0xFFFF))
					{
						continue;
					}
				}

				if (theConfig.burn_mode == E_CONFIG_SPI_NAND)
				{
					memcpy(NandPhyInfo, &theConfig.spi_nandflash_parameter[i], sizeof(T_NAND_PHY_INFO));
				} 
				else
				{
					memcpy(NandPhyInfo, &theConfig.nandflash_parameter[i], sizeof(T_NAND_PHY_INFO));
				}
				
				if ((theConfig.planform_tpye == E_ROST_PLANFORM) && (nID == 1) && (theConfig.bonline_make_image == TRUE))
				{
					//�������4K��ҳ����ôҳ��С��4K����
					if(NandPhyInfo->page_size > 4096)
					{
						*SectorSize = 4096;
					}
					else
					{
						//����ҳ��С����
						*SectorSize = NandPhyInfo->page_size;
					}
				}
				else if ((theConfig.planform_tpye == E_ROST_PLANFORM) && (theConfig.chip_type == CHIP_1080L || theConfig.chip_type == CHIP_10XXC))
				{
					//10ƽ̨���ǰ�512�ֽ�����
					*SectorSize = 512;
				}
				//ÿһ��ҳ�ж��ٸ�����
				*sector_per_page = NandPhyInfo->page_size / *SectorSize;
				break;
			}

		}
		
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_SET_NAND_PARA_START));

		burnFile.WriteLogFile(LOG_LINE_TIME,   "++set nand para++\r\n");
		burnFile.WriteLogFile(0, "page_size = %d, spare size = %d, data_len = 0x%x, cmd_len = %d\n", 
			NandPhyInfo->page_size, NandPhyInfo->spare_size,
			NandPhyInfo->data_len, NandPhyInfo->cmd_len);

		//��ȡ����nandID����produce��
		if (i == parameter_count || BT_SetNandParam(nID, NandPhyInfo) == BT_FAIL)
		{
			burnFile.WriteLogFile(LOG_LINE_TIME, "->set nand para fail\r\n" );
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_SET_NAND_PARA_FAIL));
			return FALSE;
		}
	}


	return TRUE;
}
//���÷��ļ�ϵͳ��������С
BOOL Burn_SetResvAreaSize(UINT nID, HWND hWnd, PTCHAR file_name)
{
	CLogFile  burnFile(file_name);
	PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_SET_RESV_AREA_START));
    burnFile.WriteLogFile(LOG_LINE_TIME,  "++set resv area++\r\n" );
    burnFile.WriteLogFile(0,  "nonfs_res_size = %d\n", theConfig.nonfs_res_size);
    //��λ��M
    if (BT_SetResvAreaSize(nID, theConfig.nonfs_res_size, TRUE) != BT_SUCCESS)
	{
        //Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
		burnFile.WriteLogFile(LOG_LINE_TIME, "->fail!\r\n" );
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_SET_RESV_AREA_FAIL));
		return FALSE;
	}

	return TRUE;
}

//����BIN�ļ���������
BOOL Burn_DownloadBin(UINT nID, HWND hWnd, PTCHAR file_name)
{
	UINT i;
	T_DOWNLOAD_BIN download_bin;
	CLogFile  burnFile(file_name);
	UINT bin_len = 0;
	
	USES_CONVERSION;
	PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_BIN_START));
    burnFile.WriteLogFile(LOG_LINE_TIME, "++download bin++\r\n" );
    burnFile.WriteLogFile(0,  "download_nand_count = %d\n", g_download_nand_count);
     
    for(i = 0; i < g_download_nand_count; i++)
	{
		download_bin.bCompare = g_download_nand[i].bCompare;  //�Ƿ�Ƚ�
		_tcscpy(download_bin.pc_path , theApp.ConvertAbsolutePath(g_download_nand[i].pc_path));  //pc�ϵ�·��
		download_bin.ld_addr = g_download_nand[i].ld_addr; //���ӵ�ַ
		memcpy(download_bin.file_name, T2A(g_download_nand[i].file_name), MAX_PATH);  //bin�ļ���
		
		download_bin.bBackup = g_download_nand[i].bBackup; //����
		download_bin.bUpdateSelf = theConfig.bUpdateself;  //������

		if (theConfig.planform_tpye == E_LINUX_PLANFORM)
		{
			bin_len = (UINT)(g_download_nand[i].bin_revs_size*1024*1024 + 1023)/1024;//��KΪ��λ����
		}
		else
		{
			bin_len = (UINT)g_download_nand[i].bin_revs_size;
		}

		//����ÿһ��bin�ļ������չ��С
		if (BT_Set_BinResvSize(nID, bin_len) != BT_SUCCESS)
		{
			//Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
			burnFile.WriteLogFile(LOG_LINE_TIME, "->fail!\r\n" );
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_SET_RESV_AREA_FAIL));
			return FALSE;
		}
		
		if (BT_DownloadBin(nID, &download_bin) != BT_SUCCESS)
		{
           //Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
			burnFile.WriteLogFile(LOG_LINE_TIME,  "->fail!\r\n");
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_BIN_FAIL));
			return FALSE;
		}
	}

	return TRUE;
}
//�͸�SD�ķ���
BOOL Burn_LowFormat_SD(UINT nID, HWND hWnd, PTCHAR file_name, T_PARTION_INFO *partInfo, 
					T_PMEDIUM medium, UINT StartBlock, T_MODE_CONTROL ModeCtrl, UINT *StartID, UINT *IDCnt)
{
	CAKFS cAK;
	CLogFile  burnFile(file_name);

	if (nID != 1) 
	{
		/**����ȴ���һͨ������������ok����ܼ���**/
		while (theConfig.bonline_make_image)
		{
			Sleep(10);
		}
	}

	memcpy(partInfo, theConfig.format_data, sizeof(T_PARTION_INFO)*theConfig.format_count);
	
    /*
    //����ֻ�������һ������������
    if(MODE_NEWBURN == ModeCtrl.burn_mode)
    {
        partInfo[theConfig.format_count - 1].EnlargeSize = 8 * 1024;
    }
    */
  
	PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_LOW_FORMAT_START));
    burnFile.WriteLogFile(LOG_LINE_TIME,  "++sd low format start++\r\n");
    burnFile.WriteLogFile(0, "format_count = %d, res_blocks = %d\n", theConfig.format_count, theConfig.fs_res_blocks );

	if (!cAK.LowFormat((PBYTE)partInfo, theConfig.format_count, theConfig.fs_res_blocks, 
		StartBlock, ModeCtrl.eMedium, (PBYTE)medium, StartID, IDCnt))
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "->fail!\r\n" );
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_LOW_FORMAT_FAIL));
		return FALSE;
	}
	

	return TRUE;
}

//�͸�nand
BOOL Burn_LowFormat(UINT nID, HWND hWnd, PTCHAR file_name, T_PARTION_INFO *partInfo, 
					T_PNANDFLASH nandBase, UINT StartBlock, T_MODE_CONTROL ModeCtrl, UINT *StartID, UINT *IDCnt)
{
	CAKFS cAK;
	CLogFile  burnFile(file_name);

	if (nID != 1) 
	{
		/**����ȴ���һͨ������������ok����ܼ���**/
		while (theConfig.bonline_make_image)
		{
			Sleep(10);
		}
	}

	memcpy(partInfo, theConfig.format_data, sizeof(T_PARTION_INFO)*theConfig.format_count);
	
	//��ʽ�����̵ı�־
	partInfo->resv[0] = 1;  //1��ʾ�и�ʽ���� 0��ʾ����ʽ��

    /*
    //����ֻ�������һ������������
    if(MODE_NEWBURN == ModeCtrl.burn_mode)
    {
        partInfo[theConfig.format_count - 1].EnlargeSize = 8 * 1024;
    }
    */
  
	PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_LOW_FORMAT_START));
    burnFile.WriteLogFile(LOG_LINE_TIME,  "++low format start++\r\n");
    burnFile.WriteLogFile(0, "format_count = %d, res_blocks = %d\n", theConfig.format_count, theConfig.fs_res_blocks );

	if (!cAK.LowFormat((PBYTE)partInfo, theConfig.format_count, theConfig.fs_res_blocks, 
		StartBlock, ModeCtrl.eMedium, (PBYTE)nandBase, StartID, IDCnt))
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "->fail!\r\n" );
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_LOW_FORMAT_FAIL));
		return FALSE;
	}

	return TRUE;
}

//linuxƽ̨�£� spiҲ���Դ���������
BOOL Burn_CreatePartion_SpiFlash(UINT nID, HWND hWnd, PTCHAR file_name)
{
	CLogFile  burnFile(file_name);
	T_SPIFLASH_PARTION_INFO *spi_partInfo = NULL;
	UINT i = 0;
	
	spi_partInfo = new T_SPIFLASH_PARTION_INFO[theConfig.format_count];
	if (spi_partInfo == NULL)
	{
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_CREATE_PARTITION_FAIL));
		return FALSE;
	}

	memcpy(spi_partInfo, theConfig.format_data, sizeof(T_PARTION_INFO)*theConfig.format_count);

	//��ȡspi��������Ϣ
	for (i = 0; i < theConfig.format_count; i++)
	{			
		spi_partInfo[i].Size = theConfig.spi_format_data[i].Size;   //�˱�������ͬ
	}

	PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_CREATE_PARTITION_START));
    burnFile.WriteLogFile(LOG_LINE_TIME,  "++create partition++\r\n");
    burnFile.WriteLogFile(0, "format_count = %d, res_blocks = %d\n", theConfig.format_count, theConfig.fs_res_blocks );

	//���������Ĺ�����produceִ��
    if (BT_CreatePartion_SpiFlash(nID, spi_partInfo, theConfig.format_count, theConfig.fs_res_blocks) != BT_SUCCESS)
	{
        //Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
		burnFile.WriteLogFile(LOG_LINE_TIME, "->fail!\r\n" );
		delete[] spi_partInfo; 
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_CREATE_PARTITION_FAIL));
		return FALSE;
	}
	delete[] spi_partInfo; 

	return TRUE;
}

BOOL Burn_CreatePartion(UINT nID, HWND hWnd, PTCHAR file_name, T_PARTION_INFO *partInfo)
{
	CLogFile  burnFile(file_name);
	
	memcpy(partInfo, theConfig.format_data, sizeof(T_PARTION_INFO)*theConfig.format_count);
	
    /*
    //����ֻ�������һ������������
    if(MODE_NEWBURN == ModeCtrl.burn_mode)
    {
        partInfo[theConfig.format_count - 1].EnlargeSize = 8 * 1024;
    }
    */
  
	PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_CREATE_PARTITION_START));
    burnFile.WriteLogFile(LOG_LINE_TIME,  "++create partition++\r\n");
    burnFile.WriteLogFile(0, "format_count = %d, res_blocks = %d\n", theConfig.format_count, theConfig.fs_res_blocks );

    if (BT_CreatePartion(nID, partInfo, theConfig.format_count, theConfig.fs_res_blocks) != BT_SUCCESS)
	{
        //Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
		burnFile.WriteLogFile(LOG_LINE_TIME, "->fail!\r\n" );
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_CREATE_PARTITION_FAIL));
		return FALSE;
	}

	return TRUE;
}

BOOL Burn_CreateDiskVolume(UINT nID, HWND hWnd, PTCHAR file_name, T_MODE_CONTROL ModeCtrl)
{
	UINT i;
	CLogFile  burnFile(file_name);

	PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_START_FORMAT_TRANSC));
    burnFile.WriteLogFile(LOG_LINE_TIME,  "++format++\r\n" );

	if (theConfig.pVolumeLable != NULL)
	{
		T_FORMAT_DRIVER formatDriver;
		
		if(MODE_NEWBURN == ModeCtrl.burn_mode)
		{
			for (i=0; i<theConfig.format_count; i++)
			{
				//formatDriver.driver_Name     = theConfig.pVolumeLable[i].Disk_Name;
				formatDriver.driver_Name     = theConfig.format_data[i].Disk_Name;
				strcpy(formatDriver.pstrVolumeLable, theConfig.pVolumeLable[i].volume_lable);
                burnFile.WriteLogFile(0, "%ddriver name=%c, label=%s\n", i, formatDriver.driver_Name, formatDriver.pstrVolumeLable );
				if (BT_FormatDriver(nID, &formatDriver) != BT_SUCCESS)
				{
                    //Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
					burnFile.WriteLogFile(LOG_LINE_TIME,  "->fail\r\n");
					PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_FORMAT_TRANSC_FAIAL));
					//delete[] partInfo;
					return FALSE;
				}
			}
		}
		else
		{
			char driverName[26];
			for (i=0; i<theConfig.format_count; i++)
			{
				driverName[i] = theConfig.format_data[i].Disk_Name;
			}
			
			if(BT_SUCCESS != BT_MountDriver(nID, driverName, theConfig.format_count))
			{
                //Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
				burnFile.WriteLogFile(LOG_LINE_TIME,  "->mount fail!\r\n");
				PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_FORMAT_TRANSC_FAIAL));
				//delete[] partInfo;
				return FALSE;
			}
		}
		
	}
	else
	{
        //Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
		burnFile.WriteLogFile(LOG_LINE_TIME,  "->pVolumeLable null!\r\n");
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_FORMAT_TRANSC_FAIAL));
		//delete[] partInfo;
		return FALSE;
	}
	return TRUE;
}

//���snowrdbirdL���л�ȡ�̷���Ϣ
BOOL Burn_GetDiskInfo (UINT StartID, UINT DriverCnt,  UINT *DriverNum, T_DRIVER_INFO* pDriverInfo, UCHAR MediumType)
{
	CAKFS cAK;

	return cAK.GetDriverInfo(StartID, DriverCnt, DriverNum, (UCHAR *)pDriverInfo, MediumType);
}
//������������
BOOL Burn_OnlineMakingImage(UINT nID, HWND hWnd, PTCHAR file_name, UINT sector_per_page, UINT SectorSize, UINT StartID, UINT DriverCnt, UCHAR MediumType)
{
	UINT nEnlargeDiskB_Size = 0;
	UINT i;
	UINT driverNum;
	BYTE pDriverInfoBuf[512];
	
	//������������
	if (1)//eConfig.planform_tpye == E_ROST_PLANFORM))
	{
		if ((nID == 1 && theConfig.bonline_make_image) 
			|| (CHIP_1080L == theConfig.chip_type) || (CHIP_10XXC == theConfig.chip_type))
		{
			CLogFile  burnFile(file_name);
			
			if (theConfig.format_count > 0)
			{
			   // enlargeSize setting;
				if (theConfig.burn_mode == E_CONFIG_NAND || theConfig.burn_mode == E_CONFIG_SPI_NAND)   // ֻ��nand�汾����
				{
					nEnlargeDiskB_Size = GetDISKBSizeEnlarge();    // ����ֵ
					burnFile.WriteLogFile(0, "enlarge size:%d\n", nEnlargeDiskB_Size);
					for (i=0; i<theConfig.format_count; i++)
					{
						if ((theConfig.format_data[i].bOpenZone) && (nEnlargeDiskB_Size != 0))  // �û���
						{
							theConfig.format_data[i].EnlargeSize = nEnlargeDiskB_Size; // ����ֵ
						}
					}
				}
			}
			
			burnFile.WriteLogFile(LOG_LINE_TIME, "++get disk info++\n");
			if (CHIP_1080L == theConfig.chip_type || CHIP_10XXC == theConfig.chip_type)
			{
				//ֻ��snowbirdLƽ̨ʹ��ʱ���������ص�u�̵��ļ����ڵ��̷�
				Save_udiskfile_drivernum();

				if (!Burn_GetDiskInfo(StartID, DriverCnt, &driverNum, (T_DRIVER_INFO*)pDriverInfoBuf, MediumType))
				{
					PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_IMAGE_CREATE_FAIL));
					return FALSE;
				}
			}
			else if (BT_FAIL == BT_GetDiskInfo(nID, &driverNum, (T_DRIVER_INFO*)pDriverInfoBuf))
			{
				PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_IMAGE_CREATE_FAIL));
				return FALSE;
			}

			if (0 == driverNum || driverNum > 26)
			{
				PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_IMAGE_CREATE_FAIL));
				return FALSE;
			}
			burnFile.WriteLogFile(0, "drvNum:%d\n", driverNum);
			SetPartInfo(nID, (T_DRIVER_INFO*)pDriverInfoBuf, driverNum, sector_per_page, SectorSize, nEnlargeDiskB_Size);

			//finish image make 
			//theConfig.bonline_make_image = FALSE;

			//create image
			burnFile.WriteLogFile(LOG_LINE_TIME,"create image\n");
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_START_IMAGE_CREATE));
			
			//wait creat image
			if(WaitForSingleObject(image_event, 3600000) != WAIT_OBJECT_0)
			{
				burnFile.WriteLogFile(LOG_LINE_TIME, "create time out\n");
				//Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
				PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_IMAGE_CREATE_FAIL));
				//delete[] partInfo;
				return FALSE;
			}
			
			if(E_IMG_FAIL == g_img_stat)
			{
				//Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
				PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_IMAGE_CREATE_FAIL));
				//delete[] partInfo;
				return FALSE;
			}
			else
			{
				burnFile.WriteLogFile(LOG_LINE_TIME, "++�������������++\n");
				//�����޶�ֻ��һ����������,��ô�����һ�κ�,�͸�λ,��������������,��ô�ٴν��й�ѡ
				PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_IMAGE_CREATE_RESET));
			}
			//finish image make 
			theConfig.bonline_make_image = FALSE;
			
		}
	}

	return TRUE;
}
//���ؾ����ļ�
BOOL Burn_DownloadImg(UINT nID, HWND hWnd, PTCHAR file_name, T_MODE_CONTROL ModeCtrl, UINT SectorSize, 
					  T_NAND_PHY_INFO NandPhyInfo, T_PARTION_INFO *partInfo)
{
	UINT i, n = 0;
    UINT pagesize = 0;
    UINT  driverNo;
	T_DOWNLOAD_IMG download_img;
	CLogFile  burnFile(file_name);
	CAKFS cAK;
	UINT img_buf_len = IMG_BUF_SIZE_SD;
	UINT download_MTD_num = 0;
	BOOL mtd_flag = AK_FALSE;
	CString str;
	

	USES_CONVERSION;
	
    PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_IMG_START));
    burnFile.WriteLogFile(LOG_LINE_TIME,  "++download img++\r\n");
    
	//����snowbirdl����¼�ṹ����mount��ʱû�и�ʽ����ֻ��ͨ�����ؾ���ʱ�Ÿ�ʽ��
	//���������ؾ���ʱ��snowbirdlоƬ��¼Ҫ����ÿһ�������ؾ��ļ�����ô��37��98���ֿ�
	if ((theConfig.planform_tpye == E_ROST_PLANFORM) && ((theConfig.chip_type == CHIP_1080L) || (theConfig.chip_type == CHIP_10XXC)))
	{
		download_MTD_num = theConfig.format_count;
	}
	else
	{
		//37/98
		download_MTD_num = g_download_mtd_count;
	}
	burnFile.WriteLogFile(0, "download_MTD_num:%d\r\n", download_MTD_num);

	if (theConfig.burn_mode == E_CONFIG_NAND || theConfig.burn_mode == E_CONFIG_SPI_NAND)
	{
		pagesize = NandPhyInfo.page_size;
		img_buf_len = IMG_BUF_SIZE_NAND; //NAND����usb�Ĵ����С
	} 
	if (theConfig.burn_mode == E_CONFIG_SD)
	{
		pagesize = SectorSize;
		img_buf_len = IMG_BUF_SIZE_SD; //SD����usb�Ĵ����С
	}

	for(i = 0; i < download_MTD_num; i++)
	{
		download_img.bCompare = g_download_mtd[i].bCompare;

		//����11ƽ̨�������ɾ���ÿһ��ͨ����һ�����������ֿ�
		if ((theConfig.planform_tpye == E_ROST_PLANFORM) && ((theConfig.chip_type == CHIP_1080L) || (theConfig.chip_type == CHIP_10XXC)))
		{
			mtd_flag = AK_FALSE;
			g_download_mtd_flag[nID-1] = 1;
			//�������ؾ����ж��Ƿ�Ҫ������������
			for (n = 0; n < theConfig.download_mtd_count; n++)
			{
				if (theConfig.download_mtd_data[n].disk_name[0] == theConfig.format_data[i].Disk_Name)
				{
					mtd_flag = AK_TRUE;  
					break;
				}
			}

			//���mtd_flag��AK_TRUE
			//Ҫ��������mtd�Ƿ����������������ľ��񣬷�������ԭ�е�
			if (AK_TRUE == mtd_flag)
			{
				if (!GetDownloadImgFromPCPath(download_img.pc_path, theApp.ConvertAbsolutePath(g_download_mtd[i].pc_path), pagesize))
				{
					PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_IMG_FAIL));
					return FALSE;
				}
			} 
			else
			{
				//�������еľ����ļ�
				str.Format(_T("Image\\burn%c.img"), theConfig.format_data[i].Disk_Name);
				if (!GetDownloadImgFromPCPath(download_img.pc_path, theApp.ConvertAbsolutePath(str), pagesize))
				{
					PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_IMG_FAIL));
					return FALSE;
				}
			}
			download_img.driver_name = (char)theConfig.format_data[i].Disk_Name;
		} 
		else
		{
			download_img.driver_name = (char)g_download_mtd[i].disk_name[0];
			if (!GetDownloadImgFromPCPath(download_img.pc_path, theApp.ConvertAbsolutePath(g_download_mtd[i].pc_path), pagesize))
			{
				//Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
				PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_IMG_FAIL));
				//delete[] partInfo;
				return FALSE;
			}
		}

	    //_tcscpy(download_img.pc_path, theApp.ConvertAbsolutePath(g_download_mtd[i].pc_path));
        for (driverNo=0; driverNo<theConfig.format_count; driverNo++)
        {
			//�Ƚ��б��д���ٱȽ�
            if (toupper(download_img.driver_name) == toupper(partInfo[driverNo].Disk_Name))
            {
                break;
            }
        }
        if (driverNo == theConfig.format_count)
        {
            burnFile.WriteLogFile(0, "->Driver Name no matching!\r\n" );
            continue;
        }
		

        burnFile.WriteLogFile(0,"driver name:%c, driverNo:%d, mode:%d, UserZone:%d\r\n",
            download_img.driver_name, driverNo, ModeCtrl.burn_mode, partInfo[driverNo].bOpenZone);

        if ((ModeCtrl.burn_mode != MODE_UPDATE) || (partInfo[driverNo].bOpenZone == FALSE))
        {
            burnFile.WriteLogFile(LOG_LINE_TIME, "DOWNLOAD IMG...\r\n" );
			if((theConfig.chip_type == CHIP_1080L) || (theConfig.chip_type == CHIP_10XXC))
			{
				HANDLE file = INVALID_HANDLE_VALUE;
				T_IMG_INFO img_info;
				DWORD  high = 0;
				
				//�����еľ����ļ� 
				file = CreateFile(download_img.pc_path, GENERIC_READ, FILE_SHARE_READ, 
					NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
				if(INVALID_HANDLE_VALUE == file)
				{
					PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_IMG_FAIL));
					return FALSE;
				}

				img_info.DriverName = g_nID_disk_info[nID-1].g_disk_info[i].diskName;
				//img_info.DriverName = download_img.driver_name;
				img_info.data_length = GetFileSize(file, &high);
				img_info.bCheck = download_img.bCompare;
				img_info.wFlag = AK_TRUE;
				//��ʼ���ļ�ϵͳ���fsa
				if(cAK.Init() != AK_TRUE)
				{
					burnFile.WriteLogFile(LOG_LINE_TIME,  "->fsa Init fail!\r\n" );
					CloseHandle(file);
					PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_IMG_FAIL));
					return FALSE;
				}
				//���ؾ���
				if(cAK.DownloadImg(nID, file, &img_info, img_buf_len) != AK_TRUE)
				{
					burnFile.WriteLogFile(LOG_LINE_TIME,  "->fail!\r\n" );
					CloseHandle(file);
					PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_IMG_FAIL));
					return FALSE;
				}
				CloseHandle(file);

			}
			else
			{
				//10Lƽ̨���ƽ̨�ľ�������
				if (BT_DownloadImg(nID, &download_img) != BT_SUCCESS)
				{
					burnFile.WriteLogFile(LOG_LINE_TIME,  "->fail!\r\n" );
					PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_IMG_FAIL));
					return FALSE;
				}
			}
        }
	}

	if (theConfig.planform_tpye == E_LINUX_PLANFORM && g_disk_count != 0)
	{
		for(i = 0; i < g_disk_count; i++)
		{
			download_img.bCompare = 1;  //
		
			//�������еľ����ļ�
			str.Format(_T("Image\\burn%c.img"), g_nID_disk_info[nID-1].g_disk_info[i].diskName);
			if (!GetDownloadImgFromPCPath(download_img.pc_path, theApp.ConvertAbsolutePath(str), pagesize))
			{
				PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_IMG_FAIL));
				return FALSE;
			}
			download_img.driver_name = (char)g_nID_disk_info[nID-1].g_disk_info[i].diskName;

			for (driverNo=0; driverNo<theConfig.format_count; driverNo++)
			{
				//�Ƚ��б��д���ٱȽ�
				if (toupper(download_img.driver_name) == toupper(partInfo[driverNo].Disk_Name))
				{
					break;
				}
			}
			if (driverNo == theConfig.format_count)
			{
				burnFile.WriteLogFile(0, "->Driver Name no matching!\r\n" );
				continue;
			}

			burnFile.WriteLogFile(0,"driver name:%c, driverNo:%d, mode:%d, UserZone:%d\r\n",
				download_img.driver_name, driverNo, ModeCtrl.burn_mode, partInfo[driverNo].bOpenZone);

			if (ModeCtrl.burn_mode != MODE_UPDATE)
			{
				//10Lƽ̨���ƽ̨�ľ�������
				if (BT_DownloadImg(nID, &download_img) != BT_SUCCESS)
				{
					burnFile.WriteLogFile(LOG_LINE_TIME,  "->fail!\r\n" );
					PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_IMG_FAIL));
					return FALSE;
				}
			}
		}
	}
	
	return TRUE;
}
//���ص�u�̵��ļ�
BOOL Burn_DownloadFile(UINT nID, HWND hWnd, PTCHAR file_name)
{
	UINT i, j;
	CLogFile  burnFile(file_name);
	BOOL udisk_file_flag = FALSE;
	
	USES_CONVERSION;
	PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_START_DOWNLOAD_FILE));
    burnFile.WriteLogFile(LOG_LINE_TIME,  "++download disk file++\r\n" );
    T_DOWNLOAD_FILE download_file;

    burnFile.WriteLogFile(0, "download_udisk_count:%d\r\n", g_download_udisk_count);
	for(i = 0; i < g_download_udisk_count; i++)
	{
		download_file.bCompare = g_download_udisk[i].bCompare;
		_tcscpy(download_file.pc_path, theApp.ConvertAbsolutePath(g_download_udisk[i].pc_path));
        memset(download_file.udisk_path,0,MAX_PATH+1);
		// strcpy(download_file.udisk_path, T2A(g_download_udisk[i].udisk_path));
        burnFile.WriteLogFile(0, "download_udisk_count_%d: %s\r\n", i,T2A(g_download_udisk[i].udisk_path));
		
		//linuxƽ̨����Ҫת���ַ�
		if (theConfig.planform_tpye == E_LINUX_PLANFORM)
        { 
			//���linux
			for(j = 0; j < g_disk_count; j++)
			{
				if (g_download_udisk[i].udisk_path[0] == g_nID_disk_info[nID-1].g_disk_info[j].diskName)
				{
					udisk_file_flag = TRUE;
					break;
				}
			}
			if (TRUE == udisk_file_flag)
			{
				udisk_file_flag = FALSE;
				continue;
			}

			// change to UTF-8 code 
            UINT nLen = _tcslen(g_download_udisk[i].udisk_path);
            UINT utf8Len = WideCharToMultiByte(CP_UTF8, 0, g_download_udisk[i].udisk_path, nLen,NULL, 0, NULL, NULL);
           
            WideCharToMultiByte(CP_UTF8,0,g_download_udisk[i].udisk_path,utf8Len,download_file.udisk_path,utf8Len,NULL,NULL);
        }
		else
		{
			strcpy(download_file.udisk_path, T2A(g_download_udisk[i].udisk_path));
		}
		//�����ļ�
		if (BT_DownloadFile(nID, &download_file) != BT_SUCCESS)
		{
			//Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
            burnFile.WriteLogFile(LOG_LINE_TIME, "->fail!\r\n");
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_FILE_FAIL));
			return FALSE;
		}
	}
	
	return TRUE;
}
//���ص�u�̵��ļ������swordbirdL
/*
BOOL Burn_DownloadFileNew(UINT nID, HWND hWnd, PTCHAR file_name, UINT StartID)
{
	UINT i;
	CAKFS cAK;
	CLogFile  burnFile(file_name);
	
	USES_CONVERSION;
	PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_START_DOWNLOAD_FILE));
    burnFile.WriteLogFile(LOG_LINE_TIME,  "++download disk file++\r\n" );
    T_DOWNLOAD_FILE download_file;

    burnFile.WriteLogFile(0, "download_udisk_count:%d\r\n", g_download_udisk_count);
	for(i = 0; i < g_download_udisk_count; i++)
	{
		download_file.bCompare = g_download_udisk[i].bCompare;
		_tcscpy(download_file.pc_path, theApp.ConvertAbsolutePath(g_download_udisk[i].pc_path));
        memset(download_file.udisk_path,0,MAX_PATH+1);
		// strcpy(download_file.udisk_path, T2A(g_download_udisk[i].udisk_path));
        burnFile.WriteLogFile(0, "download_udisk_count_%d: %s\r\n", i,T2A(g_download_udisk[i].udisk_path));
		
		strcpy(download_file.udisk_path, T2A(g_download_udisk[i].udisk_path));
	
		download_file.udisk_path[0] += StartID;
		//�������ID��0-25����ΪӢ����ĸ������26
		if (download_file.udisk_path[0] > ('A' + 25))
		{
            burnFile.WriteLogFile(LOG_LINE_TIME, "->fail! ID>25\r\n");
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_FILE_FAIL));
			return FALSE;
		}

		if (!cAK.DownloadFile((UINT)&download_file))
		{
            burnFile.WriteLogFile(LOG_LINE_TIME, "->fail!\r\n");
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_FILE_FAIL));
			return FALSE;
		}
	}
	
	return TRUE;
}
*/

//����boot�ļ�
BOOL Burn_DownloadBoot(UINT nID, HWND hWnd, PTCHAR file_name)
{
	CLogFile  burnFile(file_name);

	USES_CONVERSION;
    PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_BOOT_START));
    //burnFile.WriteLogFile(LOG_LINE_TIME, "++download boot file++\r\n");
    burnFile.WriteLogFile(LOG_LINE_TIME, "boot file path:%s\r\n", W2A(theConfig.path_nandboot));
	if (CHIP_37XX_L == theConfig.chip_type)
	{
		if (BT_DownloadBoot(nID, theApp.ConvertAbsolutePath(theConfig.path_nandboot_new), theConfig.chip_type) != BT_SUCCESS)
		{
			burnFile.WriteLogFile(LOG_LINE_TIME, "->fail!\r\n");
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_BOOT_FAIL));
			return FALSE;
		}
	}
	else
	{
		if (BT_DownloadBoot(nID, theApp.ConvertAbsolutePath(theConfig.path_nandboot), theConfig.chip_type) != BT_SUCCESS)
		{
			//Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
			burnFile.WriteLogFile(LOG_LINE_TIME, "->fail!\r\n");
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_BOOT_FAIL));
			return FALSE;
		}
	}

	return TRUE;
}

//����
BOOL Burn_ReSetDev(UINT nID, HWND hWnd, PTCHAR file_name)
{
	//ֻ��10��11оƬ��֧������
	if(theConfig.planform_tpye == E_ROST_PLANFORM 
		&& (CHIP_1080L == theConfig.chip_type || CHIP_11XX == theConfig.chip_type
		|| CHIP_1080A == theConfig.chip_type || CHIP_10X6 == theConfig.chip_type
		|| CHIP_10XXC == theConfig.chip_type))
	{
		if (m_worknum != 0)
		{
			//�ȴ�m_worknum == 0ʱ����������
			if(WaitForSingleObject(ResetDevice_event, 3600000) != WAIT_OBJECT_0)
			{
				PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_GET_REGVALUE_FAIL));
				return FALSE;
			}
		}
		BT_ResetDevice(nID);
	}

	return TRUE;
}

//umount nandflash
void Burn_UnMountMedium(UINT StartID, T_PMEDIUM Pmedium)
{
	if ((theConfig.planform_tpye == E_ROST_PLANFORM) && ((CHIP_1080L == theConfig.chip_type) || (CHIP_10XXC == theConfig.chip_type)))
	{
		CAKFS cAK; 
		if (theConfig.burn_mode == E_CONFIG_NAND || theConfig.burn_mode == E_CONFIG_SPI_NAND)
		{
			cAK.UnMountNandFlash(StartID);
		}
		else
		{
			cAK.UnMountMemDev(StartID);
			Burn_Free_Medium(Pmedium);
		}
		
	}
}
//mount nandflash
/*
BOOL Burn_MountMedium(UINT medium, UINT StartBlock, UINT *StartID, UINT *DriverCnt)
{
	*StartID = 0xff;

	if ((theConfig.planform_tpye == E_ROST_PLANFORM) && (CHIP_1080L == theConfig.chip_type))
	{
		CAKFS cAK; 
		
		*StartID = cAK.MountNandFlash(medium,StartBlock, NULL, (UCHAR *)DriverCnt);

	}

	if (*StartID == 0xff)
		return FALSE;
	else
		return TRUE;

}
*/
BOOL Burn_Read_spiAlldata(UINT nID, HWND hWnd, CString path, UINT spiflash_len)
{
	TCHAR pc_path[MAX_PATH+1] = {0};
	
	USES_CONVERSION;
	
	PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_START_DOWNLOAD_FILE));
	
    CreateDirectory(theApp.ConvertAbsolutePath(path + _T("//UploadBin")), NULL);

	CString strTmp;
	
    strTmp = path + _T("//UploadBin//");
	strTmp += _T("spiflash");
	strTmp += _T(".bin");
	
	_tcscpy(pc_path , theApp.ConvertAbsolutePath(strTmp));
	//�ض�bin�ļ�
	if (BT_Upload_SpiAlldata(nID, pc_path, spiflash_len) != BT_SUCCESS)
	{
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_FILE_FAIL));
		return FALSE;
	}

	
	return TRUE;

}

BOOL BurnDebugMode(UINT nID, HWND hWnd, CString path)
{
	T_UPLOAD_BIN uploadbin;
	T_U32 i;
	
	USES_CONVERSION;
	
	PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_START_DOWNLOAD_FILE));

    CreateDirectory(theApp.ConvertAbsolutePath(path + _T("//UploadBin")), NULL);
#if 1
	for(i = 0; i < g_download_nand_count; i++)
	{
		CString strTmp;
		memset(&uploadbin, 0, sizeof(T_UPLOAD_BIN));
		memcpy(uploadbin.file_name, T2A(g_download_nand[i].file_name), 16);
		
        strTmp = path + _T("//UploadBin//");
		strTmp += g_download_nand[i].file_name;
		strTmp += _T(".bin");

		_tcscpy(uploadbin.pc_path , theApp.ConvertAbsolutePath(strTmp));
		//�ض�bin�ļ�
		if (BT_UploadBin (nID, &uploadbin) != BT_SUCCESS)
		{
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_FILE_FAIL));
			return FALSE;
		}
	}
#endif
	//uplaod Bios
	CString strTmp;
	strTmp = path + _T("//UploadBin//BOOT.bin");
	//�ض�boot�ļ�
	if (BT_UploadBoot(nID, theApp.ConvertAbsolutePath(strTmp), 32*1024) != BT_SUCCESS)
	{
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_FILE_FAIL));
		return FALSE;
	}

	return TRUE;
}

//bin�ض�����
BOOL Burn_DebugMode(UINT nID, HWND hWnd, PTCHAR file_name)
{

	TCHAR   BinUpload_path[MAX_PATH+1];
	CLogFile  burnFile(file_name);
	
	burnFile.WriteLogFile(LOG_LINE_TIME, "upload bin\n");
	browser_for_binUpload(BinUpload_path);
	
	if (!BurnDebugMode(nID, hWnd, BinUpload_path)) 
	{
		return FALSE;
	}
		
	return TRUE;	
}

BOOL Burn_Get_spiAlldata(UINT nID, HWND hWnd, PTCHAR file_name, UINT spiflash_len)
{
	
	TCHAR   Uploadspi_path[MAX_PATH+1];
	CLogFile  burnFile(file_name);
	
	burnFile.WriteLogFile(LOG_LINE_TIME, "upload bin\n");
	browser_for_binUpload(Uploadspi_path);
	
	if (!Burn_Read_spiAlldata(nID, hWnd, Uploadspi_path, spiflash_len)) 
	{
		return FALSE;
	}
	
	return TRUE;	
}


//����ak10xxʱ����ô��ȡboot�µ�ֵ�Ƿ���CHIP_1080A����CHIP_1080L
BOOL Burn_GetRegValue(UINT nID, HWND hWnd, PTCHAR file_name)
{
//	CLogFile  burnFile(file_name);

//	burnFile->WriteLogFile(LOG_LINE_TIME, "Burn_GetRegValue fail\n");

	if (theConfig.chip_type == CHIP_10X6)
	{
		UINT metafixAddr = 0x6228, metafixValue;
		if (BT_GetRegValue(nID, metafixAddr, &metafixValue) != BT_SUCCESS)
		{
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_GET_REGVALUE_FAIL));
			return FALSE;
		}
		if (0x32424e41 == metafixValue)
		{
			//AK3671оƬV6.0�汾��оƬ
			theConfig.chip_type = CHIP_1080A;
		}
		else
		{
			//��ȡоƬ��chip id
			metafixAddr = 0x00400000, metafixValue;
			if (BT_GetRegValue(nID, metafixAddr, &metafixValue) != BT_SUCCESS)
			{
				PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_GET_REGVALUE_FAIL));
				return FALSE;
			}
			if (0x20120300 == metafixValue)
			{
				theConfig.chip_type = CHIP_1080L;//1080L,1060L,1050L����ͬһ��
			}
			else if(0x20120A00 == metafixValue)
			{
				theConfig.chip_type = CHIP_10XXC;
			}
		}
	}
	//������37����37L
	if (theConfig.chip_type == CHIP_37XX || theConfig.chip_type == CHIP_37XX_L)
	{
		UINT metafixAddr = 0x08000000, metafixValue;
		if (BT_GetRegValue(nID, metafixAddr, &metafixValue) != BT_SUCCESS)
		{
			return FALSE;
		}
		if (0x20130100 == metafixValue)
		{
			//AK37LоƬV6.0�汾��оƬ
			theConfig.chip_type = CHIP_37XX_L;
		}
		else
		{
			theConfig.chip_type = CHIP_37XX;
		}
	}
	return TRUE;
}

//u�̷�ʽ�µ���¼�ȴ�1Sʱ���������豸������massboot
BOOL Burn_Udisk_Wait_Massboot(CLogFile  burnFile)
{
	//UINT i   = 0;
	//BOOL udisk_flagtemp = FALSE;
	//u����¼��ʽ�£���ÿ��ʼһ���豸��m_budisk_getUSBnum�Լ�1��
	//һֱ����0ʱ�ż����¼������򶼵ȴ�״̬
	if (theConfig.bUDiskUpdate && m_budisk_getUSBnum)
	{
		
		WaitForSingleObject(g_handle,INFINITE);
		burnFile.WriteLogFile(0, "01 g_udisk_burnnum: %d\n", g_udisk_burnnum);
		g_udisk_burnnum--;
		burnFile.WriteLogFile(0, "02 g_udisk_burnnum: %d\n", g_udisk_burnnum);
		ReleaseSemaphore(g_handle,1,NULL);

		
		if (g_udisk_burnnum == 0)
		{
			SetEvent(udiskburn_event);
		}
		burnFile.WriteLogFile(0, "03 g_udisk_burnnum: %d\n", g_udisk_burnnum);
		
		//wait creat image
		if(WaitForSingleObject(udiskburn_event, 3600000) != WAIT_OBJECT_0)
		{
			return FALSE;
		}
	}
	return TRUE;

}

BOOL Burn_capacity_check_Wait(UINT nID)
{
	CString str;
	UINT idex   = 0;
	
	
	WaitForSingleObject(g_handle,INFINITE);
	g_capacity_burnnum++;
	ReleaseSemaphore(g_handle,1,NULL);

	//compare the nand size
	for (idex = 0; idex < theConfig.device_num; idex++)
	{
		if (g_capacity_size[idex] != 0 && g_capacity_flag == AK_TRUE)
		{
			if (g_capacity_size[idex] != g_capacity_size[nID-1])
			{
				//str.Format(IDS_MEDIUM_CAPACITY_CHECK_FAIL);
				//AfxMessageBox(str, MB_OK);
				g_capacity_flag = AK_FALSE;
				SetEvent(capacity_event);
			}
		}
	}
	
	//
	if (g_capacity_flag == AK_TRUE && g_capacity_burnnum == m_worknum)
	{
		SetEvent(capacity_event);
	}
	
	//wait creat image
	if(WaitForSingleObject(capacity_event, 3600000) != WAIT_OBJECT_0)
	{
		g_capacity_size[nID-1] = 0;
		return FALSE;
	}

	g_capacity_size[nID-1] = 0;
	//��һ̨��������ͬ�������Ķ�ʧ��
	if (AK_FALSE == g_capacity_flag)
	{
		return FALSE;
	}
	
	return TRUE;
	
}


BOOL BurnThread(UINT nID)
{
    HWND hWnd = AfxGetMainWnd()->GetSafeHwnd();
    CMainFrame *pMain = (CMainFrame *)AfxGetMainWnd();
	T_PARTION_INFO *partInfo = NULL;
	T_MODE_CONTROL ModeCtrl;
    T_TRANSC_PARA  transc_para = {0};

	//��asa����mac��ַ
	BOOL readmacflag = FALSE;
	TCHAR mactempbuf[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};
	TCHAR maclowbuf[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};
	UINT  macmode = 0;

	//��asa�������кŵ�ַ
	BOOL readSerialflag = FALSE;
	TCHAR serialtempbuf[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};
	TCHAR seriallowbuf[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};
	UINT  Serialmode = 0;
	UINT com_mode = 0;
	CString str;
	UINT sector_per_page = 1;
    UINT SectorSize = 512;
	UINT pagesize = 0;
	UINT StartID = 0;
	UINT IDCnt   = 0;
	UINT flash_ChipCnt = 0;
	T_NAND_PHY_INFO NandPhyInfo = {0};
	T_PMEDIUM   Pmedium = NULL;
	time_t ltime;
//	struct _timeb timebuffer;
//	char *timeline;


    TCHAR     file_name[MAX_PATH] = {0};
    SYSTEMTIME burnTime;
    GetLocalTime(&burnTime);
    swprintf(file_name, _T("log\\ch[%d]_log%02d_%02d_%02d.txt"), nID,
        burnTime.wYear,
        burnTime.wMonth,
        burnTime.wDay);
    CLogFile  burnFile(file_name);
    
    burnFile.WriteLogFile(0, "=============Burn Information For Channel[%d]=============\n", nID);
    burnFile.WriteLogFile(0, "Print Time: %d-%d-%d %02d:%02d:%02d\n\n",
        burnTime.wYear,
        burnTime.wMonth,
        burnTime.wDay,
        burnTime.wHour,
        burnTime.wMinute,
        burnTime.wSecond);
	burnFile.WriteLogFile(0, "ss g_udisk_burnnum: %d\n", g_udisk_burnnum);


	//u�̷�ʽ�µ���¼�ȴ�1Sʱ���������豸������massboot
	/*
	if (!Burn_Udisk_Wait_Massboot(burnFile))
	{
		burnFile.WriteLogFile(0, "Burn_Udisk_Wait_Massboot fail\n");
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_GET_REGVALUE_FAIL));
		return FALSE;
	}
	*/
	if (theConfig.bUDiskUpdate && m_budisk_getUSBnum)
	{
		WaitForSingleObject(g_handle,INFINITE);

		//_ftime( &timebuffer );
		//timeline = ctime( & ( timebuffer.time ) );
		//burnFile.WriteLogFile(0, "The time is %.19s.%hu", timeline, timebuffer.millitm);

		time(&ltime);
		burnFile.WriteLogFile(0, "01 g_udisk_burnnum: %d, %d\n", g_udisk_burnnum, ltime);
		g_udisk_burnnum--;
		burnFile.WriteLogFile(0, "02 g_udisk_burnnum: %d\n", g_udisk_burnnum);
		ReleaseSemaphore(g_handle,1,NULL);
	
		if (g_udisk_burnnum == 0)
		{
			SetEvent(udiskburn_event);
		}
		burnFile.WriteLogFile(0, "03 g_udisk_burnnum: %d\n", g_udisk_burnnum);
		
		//wait creat image
		if(WaitForSingleObject(udiskburn_event, 3600000) != WAIT_OBJECT_0)
		{
			burnFile.WriteLogFile(0, "Burn_Udisk_Wait_Massboot fail\n");
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_GET_REGVALUE_FAIL));
			return FALSE;
		}
	}
	

	//ÿ��ͨ����ʼʱ����״̬��0
	pMain->StartCountTime(nID);
    if (m_worknum == 0)
    {
        pMain->disable_control();
    }

    m_worknum++;
	g_nID_disk_info[nID-1].nID = nID;
	burn_detel_usb_flag[nID-1] = 1;
	g_img_stat = E_IMG_INIT;

    burnFile.WriteLogFile(LOG_LINE_TIME, "work number = %d\n", m_worknum);

	USES_CONVERSION;

	//��¼ģʽ�ĸı�
	theConfig.ChangeBurnMode(&ModeCtrl);

    burnFile.WriteLogFile(LOG_LINE_TIME, "burn_mode = %d, eMedium = %d\n", ModeCtrl.burn_mode, ModeCtrl.eMedium);
	
	//�����ak10XX,��ôҪ���ֳ���10x6����chip10XX_160K
	if (!Burn_GetRegValue(nID, hWnd, file_name))
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_GetRegValue fail\n");
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_GET_REGVALUE_FAIL));
		return FALSE;
	}

	transc_para.planform = theConfig.planform_tpye; // ƽ̨����
	transc_para.time = theConfig.event_wait_time;   //�ȴ�ʱ��
	transc_para.ChipType = theConfig.chip_type;     //оƬ����

	PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_START_BURN));

	//����ƽ̨��оƬ����Ϣ
	BT_SetTranscParam(&transc_para);

	//���ر�ƵС����
    if (!Burn_DownloadChangClock(nID, hWnd, file_name))
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_DownloadChangClock fail\n");
		return FALSE;
	}
	//�����ڴ����
    if (!Burn_SetRamParam(nID, hWnd, file_name))
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_SetRamParam fail\n");
		return FALSE;
	}

    //#ifdef  SUPPORT_LINUX
	if (theConfig.planform_tpye== E_LINUX_PLANFORM)
	{
		if (FALSE == Download_Channel_Addranddata_ToRam( nID,  hWnd, file_name))
		{
			burnFile.WriteLogFile(LOG_LINE_TIME, "->Download_Channel_Addranddata_ToRam fail.\r\n");
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_CHANNELID_FAIL));
			return FALSE;
		}	
	}//#endif //SUPPORT_LINUX 

	//��rtosƽ̨�£���Ҫд�봮������
    if (!Burn_RTOSDownComInfo(nID, hWnd, file_name))
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_RTOSDownComInfo fail\n");
		return FALSE;
	}


	if (TRUE == theConfig.bTest_RAM)
	{
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_AND_TESTRAM_START));
		if (BT_Download_and_TestRam(nID, theApp.ConvertAbsolutePath(theConfig.path_producer), 
			theConfig.producer_run_addr, theConfig.ram_param.size*1024*1024) != BT_SUCCESS)
		{
			burnFile.WriteLogFile(LOG_LINE_TIME, "download and test ram fail.\r\n" );
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_AND_TESTRAM_FAIL));
			return FALSE;
		}
		else
		{
			burnFile.WriteLogFile(LOG_LINE_TIME,  "->test ram success\r\n");
			goto BURN_SUCCESS;
		}
	}

	PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_PRODUCER_START));
    if (E_CONFIG_JTAG == theConfig.burn_mode)
	{
        burnFile.WriteLogFile(LOG_LINE_TIME,  "->Mode is JTAG. Quit!\r\n");
		goto BURN_SUCCESS;
	}
	
    burnFile.WriteLogFile(LOG_LINE_TIME,  "++Start download producer++\r\n");
    burnFile.WriteLogFile(0,  "producer path: %s, addr=%08x\n", W2A(theConfig.path_producer), theConfig.producer_run_addr);
	
	//����produce
    if (BT_DownloadProducer(nID, theApp.ConvertAbsolutePath(theConfig.path_producer), 
        theConfig.producer_run_addr) != BT_SUCCESS)
    {
		burnFile.WriteLogFile(LOG_LINE_TIME, "download producer fail.\r\n" );
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_PRODUCER_FAIL));
        return FALSE;
    }
	
	//����ģʽ
	if (E_CONFIG_DEBUG == theConfig.burn_mode)
	{
        burnFile.WriteLogFile(LOG_LINE_TIME,  "->Debug mode. Quit!\r\n");
		goto BURN_SUCCESS;
	}
	
    //wait for producer initial finish
    if (!Burn_WaitProducerInitFinish(nID, hWnd, file_name))
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_WaitProducerInitFinish fail.\r\n" );
		return FALSE;
	}   
	//��10LоƬ���ǲ���Ҫ����ת��USB
    if (!Burn_SwitchUSB11To20(nID, hWnd, file_name))
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_SwitchUSB11To20 fail.\r\n" );
		return FALSE;
	} 
	//����USB
    if (!Burn_TestPCAndProducerConnectOK(nID, hWnd, file_name))
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_TestPCAndProducerConnectOK fail.\r\n" );
		return FALSE;
	}

    if (!Burn_SetGPIO(nID, hWnd, file_name))
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_SetGPIO fail.\r\n" );
		return FALSE;
	}	

    if (!Burn_SetBurnPara(nID, hWnd, file_name))
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_SetBurnPara fail.\r\n" );
		return FALSE;
	}

	//����10bLUEƽ̨ʹ�õ�produce�Ǹ���MODE_DEBUGֵ���ж�ȡspi�ļ�����Ϣ,
	//������bin�ض�ʱ����ModeCtrl.burn_mode = MODE_DEBUG;
	if(g_bUploadbinMode && (theConfig.planform_tpye== E_ROST_PLANFORM) && (theConfig.chip_type == CHIP_1080A) && (E_CONFIG_SFLASH == theConfig.burn_mode))
	{
		ModeCtrl.burn_mode = MODE_DEBUG;
	}

    if (!Burn_SetBurnMode(nID, hWnd, file_name, ModeCtrl))
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_SetBurnMode fail.\r\n" );
		return FALSE;
	}
	
    if (!Burn_SetGPIOToNandChip(nID, hWnd, file_name, ModeCtrl))
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_SetGPIOToNandChip fail.\r\n" );
		return FALSE;
	}

	if (TRANSC_MEDIUM_EMMC == ModeCtrl.eMedium)
	{
		if (g_bUploadbinMode)
		{
			goto UPDATA_BINMODE;//bin�ض�
		}
		else
		{
			goto SET_RESV_AREA;//sd����¼
		}
	}

	if (TRANSC_MEDIUM_SPIFLASH == ModeCtrl.eMedium)
	{
		//spi ��¼
		if (burn_spiflash(nID, hWnd, ModeCtrl, file_name))
		{
			if (g_bUploadbinMode)
			{
				goto UPDATA_BINMODE;//bin�ض�
			}
			else
			{
				goto BURN_SUCCESS;
			}
		}
		else
		{
			burnFile.WriteLogFile(LOG_LINE_TIME, "burn_spiflash fail.\r\n" );
			return FALSE;
		}
	}

	//
    if (!Burn_GetFlashIDAndSetNandParam(nID, hWnd, file_name, &sector_per_page, &SectorSize, &NandPhyInfo, &flash_ChipCnt))
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_GetFlashIDAndSetNandParam fail.\r\n" );
		return FALSE;
	}

	//ֻ��ȫ����¼��nand��¼ʱ�Ż��в�������
	if (ModeCtrl.burn_mode == MODE_NEWBURN && g_bEraseMode && (theConfig.burn_mode == E_CONFIG_NAND || theConfig.burn_mode == E_CONFIG_SPI_NAND))
	{
		if (!Burn_SetEraseMode(nID, hWnd, file_name, ModeCtrl))
		{
			burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_SetEraseMode fail.\r\n" );
			return FALSE;
		}
	}
	
	//�͸����
	if(g_bEraseMode)
	{
		//Finish
		//g_bEraseMode = FALSE;
		goto BURN_SUCCESS;
	}

	PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_SET_SEC_AREA_START));
    burnFile.WriteLogFile(LOG_LINE_TIME,  "++init security area++\r\n");
    //������ȫ��
	if (BT_InitSecArea(nID, 0) != BT_SUCCESS)
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "BT_InitSecArea fail!\r\n" );
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_SET_SEC_AREA_FAIL));
		return FALSE;
	}

UPDATA_BINMODE:
	// upload bin call burndebugmode  and then finish burning
	if (g_bUploadbinMode)	
	{
		//ʵ��bin�ض�������bin���ݺ�boot����������
		if(!Burn_DebugMode(nID,  hWnd,  file_name)) 
		{
			burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_DebugMode fail!\r\n" );
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_BIN_UPLOAD_FAIL));
			return FALSE;
		}
		else
		{	
			goto BURN_SUCCESS;
		}
	}

	//ֻ����10LоƬ����nand�汾�Ż���л�ȡ������Ϣ
	if (fNand_Get_BadBlockBuf(nID, NandPhyInfo.blk_num, flash_ChipCnt, &ModeCtrl)  != BT_SUCCESS)
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "fNand_Get_BadBlockBuf fail!\r\n" );
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_GET_BAD_BLOCK_FAIL));
		return FALSE;
	}

SET_RESV_AREA:
	/*****************************************************/
	//�ж��Ƿ���¼MAC��ַ
    if (!Burn_GetMACInfo(nID, hWnd, file_name, maclowbuf, mactempbuf, &readmacflag, &macmode))
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_GetMACInfo fail!\r\n" );
		fNand_Free_BadBlockBuf(nID);
		return FALSE;
	}

	//д���к�
	if (!Burn_GetSERIALInfo(nID, hWnd, file_name, seriallowbuf, serialtempbuf, &readSerialflag, &Serialmode))
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_GetSERIALInfo fail!\r\n" );
		fNand_Free_BadBlockBuf(nID);
		return FALSE;
	}
	
	/*****************************************************/
	//���÷��ļ�ϵϵͳ��������С��
	//���Դ������в�
	if (!Burn_SetResvAreaSize(nID, hWnd, file_name))
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_SetResvAreaSize fail!\r\n" );
        Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
		return FALSE;
	}
	
	//����bin�ļ�
	if (!Burn_DownloadBin(nID, hWnd, file_name))
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_DownloadBin fail!\r\n" );
        Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
		return FALSE;
	}
	
	//snowbirdL����ģʽֻ����bin��boot
	if ((ModeCtrl.burn_mode == MODE_UPDATE) && (theConfig.planform_tpye == E_ROST_PLANFORM) && ((CHIP_1080L == theConfig.chip_type) || (CHIP_10XXC == theConfig.chip_type)))
	{
		goto BURN_BOOT;
	}

	partInfo = new T_PARTION_INFO[theConfig.format_count];
	if (partInfo == NULL)
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "partInfo null!\r\n" );
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_PARTTION_INFORMATION_IS_NULL));
		return FALSE;
	}

	T_NANDFLASH nandBase; /*�ļ�ϵͳ��ʱ����Ҫ�õ�nandbase*/
	UINT        StartBlock; //�ļ�ϵͳ�����Ŀ�ʼλ��
	UINT        capacity_size;

	if ((theConfig.planform_tpye == E_ROST_PLANFORM) && ((CHIP_1080L == theConfig.chip_type) || (CHIP_10XXC == theConfig.chip_type)))
	{
		if (TRANSC_MEDIUM_NAND == ModeCtrl.eMedium || TRANSC_MEDIUM_SPI_NAND == ModeCtrl.eMedium)
		{
			//ͨ��produce��ȡnand����Ϣ
			if (BT_GetMediumStruct(nID, (unsigned char *)&nandBase, sizeof(T_NANDFLASH), &StartBlock) != BT_SUCCESS)
			{
				//ֻ���������Զ���
				burnFile.WriteLogFile(LOG_LINE_TIME, "BT_GetMediumStruct fail!\r\n" );
				PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_GET_MEDIUM_DATAINFO_FAIL));
				Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
				delete[] partInfo;
				return FALSE;
			}
			capacity_size = nandBase.BlockPerPlane*nandBase.PlanePerChip*(nandBase.PagePerBlock*nandBase.SectorPerPage*nandBase.BytesPerSector/1024); //��kΪ��λ���
			g_capacity_size[nID-1] = capacity_size;
			

			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MMESSAGE_MEDIUM_CAPACITY_CHECK));
			//check the nand capacity is or not differnt
			if (!Burn_capacity_check_Wait(nID))
			{
				burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_capacity_check_Wait nand fail!\r\n" );
				PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_MEDIUM_CAPACITY_FAIL));
				return FALSE;
			}
			
			/**���¸�ֵnandbase�Ĳ��ֲ���**/
			Burn_ResetNandBase(nID, &nandBase);
			
			//��ȡ���еĿ��п�
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_GET_ALL_FREE_BLOCK));
			if (fNand_Get_FreeBlockBuf(nID, NandPhyInfo.chip_id, nandBase.BlockPerPlane*nandBase.PlanePerChip, flash_ChipCnt, StartBlock) != BT_SUCCESS)
			{
				burnFile.WriteLogFile(LOG_LINE_TIME, "fNand_Get_FreeBlockBuf fail!\r\n" );
				PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_GET_FREE_BLOCK_FAIL));
				Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
				delete[] partInfo;
				return FALSE;
			}

			//if (!Burn_MountMedium((UINT)&nandBase, StartBlock, &StartID, &IDCnt))
			if (!Burn_LowFormat(nID, hWnd, file_name, partInfo, &nandBase, StartBlock, ModeCtrl, &StartID, &IDCnt))
			{
				burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_LowFormat fail!\r\n" );
				Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
				delete[] partInfo;
				Burn_UnMountMedium(StartID, Pmedium);
				return FALSE;
			}

			//����Сҳ��nand����Ҫȡ�ϲ���
			if (SectorSize == 512)
			{
				sector_per_page = nandBase.BytesPerSector / SectorSize;//����һ��nand��������һ��ҳ�ж��ٸ�����
			}
			

			//StartID = 0;//��Ϊ�ײ�û��ʵ�֣������ȸ�ֵ
		}
		else if(TRANSC_MEDIUM_EMMC == ModeCtrl.eMedium)
		{
			T_EMMC_INFO medium;
			UINT     SD_StartBlock = 0;
			UINT     SecPerPg = 1;
			
			//��ȡsd������Ϣ
			if (BT_GetMediumStruct(nID, (unsigned char *)&medium, sizeof(T_EMMC_INFO), &SD_StartBlock) != BT_SUCCESS)
			{
				//ֻ���������Զ���
				burnFile.WriteLogFile(LOG_LINE_TIME, "BT_GetMediumStruct fail!\r\n" );
				PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_GET_MEDIUM_DATAINFO_FAIL));
				Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
				delete[] partInfo;
				return FALSE;
			}

			g_capacity_size[nID-1] = (medium.total_block*medium.block_size) >> 10; //��kΪ��λ���

			//check the sd capacity is or not differnt
			PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MMESSAGE_MEDIUM_CAPACITY_CHECK));
			if (!Burn_capacity_check_Wait(nID))
			{
				burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_capacity_check_Wait sd fail!\r\n" );
				PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_MEDIUM_CAPACITY_FAIL));
				return FALSE;
			}

			//����һ��medium����
			Pmedium = Burn_Malloc_Medium(nID, medium.block_size, medium.total_block, SecPerPg);
			if (NULL == Pmedium)
			{
				burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_Malloc_Medium fail!\r\n" );
				PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_MALLOC_MEDIUM_FAIL));
				Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
				Burn_Free_Medium(Pmedium);
				delete[] partInfo;
				return FALSE;
			}

			//Burn_ResetMedium(nID, &medium);
			if (!Burn_LowFormat_SD(nID, hWnd, file_name, partInfo, Pmedium, SD_StartBlock, ModeCtrl, &StartID, &IDCnt))
			{
				burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_LowFormat_SD fail!\r\n" );
				Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
				Burn_UnMountMedium(StartID, Pmedium);
				delete[] partInfo;
				return FALSE;
			}
		}
	}
	else
	{
		//10Lƽ̨��Ĵ��������Ĳ���
		if (!Burn_CreatePartion(nID, hWnd, file_name, partInfo))
		{
			burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_CreatePartion fail!\r\n" );
			Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
			//Burn_UnMountMedium(StartID);
			delete[] partInfo;
			return FALSE;
		}
	}

	//�������
	if (!Burn_CreateDiskVolume(nID, hWnd, file_name, ModeCtrl))
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_CreateDiskVolume fail!\r\n" );
		Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
		Burn_UnMountMedium(StartID, Pmedium);
		delete[] partInfo;
		return FALSE;
	}

	/**�����������������ʱ���ռ��A��ʼ�Ĵ��̣����б�����ж��driver**/
	if (!Burn_OnlineMakingImage(nID, hWnd, file_name, sector_per_page, SectorSize, StartID, IDCnt, (UCHAR)ModeCtrl.eMedium))
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_OnlineMakingImage fail!\r\n" );
		Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
		Burn_UnMountMedium(StartID, Pmedium);
		delete[] partInfo;
		return FALSE;
	}

	//�¿������ļ�
	if (!Burn_DownloadImg(nID, hWnd, file_name, ModeCtrl, SectorSize, NandPhyInfo, partInfo))
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_DownloadImg fail!\r\n" );
		Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
		Burn_UnMountMedium(StartID, Pmedium);
		delete[] partInfo;
		return FALSE;
	}

    delete[] partInfo;

	if ((theConfig.planform_tpye == E_ROST_PLANFORM) && ((CHIP_1080L == theConfig.chip_type) || (CHIP_10XXC == theConfig.chip_type)))
	{
/*
		�ٶ�������û�����ܣ����Բ�֧���ļ���ֱ��д��, ֱ���Ǿ����д��
		if (!Burn_DownloadFileNew(nID, hWnd, file_name, StartID))
		{
			Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
			Burn_UnMountMedium(StartID, Pmedium);
			return FALSE;
		}	
*/
		//umount����
		Burn_UnMountMedium(StartID, Pmedium);
		
	}
	else
	{
		//�����ļ���u����
		if (!Burn_DownloadFile(nID, hWnd, file_name))
		{
			burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_DownloadFile fail!\r\n" );
			Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
			return FALSE;
		}	
	}

BURN_BOOT:
	if (!Burn_DownloadBoot(nID, hWnd, file_name))
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_DownloadBoot fail!\r\n" );
		Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
		return FALSE;
	}
	//��¼��д��MAC��ַ
	if (!Burn_WriteMACInfo(nID, hWnd, file_name, maclowbuf, mactempbuf, readmacflag, macmode))
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_WriteMACInfo fail!\r\n" );
		Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
		return FALSE;
	}

	//��¼���к�
	if (!Burn_WriteSERIALInfo(nID, hWnd, file_name, seriallowbuf, serialtempbuf, readSerialflag, Serialmode))
	{
		burnFile.WriteLogFile(LOG_LINE_TIME, "Burn_WriteSERIALInfo fail!\r\n" );
		Burn_Fail(seriallowbuf, maclowbuf, nID, readSerialflag, readmacflag);
		return FALSE;
	}


	PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_CLOSE_START));
    burnFile.WriteLogFile(LOG_LINE_TIME, "++write config++\r\n");
    if (BT_Close(nID) != BT_SUCCESS)
    {
        burnFile.WriteLogFile(LOG_LINE_TIME,  "BT_Close fail!\r\n");
		fNand_Free_BadBlockBuf(nID);
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_CLOSE_FAIL));
        return FALSE;
    }

	//test save info before sd fs 
	//PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_BIN_START));
	/*
	//������Ҫ�ǻض����е����ݷŵ�һ���ļ��ڣ���ֻ�ж�sd����
    if (0)//(MEDIUM_EMMC == ModeCtrl.eMedium)
	{
        T_UPLOAD_BIN pUploadBin;

		_tcscpy(pUploadBin.pc_path , theApp.ConvertAbsolutePath(_T("ALLDATA.upd")));
		memcpy(pUploadBin.file_name, "ALLDATA", DOWNLOAD_BIN_FILENAME_SIZE);

		if (BT_UploadBin(nID, &pUploadBin) != BT_SUCCESS)
        {
			fNand_Free_BadBlockBuf(nID);
			//PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_DOWNLOAD_BIN_FAIL));
			return FALSE;
            
        }
	}
	*/
BURN_SUCCESS:

	PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_TASK_COMPLETE));
   
	if (m_worknum != 0)
    {
        m_worknum--;
        if (0 == m_worknum)
        {
			//������ͨ�׵Ķ���ɺ��ٽ��������¼�
			SetEvent(ResetDevice_event);
			g_bEraseMode = FALSE; //�Ƿ�͸����п�
			m_budisk_burn = FALSE; //
			m_budisk_getUSBnum = FALSE; //u����¼ģʽ�µı�־
			USB_attachflag = FALSE;
			g_capacity_flag = AK_TRUE;
            pMain->enable_control();
        }
    }

	//����¼������ͨ���󣬽���ͳһ����
	if (!Burn_ReSetDev(nID, hWnd, file_name))
	{
		burnFile.WriteLogFile(LOG_LINE_TIME,  "Burn_ReSetDev fail!\r\n");
		fNand_Free_BadBlockBuf(nID);
		PostMessage(hWnd, ON_BURNFLASH_MESSAGE, WPARAM(nID+100), LPARAM(MESSAGE_CLOSE_FAIL));
        return FALSE;
	}
	
	g_download_mtd_flag[nID-1] = 0;
	g_bUpload_spialldata = FALSE;
	g_bUploadbinMode = FALSE;
	
    burnFile.WriteLogFile(LOG_LINE_TIME, "++Burn success++\r\n");
	pMain->StopCountTime(nID);
	fNand_Free_BadBlockBuf(nID);
	pMain->set_window_title();

    return TRUE;
}

void BurnProgress(UINT nID, UINT nDatLen)
{
	HWND hWnd = AfxGetMainWnd()->GetSafeHwnd();
	PostMessage(hWnd, ON_BURNFLASH_PROCESS, WPARAM(nID+100), nDatLen);
}

//�ض�bin
T_BOOL browser_for_binUpload(TCHAR *folderPath)
{
	BROWSEINFOW opp;
	
	opp.hwndOwner = AfxGetMainWnd()->GetSafeHwnd();
	opp.pidlRoot = NULL;
	opp.pszDisplayName = folderPath;
	opp.lpszTitle = _T("ѡ��һ���ļ��У�Ȼ����'ȷ��'");
	opp.ulFlags = BIF_RETURNONLYFSDIRS;
	opp.lpfn = NULL;
	opp.iImage = 0;

	LPITEMIDLIST pIDList = SHBrowseForFolder(&opp); //������ʾѡ��Ի���

	if (pIDList)
	{
	    SHGetPathFromIDList(pIDList, folderPath);
	    return TRUE;
	}
	else 
	{
		return FALSE;
	}
}
//mac��ַ��һ
BOOL Mac_Addr_add_1(TCHAR *buf, TCHAR *buf_temp)
{
	CHAR tempbuf[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};
	TCHAR tempAddrBuf[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};
	TCHAR dstAddrBuf[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};
	UINT tempmac;
	
	//�Ƚ�mac��ַ�Ƿ���ڽ�����mac��ַ
	if(_tcscmp(buf_temp, theConfig.mac_end_low) >= 0)
	{
		return FALSE;
	}
	
	_tcscpy(buf, buf_temp);
	//�Ե�λ����ʮ�����Ƽ�һ
	sprintf(tempbuf, "%c%c%c%c%c%c", buf_temp[0], buf_temp[1], buf_temp[3], buf_temp[4], buf_temp[6], buf_temp[7]);
	//��ַ����һ
	sscanf(tempbuf, "%x", &tempmac);
	tempmac ++;
	sprintf(tempbuf, "%06x", tempmac);
	swprintf(tempAddrBuf, _T("%c%c:%c%c:%c%c"), tempbuf[0],tempbuf[1],tempbuf[2],tempbuf[3],tempbuf[4],tempbuf[5]);
	theConfig.lower_to_upper(tempAddrBuf, dstAddrBuf);
	_tcscpy(theConfig.mac_current_low, dstAddrBuf);  //��¼��ǰ��mac��ַ

	return TRUE;
}

//���кŵ�ַ��1
BOOL serial_Addr_add_1(TCHAR *buf, TCHAR *buf_temp)
{
	UINT tempsequence;
	CString str;

	USES_CONVERSION;

	//�Ƚ����к��Ƿ���ڽ��������к�
	if(_tcscmp(buf_temp, theConfig.sequence_end_low) >= 0)
	{
		return FALSE;
	}
	_tcscpy(buf, buf_temp);
	//��ַ����һ
	tempsequence = atoi(T2A(buf_temp));
	tempsequence++;
	str.Format(_T("%06d"), tempsequence);
	_tcscpy(theConfig.sequence_current_low, str); //��¼��ǰ�����к�
	
	return TRUE;
}

BOOL Get_Mac_Addr(TCHAR *buf, UINT channelID)
{
	TCHAR tempAddrBuf[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};
	TCHAR tempcurrnetbuf[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};
	UINT i;

	if(WaitForSingleObject(theConfig.m_hMutGetAddr, 5000) != WAIT_OBJECT_0)
	{
		ReleaseMutex(theConfig.m_hMutGetAddr);
		return FALSE;
	}
    //���֮ǰ��Щͨ��������¼ʧ�ܵģ���ȡ��ͨ����mac��ַ
	if (theConfig.g_mac_current_low[channelID-1][0] != 0)
	{
		memcpy(buf, theConfig.g_mac_current_low[channelID-1], MAX_MAC_SEQU_ADDR_COUNT);
		memset(theConfig.g_mac_current_low[channelID-1], 0, MAX_MAC_SEQU_ADDR_COUNT+1);

		//����¼ʧ��ʱ���ٽ�����¼�����ȫ�ֱ���ֵ������ʼֵ�Ļ�����ô������ʼֵ
		if (_tcscmp(theConfig.mac_start_low, buf) > 0)
		{
			while(1)
			{
				if (!Mac_Addr_add_1(buf, theConfig.mac_current_low))
				{
					ReleaseMutex(theConfig.m_hMutGetAddr);
					return FALSE;
				}

				//�����ж�buf�Ƿ�ȫ0������Ǿ�������
				if(!is_zero_ether_addr(buf))
				{
					break;
				}

			}
		}

		//�Ƚ�mac��ַ�Ƿ���ڽ�����ֵ
		if(_tcscmp(buf, theConfig.mac_end_low) >= 0)
		{
			ReleaseMutex(theConfig.m_hMutGetAddr);
			return FALSE;
		}
	}
	else
	{
		for (i=0; i<32; i++)
		{
			//�ж��Ƿ���Щͨ������һ������¼ʧ�ܹ�
			if (theConfig.g_mac_current_low[i][0] != 0)
			{
				memcpy(buf, theConfig.g_mac_current_low[i], MAX_MAC_SEQU_ADDR_COUNT);
				memset(theConfig.g_mac_current_low[i], 0, MAX_MAC_SEQU_ADDR_COUNT+1);
				theConfig.read_currentdevicenum_addr(CONFIG_ADDR_FILE_NAME, maccurrentlow, NULL, tempcurrnetbuf, channelID-1);
				theConfig.write_config_addr(CONFIG_ADDR_FILE_NAME, maccurrentlow, passY, tempcurrnetbuf, i);
				theConfig.write_config_addr(CONFIG_ADDR_FILE_NAME, maccurrentlow, passN, buf, channelID-1);

				ReleaseMutex(theConfig.m_hMutGetAddr);
				return TRUE;
			}
		}

		while(1)
		{
			if (!Mac_Addr_add_1(buf, theConfig.mac_current_low))
			{
				ReleaseMutex(theConfig.m_hMutGetAddr);
				return FALSE;
			}

			//�����ж�buf�Ƿ�Ϸ���������Ϸ���������
			if(!is_zero_ether_addr(buf))
			{
				break;
			}

		}
	} 
	ReleaseMutex(theConfig.m_hMutGetAddr);

	return TRUE;
}

BOOL Get_serial_Addr(TCHAR *buf, UINT channelID)
{
	TCHAR tempcurrnetbuf[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};
	UINT  i;
	CString str;
	
	if(WaitForSingleObject(theConfig.m_hMutGetAddr, 5000) != WAIT_OBJECT_0)
	{
		ReleaseMutex(theConfig.m_hMutGetAddr);
		return FALSE;
	}

    //���֮ǰ��Щͨ��������¼ʧ�ܵģ���ȡ��ͨ�������к�  
	if (theConfig.g_sequence_current_low[channelID-1][0] != 0)
	{
		memcpy(buf, theConfig.g_sequence_current_low[channelID-1], MAX_MAC_SEQU_ADDR_COUNT);
		memset(theConfig.g_sequence_current_low[channelID-1], 0, MAX_MAC_SEQU_ADDR_COUNT+1);

		//����¼ʧ��ʱ���ٽ�����¼�����ȫ�ֱ���ֵ������ʼֵ�Ļ�����ô������ʼֵ
		if (_tcscmp(theConfig.sequence_start_low, buf) > 0)
		{
			if (!serial_Addr_add_1(buf, theConfig.sequence_current_low))
			{
				ReleaseMutex(theConfig.m_hMutGetAddr);
				return FALSE;
			}
		}
		//�Ƚ������Ƿ���ڽ�����ֵ
		if(_tcscmp(buf, theConfig.sequence_end_low) >= 0)
		{
			ReleaseMutex(theConfig.m_hMutGetAddr);
			return FALSE;
		}
	}
	else
	{
		for (i=0; i<32; i++)
		{
			//�ж��Ƿ���Щͨ������һ������¼ʧ�ܹ�
			if (theConfig.g_sequence_current_low[i][0] != 0)
			{
				memcpy(buf, theConfig.g_sequence_current_low[i], MAX_MAC_SEQU_ADDR_COUNT);
				memset(theConfig.g_sequence_current_low[i], 0, MAX_MAC_SEQU_ADDR_COUNT+1);
				theConfig.read_currentdevicenum_addr(CONFIG_ADDR_FILE_NAME, sequencecurrentlow, NULL, tempcurrnetbuf, channelID-1);
				theConfig.write_config_addr(CONFIG_ADDR_FILE_NAME, sequencecurrentlow, passY, tempcurrnetbuf, i);
				theConfig.write_config_addr(CONFIG_ADDR_FILE_NAME, sequencecurrentlow, passN, buf, channelID-1);

				ReleaseMutex(theConfig.m_hMutGetAddr);
				return TRUE;
			}
		}

		if (!serial_Addr_add_1(buf, theConfig.sequence_current_low))
		{
			ReleaseMutex(theConfig.m_hMutGetAddr);
			return FALSE;
		}
	} 
	ReleaseMutex(theConfig.m_hMutGetAddr);
	return TRUE;
}

//��¼ʧ�ܺ��¼��ǰ��ֵ
void Burn_Fail_setcurrent(TCHAR *serialtempbuf, TCHAR *mactempbuf, UINT channelID)
{
	if(WaitForSingleObject(theConfig.m_hMutGetAddr, 5000) != WAIT_OBJECT_0)
	{
		ReleaseMutex(theConfig.m_hMutGetAddr);
		return;
	}

	if (mactempbuf != NULL && theConfig.g_mac_current_low[channelID][0] == NULL)
	{
		//��¼ʧ��ʱ��¼��ͨ����ֵ
		_tcscpy(theConfig.g_mac_current_low[channelID], mactempbuf);
	}
	if (serialtempbuf != NULL && theConfig.g_sequence_current_low[channelID][0] == NULL)
	{
		//��¼ʧ��ʱ��¼��ͨ����ֵ
		_tcscpy(theConfig.g_sequence_current_low[channelID], serialtempbuf);
	}


	ReleaseMutex(theConfig.m_hMutGetAddr);

}

//��¼ʧ��
void Burn_Fail(TCHAR *serialtempbuf, TCHAR *mactempbuf, UINT nID, BOOL readSerialflag, BOOL readmacflag)
{
	if(!theConfig.bUpdate && (theConfig.sequenceaddr_flag || theConfig.macaddr_flag) && !readmacflag && !readSerialflag)
	{
		Burn_Fail_setcurrent(serialtempbuf, mactempbuf, nID-1);
	}
	
	if(!theConfig.bUpdate && theConfig.sequenceaddr_flag && !readSerialflag) 
	{
		//��¼ʧ��ʱ��¼��ͨ����ֵ���ĵ���
		theConfig.write_config_addr(CONFIG_ADDR_FILE_NAME, sequencecurrentlow, passN, serialtempbuf, nID-1);
	}
	if(!theConfig.bUpdate && theConfig.macaddr_flag && !readmacflag) 
	{
		//��¼ʧ��ʱ��¼��ͨ����ֵ���ĵ���
		theConfig.write_config_addr(CONFIG_ADDR_FILE_NAME, maccurrentlow, passN, mactempbuf, nID-1);
	}
	fNand_Free_BadBlockBuf(nID);
	USB_attachflag = FALSE;
	g_download_mtd_flag[nID-1] = 0;
}

//�ж�MACADDR�Ƿ���Ч
BOOL Macaddr_isunuse(char *buf, int len)
{
	int str_len = 0;
	int i = 0;
	int flag = 0;
	CHAR temphuf[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};

	USES_CONVERSION;

	memcpy(temphuf, buf, 8);

	//�Ƚϸ�λ���Ƿ����
	if(memcmp(temphuf, T2A(theConfig.mac_start_high), 8) != 0)
	{
		return FALSE;
	}

	str_len = strlen((char *)buf);
	if (str_len != len)
	{
		return FALSE;
	}
	else
	{		
		for (i = 0; i < len; i++)
		{
			//mac��ַ���⼸λ������":"
			if (i == 2 || i == 5 || i == 8 || i == 11 || i == 14)
			{
				if (buf[i] != ':')
				{
					return FALSE;
				}
			}
			else
			{
				//mac��ַ���������֣��������
				if (!isalnum(buf[i]))
				{
					return FALSE;
				}
			}
			
		}
	}

	return TRUE;
}

//�ж����к��Ƿ���Ч
BOOL Serialaddr_isunuse(char *buf, int len)
{

	int str_len = 0;
	int i = 0;
	CHAR temphuf[MAX_MAC_SEQU_ADDR_COUNT+1] = {0};
	
	USES_CONVERSION;

	memcpy(temphuf, buf, 10);
	
	//�Ƚ�
	if(memcmp(temphuf, T2A(theConfig.sequence_start_high), 10) != 0)
	{
		return FALSE;
	}

	str_len = strlen((char *)buf);
	if (str_len != len)
	{
		return FALSE;
	}
	else
	{
		for (i = 0; i < len; i++)
		{
			//���кű��������֣��������
			if (!isalnum(buf[i]))
			{
				return FALSE;
			}
		}
	}
	return TRUE;
}

//�����ǻض��ĺ���
E_NANDERRORCODE Reset_Nand_ExReadFlag(T_PNANDFLASH nand, T_U32 chip,
                              T_U32 block, T_U32 page, T_U8* oob,T_U32 oob_len)
{
	T_MEDIUM_RW_SECTOR_INFO info = {0};
    UINT ret_val = NF_FAIL;
	UINT ID = (UINT)nand->BufStart[0];
	UINT tmpBlock;
	UCHAR *ptr;
	
	//�����֪�ǿ��п��ֱ�ӿ���������Ҫ�ٶ�
	if (page == 0 && oob_len == MTD_OOB_LEN && m_pBuf_freeBlk[ID-1] != NULL)
	{
		tmpBlock = chip * nand->BlockPerPlane * nand->PlanePerChip + block;
		ptr = &m_pBuf_freeBlk[ID-1][tmpBlock*MTD_OOB_LEN];
		if (memcmp(ptr, m_mtd_oob_invalid, MTD_OOB_LEN) != 0)
		{
			memcpy(oob, &m_pBuf_freeBlk[ID-1][tmpBlock*MTD_OOB_LEN], oob_len);
			return NF_SUCCESS;
		}
	}

	info.chip  = chip; //chip id
	info.block = block; //��
	info.page  = page;  //ҳ
    info.oob_len = oob_len; // oob����

	if (BT_MediumReadFlag(ID, oob, &info, oob_len, &ret_val, 1) != BT_SUCCESS)
	{
		return NF_FAIL;
	}
	//��������OOB
	if (page == 0 && oob_len == MTD_OOB_LEN && m_pBuf_freeBlk[ID-1] != NULL)
	{
		tmpBlock = chip  * nand->BlockPerPlane * nand->PlanePerChip + block;
		memcpy(&m_pBuf_freeBlk[ID-1][tmpBlock*MTD_OOB_LEN], oob, oob_len);
	}

	return (E_NANDERRORCODE)ret_val;
}
                          
E_NANDERRORCODE Reset_Nand_ExReadSector(T_PNANDFLASH nand, T_U32 chip,
                              T_U32 plane_num, T_U32 block, T_U32 page,T_U8 data[], T_U8* spare_tbl,T_U32 oob_len, T_U32 page_num)//SpareTbl���ǰ���MutiPlaneNum��T_MTDOOB�����ָ��
{
	T_MEDIUM_RW_SECTOR_INFO info = {0};
    UINT ret_val;
	UINT ID = (UINT)nand->BufStart[0];
	UINT tmpBlock;

	info.chip      = chip;     //chip id
	info.plane_num = plane_num;  //plane��
    info.block     = block;      // ��
	info.page      = page;       //ҳ
	info.data      = data;        //BUF
	info.spare_tbl = spare_tbl;
	info.oob_len   = oob_len;
    info.page_num  = page_num;    //ҳ��
	info.lib_type  = 1;           //1��ʾexnftl �� 0��ʾnftl

	tmpBlock = chip * nand->BlockPerPlane * nand->PlanePerChip + block;

	if (BT_MediumReadSector(ID, &info, nand->BytesPerSector * page_num, &ret_val) != BT_SUCCESS)
	{
		//�����ʧ�ܲ����ǿ��п����0
		if (page == 0 && oob_len == MTD_OOB_LEN && m_pBuf_freeBlk[ID-1] != NULL)
		{
			memset(&m_pBuf_freeBlk[ID-1][tmpBlock*MTD_OOB_LEN], BLOCK_PAGE0_FLAG_INVALID, oob_len);
		}
		return NF_FAIL;
	}

	if (page == 0 && oob_len == MTD_OOB_LEN && m_pBuf_freeBlk[ID-1] != NULL)
	{
		/***ԭ������Ϊ��Ч��ԭ������Ϊ���ܻ����д�����ȡ����ʧ��***/
		memcpy(&m_pBuf_freeBlk[ID-1][tmpBlock*MTD_OOB_LEN], spare_tbl, oob_len);
	}

	return (E_NANDERRORCODE)ret_val;
}
                              
E_NANDERRORCODE Reset_Nand_ExWriteSector(T_PNANDFLASH nand, T_U32 chip,
                              T_U32 plane_num, T_U32 block, T_U32 page,const T_U8 data[], T_U8* spare_tbl,T_U32 oob_len, T_U32 page_num)    //SpareTbl���ǰ���MutiPlaneNum��T_MTDOOB�����ָ��
{
	T_MEDIUM_RW_SECTOR_INFO info = {0};
    UINT ret_val;
	UINT ID = (UINT)nand->BufStart[0];
	UINT tmpBlock;

	info.chip      = chip;         //chip id
	info.plane_num = plane_num;    //plane num
    info.block     = block;        //��
	info.page      = page;         //ҳ
	info.data      = (BYTE *)data; //buf
	info.spare_tbl = spare_tbl;    //oob buf
	info.oob_len   = oob_len;      //oob len
    info.page_num  = page_num;     //ҳ��
	info.lib_type  = 1;            //1��ʾexnftl �� 0��ʾnftl

	//
	tmpBlock = chip * nand->BlockPerPlane * nand->PlanePerChip + block;

	//�����д��ʱ����ô�ѻ�������0
	//memset(&m_pBuf_freeBlk[ID]+block*oob_len, 0, oob_len);

	if (BT_MediumWriteSector(ID, &info, nand->BytesPerSector * page_num, &ret_val) != BT_SUCCESS)
	{
		//���дʧ�ܲ����ǿ��п����0
		if (page == 0 && oob_len == MTD_OOB_LEN && m_pBuf_freeBlk[ID-1] != NULL)
		{
			memset(&m_pBuf_freeBlk[ID-1][tmpBlock*MTD_OOB_LEN], BLOCK_PAGE0_FLAG_INVALID, oob_len);
		}
		return NF_FAIL;
	}

	if (page == 0 && oob_len == MTD_OOB_LEN && m_pBuf_freeBlk[ID-1] != NULL)
	{
		/***ԭ������Ϊ��Ч��ԭ������Ϊ���ܻ����д�����ȡ����ʧ��***/
		memcpy(&m_pBuf_freeBlk[ID-1][tmpBlock*MTD_OOB_LEN], spare_tbl, oob_len);
	}

	return (E_NANDERRORCODE)ret_val;
}
   
E_NANDERRORCODE Reset_Nand_ExEraseBlock(T_PNANDFLASH nand, T_U32 chip, T_U32 plane_num, T_U32 block)
{
	T_MEDIUM_RW_SECTOR_INFO info = {0};
    UINT ret_val;
	UINT ID = (UINT)nand->BufStart[0];
	UINT tmpBlock;

	info.chip      = chip;  //chip id
	info.plane_num = plane_num;  //plane num 
    info.block     = block;      //��
	info.lib_type  = 1;        //1��ʾexnftl 0��ʾnftl


	if (BT_MediumEraseBlock(ID, &info, &ret_val, 1) != BT_SUCCESS)
	{
		//�����ʧ�ܲ����ǿ��п����0
		if (m_pBuf_freeBlk[ID-1] != NULL)
		{
			tmpBlock = chip  * nand->BlockPerPlane * nand->PlanePerChip + block;
			memset(&m_pBuf_freeBlk[ID-1][tmpBlock*MTD_OOB_LEN], BLOCK_PAGE0_FLAG_INVALID, MTD_OOB_LEN);
		}
		return NF_FAIL;
	}

	//�ж��Ƿ���п�
	if (m_pBuf_freeBlk[ID-1] != NULL)
	{
		tmpBlock = chip  * nand->BlockPerPlane * nand->PlanePerChip + block;
		memset(&m_pBuf_freeBlk[ID-1][tmpBlock*MTD_OOB_LEN], BLOCK_PAGE0_FLAG_VALID, MTD_OOB_LEN);
	}
	

	return (E_NANDERRORCODE)ret_val;
}

//���������������а����е�block��oob������ȡ����
T_BOOL fNand_Get_FreeBlockBuf(UINT nID, T_U32 chip, T_U32 BlockNum, T_U32 nChipCnt, UINT StartBlock)
{
	UINT ooblen = MTD_OOB_LEN;
	UINT i = 0;
	UINT Blocklen = BlockNum * nChipCnt;
	UINT freebuflen = ooblen * Blocklen;
	UCHAR *ret_val = NULL;
	UINT  *ptr;
	T_MEDIUM_RW_SECTOR_INFO info = {0};
	
	info.chip  = 0;          //chip id
	info.block = StartBlock;  //��
	info.page  = 0;           //��0ҳ
    info.oob_len = MTD_OOB_LEN; //oob ����
	
	m_pBuf_freeBlk[nID-1] = (UCHAR *)malloc(freebuflen);
    if (AK_NULL == m_pBuf_freeBlk[nID-1])
    {
        return FALSE;
    }

	ret_val = (UCHAR *)malloc(Blocklen*4);
    if (NULL == ret_val)
    {
        return FALSE;
    }

	memset(m_pBuf_freeBlk[nID-1], BLOCK_PAGE0_FLAG_INVALID, freebuflen);
	memset(ret_val, 0, Blocklen*4);

	if (BT_Medium_Read_FreeBlockBuf(nID, m_pBuf_freeBlk[nID-1], &info, ooblen, ret_val, BlockNum * nChipCnt - StartBlock) != BT_SUCCESS)
	{
		free(m_pBuf_freeBlk[nID-1]);
		m_pBuf_freeBlk[nID-1] = NULL;
		free(ret_val);
		ret_val = NULL;
		return FALSE;
	}

	ptr = (UINT *)ret_val;
	for ( i = StartBlock; i < BlockNum * nChipCnt; i++)
	{
		//�жϷ���ֵ�Ƿ�ɹ�
		if (ptr[i] != NF_SUCCESS)
		{
			memset((m_pBuf_freeBlk[nID-1] + i*ooblen), BLOCK_PAGE0_FLAG_INVALID, ooblen);
		}
	}

	free(ret_val);
	ret_val = NULL;
	
	return TRUE;
}

//�ͷ��ڴ�
void fNand_Free_BadBlockBuf(UINT nID)
{

	if (m_pBuf_BadBlk[nID-1] != NULL)
	{
		free(m_pBuf_BadBlk[nID-1]);
		m_pBuf_BadBlk[nID-1] = NULL;
	}

	if (m_pBuf_freeBlk[nID-1] != NULL)
	{
		free(m_pBuf_freeBlk[nID-1]);
		m_pBuf_freeBlk[nID-1] = NULL;
	}
}

//���������������а����е�bab block������ȡ����
T_BOOL fNand_Get_BadBlockBuf(UINT nID, T_U32 BlockNum, T_U32 nChipCnt, T_MODE_CONTROL *ModeCtrl)
{
	if ((theConfig.planform_tpye == E_ROST_PLANFORM) 
		&& ((CHIP_1080L == theConfig.chip_type) || (CHIP_10XXC == theConfig.chip_type)) 
		&& (TRANSC_MEDIUM_NAND == ModeCtrl->eMedium || TRANSC_MEDIUM_SPI_NAND == ModeCtrl->eMedium))
	{
		UINT badbuflen = ((BlockNum * nChipCnt) >> 3);

		m_pBuf_BadBlk[nID-1] = (UCHAR *)malloc(badbuflen);
		if (AK_NULL == m_pBuf_BadBlk[nID-1])
		{
			return FALSE;
		}
		memset(m_pBuf_BadBlk[nID-1], 0, badbuflen);
		
		if (BT_Medium_Get_BadBlockBuf(nID, m_pBuf_BadBlk[nID-1],  badbuflen) != BT_SUCCESS)
		{
			return FALSE;
		}
	}
	
	return TRUE;
}


T_BOOL Reset_fNand_ExIsBadBlock(T_PNANDFLASH nand, T_U32 chip, T_U32 block )
{
	//T_MEDIUM_RW_SECTOR_INFO info = {0};
	T_U32 byte_loc, byte_offset;
	UINT ID = (UINT)nand->BufStart[0];  //��һ��ͨ��
/*
	info.chip      = chip;
    info.block     = block;

	if (BT_MediumIsBadBlock((UINT)nand->BufStart[0], &info, &ret_val) != BT_SUCCESS)
	{
		return FALSE;
	}
*/
	if(AK_NULL != m_pBuf_BadBlk[ID-1])
	{
		byte_loc = block / 8;
		byte_offset = 7 - block % 8;
		
		if(m_pBuf_BadBlk[ID-1][byte_loc] & (1 << byte_offset))
		{
			return TRUE;
		}
	}

	return FALSE;
}

T_BOOL Reset_fNand_ExSetBadBlock(T_PNANDFLASH nand, T_U32 chip, T_U32 block)
{
	T_MEDIUM_RW_SECTOR_INFO info = {0};
    UINT ret_val;
	T_U32 byte_loc, byte_offset;
	UINT ID = (UINT)nand->BufStart[0];

	info.chip      = chip;  // chip id
    info.block     = block; // ��

	if (BT_MediumSetBadBlock(ID, &info, &ret_val) != BT_SUCCESS)
	{
		return FALSE;
	}
	
	//��¼���鵽m_pBuf_BadBlk��
	if(AK_NULL != m_pBuf_BadBlk[ID-1])
    {
        byte_loc = block / 8;
        byte_offset = 7 - block % 8;
        m_pBuf_BadBlk[ID-1][byte_loc] |= 1 << byte_offset;
    }
	

	return ret_val;
}
/******************************************************************
NFTL ר�õĺ���
******************************************************************/
E_NANDERRORCODE Reset_Nand_WriteSector(T_PNANDFLASH nand, T_U32 chip,
                              T_U32 block, T_U32 page,const T_U8 data[], T_U8* spare_tbl,T_U32 oob_len)    //SpareTbl���ǰ���MutiPlaneNum��T_MTDOOB�����ָ��
{
	T_MEDIUM_RW_SECTOR_INFO info = {0};
    UINT ret_val;
	UINT ID = (UINT)nand->BufStart[0];
	UINT tmpBlock;

	info.chip      = chip;          //chip id
//	info.plane_num = plane_num;
    info.block     = block;         //��
	info.page      = page;          //ҳ
	info.data      = (BYTE *)data;  //data buf
	info.spare_tbl = spare_tbl;  //oob buf
	info.oob_len   = oob_len;   //oob len
//    info.page_num  = page_num;
	info.lib_type  =0;

	tmpBlock = chip  * nand->BlockPerPlane * nand->PlanePerChip + block;

	if (BT_MediumWriteSector(ID, &info, nand->BytesPerSector, &ret_val) != BT_SUCCESS)
	{
		//���дʧ�ܵ�����£��������Ϊ����
		if (page == 0 && oob_len == MTD_OOB_LEN && m_pBuf_freeBlk[ID-1] != NULL)
		{
			memset(&m_pBuf_freeBlk[ID-1][tmpBlock*MTD_OOB_LEN], BLOCK_PAGE0_FLAG_INVALID, oob_len);
		}
		return NF_FAIL;
	}
	
	if (page == 0 && oob_len == MTD_OOB_LEN && m_pBuf_freeBlk[ID-1] != NULL)
	{
		/***ԭ������Ϊ��Ч��ԭ������Ϊ���ܻ����д�����ȡ����ʧ��***/
		memcpy(&m_pBuf_freeBlk[ID-1][tmpBlock*MTD_OOB_LEN], spare_tbl, oob_len);
	}

	return (E_NANDERRORCODE)ret_val;
}

E_NANDERRORCODE Reset_Nand_ReadSector(T_PNANDFLASH nand, T_U32 chip,
                              T_U32 block, T_U32 page,T_U8 data[], T_U8* spare_tbl,T_U32 oob_len)//SpareTbl���ǰ���MutiPlaneNum��T_MTDOOB�����ָ��
{
	T_MEDIUM_RW_SECTOR_INFO info = {0};
    UINT ret_val;
	UINT ID = (UINT)nand->BufStart[0];
	UINT tmpBlock;

	info.chip      = chip;
//	info.plane_num = plane_num;
    info.block     = block;
	info.page      = page;
	info.data      = data;
	info.spare_tbl = spare_tbl;
	info.oob_len   = oob_len;
//    info.page_num  = page_num;
	info.lib_type  =0;    //1��ʾexnftl �� 0��ʾnftl

	tmpBlock = chip  * nand->BlockPerPlane * nand->PlanePerChip + block;

	if (BT_MediumReadSector((UINT)nand->BufStart[0], &info, nand->BytesPerSector, &ret_val) != BT_SUCCESS)
	{
		//�����ʧ�ܵ�����£��������Ϊ����
		if (page == 0 && oob_len == MTD_OOB_LEN && m_pBuf_freeBlk[ID-1] != NULL)
		{
			memset(&m_pBuf_freeBlk[ID-1][tmpBlock*MTD_OOB_LEN], BLOCK_PAGE0_FLAG_INVALID, oob_len);
		}
		return NF_FAIL;
	}

	if (page == 0 && oob_len == MTD_OOB_LEN && m_pBuf_freeBlk[ID-1] != NULL)
	{
		/***ԭ������Ϊ��Ч��ԭ������Ϊ���ܻ����д�����ȡ����ʧ��***/
		memcpy(&m_pBuf_freeBlk[ID-1][tmpBlock*MTD_OOB_LEN], spare_tbl, oob_len);
	}

	return (E_NANDERRORCODE)ret_val;
}

E_NANDERRORCODE Reset_Nand_EraseBlock(T_PNANDFLASH nand, T_U32 chip, T_U32 block)
{
	T_MEDIUM_RW_SECTOR_INFO info = {0};
    UINT ret_val;
	UINT ID = (UINT)nand->BufStart[0];
	UINT tmpBlock;

	info.chip      = chip;
	//info.plane_num = 1;
    info.block     = block;
	info.lib_type  =0;  //1��ʾexnftl �� 0��ʾnftl

	

	if (BT_MediumEraseBlock(ID, &info, &ret_val, 1) != BT_SUCCESS)
	{
		//�����ʧ�ܵ�����£��������Ϊ����
		if (m_pBuf_freeBlk[ID-1] != NULL)
		{
			tmpBlock = chip  * nand->BlockPerPlane * nand->PlanePerChip + block;
			memset(&m_pBuf_freeBlk[ID-1][tmpBlock*MTD_OOB_LEN], BLOCK_PAGE0_FLAG_INVALID, MTD_OOB_LEN);
		}
		return NF_FAIL;
	}

	//������ɹ�������£��������Ϊ����
	if (m_pBuf_freeBlk[ID-1] != NULL)
	{
		tmpBlock = chip  * nand->BlockPerPlane * nand->PlanePerChip + block;
		memset(&m_pBuf_freeBlk[ID-1][tmpBlock*MTD_OOB_LEN], BLOCK_PAGE0_FLAG_VALID, MTD_OOB_LEN);
	}

	return (E_NANDERRORCODE)ret_val;
}


T_U32 Reset_Medium_ReadSector(T_PMEDIUM medium, T_U8* buf, T_U32 start, T_U32 size)
{
	T_MEDIUM_RW_SECTOR_INFO info = {0};
    UINT ret_val;
	T_PC_MEDIUM *p_medium = (T_PC_MEDIUM *)((UCHAR *)medium+sizeof(T_MEDIUM));
	UINT ID = p_medium->ID;//(UINT)nand->BufStart[0];
//	UINT tmpBlock;
	
	//info.chip      = chip;
	//info.plane_num = plane_num;
   // info.block     = block;
	info.page      = start; // ҳ
	info.data      = buf;   //BUF
	//info.spare_tbl = spare_tbl;
	//info.oob_len   = oob_len;
    info.page_num  = size;   //ҳ��
	//info.lib_type  = 1;
	

	if (BT_MediumReadSector(ID, &info, (1<<medium->SecBit) * size, &ret_val) != BT_SUCCESS)
	{
		return NF_FAIL;
	}
	
	return (T_U32)ret_val;
}

T_U32 Reset_Medium_WriteSector(T_PMEDIUM medium, const T_U8 *buf, T_U32 start, T_U32 size)
{
	
	T_MEDIUM_RW_SECTOR_INFO info = {0};
    UINT ret_val;
	T_PC_MEDIUM *p_medium = (T_PC_MEDIUM *)((UCHAR *)medium+sizeof(T_MEDIUM));
	UINT ID = p_medium->ID; //(UINT)nand->BufStart[0];
	UINT max_idex = 0;
	UINT writelen = 0;
	UINT idex = 1;
	UINT temp_ret = 0;
	UINT ret_secnt = 0;
	UINT i = 0;
	BOOL flag = FALSE;//����������16�����ֲ���16��λ��ʱ
	//	UINT tmpBlock;
	
	//	info.chip      = chip;
	//	info.plane_num = plane_num;
    //info.block     = block;
	info.page      = start;        //ҳ
	info.data      = (BYTE *)buf;   //BUF
	//	info.spare_tbl = spare_tbl;
	//info.oob_len   = oob_len;
    info.page_num  = size;           //ҳ��
	//info.lib_type  = 1;


	//if (BT_MediumWriteSector(ID, &info, (1<<medium->SecBit)*size, &ret_val) != BT_SUCCESS)
	if (BT_MediumWriteSector(ID, &info, (1<<medium->SecBit) * size, &ret_val) != BT_SUCCESS)
	{
		return NF_FAIL;
	}
	

	return (T_U32)ret_val;
}


T_VOID Reset_Medium_DeleteSec(T_PMEDIUM medium,T_U32 StartSce,T_U32 SecSize)
{
	return;
}
T_BOOL Reset_Medium_Flush(T_PMEDIUM medium)
{
	return AK_TRUE;
}

VOID Burn_ResetNandBase(UINT nID, T_PNANDFLASH nand)
{
	nand->BufStart[0]  = nID;/*Ŀǰ����λ�����ͨ��������Ժ�ṹ��ɾ������ô���Զ�������һ�����������*/
	nand->ExReadFlag   = Reset_Nand_ExReadFlag;
	nand->ExRead       = Reset_Nand_ExReadSector;
	nand->ExWrite      = Reset_Nand_ExWriteSector;
	nand->ExEraseBlock = Reset_Nand_ExEraseBlock;
	nand->ExIsBadBlock = Reset_fNand_ExIsBadBlock;
	nand->ExSetBadBlock = Reset_fNand_ExSetBadBlock;
	/**���ڵͼ���ʽ����ʱ���ǵ����������ӿ����Ծ�ֱ��Ӧ����**/
	nand->ReadFlag   = Reset_Nand_ExReadFlag;
	nand->ReadSector   = Reset_Nand_ReadSector;
	nand->WriteSector  = Reset_Nand_WriteSector;
	nand->EraseBlock   = Reset_Nand_EraseBlock;
	nand->IsBadBlock   = Reset_fNand_ExIsBadBlock;
	nand->SetBadBlock  = Reset_fNand_ExSetBadBlock;
	
}
/*
VOID Burn_ResetMedium(UINT nID, T_PMEDIUM medium)
{
	medium->read = Reset_Medium_ReadSector;
	medium->write = Reset_Medium_WriteSector;
	medium->DeleteSec = AK_NULL;
	medium->flush = Reset_Medium_Flush;
}
*/

T_VOID Burn_Free_Medium(T_PMEDIUM medium)
{
	if (medium != AK_NULL)
	{
		free((T_PMEDIUM)medium);
		medium = NULL;
	}
}

T_PMEDIUM Burn_Malloc_Medium(UINT ID, UINT secsize, UINT capacity, UINT SecPerPg)
{
    T_PMEDIUM medium = NULL;
	T_PC_MEDIUM *pc_medium = NULL;
    int i = 0;
    
    medium = (T_PMEDIUM)malloc(sizeof(T_MEDIUM) + sizeof(T_PC_MEDIUM));
    if (medium == NULL)
    {
        return NULL;
    }

	pc_medium = (T_PC_MEDIUM *)((UCHAR *)medium + sizeof(T_MEDIUM));
	pc_medium->medium = medium;
	pc_medium->ID = ID;

    memset(medium,0,sizeof(T_MEDIUM));

    
    i = 0;
    while (secsize > 1)
    { 
        secsize >>= 1;
        i++;
    }
    
    medium->SecBit =(T_U8) i;
    
    i = 0;
    while (SecPerPg > 1)
    {
        SecPerPg >>= 1;
        i++;
    }
    medium->SecPerPg = i;	
    medium->PageBit =(T_U8) (i + medium->SecBit);
    //((T_POBJECT)media)->destroy = NULL;//(F_DESTROY)Medium_Destroy;
    //((T_POBJECT)media)->type = TYPE_MEDIUM;
    medium->read = Reset_Medium_ReadSector;
    medium->write = Reset_Medium_WriteSector;	
    medium->flush = Reset_Medium_Flush;
    medium->DeleteSec = NULL;
    
    medium->capacity = capacity;    //����Ϊ��λ
    medium->type = MEDIUM_SD;
    medium->msg = NULL;
   
	return (T_PMEDIUM)medium;
}
