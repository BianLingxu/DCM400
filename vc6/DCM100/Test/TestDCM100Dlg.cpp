// TestDCM50Dlg.cpp : implementation file
//
#include "stdafx.h"
#include "TestDCM100.h"
#include "TestDCM100Dlg.h"
#include "STS8100.h"
#include "STSSP8201.h"
#include "SM8213.h"
#include "SM8213_I2C.h"



/*#pragma comment(lib, "DCM100.lib")*/

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

BYTE board_slot_addr = 0;

// BYTE board_slot_addr[36] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
// 							 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 
// 							 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36};


/////////////////////////////////////////////////////////////////////////////
// CTestDCM100Dlg dialog

CTestDCM100Dlg::CTestDCM100Dlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTestDCM100Dlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTestDCM100Dlg)
	m_bindingedit = _T("");
	m_cal_relay = FALSE;
	m_sel_site = 0;
	m_pogo_relay = FALSE;
	m_sl_relay = FALSE;
	m_fl_relay = FALSE;
	m_relay_saturn1 = -1;
	m_sel_adcsite = 0;
	m_adc_os = 0;
	m_site1_DVhl_VRange = 0;
	m_site1_DVH = 2.01;
	m_site1_DVL = 0.01;
	m_site2_DVhl_VRange = 0;
	m_site2_DVH = 2.01;
	m_site2_DVL = 0.01;
	m_site1_VC_Range = -1;
	m_site2_VC_Range = -1;
	m_adresult2 = _T("");
	m_adresult1 = _T("");
	m_site1_ch_sel = 0;
	m_site2_ch_sel = 0;
	m_ext_site1_ch_sel = 0;
	m_ext_site2_ch_sel = 0;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTestDCM100Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTestDCM100Dlg)
	DDX_Text(pDX, IDC_EDIT2, m_bindingedit);
	DDX_Check(pDX, IDC_CHECK5, m_cal_relay);
	DDX_CBIndex(pDX, IDC_COMBO3, m_sel_site);
	DDX_Check(pDX, IDC_CHECK4, m_pogo_relay);
	DDX_Check(pDX, IDC_CHECK2, m_sl_relay);
	DDX_Check(pDX, IDC_CHECK3, m_fl_relay);
	DDX_Radio(pDX, IDC_RADIO1, m_relay_saturn1);
	DDX_CBIndex(pDX, IDC_COMBO2, m_sel_adcsite);
	DDX_CBIndex(pDX, IDC_COMBO1, m_adc_os);
	DDX_CBIndex(pDX, IDC_COMBO4, m_site1_DVhl_VRange);
	DDX_Text(pDX, IDC_EDIT1, m_site1_DVH);
	DDX_Text(pDX, IDC_EDIT3, m_site1_DVL);
	DDX_CBIndex(pDX, IDC_COMBO6, m_site2_DVhl_VRange);
	DDX_Text(pDX, IDC_EDIT6, m_site2_DVH);
	DDX_Text(pDX, IDC_EDIT7, m_site2_DVL);
	DDX_CBIndex(pDX, IDC_COMBO8, m_site1_VC_Range);
	DDX_CBIndex(pDX, IDC_COMBO9, m_site2_VC_Range);
	DDX_Text(pDX, IDC_EDIT11, m_adresult2);
	DDX_Text(pDX, IDC_EDIT10, m_adresult1);
	DDX_Text(pDX, IDC_EDIT12, m_site1_ch_sel);
	DDX_Text(pDX, IDC_EDIT13, m_site2_ch_sel);
	DDX_Text(pDX, IDC_EDIT14, m_ext_site1_ch_sel);
	DDX_Text(pDX, IDC_EDIT15, m_ext_site2_ch_sel);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CTestDCM100Dlg, CDialog)
	//{{AFX_MSG_MAP(CTestDCM100Dlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_binding, Onbinding)
	ON_BN_CLICKED(IDC_CHECK2, On_sl_relay)
	ON_BN_CLICKED(IDC_CHECK3, On_fl_relay)
	ON_BN_CLICKED(IDC_CHECK4, On_pogo_relay)
	ON_BN_CLICKED(IDC_CHECK5, On_cal_relay)
	ON_BN_CLICKED(IDC_RADIO1, On_sel_saturn_relay)
	ON_CBN_SELCHANGE(IDC_COMBO1, On_adc_os)
	ON_BN_CLICKED(IDC_BUTTON3, On_FPGA_CLR)
	ON_BN_CLICKED(IDC_BUTTON5, On_Saturn_Soft_Reset)
	ON_BN_CLICKED(IDC_BUTTON9, On_Site1_drvHL_output)
	ON_BN_CLICKED(IDC_BUTTON7, On_Site2_drvHL_output)
	ON_BN_CLICKED(IDC_BUTTON1, On_Site1_ext_ForceSense)
	ON_BN_CLICKED(IDC_BUTTON12, On_Site2_ext_ForceSense)
	ON_CBN_SELCHANGE(IDC_COMBO8, On_Select_Site1_VCRange)
	ON_CBN_EDITCHANGE(IDC_COMBO9, On_Select_Site2_VCRange)
	ON_BN_CLICKED(IDC_BUTTON14, On_Site1_ADC_Measure)
	ON_BN_CLICKED(IDC_BUTTON15, On_Site2_ADC_Measure)
	ON_BN_CLICKED(IDC_BUTTON16, On_loadvect)
	ON_BN_CLICKED(IDC_BUTTON17, On_checkvector)
	ON_BN_CLICKED(FPGA_DEBUG, OnFPGADebug)
	ON_BN_CLICKED(IDC_FPGAE_DBG, OnFpgaeDbg)
	ON_BN_CLICKED(IDC_BUTTON2, INER_EXT_RAM_CHECK)
	ON_BN_CLICKED(IDC_BUTTON18, OnI2CWrite)
	ON_BN_CLICKED(IDC_RADIO2, On_sel_saturn_relay)
	ON_BN_CLICKED(IDC_RADIO3, On_sel_saturn_relay)
	ON_BN_CLICKED(IDC_RADIO4, On_sel_saturn_relay)
	ON_BN_CLICKED(IDC_RADIO5, On_sel_saturn_relay)
	ON_BN_CLICKED(IDC_RADIO6, On_sel_saturn_relay)
	ON_BN_CLICKED(IDC_RADIO7, On_sel_saturn_relay)
	ON_BN_CLICKED(IDC_RADIO8, On_sel_saturn_relay)
	ON_BN_CLICKED(IDC_RADIO9, On_sel_saturn_relay)
	ON_BN_CLICKED(IDC_BUTTON4, On_Site1_drvHL_output)
	ON_BN_CLICKED(IDC_BUTTON10, On_Site2_drvHL_output)
	ON_BN_CLICKED(IDC_BUTTON11, On_Site1_ext_ForceSense)
	ON_BN_CLICKED(IDC_BUTTON13, On_Site2_ext_ForceSense)
	ON_CBN_SELCHANGE(IDC_COMBO9, On_Select_Site2_VCRange)
	ON_BN_DOUBLECLICKED(IDC_BUTTON5, OnDoubleclickedButton5)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTestDCM100Dlg message handlers

