#pragma once
/**
 * @file TestDCMTMUMeasureParamValidity.h
 * @brief Test the parameter validity of function TMUMeasure
 * @author Guangyun Wang
 * @date 2020/09/01
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controller Co., Ltd.
*/
#include "..\DCMTestMain.h"
XT_TEST(TMUParamValidityTest, TestDCMTMUMeasureParamValidity)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, PARAM_VADILITY);
	CErrorMSG errMsg(strFuncName.c_str(), "ParamValidityTest");//Error message.
	int nRetVal = 0;

	nRetVal = dcm.TMUMeasure("CH0", DCM_MEAS_FREQ_DUTY, 10, 10);
	XT_EXPECT_EQ(VECTOR_FILE_NOT_LOADED, nRetVal);
	if (VECTOR_FILE_NOT_LOADED != nRetVal)
	{
		///<Not warning when vector is not loaded
		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("No warning when vector is not loaded!");
	}

	map<BYTE, USHORT> mapSlot;
	nRetVal = GetBoardInfo(mapSlot, g_lpszVectorFilePath);
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
	dcm.SetPinGroup("G_NOTTMU", "CH1,CH3");

	const BYTE byTestPinGroupCount = 4;
	char* lpszTestPinGroup[byTestPinGroupCount] = { nullptr, "G_UNDEFINED", "G_NOTTMU","G_TMU" };
	BOOL bPinGroupValidity[byTestPinGroupCount] = { FALSE, FALSE, FALSE, TRUE };

	dcm.SetTMUMatrix("CH0", DCM_ALLSITE, DCM_TMU1);
	dcm.SetTMUMatrix("CH4", DCM_ALLSITE, DCM_TMU2);
	dcm.SetTMUParam("G_TMU", DCM_ALLSITE, DCM_POS, 0, 0);

	int nPinGroupIndex = 0;
	for (auto& lpszPinGroup : lpszTestPinGroup)
	{
		nRetVal = dcm.TMUMeasure(lpszPinGroup, DCM_MEAS_DELAY, 0, 0);
		int nStringType = dcm_GetStringType(lpszPinGroup);
		if (0 != nStringType && 1 != nStringType)
		{
			XT_EXPECT_EQ(nRetVal, PIN_GROUP_ERROR);
			if (PIN_GROUP_ERROR != nRetVal)
			{
				errMsg.AddNewError(STRING_ERROR_MSG);
				if (nullptr == lpszPinGroup)
				{
					errMsg.SaveErrorMsg("The pin group is nullptr, but the return value(%d) is not equal to PIN_GROUP_ERROR(%d)", nRetVal, PIN_GROUP_ERROR);
				}
				else
				{
					errMsg.SaveErrorMsg("The pin group(%s) is not defined, but the return value(%d) is not equal to PIN_GROUP_ERROR(%d)", lpszPinGroup, nRetVal, PIN_GROUP_ERROR);
				}
			}
		}
		else if (!bPinGroupValidity[nPinGroupIndex])
		{
			XT_EXPECT_EQ(nRetVal, TMU_CHANNEL_NOT_CONNECT);
			if (TMU_CHANNEL_NOT_CONNECT != nRetVal)
			{
				errMsg.AddNewError(STRING_ERROR_MSG);
				errMsg.SaveErrorMsg("The channel in pin group(%s) is not connected to TMU unit, but the return value(%d) is not equal to TMU_CHANNEL_NOT_CONNECT(%d)",
					lpszPinGroup, nRetVal, TMU_CHANNEL_NOT_CONNECT);
			}
		}
		else
		{
			XT_EXPECT_EQ(nRetVal, 0);
			if (0 != nRetVal)
			{
				errMsg.AddNewError(STRING_ERROR_MSG);
				errMsg.SaveErrorMsg("The channel in pin group(%s) is right, but the return value(%d) is not equal to 0",
					lpszPinGroup, nRetVal);
			}
		}
		++nPinGroupIndex;
	}

	const BYTE byTestSampleNumCount = 4;
	UINT uTestSampleNum[byTestSampleNumCount] = { 0, 10,2046, 2047 };
	const BYTE byTestHoldOffNumCount = 4;
	double dMaxTimeout = TMU_MAX_TIMEOUT * 1e-6;
	double dTestTimeout[byTestHoldOffNumCount] = { 0, 10, dMaxTimeout, dMaxTimeout + 1 };

	for (auto uSampleNum : uTestSampleNum)
	{
		for (auto dTimeout : dTestTimeout)
		{
			nRetVal = dcm.TMUMeasure("G_TMU", DCM_MEAS_FREQ_DUTY, uSampleNum, dTimeout);
			if (2046 < uSampleNum)
			{
				BYTE byMeasMode = 0;
				UINT uCurSampleNumber = 0;
				double dTimeout = 0;
				dcm_GetTMUMeasure(mapSlot.begin()->first, 0, byMeasMode, uSampleNum, dTimeout);
				XT_EXPECT_EQ((int)uSampleNum, 2046);
				if (2046 != uSampleNum)
				{
					errMsg.AddNewError(STRING_ERROR_MSG);
					errMsg.SaveErrorMsg("The sample number(%d) is over range[%d,%d], but the set value(%d) is not equal to %d",
						uSampleNum, 0, 2046, uSampleNum, 2046);
				}
				continue;
			}
			else if (dMaxTimeout < dTimeout)
			{
				BYTE byMeasMode = 0;
				UINT uCurSampleNumber = 0;
				double dCurTimeout = 0;
				dcm_GetTMUMeasure(mapSlot.begin()->first, 0, byMeasMode, uSampleNum, dCurTimeout);
				XT_EXPECT_REAL_EQ(dCurTimeout, dMaxTimeout, 2e-6);
				if (2e-6 < fabs(dMaxTimeout - dCurTimeout))
				{
					errMsg.AddNewError(STRING_ERROR_MSG);
					errMsg.SaveErrorMsg("The timeout(%.6f) is over range[%d,%d], but the set value(%d) is not equal to %d",
						dTimeout, 0, dMaxTimeout, dCurTimeout, dMaxTimeout);
				}
			}
			else
			{
				XT_EXPECT_EQ(nRetVal, 0);
				if (0 != nRetVal)
				{
					errMsg.AddNewError(STRING_ERROR_MSG);
					errMsg.SaveErrorMsg("The sample number(%d) and timeout(%.6f)is in range, but the return value(%d) is not equal to 0",
						uSampleNum, dTimeout, nRetVal);
					continue;
				}
			}
		}
	}

	errMsg.Print(this, g_lpszReportFilePath);


	dcm_CloseFile();//Unload the vector file.
}