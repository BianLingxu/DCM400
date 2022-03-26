#pragma once
/**
 * @file TestDCMSetValidPinRunningTime.h
 * @brief Test the function running time of SetValidPin
 * @author Guangyun Wang
 * @date 20021/08/04
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controller Co., Ltd.
*/
#include "..\DCMTestMain.h"
XT_TEST(FunctionRunningTimeTest, TestDCMSetValidPinRunningTime)
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

	timeReport.timeStart();
	dcm.SetValidPin("CH0,CH1");
	timeReport.timeStop();
	timeReport.addMsg("Set the valid pin of monopolize vector");
	dcm_CloseFile();

	DCM dcm1;
	dcm.LoadVectorFile(g_lpszVectorFilePath, FALSE);

	timeReport.timeStart();
	dcm.SetValidPin("CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");
	timeReport.timeStop();
	timeReport.addMsg("Set the 64 valid pins of shared vector for loaded instance");

	timeReport.timeStart();
	dcm.SetValidPin("CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7");
	timeReport.timeStop();
	timeReport.addMsg("Set the 8 valid pins of shared vector  for loaded instance");


	timeReport.timeStart();
	dcm.SetValidPin("CH0,CH1,CH2,CH3");
	timeReport.timeStop();
	timeReport.addMsg("Set the 4 valid pins of shared vector for loaded instance");


	timeReport.timeStart();
	dcm.SetValidPin("CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");
	timeReport.timeStop();
	timeReport.addMsg("Set the 64 valid pins of monopolize for instance share vector");

	timeReport.timeStart();
	dcm.SetValidPin("CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7");
	timeReport.timeStop();
	timeReport.addMsg("Set the 8 valid pins of shared vector for instance share vector");


	timeReport.timeStart();
	dcm.SetValidPin("CH0,CH1,CH2,CH3");
	timeReport.timeStop();
	timeReport.addMsg("Set the 4 valid pins of shared vector for instance share vector");

	timeReport.Print(this, g_lpszReportFilePath);

	dcm_CloseFile();
}
