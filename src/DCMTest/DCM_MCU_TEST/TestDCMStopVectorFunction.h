/*!
* @file      TestDCMStopVectorFunction.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/10/31
* @version   v 1.0.0.0
* @brief     测试StopVector功能
* @comment
*/
#include "..\DCMTestMain.h"

XT_TEST(FunctionFunctionTest, TestDCMStopVectorFunction)
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

	dcm.Connect("G_ALLPIN");
	dcm.SetPinLevel("G_ALLPIN", 3, 0, 1.5, 0.8);
	dcm.SetEdge("G_ALLPIN", "0", DCM_DTFT_NRZ, 10, 110, 10, 65);

	funcReport.AddTestItem("Stop endless vector");
	funcReport.AddClkSetting(10, 110, 10, 110, 65, 110);

	dcm.RunVectorWithGroup("G_ALLPIN", "TEST_ENDLESS_ST", "TEST_ENDLESS_SP", FALSE);

	dcm.StopVector("G_ALLPIN");

	char lpszPinName[16] = { 0 };//The pin name.
	for (auto& Slot : mapSlot)
	{
		for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL; ++usChannel)
		{
			sprintf_s(lpszPinName, 5, "CH%d", usChannel);
			for (USHORT usSiteNo = 0; usSiteNo < DCM_MAX_CONTROLLERS_PRE_BOARD; ++usSiteNo)
			{
				nRetVal = dcm.GetMCUPinRunStatus(lpszPinName, Slot.second + usSiteNo);
				XT_EXPECT_EQ(nRetVal, 1);
				if (1 != nRetVal)
				{
					//The vector is not stop.
					funcReport.SaveFailChannel(Slot.first, usSiteNo * DCM_CHANNELS_PER_CONTROL + usChannel);
				}
			}
		}
	}
	funcReport.AddTestItem("Check Invalid Site");
	USHORT usInvalidSite = mapSlot.begin()->second;
	dcm.RunVectorWithGroup("G_ALLPIN", "TEST_ENDLESS_ST", "TEST_ENDLESS_SP", FALSE);
	InvalidSite(usInvalidSite);
	nRetVal = dcm.StopVector("CH2");
	RestoreSite();
	USHORT usSiteNo = 0;
	int nTargetRunStatus = 0;
	for (auto& Slot : mapSlot)
	{
		usSiteNo = Slot.second;
		for (BYTE byController = 0; byController < DCM_MAX_CONTROLLERS_PRE_BOARD; ++byController, ++usSiteNo)
		{
			if (usInvalidSite == usSiteNo)
			{
				nTargetRunStatus = 0;
			}
			else
			{
				nTargetRunStatus = 1;
			}
			sprintf_s(lpszPinName, sizeof(lpszPinName), "CH%d", 2);
			int nRunStatus = dcm.GetMCUPinRunStatus(lpszPinName, usSiteNo);
			XT_EXPECT_EQ(nRunStatus, nTargetRunStatus);
			if (nRunStatus != nTargetRunStatus)
			{
				for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL;++usChannel)
				{
					funcReport.SaveFailChannel(Slot.first, usChannel);
				}
			}
		}
	}


	dcm.StopVector("G_ALLPIN");

	mapSlot.clear();

	funcReport.Print(this, g_lpszReportFilePath);

	dcm.Disconnect("G_ALLPIN");
	dcm_CloseFile();
}