// ImageCreate.cpp: implementation of the CImageCreate class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
//#include "AKFS.h"

extern "C"
{
	#include "anyka_types.h"
	#include "mtdlib.h"
    #include "medium.h"
    #include "fs.h"
	#include "eng_dataconvert.h"
}

#include "ImageCreate.h"
#include "config.h"

#ifdef CRC_CHECK
#define CHECK_IMG_FILE_POS	200
#define CHECK_FLAG_SIZE		4
BYTE CHECK_FLAG[CHECK_FLAG_SIZE] = {'C','H','C','K'};
extern CConfig theConfig;
#endif
T_U32 M_Write(T_PMEDIUM medium, const T_U8* buf, T_U32 sector, T_U32 size);
T_U32 M_Read(T_PMEDIUM medium, T_U8* buf, T_U32 sector, T_U32 size);
T_BOOL M_Flush(T_PMEDIUM medium);


//////////////////////////////////////////////////////////////////////////

HANDLE m_hFile = INVALID_HANDLE_VALUE;

T_U32 DriverCapacity = 0;

//////////////////////////////////////////////////////////////////////////


CImageCreate::CImageCreate()
{
	DriverCapacity = 0;
	TotalFCapacity = 0;
	bAddVolumeLable = FALSE;
}

CImageCreate::~CImageCreate()
{
	//img_destroy();
}

BOOL CImageCreate::fslib_init()
{
//�ŵ������ʼ����ʱ���ʼ����
	return TRUE;
}

BOOL CImageCreate::img_create(LPTSTR strPath, T_U32 capacity, T_U32 BytsPerSec, T_U32 BytsPerPage, T_U32 MediumType, char driverName)
{
    T_PMEDIUM medium = NULL;
	T_U32 low  = 0;
	T_U32 high = 0;
	T_IMAGE img;
    T_U8 DriverID  = 0;
    T_U8 StartID   = 0;
    T_U8 DricerCnt = 0;
    T_MEDIUM r_meidum;
    DRIVER_INFO DriverInfo = {0};

#ifdef CRC_CHECK
	memset(img.FileCheckValue, 0, CHECK_XOR_LEN);
#endif

	int i = 0, j = 0;
	
    if (BytsPerSec % 512)//������С�Ƿ���512����
    {
        return FALSE;
    }
    
	/* create mirror file */
	DWORD faConfig = GetFileAttributes(strPath); //����
	if(0xFFFFFFFF != faConfig)
	{
		faConfig &= ~FILE_ATTRIBUTE_READONLY;//ֻ��
		faConfig &= ~FILE_ATTRIBUTE_SYSTEM;
		faConfig &= ~FILE_ATTRIBUTE_TEMPORARY;
		SetFileAttributes(strPath, faConfig);
	}

	img.hFile = CreateFile(strPath, GENERIC_WRITE | GENERIC_READ,
					     FILE_SHARE_WRITE | FILE_SHARE_READ,
					     NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);//����
	if (INVALID_HANDLE_VALUE == img.hFile)
	{
        return FALSE;
	}

//	Driver_SetDefault(0);


    if (E_CONFIG_NAND == MediumType || MediumType == E_CONFIG_SPI_NAND)
    {
        DriverInfo.nMainType = MEDIUM_NANDFLASH;//nand
    }
    else
    {
        DriverInfo.nMainType = MEDIUM_SD;//sd
    }

    r_meidum.read  = M_Read;//��
    r_meidum.write = M_Write;//д

    j = 0;
    while (BytsPerPage > BytsPerSec)
    {
        BytsPerPage >>= 1;
        j++;
    }

    //r_meidum.capacity  = capacity * ((1024 * 1024) / BytsPerSec);  //��MΪ��λ
	r_meidum.capacity  = capacity * ((1024 * 16) / BytsPerSec)/16;    //��KΪ��λ

    //��������λ��������medium�Ľṹ��Ĳ���
	if (DriverInfo.nMainType == MEDIUM_SD)
	{
		DriverInfo.nBlkCnt  = r_meidum.capacity;//����
		DriverInfo.nBlkSize = BytsPerSec;//����
		DriverInfo.fRead = M_Read;
		DriverInfo.fWrite = M_Write;
	}
	else
	{
		DriverInfo.nBlkCnt  = 0;
		DriverInfo.nBlkSize = 0;

		i = 0;
		while (BytsPerSec > 1)
		{
			BytsPerSec >>= 1;
			i++;
		}

		r_meidum.SecBit   = (T_U8)i;//��λ
		r_meidum.PageBit  = (T_U8)(i + j);//ҳλ
		r_meidum.SecPerPg = (T_U8)j;//

		DriverInfo.medium = &r_meidum;//
	}
    DriverID = (T_U8)(driverName - 'A');//
    m_hFile = img.hFile;
    StartID = FS_MountMemDev(&DriverInfo, &DricerCnt, DriverID);//mount��
    if (T_U8_MAX == StartID)
    {
		CloseHandle(img.hFile);//
        m_hFile = INVALID_HANDLE_VALUE;//
        return FALSE;
    }
    DriverInfo.medium->msg  = (T_U8 *)(img.hFile);//
    m_hFile = INVALID_HANDLE_VALUE;

    if (!FS_GetDriver(&DriverInfo, DriverID))//������
    {
		CloseHandle(img.hFile);//
        return FALSE;
    }

    medium = DriverInfo.medium;//
    if (!FS_FormatDriver(DriverID, FAT_FS_ERROR))//��ʽ��
    {
		CloseHandle(img.hFile);
        return FALSE;
    }
	low  = FS_GetDriverFreeSize(driverName - 'A', &high);//��ȡ������С
	DriverCapacity = high + (low >> medium->SecBit);//
	FileSize = 0;
	//img.driver = driver;
	img.diskname = driverName;//

	m_image_array.Add(img);//

	return TRUE;
}

