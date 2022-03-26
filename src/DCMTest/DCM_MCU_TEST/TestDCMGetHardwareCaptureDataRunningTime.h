#pragma once
/*!
* @file      TestDCMGetHardwareCaptureDataRunningTime.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2020/07/15
* @version   v 1.0.0.0
* @brief     测试GetHardwareCaptureData运行时间
* @comment
*/
#include "..\DCMTestMain.h"
XT_TEST(FunctionRunningTimeTest, TestDCMGetHardwareCaptureDataRunningTime)
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
	USHORT usSiteNo = mapSlot.begin()->second;

	dcm.LoadVectorFile(g_lpszVectorFilePath, FALSE);
	dcm.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");

	dcm.Connect("G_ALLPIN");
	dcm.SetPinLevel("G_ALLPIN", 3.0, 0, 1.5, 0.8);

	dcm.RunVectorWithGroup("G_ALLPIN", "FAIL_BRAM_ST", "FAIL_BRAM_SP");
	ULONG ulCaptureData = 0;
	BYTE byCaptureData[8] = { 0 };
	timeReport.timeStart();
	nRetVal = dcm.GetHardwareCaptureData("CH0", usSiteNo, byCaptureData, sizeof(byCaptureData));
	timeReport.timeStop();
	timeReport.addMsg("Get %d lines vector data in BRAM, firstly", nRetVal);

	timeReport.timeStart();
	nRetVal = dcm.GetHardwareCaptureData("CH0", usSiteNo, byCaptureData, sizeof(byCaptureData));
	timeReport.timeStop();
	timeReport.addMsg("Get %d lines vector data in BRAM, secondly", nRetVal);


	dcm.RunVectorWithGroup("G_ALLPIN", "TEST_FAIL_ST", "TEST_FAIL_SP");

	timeReport.timeStart();
	nRetVal = dcm.GetHardwareCaptureData("CH0", usSiteNo, byCaptureData, sizeof(byCaptureData));
	timeReport.timeStop();
	timeReport.addMsg("Get %d lines vector data in DRAM, firstly", nRetVal);

	timeReport.timeStart();
	nRetVal = dcm.GetHardwareCaptureData("CH0", usSiteNo, byCaptureData, sizeof(byCaptureData));
	timeReport.timeStop();
	timeReport.addMsg("Get %d lines vector data in DRAM, secondly", nRetVal);

	timeReport.Print(this, g_lpszReportFilePath);

	dcm.Disconnect("G_ALLPIN");
	dcm_CloseFile();
}
