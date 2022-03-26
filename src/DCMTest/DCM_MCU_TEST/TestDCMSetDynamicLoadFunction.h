#pragma once
/**
 * @file TestDCMSetDynamicLoadFunction.h
 * @brief Check the function of SetDynamicLoad
 * @author Guangyun Wang
 * @date 2021/06/07
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controller Technology Co., Ltd.
*/

#include "..\DCMTestMain.h"
XT_TEST(FunctionFunctionTest, TestDCMSetDynamicLoadFunction)
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

	nRetVal = dcm.LoadVectorFile(g_lpszVectorFilePath, FALSE);
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

	//Defined pin group G_ALLPIN
	dcm.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");

	

	double dIOH = 1e-3;
	double dIOL = 1e-3;

	USHORT usDynamicPin = 0;
	char lpszDynamicPin[32] = { 0 };
	sprintf_s(lpszDynamicPin, sizeof(lpszDynamicPin), "CH%d", usDynamicPin);
	char lpszPMUPin[32] = { 0 };
	sprintf_s(lpszPMUPin, sizeof(lpszPMUPin), "CH%d", usDynamicPin + 2);

	USHORT usSiteNo = mapSlot.begin()->second;
	CHANNEL_INFO Channel;
	Channel.m_bySlotNo = mapSlot.begin()->first;
	Channel.m_usChannel = 0;
	auto CheckCurrent = [&](BOOL bIOL)
	{
		double dExpected = bIOL ? -dIOL : dIOH;
		dcm.PPMUMeasure(lpszPMUPin, 10, 10);
		double dReal = 0;

		dReal = dcm.GetPPMUMeasResult(lpszPMUPin, usSiteNo);
		XT_EXPECT_REAL_EQ(dReal, dExpected, 1e-4);
		if (1e-4 < fabs(dReal - dExpected))
		{
			funcReport.SaveFailChannel(Channel.m_bySlotNo, Channel.m_usChannel);
		}
	};
	dcm.Connect("G_ALLPIN");
	///<Check IOL
	funcReport.AddTestItem("Check IOL");
	dcm.SetPPMU(lpszPMUPin, DCM_PPMU_FVMI, 0, DCM_PPMUIRANGE_2MA);
	dcm.SetDynamicLoad(lpszDynamicPin, usSiteNo, DCM_OPEN_CLAMP_OPEN_LOAD, dIOH, dIOL, 3);
	dcm.SetChannelStatus(lpszDynamicPin, usSiteNo, DCM_HIGH_IMPEDANCE);
	CheckCurrent(TRUE);
	dcm.SetDynamicLoad(lpszDynamicPin, usSiteNo, DCM_CLOSE_CLAMP_CLOSE_LOAD, dIOH, dIOL, 3);

	///<Check IOH
	funcReport.AddTestItem("Check IOH");
	dcm.SetPPMU(lpszPMUPin, DCM_PPMU_FVMI, 3.5, DCM_PPMUIRANGE_2MA);
	dcm.SetDynamicLoad(lpszDynamicPin, usSiteNo, DCM_OPEN_CLAMP_OPEN_LOAD, dIOH, dIOL, 3);
	dcm.SetChannelStatus(lpszDynamicPin, usSiteNo, DCM_HIGH_IMPEDANCE);
	CheckCurrent(FALSE);
	dcm.SetDynamicLoad(lpszDynamicPin, usSiteNo, DCM_CLOSE_CLAMP_CLOSE_LOAD, dIOH, dIOL, 3);

	dcm.Disconnect("G_ALLPIN");
	dcm_CloseFile();
	funcReport.Print(this, g_lpszReportFilePath);
}