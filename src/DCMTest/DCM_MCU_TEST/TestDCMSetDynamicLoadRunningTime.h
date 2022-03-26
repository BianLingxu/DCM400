#pragma once
/*!
* @file      TestDCMSetDynamicLoadRunningTime.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/12/12
* @version   v 1.0.0.0
* @brief     测试SetDynamicLoad运行时间
* @comment
*/
#include "..\DCMTestMain.h"
XT_TEST(FunctionRunningTimeTest, TestDCMSetDynamicLoadRunningTime)
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

	timeReport.timeStart();
	dcm.SetDynamicLoad("CH0", iterSlot->second, DCM_CLOSE_CLAMP_OPEN_LOAD);
	timeReport.timeStop();
	timeReport.addMsg("Pin name:CH0. Mode: DCM_CLOSE_CLAMP_OPEN_LOAD");

	timeReport.timeStart();
	dcm.SetDynamicLoad("CH0", iterSlot->second, DCM_OPEN_CLAMP_CLOSE_LOAD);
	timeReport.timeStop();
	timeReport.addMsg("Pin name:CH0. Mode: DCM_OPEN_CLAMP_CLOSE_LOAD");

	timeReport.timeStart();
	dcm.SetDynamicLoad("CH0", iterSlot->second, DCM_OPEN_CLAMP_OPEN_LOAD);
	timeReport.timeStop();
	timeReport.addMsg("Pin name:CH0. Mode: DCM_OPEN_CLAMP_OPEN_LOAD");

	timeReport.timeStart();
	dcm.SetDynamicLoad("CH0", iterSlot->second, DCM_CLOSE_CLAMP_CLOSE_LOAD);
	timeReport.timeStop();
	timeReport.addMsg("Pin name:CH0. Mode: DCM_CLOSE_CLAMP_CLOSE_LOAD");

	mapSlot.clear();
	timeReport.Print(this, g_lpszReportFilePath);

	dcm_CloseFile();
}
