#pragma once
/*!
* @file      TestDCMGetFirstFailLineNoFunction.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/10/30
* @version   v 1.0.0.0
* @brief     测试GetFirstFailLineNo功能
* @comment
*/

#include "..\DCMTestMain.h"
XT_TEST(FunctionFunctionTest, TestDCMGetFirstFailLineNoFunction)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, FUNCTION);
	CFuncReport funcReport(strFuncName.c_str(), "FunctionFunctionTest");
	const int nMutualTestGroupChannelCount = 4;//The count of channel in each mutual test group.
	const ULONG ulExpectFailLineNo[nMutualTestGroupChannelCount][2] = { { 2, 7 }, { 3, 8 }, { 5, 7 }, { 4, 6 } };

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

	//Load vector.
	XT_EXPECT_EQ(nRetVal, 0);
	if (0 != nRetVal)
	{
		funcReport.AddTestItem("Load vector");
		funcReport.SaveAddtionMsg("Load vector(%s) fail.", g_lpszVectorFilePath);
		for (auto& Slot : mapSlot)
		{
			for (USHORT usChannel = 0; usChannel < DCM_MAX_CHANNELS_PER_BOARD; ++usChannel)
			{
				funcReport.SaveFailChannel(Slot.first, usChannel);
			}
		}
		mapSlot.clear();
		funcReport.Print(this, g_lpszReportFilePath);
		return;
	}
	dcm.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");

	funcReport.AddTestItem("Pin Mutual Test");

	double dSTBR = 10;//The time of STBR
	const double dAddStep = 5;
	char lpszPinName[16] = { 0 };//The pin name.
	ULONG ulFirstFailLine = 0;

	dcm.Connect("G_ALLPIN");
	dcm.SetPinLevel("G_ALLPIN", 3, 0, 1.5, 0.8);

	for (dSTBR = 30; dSTBR < 101; dSTBR += dAddStep)
	{
		dcm.SetEdge("G_ALLPIN", "0", DCM_DTFT_NRZ, 10, 110, 10, dSTBR);

		funcReport.AddClkSetting(10, 110, 10, 110, dSTBR, 110);

		dcm.RunVectorWithGroup("G_ALLPIN", "TEST_FAIL_ST", "TEST_FAIL_SP");
		//dcm.SaveFailMap(0);

		for (auto& Slot : mapSlot)
		{
			for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL; ++usChannel)
			{
				sprintf_s(lpszPinName, 5, "CH%d", usChannel);
				for (USHORT usSiteIndex = 0; usSiteIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++usSiteIndex)
				{
					nRetVal = dcm.GetFirstFailLineNo(lpszPinName, Slot.second + usSiteIndex, ulFirstFailLine);
					XT_EXPECT_EQ(nRetVal, 0);
					XT_EXPECT_EQ(ulFirstFailLine, ulExpectFailLineNo[usChannel % 4][0]);
					if (EXECUTE_SUCCESS != nRetVal || ulFirstFailLine != ulExpectFailLineNo[usChannel % 4][0])
					{
						funcReport.SaveFailChannel(Slot.first, usSiteIndex * DCM_CHANNELS_PER_CONTROL + usChannel);
					}
				}
			}
		}
	}
	
	//dcm.SaveFailMap(0);


	funcReport.AddTestItem("Check Invalid Site");
	USHORT usInvalidSite = mapSlot.begin()->second;
	InvalidSite(usInvalidSite);

	nRetVal = dcm.GetFirstFailLineNo("CH2", usInvalidSite, ulFirstFailLine);

	XT_EXPECT_EQ(nRetVal, SITE_INVALID);
	if (SITE_INVALID != nRetVal)
	{
		funcReport.SaveAddtionMsg("The return value(%d) of the invalid site is not equal to SITE_INVALID(%d).", nRetVal, SITE_INVALID);
	}
	XT_EXPECT_EQ(ulFirstFailLine, (ULONG)-1);
	if ((ULONG)-1 != ulFirstFailLine)
	{
		funcReport.SaveAddtionMsg("The first fail line number(0x%08X) of the invalid site is not equal to 0x%08X.", ulFirstFailLine, (ULONG)-1);
	}
	RestoreSite();

	mapSlot.clear();

	funcReport.Print(this, g_lpszReportFilePath);

	dcm.Disconnect("G_ALLPIN");
	dcm_CloseFile();
}