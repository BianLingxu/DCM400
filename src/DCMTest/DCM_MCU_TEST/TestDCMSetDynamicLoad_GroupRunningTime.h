#pragma once
/**
 * @file TestDCMSetDynamicLoad_GroupRunningTime.h
 * @brief Check the running time of SetDynamicLoad
 * @author Guangyun Wang
 * @date 2021/06/08
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controller Technology Co., Ltd.
*/
#include "..\DCMTestMain.h"
XT_TEST(FunctionRunningTimeTest, TestDCMSetDynamicLoad_GroupRunningTime)
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
	USHORT usSiteCount = mapSlot.size() * DCM_MAX_CONTROLLERS_PRE_BOARD;
	timeReport.timeStart();
	dcm.SetDynamicLoad("CH0", TRUE, 0.01,0.01,3.5);
	timeReport.timeStop();
	timeReport.addMsg("One pin in group, Site count:%d, open dynamic load", usSiteCount);

	timeReport.timeStart();
	dcm.SetDynamicLoad("CH0", FALSE, 0.01, 0.01, 3.5);
	timeReport.timeStop();
	timeReport.addMsg("One pin in group, Site count:%d, close dynamic load", usSiteCount);

	dcm.SetPinGroup("G_TWO", "CH0,CH1");

	timeReport.timeStart();
	dcm.SetDynamicLoad("G_TWO", TRUE, 0.01, 0.01, 3.5);
	timeReport.timeStop();
	timeReport.addMsg("Two pin in group, Site count:%d, open dynamic load", usSiteCount);

	timeReport.timeStart();
	dcm.SetDynamicLoad("G_TWO", FALSE, 0.01, 0.01, 3.5);
	timeReport.timeStop();
	timeReport.addMsg("Two pin in group, Site count:%d, close dynamic load", usSiteCount);

	mapSlot.clear();
	timeReport.Print(this, g_lpszReportFilePath);

	dcm_CloseFile();
}
