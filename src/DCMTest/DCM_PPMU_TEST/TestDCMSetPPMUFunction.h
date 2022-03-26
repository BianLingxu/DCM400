#pragma once
/**
 * @file TestDCMSetPPMUFunction.h
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controller Co., Ltd.
 * @author Guangyun Wang
 * @date 2020/07/15
 * @version v 1.0.0.0
 * @brief Test the funciont of SetPPMU
 */
#include "..\DCMTestMain.h"
XT_TEST(PMUFunctionTest, TestDCMSetPPMUFunction)
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
	PPMUMode TestPMUMode[nModeCount] = { DCM_PPMU_FVMI, DCM_PPMU_FIMV, DCM_PPMU_FIMI, DCM_PPMU_FVMV };
	string strModeName[nModeCount] = { "FVMI", "FIMV", "FIMI", "FVMV" };

	double dSetValue[nModeCount] = { 5, 0.02, 0.03, 2.5 };
	string strUnit[nModeCount] = {"V", "A", "A", "V"};
	USHORT ausChannel[DCM_CHANNELS_PER_CONTROL] = { 0 };
	BYTE bySlot[DCM_CHANNELS_PER_CONTROL] = { 0 };
	int nChannelCount = 0;
	double dRealValue = 0;
	BYTE byMode = 0;
	BYTE byRange = 0;
	int nModeIndex = 0;
	for (auto PMUMode : TestPMUMode)
	{
		funcReport.AddTestItem(strModeName[nModeIndex].c_str());
		funcReport.SetTestCondition("IRANGE: DCM_PPMUIRANGE_32MA");
		dcm.SetPPMU("G_ALLPIN", PMUMode, dSetValue[nModeIndex], DCM_PPMUIRANGE_32MA);
		for (auto& Slot : mapSlot)
		{
			for (int nControllerIndex = 0; nControllerIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++nControllerIndex)
			{
				nChannelCount = dcm_GetPinGroupChannel("G_ALLPIN", Slot.second + nControllerIndex, bySlot, ausChannel, DCM_CHANNELS_PER_CONTROL);
				for (USHORT usChannelIndex = 0; usChannelIndex < nChannelCount; ++usChannelIndex)
				{
					dRealValue = dcm_GetPPMUSetValue(bySlot[usChannelIndex], ausChannel[usChannelIndex], byMode, byRange);
					XT_EXPECT_REAL_EQ(dRealValue, dSetValue[nModeIndex], 0.1);
					if (fabs(dSetValue[nModeIndex] - dRealValue) > 0.1)
					{
						///<The set value is not equal to set expect
						funcReport.AddFailChannel(bySlot[usChannelIndex], ausChannel[usChannelIndex], dSetValue[nModeIndex], dRealValue, 1, strUnit[nModeIndex].c_str());
					}
					XT_EXPECT_EQ(byMode, PMUMode);
					if (byMode != PMUMode)
					{
						///<The mode is not right
						funcReport.AddFailChannel(bySlot[usChannelIndex], ausChannel[usChannelIndex], strModeName[nModeIndex].c_str(), strModeName[byMode].c_str(), -1);
					}
				}
			}
		}
		++nModeIndex;
	}
	char lpszPinName[32] = { 0 };

	///<Check asymmetric bind
// 	funcReport.AddTestItem("Check asymmetric bind");
// 	const USHORT usCheckChannel = 3;
// 	sprintf_s(lpszPinName, sizeof(lpszPinName), "CH%d", usCheckChannel);
// 	dcm.SetPPMU(lpszPinName, DCM_PPMU_FVMV, 0, DCM_PPMUIRANGE_2UA);
// 	InvalidSite(0);
// 	dcm.SetPPMU(lpszPinName, DCM_PPMU_FVMV, 0, DCM_PPMUIRANGE_2UA);



	funcReport.AddTestItem("Check Invalid Site");
	const USHORT usInvalidSite = mapSlot.begin()->second;

	double dInvalidSiteValue = 0;
	double dValidSiteValue = 0.5;
	PPMUMode InvalidSiteMode = DCM_PPMU_FVMV;
	PPMUMode ValidSiteMode = DCM_PPMU_FVMI;
	PPMUIRange InvalidSiteRange = DCM_PPMUIRANGE_2UA;
	PPMUIRange ValidSiteRange = DCM_PPMUIRANGE_20UA;
	double dCurSetValue = 0;
	const int nTestChannel = 2;

	double dTargetValue = 0;
	int byTargetMode = 0;
	int byTargetIRange = 0;

	sprintf_s(lpszPinName, sizeof(lpszPinName), "CH%d", nTestChannel);

	dcm.SetPPMU(lpszPinName, InvalidSiteMode, dInvalidSiteValue, InvalidSiteRange);
	InvalidSite(usInvalidSite);
	dcm.SetPPMU(lpszPinName, ValidSiteMode, dValidSiteValue,ValidSiteRange);
	USHORT usSiteNo = 0;
	USHORT usChannel = 0;
	for (auto& Slot : mapSlot)
	{
		usSiteNo = Slot.second;
		for (BYTE byController = 0; byController < DCM_MAX_CONTROLLERS_PRE_BOARD;++byController, ++usSiteNo)
		{
			if (usInvalidSite == usSiteNo)
			{
				dTargetValue = dInvalidSiteValue;
				byTargetIRange = InvalidSiteRange;
				byTargetMode = InvalidSiteMode;
			}
			else
			{
				dTargetValue = dValidSiteValue;
				byTargetIRange = ValidSiteRange;
				byTargetMode = ValidSiteMode;
			}
			usChannel = byController * DCM_CHANNELS_PER_CONTROL + nTestChannel;
			dCurSetValue = dcm_GetPPMUSetValue(Slot.first, usChannel, byMode, byRange);
			XT_EXPECT_EQ((int)byMode, byTargetMode);
			XT_EXPECT_EQ((int)byRange, byTargetIRange);
			XT_EXPECT_REAL_EQ(dCurSetValue, dTargetValue, 1e-3);
			
			if (byMode != byTargetMode)
			{
				funcReport.AddFailChannel(Slot.first, usChannel, (int)byTargetMode, (int)byMode, 0);
			}
			if (byRange != byTargetIRange)
			{
				funcReport.AddFailChannel(Slot.first, usChannel, (int)byTargetIRange, (int)byRange, 0);
			}
			if (1e-3 <= fabs(dCurSetValue - dTargetValue))
			{
				if (usInvalidSite == usSiteNo)
				{
					funcReport.AddFailChannel(Slot.first, usChannel, dTargetValue, dCurSetValue, 6, "V", "Invalid SITE_%d", usInvalidSite + 1);
				}
				else
				{
					funcReport.AddFailChannel(Slot.first, usChannel, dTargetValue, dCurSetValue, 6, "A", "Valid SITE_%d", usSiteNo + 1);
				}
			}
		}
	}
	RestoreSite();


	funcReport.Print(this, g_lpszReportFilePath);
	dcm_CloseFile();
}