BOOL CTestDCM100Dlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CTestDCM100Dlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CTestDCM100Dlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


void CTestDCM100Dlg::Onbinding() 
{
	// TODO: Add your control notification handler code here
	ULONG ChannelUP = 0;
	ULONG check_slot = 0;

	ChannelUP = read_dw(0xfc0000 + 3*4);
	ULONG big_channelup = read_dw(0xfc0000 + 2*4);
	
	//readback the check_slot signal
// 	write_dw(0xfc0000+0xf00*4, 0x1);
// 	delay_ms(500);
// 	check_slot = read_dw(0xfc0000 + 5*4);
// 	check_slot = read_dw(0xfc0000 + 4*4);

	//set LED_TEST_ADDR --SD8201Rev300
// 	 write_dw(0xfc0000+17*4, 0x2);//close LED 
// 
// 	 write_dw(0xfc0000+17*4, 0x3);//open LED
// 
// 	 write_dw(0xfc0000+17*4, 0x0);//NORMAL 
	

	ULONG bdata = 0;


	int i = 0;
	for ( i=0; i<32; i++)
	{
		if((ChannelUP >> i) & 1  == 1)
		{
			bdata = read_dw( (i+1)<<18 );

			board_slot_addr = i+1;
		}
	}

	for ( i=0; i<4; i++)
	{
		if((big_channelup >> i) & 1  == 1)
		{
			bdata = read_dw( (i+1+32)<<18 );
			
			board_slot_addr = i+1+32;
		}
	}

	////////
	//SM8205 TEST LED --i2mode 12*16+1
// 	 write_dw(((board_slot_addr<<18)+0xC1*4) , 0x5555);
// 	 write_dw(((board_slot_addr<<18)+0xC1*4) , 0xaaaa);
// 	 write_dw(((board_slot_addr<<18)+0xC1*4) , 0xffff);
// 	 write_dw(((board_slot_addr<<18)+0xC1*4) , 0);

	dcm100_binding(0, board_slot_addr, 0, 0);

	if (bdata == 0x82130200)
 	{
		m_bindingedit = "绑定成功";
		
	}
	else
	{
		m_bindingedit = "绑定失败";
 	}
	UpdateData(FALSE);


	////
//	ULONG testd=0;
// 	for(int itest=0; itest<65535; itest++)
// 	{
// 		write_dw(((board_slot_addr<<18)+0xC2*4) , 0x55aa);
// 		write_dw(((board_slot_addr<<18)+0xC1*4) , 0x5555);
// 		testd = read_dw(((board_slot_addr<<18)+0x1019*4));
// 		//delay_ms(5000);
// 		if (testd != 0x55aa)
// 		{
// 			delay_ms(1);
// 		}
// 	}


// 	short totalboardnum = 0;
// 	totalboardnum = StsScanBoard();
// 	
// 	BOARDINFO *board_info = NULL;
// 	if(board_info)
// 	{
// 		delete [] board_info;
// 		board_info = NULL;
// 	}
// 	board_info = new BOARDINFO[totalboardnum];

// 	unsigned char dcm_slotno = 0;
// 
// 	if (totalboardnum == 1)
// 	{
// 		StsGetBoardInfo(0, &board_info[0]);
// 		
// 		if (board_info[0].boardtype == 0x8202)
// 		{
// 			dcm_slotno = board_info[0].slot;
// 			g_slotaddr = dcm_slotno;	//全局输出用于计算地址
// 		}
// 	}
// 
// 	ULONG bdata =0;
// 	bdata = dcm_binding(0, dcm_slotno, 0, 0);

// 	if (bdata == 0x82020100)
// 	{
// 		m_bindingedit = "绑定成功";
// 	}
// 	else
// 	{
// 		m_bindingedit = "绑定失败";
// 	}
// 	UpdateData(FALSE);
// 	
// 	if(board_info)
// 	{
// 		delete [] board_info;
// 		board_info = NULL;
// 	}
}


void CTestDCM100Dlg::On_sel_saturn_relay() 
{
	// TODO: Add your control notification handler code here

}

void CTestDCM100Dlg::On_cal_relay() 
{
	// TODO: Add your control notification handler code here
	UpdateData(true);

	UpdateData(false);
}

void CTestDCM100Dlg::On_sl_relay() 
{
	// TODO: Add your control notification handler code here
	UpdateData(true);

	UpdateData(false);
}

void CTestDCM100Dlg::On_fl_relay() 
{
	// TODO: Add your control notification handler code here
	UpdateData(true);

	UpdateData(false);
}

void CTestDCM100Dlg::On_pogo_relay() 
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
	
	UpdateData(false);
}




void CTestDCM100Dlg::On_adc_os() 
{
	// TODO: Add your control notification handler code here
	UpdateData(true);
	UpdateData(false);
}




