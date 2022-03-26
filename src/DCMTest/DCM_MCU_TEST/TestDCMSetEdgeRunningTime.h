#pragma once
/*!
* @file      TestDCMSetEdgeRunningTime.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/12/12
* @version   v 1.0.0.0
* @brief     测试SetEdge运行时间
* @comment
*/
#include "..\DCMTestMain.h"
XT_TEST(FunctionRunningTimeTest, TestDCMSetEdgeRunningTime)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, RUNNING_TIME);
	CTimeReport timeReport(strFuncName.c_str(), "FunctionRunningTimeTest");
	
	int nRetVal = 0;
	map<BYTE, USHORT> mapSlot;
	nRetVal = GetBoardInfo(mapSlot, g_lpszVectorFilePath);

	if (0 == mapSlot.size())
	{
		//No board is inserted.
		XT_EXPECT_TRUE(FALSE);
		timeReport.SetNoBoardValid();
		timeReport.Print(this, g_lpszReportFilePath);
		return;
	}

	XT_EXPECT_EQ(nRetVal, 0);
	if (0 != nRetVal)
	{
		timeReport.addMsg("Load vector file(%s) fail, the vector file maybe not right.", g_lpszVectorFilePath);
		timeReport.Print(this, g_lpszReportFilePath);
		return;
	}

	SaveBoardSN(timeReport, mapSlot);
	auto iterSlot = mapSlot.begin();

	dcm.LoadVectorFile(g_lpszVectorFilePath, FALSE);
	dcm.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");
	dcm.SetPinGroup("G_ODDPIN", "CH1,CH3,CH5,CH7,CH9,CH11,CH13,CH15");
	dcm.SetPinGroup("G_EVENPIN", "CH0,CH2,CH4,CH6,CH8,CH10,CH12,CH14");

	double dPeriod = dcm_GetTimeSetPeriod(iterSlot->first, 0, 0);
	int nChannelCount = mapSlot.size() * DCM_MAX_CHANNELS_PER_BOARD;

	timeReport.timeStart();
	dcm.SetEdge("G_ALLPIN", "0", DCM_DTFT_NRZ, dPeriod / 8, dPeriod * 5 / 8, dPeriod / 8, dPeriod * 3 / 8);
	timeReport.timeStop();
	timeReport.addMsg("Set Time: %d channels ", nChannelCount);

	timeReport.timeStart();
	dcm.SetEdge("G_ODDPIN", "0", DCM_DTFT_NRZ, dPeriod / 8, dPeriod * 5 / 8, dPeriod / 8, dPeriod * 3 / 8);
	timeReport.timeStop();
	timeReport.addMsg("Set Time: %d channels of odd pin ", nChannelCount / 2);

	timeReport.timeStart();
	dcm.SetEdge("G_EVENPIN", "0", DCM_DTFT_NRZ, dPeriod / 8, dPeriod * 5 / 8, dPeriod / 8, dPeriod * 3 / 8);
	timeReport.timeStop();
	timeReport.addMsg("Set Time: %d channels of even pin ", nChannelCount / 2);


	timeReport.Print(this, g_lpszReportFilePath);

	dcm_CloseFile();
}
