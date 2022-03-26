#pragma once
/**
 * @file TestDCMSetCaptureRunningTime.h
 * @brief Test the function running time of SetCapture
 * @author Guangyun Wang
 * @date 20021/04/21
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controller Co., Ltd.
*/
#include "..\DCMTestMain.h"
XT_TEST(FunctionRunningTimeTest, TestDCMSetCaptureRunningTime)
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
	double dPeriod = dcm_GetTimeSetPeriod(iterSlot->first, 0, 0);
	dcm.SetEdge("G_ALLPIN", "0", DCM_DTFT_NRZ, dPeriod / 8, dPeriod * 5 / 8, dPeriod / 8, dPeriod * 3 / 8);

	dcm.RunVectorWithGroup("G_ALLPIN", "FAIL_BRAM_ST", "FAIL_BRAM_SP");
	ULONG ulCaptureData = 0;

	timeReport.timeStart();
	dcm.SetCapture("CH0", "FAIL_BRAM_ST", iterSlot->second, 10, 8);
	timeReport.timeStop();
	timeReport.addMsg("Get %d lines vector data in BRAM at %d controllers 1 channel each ", 8, 1);

	dcm.RunVectorWithGroup("G_ALLPIN", "FAIL_BRAM_ST", "FAIL_BRAM_SP");
	timeReport.timeStart();
	dcm.SetCapture("G_ALLPIN", "FAIL_BRAM_ST", iterSlot->second, 10, 8);
	timeReport.timeStop();
	timeReport.addMsg("Get %d lines vector data in BRAM at %d controllers %d channels each", 8, 1, DCM_CHANNELS_PER_CONTROL);

	dcm.RunVectorWithGroup("G_ALLPIN", "FAIL_BRAM_ST", "FAIL_BRAM_SP");
	timeReport.timeStart();
	dcm.SetCapture("G_ALLPIN", "FAIL_BRAM_ST", DCM_ALLSITE, 10, 8);
	timeReport.timeStop();
	timeReport.addMsg("Get %d lines vector data in BRAM at %d controllers %d channels each", 8, mapSlot.size() * DCM_MAX_CONTROLLERS_PRE_BOARD, DCM_CHANNELS_PER_CONTROL);

	dcm.RunVectorWithGroup("G_ALLPIN", "FAIL_BRAM_ST", "FAIL_BRAM_SP");
	timeReport.timeStart();
	dcm.SetCapture("G_ALLPIN", "FAIL_BRAM_ST", DCM_ALLSITE, 10, 16);
	timeReport.timeStop();
	timeReport.addMsg("Get %d lines vector data in BRAM at %d controllers %d channels each", 16, mapSlot.size() * DCM_MAX_CONTROLLERS_PRE_BOARD, DCM_CHANNELS_PER_CONTROL);

	dcm.RunVectorWithGroup("G_ALLPIN", "FAIL_DRAM_ST", "FAIL_DRAM_SP");
	timeReport.timeStart();
	dcm.SetCapture("CH0", "FAIL_DRAM_ST", iterSlot->second, 10, 8);
	timeReport.timeStop();
	timeReport.addMsg("Get %d lines vector data in DRAM at %d controllers 1 channel each ", 8, 1);

	dcm.RunVectorWithGroup("G_ALLPIN", "FAIL_DRAM_ST", "FAIL_DRAM_SP");
	timeReport.timeStart();
	dcm.SetCapture("G_ALLPIN", "FAIL_DRAM_ST", iterSlot->second, 10, 8);
	timeReport.timeStop();
	timeReport.addMsg("Get %d lines vector data in DRAM at %d controllers %d channels each", 8, 1, DCM_CHANNELS_PER_CONTROL);

	dcm.RunVectorWithGroup("G_ALLPIN", "FAIL_DRAM_ST", "FAIL_DRAM_SP");
	timeReport.timeStart();
	dcm.SetCapture("G_ALLPIN", "FAIL_DRAM_ST", DCM_ALLSITE, 10, 8);
	timeReport.timeStop();
	timeReport.addMsg("Get %d lines vector data in DRAM at %d controllers %d channels each", 8, mapSlot.size() * DCM_MAX_CONTROLLERS_PRE_BOARD, DCM_CHANNELS_PER_CONTROL);

	dcm.RunVectorWithGroup("G_ALLPIN", "FAIL_DRAM_ST", "FAIL_DRAM_SP");
	timeReport.timeStart();
	dcm.SetCapture("G_ALLPIN", "FAIL_DRAM_ST", DCM_ALLSITE, 10, 16);
	timeReport.timeStop();
	timeReport.addMsg("Get %d lines vector data in DRAM at %d controllers %d channels each", 16, mapSlot.size() * DCM_MAX_CONTROLLERS_PRE_BOARD, DCM_CHANNELS_PER_CONTROL);


	timeReport.Print(this, g_lpszReportFilePath);

	dcm.Disconnect("G_ALLPIN");
	dcm_CloseFile();
}
