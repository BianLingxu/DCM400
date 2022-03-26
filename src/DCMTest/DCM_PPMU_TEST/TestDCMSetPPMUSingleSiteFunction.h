#pragma once
/**
 * @file TestDCMSetPPMUSingleSiteFunction.h
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controller Co., Ltd.
 * @author Guangyun Wang
 * @date 2020/07/16
 * @version v 1.0.0.0
 * @brief Test the funciont of SetPPMUSingleSite
 */
#include "..\DCMTestMain.h"
XT_TEST(PMUFunctionTest, TestDCMSetPPMUSingleSiteFunction)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, FUNCTION);
	int nRetVal = 0;
	CMeasurementFuncReport funcReport(strFuncName.c_str(), "PMUFunctionTest");//Error message.

	map<BYTE, USHORT> mapSlot;

	nRetVal = GetBoardInfo(mapSlot, g_lpszVectorFilePath);

	if (0 == mapSlot.size())
	{
		///<No board is inserted
		XT_EXPECT_TRUE(FALSE);
		funcReport.Print(this, g_lpszReportFilePath);
		return;
	}

	SaveBoardSN(funcReport, mapSlot);

	///<Load vector
	XT_EXPECT_EQ(nRetVal, 0);
	if (0 != nRetVal)
	{
		funcReport.AddTestItem("Load vector");
		funcReport.SetFailInfo("Load vector(%s) fail.", g_lpszVectorFilePath);
		mapSlot.clear();
		funcReport.Print(this, g_lpszReportFilePath);
		return;
	}
	dcm.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");
	dcm.Disconnect("G_ALLPIN");
	const int nModeCount = 4;
	PPMUMode PMUMode[nModeCount] = { DCM_PPMU_FVMI, DCM_PPMU_FIMV, DCM_PPMU_FIMI, DCM_PPMU_FVMV };
	string strModeName[nModeCount] = { "FVMI", "FIMV", "FIMI", "FVMV" };

	double dSetValue[nModeCount] = { 5, 0.02, 0.03, 2.5 };
	string strUnit[nModeCount] = { "V", "A", "A", "V" };
	USHORT ausChannel[DCM_CHANNELS_PER_CONTROL] = { 0 };
	BYTE bySlot[DCM_CHANNELS_PER_CONTROL] = { 0 };
	int nChannelCount = 0;
	double dRealValue = 0;
	BYTE byMode = 0;
	BYTE byRange = 0;

	USHORT usSetSite = mapSlot.begin()->second;
	USHORT usSiteNo = 0;
	double dExpectValue = 0;

	for (int nModeIndex = 0; nModeIndex < nModeCount; ++nModeIndex)
	{
		funcReport.AddTestItem(strModeName[nModeIndex].c_str());
		funcReport.SetTestCondition("IRANGE: DCM_PPMUIRANGE_32MA");
		dcm.SetPPMU("G_ALLPIN", PMUMode[nModeIndex], 0, DCM_PPMUIRANGE_32MA);
		dcm.SetPPMUSingleSite("G_ALLPIN", usSetSite, PMUMode[nModeIndex], dSetValue[nModeIndex], DCM_PPMUIRANGE_32MA);
		for (auto& Slot : mapSlot)
		{
			for (int nControllerIndex = 0; nControllerIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++nControllerIndex)
			{
				usSiteNo = Slot.second + nControllerIndex;
				nChannelCount = dcm_GetPinGroupChannel("G_ALLPIN", usSiteNo, bySlot, ausChannel, DCM_CHANNELS_PER_CONTROL);
				if (usSetSite == usSiteNo)
				{
					dExpectValue = dSetValue[nModeIndex];
				}
				else
				{
					dExpectValue = 0;
				}
				for (USHORT usChannelIndex = 0; usChannelIndex < nChannelCount; ++usChannelIndex)
				{
					dRealValue = dcm_GetPPMUSetValue(bySlot[usChannelIndex], ausChannel[usChannelIndex], byMode, byRange);
					XT_EXPECT_REAL_EQ(dRealValue, dExpectValue, 0.1);
					if (0.1 < fabs(dExpectValue - dRealValue))
					{
						///<The set value is not equal to set expect
						funcReport.AddFailChannel(bySlot[usChannelIndex], ausChannel[usChannelIndex], dExpectValue, dRealValue, 1, strUnit[nModeIndex].c_str());
					}
					XT_EXPECT_EQ(byMode, PMUMode[nModeIndex]);
					if (byMode != PMUMode[nModeIndex])
					{
						///<The mode is right
						funcReport.AddFailChannel(bySlot[usChannelIndex], ausChannel[usChannelIndex], strModeName[nModeIndex].c_str(), strModeName[nModeIndex].c_str(), -1);
					}
				}
			}
		}
	}

	funcReport.Print(this, g_lpszReportFilePath);
	dcm_CloseFile();
}