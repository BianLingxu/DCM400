#pragma once
/*!
* @file      TestDCMGetStopLabelFunction.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/12/12
* @version   v 1.0.0.0
* @brief     测试GetStopLabel功能
* @comment
*/
#include "..\DCMTestMain.h"
void CheckStopLabel(CFuncReport& funcReport, XTest* xtTest, const char* cStartLabel, const char* cStopLabel, const map<BYTE, USHORT>& mapSlot)
{
	dcm.RunVectorWithGroup("G_ALLPIN", cStartLabel, cStopLabel);
	char* lpszStopLabel = nullptr;
	auto iterSlot = mapSlot.begin();
	while (mapSlot.end() != iterSlot)
	{
		for (int nSiteIndex = 0; nSiteIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++nSiteIndex)
		{
			lpszStopLabel = dcm.GetStopLabel(iterSlot->second + nSiteIndex);
			if (nullptr == lpszStopLabel)
			{
				xtTest->Expect("", cStopLabel, xtTest->EQ, __FILE__, __LINE__);
			}
			else
			{
				xtTest->Expect(lpszStopLabel, cStopLabel, xtTest->EQ, __FILE__, __LINE__);
			}
			if (nullptr == lpszStopLabel || 0 != strcmp(lpszStopLabel, cStopLabel))
			{
				for (int nPinIndex = 0; nPinIndex < DCM_CHANNELS_PER_CONTROL; ++nPinIndex)
				{
					funcReport.SaveFailChannel(iterSlot->first, nSiteIndex * DCM_CHANNELS_PER_CONTROL + nPinIndex);
				}
			}
		}
		++iterSlot;
	}
}

XT_TEST(FunctionFunctionTest, TestDCMGetStopLabelFunction)
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


	const ULONG ulBRAMCaptureData[2] = { 0x4408, 0x2204 };
	const ULONG ulDRAMCaptureData[2] = { 0x8880, 0x4440 };

	funcReport.AddTestItem("Get the Stop Label of BRAM Vector");
	CheckStopLabel(funcReport, this, "TEST_BRAM_ST", "TEST_BRAM_SP", mapSlot);

	funcReport.AddTestItem("Get the Stop Label of DRAM Vector");
	CheckStopLabel(funcReport, this, "TEST_DRAM_ST", "TEST_DRAM_SP", mapSlot);

	funcReport.Print(this, g_lpszReportFilePath);

	funcReport.AddTestItem("Check Invalid Site");
	const char* lpszPinName = "CH2";
	USHORT usInvalidSite = mapSlot.begin()->second;
	InvalidSite(usInvalidSite);
	const char* lpszStopLabel = dcm.GetStopLabel(usInvalidSite);

	XT_EXPECT_EQ(lpszStopLabel, "");
	if (0 != strcmp(lpszStopLabel, ""))
	{
		funcReport.SaveAddtionMsg("The return value(%s) of the invalid site is not equal to blank string.", lpszStopLabel);
	}

	RestoreSite();


	dcm.Disconnect("G_ALLPIN");
	dcm_CloseFile();
}