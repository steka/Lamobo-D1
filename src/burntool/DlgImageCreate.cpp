// DlgImageCreate.cpp : implementation file
//

#include "stdafx.h"
#include "burntool.h"
#include "DlgImageCreate.h"
#include "Burn.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


extern CConfig theConfig;
extern CBurnToolApp theApp;


/////////////////////////////////////////////////////////////////////////////
// DlgImageCreate dialog


DlgImageCreate::DlgImageCreate(CWnd* pParent /*=NULL*/)
	: CDialog(DlgImageCreate::IDD, pParent)
{
	//{{AFX_DATA_INIT(DlgImageCreate)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void DlgImageCreate::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(DlgImageCreate)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(DlgImageCreate, CDialog)
	//{{AFX_MSG_MAP(DlgImageCreate)
	ON_BN_CLICKED(IDCREATE, OnCreate)
	ON_BN_CLICKED(IDC_BUTTON_DEST_BROWSER, OnButtonDestBrowser)
	ON_BN_CLICKED(IDC_BUTTON_SOURCE_BROWSER, OnButtonSourceBrowser)
	ON_CBN_EDITCHANGE(IDC_COMBO_PAGESIZE, OnEditchangeComboPagesize)
	ON_BN_CLICKED(IDC_STATIC_USED_TIME, OnStaticUsedTime)
	ON_WM_TIMER()
	ON_CBN_SELCHANGE(IDC_COMBO_PAGESIZE, OnSelchangeComboPagesize)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// DlgImageCreate message handlers

HANDLE offline_image_event;
UINT   g_offline_img_stat;

BOOL DlgImageCreate::OnInitDialog() 
{
    CDialog::OnInitDialog();
    
    CString str;

	SetupDisplay();//��ʼ��һЩֵ
	
    defaultConfig();  //���߾���������Ĭ��ֵ
	
    // ������
    SetDlgItemText(IDC_EDIT_SOURCE_FOLDER, source_path);//Դ�ļ�·��
    SetDlgItemText(IDC_EDIT_DEST_FOLDER, dest_path);//Ŀ��·��
    SetDlgItemText(IDC_EDIT_IMAGE_NAME, name);//������
	
    str.Format(_T("%d"), sector_size);//
    SetDlgItemText(IDC_EDIT_SECTORSIZE,str); //������С
	
    str.Format(_T("%d"), page_size);
    SetDlgItemText(IDC_COMBO_PAGESIZE,str);//ҳ��С
	
    str.Format(_T("%d"), capacity);
    SetDlgItemText(IDC_EDIT_CAPACITY,str); //������С
    
    ctrl_time = 0;
    progCtrl = (CProgressCtrl *)GetDlgItem(IDC_PROGRESS_MAKING);
    progCtrl->SetRange(0,100);
    progCtrl->SetPos(0);
    progCtrl->SetStep(1);
	ExitFlag = TRUE;
	
    return TRUE;
}

void DlgImageCreate::SetupDisplay()
{
	CString str;

	str = theApp.GetString(IDS_SOURCE_FOLDER);
	GetDlgItem(IDC_STATIC_SOURCE_FOLDER)->SetWindowText(str); //Դ�ļ���

	str = theApp.GetString(IDS_DEST_FOLDER);
	GetDlgItem(IDC_STATIC_DEST_FOLDER)->SetWindowText(str);//Ŀ���ļ���

	str = theApp.GetString(IDS_BROWSE_SOURCE_FOLDER);
	GetDlgItem(IDC_BUTTON_SOURCE_BROWSER)->SetWindowText(str);//���Դ�ļ���

	str = theApp.GetString(IDS_BROWSE_DEST_FOLDER);
	GetDlgItem(IDC_BUTTON_DEST_BROWSER)->SetWindowText(str);//���Ŀ���ļ���

	str = theApp.GetString(IDS_IMAGE_FILE_NAME);
	GetDlgItem(IDC_STATIC_IMAGE_NAME)->SetWindowText(str);//�����ļ���

	str = theApp.GetString(IDS_TOTAL_CAPACITY);
	GetDlgItem(IDC_STATIC_CAPACITY)->SetWindowText(str);//������С

	
	str = theApp.GetString(IDS_PAGE_SIZE);
	GetDlgItem(IDC_STATIC_PAGESIZE)->SetWindowText(str);//ҳ��С

	str = theApp.GetString(IDS_SECTOR_SIZE);
	GetDlgItem(IDC_STATIC_SECTORSIZE)->SetWindowText(str);//������С

	str = theApp.GetString(IDS_MAKE_INFORMATION);
	GetDlgItem(IDC_MAKE_INFORMATION)->SetWindowText(str);//������Ϣ

	str = theApp.GetString(IDS_READY_MAKE);
	GetDlgItem(IDC_CURENT_INFO)->SetWindowText(str);//׼������

	str = theApp.GetString(IDS_MAKE_BEGIN);
	GetDlgItem(IDCREATE)->SetWindowText(str);//��ʼ

	str = theApp.GetString(IDS_MAKE_END);
	GetDlgItem(IDCANCEL)->SetWindowText(str);//���

	str = theApp.GetString(IDS_MAKE_IMAGE_TITLE);	
	SetWindowText(str);
}





void DlgImageCreate::OnCreate() 
{
	// TODO: Add your control notification handler code here
	HANDLE tHandle = INVALID_HANDLE_VALUE;
    DWORD tid = 0;
	DWORD fileCnt = 0;
    
    if (get_config_data())
    {
        if ((page_size >= sector_size) && (page_size % 512 == 0))
        {
			SetDlgItemText(IDC_CURENT_INFO, theApp.GetString(IDS_GET_FOLDER_INFO));
			prog_file_total_size = 0;
			//��ȡԴ�ļ���
			if  (!GetFileInfoInDir(source_path, &fileCnt, &prog_file_total_size, NULL))
			{
				MessageBox(theApp.GetString(IDS_GET_FOLDER_INFO_FAIL));
				return;
			}
			//����������ļ����ڿ�������ô�ͳ���
			if (capacity*1024*1024 < prog_file_total_size)
			{
				MessageBox(theApp.GetString(IDS_TOTAL_CAPACITY_NO_ENOUGH));
				return;
			}
			//���봴��������߳�
			SetDlgItemText(IDC_CURENT_INFO, theApp.GetString(IDS_MAKING));
			tHandle = CreateThread(NULL, 0, image_create_thread, this, 0, &tid);
			CloseHandle(tHandle);

        }
        else
        {
			//ҳ��С����
            MessageBox(theApp.GetString(IDS_PARAM_ERROR));
        }
        //tHandle = CreateThread(NULL, 0, offline_image_create, &m_image_create, 0, &tid);
    }

    //CDialog::OnOK();
}
void DlgImageCreate::OnCancel() 
{
	// TODO: Add extra cleanup here
	m_image_create.ExitFlag = TRUE;
    while (!ExitFlag)
    {
        Sleep(10);
    }
	
	CDialog::OnCancel();
}

void DlgImageCreate::OnButtonDestBrowser() 
{
	// TODO: Add your control notification handler code here
	browser_folder(dest_path);
    SetDlgItemText(IDC_EDIT_DEST_FOLDER, dest_path);
	
}

void DlgImageCreate::OnButtonSourceBrowser() 
{
	// TODO: Add your control notification handler code here
	browser_folder(source_path);
    SetDlgItemText(IDC_EDIT_SOURCE_FOLDER, source_path);
	
}


void DlgImageCreate::defaultConfig()
{
    CString  str;
    //Ĭ�ϳ�ʼֵ
    _tcscpy(source_path, _T("D:\\img"));//��ʼ��Դ�ļ�·��
    _tcscpy(dest_path, _T("D:\\img_output"));//Ŀ��·��
    str.Format(_T("imgA"));//������
    _tcscpy(name, str);
    capacity = 1024;//������С
    page_size = 512;//ҳ��С
    sector_size = page_size;//������С
}

BOOL DlgImageCreate::get_config_data()
{
    CString get_str;
    
    USES_CONVERSION;

    // source folder;
    GetDlgItemText(IDC_EDIT_SOURCE_FOLDER, get_str);
    if (get_str.IsEmpty())//�Ƿ�Ϊ��
    {
        SetDlgItemText(IDC_CURENT_INFO, theApp.GetString(IDS_SOURCE_FOLDER_EMPTY));
        return FALSE;
    }
    else 
    {
        _tcsncpy(source_path, get_str, get_str.GetLength() + 1); //��ȡԴ·��
    }

    // destination folder
    GetDlgItemText(IDC_EDIT_DEST_FOLDER, get_str);
    if (get_str.IsEmpty())//�Ƿ�Ϊ��
    {
        SetDlgItemText(IDC_CURENT_INFO, theApp.GetString(IDS_DEST_FOLDER_EMPTY));
        return FALSE;
    }
    else 
    {
        _tcsncpy( dest_path, get_str, get_str.GetLength() + 1); //Ŀ��·��
    }

    // image param
    GetDlgItemText(IDC_EDIT_IMAGE_NAME, get_str);
    if (get_str.IsEmpty())
    {
        SetDlgItemText(IDC_CURENT_INFO, theApp.GetString(IDS_IMAGE_NAME_EMPTY));
        return FALSE;
    }
    else
    {
        _tcsncpy( name, get_str, get_str.GetLength() + 1);//������
    }

	//��ȡ����
    GetDlgItemText(IDC_EDIT_CAPACITY, get_str);
    if (get_str.IsEmpty())
    {
        SetDlgItemText(IDC_CURENT_INFO, theApp.GetString(IDS_CAPACITY_SIZE_EMPTY));
        return FALSE;
    }
    else
    {
        capacity = atoi(T2A(get_str));//����
    }
    
    GetDlgItemText(IDC_EDIT_SECTORSIZE,get_str);
    if (get_str.IsEmpty())
    {
        SetDlgItemText(IDC_CURENT_INFO, theApp.GetString(IDS_SECTOR_SIZE_EMPTY));
        return FALSE;
    }
    else
    {
        sector_size = atoi(T2A(get_str));//������С
    }

    GetDlgItemText(IDC_COMBO_PAGESIZE,get_str);
    if (get_str.IsEmpty())
    {
        SetDlgItemText(IDC_CURENT_INFO, theApp.GetString(IDS_PAGE_SIZE_EMPTY));
        return FALSE;
    }
    else
    {
        page_size = atoi(T2A(get_str));//ҳ��С
    }
    
    return TRUE;
}

BOOL DlgImageCreate::browser_folder(TCHAR *folderPath)
{
    BROWSEINFOW opp;
    
    opp.hwndOwner = GetSafeHwnd();
    opp.pidlRoot = NULL;
    opp.pszDisplayName = folderPath;
    opp.lpszTitle = _T("Choose Path");//ѡ���ļ���·��
    opp.ulFlags = BIF_RETURNONLYFSDIRS;
    opp.lpfn = NULL;
    opp.iImage = 0;

    LPITEMIDLIST pIDList = SHBrowseForFolder(&opp); //������ʾѡ��Ի���

    if (pIDList)
    {
        SHGetPathFromIDList(pIDList, folderPath);
        return TRUE;
    }
    else return FALSE;
}

BOOL  DlgImageCreate::SetProgressPos()
{
    //progCtrl->SetPos(10);
    progCtrl->StepIt();

    return TRUE;
}

void  DlgImageCreate::StartTimer()
{
    used_time = 0;
    ctrl_time = SetTimer(1, 1000, NULL);//����ʱ��
}

void  DlgImageCreate::StopTimer()
{
    KillTimer(ctrl_time);//���ʱ��
    ctrl_time = 0;
}

DWORD  DlgImageCreate::offline_image_create()//��������
{
    UINT i=0, j=0;
    UINT path_len = 0;
    DWORD dwAttr;
    CString str;
    TCHAR  dest_file_path[MAX_PATH];
    TCHAR  img_file_path[MAX_PATH];
    
    USES_CONVERSION;

    if ( FALSE == m_image_create.fslib_init() )//��ʼ��
    {
        return 0;
    }

    path_len = _tcslen(dest_path);//·������
    if (0 == path_len)
    {
        return 0;
    }
    else if ( (path_len > 0) && (dest_path[path_len-1] != '\\') )//Ŀ��·��
    {
        dest_path[path_len] = '\\';
        dest_path[path_len+1] = 0;
    }
    
    CreateDirectory(dest_path, NULL);   // destination path
    _tcscpy(dest_file_path, dest_path);//Ŀ��
    _tcscat(dest_file_path, name);//������
    _tcscat(dest_file_path, _T(".img"));//�������ĺ�׺
    if(!m_image_create.img_create(dest_file_path, capacity, sector_size, page_size, theConfig.burn_mode, 'A'))//��ʼ��������
    {
        return 0;
    }

    dwAttr = GetFileAttributes(source_path);//����
    if(0xFFFFFFFF == dwAttr)
    {
        m_image_create.img_destroy();//����
        return 0;
    }

    _tcscpy(img_file_path, _T("A:"));
    if((dwAttr & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
    {
        if(!m_image_create.img_add_dir(source_path, T2A(img_file_path)))//���ļ���
        {
            m_image_create.img_destroy();//����
            return 0;
        }
    }
    else
    {
        if( !m_image_create.img_add_file(source_path, T2A(img_file_path)) )//���ļ�
        {
            m_image_create.img_destroy();//����
            return 0;
        }
    }
    m_image_create.img_destroy();//����
    
    return 1;
}

void DlgImageCreate::display_used_time()
{
    UINT  hour;
    UINT  minite;
    UINT  second;
    UINT  temp;

    hour = used_time/3600;//Сʱ
    temp = used_time%3600;//
    minite = temp/60;//����
    second = temp%60;//��

    CString str;
    
    str.Format(_T("%02d:%02d:%02d"),hour,minite,second);//
    SetDlgItemText(IDC_STATIC_USED_TIME, str);//
}

void DlgImageCreate::OnEditchangeComboPagesize() 
{
	// TODO: Add your control notification handler code here
}

DWORD WINAPI DlgImageCreate::image_create_thread(LPVOID para)//�ھ�����
{
	DlgImageCreate *pDTD = (DlgImageCreate *)para;
    UINT i=0, j=0;
    UINT path_len = 0;
    DWORD dwAttr;
    CString str;
    TCHAR  dest_file_path[MAX_PATH+1] = {0};
    TCHAR  img_file_path[MAX_PATH+1] = {0};
	BOOL   fs_destroy_flag = FALSE;
	DWORD  ret = 0;

    USES_CONVERSION;

	//pDTD->EnableWindow(FALSE);
    pDTD->GetDlgItem(IDCREATE)->EnableWindow(FALSE);
	pDTD->ExitFlag = FALSE;
  	//pDTD->GetDlgItem(IDCANCEL)->EnableWindow(FALSE);
    if ( FALSE == pDTD->m_image_create.fslib_init() )//��ʼ��
    {
        goto exit_image_create_thread;
    }

    path_len = _tcslen(pDTD->dest_path);//����
    if (0 == path_len)
    {
        goto exit_image_create_thread;
    }
    else if ( (path_len > 0) && (pDTD->dest_path[path_len-1] != '\\') )
    {
        pDTD->dest_path[path_len] = '\\';
        pDTD->dest_path[path_len+1] = 0;
    }
    
	pDTD->StartTimer();//��ʼʱ��
	fs_destroy_flag = TRUE;

    CreateDirectory(pDTD->dest_path, NULL);   // destination path
    _tcscpy(dest_file_path, pDTD->dest_path);//·��
    _tcscat(dest_file_path, pDTD->name);//��
    _tcscat(dest_file_path, _T(".img"));//��׺��
    if(!pDTD->m_image_create.img_create(dest_file_path, pDTD->capacity*1024, pDTD->sector_size, pDTD->page_size, theConfig.burn_mode, 'A'))
    {
        goto exit_image_create_thread;
    }
	
	
	pDTD->m_image_create.ExitFlag = FALSE;
    dwAttr = GetFileAttributes(pDTD->source_path);//����
    if(0xFFFFFFFF == dwAttr)
    {
        goto exit_image_create_thread;
    }
	/**A��img_create�ж����**/
    _tcscpy(img_file_path, _T("A:"));
    if((dwAttr & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
    {
        if(!pDTD->m_image_create.img_add_dir(pDTD->source_path, T2A(img_file_path)))//���ļ���
        {
            goto exit_image_create_thread;//
        }
    }
    else
    {
        if( !pDTD->m_image_create.img_add_file(pDTD->source_path, T2A(img_file_path)) )//���ļ�
        {
            goto exit_image_create_thread;//
        }
    }
	
	ret = 1;
exit_image_create_thread:
	if (fs_destroy_flag)
	{
		pDTD->StopTimer();
		FS_UnMountMemDev(0);
		pDTD->m_image_create.img_destroy();//
	}

	if (ret == 1)
	{
		pDTD->SetDlgItemText(IDC_STATIC_PROG_TIME, _T("100%"));//
		pDTD->progCtrl->SetPos(100);//
		pDTD->SetDlgItemText(IDC_CURENT_INFO,theApp.GetString(IDS_SUSSCCE));//
		//MessageBox(_T("sussess!"));
	}
	else
	{
		/*����������жϻ���ѭ��*/
		if (!pDTD->m_image_create.ExitFlag)
			pDTD->SetDlgItemText(IDC_CURENT_INFO, theApp.GetString(IDS_FAIL));//
		//MessageBox(_T("fail!"));
	}
	
	if (!pDTD->m_image_create.ExitFlag)//
	{
		//pDTD->EnableWindow(TRUE);
		pDTD->GetDlgItem(IDCREATE)->EnableWindow(TRUE);//
  		//pDTD->GetDlgItem(IDCANCEL)->EnableWindow(TRUE);
	}
	pDTD->ExitFlag = TRUE;
	return ret;
}

/*
���ܣ��ݹ�����ļ����е��ļ����������ļ���С������С�ֽڵ��ļ�
˵�����ļ��е��������������ܴ���4g��Ҫô���ļ���С����� 
���ߣ�luqiliu
���룺pathFloder--�ļ��е���·��
�����FileCnt-----�ļ��ĸ��� 
      FileSize----�ļ���������(unit byte)
      MinSize-----��С���ļ�
���أ��ɹ��ҵ�һ�����ϵ��ļ�����TRUE�� ���򷵻�FALSE
*/
BOOL DlgImageCreate::GetFileInfoInDir(LPTSTR pathFloder, DWORD *FileCnt, DWORDLONG *FileSize, DWORDLONG *MinSize)
{
	WIN32_FIND_DATA fd;
	HANDLE hSearch;
	TCHAR searchPath[MAX_PATH+1] = {0};
    TCHAR tmpPCPath[MAX_PATH+1] = {0};
    HANDLE hFile;
    DWORD  dwSize;
    CString str;
 
    if (pathFloder == NULL)
    {
        return FALSE;
    }
    
    _tcsncpy(searchPath, pathFloder, MAX_PATH);//���ҵ����ļ���·��
	_tcscat(searchPath, _T("\\*"));//��//

	hSearch = FindFirstFile(searchPath, &fd);//���һ��
	if(INVALID_HANDLE_VALUE == hSearch)
	{
		return FALSE;
	}
	
	USES_CONVERSION;
	
	do
	{
		if((0 != _tcscmp(fd.cFileName, _T("."))) && (0 != _tcscmp(fd.cFileName, _T(".."))))
		{
			_tcscpy(tmpPCPath, pathFloder);
            _tcscat(tmpPCPath, _T("\\"));
			_tcscat(tmpPCPath, fd.cFileName);

			/*�������ֻ���ҵ�ǰ��Ŀ¼*/
            if(MinSize == NULL && (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0)
			{
                if (!GetFileInfoInDir(tmpPCPath, FileCnt, FileSize, MinSize))//��ȡ��Ϣ
                {
                    FindClose(hSearch);

                    return FALSE;
                }
			}
            else
            {
	            //add produce length
                hFile = CreateFile(tmpPCPath, GENERIC_READ , FILE_SHARE_READ , NULL , 
					            OPEN_EXISTING , FILE_ATTRIBUTE_NORMAL , NULL);//����
	            if(hFile != INVALID_HANDLE_VALUE)
	            {
		            dwSize = GetFileSize(hFile, NULL);//��ȡ����
                    CloseHandle(hFile);
		            if(dwSize != 0xFFFFFFFF)
		            {
                        if (FileSize != NULL)
                        {
                            *FileSize += dwSize;//����
                        }
                        if (FileCnt != NULL)
                            (*FileCnt)++;

                        /*Ϊ���Զ�ƥ�侵�񣬲�����С�ֽڵľ����ļ�*/
                        if (MinSize != NULL)
                        {
                            if (*MinSize > dwSize)
                                *MinSize = dwSize;
                        }

                    }
		            else
		            {
                        FindClose(hSearch);
			            str.Format(_T("GetFileInfoInDir Cannot find file: %s"), tmpPCPath);
			            MessageBox(str);
                    
			            return FALSE;
		            }
                }
            }
		}
	}
	while(FindNextFile(hSearch, &fd));//��һ��

	FindClose(hSearch);//�ر�
    
	return TRUE;	
}

void DlgImageCreate::OnStaticUsedTime() 
{
	// TODO: Add your control notification handler code here	
}


void DlgImageCreate::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	DWORDLONG prog_pos;
    CString str;
    //progCtrl->StepIt();
	prog_pos = (m_image_create.FileSize * 100) / prog_file_total_size;//
	progCtrl->SetPos((int)prog_pos);
    
	if (prog_pos < 10)
	{
		str.Format(_T("%01d%%"), prog_pos);//
		SetDlgItemText(IDC_STATIC_PROG_TIME, str);//
	}
	else if (prog_pos < 100)
	{
		str.Format(_T("%02d%%"), prog_pos);//
		SetDlgItemText(IDC_STATIC_PROG_TIME, str);//
	}
	
    used_time++;
    display_used_time();//


	CDialog::OnTimer(nIDEvent);
}

void DlgImageCreate::OnSelchangeComboPagesize() 
{
	// TODO: Add your control notification handler code here
	CString get_str;
	UINT pagesize = 0;
	
	CComboBox* cbo = (CComboBox*)GetDlgItem(IDC_COMBO_PAGESIZE);//
    USES_CONVERSION;
	cbo->GetLBText(cbo->GetCurSel(), get_str);//
	pagesize = atoi(T2A(get_str));//
	
	if (pagesize > 4096)
	{
		SetDlgItemText(IDC_EDIT_SECTORSIZE, _T("4096"));//
	}
	else
	{
		SetDlgItemText(IDC_EDIT_SECTORSIZE, get_str);	//
	}
}
