/*!
* @file      TestDCMI2CReadDataParamValidity.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/10/17
* @version   v 1.0.0.0
* @brief     测试I2CReadData参数有效性
* @comment
*/
#include "..\DCMTestMain.h"

XT_TEST(ParamValidityTest, TestDCMI2CReadDataParamValidity)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, PARAM_VADILITY);
	CErrorMSG errMsg(strFuncName.c_str(), "ParamValidityTest");//Error message.
	int nRetVal = 0;
	const int nLengthTestCount = 6;
	double dLengthTestValue[nLengthTestCount] = { -1, 0, 2.5, 6, I2C_READ_MAX_BYTE_COUNT, I2C_READ_MAX_BYTE_COUNT + 1 };
	BYTE bySlaveAddr = 0x00;
	BYTE byREGAddr = 0xA55A55AA;
	map<BYTE, USHORT> mapSlot;
	nRetVal = GetBoardInfo(mapSlot, g_lpszVectorFilePath);
	if (0 == mapSlot.size())
	{
		///<No board is inserted
		XT_EXPECT_TRUE(FALSE);
		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("No board inserted!");
		errMsg.Print(this, g_lpszReportFilePath);
		return;
	}

	nRetVal = dcm.I2CReadData(0, 0, 2);
	XT_EXPECT_EQ(nRetVal, -3);
	if (-3 != nRetVal)
	{
		///<Channel is not set
		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("No warning when channel is not set!");
	}

	string strSCLChannel;
	string strSDAChannel;
	vector<CHANNEL_INFO> vecSDAChannnel;
	vector<CHANNEL_INFO> vecSCLChannel;
	GetI2CChannel(mapSlot, strSCLChannel, strSDAChannel, vecSCLChannel, vecSDAChannnel);
	USHORT uSiteCount = vecSDAChannnel.size();

	dcm.I2CSet(500, uSiteCount, DCM_REG8, strSCLChannel.c_str(), strSDAChannel.c_str());

	//Check the pin level.
	for (auto Length : dLengthTestValue)
	{
		nRetVal = dcm.I2CReadData(bySlaveAddr, byREGAddr, Length);
		if (I2C_READ_MAX_BYTE_COUNT < Length || 0 >= Length)
		{
			XT_EXPECT_EQ(nRetVal, -1);
			if (-1 != nRetVal)
			{
				errMsg.AddNewError(STRING_ERROR_MSG);
				char lspzDigit[10] = { 0 };
				_itoa_s(Length, lspzDigit, 10, 10);
				errMsg.SetErrorItem("nDataCount", lspzDigit);
				errMsg.SaveErrorMsg("No warning when data count of read is over scale, the return value is %d!", nRetVal);
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

	errMsg.Print(this, g_lpszReportFilePath);


	dcm_CloseFile();//Unload the vector file.
	dcm_I2CDeleteMemory();
}