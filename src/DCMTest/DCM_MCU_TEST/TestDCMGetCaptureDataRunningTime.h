#pragma once
/*!
* @file      TestDCMGetCaptureDataRunningTime.h
*
* Copyright (C) 北京华峰测控技术股份有限公司
*
* @author    Guangyun Wang
* @date      2017/12/12
* @version   v 1.0.0.0
* @brief     测试GetCaptureData运行时间
* @comment
*/
#include "..\DCMTestMain.h"
#include "..\PTE.h"
class CGetCaptureDataPTE : public CPTE
{
public:
	/**
	 * @brief Constructor
	 * @param[in] dcm The instance of DCM
	*/
	CGetCaptureDataPTE(DCM& dcm);
	/**
	 * @brief The function whose PTE will be calculated
	*/
	virtual void FuncExecute();
	/**
	 * @brief Reset the status afer function execution
	*/
	virtual void ResetStatus();
	/**
	 * @brief Set the running parameter
	 * @param lpszStartLabel The start label of the vector will be ran
	 * @param lpszStopLabel The stop label of vector will be ran
	 * @return Execute result
	 * - 0 Set the running parameter successfully
	 * - -1 The start label is nullptr
	 * - -2 The start label is not existed
	 * - -3 The stop label is nullptr
	 * - -4 The stop label is not existed
	 * - -5 The stop label is not behind the start label
	*/
	int SetRunParam(const char* lpszStartLabel, const char* lpszStopLabel);
	/**
	 * @brief Set the parameter of GetCaptureData
	 * @param[in] vecChannel The channel which one of them be gotten
	 * @param[in] lpszLabel The start label
	 * @param[in] usSiteNo The site number
	 * @param[in] nOffset The start line number offset to start label
	 * @param[in] nLineCount The line count read
	*/
	int SetParam(const std::vector<CHANNEL_INFO>& vecChannel, const char* lpszLabel, int nOffset, int nLineCount);
	/**
	 * @brief The operation when channel distribution had been redistributed
	 * @return Execute result
	 * - 0 Continue
	 * - -1 Abort
	*/
	virtual int ChannelRedistribution();
private:
	/**
	 * @brief Check the pin name whose data will be gotten
	 * @return Execute result
	 * - 0 Find the pin name
	 * - -1 Can't find pin name
	*/
	int UpdatePinName();
private:
	DCM* m_pDCM;///<The point to the instance of DCM
	std::string m_strLabel;///<The start label
	int m_nOffset;///<The offset line number refer to the start label
	int m_nLineCount;///<The line count read
	std::vector<CHANNEL_INFO> m_vecChannel;///<The channels data will be gotten
	std::string m_strPinName;///<The pin name whose data will be gotten
	std::string m_strStartLabel;///<The start label of the vector will be ran
	std::string m_strStopLabel;///<The stop label of the vector will be ran
};

CGetCaptureDataPTE::CGetCaptureDataPTE(DCM& dcm)
	: m_pDCM(&dcm)
	, m_nOffset(0)
	, m_nLineCount(0)
{
}

void CGetCaptureDataPTE::FuncExecute()
{
	ULONG ulCapture = 0;
	for (USHORT usSiteNo = 0; usSiteNo < m_usSiteCount; ++usSiteNo)
	{
		dcm.GetCaptureData(m_strPinName.c_str(), m_strLabel.c_str(), usSiteNo, m_nOffset, m_nLineCount, ulCapture);
	}
}

void CGetCaptureDataPTE::ResetStatus()
{
	dcm.RunVectorWithGroup(m_strAllPin.c_str(), m_strStartLabel.c_str(), m_strStopLabel.c_str());
}

int CGetCaptureDataPTE::SetRunParam(const char* lpszStartLabel, const char* lpszStopLabel)
{
	if (nullptr == lpszStartLabel)
	{
		return -1;
	}
	int nStartLine = dcm_GetLabelLineNo(lpszStartLabel);
	if (0 > nStartLine)
	{
		return -2;
	}
	if (nullptr == lpszStopLabel)
	{
		return -3;
	}
	int nStopLine = dcm_GetLabelLineNo(lpszStopLabel);
	if (0 > nStopLine)
	{
		return -4;
	}
	if (nStopLine <= nStartLine)
	{
		return -5;
	}
	m_strStartLabel = lpszStartLabel;
	m_strStopLabel = lpszStopLabel;
	return 0;
}

int CGetCaptureDataPTE::SetParam(const std::vector<CHANNEL_INFO>& vecChannel, const char* lpszLabel, int nOffset, int nLineCount)
{
	m_strLabel = lpszLabel;
	m_nOffset = nOffset;
	m_nLineCount = nLineCount;
	m_vecChannel = vecChannel;	
	return 0;
}

int CGetCaptureDataPTE::ChannelRedistribution()
{
	int nRetVal = UpdatePinName();
	if (0 == nRetVal)
	{
		return 0;
	}
	return -1;
}

int CGetCaptureDataPTE::UpdatePinName()
{
	int nSiteNo = -1;
	for (auto& Channel : m_vecChannel)
	{
		for (auto& Pin : m_mapPinInfo)
		{
			nSiteNo = Pin.second->GetSiteNo(Channel);
			if (0 <= nSiteNo)
			{
				m_strPinName = Pin.first;
				break;
			}
		}
		if (0 <= nSiteNo)
		{
			break;
		}
	}
	if (0 > nSiteNo)
	{
		return -1;
	}
	return 0;
}

