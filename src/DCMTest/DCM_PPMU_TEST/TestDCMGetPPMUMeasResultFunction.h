#pragma once
/**
 * @file TestDCMSetPPMUFunction.h
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controller Co., Ltd.
 * @author Guangyun Wang
 * @date 2020/07/15
 * @version v 1.0.0.0
 * @brief Test the function of GetPPMUMeasResult
 */
#include "..\DCMTestMain.h"
XT_TEST(PMUFunctionTest, TestDCMGetPPMUMeasResultFunction)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, FUNCTION);
	int nRetVal = 0;
	CMeasurementFuncReport Report(strFuncName.c_str(), "PMUFunctionTest");//Error message.

	map<BYTE, USHORT> mapSlot;

	nRetVal = GetBoardInfo(mapSlot, g_lpszVectorFilePath);

	if (0 == mapSlot.size())
	{
		///<No board is inserted
		XT_EXPECT_TRUE(FALSE);
		Report.Print(this, g_lpszReportFilePath);
		return;
	}

	SaveBoardSN(Report, mapSlot);

	///<Load vector
	XT_EXPECT_EQ(nRetVal, 0);
	if (0 != nRetVal)
	{
		Report.AddTestItem("Load vector");
		Report.SetFailInfo("Load vector(%s) fail.", g_lpszVectorFilePath);
		mapSlot.clear();
		Report.Print(this, g_lpszReportFilePath);
		return;
	}
	nRetVal = CheckPPMUConnection(Report, mapSlot);
	XT_EXPECT_EQ(nRetVal, 0);
	if (0 != nRetVal)
	{
		Report.Print(this, g_lpszReportFilePath);
		dcm_CloseFile();
		return;
	}
	dcm.Connect("G_ALLPIN");

	USHORT usSiteNo = 0;
	char lpszPinName[32] = { 0 };
	USHORT usMeasureChannelCount = DCM_CHANNELS_PER_CONTROL / 2;
	BYTE bySlotNo = 0;
	USHORT usChannel = 0;
	const int nModeCount = 4;
	PPMUMode PMUMode[nModeCount] = { DCM_PPMU_FVMI, DCM_PPMU_FIMV, DCM_PPMU_FIMI, DCM_PPMU_FVMV };
	string strModeName[nModeCount] = { "FVMI", "FIMV", "FIMI", "FVMV" };
	const char lpszUnit[nModeCount][4] = { "A", "V", "A", "V" };

	double dSetValue[nModeCount] = { 4, 0.001, 0.003, 2.5 };
	double dExpectValue[nModeCount] = { 4e-3, 1, 0.003, 2.5 };

	const BYTE byTestTimes = 2;
	const char lpszTestPinGroup[byTestTimes][32] = { "G_FIMV", "G_FVMI" };
	double dMeasureValue = 0;
	for (int nModeIndex = 0; nModeIndex < nModeCount; ++nModeIndex)
	{
		Report.AddTestItem(strModeName[nModeIndex].c_str());
		Report.SetTestCondition("IRANGE: DCM_PPMUIRANGE_32MA");
		int nPinGroupIndex = 0;
		for (auto& lpszPinGroup : lpszTestPinGroup)
		{
			dcm.SetPPMU(lpszTestPinGroup[1 - nPinGroupIndex], DCM_PPMU_FVMI, 0, DCM_PPMUIRANGE_32MA);

			dcm.SetPPMU(lpszPinGroup, PMUMode[nModeIndex], dSetValue[nModeIndex], DCM_PPMUIRANGE_32MA);
			delay_us(150);
			dcm.PPMUMeasure(lpszPinGroup, 10, 10);

			for (auto& Slot : mapSlot)
			{
				for (int nControllerIndex = 0; nControllerIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++nControllerIndex)
				{
					usSiteNo = Slot.second + nControllerIndex;
					for (USHORT usChannelIndex = 0; usChannelIndex < usMeasureChannelCount; ++usChannelIndex)
					{
						USHORT usCurChannel = usChannelIndex / 2 * 4 + usChannelIndex % 2 + nPinGroupIndex * 2;
						sprintf_s(lpszPinName, sizeof(lpszPinName), "CH%d", usCurChannel);
						dMeasureValue = dcm.GetPPMUMeasResult(lpszPinName, usSiteNo);
						XT_EXPECT_REAL_EQ(dMeasureValue, dExpectValue[nModeIndex], 0.1);
						if (0.1 < fabs(dMeasureValue - dExpectValue[nModeIndex]))
						{
							dcm_GetPinGroupChannel(lpszPinName, usSiteNo, &bySlotNo, &usChannel, 1);
							Report.AddFailChannel(bySlotNo, usChannel, dExpectValue[nModeIndex], dMeasureValue, 6, lpszUnit[nModeIndex]);
						}
					}
				}
			}
			dcm.SetPPMU(lpszPinGroup, DCM_PPMU_FVMI, 0, DCM_PPMUIRANGE_32MA);
			++nPinGroupIndex;
		}
	}

	///<Check the parallel measurement
	Report.AddTestItem("Test parallel measurement");
	dcm_CloseFile();
	dcm_EnableAddPin(TRUE, TRUE);
	bySlotNo = mapSlot.begin()->first;
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

	dSetValue[0] = 5e-3;
	dcm.SetPPMU("G_FORCE", DCM_PPMU_FIMV, dSetValue[0], DCM_PPMUIRANGE_32MA);
	dcm.SetPPMU("G_MEAS", DCM_PPMU_FVMI, 0, DCM_PPMUIRANGE_32MA);
	dcm.PPMUMeasure("G_MEAS", 10, DCM_PPMU_10US);
	dExpectValue[0] = -dSetValue[0];
	const char* lpszPin[2] = { "CH0", "CH1" };
	for (USHORT usSiteNo = 0; usSiteNo < 2;++usSiteNo)
	{
		for (auto& PinName : lpszPin)
		{
			dMeasureValue = dcm.GetPPMUMeasResult(PinName, usSiteNo);
			XT_EXPECT_REAL_EQ(dMeasureValue, dExpectValue[0], 1e-4);
			if (1e-4 < fabs(dMeasureValue - dExpectValue[0]))
			{
				int nPinNo = dcm_GetPinNo(PinName);
				USHORT usChannel = 0;
				BYTE byCurSlot = dcm_GetPinSlotChannel(nPinNo, usSiteNo, usChannel);
				Report.AddFailChannel(bySlotNo, usChannel, dExpectValue[0] * 1e3, dMeasureValue * 1e3, 3, "mA");
			}
		}
	}
	dcm.Disconnect("G_ALLPIN");
	dcm_CloseFile();
	Report.Print(this, g_lpszReportFilePath);
	dcm_CloseFile();
}