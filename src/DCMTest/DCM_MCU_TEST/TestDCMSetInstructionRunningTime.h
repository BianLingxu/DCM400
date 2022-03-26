/**
 * @file TestDCMSetInstructionRunningTime.h
 * @brief Test the function running time of SetInstruction
 * @author Guangyun Wang
 * @date 20020/11/27
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controller Co., Ltd.
*/
#pragma once
#include "..\DCMTestMain.h"

XT_TEST(FunctionRunningTimeTest, TestDCMSetInstructionRunningTime)
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


	int nControllerCount = mapSlot.size() * DCM_MAX_CONTROLLERS_PRE_BOARD;

	timeReport.timeStart();
	dcm.SetInstruction("G_ALLPIN", "TEST_INS_ST", 1, "JUMP", "TEST_INS_SP");
	timeReport.timeStop();
	timeReport.addMsg("Set JUMP instruction to %d controllers", nControllerCount);

	timeReport.timeStart();
	dcm.SetInstruction("G_ALLPIN", "TEST_INS_ST", 1, "REPEAT", "10");
	timeReport.timeStop();
	timeReport.addMsg("Set REPEAT instruction to %d controllers", nControllerCount);


	timeReport.Print(this, g_lpszReportFilePath);
	dcm_CloseFile();
}