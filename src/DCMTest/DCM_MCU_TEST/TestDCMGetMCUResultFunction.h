#pragma once
/*!
* @file      TestDCMGetMCUResultFunction.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/11/20
* @version   v 1.0.0.0
* @brief     测试GetMCUResult功能
* @comment
*/
#include "..\DCMTestMain.h"
XT_TEST(FunctionFunctionTest, TestDCMGetMCUResultFunction)
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

	//Load vector.
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
	const char* lpszLabel[4] = { "ALL_PASS_ST","ALL_PASS_SP","TEST_FAIL_ST","TEST_FAIL_SP" };

	double dSTBR = 10;//The time of STBR
	ULONG ulFailCount = 0;

	ULONG ulFailLineNo[100] = { 0 };
	nRetVal = 0;
	BOOL bTestPass = FALSE;
	dcm.Connect("G_ALLPIN");
	dcm.SetPinLevel("G_ALLPIN", 3, 0, 1.5, 0.8);
	BOOL bFail[DCM_CHANNELS_PER_CONTROL] = { 0 };
	memset(bFail, 0, sizeof(bFail));
	BOOL bAllFail = TRUE;

	dcm.SetEdge("G_ALLPIN", "0", DCM_DTFT_NRZ, 10, 110, 10, 60);

	funcReport.AddTestItem("ALL PASS vector");
	dcm.RunVectorWithGroup("G_ALLPIN", lpszLabel[0], lpszLabel[1]);
	//dcm.SaveFailMap(0);
	iterSlot = mapSlot.begin();
	while (mapSlot.end() != iterSlot)
	{
		for (USHORT usSiteIndex = 0; usSiteIndex < 4;++usSiteIndex)
		{
			nRetVal = dcm.GetMCUResult(iterSlot->second + usSiteIndex);
			XT_EXPECT_EQ(nRetVal, 0);
			if (0 != nRetVal)
			{
				for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL;++usChannel)
				{
					funcReport.SaveFailChannel(iterSlot->first, usSiteIndex * DCM_CHANNELS_PER_CONTROL + usChannel);
				}
			}
		}

		++iterSlot;
	}

//	dcm.SaveFailMap(0);

	funcReport.AddTestItem("FAIL vector");
	dcm.RunVectorWithGroup("G_ALLPIN", lpszLabel[2], lpszLabel[3]);


	iterSlot = mapSlot.begin();
	while (mapSlot.end() != iterSlot)
	{
		for (USHORT usSiteIndex = 0; usSiteIndex < 4; ++usSiteIndex)
		{
			nRetVal = dcm.GetMCUResult(iterSlot->second + usSiteIndex);
			XT_EXPECT_EQ(nRetVal, 1);
			if (1 != nRetVal)
			{
				for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL; ++usChannel)
				{
					funcReport.SaveFailChannel(iterSlot->first, usSiteIndex * DCM_CHANNELS_PER_CONTROL + usChannel);
				}
			}
		}

		++iterSlot;
	}
//	dcm.SaveFailMap(0);

	funcReport.AddTestItem("Check Invalid Site");
	const char* lpszPinName = "CH2";
	USHORT usInvalidSite = mapSlot.begin()->second;
	InvalidSite(usInvalidSite);
	nRetVal = dcm.GetMCUResult(usInvalidSite);

	XT_EXPECT_EQ(nRetVal, SITE_INVALID);
	if (SITE_INVALID != nRetVal)
	{
		funcReport.SaveAddtionMsg("The return value(%d) of the invalid site is not equal to SITE_INVALID(%d).", nRetVal, SITE_INVALID);
	}
	RestoreSite();

	funcReport.AddTestItem("Check Latest run result");
	///<First the all channel is fail
	char lpszCurPin[32] = { 0 };
	const USHORT usNotRunPin = 2;
	const USHORT usExceptPin = 0;
	sprintf_s(lpszCurPin, sizeof(lpszCurPin), "CH%d", usNotRunPin);

	dcm.WriteWaveData(lpszCurPin, "ALL_PASS_ST", DCM_ALLSITE, 2, 8, 0xFF);

	dcm.RunVectorWithGroup("G_ALLPIN", lpszLabel[0], lpszLabel[1]);

	string strPinList;
	for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL;++usChannel)
	{
		if (usNotRunPin == usChannel || usExceptPin == usChannel)
		{
			continue;
		}
		sprintf_s(lpszCurPin, sizeof(lpszCurPin), "CH%d,", usChannel);
		strPinList += lpszCurPin;
	}
	dcm.SetPinGroup("G_RUN", strPinList.c_str());
	dcm.RunVectorWithGroup("G_RUN", "ALL_PASS_ST", "ALL_PASS_SP");

	for (auto& Slot : mapSlot)
	{
		USHORT usSiteNo = Slot.second;
		for (BYTE byControllerIndex = 0; byControllerIndex < DCM_MAX_CONTROLLERS_PRE_BOARD;++byControllerIndex,++usSiteNo)
		{
			nRetVal = dcm.GetMCUResult(usSiteNo);
			XT_EXPECT_EQ(nRetVal, 0);
			if (0 != nRetVal)
			{
				funcReport.SaveAddtionMsg("The MCU result should be pass(Not run Pin %d and pin %d), but the result is fail", usNotRunPin, usExceptPin);
				USHORT usChannel = byControllerIndex * DCM_CHANNELS_PER_CONTROL;
				funcReport.SaveFailChannel(Slot.first, usChannel + usNotRunPin);
				funcReport.SaveFailChannel(Slot.first, usChannel + usExceptPin);
			}
		}
	}

	sprintf_s(lpszCurPin, sizeof(lpszCurPin), "CH%d", usNotRunPin);
	dcm.WriteWaveData(lpszCurPin, "ALL_PASS_ST", DCM_ALLSITE, 2, 8, 0x69);


	funcReport.Print(this, g_lpszReportFilePath);

	dcm.Disconnect("G_ALLPIN");
	dcm_CloseFile();
}