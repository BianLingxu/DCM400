#pragma once
/*!
* @file      TestDCMGetMCUResultParamValidity.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/09/28
* @version   v 1.0.0.0
* @brief     测试GetMCUResult参数有效性
* @comment
*/
#include "..\DCMTestMain.h"

XT_TEST(ParamValidityTest, TestDCMGetMCUResultParamValidity)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, PARAM_VADILITY);
	CErrorMSG errMsg(strFuncName.c_str(), "ParamValidityTest");//Error message.
	int nRetVal = 0;

	nRetVal = dcm.GetMCUResult(0);
	XT_EXPECT_EQ(VECTOR_FILE_NOT_LOADED, nRetVal);
	if (VECTOR_FILE_NOT_LOADED != nRetVal)
	{
		///<Not warning when vector is not loaded
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

	USHORT usValidSiteCount = dcm_GetVectorSiteCount();
	bool* pbySiteExisted = nullptr;
	try
	{
		pbySiteExisted = new bool[usValidSiteCount];
		memset(pbySiteExisted, 0, usValidSiteCount * sizeof(bool));
	}
	catch (const std::exception&)
	{
		return;
	}

	USHORT usFirstValidSite = -1;
	auto iterSlot = mapSlot.begin();
	while (mapSlot.end() != iterSlot)
	{
		if (-1 == usFirstValidSite)
		{
			usFirstValidSite = iterSlot->second;
		}
		for (BYTE byControllerIndex = 0; byControllerIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++byControllerIndex)
		{
			pbySiteExisted[iterSlot->second + byControllerIndex] = true;
		}
		++iterSlot;
	}
	
	dcm.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH8,CH10,CH11,CH12,CH13,CH4,CH15");

	dcm.RunVectorWithGroup("G_ALLPIN", "FIRST_ST", "FIRST_SP");

	USHORT usSiteCount = usValidSiteCount + 1;
	int nExpectValue = 0;
	for (USHORT usSiteNo = 0; usSiteNo < usSiteCount; ++usSiteNo)
	{
		nRetVal = dcm.GetMCUResult(usSiteNo);
		if (usSiteNo >= usValidSiteCount)
		{
			XT_EXPECT_EQ(nRetVal, SITE_ERROR);
			if (SITE_ERROR != nRetVal)
			{
				errMsg.AddNewError(STRING_ERROR_MSG);
				errMsg.SaveErrorMsg("The site(%d) is over range[%d,%d], but the return(%d) is not equal to %d", usSiteNo, 0, usValidSiteCount - 1, nRetVal, SITE_ERROR);
			}
			continue;
		}
		else if(!pbySiteExisted[usSiteNo])
		{
			XT_EXPECT_EQ(nRetVal, BOARD_NOT_INSERT_ERROR);
			if (BOARD_NOT_INSERT_ERROR != nRetVal)
			{
				errMsg.AddNewError(STRING_ERROR_MSG);
				errMsg.SaveErrorMsg("The board is not existed, but the return(%d) is not equal to %d", nRetVal, BOARD_NOT_INSERT_ERROR);
			}
		}
		else if (0 > nRetVal)
		{
			XT_EXPECT_GREATEREQ(nRetVal, 0);
			errMsg.AddNewError(STRING_ERROR_MSG);
			errMsg.SaveErrorMsg("Unknown error, the return value is %d", nRetVal);
		}
	}
	if (nullptr != pbySiteExisted)
	{
		delete[] pbySiteExisted;
		pbySiteExisted = nullptr;
	}
	errMsg.Print(this, g_lpszReportFilePath);


	dcm_CloseFile();//Unload the vector file.
}