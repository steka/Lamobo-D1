// PageDownload.cpp : implementation file
//

#include "stdafx.h"
#include "burntool.h"
#include "PageDownload.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CConfig theConfig;
extern CBurnToolApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CPageDownload property page

IMPLEMENT_DYNCREATE(CPageDownload, CPropertyPage)

CPageDownload::CPageDownload() : CPropertyPage(CPageDownload::IDD)
{
	//{{AFX_DATA_INIT(CPageDownload)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}

CPageDownload::~CPageDownload()
{
}

void CPageDownload::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CPageDownload)
	DDX_Control(pDX, IDC_LIST_DOWNLOAD_MTD, m_download_mtd_list);
	DDX_Control(pDX, IDC_LIST_DOWNLOAD_NAND, m_download_nand_list);
	DDX_Control(pDX, IDC_LIST_DOWNLOAD_UDISK, m_download_udisk_list);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CPageDownload, CPropertyPage)
	//{{AFX_MSG_MAP(CPageDownload)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CPageDownload message handlers
BOOL CPageDownload::get_config_data(CConfig &config)
{
	CString str;
	UINT count;
	UINT i;
	
	USES_CONVERSION;

	// free memory space assigned before
	if(config.download_udisk_data)//��Ϊ��
	{
		delete[] config.download_udisk_data;//�ͷ��ڴ�
		config.download_udisk_data = NULL;//���
	}

	if(config.download_nand_data)//��Ϊ��
	{
		delete[] config.download_nand_data;//�ͷ��ڴ�
		config.download_nand_data = NULL;//���
	}
	
	// get download udisk data
	count = m_download_udisk_list.GetItemCount();//��ȡu���ļ��ĸ���

	config.download_udisk_count = count;
	config.download_udisk_data = new T_DOWNLOAD_UDISK[count];//�����ڴ�

	for(i = 0; i < count; i++)
	{
		str = m_download_udisk_list.GetItemText(i, 1);
		config.download_udisk_data[i].bCompare = (theApp.GetString(IDS_DOWNLOAD_YES) == str) ? TRUE : FALSE;

		m_download_udisk_list.GetItemText(i, 2, config.download_udisk_data[i].pc_path, MAX_PATH);
		m_download_udisk_list.GetItemText(i, 3, config.download_udisk_data[i].udisk_path, MAX_PATH);
	}

	// get download nand data
	count = m_download_nand_list.GetItemCount();//��ȡ������ļ�����

	config.download_nand_count = count;
	config.download_nand_data = new T_DOWNLOAD_NAND[count];//�����ڴ�

	for(i = 0; i < count; i++)
	{
		str = m_download_nand_list.GetItemText(i, 1);//�Ƿ�Ƚ�
		config.download_nand_data[i].bCompare = (theApp.GetString(IDS_DOWNLOAD_YES) == str) ? TRUE : FALSE;

		m_download_nand_list.GetItemText(i, 2, config.download_nand_data[i].pc_path, MAX_PATH);

		str = m_download_nand_list.GetItemText(i, 3);//���ӵ�ַ
		config.download_nand_data[i].ld_addr = config.hex2int(T2A(str));

		m_download_nand_list.GetItemText(i, 4, config.download_nand_data[i].file_name, MAX_PATH);
		
		str = m_download_nand_list.GetItemText(i, 5);//�Ƿ񱸷�
		config.download_nand_data[i].bBackup = (theApp.GetString(IDS_BACKUP_YES) == str) ? TRUE : FALSE;		

		str = m_download_nand_list.GetItemText(i, 6);//bin����С
		//config.download_nand_data[i].bin_revs_size = config.hex2int(T2A(str));
		config.download_nand_data[i].bin_revs_size = (FLOAT)atof(T2A(str));
	}

	// get download mtd data
	count = m_download_mtd_list.GetItemCount();//��ȡ���ؾ����ļ��ĸ���

	config.download_mtd_count = count;
	config.download_mtd_data = new T_DOWNLOAD_MTD[count];//�����ڴ�

	for(i = 0; i < count; i++)
	{
		str = m_download_mtd_list.GetItemText(i, 1);//�Ƿ�Ƚ�
		config.download_mtd_data[i].bCompare = (theApp.GetString(IDS_DOWNLOAD_YES) == str) ? TRUE : FALSE;

		m_download_mtd_list.GetItemText(i, 2, config.download_mtd_data[i].pc_path, MAX_PATH);

		m_download_mtd_list.GetItemText(i, 3, config.download_mtd_data[i].disk_name, MAX_PATH);
	}
	
	return TRUE;
}

