#pragma once
/*!
* @file      TestDCMRunVectorWithGroupRunningTime.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/11/14
* @version   v 1.0.0.0
* @brief     测试RunVectorWithGroup运行时间
* @comment
*/
#include "..\DCMTestMain.h"

XT_TEST(FunctionRunningTimeTest, TestDCMRunVectorWithGroupRunningTime)
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

	dcm.SetPinLevel("G_ALLPIN", 3.0, 0, 1.5, 0.8);
	double dPeriod = dcm_GetTimeSetPeriod(mapSlot.begin()->first, 0, 0);
	dcm.SetEdge("G_ALLPIN", "0", DCM_DTFT_NRZ, dPeriod / 8, dPeriod * 3 / 4, dPeriod / 8, dPeriod * 5 / 8);

	char *lpszLabel[4] = { "TEST_FAIL_ST","TEST_FAIL_SP" ,"FIRST_ST","FIRST_SP"};

	ULONG ulLineCount = 0;

	timeReport.timeStart();
	dcm.RunVectorWithGroup("G_ALLPIN", lpszLabel[2], lpszLabel[3]);
	timeReport.timeStop();
	dcm_GetVectorLineCount(lpszLabel[2], lpszLabel[3], ulLineCount);
	timeReport.addMsg("Run %d lines vector without DRAM and period is %.1f ns.", ulLineCount, dPeriod);

	timeReport.timeStart();
	dcm.RunVectorWithGroup("G_ALLPIN", lpszLabel[0], lpszLabel[1]);
	timeReport.timeStop();
	dcm_GetVectorLineCount(lpszLabel[0], lpszLabel[1], ulLineCount);
	dPeriod = dcm_GetTimeSetPeriod(mapSlot.begin()->first, 0, 0);
	timeReport.addMsg("Run %d lines vector with DRAM and period is %.1f ns.", ulLineCount, dPeriod);


	timeReport.timeStart();
	dcm.RunVectorWithGroup("G_ALLPIN", lpszLabel[0], lpszLabel[1], FALSE);
	timeReport.timeStop();
	timeReport.addMsg("Run %d lines vector with DRAM and period is %.1f ns, not wait vector stop.", ulLineCount, dPeriod);
	mapSlot.clear();

	timeReport.Print(this, g_lpszReportFilePath);

	dcm_CloseFile();
}
