#pragma once
/*!
* @file      TestDCMI2CWriteMultiDataParamValidity.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/10/17
* @version   v 1.0.0.0
* @brief     测试I2CWriteMultiData参数有效性
* @comment
*/
#include "..\DCMTestMain.h"

XT_TEST(ParamValidityTest, TestDCMI2CWriteMultiDataParamValidity)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, PARAM_VADILITY);
	CErrorMSG errMsg(strFuncName.c_str(), "ParamValidityTest");//Error message.
	double dPeriod = 0;
	int nRetVal = 0;

	const int nLengthTestCount = 8;
	double dLengthTestValue[nLengthTestCount] = { -1, 0, 2.5, I2C_WRITE_MAX_BYTE_COUNT, I2C_WRITE_MAX_BYTE_COUNT + 1};
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
	string strSCLChannel;
	string strSDAChannel;
	vector<CHANNEL_INFO> vecSDAChannel;
	vector<CHANNEL_INFO> vecSCLChannel;
	GetI2CChannel(mapSlot, strSCLChannel, strSDAChannel, vecSCLChannel, vecSDAChannel);
	USHORT uSiteCount = vecSCLChannel.size();

	const int TestDataByte = 5;
	BYTE byWriteWaveData[SITE_NUM][TestDataByte] = { 0 };
	BYTE* pbySiteData[SITE_NUM] = { nullptr };
	for (USHORT usSiteIndex = 0; usSiteIndex < uSiteCount; ++usSiteIndex)
	{
		pbySiteData[usSiteIndex] = byWriteWaveData[usSiteIndex];
		memset(byWriteWaveData, 0xAA, sizeof(byWriteWaveData));
	}

	nRetVal = dcm.I2CWriteMultiData(0, 0, TestDataByte, pbySiteData);
	XT_EXPECT_EQ(nRetVal, -2);
	if (-2 != nRetVal)
	{
		//Channel is not set.
		XT_EXPECT_TRUE(FALSE);
		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("No warning when channel is not set!");
	}

	dcm.I2CSet(500, SITE_NUM, DCM_REG8, strSCLChannel.c_str(), strSDAChannel.c_str());

	for (auto DataByteCount : dLengthTestValue)
	{
		nRetVal = dcm.I2CWriteMultiData(bySlaveAddr, byREGAddr, DataByteCount, pbySiteData);
		if (I2C_WRITE_MAX_BYTE_COUNT < DataByteCount || 0 >= DataByteCount)
		{
			XT_EXPECT_EQ(nRetVal, -1);
			if (-1 != nRetVal)
			{
				errMsg.AddNewError(STRING_ERROR_MSG);
				char lpszDigit[10] = { 0 };
				_itoa_s(DataByteCount, lpszDigit, 10, 10);
				errMsg.SetErrorItem("nDataLength", lpszDigit);
				errMsg.SaveErrorMsg("No warning when array length is error, the return value is %d!", nRetVal);
			}
		}
		else
		{
			XT_EXPECT_EQ(nRetVal, 0);
			if (0 != nRetVal)
			{
				//Return value is not right.
				errMsg.AddNewError(STRING_ERROR_MSG);
				errMsg.SaveErrorMsg("Unknown error, the return value is %d!", nRetVal);
			}
		}
	}

	///<Check parameter nullptr
	nRetVal = dcm.I2CWriteMultiData(0xAA, 0xA5, 4, nullptr);
	XT_EXPECT_EQ(nRetVal, -3);
	if (-3 != nRetVal)
	{
		///<Return value is not right
		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("The data array is nullptr, but the return value(%d) is not equal to -3", nRetVal);
	}

	nRetVal = dcm.I2CWriteMultiData(0xAA, 0xA5, I2C_WRITE_MAX_BYTE_COUNT, nullptr);
	XT_EXPECT_EQ(nRetVal, -3);
	if (-3 != nRetVal)
	{
		///<Return value is not right
		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("The data count equal maximum byte limited, but the return value(%d) is not equal to -3(array point is nullptr)", nRetVal);
	}

	errMsg.Print(this, g_lpszReportFilePath);


	dcm_CloseFile();//Unload the vector file.
	dcm_I2CDeleteMemory();
}