BOOL CPageDownload::set_config_item(CConfig &config)
{
	UINT i, len = 0, bin_len = 0;
	CString str;

	USES_CONVERSION;

	//set download udisk list
	for(i = 0; i < config.download_udisk_count; i++)
	{
		str.Format(_T("%d"), i+1);
		m_download_udisk_list.InsertItem(i, str);

		if(config.download_udisk_data[i].bCompare)//�Ƚ�
		{
			m_download_udisk_list.SetItemText(i, 1, theApp.GetString(IDS_DOWNLOAD_YES));
		}
		else
		{
			m_download_udisk_list.SetItemText(i, 1, theApp.GetString(IDS_DOWNLOAD_NO));
		}

		m_download_udisk_list.SetItemText(i, 2, config.download_udisk_data[i].pc_path);//PC·��
		m_download_udisk_list.SetItemText(i, 3, config.download_udisk_data[i].udisk_path);//·��
	}
	
	//set download nand list
	for(i = 0; i < config.download_nand_count; i++)
	{
		str.Format(_T("%d"), i+1);
		m_download_nand_list.InsertItem(i, str);

		if(config.download_nand_data[i].bCompare)//�Ƚ�
		{
			m_download_nand_list.SetItemText(i, 1, theApp.GetString(IDS_DOWNLOAD_YES));
		}
		else
		{
			m_download_nand_list.SetItemText(i, 1, theApp.GetString(IDS_DOWNLOAD_NO));
		}

		m_download_nand_list.SetItemText(i, 2, config.download_nand_data[i].pc_path);//PC·��

		str.Format(_T("0x%x"), theConfig.download_nand_data[i].ld_addr);//���ӵ�ַ
		m_download_nand_list.SetItemText(i, 3, str);
		
		m_download_nand_list.SetItemText(i, 4, config.download_nand_data[i].file_name);

		if(config.download_nand_data[i].bBackup)//����
		{
			m_download_nand_list.SetItemText(i, 5, theApp.GetString(IDS_BACKUP_YES));
		}
		else
		{
			m_download_nand_list.SetItemText(i, 5, theApp.GetString(IDS_BACKUP_NO));
		}
	
		if (config.planform_tpye == E_LINUX_PLANFORM)
		{
			str.Format(_T("%f"), config.download_nand_data[i].bin_revs_size);//֧��С��
			len = config.float_num(T2A(str));
			str.Format(_T("%.*f"), len, config.download_nand_data[i].bin_revs_size);//bin��С
		}
		else
		{
			bin_len = (UINT)config.download_nand_data[i].bin_revs_size;
			str.Format(_T("%d"), bin_len);//bin��С
		}
		
		m_download_nand_list.SetItemText(i, 6, str);
	}

	//set download mtd list
	for(i = 0; i < config.download_mtd_count; i++)
	{
		str.Format(_T("%d"), i+1);
		m_download_mtd_list.InsertItem(i, str);

		if(config.download_mtd_data[i].bCompare)//�Ƚ�
		{
			m_download_mtd_list.SetItemText(i, 1, theApp.GetString(IDS_DOWNLOAD_YES));
		}
		else
		{
			m_download_mtd_list.SetItemText(i, 1, theApp.GetString(IDS_DOWNLOAD_NO));
		}

		m_download_mtd_list.SetItemText(i, 2, config.download_mtd_data[i].pc_path);//pc
		m_download_mtd_list.SetItemText(i, 3, config.download_mtd_data[i].disk_name);//u��
	}

	return TRUE;
}

