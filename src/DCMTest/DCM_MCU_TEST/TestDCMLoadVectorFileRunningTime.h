#pragma once
/*!
* @file      TestDCMLoadVectorFileRunningTime.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/11/14
* @version   v 1.0.0.0
* @brief     测试LoadVectorFile运行时间
* @comment
*/
#include "..\DCMTestMain.h"
XT_TEST(FunctionRunningTimeTest, TestDCMLoadVectorFileRunningTime)
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

	timeReport.timeStart();
	dcm.LoadVectorFile(g_lpszVectorFilePath);
	timeReport.timeStop();
	ULONG ulVectorLineCount = 0;
	dcm_GetVectorLineCount("", "", ulVectorLineCount);
	timeReport.addMsg("The line count of the vector is %d.", ulVectorLineCount);



	timeReport.timeStart();
	dcm.LoadVectorFile(g_lpszVectorFilePath, FALSE);
	timeReport.timeStop();
	timeReport.addMsg("The line count of the vector is %d, not load vector repeatedly", ulVectorLineCount);

	timeReport.Print(this, g_lpszReportFilePath);

	mapSlot.clear();
	dcm_CloseFile();
}
