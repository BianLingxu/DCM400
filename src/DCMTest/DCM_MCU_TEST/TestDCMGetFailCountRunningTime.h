#pragma once
/*!
* @file      TestDCMGetFailCountRunningTime.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/11/15
* @version   v 1.0.0.0
* @brief     测试GetFailCount运行时间
* @comment
*/
#include "..\DCMTestMain.h"
XT_TEST(FunctionRunningTimeTest, TestDCMGetFailCountRunningTime)
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

	iterSlot = mapSlot.begin();
	dcm.Connect("G_ALLPIN");
	dcm.SetPinLevel("G_ALLPIN", 3.0, 0, 1.5, 0.8);
	double dPeriod = dcm_GetTimeSetPeriod(iterSlot->first, 0, 0);
	dcm.SetEdge("G_ALLPIN", "0", DCM_DTFT_NRZ, dPeriod / 8, dPeriod * 3 / 4, dPeriod / 8, dPeriod * 5 / 8);

	char *lpszLabel[6] = { "ALL_PASS_ST","ALL_PASS_SP" ,"FAIL_BRAM_ST","FAIL_BRAM_SP","FAIL_DRAM_ST","FAIL_DRAM_SP" };


	dcm.RunVectorWithGroup("G_ALLPIN", lpszLabel[0], lpszLabel[1]);
	ULONG ulFailCount = 0;
	timeReport.timeStart();
	dcm.GetFailCount("CH0", iterSlot->second, ulFailCount);
	timeReport.timeStop();
	ULONG ulLineCount = 0;
	dcm_GetVectorLineCount(lpszLabel[0], lpszLabel[1], ulLineCount);
	timeReport.addMsg("%d lines vector. ALL PSS.", ulLineCount);


	dcm.RunVectorWithGroup("G_ALLPIN", lpszLabel[2], lpszLabel[3]);
	timeReport.timeStart();
	dcm.GetFailCount("CH0", iterSlot->second, ulFailCount);
	timeReport.timeStop();
	dcm_GetVectorLineCount(lpszLabel[2], lpszLabel[3], ulLineCount);
	timeReport.addMsg("%d lines vector without DRAM line. %d lines fail.", ulLineCount,ulFailCount);

	dcm.RunVectorWithGroup("G_ALLPIN", lpszLabel[4], lpszLabel[5]);
	timeReport.timeStart();
	dcm.GetFailCount("CH0", iterSlot->second, ulFailCount);
	timeReport.timeStop();
	dcm_GetVectorLineCount(lpszLabel[4], lpszLabel[5], ulLineCount);
	timeReport.addMsg("%d lines vector with DRAM line. %d lines fail.", ulLineCount, ulFailCount);
	
	mapSlot.begin();
	timeReport.Print(this, g_lpszReportFilePath);

	dcm.Disconnect("G_ALLPIN");
	dcm_CloseFile();
}
