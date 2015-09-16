// Update.h: interface for the CUpdate class.
//
//* @date 2009/11/11
//
// * @version 1.0
//
// * @author Lu Qiliu.
//
// * Copyright 2009 Anyka corporation, Inc. All rights reserved.
//
// * ANYKA PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
// */
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UPDATE_H__55137F11_4698_4E0D_8E3A_4B119C49AFF8__INCLUDED_)
#define AFX_UPDATE_H__55137F11_4698_4E0D_8E3A_4B119C49AFF8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
//format��Ϣ����
//tagUpdateFileHead
//
//


//T_UPDATE_FILE_HEAD
//�ļ�ͷ��Ϣ���̶�Ϊ"anyka106"
//format��Ϣ����
//format��Ϣƫ����
//nand��Ϣƫ����
 //spi��Ϣ����
//spi��Ϣƫ����
//producerƫ����
//producerƫ����
//producer�ļ���С
//biosƫ����
//bios�ļ���С
//udiskĿ¼��Ϣ����
//udiskĿ¼��Ϣƫ����
//udisk�ļ�����
typedef struct tagUpdateFileHead
{
	BYTE head_info[8];			//�ļ�ͷ��Ϣ���̶�Ϊ"anyka106"
	UINT format_count;			//format��Ϣ����
    UINT format_offset;			//format��Ϣƫ����
	UINT nand_count;			//nand��Ϣ����
    UINT nand_offset;			//nand��Ϣƫ����
	//UINT spi_count;			    //spi��Ϣ����
    //UINT spi_offset;			//spi��Ϣƫ����
	UINT producer_offset;		//producerƫ����
	DWORD producer_size;		//producer�ļ���С
	UINT bios_offset;			//biosƫ����
	DWORD bios_size;			//bios�ļ���С
	UINT udisk_info_count;		//udiskĿ¼��Ϣ����
	UINT udisk_info_offset;		//udiskĿ¼��Ϣƫ����
	UINT udisk_file_count;		//udisk�ļ�����
	UINT udisk_file_info_offset;//udisk�ļ���Ϣƫ����
	UINT file_count;			//nand�ļ�����
    UINT file_info_offset;		//nand��Ϣƫ����
	UINT config_tool[8][2];		//���ù����ļ���Ϣʹ��,config_tool[0]--ImageRse,config_tool[1]--PROG
	//��¼������Ϣʹ��: 
	//Other_config[0]:Usb mode
	//Other_config[1]:NonFS reserve size
	//Other_config[2]:fs reserve size
	//Other_config[3]:volumelable data offset
	//Other_config[4]:bUpdateself
	//Other_config[5]:snowbirdsE
    //Other_config[6]:burn_mode
	UINT Other_config[8];	//	
	UINT check_sum;	//			
}
T_UPDATE_FILE_HEAD;

//T_FILE_INFO
//�Ƿ�Ƚ�
//���ӵ�ַ
//�ļ�����
//�ļ�ƫ����
//�ļ������洢���豸���ļ�����
typedef struct tagFileInfo
{
	BYTE bCompare;				//�Ƿ�Ƚ�
	BYTE bBack;                 //�Ƿ񱸷�
	BYTE bin_len;               //bin����С
	BYTE resev;                 //δ�ã�����
	UINT ld_addr;				//���ӵ�ַ
	UINT file_length;			//�ļ�����
    UINT file_offset;			//�ļ�ƫ����
	CHAR file_name[16];			//�ļ������洢���豸���ļ�����
	UINT check_sum;//
}
T_FILE_INFO;

//T_FILE_INFO
//�Ƿ�Ƚ�
//���ӵ�ַ
//�ļ�����
//�ļ�ƫ����
//�ļ������洢���豸���ļ�����
typedef struct tagFileInfo_linux
{
	BYTE bCompare;				//�Ƿ�Ƚ�
	BYTE bBack;                 //�Ƿ񱸷�
	BYTE resev1;               
	BYTE resev2;                 //δ�ã�����
	UINT ld_addr;				//���ӵ�ַ
	UINT file_length;			//�ļ�����
    UINT file_offset;			//�ļ�ƫ����
	CHAR file_name[16];			//�ļ������洢���豸���ļ�����
	FLOAT bin_len;              //bin����С
	UINT check_sum;              
}
T_FILE_INFO_LINUX;

