#pragma once
/**
 * @file TestDCMGetPPMUMeasResultParamValidity.h
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controller Co., Ltd.
 * @author Guangyun Wang
 * @date 2020/07/16
 * @version v 1.0.0.0
 * @brief Test the parameter validity of GetPPMUMeasResult
 */
#include "..\DCMTestMain.h"

inline double CheckGetPMUParam(CErrorMSG& errMsg, const std::map<BYTE, USHORT>& mapSlot, double dRetVal, const char* lpszPinName, BYTE byPinType, USHORT usSiteID, BOOL& bFuccess)
{
	bFuccess = FALSE;
	USHORT uSiteCount = dcm_GetVectorSiteCount();
	double dExpectValue = MAX_MEASURE_VALUE;

	int nStringType = dcm_GetStringType(lpszPinName);

	if (usSiteID > uSiteCount)
	{
		if (EQUAL_ERROR < fabs(MAX_MEASURE_VALUE - dRetVal))
		{
			dExpectValue = 0;
			errMsg.AddNewError(STRING_ERROR_MSG);
			char lpszDigit[10] = { 0 };
			_itoa_s(usSiteID, lpszDigit, 10, 10);
			errMsg.SetErrorItem("SiteID", lpszDigit);
			errMsg.SaveErrorMsg("The site id is in scale, the return value is %d!", dRetVal);
		}
		dExpectValue = MAX_MEASURE_VALUE;
		bFuccess = TRUE;
	}
	else if (0 > nStringType)
	{
		dExpectValue = MAX_MEASURE_VALUE;
		if (0 != byPinType)
		{
			dExpectValue = MAX_MEASURE_VALUE;
		}
		if (EQUAL_ERROR < fabs(MAX_MEASURE_VALUE - dRetVal))
		{
			//No warning when pin name is not set.
			errMsg.AddNewError(STRING_ERROR_MSG);
			if (0 == byPinType)
			{
				errMsg.SetErrorItem("lpszPinName", lpszPinName);
				errMsg.SaveErrorMsg("No warning when pin name is not defined, the return value is %f!", dRetVal);
			}
			else
			{
				errMsg.SetErrorItem("lpszPinGroup", lpszPinName);
				errMsg.SaveErrorMsg("No warning when pin group is not defined, the return value is %f!", dRetVal);
			}
		}
	}
	else
	{
		for (auto& Slot : mapSlot)
		{
			if (usSiteID >= Slot.second && usSiteID < Slot.second + DCM_MAX_CONTROLLERS_PRE_BOARD)
			{
				if (EQUAL_ERROR > fabs(MAX_MEASURE_VALUE - dRetVal))
				{
					errMsg.AddNewError(STRING_ERROR_MSG);
					errMsg.SaveErrorMsg("The board is inserted, but return (%f).", dRetVal);
				}
				bFuccess = TRUE;
				return MAX_MEASURE_VALUE;
			}
		}

		dExpectValue = MAX_MEASURE_VALUE;
		if (EQUAL_ERROR < fabs(MAX_MEASURE_VALUE - dRetVal))
		{
			errMsg.AddNewError(STRING_ERROR_MSG);
			char lpszDigit[10] = { 0 };
			_itoa_s(usSiteID, lpszDigit, 10, 10);
			errMsg.SaveErrorMsg("The board is not inserted, but return(%f) value is not %f.", dRetVal, MAX_MEASURE_VALUE);
		}
	}
	return dExpectValue;
}

XT_TEST(PMUParamValidityTest, TestDCMGetPPMUMeasResultParamValidity)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, PARAM_VADILITY);
	CErrorMSG errMsg(strFuncName.c_str(), "ParamValidityTest");//Error message.
	int nRetVal = 0;
	const int nPinNameTestCount = 3;
	char* lpszTestPinName[nPinNameTestCount] = { nullptr, "NOPIN", "CH0" };

	DWORD dwCaptureData = 0;
	double dRetVal = dcm.GetPPMUMeasResult(nullptr, 0);
	XT_EXPECT_REAL_EQ(MAX_MEASURE_VALUE, dRetVal, EQUAL_ERROR);
	if (EQUAL_ERROR < fabs(dRetVal - MAX_MEASURE_VALUE))
	{
		//Not warning when vector is not loaded.
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

	dcm.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");

	USHORT uSiteCount = dcm_GetVectorSiteCount() + 1;
	double dExpectValue = 0;

	const BYTE byCaptureCountPerSector = 6;
	const BYTE bySectorCount = 8;
	const int nCaptureLineCount = byCaptureCountPerSector * bySectorCount;
	double dMeasureResult = 0;
	BOOL bSuccess = FALSE;
	for (auto& lpszPinName : lpszTestPinName)
	{
		for (USHORT usSiteID = 0; usSiteID < uSiteCount; ++usSiteID)
		{
			dMeasureResult = dcm.GetPPMUMeasResult(lpszPinName, usSiteID);
			dExpectValue = CheckGetPMUParam(errMsg, mapSlot, dMeasureResult, lpszPinName, 0, usSiteID, bSuccess);
			if (bSuccess)
			{
				XT_EXPECT_LESSEQ(dMeasureResult, dExpectValue, 1e-6);
			}
			else
			{
				XT_EXPECT_REAL_EQ(dMeasureResult, dExpectValue, 1e-6);
			}			
		}
	}


	const USHORT usInvalidSite = mapSlot.begin()->second;

	InvalidSite(usInvalidSite);

	dMeasureResult = dcm.GetPPMUMeasResult("CH2", usInvalidSite);
	XT_EXPECT_REAL_EQ(dMeasureResult, MAX_MEASURE_VALUE, 1e-6);
	if (0.1 < fabs(dMeasureResult - MAX_MEASURE_VALUE))
	{
		errMsg.SaveErrorMsg("The SITE_%d is invalid, but the return value(%.0f) is not equal to MAX_MEASURE_VALUE(%.0f).", usInvalidSite + 1, dMeasureResult, MAX_MEASURE_VALUE);
	}

	RestoreSite();

	mapSlot.clear();
	errMsg.Print(this, g_lpszReportFilePath);

	dcm_CloseFile();//Unload the vector file.
}