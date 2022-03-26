#pragma once
/*!
* @file      TestDCMGetFailLineNoParamValidity.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/10/09
* @version   v 1.0.0.0
* @brief     测试GetFailLineNo参数有效性
* @comment
*/
#include "..\DCMTestMain.h"

XT_TEST(ParamValidityTest, TestDCMGetFailLineNoParamValidity)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, PARAM_VADILITY);
	CErrorMSG errMsg(strFuncName.c_str(), "ParamValidityTest");//Error message.
	int nRetVal = 0;
	const int nPinNameTestCount = 3;
	char* pinName[nPinNameTestCount] = { nullptr, "NOPIN", "CH0" };
	const int nSiteTestCount = 4;
	USHORT uTestSite[nSiteTestCount] = { 0, 1, SITE_NUM, SITE_NUM + 1 };
	const int nCountTestCount = 5;
	USHORT uTestCount[nCountTestCount] = { -1, 0, 10, 100, 200 };
	ULONG ulFailLineNo[100] = { 0 };

	int nValidBoardCount = dcm_GetValidBoard();
	if (0 == nValidBoardCount)
	{
		//No board is inserted.
		XT_EXPECT_TRUE(FALSE);
		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("No board inserted!");
		errMsg.Print(this, g_lpszReportFilePath);
		return;
	}

	nRetVal = dcm.GetFailLineNo(nullptr, 0,0, ulFailLineNo);
	if (VECTOR_FILE_NOT_LOADED != nRetVal)
	{
		//Not warning when vector is not loaded.
		XT_EXPECT_EQ(VECTOR_FILE_NOT_LOADED, nRetVal);
		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("No warning when vector is not loaded!");
	}

	map<BYTE, USHORT> mapSlot;
	nRetVal = GetBoardInfo(mapSlot, g_lpszVectorFilePath);
	

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
	for (int nPinNameIndex = 0; nPinNameIndex < nPinNameTestCount; ++nPinNameIndex)
	{
		for (USHORT uSiteID = 0; uSiteID < uSiteCount ; ++uSiteID)
		{
			nRetVal = dcm.GetFailLineNo(pinName[nPinNameIndex], uSiteID, 100, ulFailLineNo);
			nExpectValue = CheckParamValidity(errMsg, mapSlot, nRetVal, pinName[nPinNameIndex], 0, uSiteID, 0);
			XT_EXPECT_EQ(nRetVal, nExpectValue);
		}
	}

	errMsg.Print(this, g_lpszReportFilePath);


	dcm_CloseFile();//Unload the vector file.
}