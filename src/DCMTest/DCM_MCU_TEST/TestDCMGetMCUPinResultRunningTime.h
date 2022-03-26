#pragma once
/*!
* @file      TestDCMGetMCUPinResultRunningTime.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/12/12
* @version   v 1.0.0.0
* @brief     测试GetMCUPinResult运行时间
* @comment
*/
#include "..\DCMTestMain.h"
XT_TEST(FunctionRunningTimeTest, TestDCMGetMCUPinResultRunningTime)
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
	dcm.Connect("G_ALLPIN");
	dcm.SetPinLevel("G_ALLPIN", 3.0, 0, 1.5, 0.8);

	dcm.RunVectorWithGroup("G_ALLPIN", "ALL_PASS_ST", "ALL_PASS_SP");
	timeReport.timeStart();
	dcm.GetMCUPinResult("CH0", iterSlot->second);
	timeReport.timeStop();
	timeReport.addMsg("All Pass Vector");


	dcm.RunVectorWithGroup("G_ALLPIN", "FAIL_DRAM_ST", "FAIL_DRAM_SP");
	timeReport.timeStart();
	dcm.GetMCUPinResult("CH0", iterSlot->second);
	timeReport.timeStop();
	timeReport.addMsg("Have Fail Vector");

	mapSlot.clear();
	timeReport.Print(this, g_lpszReportFilePath);

	dcm.Disconnect("G_ALLPIN");
	dcm_CloseFile();
}
