#pragma once
/*!
* @file      TestDCMGetFailCountFunction.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/10/30
* @version   v 1.0.0.0
* @brief     测试GetFailCount功能
* @comment
*/
#include "..\DCMTestMain.h"
XT_TEST(FunctionFunctionTest, TestDCMGetFailCountFunction)
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

	const int nFailSectionCount = 8;//The count of fail section.
	const int nFailCountPerSectionPerPin = 2;//The fail count of the channel per section.

	const BYTE byLoopFail = 2;
	const int nFailCountPerPin = nFailCountPerSectionPerPin * nFailSectionCount + byLoopFail;//The sum number of the fail line per channel.

	double dSTBR = 10;//The time of STBR
	const double dAddStep = 2.5;
	char lpszPinName[10] = { 0 };//The pin name.
	ULONG ulFailCount = 0;

	dcm.Connect("G_ALLPIN");
	dcm.SetPinLevel("G_ALLPIN", 3, 0, 1.5, 0.8);
	
	ULONG ulCtrlFailCount[DCM_MAX_CONTROLLERS_PRE_BOARD * DCM_MAX_BOARD_NUM] = { 0 };

	funcReport.AddTestItem("Pin Mutual Test");

	for (dSTBR = 30; dSTBR < 110; dSTBR += dAddStep)
	{
		dcm.SetEdge("G_ALLPIN", "0", DCM_DTFT_NRZ, 10, 110, 10, dSTBR);

		funcReport.AddClkSetting(10, 110, 10, 10, dSTBR, dSTBR);

		delay_ms(10);

		dcm.RunVectorWithGroup("G_ALLPIN", "TEST_FAIL_ST", "TEST_FAIL_SP");

		//dcm.SaveFailMap(0);
		iterSlot = mapSlot.begin();
		while (mapSlot.end() != iterSlot)
		{
			for (USHORT usChannelNo = 0; usChannelNo < DCM_CHANNELS_PER_CONTROL;++usChannelNo)
			{
				sts_sprintf(lpszPinName, 5, "CH%d", usChannelNo);
				for (USHORT usSiteIndex = 0; usSiteIndex < DCM_MAX_CONTROLLERS_PRE_BOARD;++usSiteIndex)
				{
					dcm.GetFailCount(lpszPinName, iterSlot->second + usSiteIndex, ulFailCount);
					XT_EXPECT_EQ((int)ulFailCount, (int)nFailCountPerPin);
					if (ulFailCount != nFailCountPerPin)
					{
						funcReport.SaveFailChannel(iterSlot->first, usSiteIndex * DCM_CHANNELS_PER_CONTROL + usChannelNo);
					}
				}
			}
			++iterSlot;
		}
	}

	funcReport.AddTestItem("Check Invalid Site");
	USHORT usInvalidSite = mapSlot.begin()->second;
	InvalidSite(usInvalidSite);
	nRetVal = dcm.GetFailCount("CH2", usInvalidSite, ulFailCount);

	XT_EXPECT_EQ(nRetVal, SITE_INVALID);
	if (SITE_INVALID != nRetVal)
	{
		funcReport.SaveAddtionMsg("The return value(%d) of the invalid site is not equal to SITE_INVALID(%d).", nRetVal, SITE_INVALID);
	}
	XT_EXPECT_EQ(ulFailCount, (ULONG)-1);
	if ((ULONG)-1 != ulFailCount)
	{
		funcReport.SaveAddtionMsg("The fail count(0x%08X) of the invalid site is not equal to 0x%08X.", ulFailCount, (ULONG)-1);
	}
	RestoreSite();


	mapSlot.clear();
	funcReport.Print(this, g_lpszReportFilePath);

	dcm.Disconnect("G_ALLPIN");
	dcm_CloseFile();
}