void CTestDCM100Dlg::On_FPGA_CLR() 
{
	// TODO: Add your control notification handler code here
	ULONG rddata = 0;
	ULONG board = board_slot_addr;
	ULONG ctrl_addr1 = 0xfc0000 + 0x1024*4;
	ULONG waddr2 = 0xfc0000 + 0x1025*4;
	ULONG wlth_addr3 = 0xfc0000 + 0x1026*4;
	ULONG wd_addr4 = 0xfc0000 + 0x1027*4;
	ULONG cmd_addr5= 0xfc0000 + 0x1028*4;
	ULONG rd_flash_addr = 0xfc0000 + 0xd*4;
	ULONG rd_data =0;

	write_dw( ctrl_addr1, 0x8000);
	write_dw( cmd_addr5, 0x9F); //RDID
 	_delay_ms(1);
 	rd_data =  read_dw(rd_flash_addr);//read flash datao
 
	write_dw( cmd_addr5, 0x05);//RDSR
	_delay_ms(1);
	rd_data =  read_dw(rd_flash_addr);//read flash datao



	BYTE head_data[112] = {0};
	
	ULONG checkdata[64] = {0};
	BYTE sdata[256] = {0};
	BYTE cdata[256] = {0};
	BYTE othsize = 0;
	int lptime  = 0;
	BYTE read_fail = 0;
	ULONG fsize = 0;
	BYTE chkres = 0;
	FILE *fp = NULL;
	char bitfilename[] = "E:\\sfp_fpga1008.bit";
	int loop=0;
	//----------------Erase-------------------//
/*
	write_dw( cmd_addr5, 0x06); //WDEN
	_delay_ms(1);
	// CMD=0xD8 Sector Erase 3 second
	// CMD=0xC7 bulk Erase 80 second
	write_dw( cmd_addr5, 0xC7);
	_delay_ms(80000);
*/
	//-------------------------------------//
	// -----------------循环写入-----------//

	fp=fopen(bitfilename,"rb+");
	fseek(fp, 0, SEEK_END);
	fsize=ftell(fp);
	
	fseek(fp, 0, SEEK_SET);
	fread(head_data,sizeof(BYTE),112,fp);
	fclose(fp);

	lptime = (fsize-112)/256; //16384;
	othsize = (fsize-112)%256;
	//write flash
	for ( loop=0; loop<=lptime; loop++)//16384*256*8 = 32Mbit
//	for (int loop=0; loop<1; loop++)
	{
		
		chkres = 0;
		fp=fopen(bitfilename,"rb+");
		fseek(fp, (loop*256+112), SEEK_SET);
		fread(cdata,sizeof(BYTE),256,fp);
		fclose(fp);
		
		//read flash
		write_dw( ctrl_addr1, 0x80000000);//clr_ram_addr // 回读fifo数据
		write_dw( ctrl_addr1, 0x0);//
		
		write_dw( waddr2, loop*256);//waddr
		write_dw( wlth_addr3, 0x40);// length "64DB=256B"

		write_dw( ctrl_addr1, 0x80000000);//clr_ram_addr
		write_dw( ctrl_addr1, 0x0);//clr_ram_addr
		int j = 0;
		for (j=0; j<64; j++)
		{
			checkdata[j] = 0;
		}
		for (j=0; j<256; j++)
		{
			checkdata[j/4] = checkdata[j/4] | (cdata[j]<<((3-(j%4))*8)); //combind data
		}
		for (j=0; j<64; j++)
		{
			_delay_us(1);
			write_dw( wd_addr4, checkdata[j]);//write data
			_delay_us(1);
		}
		write_dw( ctrl_addr1, 0x80000000);//clr_ram_addr
		write_dw( ctrl_addr1, 0x40004000);//clr_ram_addr

		for (j=0; j<64; j++)
		{
			
			rddata = read_dw(rd_flash_addr);//write data

			if (rddata!=checkdata[j])
			{
				rddata = 0;
			}
			
		}

		//-------------------//
		//write_dw( cmd_addr5, 0x06); //WDEN
		//_delay_us(10);
		//write_dw( cmd_addr5, 0x02);//PP
 		//_delay_us(500);
		
// 		write_dw( cmd_addr5, 0x03); //READ FLASH
// 		_delay_us(200);
// 		
// 		write_dw( ctrl_addr1, 0x80000000);//clr_ram_addr
// 		write_dw( ctrl_addr1, 0x20000000);//clr_ram_addr+read_ram selfcheck
// 		
// 		
// 		for (int j=0; j<64; j++)
// 		{
// 			checkdata[j] = read_dw(rd_flash_addr);//read flash data
// 		}
// 		
		
		write_dw( ctrl_addr1, 0x80000000);//clr_ram_addr
		write_dw( ctrl_addr1, 0x00000000);//clr_ram_addr
		
// 		FILE *fp = NULL;
// 		char filename[] = "E:\\spi_flash_data.bin";
// 		
// 		fp=fopen(filename,"rb+");
// 		if (fp==NULL)	// 打开文件失败
// 		{
// 			fp=fopen(filename,"wb+");
// 		} 
// 		
// 		fseek(fp, 0, SEEK_END);
// 		
// 		for (int i=0; i<256; i++)
// 		{
// 			sdata[i] = checkdata[i/4]>>((3-i)*8) & 0xff; //调整字节输出顺序
// 		}
// 		
// 		for (i=0; i<256; i++)
// 		{
// 			if (sdata[i] != cdata[i])
// 			{
// 				chkres = 1;
// 				break;
// 			}	
// 		}
// 		
// 		
// 		if (chkres==0)//正确就写入文件
// 		{
// 			fwrite(sdata,sizeof(BYTE),256,fp);
// 		}
// 		else if ((loop==lptime)&(othsize==i))
// 		{
// 			fwrite(sdata,sizeof(BYTE),256,fp);
// 		}
// 		else
// 		{
// 			loop--;
// 			read_fail = 1;
// 		}
// 		fclose(fp);
		
		for (int i=0; i<256; i++)
		{
			cdata[i] = 0xff;
		}
		
	}


    //-------------------------------//
	//------- 循环读出验证-----------//
	
	
	fp=fopen(bitfilename,"rb+");
	fseek(fp, 0, SEEK_END);
	fsize=ftell(fp);

	fseek(fp, 0, SEEK_SET);
	fread(head_data,sizeof(BYTE),112,fp);
	fclose(fp);

	
	char filename[] = "E:\\spi_flash_data.bin";
	fp=fopen(filename,"rb+");
	if (fp==NULL)	// 打开文件失败
	{
		fp=fopen(filename,"wb+");
	} 
	fseek(fp, 0, SEEK_SET);
	fwrite(head_data,sizeof(BYTE),112,fp);
	fclose(fp);
	
	lptime = (fsize-112)/256; //16384;
	othsize = (fsize-112)%256;


	
	for ( loop=0; loop<=lptime; loop++)//16384*256*8 = 32Mbit
	{
	
		chkres = 0;
		fp=fopen(bitfilename,"rb+");
		fseek(fp, (loop*256+112), SEEK_SET);
		fread(cdata,sizeof(BYTE),256,fp);
		fclose(fp);

		//read flash
		write_dw( ctrl_addr1, 0x80000000);//clr_ram_addr // 回读fifo数据
		write_dw( ctrl_addr1, 0x0);//

		write_dw( waddr2, loop*256);//waddr
		write_dw( wlth_addr3, 0x40);// length "64DB=256B"

		write_dw( cmd_addr5, 0x03); //READ FLASH
		_delay_us(200);

		write_dw( ctrl_addr1, 0x80000000);//clr_ram_addr
		write_dw( ctrl_addr1, 0x20000000);//clr_ram_addr+read_ram selfcheck


		for (int j=0; j<64; j++)
		{
			checkdata[j] = 0;
			checkdata[j] = read_dw(rd_flash_addr);//read flash data
		}


		write_dw( ctrl_addr1, 0x80000000);//clr_ram_addr
		write_dw( ctrl_addr1, 0x00000000);//clr_ram_addr

		FILE *fp = NULL;
		char filename[] = "E:\\spi_flash_data.bin";

		fp=fopen(filename,"rb+");
		if (fp==NULL)	// 打开文件失败
		{
			fp=fopen(filename,"wb+");
		} 

		fseek(fp, 0, SEEK_END);
		int i = 0;
		for (i=0; i<256; i++)
		{
			sdata[i] = checkdata[i/4]>>((3-i)*8) & 0xff; //调整字节输出顺序
		}
		
		for (i=0; i<256; i++)
		{
			if (sdata[i] != cdata[i])
			{
				chkres = 1;
				break;
			}	
		}
		

		if (chkres==0)//正确就写入文件
		{
			fwrite(sdata,sizeof(BYTE),256,fp);
		}
		else if ((loop==lptime)&(othsize==i))
		{
			fwrite(sdata,sizeof(BYTE),256,fp);
		}
		else
		{
			loop--;
			read_fail = 1;
		}
		fclose(fp);

		for (i=0; i<256; i++)
		{
			cdata[i] = 0xff;
		}
	
	}
	
	
	
	if(read_fail)
		chkres = 1;
	else
		chkres = 0;
	
	// 
	// 	write_dw( cmd_addr5, 0x06); //WDEN
	// 	_delay_ms(1);
	// 	write_dw(ctrl_addr1, 0x80ff);//rddata = 0x9C
	// 	write_dw( cmd_addr5, 0x01);//WRSR
	// 	_delay_ms(1);
	// 	write_dw( cmd_addr5, 0x05);//RDSR
	// 	_delay_ms(1);
	// 	rd_data =  read_dw(rd_flash_addr);//read flash datao
	// 
	// 	write_dw( cmd_addr5, 0x06); //WDEN
	// 	_delay_ms(1);
	// 	write_dw(ctrl_addr1, 0x8000);
	// 	write_dw( cmd_addr5, 0x01);//WRSR
	// 	_delay_ms(1);
	// 	write_dw( cmd_addr5, 0x05);//RDSR
	// 	_delay_ms(1);
	// 	rd_data =  read_dw(rd_flash_addr);//read flash datao



// 	for (int j=0; j<128; j++)
// 	{
// 		wdata[j] = j+ (j<<8) + (j<<16) + (j<<24);
// 	}
// 
// 	write_dw( (board<<18) + (0xa016<<2), 0x80000000);//clr_ram_addr
// 	write_dw( (board<<18) + (0xa016<<2), 0x0);//clr_ram_addr
// 	for (j=0; j<128; j++)
// 	{
// 		write_dw( (board<<18) + (0xa010<<2), wdata[j]);//write data
// 	}
// 	write_dw( (board<<18) + (0xa016<<2), 0x80000000);//clr_ram_addr
// 	write_dw( (board<<18) + (0xa016<<2), 0x40000000);//clr_ram_addr+selfcheck enable
// 
// 
// 	for (j=0; j<128; j++)
// 	{
// 		rdata[j] = read_dw((board<<18) +(0xa005<<2));//read w ram data
// 	}
// 
// 	for (j=0; j<128; j++)
// 	{
// 		if(rdata[j]!=wdata[j]) 
// 		{
// 			j++;
// 		}
// 	}
// 
// 	write_dw( (board<<18) + (0xa016<<2), 0x80000000);//clr_ram_addr
// 	write_dw( (board<<18) + (0xa016<<2), 0x0);//clr_ram_addr
// 
// 
// 	
// 	write_dw( (board<<18) + (0xa012<<2), 0x00);//waddr&raddr
// 	write_dw( (board<<18) + (0xa013<<2), 0x40);//write&read length "PP Max=256B"
// //	write_dw( (board<<18) + (0xa014<<2), 0x00);//raddr
// //	write_dw( (board<<18) + (0xa015<<2), 0x02);//read length
// 
// 	write_dw( (board<<18) + (0xa016<<2), 0x80000000);//clr_ram_addr
// 	write_dw( (board<<18) + (0xa016<<2), 0x0);//clr_ram_addr
// 
// 	write_dw( (board<<18) + (0xa011<<2), 0x06); //WDEN
// 	_delay_ms(1);
// 	write_dw( (board<<18) + (0xa011<<2), 0xd8); //CMD=0xD8 Sector Erase 3 second
// 	// CMD=0xC7 bulk Erase 80 second
// 	//_delay_ms(3000);
// 
// 	rd_fifo_data = 1;
// 	while ((rd_fifo_data&0x1) == 0x1)
// 	{   
// 		_delay_ms(10);
// 		write_dw( (board<<18) + (0xa011<<2), 0x05);//RDSR
// 		_delay_ms(10);
// 		rd_fifo_data =  read_dw((board<<18) +(0xa006<<2));//read flash datao
// 
// 	}
// 
// 
// 
// 	write_dw( (board<<18) + (0xa011<<2), 0x06); //WDEN
// 	_delay_ms(1);
// 	write_dw( (board<<18) + (0xa011<<2), 0x02);//PP
// 	_delay_ms(1000);
// // 	rd_fifo_data = 1;
// // 	while ((rd_fifo_data&0x1) == 0x1)
// // 	{
// // 		write_dw( (board<<18) + (0xa011<<2), 0x05);//RDSR
// // 		_delay_ms(10);
// // 		rd_fifo_data =  read_dw((board<<18) +(0xa006<<2));//read flash datao
// // 		
// // 	}
// 
// 	write_dw( (board<<18) + (0xa012<<2), 0x100);//waddr
// 	write_dw( (board<<18) + (0xa013<<2), 0x40);//write length "PP Max=256B"
// 
// 	write_dw( (board<<18) + (0xa011<<2), 0x06); //WDEN
// 	_delay_ms(1);
// 	write_dw( (board<<18) + (0xa011<<2), 0x02);//PP
// 	_delay_ms(1000);
// // 	rd_fifo_data = 1;
// // 	while ((rd_fifo_data&0x1) == 0x1)
// // 	{
// // 		write_dw( (board<<18) + (0xa011<<2), 0x05);//RDSR
// // 		_delay_ms(10);
// // 		rd_fifo_data =  read_dw((board<<18) +(0xa006<<2));//read flash datao
// // 		
// // 	}
// 
// 
// //	write_dw( (board<<18) + (0xa012<<2), 0x0);//waddr
// //	write_dw( (board<<18) + (0xa013<<2), 0x40);//write length "PP Max=256B"
// 

}

