#pragma once
/**
 * @file TestDCMGetRunLineCountRunningTime.h
 * @brief Check the running time of GetRunLineCount
 * @author Guangyun Wang
 * @date 2021/04/23
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controller Technology Co., Ltd.
*/
#include "MCUCase.h"
XT_TEST(FunctionRunningTimeTest, TestDCMGetRunLineCountRunningTime)
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
	dcm.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");
	
	dcm.RunVectorWithGroup("G_ALLPIN", "TEST_BRAM_ST", "TEST_BRAM_SP");
	ULONG ulLineCount = 0;
	timeReport.timeStart();
	dcm.GetRunLineCount("CH0", mapSlot.begin()->second, ulLineCount);
	timeReport.timeStop();
	timeReport.addMsg("Get the run line count in BRAM vector.");

	dcm.RunVectorWithGroup("G_ALLPIN", "TEST_FAIL_ST", "TEST_FAIL_SP");
	timeReport.timeStart();
	dcm.GetRunLineCount("CH0", mapSlot.begin()->second, ulLineCount);
	timeReport.timeStop();
	timeReport.addMsg("Get the run line count in DRAM vector.");

	dcm.RunVectorWithGroup("G_ALLPIN", "TEST_ENDLESS_ST", "TEST_ENDLESS_SP", FALSE);
	timeReport.timeStart();
	dcm.GetRunLineCount("CH0", mapSlot.begin()->second, ulLineCount);
	timeReport.timeStop();
	timeReport.addMsg("Get the run line count in running vector.");
	dcm.StopVector("G_ALLPIN");
	
	timeReport.Print(this, g_lpszReportFilePath);
	dcm_CloseFile();
}