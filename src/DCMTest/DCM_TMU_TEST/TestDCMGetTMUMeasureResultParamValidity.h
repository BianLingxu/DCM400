#pragma once
/**
 * @file TestDCMGetTMUMeasureResultParamValidity.h
 * @brief Test the parameter validity of function GetTMUMeasureResult
 * @author Guangyun Wang
 * @date 2020/09/01
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controller Co., Ltd.
*/
#include "..\DCMTestMain.h"
XT_TEST(TMUParamValidityTest, TestDCMGetTMUMeasureResultParamValidity)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, PARAM_VADILITY);
	CErrorMSG errMsg(strFuncName.c_str(), "ParamValidityTest");//Error message.

	double dMeasResult = 0;

	dMeasResult = dcm.GetTMUMeasureResult("CH0", 0, DCM_FREQ);

	XT_EXPECT_REAL_EQ(dMeasResult, TMU_ERROR, 0.1);
	if (0.1 < fabs(TMU_ERROR - dMeasResult))
	{
		///<Not warning when vector is not loaded
		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("No warning when vector is not loaded!");
	}

	map<BYTE, USHORT> mapSlot;
	int nRetVal = GetBoardInfo(mapSlot, g_lpszVectorFilePath);
	if (0 == mapSlot.size())
	{
		///<No board is inserted.
		XT_EXPECT_TRUE(FALSE);
		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("No board inserted!");
		errMsg.Print(this, g_lpszReportFilePath);
		return;
	}

	XT_EXPECT_EQ(nRetVal, 0);
	if (0 != nRetVal)
	{
		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("Load vector(%s) fail.", g_lpszVectorFilePath);
		mapSlot.clear();
		errMsg.Print(this, g_lpszReportFilePath);
		return;
	}

	USHORT usValidSiteCount = dcm_GetVectorSiteCount();

	dcm.SetPinGroup("G_TMU", "CH0,CH4");

	const BYTE byTestPinNameCount = 4;
	char* lpszTestPinName[byTestPinNameCount] = { nullptr, "G_UNDEFINED", "CH3","CH4" };
	BOOL bPinNameValidity[byTestPinNameCount] = { FALSE, FALSE, FALSE, TRUE };

	dcm.SetTMUMatrix("CH0", DCM_ALLSITE, DCM_TMU1);
	dcm.SetTMUMatrix("CH4", DCM_ALLSITE, DCM_TMU2);
	dcm.SetTMUParam("G_TMU", DCM_ALLSITE, DCM_POS, 0, 0);
	dcm.TMUMeasure("G_TMU", DCM_MEAS_FREQ_DUTY, 0, 0);
	DWORD dwSiteStatusBackup = STSGetsSiteStatus();

	USHORT usInvalidSite = mapSlot.begin()->second + 1;

	DWORD dwCurSiteStatus = dwSiteStatusBackup & (~(1 << usInvalidSite));
	StsSetSiteStatus(dwCurSiteStatus);

	USHORT usTestSiteCount = usValidSiteCount + 1;
	Sleep(1000);
	int nPinNameIndex = 0;
	for (auto& lpszPinName : lpszTestPinName)
	{
		for (USHORT usSiteNo = 0; usSiteNo < usTestSiteCount; ++usSiteNo)
		{
			dMeasResult = dcm.GetTMUMeasureResult(lpszPinName, usSiteNo, DCM_FREQ);

			if (nullptr != lpszPinName && usValidSiteCount <= usSiteNo)
			{
				XT_EXPECT_REAL_EQ(dMeasResult, TMU_ERROR, 0.1);
				if (0.1 < fabs( TMU_ERROR - dMeasResult))
				{
					errMsg.AddNewError(STRING_ERROR_MSG);
					errMsg.SaveErrorMsg("The site number(%d) is over range(%d), but the return value(%.0f) is not equal to TMU_ERROR(%.0f)",
						usSiteNo, usValidSiteCount, dMeasResult, SITE_ERROR);
				}
				continue;
			}
			int nStringType = dcm_GetStringType(lpszPinName);
			if (0 != nStringType)
			{
				XT_EXPECT_REAL_EQ(dMeasResult, TMU_ERROR, 0.1);
				if (0.1 < fabs(TMU_ERROR - dMeasResult))
				{
					errMsg.AddNewError(STRING_ERROR_MSG);
					if (nullptr == lpszPinName)
					{
						errMsg.SaveErrorMsg("The pin is nullptr, but the return value(%.0f) is not equal to TMU_ERROR(%.0f)", dMeasResult, TMU_ERROR);
					}
					else
					{
						errMsg.SaveErrorMsg("The pin name(%s) is not defined, but the return value(%.0f) is not equal to TMU_ERROR(%.0f)", lpszPinName, dMeasResult, TMU_ERROR);
					}
				}
				continue;
			}
			///<Check site
			BOOL bBoardExisted = FALSE;
			for (auto& Slot : mapSlot)
			{
				if (usSiteNo >= Slot.second && usSiteNo < Slot.second + DCM_MAX_CONTROLLERS_PRE_BOARD)
				{
					bBoardExisted = TRUE;
					break;
				}
			}
			if (!bBoardExisted)
			{
				XT_EXPECT_REAL_EQ(dMeasResult, TMU_ERROR, 0.1);
				if (0.1 < fabs(TMU_ERROR - dMeasResult))
				{
					errMsg.AddNewError(STRING_ERROR_MSG);
					errMsg.SaveErrorMsg("The board used in SITE_%d is not existed, but the return value(%.0f) is not equal to TMU_ERROR(%.0f)", usSiteNo + 1, dMeasResult, TMU_ERROR);
				}
				continue;
			}
			else if (usInvalidSite == usSiteNo)
			{
				XT_EXPECT_REAL_EQ(dMeasResult, TMU_ERROR, 0.1);
				if (0.1 < fabs(TMU_ERROR - dMeasResult))
				{
					errMsg.AddNewError(STRING_ERROR_MSG);
					errMsg.SaveErrorMsg("The SITE_%d is invalid, but the return value(%.0f) is not equal to TMU_ERROR(%.0f)",
						usSiteNo + 1, dMeasResult, TMU_ERROR);
				}
				continue;
			}

			if (!bPinNameValidity[nPinNameIndex])
			{
				XT_EXPECT_REAL_EQ(dMeasResult, TMU_ERROR, 0.1);
				if (0.1 < fabs(TMU_ERROR - dMeasResult))
				{
					errMsg.AddNewError(STRING_ERROR_MSG);
					errMsg.SaveErrorMsg("The channel in pin group(%s) is not connected to TMU unit, but the return value(%.0f) is not equal to TMU_ERROR(%.0f)",
						lpszPinName, dMeasResult, TMU_ERROR);
				}
				continue;
			}
			continue;
		}
		++nPinNameIndex;
	}

	StsSetSiteStatus(dwSiteStatusBackup);
	

	errMsg.Print(this, g_lpszReportFilePath);


	dcm_CloseFile();//Unload the vector file.
}