#pragma once
/*!
* @file      TestDCMI2CSetSCLEdgeParamValidity.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2020/02/03
* @version   v 1.0.0.0
* @brief     测试I2CSetSCLEdge参数有效性
* @comment
*/
#include "..\DCMTestMain.h"
XT_TEST(ParamValidityTest, TestDCMI2CSetSCLEdgeParamValidity)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, PARAM_VADILITY);
	CErrorMSG errMsg(strFuncName.c_str(), "ParamValidityTest");//Error message.
	double dPeriod = 500;
	int nRetVal = 0;
	const int nTimeTestCount = 5;
	double dTimeTestValue[nTimeTestCount] = { -3, 50, 60, 70,100 };
	

	map<BYTE, USHORT> mapSlot;
	nRetVal = GetBoardInfo(mapSlot, nullptr);
	if (0 == mapSlot.size())
	{
		//No board is inserted.
		XT_EXPECT_TRUE(FALSE);
		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("No board inserted!");
		errMsg.Print(this, g_lpszReportFilePath);
		return;
	}

	nRetVal = dcm.I2CSetSCLEdge(10, 160);
	if (-1 != nRetVal)
	{
		//Not warning when vector is not loaded.
		XT_EXPECT_EQ(VECTOR_FILE_NOT_LOADED, nRetVal);
		errMsg.AddNewError(STRING_ERROR_MSG);
		errMsg.SaveErrorMsg("No Warning when not set SCL/SDA channel, the return value is(%d).", nRetVal);
	}
	string strSCLChannel;
	string strSDAChannel;
	vector<CHANNEL_INFO> vecSDAChannel;
	vector<CHANNEL_INFO> vecSCLChannel;
	GetI2CChannel(mapSlot, strSCLChannel, strSDAChannel, vecSCLChannel, vecSDAChannel);
	USHORT uSiteCount = vecSCLChannel.size();

	dcm.I2CSet(dPeriod, uSiteCount, DCM_REG8, strSCLChannel.c_str(), strSDAChannel.c_str());

	//Check the CLK edge setting.
	int nTimeTestValidIndex = 1;
	dTimeTestValue[nTimeTestValidIndex] = dPeriod * 1 / 4;
	dTimeTestValue[nTimeTestValidIndex + 1] = dPeriod * 3 / 4;
	dTimeTestValue[nTimeTestCount - 2] = dPeriod;
	dTimeTestValue[nTimeTestCount - 1] = dPeriod + 10;

	for (int nT1R = 0; nT1R < nTimeTestCount; ++nT1R)
	{
		for (int nT1F = 0; nT1F < nTimeTestCount; ++nT1F)
		{
			nRetVal = dcm.I2CSetSCLEdge(dTimeTestValue[nT1R], dTimeTestValue[nT1F]);

			if (-EQUAL_ERROR > dTimeTestValue[nT1R]|| dTimeTestValue[nT1F] > dPeriod + EQUAL_ERROR || dTimeTestValue[nT1R] + 1 > dTimeTestValue[nT1F] + EQUAL_ERROR)
			{
				XT_EXPECT_EQ(nRetVal, -2);
				if (-2 != nRetVal)
				{
					errMsg.AddNewError(STRING_ERROR_MSG);
					char cMsg[256] = { 0 };
					sts_sprintf(cMsg, 256, "T1R:%.2f,T1F:%.2f", dTimeTestValue[nT1R], dTimeTestValue[nT1F]);
					errMsg.SetErrorItem("CLK edge", cMsg);
					errMsg.SaveErrorMsg("The clk edge is error but no warning, the return value is %d!", nRetVal);
					continue;
				}
			}
			else
			{
				XT_EXPECT_EQ(nRetVal, 0);
				if (0 != nRetVal)
				{
					errMsg.AddNewError(STRING_ERROR_MSG, "G_ALLPIN", 2);
					errMsg.SaveErrorMsg("Unknown error, the return value is %d!", nRetVal);
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