BOOL CPageDownload::OnInitDialog() 
{
	CPropertyPage::OnInitDialog();

	CString str;

	SetupDisplay();
	
	ListView_SetExtendedListViewStyle(m_download_udisk_list.m_hWnd, LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT); 
	ListView_SetExtendedListViewStyle(m_download_nand_list.m_hWnd, LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	ListView_SetExtendedListViewStyle(m_download_mtd_list.m_hWnd, LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	
	str = theApp.GetString(IDS_DOWNLOAD_NUMBER);
	m_download_udisk_list.InsertColumn(0, str, LVCFMT_LEFT, 40);//U��

	str = theApp.GetString(IDS_DOWNLOAD_COMPARE);
	m_download_udisk_list.InsertColumn(1, str, LVCFMT_LEFT, 60);//�Ƿ�Ƚ�

	str.Format(_T("%s|%s|"), theApp.GetString(IDS_DOWNLOAD_YES), theApp.GetString(IDS_DOWNLOAD_NO));
    m_download_udisk_list.SetSubItemPopItem(1, T_POP_CMBBOX, str);//

	str = theApp.GetString(IDS_DOWNLOAD_PC_PATH);//PC·��
	m_download_udisk_list.InsertColumn(2, str, LVCFMT_LEFT, 150);

	str.Format(_T("%s|%s|"), theApp.GetString(IDS_DOWNLOAD_FILE), theApp.GetString(IDS_DOWNLOAD_DIR));
    m_download_udisk_list.SetSubItemPopItem(2, T_POP_MENU, str);//

	str = theApp.GetString(IDS_DOWNLOAD_UDISK_PATH);//U��
	m_download_udisk_list.InsertColumn(3, str, LVCFMT_LEFT, 150);
	
	str = theApp.GetString(IDS_DOWNLOAD_NUMBER);//
	m_download_nand_list.InsertColumn(0, str, LVCFMT_LEFT, 40);

	str = theApp.GetString(IDS_DOWNLOAD_COMPARE);
	m_download_nand_list.InsertColumn(1, str, LVCFMT_LEFT, 60);

	str.Format(_T("%s|%s|"), theApp.GetString(IDS_DOWNLOAD_YES), theApp.GetString(IDS_DOWNLOAD_NO));
    m_download_nand_list.SetSubItemPopItem(1, T_POP_CMBBOX, str);
	
	str = theApp.GetString(IDS_DOWNLOAD_PC_PATH);
	m_download_nand_list.InsertColumn(2, str, LVCFMT_LEFT, 150);

	str.Format(_T("%s|"), theApp.GetString(IDS_DOWNLOAD_FILE));
    m_download_nand_list.SetSubItemPopItem(2, T_POP_MENU, str);

	//_T("Link Address")
	str = theApp.GetString(IDS_LINK_ADDRESS);
	m_download_nand_list.InsertColumn(3, str, LVCFMT_LEFT, 100);

	//_T("File Name")
	str = theApp.GetString(IDS_FILE_NAME);
	m_download_nand_list.InsertColumn(4, str, LVCFMT_LEFT, 90);

	//yes or no backup
	str = theApp.GetString(IDS_DOWNLOAD_BACKUP);
	m_download_nand_list.InsertColumn(5, str, LVCFMT_LEFT, 60);

	str.Format(_T("%s|%s|"), theApp.GetString(IDS_BACKUP_YES), theApp.GetString(IDS_BACKUP_NO));
    m_download_nand_list.SetSubItemPopItem(5, T_POP_CMBBOX, str);

	//bin file extern_size
	str = theApp.GetString(IDS_SETBIN_EXCTERNSIZE);
	m_download_nand_list.InsertColumn(6, str, LVCFMT_LEFT, 150);

	str = theApp.GetString(IDS_DOWNLOAD_NUMBER);
	m_download_mtd_list.InsertColumn(0, str, LVCFMT_LEFT, 40);

	str = theApp.GetString(IDS_DOWNLOAD_COMPARE);
	m_download_mtd_list.InsertColumn(1, str, LVCFMT_LEFT, 60);

	str.Format(_T("%s|%s|"), theApp.GetString(IDS_DOWNLOAD_YES), theApp.GetString(IDS_DOWNLOAD_NO));
    m_download_mtd_list.SetSubItemPopItem(1, T_POP_CMBBOX, str);

	str = theApp.GetString(IDS_DOWNLOAD_PC_PATH);
	m_download_mtd_list.InsertColumn(2, str, LVCFMT_LEFT, 150);

	str.Format(_T("%s|%s|"), theApp.GetString(IDS_DOWNLOAD_FILE), theApp.GetString(IDS_DOWNLOAD_DIR));
    m_download_mtd_list.SetSubItemPopItem(2, T_POP_MENU, str);

	str = theApp.GetString(IDS_DOWNLOAD_DISK_NAME);
	m_download_mtd_list.InsertColumn(3, str, LVCFMT_LEFT, 150);

	set_config_item(theConfig);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CPageDownload::SetupDisplay()
{
	CString str;

	str = theApp.GetString(IDS_DOWNLOAD_U_DISK);
	GetDlgItem(IDC_STATIC_DOWNLOAD_UDISK)->SetWindowText(str);

	str = theApp.GetString(IDS_DOWNLOAD_NANDFLASH);
	GetDlgItem(IDC_STATIC_DOWNLOAD_NAND)->SetWindowText(str);

	str = theApp.GetString(IDS_DOWNLOAD_MTD);
	GetDlgItem(IDC_STATIC_DOWNLOAD_MTD)->SetWindowText(str);
}
