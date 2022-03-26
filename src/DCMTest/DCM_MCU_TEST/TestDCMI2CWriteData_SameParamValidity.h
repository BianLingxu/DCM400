#pragma once
/**
 * @file TestDCMI2CWriteData_SameParamValidity.h
 * @brief Check the parameter validity of I2CWriteData
 * @author Guangyun Wang
 * @date 2021/08/02
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controller Technology Co., Ltd.
*/
#include "..\DCMTestMain.h"
XT_TEST(ParamValidityTest, TestDCMI2CWriteData_SameParamValidity)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, PARAM_VADILITY);
	CErrorMSG errMsg(strFuncName.c_str(), "ParamValidityTest");//Error message.
	double dPeriod = 0;
	int nRetVal = 0;

	const int nLengthTestCount = 8;
	double dLengthTestValue[nLengthTestCount] = { -1,0,2.5,1, I2C_WRITE_MAX_BYTE_COUNT + 1};
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

	BYTE abyData[5] = { 0xAA,0xBB,0x55,0xA5, 0x5A };
	nRetVal = dcm.I2CWriteData(0, 0, 5, abyData);
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

	for (auto Length : dLengthTestValue)
	{
		nRetVal = dcm.I2CWriteData(bySlaveAddr, byREGAddr, Length, abyData);
		if (I2C_WRITE_MAX_BYTE_COUNT < Length || 0 >= Length)
		{
			XT_EXPECT_EQ(nRetVal, -1);
			if (-1 != nRetVal)
			{
				errMsg.AddNewError(STRING_ERROR_MSG);
				char cDigit[10] = { 0 };
				_itoa_s(Length, cDigit, 10, 10);
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
	
	nRetVal = dcm.I2CWriteData(0xAA, 0xA5, I2C_WRITE_MAX_BYTE_COUNT, (BYTE*)nullptr);
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