#pragma once
/*!
* @file      TestDCMSetWaveDataParamParamValidity.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2020/01/06
* @version   v 1.0.0.0
* @brief     测试SetWaveDataParam参数有效性
* @comment
*/
#include "..\DCMTestMain.h"

XT_TEST(ParamValidityTest, TestDCMSetWaveDataParamParamValidity)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, PARAM_VADILITY);
	CErrorMSG errMsg(strFuncName.c_str(), "ParamValidityTest");//Error message.
	int nRetVal = 0;
	const int nPinGroupTestCount = 3;
	char* pinGroup[nPinGroupTestCount] = { nullptr, "G_NOPIN", "G_ALLPIN" };
	const int nTestCount = 5;
	const char* lpszTestLabel[nTestCount] = { nullptr, "NO_LABEL", "", "FIRST_ST", "FIRST_SP" };
	const int nTestSiteCount = 4;

	DWORD dwWaveData = 0;
	nRetVal = dcm.SetWaveDataParam(nullptr, "FIRST_ST", 0, 12);
	if (VECTOR_FILE_NOT_LOADED != nRetVal)
	{
		//Not warning when vector is not loaded.
		XT_EXPECT_EQ(VECTOR_FILE_NOT_LOADED, nRetVal);
		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("No warning when vector is not loaded!");
	}

	map<BYTE, USHORT> mapSlot;
	nRetVal = GetBoardInfo(mapSlot, g_lpszVectorFilePath);
	if (0 == mapSlot.size())
	{
		//No board is inserted.
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

	auto iterSlot = mapSlot.begin();
	USHORT uSiteCount = dcm_GetVectorSiteCount() + 1;

	//Defined pin group G_ALLPIN
	dcm.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");
	int nExpectValue = 0;
	iterSlot = mapSlot.begin();
	for (int nPinGroupIndex = 0; nPinGroupIndex < nPinGroupTestCount; ++nPinGroupIndex)
	{
		for (int nLabelIndex = 0; nLabelIndex < nTestCount; ++nLabelIndex)
		{
			nRetVal = dcm.SetWaveDataParam(pinGroup[nPinGroupIndex], lpszTestLabel[nLabelIndex], 0, 20);

			nExpectValue = CheckParamValidity(errMsg, mapSlot, nRetVal, pinGroup[nPinGroupIndex], 1, iterSlot->second, 0, TRUE, lpszTestLabel[nLabelIndex]);
			XT_EXPECT_EQ(nRetVal, nExpectValue);
		}
	}
	ULONG ulLineCount = 0;
	dcm_GetVectorLineCount("", "", ulLineCount);
	int nLabelLine = dcm_GetLabelLineNo("FIRST_ST", TRUE);
	int nMaxLineCount = ulLineCount - dcm_GetLabelLineNo("FIRST_ST", FALSE);
	const int nTestOffsetCount = 3;
	ULONG ulOffset[nTestOffsetCount] = { 0, nMaxLineCount, nMaxLineCount + 10 };
	const int nCountTestCount = 5;
	int nCountTestItem[nCountTestCount] = { -1, 0, 15, 32, 33 };

	int nCalCount = 0;
	for (int nOffsetIndex = 0; nOffsetIndex < nTestOffsetCount; ++nOffsetIndex)
	{
		for (int nCountIndex = 0; nCountIndex < nCountTestCount; ++nCountIndex)
		{
			nRetVal = dcm.SetWaveDataParam("G_ALLPIN", "FIRST_ST", ulOffset[nOffsetIndex], nCountTestItem[nCountIndex]);

			if (nMaxLineCount <= nLabelLine + ulOffset[nOffsetIndex])
			{
				if (OFFSET_ERROR != nRetVal)
				{
					errMsg.AddNewError(STRING_ERROR_MSG, "G_ALLPIN", 2);
					char cDigit[10] = { 0 };
					_itoa_s(ulOffset[nOffsetIndex], cDigit, 10, 10);
					errMsg.SetErrorItem("Offset", cDigit);
					errMsg.SaveErrorMsg("No warning when the offset plus count is over range, the return value is %d!", nRetVal);
				}
			}
			else if (0 >= nCountTestItem[nCountIndex] || nMaxLineCount < nCountTestItem[nCountIndex] + ulOffset[nOffsetIndex])
			{
				XT_EXPECT_EQ(nRetVal, VECTOR_LINE_OVER_RANGE);
				if (VECTOR_LINE_OVER_RANGE != nRetVal)
				{
					errMsg.AddNewError(STRING_ERROR_MSG, "G_ALLPIN", 2);
					char cDigit[10] = { 0 };
					_itoa_s(ulOffset[nOffsetIndex], cDigit, 10, 10);
					errMsg.SetErrorItem("Offset", cDigit);
					errMsg.SaveErrorMsg("No warning when the offset plus count is over range, the return value is %d!", nRetVal);
				}
			}
			else
			{
				XT_EXPECT_EQ(nRetVal, EXECUTE_SUCCESS);
				if (EXECUTE_SUCCESS != nRetVal)
				{
					errMsg.AddNewError(STRING_ERROR_MSG, "G_ALLPIN", 2);
					char cDigit[10] = { 0 };
					_itoa_s(ulOffset[nOffsetIndex], cDigit, 10, 10);
					errMsg.SaveErrorMsg("Unknown error, the return value is %d!", nRetVal);
				}
			}
		}
	}
	mapSlot.clear();
	errMsg.Print(this, g_lpszReportFilePath);

	dcm_CloseFile();//Unload the vector file.
}