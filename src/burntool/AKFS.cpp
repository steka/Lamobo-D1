// 
//AKFS.cpp: implementation of the CAKFS class.
//
//
//////////////////////////////////////////////////////////////////////


//���ļ����ļ�ϵͳ�ĵ��ýӿ�
//ר���ṩburn�����ļ�ϵͳ�Ľӿ�ʹ��
//ע�⣺����¼����ʹ��mount����֧�ֶ��nand����mount�̹��ܵġ�


#include "stdafx.h"
#include "AKFS.h"
#include "transc.h"
#include "ImageCreate.h"


//�˴��ǰ���C���ԵĴ���
extern "C"
{
	#include "anyka_types.h"
	#include "mtdlib.h"
    #include "medium.h"
    #include "fs.h"
	#include "eng_dataconvert.h"
	#include "fsa.h"
	#include "fha.h"
}

//�ص�������
T_U8 Test_InStream(T_VOID);
T_VOID Test_OutStream(T_U16 ch);
T_U32 Test_GetSecond(T_VOID);//��ȡʱ��
T_VOID Test_SetSecond(T_U32 seconds);//����ʱ��
T_S32 Test_CrtSem(T_U32 initial_count, T_U8 suspend_type, T_S8 *FuncName , T_U32 Line);//�����ź���
T_S32 Test_RelSem(T_S32 semaphore, T_S8 *FuncName , T_U32 Line);//�ͷ��ź���
T_S32 Test_DelSem(T_S32 semaphore, T_S8 *FuncName , T_U32 Line);//ɾ����Ϣ��
T_S32 Test_ObtSem(T_S32 semaphore, T_U32 suspend, T_S8 *FuncName , T_U32 Line);//�ȴ��ź���
T_pVOID	Test_RamAlloc(T_U32 size, T_S8 *filename, T_U32 fileline); //�����ڴ�
T_pVOID	Test_RamRealloc(T_pVOID var, T_U32 size, T_S8 *filename, T_U32 fileline);//�ط����ڴ�
T_pVOID Test_RamFree(T_pVOID var, T_S8 *filename, T_U32 fileline);//�ͷ��ڴ�
T_U32 Test_GetChipType(T_VOID);//��ȡоƬ����
T_pVOID Test_Memcpy(T_pVOID dst, T_pCVOID src, T_U32 count);//����
T_pVOID Test_MemSet(T_pVOID buf, T_S32 value, T_U32 count);//���
T_S32   Test_MemCmp(T_pCVOID buf1, T_pCVOID buf2, T_U32 count);//�Ƚ�
T_pVOID Test_MemMov(T_pVOID dst, T_pCVOID src, T_U32 count);//����
T_pVOID Test_MemMov_FSA(T_pVOID dst,  T_pCVOID src, T_U32 count);
T_S32 Test_Printf(T_pCSTR s, ...);
void FPrintf(T_pCSTR s, ...);

//T_U32 GetImgCapacity(HANDLE hFile);
static T_VOID fs_sys_sleep(T_U32 ms);

//�˴��ǰ���C���ԵĴ���
extern "C"
{
	T_S8  Utl_StrCmpNC(T_pCSTR str1, T_pCSTR str2, T_U16 length)
	{
		return strncmp((char*)str1, (char*)str2, length);
	}

	T_S16 Utl_UStrCmpN(T_pCWSTR str1, T_pCWSTR str2, T_U16 length)
	{
		T_U32 i;

		for(i = 0; i < length; i++)
		{
			if(str1[i] > str2[i])
			{
				return 1;
			}
			else if(str1[i] < str2[i])
			{
				return -1;
			}
		}
    
		return 0;
	}

	T_S16	Utl_UStrCmpNC(T_pCWSTR str1, T_pCWSTR str2, T_U16 length)
	{
		T_U32 i;

		for(i = 0; i < length; i++)
		{
			if(str1[i] > str2[i])
			{
				return 1;
			}
			else if(str1[i] < str2[i])
			{
				return -1;
			}
		}
    
		return 0;
	}
/*���ڲ��õ�mtd�ĺ���������������һЩ�պ������*/
/*
//#if 0

    T_VOID Medium_Destroy(T_PMEDIUM medium)
    {
        if (medium != AK_NULL)
             free((T_pVOID)medium);
    }

    T_VOID MtdLib_SetCallBack(T_PMTDCALLBACK pMtdConfig)
    {

    }

    T_PMEDIUM Nand_CreateMedium(T_PNANDFLASH nand,  T_U32 StartBlk, T_U32 BlkCnt, T_U16 *RsvBlkNum)
    {
        return NULL;
    }

    T_PMEDIUM Medium_CreatePartition(T_PMEDIUM large, T_U32 StartPage,
                                 T_U32 PageCnt, T_U32 SecSize, T_U32 ProtectLevel)
    {
        return NULL;
    }
    
    T_U32 Medium_GetMTDRsvInfo(T_PNANDFLASH nand, T_U32 StartBlock, T_U32 TolBlock, 
                                     T_U16 RsvBlockPerMTD, T_U16 *RsvBlock, T_U16 *RsvNum)
    {
        return 0;
    }
    void Medium_ConnectFS(T_PMEDIUM medium, const T_U8 data[], T_U32 sector,T_U32 size)
    {
    }

    T_U32 NandMtd_Format(T_PNANDFLASH nand, T_U32 StartBlock, T_U32 BlkCnt)
    {
        return 0;
    }
//#endif
	*/
}

