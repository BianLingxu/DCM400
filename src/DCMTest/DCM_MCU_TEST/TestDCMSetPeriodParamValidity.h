#pragma once
/*!
* @file      TestDCMSetRateParamValidity.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/09/05
* @version   v 1.0.0.0
* @brief     测试SetRate参数有效性
* @comment   
*/

#include "..\DCMTestMain.h"
XT_TEST(ParamValidityTest, TestDCMSetPeriodParamValidity)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, PARAM_VADILITY);
	CErrorMSG errMsg(strFuncName.c_str(), "ParamValidityTest");//Error message.
	const int nTestItemCount = 10;
	int nTimeSet = 0;
	double dPeriod = 0;
	int nRetVal = 0;
	double dReadRate = 0;
	double dSetRate[nTestItemCount] = { -10, 0, 5, 10, MIN_PERIOD, MIN_PERIOD * 2, MAX_PERIOD, MAX_PERIOD + 10 };

	const double dResolution = 5;
	map<BYTE, USHORT> mapSlot;
	GetBoardInfo(mapSlot, g_lpszVectorFilePath, TRUE);
	BYTE byBoardCount = dcm_GetBoardInfo(nullptr, 0);
	if (0 == mapSlot.size())
	{
		///<No board is inserted
		XT_EXPECT_TRUE(FALSE);
		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("No board inserted!");
		errMsg.Print(this, g_lpszReportFilePath);
		return;
	}
	int nMaxTimesetCount = TIMESET_COUNT - 1;
	nRetVal = dcm.SetPeriod("0", 200);

	if (VECTOR_FILE_NOT_LOADED != nRetVal)
	{

		XT_EXPECT_EQ(VECTOR_FILE_NOT_LOADED, nRetVal);

		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("No Warning when vector is not loaded!");
	}

	dcm.LoadVectorFile(g_lpszVectorFilePath, FALSE);

	char lpszTimesetName[32] = { 0 };
	auto iterSlot = mapSlot.begin();
	for (int nRateIndex = 0; nRateIndex < nTestItemCount; ++nRateIndex)
	{
		for (int nTimeset = -1; nTimeset < nMaxTimesetCount; ++nTimeset)
		{
			_itoa_s(nTimeset, lpszTimesetName, sizeof(lpszTimesetName), 10);
			nRetVal = dcm.SetPeriod(lpszTimesetName, dSetRate[nRateIndex]);
			if (0 == nRetVal)
			{
				iterSlot = mapSlot.begin();
				while (mapSlot.end() != iterSlot)
				{
					for (int nCtlIndex = 0; nCtlIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++nCtlIndex)
					{
						dReadRate = dcm_GetTimeSetPeriod(iterSlot->first, nCtlIndex, nTimeset);
						double dError = fabs(dReadRate - dSetRate[nRateIndex]);
						if (MAX_PERIOD < dSetRate[nRateIndex] || dResolution < dError)
						{
							//The rate in register is not equal to the rate written.
							errMsg.AddNewError(VALUE_ERROR);
							errMsg.SetErrorItem("Period", 0, -1, VALUE_NOT_EQUAL);
							if (MAX_PERIOD < dSetRate[nRateIndex])
							{
								XT_EXPECT_REAL_EQ(dReadRate, dSetRate[nRateIndex], dResolution);
								errMsg.SaveErrorMsg(dReadRate, dSetRate[nRateIndex], dResolution, VALUE_DOUBLE_ONE_DECIMAL);
							}
							else
							{
								XT_EXPECT_REAL_EQ(dReadRate, dSetRate[nRateIndex], dResolution);
								errMsg.SaveErrorMsg(dReadRate, dSetRate[nRateIndex], dResolution, VALUE_DOUBLE_ONE_DECIMAL);
							}

						}
					}
					++iterSlot;
				}
			}
			else
			{
				if (TIMESET_ERROR == nRetVal)
				{
					if (0 < nTimeset && nMaxTimesetCount > nTimeset)
					{
						XT_EXPECT_TRUE(FALSE);
						errMsg.AddNewError(STRING_ERROR_MSG);
						char lpszDigit[10] = { 0 };
						_itoa_s(nTimeset, lpszDigit, 10, 10);
						errMsg.SetErrorItem("lpszTimeset", lpszDigit);
						errMsg.SaveErrorMsg("The timeset(%s) is over scale, but return value(%d) is not equal to %d!", lpszDigit, nRetVal, TIMESET_ERROR);
					}
				}
				else if (RATE_ERROR == nRetVal)
				{
					if (MAX_PERIOD + EQUAL_ERROR >= dSetRate[nRateIndex] && MIN_PERIOD - EQUAL_ERROR <= dSetRate[nRateIndex])
					{
						XT_EXPECT_EQ(nRetVal, 0);
						errMsg.AddNewError(STRING_ERROR_MSG);
						char cRate[32] = { 0 };
						sts_sprintf(cRate, 32, "%.2f", dSetRate[nRateIndex]);
						errMsg.SetErrorItem("Rate", cRate);
						errMsg.SaveErrorMsg("Rate is right but return RATE_ERROR!");
					}
				}
			}
		}
	}

	errMsg.Print(this, g_lpszReportFilePath);

	dcm_CloseFile();//Unload the vector file.
}