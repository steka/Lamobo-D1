
#include "StdAfx.h"
#include "logFile.h"
//#include  <io.h>
//#include  <wchar.h>

CLogFile::CLogFile()
{
    hLogFile = INVALID_HANDLE_VALUE;//��ʼ��
    memset(file_name,0,MAX_PATH);//����0
}

CLogFile::CLogFile(TCHAR *name)
{
    hLogFile = INVALID_HANDLE_VALUE;//��ʼ��
    memset(file_name,0,MAX_PATH);//����0

    SetFileName(name);//�����ļ���
    InitFile();
}

CLogFile::~CLogFile()
{
    if (hLogFile != INVALID_HANDLE_VALUE)
    {
        WriteLogFile(LOG_LINE_TIME, "close file\r\n\r\n\r\n\r\n");
        CloseHandle(hLogFile);//�رվ��
    }
}

BOOL CLogFile::InitFile()
{
    hLogFile = INVALID_HANDLE_VALUE;
	
	//�ж��ļ��Ƿ����
	//if (_access(file_name, 0) != INVALID_HANDLE_VALUE)
//	{
//		return TRUE;
//	}
	
    hLogFile = CreateFile(file_name, GENERIC_WRITE, FILE_SHARE_READ , NULL, 
					OPEN_ALWAYS , FILE_ATTRIBUTE_NORMAL , NULL);//�����ļ�

    if (hLogFile == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    SetFilePointer(hLogFile, 0, NULL, FILE_END);//�����ļ�λ��
    return TRUE;
}

VOID  CLogFile::SetFileName(TCHAR *name)
{
    if ( (name[1] == ':') && (name[2] == '\\') )   
    {
        memcpy(file_name, name, MAX_PATH);
    }
    else 
    {
        GetCurPCPath(file_name, MAX_PATH);//��ȡ��ǰ�ļ�·��
        _tcscat(file_name, name);
    }
}

DWORD CLogFile::WriteLogFile(UCHAR wFlag, const char *fmt, ...)
{
    va_list ap;
    char    pbuf[300];
    UINT    buf_size;
    DWORD   write_len = 0;
    SYSTEMTIME  logTime;

	logTime.wYear = 0;
	logTime.wMonth = 0;
	logTime.wDay = 0;
    
    if (hLogFile == INVALID_HANDLE_VALUE)
    {
        return 0;
    }
    
    if (wFlag != 0)
    {
        GetLocalTime(&logTime);
    }

    if ( (wFlag&LOG_LINE_DATE) != 0 )//д��ǰ����
    {
        sprintf(pbuf + write_len, "Date: %d-%d-%d ", logTime.wYear,logTime.wMonth,logTime.wDay);
        write_len = strlen(pbuf);
    }

    if ( (wFlag&LOG_LINE_TIME) != 0 )//д��ǰʱ��
    {
        sprintf(pbuf + write_len, "[%02d:%02d:%02d]", logTime.wHour,logTime.wMinute,logTime.wSecond);
        write_len += 10;
        // write_len = strlen(pbuf);
    }

    va_start(ap, fmt);//д����
	_vsnprintf(pbuf + write_len, 300 - write_len, fmt, ap);
	va_end(ap);

    buf_size = strlen(pbuf);//����
    return WriteFile(hLogFile, (LPVOID)pbuf, buf_size, &write_len, NULL);
}

BOOL CLogFile::GetCurPCPath(PTCHAR curPCPath, int buf_len)
{
    UINT len;
    CString sPath;

  	GetModuleFileName(NULL,sPath.GetBufferSetLength(buf_len+1),buf_len);//��ȡ�ļ���
	sPath.ReleaseBuffer();
	len = sPath.ReverseFind ('\\');
	sPath=sPath.Left (len+1);

    memcpy(curPCPath, (char *)sPath.GetBuffer(buf_len), buf_len);
    return TRUE;
}

BOOL  CLogFile::CheckFileSize(DWORD file_max_size)
{
    DWORD  file_size_high = 0;
    DWORD  file_size_low  = 0;

    hLogFile = INVALID_HANDLE_VALUE;
    hLogFile = CreateFile(file_name,           // open MYFILE.TXT 
                GENERIC_READ,              // open for reading 
                FILE_SHARE_READ,           // share for reading 
                NULL,                      // no security 
                OPEN_EXISTING,             // existing file only 
                FILE_ATTRIBUTE_NORMAL,     // normal file 
                NULL);                     // no attr. template 

    if (hLogFile == INVALID_HANDLE_VALUE)
    {
        return FALSE;
    }

    file_size_low = GetFileSize(hLogFile, &file_size_high);//�ļ���С
    CloseHandle(hLogFile);//�ر�

    if ((file_size_low > file_max_size) || (file_size_high != 0))
    {
        DeleteFile(file_name);
        return FALSE;
    }
    
    return TRUE;
}
