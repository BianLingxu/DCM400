// TestDCM50.h : main header file for the TESTDCM50 application
//

#if !defined(AFX_TESTDCM50_H__05B61020_3625_4068_A99D_05413432CA82__INCLUDED_)
#define AFX_TESTDCM50_H__05B61020_3625_4068_A99D_05413432CA82__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CTestDCM100App:
// See TestDCM50.cpp for the implementation of this class
//

class CTestDCM100App : public CWinApp
{
public:
	CTestDCM100App();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTestDCM100App)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CTestDCM100App)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TESTDCM50_H__05B61020_3625_4068_A99D_05413432CA82__INCLUDED_)
