#pragma once
/*!
* @file      TestDCMGetMCUPinRunStatusFunction.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/10/31
* @version   v 1.0.0.0
* @brief     测试GetMCUPinRunStatus功能
* @comment
*/
#include "..\DCMTestMain.h"

XT_TEST(FunctionFunctionTest, TestDCMGetMCUPinRunStatusFunction)
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

	double dSTBR = 10;//The time of STBR
	const double dAddStep = 5;
	char lpszPinName[10] = { 0 };//The pin name.
	ULONG ulFailCount = 0;

	ULONG ulFailLineNo[100] = { 0 };
	BOOL bTestPass = FALSE;
	dcm.Connect("G_ALLPIN");
	dcm.SetPinLevel("G_ALLPIN", 3, 0, 1.5, 0.8);

	dcm.SetEdge("G_ALLPIN", "0", DCM_DTFT_NRZ, 10, 110, 10, 65);

	funcReport.AddTestItem("Endless vector test");

	dcm.RunVectorWithGroup("G_ALLPIN", "TEST_ENDLESS_ST", "TEST_ENDLESS_SP", FALSE);
	iterSlot = mapSlot.begin();
	while (mapSlot.end() != iterSlot)
	{
		for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL; ++usChannel)
		{
			sts_sprintf(lpszPinName, 5, "CH%d", usChannel);
			for (USHORT usSiteIndex = 0; usSiteIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++usSiteIndex)
			{
				nRetVal = dcm.GetMCUPinRunStatus(lpszPinName, iterSlot->second + usSiteIndex);
				XT_EXPECT_EQ(nRetVal, 0);
				if (0 != nRetVal)
				{
					//The vector is running, but the return value meaning vector is stop.
					funcReport.SaveFailChannel(iterSlot->first, usSiteIndex * DCM_CHANNELS_PER_CONTROL + usChannel);
				}
			}
		}
		++iterSlot;
	}
	
	dcm.StopVector("G_ALLPIN");

	funcReport.AddTestItem("Normal vector test");

	dcm.RunVectorWithGroup("G_ALLPIN", "TEST_FAIL_ST", "TEST_FAIL_SP");
	iterSlot = mapSlot.begin();
	while (mapSlot.end() != iterSlot)
	{
		for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL; ++usChannel)
		{
			sts_sprintf(lpszPinName, 5, "CH%d", usChannel);
			for (USHORT usSiteIndex = 0; usSiteIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++usSiteIndex)
			{
				nRetVal = dcm.GetMCUPinRunStatus(lpszPinName, iterSlot->second + usSiteIndex);
				XT_EXPECT_EQ(nRetVal, 1);
				if (1 != nRetVal)
				{
					//The vector is stop, but the return value meaning vector is running.
					funcReport.SaveFailChannel(iterSlot->first, usSiteIndex * DCM_CHANNELS_PER_CONTROL + usChannel);
				}
			}
		}
		++iterSlot;
	}

	funcReport.AddTestItem("Check Invalid Site");
	USHORT usInvalidSite = mapSlot.begin()->second;
	InvalidSite(usInvalidSite);
	nRetVal = dcm.GetMCUPinRunStatus("CH2", usInvalidSite);

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