/*
//	驱动器软复位及初始化函数，用于软件复位saturn，
//	断开输出继电器relay2
//	设置vmid_adj = 0x1011; 
//	设置偏置为0x7FFF; 设置增益为0x7FFF;
//	设置参考点为Saturn内部地作为基准。
*/
void CTestDCM100Dlg::On_Saturn_Soft_Reset()
{
	ULONG rdata=0;
	LARGE_INTEGER tick;
	LARGE_INTEGER time1;
	LARGE_INTEGER time2;
	QueryPerformanceFrequency(&tick);
	
	QueryPerformanceCounter(&time1);
	for (int i=0; i<1000; i++)
	{
		//write_dw(0x8000000, 0);
		//write_dw(0xfc0000, 0);
		//write_dw((board_slot_addr<<18) + i*4 , 0);

		rdata = read_dw(0xfc0000+3*4);
		//rdata = read_dw((board_slot_addr<<18) + i*4 );
		//rdata = read_dw(0x8000000);
	}
	QueryPerformanceCounter(&time2);
	double us = (time2.QuadPart - time1.QuadPart)*1e6/tick.QuadPart;

	// TODO: Add your control notification handler code here
// 	initdata();
// 	dcm_set_pmu_ctrl_data_source(board_slot_addr, CTRLA, PC_DATA);
// 	dcm_set_pmu_ctrl_data_source(board_slot_addr, CTRLB, PC_DATA);
// 	
// 	unsigned char chs[1024] = {0};
// 	unsigned short data[1024] = {0};
// 	
// 	int channelno = 32;
// 	for (int i=0; i<channelno; i++)
// 	{
// 		chs[i] = 1;
// 	}
// 	
// 	for (i=0; i<channelno; i++)
// 	{
// 		data[i] = 0x0000;
// 	}
// 	set_saturn_pc(0xC002, data, chs);		//软件复位
// 	delay_ms(2);
// 	
// 	for (i=0; i<channelno; i++)
// 	{
// 		data[i] = 0x4000;
// 	}
// 	set_saturn_pc(0x8006, data, chs);		//断开输出继电器
// 	delay_ms(2);
// 
// 	for (i=0; i<channelno; i++)
// 	{
// 		data[i] = 0x0010|Vmid_Adj;
// 	}
// 	set_saturn_pc(0xC003, data, chs);		//设置Vmid_bias
// 	
// 	for (i=0; i<channelno; i++)
// 	{
// 		data[i] = 0x7FFF;
// 	}
// 	set_saturn_pc(0x0020, data, chs);		//设置DVH offset
// 	set_saturn_pc(0x0040, data, chs);		//设置DVH gain
// 	set_saturn_pc(0x0021, data, chs);		//设置DVL offset
// 	set_saturn_pc(0x0041, data, chs);		//设置DVL gain
// 
// // 	for (i=0; i<channelno; i++)
// // 	{
// // 		data[i] = 0xFFFF;
// // 	}
// // 	set_saturn_pc(0x0000, data, chs);		//设置DAC DC Level
// 
// 	for (i=0; i<channelno; i++)
// 	{
// 		data[i] = 0x168C;					//器件地，将Din接到Monitor上
// 	}
// 	set_saturn_pc(0x8007, data, chs);		//设置内部基准
}

