#pragma once
/*!
* @file      TestDCMLoadVectorFileFunction.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/10/25
* @version   v 1.0.0.0
* @brief     测试Connect功能
* @comment
*/
#include "..\DCMTestMain.h"
XT_TEST(FunctionFunctionTest, TestDCMLoadVectorFileFunction)
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


	funcReport.AddTestItem("Check the sign of load vector file success");

	dcm.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");
	const int nCycleCount = 1;
	nRetVal = dcm.Connect("G_ALLPIN");
	if (VECTOR_FILE_NOT_LOADED == nRetVal)
	{
		XT_EXPECT_NE(VECTOR_FILE_NOT_LOADED, nRetVal);
		iterSlot = mapSlot.begin();
		while (mapSlot.end() != iterSlot)
		{
			for (USHORT usChannel = 0; usChannel < DCM_MAX_CHANNELS_PER_BOARD; ++usChannel)
			{
				funcReport.SaveFailChannel(iterSlot->first, usChannel);
			}
			++iterSlot;
		}
	}


	LARGE_INTEGER timeStart, timeStop, timeFreq;
	QueryPerformanceFrequency(&timeFreq);

	funcReport.AddTestItem("Twice load without load vector");

	QueryPerformanceCounter(&timeStart);
	dcm.LoadVectorFile(g_lpszVectorFilePath, FALSE);
	QueryPerformanceCounter(&timeStop);

	double dTime = (double)(timeStop.QuadPart - timeStart.QuadPart) / timeFreq.QuadPart * 1000;
	if (1000 < dTime)
	{
		XT_EXPECT_LESS(dTime, (double)500);
		iterSlot = mapSlot.begin();
		while (mapSlot.end() != iterSlot)
		{
			for (USHORT usChannel = 0; usChannel < DCM_MAX_CHANNELS_PER_BOARD; ++usChannel)
			{
				funcReport.SaveFailChannel(iterSlot->first, usChannel);
			}
			++iterSlot;
		}
	}


	dcm.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");
	dcm.SetPinLevel("G_ALLPIN", 3, 0, 1.5, 0.8);

	iterSlot = mapSlot.begin();
	double dPeriod = dcm_GetTimeSetPeriod(iterSlot->first, 0, 0);
	dcm.SetEdge("G_ALLPIN", "0", DCM_DTFT_NRZ, dPeriod / 8, dPeriod * 5 / 8, dPeriod / 8, dPeriod * 3 / 8);

	const ULONG ulExpectBaseFailLine[2] = { 2,3 };
	const int nFailStep = 100;
	const int nGetFailCount = 10;

	funcReport.AddTestItem("Mutual_test");

	dcm.RunVectorWithGroup("G_ALLPIN", "TEST_RUN_ST", "TEST_RUN_SP");

	char lpszPinName[16] = { 0 };
	ULONG ulFailLineNo[100] = { 0 };
	int nBaseFailLine = 0;
	ULONG ulCurFailLine = 0;

	iterSlot = mapSlot.begin();
	while (mapSlot.end() == iterSlot)
	{
		for (int nChannel = 0; nChannel < DCM_CHANNELS_PER_CONTROL; ++nChannel)
		{
			sts_sprintf(lpszPinName, 5, "CH%d", nChannel);
			for (int nSiteIndex = 0; nSiteIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++nSiteIndex)
			{
				nRetVal = dcm.GetFailLineNo(lpszPinName, iterSlot->second, nGetFailCount, ulFailLineNo);
				XT_EXPECT_NE(NO_FAIL_LINE, nRetVal);
				if (NO_FAIL_LINE == nRetVal)
				{
					funcReport.SaveFailChannel(iterSlot->first, nChannel);
					continue;
				}

				if (0 == nChannel % 4 || 1 == nChannel % 4)
				{
					nBaseFailLine = ulExpectBaseFailLine[0];
				}
				else
				{
					nBaseFailLine = ulExpectBaseFailLine[1];
				}

				for (int nFailIndex = 0; nFailIndex < nGetFailCount; ++nFailIndex)
				{
					ulCurFailLine = nBaseFailLine + nFailStep * nFailIndex;
					XT_EXPECT_EQ(ulFailLineNo[nFailIndex], ulCurFailLine);
					if (ulFailLineNo[nFailIndex] != ulCurFailLine)
					{
						funcReport.SaveFailChannel(iterSlot->second, nChannel);
						break;
					}
				}
			}
		}

		++iterSlot;
	}
	funcReport.Print(this, g_lpszReportFilePath);

	dcm.Disconnect("G_ALLPIN");//接通各物理通道的输出继电器
}