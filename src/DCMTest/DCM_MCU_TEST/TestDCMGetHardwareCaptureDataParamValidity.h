#pragma once
/*!
* @file      TestDCMGetHardwareCaptureDataParamValidity.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2020/07/15
* @version   v 1.0.0.0
* @brief     测试GetHardwareCaptureData参数有效性
* @comment
*/
#include "..\DCMTestMain.h"
XT_TEST(ParamValidityTest, TestDCMGetHardwareCaptureDataParamValidity)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, PARAM_VADILITY);
	CErrorMSG errMsg(strFuncName.c_str(), "ParamValidityTest");//Error message.
	int nRetVal = 0;
	const int nPinNameTestCount = 3;
	char* lpszTestPinName[nPinNameTestCount] = { nullptr, "NOPIN", "CH0" };

	DWORD dwCaptureData = 0;
	nRetVal = dcm.GetHardwareCaptureData(nullptr, 0, 0, 0);
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
	dcm.RunVectorWithGroup("G_ALLPIN", "TEST_FAIL_ST", "TEST_FAIL_SP");

	USHORT uSiteCount = dcm_GetVectorSiteCount() + 1;
	int nExpectValue = 0;
	int nLineCount = 20;
	int nLabelLineNo = 0;
	BYTE byData[50] = { 0 };

	const BYTE byCaptureCountPerSector = 6;
	const BYTE bySectorCount = 8;
	const int nCaptureLineCount = byCaptureCountPerSector * bySectorCount + 4;///<4 is  cpature in loop
	for (auto& lpszPinName : lpszTestPinName)
	{
		for (USHORT uSiteID = 0; uSiteID < uSiteCount; ++uSiteID)
		{
			nRetVal = dcm.GetHardwareCaptureData(lpszPinName, uSiteID, byData, sizeof(byData));
			nExpectValue = CheckParamValidity(errMsg, mapSlot, nRetVal, lpszPinName, 0, uSiteID, nCaptureLineCount);
			XT_EXPECT_EQ(nRetVal, nExpectValue);
		}
	}
	mapSlot.clear();
	errMsg.Print(this, g_lpszReportFilePath);

	dcm_CloseFile();//Unload the vector file.
}