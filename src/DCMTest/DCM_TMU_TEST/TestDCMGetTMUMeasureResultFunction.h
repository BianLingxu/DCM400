#pragma once
/**
 * @file TestDCMGetTMUMeasureResultFunction.h
 * @brief Test the function of function GetTMUMeasureResult
 * @author Guangyun Wang
 * @date 2020/09/02
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controller Co., Ltd.
*/
#include "..\DCMTestMain.h"
XT_TEST(TMUFunctionTest, TestDCMGetTMUMeasureResultFunction)
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
	dcm.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");
	dcm.SetPinGroup("G_TMU", "CH0,CH4");
	dcm.SetPinGroup("G_RUNPIN", "CH0,CH2,CH4,CH6");

	const USHORT usTMUChannelCount = 2;
	USHORT usTMUChannel[usTMUChannelCount] = { 0,4 };

	double dBackupEdge[EDGE_COUNT] = { 0 };
	WAVE_FORMAT BackupWaveFormat = WAVE_FORMAT::NRZ;
	IO_FORMAT BackupIoFormat = IO_FORMAT::NRZ;
	COMPARE_MODE BackupCompareMode = COMPARE_MODE::EDGE;

	double dBackupPeriod = dcm_GetTimeSetPeriod(mapSlot.begin()->first, 0, 0);
	dcm_GetEdge(mapSlot.begin()->first, 0, 0, dBackupEdge, BackupWaveFormat, BackupIoFormat, BackupCompareMode);

	double dPeriod = 100;
	dcm.Connect("G_RUNPIN");
	dcm.SetPinLevel("G_RUNPIN", 3, 0, 1.5, 0.8);
	dcm.SetPeriod("0", dPeriod);
	nRetVal = dcm.SetEdge("G_RUNPIN", "0", DCM_DTFT_RO, DCM_IO_NRZ, 0, dPeriod / 2, 0, dPeriod / 2, dPeriod * 3 / 4);
	
	nRetVal = dcm.SetTMUMatrix("CH0", DCM_ALLSITE, DCM_TMU1);
	nRetVal = dcm.SetTMUMatrix("CH4", DCM_ALLSITE, DCM_TMU2);
	nRetVal = dcm.SetTMUParam("G_TMU", DCM_ALLSITE, DCM_NEG, 0, 0);
	nRetVal = dcm.TMUMeasure("G_TMU", DCM_MEAS_FREQ_DUTY, 1, 10);

	dcm.RunVectorWithGroup("G_RUNPIN", "TMU_ST", "TMU_SP");

	double dMeasurePeriod = 0;
	double dDuty = 50;
	char lpszPinName[32] = { 0 };

	const BYTE byMeasureTypeCount = 3;
	DCM_TMU_MEAS_TYPE MeasType[byMeasureTypeCount] = { DCM_FREQ, DCM_HIGH_DUTY, DCM_LOW_DUTY };
	char* lpszMeasureType[byMeasureTypeCount] = {"Frequency", "High duty", "Low duty"};
	char* lpszUnits[byMeasureTypeCount] = {"ns", "", ""};
	double dResolution[byMeasureTypeCount] = {5, 3, 3};
	double dGain[byMeasureTypeCount] = {1e6, 1, 1};
	double dTargetValue = 0;
	int nTypeIndex = 0;
	for (auto MeasureType : MeasType)
	{
		Report.AddTestItem(lpszMeasureType[nTypeIndex]);
		dTargetValue = dPeriod;
		if (0 != nTypeIndex)
		{
			dTargetValue = dDuty;
		}
		for (auto& Slot : mapSlot)
		{
			for (BYTE byControllerIndex = 0; byControllerIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++byControllerIndex)
			{
				for (auto usChannel : usTMUChannel)
				{
					sprintf_s(lpszPinName, sizeof(lpszPinName), "CH%d", usChannel);
					USHORT usBoardChannel = byControllerIndex * DCM_CHANNELS_PER_CONTROL + usChannel;
					double dMeasResult = dcm.GetTMUMeasureResult(lpszPinName, Slot.second + byControllerIndex, MeasureType);
					if (-EQUAL_ERROR < dMeasResult)
					{
						if (0 == nTypeIndex)
						{
							dMeasResult = 1 / dMeasResult;
						}
						dMeasResult *= dGain[nTypeIndex];
					}
					XT_EXPECT_REAL_EQ(dMeasResult, dTargetValue, dResolution[nTypeIndex]);
					if (dResolution[nTypeIndex] < fabs(dMeasResult - dTargetValue))
					{
						Report.AddFailChannel(Slot.first, usBoardChannel, dTargetValue, dMeasResult, dResolution[nTypeIndex], lpszUnits[nTypeIndex]);
					}
				}
			}
		}
		++nTypeIndex;
	}

	dcm.SetPeriod("0", dBackupPeriod);
	dcm.SetEdge("G_RUNPIN", "0", (DataFormat)BackupWaveFormat, (IOFormat)BackupIoFormat, dBackupEdge[0], dBackupEdge[1], dBackupEdge[2], dBackupEdge[3], dBackupEdge[4]);

	dcm.Disconnect("G_RUNPIN");

	Report.Print(this, g_lpszReportFilePath);
	dcm_CloseFile();
}