XT_TEST(FunctionRunningTimeTest, TestDCMGetCaptureDataRunningTime)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, RUNNING_TIME);
	CTimeReport timeReport(strFuncName.c_str(), "FunctionRunningTimeTest");
	
	int nRetVal = 0;
	map<BYTE, USHORT> mapSlot;
	nRetVal = GetBoardInfo(mapSlot, g_lpszVectorFilePath);

	if (0 == mapSlot.size())
	{
		//No board is inserted.
		XT_EXPECT_TRUE(FALSE);
		timeReport.SetNoBoardValid();
		timeReport.Print(this, g_lpszReportFilePath);
		return;
	}

	XT_EXPECT_EQ(nRetVal, 0);
	if (0 != nRetVal)
	{
		timeReport.addMsg("Load vector file(%s) fail, the vector file maybe not right.", g_lpszVectorFilePath);
		timeReport.Print(this, g_lpszReportFilePath);
		return;
	}


	SaveBoardSN(timeReport, mapSlot);
	auto iterSlot = mapSlot.begin();


	dcm.LoadVectorFile(g_lpszVectorFilePath, FALSE);
	dcm.SetPinGroup("G_ALLPIN", "CH0,CH1,CH2,CH3,CH4,CH5,CH6,CH7,CH8,CH9,CH10,CH11,CH12,CH13,CH14,CH15");

	dcm.Connect("G_ALLPIN");
	dcm.SetPinLevel("G_ALLPIN", 3.0, 0, 1.5, 0.8);
	double dPeriod = dcm_GetTimeSetPeriod(iterSlot->first, 0, 0);
	dcm.SetEdge("G_ALLPIN", "0", DCM_DTFT_NRZ, dPeriod / 8, dPeriod * 5 / 8, dPeriod / 8, dPeriod * 3 / 8);

	dcm.RunVectorWithGroup("G_ALLPIN", "FAIL_BRAM_ST", "FAIL_BRAM_SP");
	ULONG ulCaptureData = 0;

	timeReport.timeStart();
	dcm.GetCaptureData("CH0", "FAIL_BRAM_ST", iterSlot->second, 10, 8, ulCaptureData);
	timeReport.timeStop();
	timeReport.addMsg("Get %d lines vector data in BRAM", 8);

	timeReport.timeStart();
	dcm.GetCaptureData("CH0", "FAIL_BRAM_ST", iterSlot->second, 10, 16, ulCaptureData);
	timeReport.timeStop();
	timeReport.addMsg("Get %d lines vector data in BRAM", 16);

	dcm.RunVectorWithGroup("G_ALLPIN", "FAIL_DRAM_ST", "FAIL_DRAM_SP");
	
	timeReport.timeStart();
	dcm.GetCaptureData("CH0", "FAIL_DRAM_ST", iterSlot->second, 20, 8, ulCaptureData);
	timeReport.timeStop();
	ULONG ulFailCount = 0;
	dcm.GetFailCount("CH0", iterSlot->second, ulFailCount);
	timeReport.addMsg("Get %d lines vector data in DRAM, fail %d", 8, ulFailCount);

	timeReport.timeStart();
	dcm.GetCaptureData("CH0", "FAIL_DRAM_ST", iterSlot->second, 20, 16, ulCaptureData);
	timeReport.timeStop();
	timeReport.addMsg("Get %d lines vector data in DRAM", 16);


	///<Get the PTE data
	vector<CHANNEL_INFO> vecChannel;
	for (auto& Slot : mapSlot)
	{
		for (USHORT usChannel = 0; usChannel < DCM_MAX_CHANNELS_PER_BOARD;++usChannel)
		{
			vecChannel.push_back(CHANNEL_INFO(Slot.first, usChannel));
		}
	}

	timeReport.SetAdditionItemTittle("PTE Check");
	CGetCaptureDataPTE PTEData(dcm);
	PTEData.SetRunParam("FAIL_BRAM_ST", "FAIL_BRAM_SP");
	PTEData.SetParam(vecChannel, "FAIL_BRAM_ST", 10, 16);
	double dPTE = PTEData.GetPTE(1, FALSE, 4, 4, 10);
	timeReport.AdditionItem("16 Line BRAM Data at Qual Site in One Controller", dPTE);


	dPTE = PTEData.GetPTE(4, FALSE, 4, 4, 10);
	timeReport.AdditionItem("16 Line BRAM Data at Qual Site in 4 Controller Unparallel", dPTE);


	dPTE = PTEData.GetPTE(4, TRUE, 4, 4, 10);
	timeReport.AdditionItem("16 Line BRAM Data at Qual Site in 4 Controller in Parallel", dPTE);


	PTEData.SetRunParam("FAIL_DRAM_ST", "FAIL_DRAM_SP");
	PTEData.SetParam(vecChannel, "FAIL_DRAM_ST", 20, 16);
	dPTE = PTEData.GetPTE(1, FALSE, 4, 4, 10);
	timeReport.AdditionItem("16 Line DRAM Data at Qual Site in One Controller", dPTE);


	dPTE = PTEData.GetPTE(4, FALSE, 4, 4, 10);
	timeReport.AdditionItem("16 Line DRAM Data at Qual Site in 4 Controller Unparallel", dPTE);


	dPTE = PTEData.GetPTE(4, TRUE, 4, 4, 10);
	timeReport.AdditionItem("16 Line DRAM Data at Qual Site in 4 Controller in Parallel", dPTE);

	timeReport.Print(this, g_lpszReportFilePath);

	dcm.Disconnect("G_ALLPIN");
	dcm_CloseFile();
}
