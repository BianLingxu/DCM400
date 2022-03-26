#pragma once
/*!
* @file      TestDCMI2CWriteDataParamValidity.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/10/13
* @version   v 1.0.0.0
* @brief     测试I2CWriteData参数有效性
* @comment
*/
#include "..\DCMTestMain.h"
XT_TEST(ParamValidityTest, TestDCMI2CWriteDataParamValidity)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, PARAM_VADILITY);
	CErrorMSG errMsg(strFuncName.c_str(), "ParamValidityTest");//Error message.
	double dPeriod = 0;
	int nRetVal = 0;

	const int nLengthTestCount = 8;
	double dLengthTestValue[nLengthTestCount] = {-1,0,2.5,1,2,3,4,5};
	BYTE bySlaveAddr = 0x00;
	BYTE byREGAddr = 0xA0;

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

	ULONG ulWriteData[SITE_NUM] = { 0 };
	nRetVal = dcm.I2CWriteData(0, 0, 2, ulWriteData);
	if (-2 != nRetVal)
	{
		//Channel is not set.

		XT_EXPECT_TRUE(FALSE);

		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("No warning when channel is not set!");
	}

	string strSCLChannel;
	string strSDAChannel;
	vector<CHANNEL_INFO> vecSDAChannel;
	vector<CHANNEL_INFO> vecSCLChannel;
	GetI2CChannel(mapSlot, strSCLChannel, strSDAChannel, vecSCLChannel, vecSDAChannel);
	USHORT uSiteCount = vecSCLChannel.size();
	//Load vector.
	dcm.I2CSet(500, uSiteCount, DCM_REG8, strSCLChannel.c_str(), strSDAChannel.c_str());

	//Check the pin level.
	for (int nLengthTestIndex = 0; nLengthTestIndex < nLengthTestCount; ++ nLengthTestIndex)
	{
		nRetVal = dcm.I2CWriteData(bySlaveAddr, byREGAddr, dLengthTestValue[nLengthTestIndex], ulWriteData);
		if (0 >= dLengthTestValue[nLengthTestIndex] || 4 < dLengthTestValue[nLengthTestIndex])
		{
			XT_EXPECT_EQ(nRetVal, -1);
			if (-1 != nRetVal)
			{
				errMsg.AddNewError(STRING_ERROR_MSG);
				char cDigit[10] = { 0 };
				_itoa_s(dLengthTestValue[nLengthTestIndex], cDigit, 10, 10);
				errMsg.SetErrorItem("nDataLength", cDigit);
				errMsg.SaveErrorMsg("No warning when array length is error, the return value is %d!", nRetVal);
			}
		}
		else
		{
			XT_EXPECT_EQ(nRetVal, 0);
			if (0 != nRetVal)
			{
				///<Return value is not right
				errMsg.AddNewError(STRING_ERROR_MSG);
				errMsg.SaveErrorMsg("Unknown error, the return value is %d!", nRetVal);
			}
		}
	}
	
	///<Check parameter nullptr
	nRetVal = dcm.I2CWriteData(0xAA, 0xA5, 4, (BYTE*)nullptr);
	XT_EXPECT_EQ(nRetVal, -3);
	if (-3 != nRetVal)
	{
		///<Return value is not right
		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("The data array is nullptr, but the return value(%d) is not equal to -3", nRetVal);
	}
	
	errMsg.Print(this, g_lpszReportFilePath);


	dcm_CloseFile();//Unload the vector file.
	dcm_I2CDeleteMemory();
}