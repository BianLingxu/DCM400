#pragma once
/*!
* @file      TestDCMSetlpszPinGroupParamValidity.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/10/09
* @version   v 1.0.0.0
* @brief     测试SetlpszPinGroup参数有效性
* @comment
*/
#include "..\DCMTestMain.h"
XT_TEST(ParamValidityTest, TestDCMSetPinGroupParamValidity)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, PARAM_VADILITY);
	CErrorMSG errMsg(strFuncName.c_str(), "ParamValidityTest");//Error message.
	int nRetVal = 0;
	const int nlpszPinGroupTestCount = 5;
	char* lpszPinGroup[nlpszPinGroupTestCount] = { nullptr, " ", "G_PIN0_1","G_PIN0_1", "G_PIN3_4" };
	const int nPinNameTestCount = 5;
	char* pinName[nPinNameTestCount] = { nullptr, " ", "NOPIN", "CH0,CH1", " CH3,CH4" };

	int nBoardCount = dcm_GetValidBoard();
	if (0 == nBoardCount)
	{
		//No board is inserted.
		XT_EXPECT_TRUE(FALSE);
		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("No board inserted!");
		errMsg.Print(this, g_lpszReportFilePath);
		return;
	}

	DWORD dwCaptureData = 0;
	nRetVal = dcm.SetPinGroup(nullptr, nullptr);
	if (VECTOR_FILE_NOT_LOADED != nRetVal)
	{
		///<Not warning when vector is not loaded
		XT_EXPECT_NE(VECTOR_FILE_NOT_LOADED, nRetVal);
		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("No Warning when vector is not loaded!");
	}

	//Load vector.
	dcm.LoadVectorFile(g_lpszVectorFilePath, FALSE);
	for (int nlpszPinGroupTestIndex = 0; nlpszPinGroupTestIndex < nlpszPinGroupTestCount; ++nlpszPinGroupTestIndex)
	{
		for (int nPinNameTestIndex = 0; nPinNameTestIndex < nPinNameTestCount; ++nPinNameTestIndex)
		{
			nRetVal = dcm.SetPinGroup(lpszPinGroup[nlpszPinGroupTestIndex], pinName[nPinNameTestIndex]);

			if (2 > nlpszPinGroupTestIndex || 3 > nPinNameTestIndex)
			{
				if (2 > nlpszPinGroupTestIndex || nullptr == lpszPinGroup[nlpszPinGroupTestIndex])
				{
					XT_EXPECT_EQ(nRetVal, PIN_GROUP_FORMAT_ERROR);
					if (PIN_GROUP_FORMAT_ERROR != nRetVal)
					{
						errMsg.AddNewError(STRING_ERROR_MSG);
						errMsg.SaveErrorMsg("Pin group format is error, the return value(%d) is not equal to %d!", nRetVal, PIN_GROUP_FORMAT_ERROR);
					}
				}
				else if (3 > nPinNameTestIndex || nullptr == pinName[nPinNameTestIndex])
				{
					XT_EXPECT_EQ(nRetVal, PIN_NAME_ERROR);
					if (PIN_NAME_ERROR != nRetVal)
					{
						errMsg.AddNewError(STRING_ERROR_MSG);
						errMsg.SaveErrorMsg("Pin name format is error, the return value(%d) is not equal to %d!", nRetVal, PIN_NAME_ERROR);
					}

				}
			}
			else
			{
				if (EXECUTE_SUCCESS != nRetVal)
				{
					int nStringType = dcm_GetStringType(lpszPinGroup[nlpszPinGroupTestIndex]);
					if (PIN_GROUP_NAME_CONFLICT == nRetVal && 1 == nStringType)
					{
						continue;
					}
					else
					{
						XT_EXPECT_TRUE(FALSE);
						errMsg.AddNewError(STRING_ERROR_MSG);
						errMsg.SaveErrorMsg("Unknown error, the return value is %d!", nRetVal);
					}
				}
			}
		}
	}

	errMsg.Print(this, g_lpszReportFilePath);

	dcm_CloseFile();//Unload the vector file.
}