#pragma once
/*!
* @file      TestDCMI2CSetSDAEdgeParamValidity.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2020/02/03
* @version   v 1.0.0.0
* @brief     测试I2CSetSDAEdge参数有效性
* @comment
*/
#include "..\DCMTestMain.h"
XT_TEST(ParamValidityTest, TestDCMI2CSetSDAEdgeParamValidity)
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

	nRetVal = dcm.I2CSetSDAEdge(10, 160, 10, 60);
	if (-1 != nRetVal)
	{
		XT_EXPECT_EQ(nRetVal, -1);
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
			for (int nIOR = 0; nIOR < nTimeTestCount; ++nIOR)
			{
				for (int nSTBR = 0; nSTBR < nTimeTestCount; ++nSTBR)
				{
					nRetVal = dcm.I2CSetSDAEdge(dTimeTestValue[nT1R], dTimeTestValue[nT1F], dTimeTestValue[nIOR], dTimeTestValue[nSTBR]);

					if (-EQUAL_ERROR > dTimeTestValue[nT1R] || -EQUAL_ERROR > dTimeTestValue[nT1F]
						|| dTimeTestValue[nT1R] > dPeriod + EQUAL_ERROR || dTimeTestValue[nT1F] > dPeriod + EQUAL_ERROR
						|| -EQUAL_ERROR > dTimeTestValue[nIOR] || dTimeTestValue[nIOR] > dPeriod + EQUAL_ERROR 
						|| -EQUAL_ERROR > dTimeTestValue[nSTBR] || dTimeTestValue[nSTBR] > dPeriod + EQUAL_ERROR)
					{
						XT_EXPECT_EQ(nRetVal, -2);
						if (-2 != nRetVal)
						{
							errMsg.AddNewError(STRING_ERROR_MSG);
							char cMsg[256] = { 0 };
							sts_sprintf(cMsg, 256, "T1R:%.2f,T1F:%.2f,IOR:%.2f,STBR:%.2f", dTimeTestValue[nT1R], dTimeTestValue[nT1F],
								dTimeTestValue[nIOR], dTimeTestValue[nSTBR]);
							errMsg.SetErrorItem("CLK edge", cMsg);
							errMsg.SaveErrorMsg("The edge is error but no warning, the return value is %d!", nRetVal);
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
		}
	}

	errMsg.Print(this, g_lpszReportFilePath);
	mapSlot.clear();
	vecSCLChannel.clear();
	vecSDAChannel.clear();

	dcm_CloseFile();//Unload the vector file.
	dcm_I2CDeleteMemory();
}