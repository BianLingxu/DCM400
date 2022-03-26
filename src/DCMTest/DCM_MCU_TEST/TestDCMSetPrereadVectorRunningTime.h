#pragma once
/**
 * @file TestDCMSetPrereadVectorRunningTime.h
 * @brief Check the running time of SetPrereadVector
 * @author Guangyun Wang
 * @date 2021/06/02
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controller Technology Co., Ltd.
*/

#include "..\DCMTestMain.h"
XT_TEST(FunctionRunningTimeTest, TestDCMSetPrereadVectorRunningTime)
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

	char* lpszLabel[4] = { "TEST_FAIL_ST","TEST_FAIL_SP" ,"FIRST_ST","FIRST_SP" };

	ULONG ulLineCount = 0;

	timeReport.timeStart();
	dcm.SetPrereadVector(lpszLabel[2], lpszLabel[3]);
	timeReport.timeStop();
	dcm_GetVectorLineCount(lpszLabel[2], lpszLabel[3], ulLineCount);
	timeReport.addMsg("Preread %d lines vector without DRAM", ulLineCount);

	timeReport.timeStart();
	dcm.SetPrereadVector(lpszLabel[0], lpszLabel[1]);
	timeReport.timeStop();
	dcm_GetVectorLineCount(lpszLabel[0], lpszLabel[1], ulLineCount);
	timeReport.addMsg("Preread %d lines vector with DRAM.", ulLineCount);

	timeReport.Print(this, g_lpszReportFilePath);

	dcm_CloseFile();
}
