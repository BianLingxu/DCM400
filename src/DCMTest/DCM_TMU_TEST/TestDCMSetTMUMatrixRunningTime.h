#pragma once
/**
 * @file TestDCMSetTMUMatixRunningTime.h
 * @brief Test the running time of function SetTMUMatrix
 * @author Guangyun Wang
 * @date 2020/09/03
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controller Co., Ltd.
*/
#include "..\DCMTestMain.h"

XT_TEST(TMURunningTimeTest, TestDCMSetTMUMatixRunningTime)
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

	dcm.LoadVectorFile(g_lpszVectorFilePath, FALSE);

	timeReport.SetTimes(100);
	timeReport.timeStart();
	for (int nIndex = 0; nIndex < 100; ++nIndex)
	{
		dcm.SetTMUMatrix("CH0", DCM_ALLSITE, DCM_TMU1);
	}
	timeReport.timeStop();
	timeReport.addMsg("Connect %d channels' even TMU", mapSlot.size() * DCM_MAX_CONTROLLERS_PRE_BOARD);

	timeReport.timeStart();
	for (int nIndex = 0; nIndex < 100; ++nIndex)
	{
		dcm.SetTMUMatrix("CH0", mapSlot.begin()->second, DCM_TMU1);
	}
	timeReport.timeStop();
	timeReport.addMsg("Connect %d channels' even TMU", 1);


	timeReport.timeStart();
	for (int nIndex = 0; nIndex < 100; ++nIndex)
	{
		dcm.SetTMUMatrix("CH0", DCM_ALLSITE, DCM_TMU2);
	}
	timeReport.timeStop();
	timeReport.addMsg("Connect %d channels' odd TMU", mapSlot.size() * DCM_MAX_CONTROLLERS_PRE_BOARD);

	timeReport.timeStart();
	for (int nIndex = 0; nIndex < 100; ++nIndex)
	{
		dcm.SetTMUMatrix("CH0", mapSlot.begin()->second, DCM_TMU2);
	}
	timeReport.timeStop();
	timeReport.addMsg("Connect %d channels' odd TMU", 1);

	timeReport.Print(this, g_lpszReportFilePath);

	dcm_CloseFile();
}
