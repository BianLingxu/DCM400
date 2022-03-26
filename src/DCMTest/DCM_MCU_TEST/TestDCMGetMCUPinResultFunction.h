#pragma once
/*!
* @file      TestDCMGetMCUPinResultFunction.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/10/31
* @version   v 1.0.0.0
* @brief     测试GetMCUPinResult功能
* @comment
*/
#include "..\DCMTestMain.h"

XT_TEST(FunctionFunctionTest, TestDCMGetMCUPinResultFunction)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, FUNCTION);
	CFuncReport funcReport(strFuncName.c_str(), "FunctionFunctionTest");

	map<BYTE, USHORT> mapSlot;
	int nRetVal = GetBoardInfo(mapSlot, g_lpszVectorFilePath);

	if (0 == mapSlot.size())
	{
		//No board is inserted.
		XT_EXPECT_TRUE(FALSE);
		funcReport.SetNoBoardValid();
		funcReport.Print(this, g_lpszReportFilePath);
		return;
	}

	SaveBoardSN(funcReport, mapSlot);
	auto iterSlot = mapSlot.begin();

	dcm.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");

	const char* lpszLabel[4] = {"ALL_PASS_ST","ALL_PASS_SP","TEST_FAIL_ST","TEST_FAIL_SP"};

	dcm.Connect("G_ALLPIN");
	dcm.SetPinLevel("G_ALLPIN", 3, 0, 1.5, 0.8);

	dcm.SetEdge("G_ALLPIN", "0", DCM_DTFT_NRZ, 10, 110, 10, 100);

	funcReport.AddTestItem("PASS vector");
	dcm.RunVectorWithGroup("G_ALLPIN", lpszLabel[0], lpszLabel[1]);

	char lpszPinName[32] = { 0 };
	while (mapSlot.end() != iterSlot)
	{
		for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL;++usChannel)
		{
			for (USHORT uSiteID = 0; uSiteID < DCM_MAX_CONTROLLERS_PRE_BOARD;++uSiteID)
			{
				sprintf_s(lpszPinName, sizeof(lpszPinName), "CH%d", usChannel);
				nRetVal = dcm.GetMCUPinResult(lpszPinName, iterSlot->second + uSiteID);
				XT_EXPECT_EQ(nRetVal, 0);
				if (0 != nRetVal)
				{
					funcReport.SaveFailChannel(iterSlot->first, uSiteID * DCM_CHANNELS_PER_CONTROL + usChannel);
				}
			}
		}
		++iterSlot;
	}


	funcReport.AddTestItem("FAIL vector");
	dcm.RunVectorWithGroup("G_ALLPIN", lpszLabel[2], lpszLabel[3]);
	while (mapSlot.end() != iterSlot)
	{
		for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL; ++usChannel)
		{
			for (USHORT uSiteID = 0; uSiteID < DCM_MAX_CONTROLLERS_PRE_BOARD; ++uSiteID)
			{
				sprintf_s(lpszPinName, sizeof(lpszPinName), "CH%d", usChannel);
				nRetVal = dcm.GetMCUPinResult(lpszPinName, iterSlot->second + uSiteID);
				XT_EXPECT_EQ(nRetVal, 0);
				if (1 != nRetVal)
				{
					funcReport.SaveFailChannel(iterSlot->first, uSiteID * DCM_CHANNELS_PER_CONTROL + usChannel);
				}
			}
		}
		++iterSlot;
	}

	funcReport.AddTestItem("Check Invalid Site");
	USHORT usInvalidSite = mapSlot.begin()->second;
	InvalidSite(usInvalidSite);
	nRetVal = dcm.GetMCUPinResult("CH2", usInvalidSite);

	XT_EXPECT_EQ(nRetVal, SITE_INVALID);
	if (SITE_INVALID != nRetVal)
	{
		funcReport.SaveAddtionMsg("The return value(%d) of the invalid site is not equal to SITE_INVALID(%d).", nRetVal, SITE_INVALID);
	}
	RestoreSite();


	mapSlot.clear();

	funcReport.Print(this, g_lpszReportFilePath);

	dcm.Disconnect("G_ALLPIN");
	dcm_CloseFile();
}