//T_UDISK_INFO
//�Ƿ�Ƚ�
//pc·��
//udisk·��
typedef struct tagUdiskInfo
{
	UINT bCompare;				//�Ƿ�Ƚ�
	CHAR pc_path[MAX_PATH];		//pc·��
	CHAR udisk_path[MAX_PATH];	//udisk·��
	UINT check_sum;//
}
T_UDISK_INFO;

//T_UDISK_UPDATE_FILE_INFO
//pc�������ļ�·��
//�ļ�����
//�ļ�ƫ����
typedef struct tagUdiskFileInfo
{
	CHAR pc_file_path[MAX_PATH];//pc�������ļ�·��
	UINT file_length;			//�ļ�����
	UINT file_offset;			//�ļ�ƫ����
	UINT check_sum;//
}
T_UDISK_UPDATE_FILE_INFO;

//����CUpdate

class CUpdate  
{
public:
	CUpdate();//
	virtual ~CUpdate();//

    BOOL ExportUpdateFile(CConfig *pCFG, CString file_path, CString strCheck);//
    BOOL ImportUpdateFile(CConfig *pCFG, CString file_path);//

protected:
	BOOL CreateUpdateDir();//�����ļ���
	BOOL ReadBuffer(HANDLE hFile, LPVOID buf, UINT begin, UINT size);//��
	BOOL WriteBuffer(HANDLE hFile, LPVOID buf, UINT begin, UINT size);//д
    UINT Cal_CheckSum(PBYTE data, UINT len); //���
	UINT GetFileCnt(CString path, UINT* pFileCnt);//��ȡ�ļ�����
	UINT GetUdiskFileCount(CConfig *pCFG);//��ȡu���ļ�
	BOOL StoreFile(HANDLE hPacketFile, TCHAR* file_path, UINT file_offset, UINT file_len);//�洢�ļ�
    BOOL StoreProducer(HANDLE hPacketFile, CConfig *pCFG, T_UPDATE_FILE_HEAD *pFileHead);//produce
	BOOL StoreBios(HANDLE hPacketFile, CConfig *pCFG, T_UPDATE_FILE_HEAD *pFileHead);//bois
	BOOL StoreNandFile(HANDLE hPacketFile, CConfig *pCFG, T_UPDATE_FILE_HEAD *pFileHead);//nand
	BOOL StoreNandFile_linux(HANDLE hPacketFile, CConfig *pCFG, T_UPDATE_FILE_HEAD *pFileHead);
	BOOL StoreUdiskFile(HANDLE hPacketFile, CConfig *pCFG, T_UPDATE_FILE_HEAD *pFileHead);//u��
	void SetUdiskFileInfo(CString strPath, T_UDISK_UPDATE_FILE_INFO *pUdisFileInfo, UINT *pFileCnt);
	
	BOOL Del_UpdateFiles(LPCTSTR file_path);
	BOOL UnpacketFile(HANDLE hSourceFile, LPCTSTR file_path, UINT file_offset, UINT file_length);//�����ļ�
	BOOL UnpacketUdiskFile(HANDLE hSourceFile, T_UPDATE_FILE_HEAD *pFileHead, T_DOWNLOAD_UDISK *pDownloadUdisk);//����nand
	BOOL UnpacketNandFile(HANDLE hSourceFile, T_UPDATE_FILE_HEAD *pFileHead, T_DOWNLOAD_NAND* pDownloadNand);
	BOOL UnpacketNandFile_linux(HANDLE hSourceFile, T_UPDATE_FILE_HEAD *pFileHead, T_DOWNLOAD_NAND* pDownloadNand);

	BOOL WriteCheckExport(HANDLE hFile, CString strCheck);
    BOOL CUpdate::UnpacketCheckExport(HANDLE hSourceFile, BYTE str[]);
private:
	UINT m_nand_file_offset;//
	CString m_update_folder;//
};

#endif // !defined(AFX_UPDATE_H__55137F11_4698_4E0D_8E3A_4B119C49AFF8__INCLUDED_)