void CImageCreate::img_destroy()
{
	int count;
	int i;

	count = m_image_array.GetSize();//��С

	for(i = 0; i < count; i++)
	{
#ifdef CRC_CHECK
		DWORD writeLen;
		BYTE check_buf[CHECK_FLAG_SIZE + CHECK_XOR_LEN];//

		if (!bAddVolumeLable)
		{
			SetFilePointer(m_image_array[i].hFile, CHECK_IMG_FILE_POS, NULL, FILE_BEGIN);//

			memcpy(check_buf, CHECK_FLAG, CHECK_FLAG_SIZE);//

			memcpy(check_buf + CHECK_FLAG_SIZE, m_image_array[i].FileCheckValue, CHECK_XOR_LEN);//

			WriteFile(m_image_array[i].hFile, check_buf, CHECK_FLAG_SIZE+CHECK_XOR_LEN, &writeLen, NULL);//
		}
#endif
		if(m_image_array[i].hFile != INVALID_HANDLE_VALUE)
		{
			CloseHandle(m_image_array[i].hFile);//�ر�
		}
	}

	m_image_array.RemoveAll();//
}

BOOL CImageCreate::img_add_file(LPTSTR pathPC, LPSTR pathIMG)
{
	TCHAR PCPath_temp[MAX_PATH+1] = {0};
	char PCPath[MAX_PATH+1] = {0};
    char dirPath[MAX_PATH+1] = {0};
	int strLen;
	int i, j;
    int PCStrLen = 0;
	
    if(pathIMG[1] == 0 || pathIMG[1] != ':')//�������ó� A:����ʽ
    {
        return FALSE;
    }

	if(pathIMG[0] == '.')
	{
		return TRUE;
	}
    
    USES_CONVERSION;
    if (pathPC != NULL)
	{
		_tcsncpy(PCPath_temp, pathPC, MAX_PATH);//
	    strncpy(PCPath, T2A(pathPC), MAX_PATH);//����
	}

    strncpy(dirPath, pathIMG, MAX_PATH);
	strLen   = strlen(dirPath);//����
    PCStrLen = strlen(PCPath);//PC����

	//find dir path
	for(i = strLen-1; i >= 0; i--)
	{
		if(dirPath[i] == '\\' || dirPath[i] == '/')
		{
			dirPath[i] = 0;
			break;
		}
	}

	//find dir path
	for(j = PCStrLen-1; j >= 0; j--)
	{
		if(PCPath[j] == '\\' || PCPath[j] == '/')
		{
            j++;
			break;
		}
	}

    //create directory, because of sizeof(x:) = 2, so i > 2
	if(i > 2 && !File_MkdirsAsc((unsigned char *)dirPath))//�����ļ���
	{
		return FALSE;
	}

    if (pathPC == NULL)
    {
        return TRUE;
    }

    //is folder
    if (i == strLen-1)
    {
        dirPath[i] = '\\';
        i++;
        if (j != 0)
        {
            j++;
        }
        if (i > MAX_PATH)
        {
            return FALSE;
        }

        memcpy(&dirPath[i], &PCPath[j], MAX_PATH - i);//
    }
    else if (i != 0)
    {
        dirPath[i] = '\\';
    }

	//if(i == strLen-1)
	//{
	//	return TRUE;
	//}

	//open file
    T_U32 file = 0;
	T_U32 temp = 0; 
//	T_PFAT fat;
	T_U32 ClusBitBytes = 0;

	file = File_OpenAsc(NULL, (T_U8*)dirPath, FILE_MODE_CREATE);//���ļ���
	if(0 == file)
	{
		return FALSE;
	}

	// check if the capacity is enough
/*	fat = (T_PFAT)file->attr->driver->msg;
	ClusBitBytes = fat->SecBit + fat->ClusBit;
	temp = fd.nFileSizeLow;
	temp = ((temp + ((1 << ClusBitBytes) - 1)) >> ClusBitBytes);
	TotalFCapacity += temp * fat->SecPerClus;
	if (TotalFCapacity > DriverCapacity)
	{
		File_Close(file);
		return FALSE;
	}
*/
	//open PC file and trans
	//FILE *fp;
	DWORD count;
	T_U32 writelen = 0;
	char buf[4096];

	HANDLE hFile = INVALID_HANDLE_VALUE;
	hFile = CreateFile(PCPath_temp, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if(INVALID_HANDLE_VALUE == hFile)
	{
		DWORD ptr = GetLastError();//
	    File_Close(file);//�ر�
		return FALSE;
	}

	while(ReadFile(hFile, buf, 4096, &count, NULL))
	{
		/*�����־�Ǹ�����а���Ӧ�õ�ʱ���˳��ģ���������Ӧ�õ�ǰ��ֵΪFALSE*/
		if (ExitFlag)
		{
			CloseHandle(hFile);//�ر�
	        File_Close(file);//�ر�
			return FALSE;
		}

		if(0 == count)
		{
			break;
		}

		FileSize += count;
#ifdef CRC_CHECK
		{
			for(i=0; i<m_image_array.GetSize(); i++)
			{
				if(pathIMG[0] == m_image_array[i].diskname)
				{
					break;
				}
			}

			theConfig.check_xor_64(m_image_array[i].FileCheckValue, (T_U8*)buf, count);
		}
#endif	
		writelen = File_Write(file, buf, count);//д����
		if( writelen != (T_U32)count)
		{
			CloseHandle(hFile);//�ر�
	        File_Close(file);//�ر�
			return FALSE;
		}
	}
	//fclose(fp);
	CloseHandle(hFile);//�ر�
	File_Close(file);//�ر��ļ�
	return TRUE;
}

BOOL CImageCreate::img_add_dir(LPTSTR pathPC, LPSTR pathIMG)
{
	int len;
	TCHAR searchPathPC[MAX_PATH];
	char searchPathDisk[MAX_PATH];
	
	_tcsncpy(searchPathPC, pathPC, MAX_PATH);//
	len = _tcslen(searchPathPC);
	if(len > 0 && searchPathPC[len-1] != '\\')
	{
		searchPathPC[len] = '\\';//
		searchPathPC[len+1] = 0;//
	}

	strncpy(searchPathDisk, pathIMG, MAX_PATH);
	len = strlen(searchPathDisk);//����
	if(len > 0 && searchPathDisk[len-1] != '/')
	{
		searchPathDisk[len] = '/';
		searchPathDisk[len+1] = 0;
	}

	if(FindFileInDir(searchPathPC, searchPathDisk))//�����ļ���
	{
		return TRUE;
	}
	else
	{
		return AK_FALSE;
	}
}


BOOL CImageCreate::FindFileInDir(LPTSTR pathPC, LPSTR pathImg)
{
	WIN32_FIND_DATA fd;
	HANDLE hSearch;
	TCHAR subPathPC[MAX_PATH+1];
	char subPathImg[MAX_PATH+1];
	TCHAR searchPath[MAX_PATH+1];

	_tcsncpy(searchPath, pathPC, MAX_PATH);
	_tcscat(searchPath, _T("*"));

	hSearch = FindFirstFile(searchPath, &fd);//���ҵ�һ��
	if(INVALID_HANDLE_VALUE == hSearch)
	{
		return FALSE;
	}
	
	USES_CONVERSION;
	
	do
	{
		/*���ǵ��ļ�ϵͳ������.��ͷ���ļ������ߵ��ļ���*/
		if (fd.cFileName[0] != 0x2E)
		//if((0 != _tcscmp(fd.cFileName, _T("."))) && (0 != _tcscmp(fd.cFileName, _T(".."))))
		{
			_tcsncpy(subPathPC, pathPC, MAX_PATH);
			_tcscat(subPathPC, fd.cFileName);//
			
			strncpy(subPathImg, pathImg, MAX_PATH);
			strcat(subPathImg, T2A(fd.cFileName));//

			if((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
			{
				strcat(subPathImg, "/");
				if(!img_add_file(NULL, subPathImg))//���ļ�
				{
					return FALSE;
				}

				_tcscat(subPathPC, _T("\\"));
				if(!FindFileInDir(subPathPC, subPathImg))//���ļ���
				{
					return FALSE;
				}
			}
			else
			{
				if(!img_add_file(subPathPC, subPathImg))//���ļ�
				{
					return FALSE;
				}
			}
		}

		if(!FindNextFile(hSearch, &fd))//����һ�¸�
		{
			break;
		}
	}
	while(1);

	FindClose(hSearch);
	return TRUE;	
}

extern "C"
{
    extern T_BOOL File_CreateVolumeAsc(const T_U8* FileName);
}

BOOL CImageCreate::img_add_volume_lable(T_U8* volume_name)//���
{

    if (volume_name != NULL)//
    {
        return  File_CreateVolumeAsc((const T_U8*)volume_name);//�������
    }

    return  TRUE;
}

T_U32 M_Write(T_PMEDIUM medium, const T_U8* buf, T_U32 sector, T_U32 size)
{
	HANDLE hFile = (HANDLE)(medium->msg);
	DWORD dwCB;
	T_S32 high = 0;
	T_U32 ret = 0;

	if(NULL == hFile)//
	{
        if (INVALID_HANDLE_VALUE == m_hFile)//�Ƿ���Ч
        {
    		return 0;
        }

        hFile = m_hFile;//
	}
	
	high = sector >> (32 - medium->SecBit);
	
	SetFilePointer(hFile, sector <<  medium->SecBit, &high, FILE_BEGIN);	//ƫ��
	if (WriteFile(hFile, buf, (1 << medium->SecBit) * size, &dwCB, NULL))//д
	{
		ret = size;
	}

	return ret;
}

T_U32 M_Read(T_PMEDIUM medium, T_U8 *buf, T_U32 sector, T_U32 size)
{
	HANDLE hFile = (HANDLE)(medium->msg);
	DWORD  dwCB;
	T_S32  high = 0;
	T_U32  ret  = 0;
	T_BOOL bret = AK_FALSE;
	
	if (NULL == hFile)//
	{
        if (INVALID_HANDLE_VALUE == m_hFile)//�Ƿ���Ч
        {
    		return 0;
        }

        hFile = m_hFile;
	}

	SetFilePointer(hFile, sector << medium->SecBit, &high, FILE_BEGIN);//ƫ��
	bret = ReadFile(hFile, buf, (1 << medium->SecBit) * size, &dwCB, NULL);//��
	if (bret)
	{
	    ret = size;
	}
    return ret;
}

T_BOOL M_Flush(T_PMEDIUM medium)
{
	HANDLE hFile = (HANDLE)(medium->msg);

	if(NULL == hFile)
	{
        if (INVALID_HANDLE_VALUE == m_hFile)//�Ƿ���Ч
        {
    		return 0;
        }

        hFile = m_hFile;
    }

	FlushFileBuffers(hFile);//flush
	return AK_TRUE;
}



