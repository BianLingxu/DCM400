#pragma once
/*!
* @file      TestDCMI2CGetBitDataParamValidity.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2020/2/3
* @version   v 1.0.0.0
* @brief     测试I2CGetBitData参数有效性
* @comment
*/
#include "..\DCMTestMain.h"
XT_TEST(ParamValidityTest, TestDCMI2CGetBitDataParamValidity)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, PARAM_VADILITY);
	CErrorMSG errMsg(strFuncName.c_str(), "ParamValidityTest");//Error message.
	double dPeriod = 500;
	ULONG ulRetVal = 0;
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

	ulRetVal = dcm.I2CGetBitData(0, 0, 1);
	XT_EXPECT_EQ(ulRetVal, (ULONG)0xFFFFFFFF);
	if (0xFFFFFFFF != ulRetVal)
	{
		//Channel is not set.
		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("No warning when channel is not set!");
	}

	string strSCLChannel;
	string strSDAChannel;
	vector<CHANNEL_INFO> vecSDAChannel;
	vector<CHANNEL_INFO> vecSCLChannel;
	GetI2CChannel(mapSlot, strSCLChannel, strSDAChannel, vecSCLChannel, vecSDAChannel);
	USHORT usSiteCount = vecSCLChannel.size();
	int nReadDataByte = 3;

	dcm.I2CSet(500, 2, DCM_REG8, "S0_0,S0_1", "S0_2,S0_3");

	dcm.I2CReadData(0xA0, 0xAA, nReadDataByte);
	ulRetVal = dcm.I2CGetBitData(0, 0, 0);
	XT_EXPECT_EQ(ulRetVal, (ULONG)0xFFFFFFFF);
	if (0xFFFFFFFF != ulRetVal)
	{
		//Channel is not set.
		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("No warning when board of site is not inserted!");
	}

	dcm.I2CSet(dPeriod, usSiteCount, DCM_REG8, strSCLChannel.c_str(), strSDAChannel.c_str());

	ulRetVal = dcm.I2CGetBitData(0, 0, 0);
	XT_EXPECT_EQ(ulRetVal, (ULONG)0xFFFFFFFF);
	if (0xFFFFFFFF != ulRetVal)
	{
		//Channel is not set.
		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("No warning when not read data!");
	}

	dcm.I2CReadData(0xA0, 0xAA, nReadDataByte);
	const BYTE byTestSiteCount = 4;
	
	BYTE byBitsCount = nReadDataByte * 8;
	USHORT usTestSite[byTestSiteCount] = {0, 2, usSiteCount, usSiteCount + 1};
	const BYTE byTestStartBitCount = 5;
	BYTE byStartBit[byTestStartBitCount] = { -1, 0,1,byBitsCount, byBitsCount + 1 };
	for (BYTE byTestSiteIndex = 0; byTestSiteIndex < byTestSiteCount; ++byTestSiteIndex)
	{
		for (BYTE byStartBitIndex = 0; byStartBitIndex < byTestStartBitCount; ++byStartBitIndex)
		{
			for (int nBitCount = -1; nBitCount < 33; ++nBitCount)
			{
				ulRetVal = dcm.I2CGetBitData(usTestSite[byTestSiteIndex], byStartBit[byStartBitIndex], nBitCount);
				if (usSiteCount <= usTestSite[byTestSiteIndex])
				{
					XT_EXPECT_EQ(ulRetVal, (ULONG)0xFFFFFFFF);
					if (0xFFFFFFFF != ulRetVal)
					{
						errMsg.AddNewError(STRING_ERROR_MSG);
						errMsg.SaveErrorMsg("Site(%d) is error, but the return value(0x%X) is not equal expect(0x%X)!", usTestSite[byTestSiteIndex], ulRetVal, 0xFFFFFFFF);
						continue;
					}
				}
				else if (byTestStartBitCount <= byStartBit[byStartBitIndex])
				{
					XT_EXPECT_EQ(ulRetVal, (ULONG)0xFFFFFFFF);
					if (0xFFFFFFFF != ulRetVal)
					{
						errMsg.AddNewError(STRING_ERROR_MSG);
						errMsg.SaveErrorMsg("Start bit(%d) is error, but the return value(0x%X) is not equal expect(0x%X)!", byStartBit[byStartBitIndex], ulRetVal, 0xFFFFFFFF);
						continue;
					}
				}
				else if (0 >= nBitCount || byBitsCount < nBitCount + byStartBit[byStartBitIndex])
				{
					XT_EXPECT_EQ(ulRetVal, (ULONG)0xFFFFFFFF);
					if (0xFFFFFFFF != ulRetVal)
					{
						errMsg.AddNewError(STRING_ERROR_MSG);
						errMsg.SaveErrorMsg("Get bits count(%d) is over range[%d, %d], but the return value(0x%X) is not equal expect(0x%X)!", nBitCount, 0, byBitsCount - byStartBit[byStartBitIndex], ulRetVal, 0xFFFFFFFF);
						continue;
					}
				}
				else if (0xFFFFFFFF == ulRetVal)
				{
					XT_EXPECT_NE(ulRetVal, (ULONG)0xFFFFFFFF);
					if (0xFFFFFFFF != ulRetVal)
					{
						errMsg.AddNewError(STRING_ERROR_MSG);
						errMsg.SaveErrorMsg("Unknown error, the return value(0x%X) is not equal expect(0x%X)!", ulRetVal, 0xFFFFFFFF);
						continue;
					}
				}
			}
		}
	}

	errMsg.Print(this, g_lpszReportFilePath);

	mapSlot.clear();
	vecSCLChannel.clear();
	vecSDAChannel.clear();

	dcm_CloseFile();//Unload the vector file.
	dcm_I2CDeleteMemory();
}