//���º����ǻص�ʹ�õ�
T_U8 Test_InStream(T_VOID)
{
     return 0;
}

T_VOID Test_OutStream(T_U16 ch)
{
}
//��ȡʱ��
T_U32 Test_GetSecond(T_VOID)
{
	struct tm tmBase;
	time_t tBase;
	time_t tNow;

	memset(&tmBase, 0, sizeof(tmBase));
	tmBase.tm_year = 79;
	tmBase.tm_mon = 12;
	tmBase.tm_mday = 1;

	tBase = mktime(&tmBase);
	time(&tNow);

	return tNow - tBase;
}
//����ʱ��
T_VOID Test_SetSecond(T_U32 seconds)
{
//	WinSecond = seconds;
}
//�����ź���
T_S32 Test_CrtSem(T_U32 initial_count, T_U8 suspend_type, T_S8 *FuncName , T_U32 Line)
{
	HANDLE handle = NULL;

	handle = CreateSemaphore(NULL, initial_count,initial_count, NULL);
	//if (handle == NULL)
		
	return (T_S32)handle;
}
//�ͷ��ź���
T_S32 Test_RelSem(T_S32 semaphore, T_S8 *FuncName , T_U32 Line)
{
	T_S8 result = 0;

	result = ReleaseSemaphore((HANDLE)semaphore,1,NULL);
	return 0;
}
//ɾ���ź���
T_S32 Test_DelSem(T_S32 semaphore, T_S8 *FuncName , T_U32 Line)
{
	if (CloseHandle((HANDLE)semaphore))
	{
		return 0;
	}
	else
	{
		return -1;
	}
}
//�ȴ��ź���
T_S32 Test_ObtSem(T_S32 semaphore, T_U32 suspend, T_S8 *FuncName , T_U32 Line)
{
	WaitForSingleObject((HANDLE)semaphore,INFINITE);
	return 0;
}
//�����ڴ�
T_pVOID	Test_RamAlloc(T_U32 size, T_S8 *filename, T_U32 fileline)
{
	T_pVOID ptr = NULL;

	ptr =  (T_pVOID)malloc(size);

	memset(ptr, 0, size);

	return ptr;
}
//�������ڴ�
T_pVOID	Test_RamRealloc(T_pVOID var, T_U32 size, T_S8 *filename, T_U32 fileline)
{
	T_pVOID ptr = NULL;

	ptr = realloc(var, size);

	return ptr;
}
//�ͷ��ڴ�
T_pVOID Test_RamFree(T_pVOID var, T_S8 *filename, T_U32 fileline)
{
	free(var);

	return AK_NULL;
}
//��ȡоƬ
T_U32 Test_GetChipType(T_VOID)
{
    return FS_AK32XX;
}
//����
T_pVOID Test_Memcpy(T_pVOID dst, T_pCVOID src, T_U32 count)
{
	PBYTE pDst = (PBYTE)dst;
	PBYTE pSrc = (PBYTE)src;
	UINT i;

	if(NULL == dst || NULL == src)
	{
		return NULL;
	}

	for(i = 0; i < count; i++)
	{
		pDst[i] = pSrc[i];
	}

	return dst;
}

T_pVOID Test_MemSet(T_pVOID buf, T_S32 value, T_U32 count)
{
	if(NULL == buf)
	{
		return NULL;
	}

	UINT i;
	PBYTE pBuf = (PBYTE)buf;

	for(i = 0; i < count; i++)
	{
		pBuf[i] = (BYTE)value;
	}

	return buf;
}

T_S32 Test_MemCmp(T_pCVOID buf1, T_pCVOID buf2, T_U32 count)
{
	return memcmp(buf1, buf2, count);
}

