#pragma once
/**
 * @file TestDCMPPMUMeasureRunningTime.h
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controller Co., Ltd.
 * @author Guangyun Wang
 * @date 2020/07/17
 * @version v 1.0.0.0
 * @brief Test the execution time of PPMUMeasure
 */
#include "..\DCMTestMain.h"

XT_TEST(PMURunningTimeTest, TestDCMPPMUMeasureRunningTime)
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

	int nChannelCount = mapSlot.size() * DCM_CHANNELS_PER_CONTROL;


	///<FVMV
	timeReport.timeStart();
	dcm.PPMUMeasure("G_ODDPIN", 10, 10);
	timeReport.timeStop();
	timeReport.addMsg("Measure %d channels' FVMV 2V, Sample %d, Period %d", nChannelCount / 2, 10, 10);

	timeReport.timeStart();
	dcm.PPMUMeasure("G_ALLPIN", 10, 10);
	timeReport.timeStop();
	timeReport.addMsg("Measure %d channels' FVMV 2V, Sample %d, Period %d", nChannelCount, 10, 10);

	timeReport.timeStart();
	dcm.PPMUMeasure("G_ALLPIN", 10, 100);
	timeReport.timeStop();
	timeReport.addMsg("Measure %d channels' FVMV 2V, Sample %d, Period %d", nChannelCount, 10, 100);

	timeReport.timeStart();
	dcm.PPMUMeasure("G_ALLPIN", 100, 10);
	timeReport.timeStop();
	timeReport.addMsg("Measure %d channels' FVMV 2V, Sample %d, Period %d", nChannelCount, 100, 10);

	///<FVMI
	dcm.SetPPMU("G_ALLPIN", DCM_PPMU_FVMV, 2, DCM_PPMUIRANGE_32MA);

	timeReport.timeStart();
	dcm.PPMUMeasure("G_ODDPIN", 10, 10);
	timeReport.timeStop();
	timeReport.addMsg("Measure %d channels' FVMI 2V, Sample %d, Period %d", nChannelCount / 2, 10, 10);

	timeReport.timeStart();
	dcm.PPMUMeasure("G_ALLPIN", 10, 10);
	timeReport.timeStop();
	timeReport.addMsg("Measure %d channels' FVMI 2V, Sample %d, Period %d", nChannelCount, 10, 10);

	timeReport.timeStart();
	dcm.PPMUMeasure("G_ALLPIN", 10, 100);
	timeReport.timeStop();
	timeReport.addMsg("Measure %d channels' FVMI 2V, Sample %d, Period %d", nChannelCount, 10, 100);

	timeReport.timeStart();
	dcm.PPMUMeasure("G_ALLPIN", 100, 10);
	timeReport.timeStop();
	timeReport.addMsg("Measure %d channels' FVMI 2V, Sample %d, Period %d", nChannelCount, 100, 10);


	///<FIMV
	dcm.SetPPMU("G_ALLPIN", DCM_PPMU_FIMI, 1e-3, DCM_PPMUIRANGE_32MA);

	timeReport.timeStart();
	dcm.PPMUMeasure("G_ODDPIN", 10, 10);
	timeReport.timeStop();
	timeReport.addMsg("Measure %d channels' FIMV 1mA, Sample %d, Period %d", nChannelCount / 2, 10, 10);

	timeReport.timeStart();
	dcm.PPMUMeasure("G_ALLPIN", 10, 10);
	timeReport.timeStop();
	timeReport.addMsg("Measure %d channels' FIMV 1mA, Sample %d, Period %d", nChannelCount, 10, 10);

	timeReport.timeStart();
	dcm.PPMUMeasure("G_ALLPIN", 10, 100);
	timeReport.timeStop();
	timeReport.addMsg("Measure %d channels' FIMV 1mA, Sample %d, Period %d", nChannelCount, 10, 100);

	timeReport.timeStart();
	dcm.PPMUMeasure("G_ALLPIN", 100, 10);
	timeReport.timeStop();
	timeReport.addMsg("Measure %d channels' FIMV 1mA, Sample %d, Period %d", nChannelCount, 100, 10);


	///<FIMI
	dcm.SetPPMU("G_ALLPIN", DCM_PPMU_FIMI, 1e-3, DCM_PPMUIRANGE_32MA);

	timeReport.timeStart();
	dcm.PPMUMeasure("G_ODDPIN", 10, 10);
	timeReport.timeStop();
	timeReport.addMsg("Measure %d channels' FIMI 1mA, Sample %d, Period %d", nChannelCount / 2, 10, 10);

	timeReport.timeStart();
	dcm.PPMUMeasure("G_ALLPIN", 10, 10);
	timeReport.timeStop();
	timeReport.addMsg("Measure %d channels' FIMI 1mA, Sample %d, Period %d", nChannelCount, 10, 10);

	timeReport.timeStart();
	dcm.PPMUMeasure("G_ALLPIN", 10, 100);
	timeReport.timeStop();
	timeReport.addMsg("Measure %d channels' FIMI 1mA, Sample %d, Period %d", nChannelCount, 10, 100);

	timeReport.timeStart();
	dcm.PPMUMeasure("G_ALLPIN", 100, 10);
	timeReport.timeStop();
	timeReport.addMsg("Measure %d channels' FIMI 1mA, Sample %d, Period %d", nChannelCount, 100, 10);

	dcm_CloseFile();
	dcm_EnableAddPin(TRUE, TRUE);
	BYTE bySlotNo = mapSlot.begin()->first;
	char lpszChannel[32] = { 0 };
	sprintf_s(lpszChannel, sizeof(lpszChannel), "S%d_0,S%d_19", bySlotNo, bySlotNo);
	dcm_AddPin("CH0", 0, lpszChannel);
	sprintf_s(lpszChannel, sizeof(lpszChannel), "S%d_3,S%d_16", bySlotNo, bySlotNo);
	dcm_AddPin("CH1", 1, lpszChannel);
	sprintf_s(lpszChannel, sizeof(lpszChannel), "S%d_1,S%d_17", bySlotNo, bySlotNo);
	dcm_AddPin("CH3", 3, lpszChannel);
	sprintf_s(lpszChannel, sizeof(lpszChannel), "S%d_2,S%d_18", bySlotNo, bySlotNo);
	dcm_AddPin("CH4", 4, lpszChannel);
	dcm.SetPinGroup("G_FORCE", "CH3,CH4");
	dcm.SetPinGroup("G_MEAS", "CH0,CH1");
	dcm.SetPinGroup("G_ALLPIN", "G_FORCE,G_MEAS");
	dcm.Connect("G_ALLPIN");

	dcm.SetPPMU("G_FORCE", DCM_PPMU_FIMV, 1e-4, DCM_PPMUIRANGE_32MA);
	dcm.SetPPMU("G_MEAS", DCM_PPMU_FVMI, 0, DCM_PPMUIRANGE_32MA);
	
	timeReport.timeStart();
	dcm.PPMUMeasure("G_MEAS", 10, DCM_PPMU_10US);
	timeReport.timeStop();
	timeReport.addMsg("Parallel measurement in two controllers");
	dcm.Disconnect("G_ALLPIN");


	timeReport.Print(this, g_lpszReportFilePath);

	dcm_CloseFile();
}
