#pragma once
/*!
* @file      TestDCMLoadVectorFileParamValidity.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/10/09
* @version   v 1.0.0.0
* @brief     测试LoadVectorFile参数有效性
* @comment
*/

#include "..\DCMTestMain.h"
XT_TEST(ParamValidityTest, TestDCMLoadVectorFileParamValidity)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, PARAM_VADILITY);
	CErrorMSG errMsg(strFuncName.c_str(), "ParamValidityTest");//Error message.
	int nRetVal = 0;
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
	
	const int nFileCount = 4;
	const char* lpszFilePath[nFileCount] = { nullptr, "NOTFile","F:\\HardwareDiver\\STS8250\\trunk\\src\\DCMTest\\oldVector.vec", g_lpszReportFilePath };
	
	for (int nTestIndex = 0; nTestIndex < nFileCount; ++nTestIndex)
	{
		for (int nLoadVector = 0; nLoadVector < 2; ++nLoadVector)
		{
			nRetVal = dcm.LoadVectorFile(lpszFilePath[nTestIndex], BOOL(nLoadVector));
			if (2 > nTestIndex)
			{
				XT_EXPECT_EQ(nRetVal, VECTOR_FILE_NOT_LOADED);
				if (VECTOR_FILE_NOT_LOADED != nRetVal)
				{
					errMsg.AddNewError(STRING_ERROR_MSG);
					errMsg.SaveErrorMsg("No warning when the vector file is error, the return value is %d!", nRetVal);
				}
			}
			else
			{
				if (EXECUTE_SUCCESS != nRetVal && (BOARD_NOT_INSERT_ERROR == nRetVal && 2 != nTestIndex))
				{
					errMsg.AddNewError(STRING_ERROR_MSG);
					errMsg.SaveErrorMsg("Unknown error, the return value is %d!", nRetVal);
				}
			}
		}
	}

	errMsg.Print(this, g_lpszReportFilePath);

	dcm_CloseFile();//Unload the vector file.
}