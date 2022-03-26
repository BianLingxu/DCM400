#pragma once
/*!
* @file      TestDCMI2CConnectParamValidity.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2020/2/3
* @version   v 1.0.0.0
* @brief     测试I2CConnect参数有效性
* @comment
*/
#include "..\DCMTestMain.h"
XT_TEST(ParamValidityTest, TestDCMI2CConnectParamValidity)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, PARAM_VADILITY);
	CErrorMSG errMsg(strFuncName.c_str(), "ParamValidityTest");//Error message.
	double dPeriod = 500;
	int nRetVal = 0;

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

	nRetVal = dcm.I2CConnect();
	if (-1 != nRetVal)
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
	dcm.I2CSet(dPeriod, uSiteCount, DCM_REG8, strSCLChannel.c_str(), strSDAChannel.c_str());

	//Check the pin level.
	nRetVal = dcm.I2CConnect();
	XT_EXPECT_EQ(nRetVal, 0);
	if (0 != nRetVal)
	{
		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("Unknown error, the return value(%d) is not equal to expectvalue (%d)!", nRetVal, 0);
	}
	dcm.I2CDisconnect();
	errMsg.Print(this, g_lpszReportFilePath);

	mapSlot.clear();
	vecSCLChannel.clear();
	vecSDAChannel.clear();

	dcm_CloseFile();//Unload the vector file.
	dcm_I2CDeleteMemory();
}