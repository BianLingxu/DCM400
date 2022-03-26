#pragma once
/*!
* @file      TestDCMGetFirstFailLineNoParamValidity.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/10/09
* @version   v 1.0.0.0
* @brief     测试GetFirstFailLineNo参数有效性
* @comment
*/
//#define ONLYCODE 1
#include "..\DCMTestMain.h"

XT_TEST(ParamValidityTest, TestDCMGetFirstFailLineNoParamValidity)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, PARAM_VADILITY);
	CErrorMSG errMsg(strFuncName.c_str(), "ParamValidityTest");//Error message.
	int nRetVal = 0;
	const int nPinNameTestCount = 3;
	char* lpszTestPinName[nPinNameTestCount] = { nullptr, "NOPIN", "CH0" };
	ULONG ulFirstFailLineNo = 0;

	nRetVal = dcm.GetFirstFailLineNo(nullptr, 0, ulFirstFailLineNo);
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
	int nExpectValue = 0;
	for (auto& lpszPinName : lpszTestPinName)
	{
		for (USHORT uSiteID = 0; uSiteID < uSiteCount; ++uSiteID)
		{
			nRetVal = dcm.GetFirstFailLineNo(lpszPinName, uSiteID, ulFirstFailLineNo);
			nExpectValue = CheckParamValidity(errMsg, mapSlot, nRetVal, lpszPinName, 0, uSiteID, NO_FAIL_LINE);
			XT_EXPECT_EQ(nRetVal, nExpectValue);
		}
	}
	mapSlot.clear();
	errMsg.Print(this, g_lpszReportFilePath);


	dcm_CloseFile();//Unload the vector file.
}