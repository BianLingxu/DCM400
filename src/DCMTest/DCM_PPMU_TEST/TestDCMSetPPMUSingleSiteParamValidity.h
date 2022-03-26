#pragma once
/**
 * @file TestDCMSetPPMUSingleSiteParamValidity.h
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controller Co., Ltd.
 * @author Guangyun Wang
 * @date 2020/07/16
 * @version v 1.0.0.0
 * @brief Test the parameter validity of SetPPMUSingleSite
 */
#include "..\DCMTestMain.h"
XT_TEST(PMUParamValidityTest, TestDCMSetPPMUSingleSiteParamValidity)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, PARAM_VADILITY);
	CErrorMSG errMsg(strFuncName.c_str(), "ParamValidityTest");//Error message.
	int nRetVal = 0;
	const int nPinGroupTestCount = 4;
	char* lpszTestPinGroup[nPinGroupTestCount] = { nullptr, "G_NOPIN", "G_ALLPIN", "CH0" };

	map<BYTE, USHORT> mapSlot;
	GetBoardInfo(mapSlot, g_lpszVectorFilePath, TRUE);

	if (0 == mapSlot.size())
	{
		//No board is inserted.
		XT_EXPECT_TRUE(FALSE);
		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("No board inserted!");
		errMsg.Print(this, g_lpszReportFilePath);
		return;
	}

	DWORD dwSplitData = 0;
	nRetVal = dcm.SetPPMU("FIRST_ST", DCM_PPMU_FVMV, 0);
	if (VECTOR_FILE_NOT_LOADED != nRetVal)
	{
		//Not warning when vector is not loaded.
		XT_EXPECT_EQ(VECTOR_FILE_NOT_LOADED, nRetVal);
		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("Not warning when vector is not loaded!");
	}
	//Load vector.
	dcm.LoadVectorFile(g_lpszVectorFilePath, FALSE);

	const int nValueTestCount = 5;
	double dTestValue[nValueTestCount] = { -3, -1.5, 0, 6, 7 };

	USHORT uTestSiteCount = dcm_GetVectorSiteCount() + 1;
	int nStringType = 0;
	//Defined pin group G_ALLPIN
	dcm.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");
	///<Test pin group
	dcm.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");
	int nExpectValue = 0;
	for (auto& Slot : mapSlot)
	{
		for (auto& lpszPinGroup : lpszTestPinGroup)
		{
			for (int nSiteID = 0; nSiteID < uTestSiteCount; ++nSiteID)
			{
				nRetVal = dcm.SetPPMUSingleSite(lpszPinGroup, nSiteID, DCM_PPMU_FVMV, 0, DCM_PPMUIRANGE_32MA);

				nExpectValue = CheckParamValidity(errMsg, mapSlot, nRetVal, lpszPinGroup, 1, nSiteID, 0);
				XT_EXPECT_EQ(nRetVal, nExpectValue);
			}
		}
	}
	USHORT usFirstValidSite = mapSlot.begin()->second;
	BYTE byPMUMode = 0;
	BYTE byIRange = 0;
	BYTE byTargeMode = 0;
	double dTargetVoltage = 0;
	double dRealVoltage = 0;
	BOOL bFV = TRUE;
	const BYTE byTestVoltageCount = 5;
	double dTestVoltage[byTestVoltageCount] = { PIN_LEVEL_MIN - 1, PIN_LEVEL_MIN, 0, PIN_LEVEL_MAX, PIN_LEVEL_MAX + 1 };
	const char lpszMode[4][32] = { "DCM_PPMU_FVMI","DCM_PPMU_FIMV","DCM_PPMU_FIMI","DCM_PPMU_FVMV" };
	double dSetValue = 0;
	for (BYTE byModeIndex = 0; byModeIndex < 5; ++byModeIndex)
	{
		for (BYTE bySetVoltageIndex = 0; bySetVoltageIndex < byTestVoltageCount; ++bySetVoltageIndex)
		{
			if (DCM_PPMU_FIMI == byModeIndex || DCM_PPMU_FIMV == byModeIndex)
			{
				bySetVoltageIndex = byTestVoltageCount;
				dSetValue = 0;
				bFV = FALSE;
			}
			else
			{
				dSetValue = dTestVoltage[bySetVoltageIndex];
				bFV = TRUE;
			}

			nRetVal = dcm.SetPPMUSingleSite("G_ALLPIN", usFirstValidSite, PPMUMode(byModeIndex), dSetValue, DCM_PPMUIRANGE_32MA);
			XT_EXPECT_EQ(nRetVal, 0);
			if (0 != nRetVal)
			{
				errMsg.SetErrorItem("Mode");
				errMsg.SaveErrorMsg("Unknown error");
				continue;
			}
			byTargeMode = byModeIndex;
			if (DCM_PPMU_FVMV < byModeIndex)
			{
				byTargeMode = DCM_PPMU_FVMV;
			}
			if (bFV)
			{
				if (PIN_LEVEL_MAX + EQUAL_ERROR < dSetValue)
				{
					dTargetVoltage = PIN_LEVEL_MAX;
				}
				else if (PIN_LEVEL_MIN - EQUAL_ERROR > dSetValue)
				{
					dTargetVoltage = PIN_LEVEL_MIN;
				}
				else
				{
					dTargetVoltage = dSetValue;
				}
			}
			else
			{
				dTargetVoltage = dSetValue;
			}
			dRealVoltage = dcm_GetPPMUSetValue(mapSlot.begin()->first, 0, byPMUMode, byIRange);
			XT_EXPECT_EQ(byPMUMode, byTargeMode);
			if (byPMUMode != byTargeMode)
			{
				errMsg.SetErrorItem("Mode");
				errMsg.SaveErrorMsg("The mode is %s, but the mode in hardware is %s", lpszMode[byTargeMode], lpszMode[byPMUMode]);
				continue;
			}
			XT_EXPECT_REAL_EQ(dRealVoltage, dTargetVoltage, 0.1);
			if (0.1 < fabs(dRealVoltage - dTargetVoltage))
			{
				errMsg.SetErrorItem("dSetValue");
				if (bFV)
				{
					errMsg.SaveErrorMsg("The mode is %s, set voltage is %.1f, the target voltage(%.1f) is not equal to input(%.1f) ",
						lpszMode[byTargeMode], dSetValue, dTargetVoltage, dRealVoltage);
				}
				else
				{
					errMsg.SaveErrorMsg("The mode is %s, set current is %.6f, the target current(%.6f) is not equal to input(%.6f) ",
						lpszMode[byTargeMode], dSetValue, dTargetVoltage, dRealVoltage);
				}
				errMsg.SaveErrorMsg("The mode is %s, but the mode in hardware is %s", lpszMode[byTargeMode], lpszMode[byPMUMode]);
			}
		}
	}
	double dbIRange[5] = { 2E-6, 20E-6, 200E-6, 2E-3, 32E-3 };

	const char lpszRange[DCM_PPMUIRANGE_32MA + 1][32] = { "DCM_PPMUIRANGE_2UA","DCM_PPMUIRANGE_20UA", "DCM_PPMUIRANGE_200UA", "DCM_PPMUIRANGE_2MA",	"DCM_PPMUIRANGE_32MA" };
	const int nTestValueCount = 6;
	double dTestCurrent[nTestValueCount] = { 0 };
	BYTE byTargetRange = 0;
	double byRealValue = 0;
	double dTargetCurrent = 0;

	for (BYTE byRangeIndex = 0; byRangeIndex <= DCM_PPMUIRANGE_32MA + 1; ++byRangeIndex)
	{
		byTargetRange = DCM_PPMUIRANGE_32MA >= byRangeIndex ? byRangeIndex : DCM_PPMUIRANGE_32MA;
		dTestCurrent[0] = -dbIRange[byTargetRange] - 10;
		dTestCurrent[1] = -dbIRange[byTargetRange];
		dTestCurrent[2] = 0;
		dTestCurrent[3] = dbIRange[byTargetRange] / 2;
		dTestCurrent[4] = dbIRange[byTargetRange];
		dTestCurrent[5] = dbIRange[byTargetRange] + 1;
		for (auto& dCurrent : dTestCurrent)
		{
			nRetVal = dcm.SetPPMUSingleSite("G_ALLPIN", usFirstValidSite, DCM_PPMU_FIMV, dCurrent, PPMUIRange(byRangeIndex));
			XT_EXPECT_EQ(nRetVal, 0);
			if (0 != nRetVal)
			{
				errMsg.SetErrorItem("Mode");
				errMsg.SaveErrorMsg("Unknown error");
				continue;
			}
			if (-dbIRange[byTargetRange] - EQUAL_ERROR > dCurrent)
			{
				dTargetCurrent = -dbIRange[byTargetRange];
			}
			else if (dbIRange[byTargetRange] + EQUAL_ERROR < dCurrent)
			{
				dTargetCurrent = dbIRange[byTargetRange];
			}
			else
			{
				dTargetCurrent = dCurrent;
			}

			byRealValue = dcm_GetPPMUSetValue(mapSlot.begin()->first, 0, byPMUMode, byIRange);
			XT_EXPECT_EQ(byIRange, byTargetRange);
			if (byIRange != byTargetRange)
			{
				errMsg.SetErrorItem("IRange");
				errMsg.SaveErrorMsg("Current range(%s) is not equal to input(%s)", lpszRange[byIRange], lpszRange[byTargetRange]);
				continue;
			}
			XT_EXPECT_REAL_EQ(byRealValue, dTargetCurrent, 1e-5);
			if (1e-5 < fabs(dTargetCurrent - byRealValue))
			{
				errMsg.SetErrorItem("dSetValue");
				errMsg.SaveErrorMsg("Current range(%f) is not equal to input(%f)", byRealValue, dTargetCurrent);
				continue;
			}
		}
	}


	const USHORT usInvalidSite = mapSlot.begin()->second;

	InvalidSite(usInvalidSite);

	nRetVal = dcm.SetPPMUSingleSite("CH2", usInvalidSite, DCM_PPMU_FVMV, 0, DCM_PPMUIRANGE_32MA);
	XT_EXPECT_EQ(nRetVal, SITE_INVALID);
	if (SITE_INVALID != nRetVal)
	{
		errMsg.AddNewError(VALUE_ERROR);
		errMsg.SaveErrorMsg("The SITE_%d is invalid, but the return value(%d) is not equal to SITE_INVALID(%d).", usInvalidSite + 1, nRetVal, SITE_INVALID);
	}

	RestoreSite();

	errMsg.Print(this, g_lpszReportFilePath);

	dcm_CloseFile();//Unload the vector file.
}