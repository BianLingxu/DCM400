#pragma once
/**
 * @file TestDCMSetPPMURunningTime.h
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controller Co., Ltd.
 * @author Guangyun Wang
 * @date 2020/07/17
 * @version v 1.0.0.0
 * @brief Test the execution time of SetPPMU
 */
#include "..\DCMTestMain.h"

XT_TEST(PMURunningTimeTest, TestDCMSetPPMURunningTime)
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
	dcm.SetPinGroup("G_ODDPIN", "CH1,CH3,CH5,CH7,CH9,CH11,CH13,CH15");
	dcm.SetPinGroup("G_EVENPIN", "CH0,CH2,CH4,CH6,CH8,CH10,CH12,CH14");

	int nChannelCount = mapSlot.size() * DCM_MAX_CHANNELS_PER_BOARD;

	timeReport.timeStart();
	dcm.SetPPMU("G_ALLPIN", DCM_PPMU_FVMV, 0, DCM_PPMUIRANGE_32MA);
	timeReport.timeStop();
	timeReport.addMsg("Set %d channels' PMU to FVMV 0V", nChannelCount);

	timeReport.timeStart();
	dcm.SetPPMU("G_ODDPIN", DCM_PPMU_FVMV, 0, DCM_PPMUIRANGE_32MA);
	timeReport.timeStop();
	timeReport.addMsg("Set %d channels' PMU to FVMV 0V", nChannelCount / 2);

	timeReport.timeStart();
	dcm.SetPPMU("G_EVENPIN", DCM_PPMU_FVMV, 0, DCM_PPMUIRANGE_32MA);
	timeReport.timeStop();
	timeReport.addMsg("Set %d channels' PMU to FVMV 0V", nChannelCount / 2);

	timeReport.Print(this, g_lpszReportFilePath);

	dcm_CloseFile();
}
