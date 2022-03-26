#pragma once
/*!
* @file      TestDCMGetStopLineNoFunction.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/12/12
* @version   v 1.0.0.0
* @brief     测试GetStopLineNo功能
* @comment
*/
#include "..\DCMTestMain.h"
void CheckStopLineNo(CFuncReport& funcReport, XTest* xtTest, const char* lpszStartLabel, const char* lpszStopLabel, const map<BYTE, USHORT>& mapSlot)
{
	ULONG ulExpectStopLineNo = dcm_GetLabelLineNo(lpszStopLabel, FALSE);

	dcm.RunVectorWithGroup("G_ALLPIN", lpszStartLabel, lpszStopLabel);
	ULONG ulRealStopLineNo = 0;
	auto iterSlot = mapSlot.begin();
	while (mapSlot.end() != iterSlot)
	{
		for (USHORT uSiteID = 0; uSiteID < DCM_MAX_CONTROLLERS_PRE_BOARD; ++uSiteID)
		{
			dcm.GetStopLineNo(iterSlot->second + uSiteID, ulRealStopLineNo);
			xtTest->Expect(ulRealStopLineNo, ulExpectStopLineNo, xtTest->EQ, __FILE__, __LINE__);

			if (ulRealStopLineNo != ulExpectStopLineNo)
			{
				for (int nPinIndex = 0; nPinIndex < DCM_CHANNELS_PER_CONTROL; ++nPinIndex)
				{
					funcReport.SaveFailChannel(iterSlot->first, uSiteID * DCM_CHANNELS_PER_CONTROL + nPinIndex);
				}
			}
		}
		++iterSlot;
	}
}

XT_TEST(FunctionFunctionTest, TestDCMGetStopLineNoFunction)
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

	XT_EXPECT_EQ(nRetVal, 0);
	if (0 != nRetVal)
	{
		funcReport.AddTestItem("Load vector");
		funcReport.SaveAddtionMsg("Load vector(%s) fail.", g_lpszVectorFilePath);
		iterSlot = mapSlot.begin();
		while (mapSlot.end() != iterSlot)
		{
			for (USHORT usChannel = 0; usChannel < DCM_MAX_CHANNELS_PER_BOARD; ++usChannel)
			{
				funcReport.SaveFailChannel(iterSlot->first, usChannel);
			}
			++iterSlot;
		}
		mapSlot.clear();
		funcReport.Print(this, g_lpszReportFilePath);
		return;
	}

	dcm.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");
	dcm.Connect("G_ALLPIN");
	dcm.SetPinLevel("G_ALLPIN", 3, 0, 1.5, 0.8);

	iterSlot = mapSlot.begin();

	const ULONG ulBRAMCaptureData[2] = { 0x4408, 0x2204 };
	const ULONG ulDRAMCaptureData[2] = { 0x8880, 0x4440 };

	funcReport.AddTestItem("Get the Stop line No. of BRAM Vector");
	CheckStopLineNo(funcReport, this, "TEST_BRAM_ST", "TEST_BRAM_SP", mapSlot);

	funcReport.AddTestItem("Get the Stop line No. of DRAM Vector");
	CheckStopLineNo(funcReport, this, "TEST_DRAM_ST", "TEST_DRAM_SP", mapSlot);

	funcReport.AddTestItem("Check Invalid Site");
	const char* lpszPinName = "CH2";
	USHORT usInvalidSite = mapSlot.begin()->second;
	InvalidSite(usInvalidSite);
	ULONG ulStopLineNo = 0;
	nRetVal = dcm.GetStopLineNo(usInvalidSite, ulStopLineNo);

	XT_EXPECT_EQ(nRetVal, SITE_INVALID);
	if (SITE_INVALID != nRetVal)
	{
		funcReport.SaveAddtionMsg("The return value(%d) of the invalid site is not equal to SITE_INVALID(%d).", nRetVal, SITE_INVALID);
	}
	XT_EXPECT_EQ(ulStopLineNo, (ULONG)-1);

	if ((ULONG)-1 != ulStopLineNo)
	{
		funcReport.SaveAddtionMsg("The stop line number(0x%08X) of the invalid site is not equal to 0x%08X.", ulStopLineNo, (ULONG)-1);
	}
	RestoreSite();


	funcReport.Print(this, g_lpszReportFilePath);


	dcm.Disconnect("G_ALLPIN");
	dcm_CloseFile();
}