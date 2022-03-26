#pragma once
/*!
* @file      TestDCMWriteWaveDataParamValidity.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/09/30
* @version   v 1.0.0.0
* @brief     测试WriteWaveData参数有效性
* @comment
*/

#include "..\DCMTestMain.h"
XT_TEST(ParamValidityTest, TestDCMWriteWaveDataParamValidity)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, PARAM_VADILITY);
	CErrorMSG errMsg(strFuncName.c_str(), "ParamValidityTest");//Error message.
	int nRetVal = 0;
	const int nPinGroupTestCount = 4;
	char* pinGroup[nPinGroupTestCount] = { nullptr, "G_NOPIN", "G_ALLPIN", "CH0" };
	const int nTestCount = 5;
	const char* lpszTestLabel[nTestCount] = { nullptr, "NO_LABEL", "", "FIRST_ST", "FIRST_SP" };
	const int nTestSiteCount = 4;


	DWORD dwWaveData = 0;
	nRetVal = dcm.WriteWaveData(nullptr, "FIRST_ST", 0, 0, 0, dwWaveData);
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
	while (mapSlot.end() != iterSlot)
	{
		for (int nPinGroupIndex = 0; nPinGroupIndex < nPinGroupTestCount; ++nPinGroupIndex)
		{
			for (int nLabelIndex = 0; nLabelIndex < nTestCount; ++nLabelIndex)
			{
				for (int nSiteID = 0; nSiteID < uSiteCount; ++nSiteID)
				{
					nRetVal = dcm.WriteWaveData(pinGroup[nPinGroupIndex], lpszTestLabel[nLabelIndex], nSiteID, 0, 20, dwWaveData);

					nExpectValue = CheckParamValidity(errMsg, mapSlot, nRetVal, pinGroup[nPinGroupIndex], 1, nSiteID, 0, TRUE, lpszTestLabel[nLabelIndex]);
					XT_EXPECT_EQ(nRetVal, nExpectValue);
				}
			}
		}
		++iterSlot;
	}
	ULONG ulLineCount = 0;
	dcm_GetVectorLineCount("", "", ulLineCount);
	int nMaxLineCount = ulLineCount - dcm_GetLabelLineNo("FIRST_ST", FALSE);
	const int nTestOffsetCount = 3;
	ULONG ulOffset[nTestOffsetCount] = { 0, nMaxLineCount, nMaxLineCount + 10 };
	const int nCountTestCount = 5;
	int nCountTestItem[nCountTestCount] = { -1, 0, 15, 32, 33 };

	int nCalCount = 0;
	iterSlot = mapSlot.begin();
	for (int nOffsetIndex = 0; nOffsetIndex < nTestOffsetCount; ++nOffsetIndex)
	{
		for (int nCountIndex = 0; nCountIndex < nCountTestCount; ++nCountIndex)
		{
			nRetVal = dcm.WriteWaveData("G_ALLPIN", "FIRST_ST", iterSlot->second, ulOffset[nOffsetIndex], nCountTestItem[nCountIndex], dwWaveData);
			if (1 > nCountTestItem[nCountIndex])
			{
				nCalCount = 1;
			}
			else if (32 < nCountTestItem[nCountIndex])
			{
				nCalCount = 32;
			}
			else
			{
				nCalCount = nCountTestItem[nCountIndex];
			}
			if (nMaxLineCount <= ulOffset[nOffsetIndex])
			{
				XT_EXPECT_EQ(nRetVal, OFFSET_ERROR);
				if (OFFSET_ERROR != nRetVal)
				{
					errMsg.AddNewError(STRING_ERROR_MSG, "G_ALLPIN", 2);
					char cDigit[10] = { 0 };
					_itoa_s(ulOffset[nOffsetIndex], cDigit, 10, 10);
					errMsg.SetErrorItem("ulOffset", cDigit);
					errMsg.SaveErrorMsg("No warning when the offset plus count is over range, the return value is %d!", nRetVal);
				}
			}
			else if (nMaxLineCount < nCalCount + ulOffset[nOffsetIndex])
			{
				XT_EXPECT_EQ(nRetVal, VECTOR_LINE_OVER_RANGE);
				if (VECTOR_LINE_OVER_RANGE != nRetVal)
				{
					errMsg.AddNewError(STRING_ERROR_MSG, "G_ALLPIN", 2);
					char cDigit[10] = { 0 };
					_itoa_s(ulOffset[nOffsetIndex], cDigit, 10, 10);
					errMsg.SetErrorItem("nLineCoun", cDigit);
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