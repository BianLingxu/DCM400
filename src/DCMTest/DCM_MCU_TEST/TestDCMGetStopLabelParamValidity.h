#pragma once
/*!
* @file      TestDCMGetStopLabelParamValidity.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/10/09
* @version   v 1.0.0.0
* @brief     测试0GetStopLabel参数有效性
* @comment
*/
#include "..\DCMTestMain.h"
XT_TEST(ParamValidityTest, TestDCMGetStopLabelParamValidity)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, PARAM_VADILITY);
	CErrorMSG errMsg(strFuncName.c_str(), "ParamValidityTest");//Error message.
	char* lpszRetVal = nullptr;
	const int nSiteTestCount = 4;
	USHORT uTestSite[nSiteTestCount] = { 0, 1, SITE_NUM, SITE_NUM + 1 };


	BOOL bValidSite[MAX_SITE] = { 0 };


	lpszRetVal = dcm.GetStopLabel(0);
	if (0 != strlen(lpszRetVal))
	{
		//Not warning when vector is not loaded.
		XT_EXPECT_EQ(lpszRetVal, "");

		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("No Warning when vector is not loaded!");
	}
	map<BYTE, USHORT> mapSlot;
	int nRetVal = GetBoardInfo(mapSlot, g_lpszVectorFilePath);
	if (0 == mapSlot.size())
	{
		//No board is inserted.
		XT_EXPECT_TRUE(FALSE);
		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("No board inserted!");
		errMsg.Print(this, g_lpszReportFilePath);
		return;
	}

	XT_EXPECT_EQ(nRetVal, 0);
	if (0 != nRetVal)
	{
		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("Load vector(%s) fail.", g_lpszVectorFilePath);
		mapSlot.clear();
		errMsg.Print(this, g_lpszReportFilePath);
		return;
	}

	dcm.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");
	dcm.RunVectorWithGroup("G_ALLPIN", "FIRST_ST", "FIRST_SP");

	USHORT uVectorSiteCount = dcm_GetVectorSiteCount();
	USHORT uSiteCount = uVectorSiteCount + 1;
	BOOL bFail = FALSE;
	for (USHORT usSiteNo = 0; usSiteNo < uSiteCount; ++usSiteNo)
	{
		lpszRetVal = dcm.GetStopLabel(usSiteNo);
		bFail = FALSE;
		for (auto& Slot : mapSlot)
		{
			if (usSiteNo >= Slot.second && usSiteNo < Slot.second + DCM_MAX_CONTROLLERS_PRE_BOARD)
			{
				if (0 == strlen(lpszRetVal) || 0 != strcmp(lpszRetVal, "FIRST_SP"))
				{
					XT_EXPECT_TRUE(FALSE);
					errMsg.AddNewError(STRING_ERROR_MSG);
					errMsg.SaveErrorMsg("The board is inserted, but return (%s).", lpszRetVal);
				}
				bFail = TRUE;
				break;
			}
		}
		if (bFail)
		{
			continue;
		}

		if (usSiteNo >= uVectorSiteCount && 0 != strlen(lpszRetVal))
		{
			errMsg.AddNewError(STRING_ERROR_MSG);
			char cDigit[10] = { 0 };
			_itoa_s(usSiteNo, cDigit, 10, 10);
			errMsg.SetErrorItem("usSiteNo", cDigit);
			errMsg.SaveErrorMsg("The site number is over scale, the return value(%s) is not blank", lpszRetVal);
			continue;
		}
		XT_EXPECT_EQ(strlen(lpszRetVal), (size_t)0);
		if (0 != strlen(lpszRetVal))
		{
			errMsg.AddNewError(STRING_ERROR_MSG);
			char cDigit[10] = { 0 };
			_itoa_s(usSiteNo, cDigit, 10, 10);
			errMsg.SaveErrorMsg("The board is not inserted, but return(%s) value is not balnk.", lpszRetVal);
		}
	}

	errMsg.Print(this, g_lpszReportFilePath);


	dcm_CloseFile();//Unload the vector file.
}