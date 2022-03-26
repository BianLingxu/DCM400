#pragma once
/**
 * @file TestDCMTMUMeasureFunction.h
 * @brief Test the function of function TMUMeasure
 * @author Guangyun Wang
 * @date 2020/09/02
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controller Co., Ltd.
*/
#include "..\DCMTestMain.h"
XT_TEST(TMUFunctionTest, TestDCMTMUMeasureFunction)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, FUNCTION);
	int nRetVal = 0;
	CFuncReport Report(strFuncName.c_str(), "PMUFunctionTest");//Error message.

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
		Report.SaveAddtionMsg("Load vector(%s) fail.", g_lpszVectorFilePath);
		mapSlot.clear();
		Report.Print(this, g_lpszReportFilePath);
		return;
	}
	dcm.SetPinGroup("G_TMU", "CH0,CH4");
	dcm.SetTMUMatrix("CH0", DCM_ALLSITE, DCM_TMU1);
	dcm.SetTMUMatrix("CH4", DCM_ALLSITE, DCM_TMU2);

	const USHORT usTMUChannelCount = 2;
	USHORT usTMUChannel[usTMUChannelCount] = { 0,4 };

	BYTE byCurMeasMode = 0;
	UINT uCurSampleNum = 0;
	double dCurTimeout = 0;
	Report.AddTestItem("Measurement mode");

	for (BYTE byMeasMode = DCM_MEAS_FREQ_DUTY; byMeasMode <= DCM_MEAS_DELAY; ++byMeasMode)
	{
		dcm.TMUMeasure("G_TMU", (DCM_TMU_MEAS_MODE)byMeasMode, 10, 10);
		for (auto& Slot : mapSlot)
		{
			for (BYTE byControllerIndex = 0; byControllerIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++byControllerIndex)
			{
				for (auto usTMUCurChannel : usTMUChannel)
				{
					USHORT usChannel = byControllerIndex * DCM_CHANNELS_PER_CONTROL + usTMUCurChannel;
					dcm_GetTMUMeasure(Slot.first, usChannel, byCurMeasMode, uCurSampleNum, dCurTimeout);
					XT_EXPECT_EQ(byMeasMode, byCurMeasMode);
					if (byMeasMode != byCurMeasMode)
					{
						Report.SaveFailChannel(Slot.first, usChannel);
					}
				}
			}
		}
	}


	Report.AddTestItem("Sample number");

	const BYTE bySampleNumCount = 4;
	UINT uTestSampleNum[bySampleNumCount] = { 1, 100,200, 2046 };
	
	for (auto uSampleNum : uTestSampleNum)
	{
		dcm.TMUMeasure("G_TMU", DCM_MEAS_FREQ_DUTY, uSampleNum, 0);
		for (auto& Slot : mapSlot)
		{
			for (BYTE byControllerIndex = 0; byControllerIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++byControllerIndex)
			{
				for (USHORT usTMUChannelIndex = 0; usTMUChannelIndex < 2; ++usTMUChannelIndex)
				{
					USHORT usChannel = byControllerIndex * DCM_CHANNELS_PER_CONTROL + usTMUChannel[usTMUChannelIndex];
					dcm_GetTMUMeasure(Slot.first, usChannel, byCurMeasMode, uCurSampleNum, dCurTimeout);
					XT_EXPECT_EQ((int)uSampleNum, (int)uCurSampleNum);
					if (uSampleNum != uCurSampleNum)
					{
						Report.SaveFailChannel(Slot.first, usChannel);
						continue;
					}
				}
			}
		}
	}

	Report.AddTestItem("Timeout");
	const BYTE byTimeoutCount = 4;
	double dTestTimeout[byTimeoutCount] = { 1, 100,200, TMU_MAX_TIMEOUT * 1e-6 };

	for (auto dTimeout : dTestTimeout)
	{
		dcm.TMUMeasure("G_TMU", DCM_MEAS_FREQ_DUTY, 10, dTimeout);
		for (auto& Slot : mapSlot)
		{
			for (BYTE byControllerIndex = 0; byControllerIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++byControllerIndex)
			{
				for (USHORT usTMUChannelIndex = 0; usTMUChannelIndex < 2; ++usTMUChannelIndex)
				{
					USHORT usChannel = byControllerIndex * DCM_CHANNELS_PER_CONTROL + usTMUChannel[usTMUChannelIndex];
					dcm_GetTMUMeasure(Slot.first, usChannel, byCurMeasMode, uCurSampleNum, dCurTimeout);

					XT_EXPECT_REAL_EQ(dTimeout, dCurTimeout, 4e-6);
					if (4e-6 <= fabs(dTimeout - dCurTimeout))
					{
						Report.SaveFailChannel(Slot.first, usChannel);
					}
				}
			}
		}
	}

	Report.Print(this, g_lpszReportFilePath);
	dcm_CloseFile();
}