// TestDCM50Dlg.h : header file
//

#if !defined(AFX_TESTDCM50DLG_H__9490D97F_E0D8_4D84_B9E6_1F557FCB022E__INCLUDED_)
#define AFX_TESTDCM50DLG_H__9490D97F_E0D8_4D84_B9E6_1F557FCB022E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CTestDCM100Dlg dialog

class CTestDCM100Dlg : public CDialog
{
// Construction
public:
	CTestDCM100Dlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CTestDCM100Dlg)
	enum { IDD = IDD_TESTDCM100_DIALOG };
	CString	m_bindingedit;
	BOOL	m_cal_relay;
	int		m_sel_site;
	BOOL	m_pogo_relay;
	BOOL	m_sl_relay;
	BOOL	m_fl_relay;
	int		m_relay_saturn1;
	int		m_sel_adcsite;
	int		m_adc_os;
	int		m_site1_DVhl_VRange;
	double	m_site1_DVH;
	double	m_site1_DVL;
	int		m_site2_DVhl_VRange;
	double	m_site2_DVH;
	double	m_site2_DVL;
	int		m_site1_VC_Range;
	int		m_site2_VC_Range;
	CString	m_adresult2;
	CString	m_adresult1;
	int		m_site1_ch_sel;
	int		m_site2_ch_sel;
	int		m_ext_site1_ch_sel;
	int		m_ext_site2_ch_sel;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTestDCM100Dlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CTestDCM100Dlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void On_Binding();
	afx_msg void Onbanding();
	afx_msg void Onbinding();
	afx_msg void OnCAL_EXT();
	afx_msg void On_sl_relay();
	afx_msg void On_fl_relay();
	afx_msg void On_pogo_relay();
	afx_msg void On_cal_relay();
	afx_msg void On_sel_saturn_relay();
	afx_msg void On_adc_os();
	afx_msg void On_FPGA_CLR();
	afx_msg void On_Saturn_Soft_Reset();
	afx_msg void On_Site1_drvHL_output();
	afx_msg void On_Site2_drvHL_output();
	afx_msg void On_Site1_ext_ForceSense();
	afx_msg void On_Site2_ext_ForceSense();
	afx_msg void On_Select_Site1_VCRange();
	afx_msg void On_Select_Site2_VCRange();
	afx_msg void On_Site1_ADC_Measure();
	afx_msg void On_Site2_ADC_Measure();
	afx_msg void On_loadvect();
	afx_msg void On_checkvector();
	afx_msg void OnFPGADebug();
	afx_msg void OnFpgaeDbg();
	afx_msg void INER_EXT_RAM_CHECK();
	afx_msg void OnI2CWrite();
	afx_msg void OnDoubleclickedButton5();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TESTDCM50DLG_H__9490D97F_E0D8_4D84_B9E6_1F557FCB022E__INCLUDED_)
