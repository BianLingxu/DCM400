#pragma once
/**
 * @file TestDCMPMUMeasureParamValidity.h
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controller Co., Ltd.
 * @author Guangyun Wang
 * @date 2020/11/26
 * @version v 1.0.0.0
 * @brief Test the parameter validity of PMUMeasure
 */
#include "..\DCMTestMain.h"
XT_TEST(PMUParamValidityTest, TestDCMPPMUMeasureParamValidity)
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
			nRetVal = dcm.PPMUMeasure(lpszPinGroup, 10, 10);
			nExpectValue = CheckParamValidity(errMsg, mapSlot, nRetVal, lpszPinGroup, 1, Slot.second, 0);
			XT_EXPECT_EQ(nRetVal, nExpectValue);
		}
	}

	const UINT uTestSampleTimes[5] = { 10, 1, 100, 512, 513 };
	const UINT uTargetSampleTimes[5] = { 10, 1, 100, 512, 512 };
	const double dTestSamplePeriod[16] = {-1, 0, 5,8,10,15,20,35,40,60,80,120,160,240,320,500 };
	const double dTargetSamplePeriod[16] = {5, 5, 5,10,10,20,20,40,40,80,80,160,160,320,320,320 };

	char lpszPinName[32] = { 0 };
	UINT uActualSampleTimes = 0;
	double dActualSamplePeriod = 0;
	int uSampleTimesIndex = 0;
	int uSamplePeriodIndex = 0;
	UINT uCurTargetSampeTimes = 0;
	double dCurTargetSamplePeriod = 0;
	BOOL bFail = FALSE;
	for (auto uSampleTimes : uTestSampleTimes)
	{
		uSamplePeriodIndex = 0;
		for (auto dSamplePeriod : dTestSamplePeriod)
		{
			uCurTargetSampeTimes = uTargetSampleTimes[uSampleTimesIndex];
			dCurTargetSamplePeriod = dTargetSamplePeriod[uSamplePeriodIndex];
			dcm.SetPPMU("G_ALLPIN", DCM_PPMU_FVMV, 0);
			nRetVal = dcm.PPMUMeasure("G_ALLPIN", uSampleTimes, dSamplePeriod);
			for (auto Slot : mapSlot)
			{
				for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL;++usChannel)
				{
					sprintf_s(lpszPinName, sizeof(lpszPinName), "CH%d", usChannel);
					USHORT usPinNo = dcm_GetPinNo(lpszPinName);
					for (BYTE byControllerIndex = 0; byControllerIndex < DCM_MAX_CONTROLLERS_PRE_BOARD;++byControllerIndex)
					{
						USHORT usSiteNo = Slot.second + byControllerIndex;
						uActualSampleTimes = dcm_getSampleTimes(usPinNo, usSiteNo);
						dActualSamplePeriod = dcm_getSampleInterval(usPinNo, usSiteNo);
						XT_EXPECT_EQ(uActualSampleTimes, uCurTargetSampeTimes);
						XT_EXPECT_REAL_EQ(dActualSamplePeriod, dCurTargetSamplePeriod, 1e-6);
						if (uActualSampleTimes != uCurTargetSampeTimes)
						{
							if (bFail)
							{
								errMsg.AddNewError(STRING_ERROR_MSG);
								bFail = FALSE;
							}
							errMsg.SaveErrorMsg("The set sample times(%d), but the actual sample times(%d) is not equal to target sample times(%d).", 
								uSampleTimes, uActualSampleTimes, uCurTargetSampeTimes);
							errMsg.SetErrorItem(lpszPinName, "uSampleTimes", TRUE, Slot.first, usChannel + DCM_CHANNELS_PER_CONTROL * byControllerIndex, VALUE_NOT_EQUAL);
						}
						if (1e-6 < fabs(dActualSamplePeriod - dCurTargetSamplePeriod))
						{
							if (bFail)
							{
								errMsg.AddNewError(STRING_ERROR_MSG);
								bFail = FALSE;
							}
							errMsg.SaveErrorMsg("The set sample period(%.0f), but the actual sample times(%.0f) is not equal to target sample times(%.0f).", 
								dSamplePeriod, dActualSamplePeriod, dCurTargetSamplePeriod);
							errMsg.SetErrorItem(lpszPinName, "uSampleTimes", TRUE, Slot.first, usChannel + DCM_CHANNELS_PER_CONTROL * byControllerIndex, VALUE_NOT_EQUAL);
						}
					}
				}
			}
			++uSamplePeriodIndex;
		}
		++uSampleTimesIndex;
	}

	double dSwitchTestPeriod[7] = { 5.0, 10.0, 20.0, 40.0, 80.0, 160.0, 320.0 };
	bFail = FALSE;
	for (auto dFirstPeriod : dSwitchTestPeriod)
	{
		for (auto dSecondPeriod : dSwitchTestPeriod)
		{
			dcm.PPMUMeasure("G_ALLPIN", 1, dFirstPeriod);
			dcm.PPMUMeasure("G_ALLPIN", 1, dSecondPeriod);

			auto iterSlot = mapSlot.begin();
			USHORT usSiteNo = iterSlot->second;

			sprintf_s(lpszPinName, sizeof(lpszPinName), "CH%d", 0);
			USHORT usPinNo = dcm_GetPinNo(lpszPinName);

			dActualSamplePeriod = dcm_getSampleInterval(usPinNo, usSiteNo);
			XT_EXPECT_REAL_EQ(dActualSamplePeriod, dSecondPeriod, 1e-6);

			if (1e-6 < fabs(dActualSamplePeriod - dSecondPeriod))
			{
				if (!bFail)
				{
					errMsg.AddNewError(STRING_ERROR_MSG);
					bFail = TRUE;
				}
				errMsg.SaveErrorMsg("Switch from %.0f to %.0f, The set sample period(%.0f), but the actual sample times(%.0f) is not equal to target sample times(%.0f).",
					dFirstPeriod, dSecondPeriod, dSecondPeriod, dActualSamplePeriod, dSecondPeriod);
			}
		}
	}

	errMsg.Print(this, g_lpszReportFilePath);

	dcm_CloseFile();//Unload the vector file.
}