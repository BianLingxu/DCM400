#pragma once
/*!
* @file      TestDCMSetPeriodFunction.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/11/14
* @version   v 1.0.0.0
* @brief     测试SetPeriod功能
* @comment
*/
#include "..\DCMTestMain.h"
XT_TEST(FunctionFunctionTest, TestDCMSetPeriodFunction)
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

	
	dcm.Connect("G_ALLPIN");
	dcm.SetPinLevel("G_ALLPIN", 3, 0, 1.5, 0.8);

	LARGE_INTEGER timeStart, timeStop, timeFreq;
	double dTimeConsum[2] = { 0 };
	QueryPerformanceFrequency(&timeFreq);
		
	iterSlot = mapSlot.begin();
	double dPeriod = 0 ;
	const char* lpszVectorLabel[2] = { "TEST_FAIL_ST", "TEST_FAIL_SP" };
	dPeriod = dcm_GetTimeSetPeriod(iterSlot->first, 0, 0);
	ULONG ulVectorLineCount = 0;
	dcm_GetVectorLineCount(lpszVectorLabel[0], lpszVectorLabel[1], ulVectorLineCount);

	QueryPerformanceCounter(&timeStart);
	dcm.RunVectorWithGroup("G_ALLPIN", lpszVectorLabel[0], lpszVectorLabel[1]);
	QueryPerformanceCounter(&timeStop);
	dTimeConsum[0] = (double)(timeStop.QuadPart - timeStart.QuadPart) / timeFreq.QuadPart*1e6;

			
	funcReport.AddTestItem("Check Invalid Site");
	const USHORT usInvalidSite = mapSlot.begin()->second;
	const double dInvalidSitePeriod = 200;
	const double dValidSitePeriod = 300;
	double dBackupPeriod = dcm_GetTimeSetPeriod(mapSlot.begin()->first, 0, 0);

	dcm.SetPeriod("0", dInvalidSitePeriod);

	InvalidSite(usInvalidSite);
	dcm.SetPeriod("0", dValidSitePeriod);

	BYTE bySlotNo = 0;
	USHORT usChannel = 0;

	double dTargetPeriod = 0;
	for (auto& Slot : mapSlot)
	{
		USHORT usBaseSiteNo = Slot.second;
		usChannel = 0;
		for (BYTE byControllerIndex = 0; byControllerIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++byControllerIndex)
		{
			USHORT usSiteNo = usBaseSiteNo + byControllerIndex;
			if (usInvalidSite == usSiteNo)
			{
				dTargetPeriod = dInvalidSitePeriod;
			}
			else
			{
				dTargetPeriod = dValidSitePeriod;
			}
			double dCurPeriod = dcm_GetTimeSetPeriod(Slot.first, byControllerIndex, 0);
			XT_EXPECT_REAL_EQ(dCurPeriod, dTargetPeriod, 0.1);
			if (0.1 < fabs(dTargetPeriod - dCurPeriod))
			{
				for (USHORT usChanelIndex = 0; usChanelIndex < DCM_CHANNELS_PER_CONTROL; ++usChanelIndex, ++usChannel)
				{
					funcReport.SaveFailChannel(bySlotNo, usChannel);
				}
			}
		}
	}
	RestoreSite();
	dcm.SetPeriod("0", dBackupPeriod);

	funcReport.Print(this, g_lpszReportFilePath);

	dcm.Disconnect("G_ALLPIN");
	dcm_CloseFile();
}