void CTestDCM100Dlg::On_Site1_drvHL_output() 
{
	// TODO: Add your control notification handler code here
// 	WORD buttonid = LOWORD(GetCurrentMessage()->wParam);	//获取按钮的id号
// 
// 	UpdateData(true);
// 	initdata();
// 	dcm_set_pmu_ctrl_data_source(board_slot_addr, CTRLA, PC_DATA);
// 	unsigned char chs[1024] = {0};
// 	unsigned short data[1024] = {0};
// 	int channelno = 16;
// 
// 	for (int i=0; i<channelno; i++)
// 	{
// 		chs[i] = 1;
// 	}
// 
// 	for (i=0; i<channelno; i++)
// 	{
// 		data[i] = 0x5000;
// 	}
// 	set_saturn_pc(0x8006, data, chs);		// 输出继电器CBIT2闭合
// 
// 	switch(buttonid)
// 	{
// 	case IDC_BUTTON9:
// 		for (i=0; i<channelno; i++)
// 		{
// 			data[i] = 0x012D;		// cpu 输出 高
// 		}
// 	    break;
// 	case IDC_BUTTON4:
// 	default:
// 		for (i=0; i<channelno; i++)
// 		{
// 			data[i] = 0x012C;		// cpu 输出 低
// 		}
// 		break;
// 	}
// 	set_saturn_pc(0x8000, data, chs);		
// 
// 	switch(m_site1_DVhl_VRange)
// 	{
// 	case 1:
// 		m_site1_DVhl_VRange = VR_16V;
// 		break;
// 	case 2:
// 		m_site1_DVhl_VRange = VR_32V;
// 	    break;
// 	case 0:
// 	default:
// 		m_site1_DVhl_VRange = VR_8V;	
// 	    break;
// 	}
// 	dcm_set_driver_voltage(m_site1_DVH, m_site1_DVL, m_site1_DVhl_VRange, chs);
// 
// 	UpdateData(false);
}

void CTestDCM100Dlg::On_Site2_drvHL_output() 
{
	// TODO: Add your control notification handler code here
// 	WORD buttonid = LOWORD(GetCurrentMessage()->wParam);	//获取按钮的id号
// 	UpdateData(true);
// 	initdata();
// 	dcm_set_pmu_ctrl_data_source(board_slot_addr, CTRLB, PC_DATA);
// 	unsigned char chs[1024] = {0};
// 	unsigned short data[1024] = {0};
// 	int channelno = 32;
// 	
// 	for (int i=16; i<channelno; i++)
// 	{
// 		chs[i] = 1;
// 	}
// 	
// 	for (i=16; i<channelno; i++)
// 	{
// 		data[i] = 0x5000;
// 	}
// 	set_saturn_pc(0x8006, data, chs);		// 输出继电器CBIT2闭合
// 	
// 	switch(buttonid)
// 	{
// 	case IDC_BUTTON7:
// 		for (i=16; i<channelno; i++)
// 		{
// 			data[i] = 0x012D;		// cpu 输出 高
// 		}
// 		break;
// 	case IDC_BUTTON10:
// 	default:
// 		for (i=16; i<channelno; i++)
// 		{
// 			data[i] = 0x012C;		// cpu 输出 低
// 		}
// 		break;
// 	}
// 	set_saturn_pc(0x8000, data, chs);		
// 	
// 	switch(m_site2_DVhl_VRange)
// 	{
// 	case 1:
// 		m_site2_DVhl_VRange = VR_16V;
// 		break;
// 	case 2:
// 		m_site2_DVhl_VRange = VR_32V;
// 		break;
// 	case 0:
// 	default:
// 		m_site2_DVhl_VRange = VR_8V;	
// 		break;
// 	}
// 	dcm_set_driver_voltage(m_site2_DVH, m_site2_DVL, m_site2_DVhl_VRange, chs);
// 	
// 	UpdateData(false);
}

