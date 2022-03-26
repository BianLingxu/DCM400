#pragma once
/*!
* @file      TestDCMGetStopLineNoParamValidity.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/10/09
* @version   v 1.0.0.0
* @brief     测试GetStopLineNo参数有效性
* @comment
*/

#include "..\DCMTestMain.h"

XT_TEST(ParamValidityTest, TestDCMGetStopLineNoParamValidity)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, PARAM_VADILITY);
	CErrorMSG errMsg(strFuncName.c_str(), "ParamValidityTest");//Error message.
	int nRetVal = 0;
	ULONG ulStopLineNo = 0;
	nRetVal = dcm.GetStopLineNo(0, ulStopLineNo);
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
	USHORT uVectorSiteCount = dcm_GetVectorSiteCount();
	USHORT uSiteCount = uVectorSiteCount + 1;
	for (USHORT uSiteID = 0; uSiteID < uSiteCount; ++uSiteID)
	{
		nRetVal = dcm.GetStopLineNo(uSiteID, ulStopLineNo);
		if (SITE_ERROR == nRetVal)
		{
			XT_EXPECT_LESSEQ(uSiteID, uVectorSiteCount);
			if (uSiteID < uVectorSiteCount)
			{
				errMsg.AddNewError(STRING_ERROR_MSG);
				char cDigit[10] = { 0 };
				_itoa_s(uSiteID, cDigit, 10, 10);
				errMsg.SetErrorItem("SiteID", cDigit);
				errMsg.SaveErrorMsg("The site id is in scale, the return value is %d!", nRetVal);
			}
		}
		else
		{
			iterSlot = mapSlot.begin();
			while (mapSlot.end() != iterSlot)
			{
				if (uSiteID >= iterSlot->second && uSiteID < iterSlot->second + DCM_MAX_CONTROLLERS_PRE_BOARD)
				{
					if (0 > nRetVal)
					{
						errMsg.AddNewError(STRING_ERROR_MSG);
						errMsg.SaveErrorMsg("The board is inserted, but return (%d).", nRetVal);
					}
					break;
				}
				++iterSlot;
			}
			if (mapSlot.end() != iterSlot)
			{
				continue;
			}

			XT_EXPECT_EQ(nRetVal, (int)BOARD_NOT_INSERT_ERROR);
			if (BOARD_NOT_INSERT_ERROR != nRetVal)
			{
				errMsg.AddNewError(STRING_ERROR_MSG);
				char cDigit[10] = { 0 };
				_itoa_s(uSiteID, cDigit, 10, 10);
				errMsg.SaveErrorMsg("The board is not inserted, but return(%d) value is not BOARD_NOT_INSERTED(%d).", nRetVal, BOARD_NOT_INSERT_ERROR);
			}
		}
	}

	errMsg.Print(this, g_lpszReportFilePath);


	dcm_CloseFile();//Unload the vector file.
}