#pragma once
/**
 * @file TestDCMGetRunLineCountParamValidity.h
 * @brief Check the parameter validity of GetRunLineCount
 * @author Guangyun Wang
 * @date 2021/04/23
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controller Technology Co., Ltd.
*/
#include "..\DCMTestMain.h"

XT_TEST(ParamValidityTest, TestDCMGetRunLineCountParamValidity)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, PARAM_VADILITY);
	CErrorMSG errMsg(strFuncName.c_str(), "ParamValidityTest");//Error message.
	int nRetVal = 0;
	const int nPinNameTestCount = 4;
	char* lpszPinName[nPinNameTestCount] = { nullptr, " ", "CH44", "CH0" };
	ULONG ulLineCount = 0;
	nRetVal = dcm.GetRunLineCount(nullptr, 0, ulLineCount);
	if (VECTOR_FILE_NOT_LOADED != nRetVal)
	{
		//Not warning when vector is not loaded.
		XT_EXPECT_EQ(nRetVal, VECTOR_FILE_NOT_LOADED);

		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("No Warning when vector is not loaded!");
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
	int nExpectRetVal = 0;
	USHORT uSiteCount = dcm_GetVectorSiteCount() + 1;

	for (auto& PinName : lpszPinName)
	{
		for (int nSiteNo = 0; nSiteNo < uSiteCount; ++nSiteNo)
		{
			nRetVal = dcm.GetRunLineCount(PinName, nSiteNo, ulLineCount);
			nExpectRetVal = CheckParamValidity(errMsg, mapSlot, nRetVal, PinName, 0, nSiteNo, 0);
			XT_EXPECT_EQ(nRetVal, nExpectRetVal);
		}
	}

	USHORT usInvalidSite = mapSlot.begin()->second;
	InvalidSite(usInvalidSite);
	nRetVal = dcm.GetRunLineCount("CH0", usInvalidSite, ulLineCount);
	XT_EXPECT_EQ(nRetVal, SITE_INVALID);
	if (SITE_INVALID != nRetVal)
	{
		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("The SITE_%d is invalid, but the return value(%d) is not equal to SITE_INVALID(%d).", usInvalidSite + 1, nRetVal, SITE_INVALID);
	}
	RestoreSite();
	errMsg.Print(this, g_lpszReportFilePath);


	dcm_CloseFile();//Unload the vector file.
}