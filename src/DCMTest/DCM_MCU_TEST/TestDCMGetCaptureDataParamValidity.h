#pragma once
/*!
* @file      TestDCMGetCaptureDataParamValidity.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/09/27
* @version   v 1.0.0.0
* @brief     测试GetCaptureData参数有效性
* @comment
*/

#include "..\DCMTestMain.h"
XT_TEST(ParamValidityTest, TestDCMGetCaptureDataParamValidity)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, PARAM_VADILITY);
	CErrorMSG errMsg(strFuncName.c_str(), "ParamValidityTest");//Error message.
	int nRetVal = 0;
	const int nPinNameTestCount = 3;
	char* lpszPinName[nPinNameTestCount] = { nullptr, "NOPIN", "CH0" };
	const int nTestCount = 5;
	const char* LabelTestValue[nTestCount] = { nullptr, "NO_LABEL", "", "FIRST_ST", "FIRST_SP" };

	DWORD dwCaptureData = 0;
	nRetVal = dcm.GetCaptureData(nullptr, "FIRST_ST", 0, 0, 0, dwCaptureData);
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

	dcm.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");
	dcm.RunVectorWithGroup("G_ALLPIN", "FIRST_ST", "FIRST_SP");

	int nRanStartLineNo = dcm_GetLabelLineNo("FIRST_ST");
	int nRanStopLineNo = dcm_GetLabelLineNo("FIRST_SP");

	USHORT uSiteCount = dcm_GetVectorSiteCount() + 1;
	int nExpectValue = 0;
	int nCorrectValue = 0;
	int nLineCount = 20;
	int nLabelLineNo = 0;
	for (int nPinNameIndex = 0; nPinNameIndex < nPinNameTestCount; ++nPinNameIndex)
	{
		for (USHORT uSiteID = 0; uSiteID < uSiteCount; ++uSiteID)
		{
			for (int nLabelIndex = 0; nLabelIndex < nTestCount; ++nLabelIndex)
			{
				nCorrectValue = 0;
				nRetVal = dcm.GetCaptureData(lpszPinName[nPinNameIndex], LabelTestValue[nLabelIndex], uSiteID, 0, nLineCount, dwCaptureData);
				nLabelLineNo = dcm_GetLabelLineNo(LabelTestValue[nLabelIndex]);
				if (0 > nLabelLineNo)
				{
					nCorrectValue = START_LABEL_ERROR;
				}
				else if (nLabelLineNo < nRanStartLineNo || nLabelLineNo + nLineCount > nRanStopLineNo)
				{
					nCorrectValue = VECTOR_NOT_IN_LAST_RAN;
				}
				nExpectValue = CheckParamValidity(errMsg, mapSlot, nRetVal, lpszPinName[nPinNameIndex], 0, uSiteID, nCorrectValue, TRUE, LabelTestValue[nLabelIndex], FALSE);
				XT_EXPECT_EQ(nRetVal, nExpectValue);
			}
		}
	}
	mapSlot.clear();
	errMsg.Print(this, g_lpszReportFilePath);

	dcm_CloseFile();//Unload the vector file.
}