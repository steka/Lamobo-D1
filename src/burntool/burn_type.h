/**
 * @file burn_lib.h
 * @brief API of burn lib 
 * Copyright (C) 2007 Anyka (GuangZhou) Software Technology Co., Ltd.
 * @author Zhijun Liao
 * @date 
 * @version 1.0
 * @ref 
 */

#ifndef __BURN_TYPE_H__
#define __BURN_TYPE_H__

typedef enum
{
	MEMORY_TYPE_SDRAM = 0,		//SDRAM
	MEMORY_TYPE_SRAM ,			//SRAM
}E_MEMORY_TYPE;

typedef enum
{
	CHIP_3223 = 0,				//оƬ�ͺţ�AK_3223
	CHIP_3224,					//оƬ�ͺţ�AK_3224
	CHIP_3225,					//оƬ�ͺţ�AK_3225
	CHIP_322L,					//оƬ�ͺţ�AK_322L
	CHIP_36XX,					//оƬ�ͺţ�AK_36XX
	CHIP_3631,					//оƬ�ͺţ�AK_3631
	CHIP_RESERVER				//����λ
}E_CHIP_TYPE;

typedef enum
{
	NULL_TRANSC = 0x0,						//NULL����
	CONN_TEST_TRANSC,						//���Դ�������
	ERASE_TRANSC,							//��������
	FORMAT_TRANSC,							//��ʽ��
	WRITE_DATA_TRANSC,						//д����
	WRITE_FILE_TRANSC,						//д�ļ�
	SET_REGISTER_TRANS,						//���üĴ���
	FAT_TRANS,								//��ʽ��								
	COMPARE_TRANS,							//�Ƚ�
	WRITE_CONFIG,							//дConfig��Ϣ
	GET_CHIPPARA_TRANS,						//��ȡChip����
	SET_CHIPPARA_TRANS,						//����Chip����
	SET_INFOR_TRANS,						//set information
	RESET_USB_TRANS,						//Reset USB
}E_TRANSC_TYPE;

typedef struct
{
	E_MEMORY_TYPE	type;		//RAM����
	UINT size;					//RAM��С
	UINT banks;					//RAM Banks
	UINT row;					//RAM row
	UINT column;				//RAM Column
	UINT control_addr;			//RAM Control register address
	UINT control_value;			//RAM Control value
}T_RAM_PARAM;


#pragma pack(1)

//NandFlash��������
typedef struct 
{
	UINT32  chip_id;			//оƬID��
	USHORT  page_size;			//Page��С
	USHORT  page_per_blk;		//һ��block����Page��
	USHORT  blk_num;			//��block��Ŀ
	USHORT  group_blk_num;		//if size is 512M, A29 must the same when copy back, we thought the A29=0 is a group,A29=1 is another group
	USHORT  plane_blk_num;		//
	UCHAR   spare_size;			//spare size
	UCHAR   col_cycle;			//column address cycle
	UCHAR   lst_col_mask;		//last column address cycle mask bit
	UCHAR   row_cycle;			//row address cycle
	UCHAR   last_row_mask;		//last row address cycle mask bit
	UCHAR   custom_nd;			//if is a custom nandflash(other not the typic nandflash), set bit, use this would fast
    UINT32	flag;				//
	UINT32  cmd_len;			//nandflash command length
    UINT32  data_len;			//nandflash data length
	UCHAR   des_str[255];		//������
}T_NAND_PHY_INFO_TRANSC;

#pragma pack()


class IComm
{
public:
	virtual BOOL Read(BYTE buf[], int size, int *upload_count) = 0;
	virtual BOOL Write(BYTE buf[], int size) = 0;

	virtual BOOL write_transc_packet(BYTE transc_type, BYTE data[], int data_length) = 0;
	virtual BYTE read_transc_ack() = 0;
};

#endif