void CTestDCM100Dlg::On_Site1_ext_ForceSense() 
{
	// TODO: Add your control notification handler code here

// 	WORD buttonid = LOWORD(GetCurrentMessage()->wParam);	//获取按钮的id号
// 
// 	UpdateData(true);
// 	initdata();
// 	dcm_set_pmu_ctrl_data_source(board_slot_addr, CTRLA, PC_DATA);
// 	unsigned char chs[1024] = {0};
// 	unsigned short data[1024] = {0};
// 
// 	if (m_ext_site1_ch_sel % 2)	//奇数通道
// 	{
// 		chs[m_ext_site1_ch_sel-1] = 1;					//相邻通道关闭
// 		switch(buttonid)
// 		{
// 		case IDC_BUTTON12:
// 			data[m_ext_site1_ch_sel-1] = 0x0800;		// 断开force开关
// 			break;
// 		case IDC_BUTTON13:
// 		default:
// 			data[m_ext_site1_ch_sel-1] = 0x0200;		// 断开sense开关
// 			break;
// 		}
// 		set_saturn_pc(0x8006, data, chs);
// 	} 
// 	else						//偶数通道
// 	{
// 		chs[m_ext_site1_ch_sel+1] = 1;				//相邻通道关闭
// 		switch(buttonid)
// 		{
// 		case IDC_BUTTON12:
// 			data[m_ext_site1_ch_sel+1] = 0x0800;		// 断开force开关
// 			break;
// 		case IDC_BUTTON13:
// 		default:
// 			data[m_ext_site1_ch_sel+1] = 0x0200;		// 断开sense开关
// 			break;
// 		}
// 		set_saturn_pc(0x8006, data, chs);
// 	}
// 
// 	chs[m_ext_site2_ch_sel] = 1;	
// 	switch(buttonid)
// 	{
// 	case IDC_BUTTON1:
// 		data[m_ext_site1_ch_sel] = 0x0C00;		// 闭合force开关
// 		break;
// 	case IDC_BUTTON11:
// 	default:
// 		data[m_ext_site1_ch_sel] = 0x0300;		// 闭合sense开关
// 		break;
// 	}
// 	set_saturn_pc(0x8006, data, chs);
}

void CTestDCM100Dlg::On_Site2_ext_ForceSense() 
{
	// TODO: Add your control notification handler code here
// 	WORD buttonid = LOWORD(GetCurrentMessage()->wParam);	//获取按钮的id号
// 	
// 	UpdateData(true);
// 	initdata();
// 	dcm_set_pmu_ctrl_data_source(board_slot_addr, CTRLB, PC_DATA);
// 	unsigned char chs[1024] = {0};
// 	unsigned short data[1024] = {0};
// 
// 	m_ext_site2_ch_sel = m_ext_site2_ch_sel + 16;
// 
// 	if (m_ext_site2_ch_sel % 2)	//奇数通道
// 	{
// 		chs[m_ext_site2_ch_sel-1] = 1;					//相邻通道关闭
// 		switch(buttonid)
// 		{
// 		case IDC_BUTTON12:
// 			data[m_ext_site2_ch_sel-1] = 0x0800;		// 断开force开关
// 			break;
// 		case IDC_BUTTON13:
// 		default:
// 			data[m_ext_site2_ch_sel-1] = 0x0200;		// 断开sense开关
// 			break;
// 		}
// 		set_saturn_pc(0x8006, data, chs);
// 	} 
// 	else						//偶数通道
// 	{
// 		chs[m_ext_site2_ch_sel+1] = 1;				//相邻通道关闭
// 		switch(buttonid)
// 		{
// 		case IDC_BUTTON12:
// 			data[m_ext_site2_ch_sel+1] = 0x0800;		// 断开force开关
// 			break;
// 		case IDC_BUTTON13:
// 		default:
// 			data[m_ext_site2_ch_sel+1] = 0x0200;		// 断开sense开关
// 			break;
// 		}
// 		set_saturn_pc(0x8006, data, chs);
// 	}
// 	
// 	initdata();
// 	chs[m_ext_site2_ch_sel] = 1;	
// 	switch(buttonid)
// 	{
// 	case IDC_BUTTON12:
// 		data[m_ext_site2_ch_sel] = 0x0C00;		// 闭合force开关
// 		break;
// 	case IDC_BUTTON13:
// 	default:
// 		data[m_ext_site2_ch_sel] = 0x0300;		// 闭合sense开关
// 		break;
// 	}
// 	set_saturn_pc(0x8006, data, chs);
}


void CTestDCM100Dlg::On_Select_Site1_VCRange() 
{
	// TODO: Add your control notification handler code here
// 	unsigned char chs[1024] = {0};
// 	unsigned char vcrange[1024] = {0};
// 	int channelno = 16;
// 	unsigned char tempdata = 0;
// 	UpdateData(true);
// 	for (int i=0; i<channelno; i++)
// 	{
// 		chs[i] = 1;
// 	}
// 	switch(m_site1_VC_Range)
// 	{
// 	case 0:
// 		tempdata = VRANGE_0_25;
// 		break;
// 	case 1:
// 		tempdata = VRANGE_1;
// 		break;
// 	case 2:
// 		tempdata = VRANGE_5;
// 	    break;
// 	case 3:
// 		tempdata = VRANGE_10;
// 	    break;
// 	default:
// 		tempdata = VRANGE_0_25;
// 	    break;
// 	}
// 
// 	for (i=0; i<channelno; i++)
// 	{
// 		vcrange[i] = tempdata;
// 	}
// 	dcm_set_vc_range_pc(vcrange, chs);
}

void CTestDCM100Dlg::On_Select_Site2_VCRange() 
{
	// TODO: Add your control notification handler code here
	// TODO: Add your control notification handler code here
// 	unsigned char chs[1024] = {0};
// 	unsigned char vcrange[1024] = {0};
// 	int channelno = 32;
// 	unsigned char tempdata = 0;
// 	UpdateData(true);
// 	for (int i=16; i<channelno; i++)
// 	{
// 		chs[i] = 1;
// 	}
// 	switch(m_site2_VC_Range)
// 	{
// 	case 0:
// 		tempdata = VRANGE_0_25;
// 		break;
// 	case 1:
// 		tempdata = VRANGE_1;
// 		break;
// 	case 2:
// 		tempdata = VRANGE_5;
// 		break;
// 	case 3:
// 		tempdata = VRANGE_10;
// 		break;
// 	default:
// 		tempdata = VRANGE_0_25;
// 		break;
// 	}
// 	
// 	for (i=16; i<channelno; i++)
// 	{
// 		vcrange[i] = tempdata;
// 	}
// 	dcm_set_vc_range_pc(vcrange, chs);
}