T_pVOID Test_MemMov(T_pVOID dst, T_pCVOID src, T_U32 count)
{
	return memmove(dst, src, count);
}

T_pVOID Test_MemMov_FSA(T_pVOID dst, T_pCVOID src, T_U32 count)
{
	return memmove(dst, src, count);
}


T_S32 Test_Printf(T_pCSTR s, ...)
{
	return 0;
}

void FPrintf(T_pCSTR s, ...)
{
}

static T_VOID fs_sys_sleep(T_U32 ms)
{
    
}
//FSA�����ڴ�
T_pVOID	Test_RamAlloc_FSA(T_U32 size)
{
	T_pVOID ptr = NULL;

	ptr =  (T_pVOID)malloc(size);

	memset(ptr, 0, size);

	return ptr;
}
//FSA�ͷ��ڴ�
T_pVOID Test_RamFree_FSA(T_pVOID var)
{
	free(var);

	return AK_NULL;
}
//fsa��ȡMEDIUM
T_pVOID Test_FSA_GetImgMedium(T_U8 driverID)
{
    DRIVER_INFO DriverInfo = {0};
    
    if (!FS_GetDriver(&DriverInfo, driverID))
    {
        return 0;
    }
	
    return DriverInfo.medium;
}
//��
T_hFILE Fwl_FileOpen(T_pCSTR path, T_U32 mode)
{
    T_U32 file = 0;
    T_U32 i, len;

	//��ȡ·���ĳ���
    len = strlen((CHAR *)path);
    for (i=len; i !=0; i--)
    {
        if (path[i] == '/' || path[i] == '\\')
            break;
    }
	//ͨ��·�������ļ���
    if (i != 0 && path[i-1] != ':')
    {
        T_U8 buf[300]; 

        if (i > 300)
            return FS_INVALID_HANDLE;

        memcpy(buf, path, 300);
        buf[i] = 0;

        if (!File_MkdirsAsc(buf))
        {
            return FS_INVALID_HANDLE;
        }
    }
    //���ļ���
    file = File_OpenAsc(0, (T_U8 *)path, mode);

    if (!File_IsFile(file))
    {
        File_Close(file);
        
        printf("FWL FILE ERROR: Fwl_FileOpen: \n");
        return FS_INVALID_HANDLE;
    }

    return (T_hFILE)file;
}
//�ر�
T_BOOL  Fwl_FileClose(T_hFILE hFile)
{
	//�жϾ���Ƿ���Ч
    if (hFile != FS_INVALID_HANDLE)
    {
        File_Close((T_hFILE)hFile);
    }

	return AK_TRUE;
}
//ƫ��
T_U32   Fwl_FileSeek(T_hFILE hFile, T_S32 offset, T_U16 origin)
{
	//�жϾ���Ƿ���Ч
    if (hFile == FS_INVALID_HANDLE )
    {
        return FS_INVALID_SEEK;
    }

    return File_SetFilePtr((T_hFILE)hFile, offset, origin);    
}
//��
T_U32   Fwl_FileRead(T_hFILE hFile, T_pVOID buffer, T_U32 count)
{
    T_U32 ret = 0;
	//�жϾ���Ƿ���Ч
    if (hFile != FS_INVALID_HANDLE)
    {     
        ret = File_Read((T_hFILE)hFile, (T_U8*)buffer, count);
    }
    
    return ret;
}
//д����
T_U32   Fwl_FileWrite(T_hFILE hFile, T_pCVOID buffer, T_U32 count)
{
    T_U32 ret = 0;
	
	//�жϾ���Ƿ���Ч
    if (hFile != FS_INVALID_HANDLE)
    {
        ret = File_Write((T_hFILE)hFile, (T_U8*)buffer, count);
    }

    return ret;
}

