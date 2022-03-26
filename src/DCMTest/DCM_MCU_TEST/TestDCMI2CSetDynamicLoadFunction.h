#pragma once
/**
 * @file TestDCMI2CSetDynamicLoadFunction.h
 * @brief Check the function of I2CSetDynamicLoad
 * @author Guangyun Wang
 * @date 2021/06/02
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controller Technology Co., Ltd.
*/

#include "..\DCMTestMain.h"
XT_TEST(FunctionFunctionTest, TestDCMI2CSetDynamicLoadFunction)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, FUNCTION);
	CFuncReport FuncReport(strFuncName.c_str(), "FunctionFunctionTest");

	map<BYTE, USHORT> mapSlot;
	int nRetVal = GetBoardInfo(mapSlot, g_lpszI2CVectorFilePath);
	if (0 == mapSlot.size())
	{
		//No board is inserted.
		XT_EXPECT_TRUE(FALSE);
		FuncReport.SetNoBoardValid();
		FuncReport.Print(this, g_lpszReportFilePath);
		return;
	}

	dcm.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");

	SaveBoardSN(FuncReport, mapSlot);
	auto iterSlot = mapSlot.begin();

	string strSCLChannel;
	string strSDAChannel;
	vector<CHANNEL_INFO> vecSDAChannel;
	vector<CHANNEL_INFO> vecSCLChannel;
	vector<CHANNEL_INFO>* pavecChannel[2] = { &vecSCLChannel, &vecSDAChannel };
	GetI2CChannel(mapSlot, strSCLChannel, strSDAChannel, vecSCLChannel, vecSDAChannel);
	USHORT usSiteCount = vecSCLChannel.size();
	dcm.I2CSet(1000, usSiteCount, DCM_REG8, strSCLChannel.c_str(), strSDAChannel.c_str());

	string strSCLMutualPinGroup;
	string strSDAMutualPinGroup;
	string strI2CMutualPinGroup;
	string strI2CPinGroup;
	vector<CHANNEL_INFO> vecSCLMutual;
	vector<CHANNEL_INFO> vecSDAMutual;
	vector<CHANNEL_INFO> vecI2CMutual;
	vector<CHANNEL_INFO> vecI2CChannel;
	CHANNEL_INFO ChannelInfo;
	BOOL bSCL = TRUE;
	char lpszChannel[32] = { 0 };
	USHORT usMutualChannel = 0;
	for (auto& pI2CChannel : pavecChannel)
	{
		for (auto& Channel : *pI2CChannel)
		{
			usMutualChannel = Channel.m_usChannel;
			if (2 <= usMutualChannel % 4)
			{
				usMutualChannel -= 2;
			}
			else
			{
				usMutualChannel += 2;
			}
			sprintf_s(lpszChannel, sizeof(lpszChannel), "CH%d,", usMutualChannel % DCM_CHANNELS_PER_CONTROL);
			vecI2CChannel.push_back(Channel);


			strI2CMutualPinGroup += lpszChannel;
			ChannelInfo = Channel;
			ChannelInfo.m_usChannel = usMutualChannel;
			vecI2CMutual.push_back(ChannelInfo);
			if (bSCL)
			{
				strSCLMutualPinGroup += lpszChannel;
				vecSCLMutual.push_back(ChannelInfo);
			}
			else
			{
				strSDAMutualPinGroup += lpszChannel;
				vecSDAMutual.push_back(ChannelInfo);
			}
			sprintf_s(lpszChannel, sizeof(lpszChannel), "CH%d,", Channel.m_usChannel % DCM_CHANNELS_PER_CONTROL);
			strI2CPinGroup += lpszChannel;
		}
		bSCL = FALSE;
	}
	dcm.SetPinGroup("G_SCLMUTUAL", strSCLMutualPinGroup.c_str());
	dcm.SetPinGroup("G_SDAMUTUAL", strSDAMutualPinGroup.c_str());
	dcm.SetPinGroup("G_I2CMUTUAL", strI2CMutualPinGroup.c_str());
	dcm.SetPinGroup("G_I2C", strI2CPinGroup.c_str());
	strSCLMutualPinGroup = "G_SCLMUTUAL";
	strSDAMutualPinGroup = "G_SDAMUTUAL";
	strI2CMutualPinGroup = "G_I2CMUTUAL";
	strI2CPinGroup = "G_I2C";


	double dIOH = 1e-3;
	double dIOL = 2e-3;

	vector<CHANNEL_INFO>* pvecDynamic = nullptr;
	vector<CHANNEL_INFO>* pvecPMU = nullptr;
	string* pstrPinGroup = nullptr;
	auto CheckCurrent = [&](DCM_I2C_CHANNEL I2CChannel, BOOL bIOL)
	{
		switch (I2CChannel)
		{
		case DCM_I2C_SCL:
			pvecPMU = &vecSCLMutual;
			pvecDynamic = &vecSCLChannel;
			pstrPinGroup = &strSCLMutualPinGroup;
			break;
		case DCM_I2C_SDA:
			pvecPMU = &vecSDAMutual;
			pvecDynamic = &vecSDAChannel;
			pstrPinGroup = &strSDAMutualPinGroup;
			break;
		case DCM_I2C_BOTH:
			pvecPMU = &vecI2CMutual;
			pvecDynamic = &vecI2CChannel;
			pstrPinGroup = &strI2CMutualPinGroup;
			break;
		default:
			break;
		}

		double dExpected = bIOL ? -dIOL : dIOH;
		dcm.PPMUMeasure(pstrPinGroup->c_str(), 10, 10);
		int nPinCount = pvecPMU->size();
		USHORT usSiteNo = 0;
		double dReal = 0;
		int nCheckIndex = 0;
		for (auto& Channel : *pvecPMU)
		{
			auto iterSlot = mapSlot.find(Channel.m_bySlotNo);
			if (mapSlot.end() == iterSlot)
			{
				///<Not will happen
				++nCheckIndex;
				continue;
			}
			char lpszPin[32] = { 0 };
			sprintf_s(lpszPin, sizeof(lpszPin), "CH%d", Channel.m_usChannel% DCM_CHANNELS_PER_CONTROL);
			usSiteNo = iterSlot->second + Channel.m_usChannel / DCM_CHANNELS_PER_CONTROL;
			dReal = dcm.GetPPMUMeasResult(lpszPin, usSiteNo);
			
			XT_EXPECT_REAL_EQ(dReal, dExpected, 2e-4);
			if (2e-4 < fabs(dReal - dExpected))
			{
				FuncReport.SaveFailChannel(pvecDynamic->at(nCheckIndex).m_bySlotNo, pvecDynamic->at(nCheckIndex).m_usChannel);
			}
			++nCheckIndex;
		}
	};
	dcm.Connect("G_ALLPIN");
	///<Check IOL
	FuncReport.AddTestItem("Check IOL");
	dcm.SetPPMU(strI2CMutualPinGroup.c_str(), DCM_PPMU_FVMI, 0, DCM_PPMUIRANGE_2MA);
	dcm.I2CSetDynamicLoad(DCM_I2C_BOTH, TRUE, dIOH, dIOL, 3);
	dcm.SetChannelStatus(strI2CPinGroup.c_str(), DCM_ALLSITE, DCM_HIGH_IMPEDANCE);
	CheckCurrent(DCM_I2C_BOTH, TRUE);
	dcm.I2CSetDynamicLoad(DCM_I2C_BOTH, FALSE, dIOH, dIOL, 3);

	///<Check IOH
	FuncReport.AddTestItem("Check IOH");
	dcm.SetPPMU(strI2CMutualPinGroup.c_str(), DCM_PPMU_FVMI, 3.5, DCM_PPMUIRANGE_2MA);
	dcm.I2CSetDynamicLoad(DCM_I2C_BOTH, TRUE, dIOH, dIOL, 3);
	dcm.SetChannelStatus(strI2CPinGroup.c_str(), DCM_ALLSITE, DCM_HIGH_IMPEDANCE);
	CheckCurrent(DCM_I2C_BOTH, FALSE);
	dcm.I2CSetDynamicLoad(DCM_I2C_BOTH, FALSE, dIOH, dIOL, 3);

	///<Check Channel SCL:IOL SDA:IOH
	FuncReport.AddTestItem("Check Channel(SCL:IOL, SDA:IOH)");
	dcm.SetPPMU(strI2CMutualPinGroup.c_str(), DCM_PPMU_FVMI, 3, DCM_PPMUIRANGE_2MA);
	dcm.I2CSetDynamicLoad(DCM_I2C_SCL, TRUE, dIOH, dIOL, 3.5);
	dcm.I2CSetDynamicLoad(DCM_I2C_SDA, TRUE, dIOH, dIOL, 2);
	dcm.SetChannelStatus(strI2CPinGroup.c_str(), DCM_ALLSITE, DCM_HIGH_IMPEDANCE);
	CheckCurrent(DCM_I2C_SCL, TRUE);
	CheckCurrent(DCM_I2C_SDA, FALSE);
	dcm.I2CSetDynamicLoad(DCM_I2C_BOTH, FALSE, dIOH, dIOL, 3);

	///<Check Channel SCL:IOH SDA:IOL
	FuncReport.AddTestItem("Check Channel(SCL:IOH, SDA:IOL)");
	dcm.SetPPMU(strI2CMutualPinGroup.c_str(), DCM_PPMU_FVMI, 3, DCM_PPMUIRANGE_2MA);
	dcm.I2CSetDynamicLoad(DCM_I2C_SCL, TRUE, dIOH, dIOL, 2);
	dcm.I2CSetDynamicLoad(DCM_I2C_SDA, TRUE, dIOH, dIOL, 3.5);
	dcm.SetChannelStatus(strI2CPinGroup.c_str(), DCM_ALLSITE, DCM_HIGH_IMPEDANCE);
	CheckCurrent(DCM_I2C_SCL, FALSE);
	CheckCurrent(DCM_I2C_SDA, TRUE);
	dcm.I2CSetDynamicLoad(DCM_I2C_BOTH, FALSE, dIOH, dIOL, 3);

	dcm.InitMCU("G_ALLPIN");
	dcm.Disconnect("G_ALLPIN");
	dcm_CloseFile();
	FuncReport.Print(this, g_lpszReportFilePath);
}