void CTestDCM100Dlg::On_Site1_ADC_Measure() 
{
	// TODO: Add your control notification handler code here


	

//	UpdateData(false);
}

void CTestDCM100Dlg::On_Site2_ADC_Measure() 
{
	// TODO: Add your control notification handler code here
	// TODO: Add your control notification handler code here


	BOOL selfcheckres = false;
	
	char filename[] = "..\\PPTMU_Calib.txt";
	BYTE chno = 0;
	//	ULONG wdata  = 0;
	// 	ULONG mdata[16] = {0x0,       //测试TMUSTA到Foutp的时间 
	// 					   0x240000, //测试上升时间Tr
	// 					   0x530000, //测试下降时间Tf
	// 					   0x320000, //测试正脉宽
	// 					   0x230000, //测试负脉宽
	// 					   0x0,      //测试TMUSTA到Foutp的时间
	// 					   0x100000, //测试TMUSTA到Foutn的时间
	// 					   0x200000, //测试TMUSTA到Hp的时间
	// 					   0x300000, //测试TMUSTA到Hn的时间
	// 					   0x400000, //测试TMUSTA到Lp的时间
	// 					   0x500000, //测试TMUSTA到Ln的时间
	// 					   0x6603e8, //当当TCLKSEL = 1是内部信号测量，当为0时为H波形周期测量。
	// 					   0x660064, //当当TCLKSEL = 1是内部信号测量，当为0时为H波形周期测量。，
	// 					   0x0,
	// 					   0x0,
	// 					   0x406603e8 //当TCLKSEL = 1是calib，当为0是H时钟波形周期测量。
// 	}; //calib operand data	
	BYTE mode = 0;

	dcm100_calib_check(filename, board_slot_addr, chno, mode, 0);


	

}

void CTestDCM100Dlg::On_loadvect() 
{
	// TODO: Add your control notification handler code here
//	dch50_loadvectfile("D:\\STS8250\\7400.vec");

	char filename[] = "..\\rpt.txt";

      dcm100_sdram_selfcheck(filename,board_slot_addr, 0);
}

void CTestDCM100Dlg::On_checkvector() 
{
	// TODO: Add your control notification handler code here
	ULONG ChannelUP = 0;
	
	if (board_slot_addr==0)
	{
		ChannelUP = read_dw(0xfc0000 + 3*4);
		for (int i=0; i<32; i++)
		{
			if((ChannelUP >> i) & 1  == 1)
			{
				board_slot_addr = i+1;
			}
		}
	}

	char filename[] = "..\\PPTMU_Calib.txt";

	dcm100_wavecheck(filename, board_slot_addr, 0);
}