//�����ļ���
T_BOOL  Fwl_MkDir(T_pCSTR path)
{
    return File_MkdirsAsc((T_U8* )path);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAKFS::CAKFS()
{
	//initFlag = FALSE;
}

CAKFS::~CAKFS()
{
	UINT i = 0;

	i++;
}

BOOL CAKFS::Init(VOID)
{
    T_FSCallback mountinit;
	T_FSA_LIB_CALLBACK fsainit;

    mountinit.out = Test_OutStream;
    mountinit.in  = Test_InStream;
    mountinit.fGetSecond = Test_GetSecond;
    mountinit.fSetSecond = Test_SetSecond;    
    mountinit.fAscToUni  = AnsiStr2UniStr;
    mountinit.fUniToAsc  = UniStr2AnsiStr;
    mountinit.fRamAlloc  = Test_RamAlloc;
    mountinit.fRamRealloc = Test_RamRealloc;
    mountinit.fRamFree   = Test_RamFree;
    mountinit.fCrtSem    = Test_CrtSem;
    mountinit.fDelSem    = Test_DelSem;
    mountinit.fObtSem    = Test_ObtSem;
    mountinit.fRelSem    = Test_RelSem;
    mountinit.fMemCpy    = Test_Memcpy;
    mountinit.fMemSet    = Test_MemSet;
    mountinit.fMemMov    = Test_MemMov;
    mountinit.fMemCmp    = Test_MemCmp;

    mountinit.fPrintf    = Test_Printf;
    mountinit.fGetChipId = Test_GetChipType;
    mountinit.fSysRst    = NULL;  ///ĿǰӦ�ò��� 
    mountinit.fRandSeed  = NULL;  //ĿǰӦ�ò��� 
    mountinit.fGetRand   = NULL;  //ĿǰӦ�ò��� 
    mountinit.fMountThead= NULL;  //ĿǰӦ�ò��� 
    mountinit.fKillThead = NULL;  //ĿǰӦ�ò��� 
    mountinit.fSystemSleep= fs_sys_sleep;

    //mount��ʼ��
    if (FS_InitCallBack(&mountinit, 64))
	{		
		return FALSE;
	}
    
	fsainit.MemCmp       = Test_MemCmp;
	fsainit.MemCpy       = Test_Memcpy;
	fsainit.MemSet       = Test_MemSet;
	fsainit.MemMov       = Test_MemMov_FSA;
	fsainit.Printf       = Test_Printf;
	fsainit.RamAlloc     = Test_RamAlloc_FSA;
	fsainit.RamFree      = Test_RamFree_FSA;
	fsainit.GetImgMedium = Test_FSA_GetImgMedium;
	fsainit.fFs.FileClose = Fwl_FileClose;
	fsainit.fFs.FileOpen  = Fwl_FileOpen;
	fsainit.fFs.FileRead  = Fwl_FileRead;
	fsainit.fFs.FileSeek  = NULL;//Fwl_FileSeek;//ĿǰӦ�ò��� 
	fsainit.fFs.FileWrite = Fwl_FileWrite;
	fsainit.fFs.FsMkDir   = Fwl_MkDir;
    
	//fsa��ʼ��
	if (FSA_init(&fsainit) != FSA_SUCCESS)
	{
		return FALSE;
	}

	return TRUE;
}

//Destroy�ļ�ϵͳ
VOID CAKFS::Destroy(VOID)
{
	//FS_Destroy();
}



//�͸�nand��sd�ķ���
BOOL CAKFS::LowFormat(PBYTE PartitionInfo, UINT nNumPartion, UINT resvSize, UINT StartBlock, 
					  UINT MediumType, PBYTE MediumInfo, UINT *StartID, UINT *IDCnt)
{
	FORMAT_INFO info = {0};
	BOOL RetVal;

	//nand
	if (MediumType == TRANSC_MEDIUM_NAND || MediumType == TRANSC_MEDIUM_SPI_NAND)
	{
		info.MediumType = FS_NAND;
	}
	else if (MediumType == TRANSC_MEDIUM_EMMC)
	{
		//������sd��
		info.MediumType = FS_SD;
	}
	else
	{
		return FALSE;
	}
	info.obj  = (T_U32)MediumInfo;

	//�͸�nand��sd
	RetVal = FS_LowFormat((T_FS_PARTITION_INFO *)PartitionInfo, nNumPartion, resvSize, StartBlock, &info);
    //mount���ǵ���һ��ר��¼ʱ�õĿ⣬
	//�����ֵ�Ƿ���id��idcnt
	*StartID = (UINT)info.MediumType;
	*IDCnt   = info.obj;

	return RetVal;
}

/***���е���Ϣ����obj����***/
BOOL CAKFS::DownloadFile(UINT obj)
{
	T_DOWNLOAD_FILE *download_udisk = (T_DOWNLOAD_FILE *)obj;
	DWORD dwAttribute;
    CImageCreate cImage;
	
	if (NULL == download_udisk)
	{
		return FALSE;
	}
	//��ȡ�ļ�������
	dwAttribute = GetFileAttributes(download_udisk->pc_path);
	cImage.ExitFlag = FALSE;
	/**������Ƚ��ļ��˱Ƚ��鷳**/
	if((dwAttribute & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
	{
		//�����ļ���
		if (!cImage.img_add_dir(download_udisk->pc_path, download_udisk->udisk_path))
		{
			return FALSE;
		}
	}
	else
	{
		//�����ļ�
		if (!cImage.img_add_file(download_udisk->pc_path, download_udisk->udisk_path))
		{
			return FALSE;
		}
	}

	return TRUE;
}
//umount �ƶ�����
VOID CAKFS::UnMountMemDev(UCHAR DriverID)
{
	FS_UnMountMemDev(DriverID);
	//FS_Destroy();
}
//umount nandflash
VOID CAKFS::UnMountNandFlash(UCHAR DriverID)
{
	FS_UnMountnNandflash(DriverID);
	//FS_Destroy();
}
//mount nandflash
UINT CAKFS::MountNandFlash(UINT NandBase, UINT StartBlock, UCHAR DriverList[], UCHAR *DriverCnt)
{
    return FS_MountNandFlash((T_PNANDFLASH)NandBase, StartBlock, DriverList, DriverCnt);
	//FS_UnMountnNandflash(DriverID);
	//FS_Destroy();
}
//��ȡ������Ϣ
BOOL CAKFS::GetDriverInfo(UINT StartID, UINT DriverCnt,  UINT *DriverNum, UCHAR *Info, UINT MediumType)
{
	UINT i;
    T_DRIVER_INFO *pDriverInfo = (T_DRIVER_INFO *)Info;
    DRIVER_INFO Driver_Info;

    *DriverNum = 0;

	//�����ĸ���
    for (i=0; i<DriverCnt; i++)
	{
		//ͨ��id��ȡ��Ӧ�ķ���
		if (FS_GetDriver(&Driver_Info, StartID+i))
		{
            (*DriverNum)++;
            pDriverInfo->DiskName = Driver_Info.DriverID + 'A';
            if (MediumType == TRANSC_MEDIUM_NAND || MediumType == TRANSC_MEDIUM_SPI_NAND)
            {
				//nand
                pDriverInfo->PageCnt  = Driver_Info.medium->capacity >> Driver_Info.medium->SecPerPg;
            }
            else
            {
				//sd
                pDriverInfo->PageCnt  = Driver_Info.medium->capacity;//sd��ֻ��������С
            }
            
            pDriverInfo++;
		}
		else
		{
			return FALSE;
		}
    }

    if (0 == *DriverNum)
    {
        return FALSE;
    }

	return TRUE;
}

//���snowbirdl����¼�ṹ
//�������ؾ����ļ�
BOOL CAKFS::DownloadImg(UINT nID, HANDLE hFile, T_IMG_INFO *img_info, UINT img_buf_len)
{
	T_U32 file_len;
	T_U32 read_len;
	T_U32 write_size;
	T_U8 *pBuf = AK_FALSE;
	T_BOOL ret = AK_FALSE;
	T_hFILE handle = FS_INVALID_HANDLE;
	
	//�ж��Ƿ�Ϸ�
	if(INVALID_HANDLE_VALUE == hFile || AK_NULL == img_info)
	{
		return AK_FALSE;
	}
	
	file_len = img_info->data_length;//�����ļ��ĳ���
	//�ж��ļ������Ƿ�0
	if (0 == file_len)
	{
		return AK_FALSE;
	}
	
	pBuf = (T_U8 *)Test_RamAlloc_FSA(img_buf_len);
	memset(pBuf,0,img_buf_len);
	if(AK_NULL == pBuf)
	{

		return AK_FALSE;
	}
	
	//�˽ӿڷ���һ���ṹ�壬
	//ʹ��ʱҪ����ת�������ڲ�ʵ��
	handle = FSA_write_image_begin_burn(img_info);
	if (FS_INVALID_HANDLE == handle)
	{
		FSA_write_image_end_burn(handle);
		Test_RamFree_FSA(pBuf);
		return AK_FALSE;
	}
	
	
	while (1)
	{   
		//��ȡ����
		ret = ReadFile(hFile, pBuf, img_buf_len, &read_len, NULL);
		if (!ret)
		{
			FSA_write_image_end_burn(handle);
			Test_RamFree_FSA(pBuf);
			return AK_FALSE;
		}
		else
		{
			write_size = read_len;
		}
		//ÿһ����write_size��С����д��ȥ
		if (FSA_FAIL == FSA_write_image_burn(handle, pBuf, write_size))
		{
			FSA_write_image_end_burn(handle);
			Test_RamFree_FSA(pBuf);
			return AK_FALSE;
		}

		BT_DownloadImg_length(nID,  write_size);

		file_len -= write_size;
		//��������������˳�ȥ
		if (0 == read_len || 0 == file_len)
		{
			break;
		}
	} 
	//�ͷ��ڴ�
	FSA_write_image_end_burn(handle);
	Test_RamFree_FSA(pBuf);
	return TRUE;
}