void CTestDCM100Dlg::OnFpgaeDbg() 
{
	// TODO: Add your control notification handler code here
		ULONG data = 0x55aaaa55;
	ULONG rddata = 0;
	ULONG board = board_slot_addr;
	//address check
	int i = 0;
	for (i=0; i<0x1fff; i++)
	{
		data = 0x0 + i;
		write_dw( (board<<18) + ((0xa000+i)<<2), data);
		rddata = read_dw((board<<18) +(0xa004<<2)); //read address
		
		if ((rddata&0x1fff)!=data)
		{
			i++;
		}
		
	}

	//data register check
	for ( i=0; i<0xffff; i++)
	{
		data = i;
		write_dw( (board<<18) + (0xa001<<2), data);
		write_dw((board<<18) + (0xa002<<2), (data+1));
		rddata = read_dw((board<<18) +(0xa003<<2)); //read self check register
		
		if (rddata!=data)
		{
			i++;
		}
	}

    //data register check
	for ( i=0; i<0xffff; i++)
	{
		data = i<<16;
		write_dw( (board<<18) + (0xa001<<2), data);
		write_dw((board<<18) + (0xa002<<2), (data+1));
		rddata = read_dw((board<<18) +(0xa003<<2)); //read self check register

		if (rddata!=data)
		{
			i++;
		}

	}

	ULONG checkdata = 0;
	ULONG wdata[128] = {0};
	ULONG rdata[128] = {1};
	ULONG rd_fifo_data = 1;
	int j = 0;
	for (j=0; j<128; j++)
	{
		wdata[j] = j+ (j<<8) + (j<<16) + (j<<24);
	}

	write_dw( (board<<18) + (0xa016<<2), 0x80000000);//clr_ram_addr
	write_dw( (board<<18) + (0xa016<<2), 0x0);//clr_ram_addr
	for (j=0; j<128; j++)
	{
		write_dw( (board<<18) + (0xa010<<2), wdata[j]);//write data
	}
	write_dw( (board<<18) + (0xa016<<2), 0x80000000);//clr_ram_addr
	write_dw( (board<<18) + (0xa016<<2), 0x40000000);//clr_ram_addr+selfcheck enable


	for (j=0; j<128; j++)
	{
		rdata[j] = read_dw((board<<18) +(0xa005<<2));//read w ram data
	}

	for (j=0; j<128; j++)
	{
		if(rdata[j]!=wdata[j]) 
		{
			j++;
		}
	}

	write_dw( (board<<18) + (0xa016<<2), 0x80000000);//clr_ram_addr
	write_dw( (board<<18) + (0xa016<<2), 0x0);//clr_ram_addr


	
	write_dw( (board<<18) + (0xa012<<2), 0x00);//waddr&raddr
	write_dw( (board<<18) + (0xa013<<2), 0x40);//write&read length "PP Max=256B"
//	write_dw( (board<<18) + (0xa014<<2), 0x00);//raddr
//	write_dw( (board<<18) + (0xa015<<2), 0x02);//read length

	write_dw( (board<<18) + (0xa016<<2), 0x80000000);//clr_ram_addr
	write_dw( (board<<18) + (0xa016<<2), 0x0);//clr_ram_addr

	write_dw( (board<<18) + (0xa011<<2), 0x06); //WDEN
	_delay_ms(1);
	write_dw( (board<<18) + (0xa011<<2), 0xd8); //CMD=0xD8 Sector Erase 3 second
	// CMD=0xC7 bulk Erase 80 second
	//_delay_ms(3000);

	rd_fifo_data = 1;
	while ((rd_fifo_data&0x1) == 0x1)
	{   
		_delay_ms(10);
		write_dw( (board<<18) + (0xa011<<2), 0x05);//RDSR
		_delay_ms(10);
		rd_fifo_data =  read_dw((board<<18) +(0xa006<<2));//read flash datao

	}



	write_dw( (board<<18) + (0xa011<<2), 0x06); //WDEN
	_delay_ms(1);
	write_dw( (board<<18) + (0xa011<<2), 0x02);//PP
	_delay_ms(1000);
// 	rd_fifo_data = 1;
// 	while ((rd_fifo_data&0x1) == 0x1)
// 	{
// 		write_dw( (board<<18) + (0xa011<<2), 0x05);//RDSR
// 		_delay_ms(10);
// 		rd_fifo_data =  read_dw((board<<18) +(0xa006<<2));//read flash datao
// 		
// 	}

	write_dw( (board<<18) + (0xa012<<2), 0x100);//waddr
	write_dw( (board<<18) + (0xa013<<2), 0x40);//write length "PP Max=256B"

	write_dw( (board<<18) + (0xa011<<2), 0x06); //WDEN
	_delay_ms(1);
	write_dw( (board<<18) + (0xa011<<2), 0x02);//PP
	_delay_ms(1000);
// 	rd_fifo_data = 1;
// 	while ((rd_fifo_data&0x1) == 0x1)
// 	{
// 		write_dw( (board<<18) + (0xa011<<2), 0x05);//RDSR
// 		_delay_ms(10);
// 		rd_fifo_data =  read_dw((board<<18) +(0xa006<<2));//read flash datao
// 		
// 	}


//	write_dw( (board<<18) + (0xa012<<2), 0x0);//waddr
//	write_dw( (board<<18) + (0xa013<<2), 0x40);//write length "PP Max=256B"

	write_dw( (board<<18) + (0xa016<<2), 0x80000000);//clr_ram_addr
	write_dw( (board<<18) + (0xa016<<2), 0x0);//clr_ram_addr

	write_dw( (board<<18) + (0xa012<<2), 0x00);//waddr
	write_dw( (board<<18) + (0xa013<<2), 0x80);//write length "PP Max=256B"


	write_dw( (board<<18) + (0xa011<<2), 0x03); //READ FLASH
	_delay_ms(2000);
// 	rd_fifo_data = 1;
// 	while ((rd_fifo_data&0x1) == 0x1)
// 	{
// 		write_dw( (board<<18) + (0xa011<<2), 0x05);//RDSR
// 		_delay_ms(10);
// 		rd_fifo_data =  read_dw((board<<18) +(0xa006<<2));//read flash datao
// 		
// 	}

	write_dw( (board<<18) + (0xa016<<2), 0x80000000);//clr_ram_addr
	write_dw( (board<<18) + (0xa016<<2), 0x20000000);//clr_ram_addr+read_ram selfcheck

//	checkdata = read_dw((board<<18) +(0xa006<<2));//read r ram data

//	write_dw( (board<<18) + (0xa016<<2), 0x00);//clr_ram_addr
//

	for (j=0; j<128; j++)
	{
		rdata[j] = 0;
	}

// 	rd_fifo_data =  read_dw((board<<18) +(0xa001<<2));//read flash datao
// 
// 	rd_fifo_data =  read_dw((board<<18) +(0xa001<<2));//read flash data
	for (j=0; j<128; j++)
	{
		rdata[j] = read_dw((board<<18) +(0xa001<<2));//read flash data
	}


	write_dw( (board<<18) + (0xa016<<2), 0x80000000);//clr_ram_addr
	write_dw( (board<<18) + (0xa016<<2), 0x00000000);//clr_ram_addr

		
		write_dw( (board<<18) + (0xa011<<2), 0x9F); //RDID
		_delay_ms(1);
		rd_fifo_data =  read_dw((board<<18) +(0xa006<<2));//read flash datao


		write_dw( (board<<18) + (0xa016<<2), 0x0);//clr_ram_addr
		write_dw( (board<<18) + (0xa011<<2), 0x06); //WDEN
		_delay_ms(1);
		write_dw( (board<<18) + (0xa011<<2), 0x01);//WRSR
		_delay_ms(1);
		write_dw( (board<<18) + (0xa011<<2), 0x05);//RDSR
		_delay_ms(1);
		rd_fifo_data =  read_dw((board<<18) +(0xa006<<2));//read flash datao
	
}

void CTestDCM100Dlg::OnFPGADebug() 
{
	
	m_adresult2 = "check...";
	UpdateData(FALSE);
	UpdateWindow();
	ULONG ChannelUP = 0;
	if (board_slot_addr==0)
	{
		ChannelUP = read_dw(0xfc0000 + 3*4);
		for (int i=0; i<32; i++)
		{
			if((ChannelUP >> i) & 1  == 1)
			{
				board_slot_addr = i+1;
			}
		}
	}

	
	// TODO: Add your control notification handler code here
	BOOL selfcheckres = false;

// 	dcm100_binding(0, board_slot_addr, 0, 0);
	for (int i = 0; i<4; i++)
	{
		dcm100_powerup_set_initdata(board_slot_addr,i);
	}
	char filename[] = "..\\dcm_selfcheck_rpt.txt";
	int res[64] = {0};
	
	selfcheckres = dcm100_selfcheck(filename, 0, res);

	
	if (selfcheckres)
	{
		m_adresult2 = "自检 Pass!";
		
	}
	else
	{
		m_adresult2 = "自检 Fail!";
	}
	UpdateData(FALSE);
	UpdateWindow();
	
}

void CTestDCM100Dlg::INER_EXT_RAM_CHECK() 
{
	// TODO: Add your control notification handler code here
	char filename[] = "..\\rpt.txt";
	
     
	dcm100_switch_ram_selfcheck(filename,board_slot_addr,0);
	
}

void CTestDCM100Dlg::OnI2CWrite() 
{
	LARGE_INTEGER tick;
	LARGE_INTEGER time1;
	LARGE_INTEGER time2;
	QueryPerformanceFrequency(&tick);
	// TODO: Add your control notification handler code here
	
//	dcm100_i2cset(2500,DCM100_NORMAL_I2C,0,1,-1,-1,-1,-1,-1,-1);
//	dcm100_i2cpinlevel(3 , 0, 2.4, 0.8);
//	dcm100_i2creaddata(2,6,2);

// 	dcm100_i2cwritedata(2,7,"55aa");
// 	dcm100_i2cwritedata(2,7,"aa55");
// 
 	QueryPerformanceCounter(&time1);
 	for (int i=0; i<100; i++)
	{
		//I2CWriteData(7,6,"0f5a");
		//I2CWriteData(7,6,"ada5");
//		dcm100_i2creaddata(2,6,2);
	}
 	QueryPerformanceCounter(&time2);
	double us = (time2.QuadPart - time1.QuadPart)*1e6/tick.QuadPart;
	
}

void CTestDCM100Dlg::OnDoubleclickedButton5() 
{
	// TODO: Add your control notification handler code here
	
}
