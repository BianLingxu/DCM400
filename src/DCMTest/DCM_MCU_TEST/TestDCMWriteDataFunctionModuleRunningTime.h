#pragma once
/**
 * @file TestDCMWriteDataFunctionModuleRunningTime.h
 * @brief Test the function moduel running time of writting data
 * @author Guangyun Wang
 * @date 20021/09/02
 * @copyright AccoTEST Business Unit of Beijing Huafeng Test & Controller Co., Ltd.
*/
#include "..\DCMTestMain.h"
#include "..\PTE.h"
#define ALL_SITE 0xFFFF
class CWriteDataModulePTE : public CPTE
{
public:
	/**
	 * @brief Constructor
	 * @param[in] dcm The instance of DCM
	 * @param[in] lpszVectorFile The vector file used to restore the vector in board
	*/
	CWriteDataModulePTE(DCM& dcm, const char* lpszVectorFile);
	/**
	 * @brief Destructor
	*/
	~CWriteDataModulePTE();
	/**
	 * @brief The function whose PTE will be calculated
	*/
	virtual void FuncExecute();
	/**
	 * @brief Reset the status afer function execution
	*/
	virtual void ResetStatus();
	/**
	 * @brief Set the maximum site count
	 * @param[in] usSiteCount The site count
	 * @note Must set before set the line information, or the data set will missing
	*/
	void SetMaxSiteCount(USHORT usSiteCount);
	/**
	 * @brief Set the line information of the line whose data will be written
	 * @param lpszStartLabel The start label
	 * @param nOffset The offset line to the label
	 * @param nLineCount The line count
	 * @return Execute result
	 * - 0 Set the line information successfully
	 * - -1 The label is nullptr
	 * - -2 The label is not existed
	 * - -3 The offset is over range
	 * - -4 The line count is over range
	 * - -5 Not set maximum site count
	 * - -6 Allocate memory fail
	*/
	int SetLineInfo(const char* lpszStartLabel, int nOffset, int nLineCount);
	/**
	 * @brief Set the site data
	 * @param usSiteNo The site number
	 * @param pbyData The data will be written
	 * @return Execute result
	 * - 0 Set site data successfully
	 * - -1 Not set the line information
	 * - -2 The site number is over range
	 * - -3 The data point is nullptr
	*/
	int SetSiteData(USHORT usSiteNo, BYTE* pbyData);
	/**
	 * @brief Preread vector for written
	 * @param[in] bEnable Enable preread vector
	 * @param[in] lpszStartLabel The start label
	 * @param[in] lpszStopLabel The stop label
	 * @return Execute result
	 * - 0 Preread vector successfully
	 * - -1 The start label is nullptr
	 * - -2 The start label is not existed
	 * - -3 The stop label is nullptr
	 * - -4 The stop label is not existed
	 * - -5 The stop label is not behind the start label
	*/
	int PrereadVector(BOOL bEnable, const char* lpszStartLabel, const char* lpszStopLabel);
private:
	/**
	 * @brief Clear memory
	*/
	void ClearMemory();
private:
	DCM* m_pDCM;///<The point to DCM instance
	USHORT m_usMaxSiteCount;///<The maximum site count
	BOOL m_bDataAllSame;///<Whether the data of each site is same
	BOOL m_bEnablePreread;///<Wheter enable preread vector
	int m_nOffset;///<The offset to start label
	int m_nLineCount;///<The line count whose data will be written
	BYTE** m_ppDataWritten;///<The data will be written
	std::string m_strVectorFile;///<The vector file
	std::string m_strWriteLabel;///<The start label
	std::string m_strStartLabel;///<The start label of preread vector
	std::string m_strStopLabel;///<The stop label of preread vector
};

CWriteDataModulePTE::CWriteDataModulePTE(DCM& dcm, const char* lpszVectorFile)
	: m_pDCM(&dcm)
	, m_usMaxSiteCount(0)
	, m_nLineCount(0)
	, m_nOffset(0)
	, m_bDataAllSame(TRUE)
	, m_ppDataWritten(nullptr)
	, m_bEnablePreread(FALSE)
	, m_strVectorFile(lpszVectorFile)
{

}

CWriteDataModulePTE::~CWriteDataModulePTE()
{
	ClearMemory();
	dcm_CloseFile();
	//dcm.LoadVectorFile(m_strVectorFile.c_str());
}

void CWriteDataModulePTE::FuncExecute()
{
	m_pDCM->SetWaveDataParam(m_strAllPin.c_str(), m_strWriteLabel.c_str(), m_nOffset, m_nLineCount);
	if (0 == m_usSiteCount)
	{
		m_pDCM->SetSiteWaveData(0, m_ppDataWritten[0]);
	}
	else if (m_bDataAllSame)
	{
		m_pDCM->SetSiteWaveData(DCM_ALLSITE, m_ppDataWritten[0]);
	}
	else
	{
		for (USHORT usSiteNo = 0; usSiteNo < m_usSiteCount; ++usSiteNo)
		{
			m_pDCM->SetSiteWaveData(usSiteNo, m_ppDataWritten[usSiteNo]);
		}
	}
	m_pDCM->WriteWaveData();
}

void CWriteDataModulePTE::ResetStatus()
{
	int nByteCount = (m_nLineCount + 7) / 8;
	BYTE* pbyData = nullptr;
	try
	{
		pbyData = new BYTE[nByteCount];
		memset(pbyData, 0x00, nByteCount * sizeof(BYTE));
	}
	catch (const std::exception&)
	{
		return;
	}
	m_pDCM->SetWaveDataParam(m_strAllPin.c_str(), m_strWriteLabel.c_str(), m_nOffset, m_nLineCount);
	m_pDCM->SetSiteWaveData(DCM_ALLSITE, pbyData);
	m_pDCM->WriteWaveData();
	if (nullptr != pbyData)
	{
		delete[] pbyData;
		pbyData = nullptr;
	}
	dcm_ClearePreadVector();
	if (m_bEnablePreread)
	{
		dcm_SetPrereadVector(m_strStartLabel.c_str(), m_strStopLabel.c_str());
	}
}

void CWriteDataModulePTE::SetMaxSiteCount(USHORT usSiteCount)
{
	if (m_usMaxSiteCount == usSiteCount)
	{
		return;
	}
	ClearMemory();
	m_usMaxSiteCount = usSiteCount;
}

int CWriteDataModulePTE::SetLineInfo(const char* lpszStartLabel, int nOffset, int nLineCount)
{
	if (nullptr == lpszStartLabel)
	{
		return -1;
	}
	int nLineNo = dcm_GetLabelLineNo(lpszStartLabel, FALSE);
	if (0 > nLineNo)
	{
		return -2;
	}
	ULONG ulVectorLineCount = 0;
	dcm_GetVectorLineCount("", "", ulVectorLineCount);
	if (ulVectorLineCount <= nLineNo + nOffset)
	{
		return -3;
	}
	if (0 >= nLineCount || ulVectorLineCount < nLineNo + nOffset + nLineCount)
	{
		return -4;
	}
	if (0 == m_usMaxSiteCount)
	{
		return -5;
	}
	m_strWriteLabel = lpszStartLabel;
	m_nOffset = nOffset;
	m_nLineCount = nLineCount;
	if (nullptr == m_ppDataWritten)
	{
		int nByteCount = (m_nLineCount + 7) / 8;
		try
		{
			m_ppDataWritten = new BYTE * [m_usMaxSiteCount];
			for (USHORT usSiteNo = 0; usSiteNo < m_usMaxSiteCount; ++usSiteNo)
			{
				m_ppDataWritten[usSiteNo] = new BYTE[nByteCount];
				memset(m_ppDataWritten[usSiteNo], 0, nByteCount * sizeof(BYTE));
			}
		}
		catch (const std::exception&)
		{
			return -6;
		}
	}
	return 0;
}

int CWriteDataModulePTE::SetSiteData(USHORT usSiteNo, BYTE* pbyData)
{
	if (nullptr == m_ppDataWritten)
	{
		///<Not set the line information
		return -1;
	}
	if (ALL_SITE != usSiteNo && m_usMaxSiteCount <= usSiteNo)
	{
		///<The site number is over range
		return -2;
	}
	if (nullptr == pbyData)
	{
		return -3;
	}
	int nByteCount = (m_nLineCount + 7) / 8;
	if (ALL_SITE != usSiteNo)
	{
		memcpy_s(m_ppDataWritten[usSiteNo], nByteCount * sizeof(BYTE), pbyData, nByteCount * sizeof(BYTE));
		m_bDataAllSame = FALSE;
	}
	else
	{
		m_bDataAllSame = TRUE;
		memcpy_s(m_ppDataWritten[0], nByteCount * sizeof(BYTE), pbyData, nByteCount * sizeof(BYTE));
	}
	return 0;
}

inline int CWriteDataModulePTE::PrereadVector(BOOL bEnable, const char* lpszStartLabel, const char* lpszStopLabel)
{
	if (!bEnable)
	{
		dcm_ClearePreadVector();
		m_bEnablePreread = FALSE;
		return 0;
	}
	if (nullptr == lpszStartLabel)
	{
		return -1;
	}
	int nStartLineNo = dcm_GetLabelLineNo(lpszStartLabel);
	if (0 > nStartLineNo)
	{
		return -2;
	}
	if (nullptr == lpszStopLabel)
	{
		return -3;
	}
	int nStopLineNo = dcm_GetLabelLineNo(lpszStopLabel);
	if (0 > nStopLineNo)
	{
		return -4;
	}
	if (nStartLineNo > nStopLineNo)
	{
		return -5;
	}
	m_bEnablePreread = bEnable;
	m_strStartLabel = lpszStartLabel;
	m_strStopLabel = lpszStopLabel;
	
	return 0;
}

void CWriteDataModulePTE::ClearMemory()
{
	if (nullptr != m_ppDataWritten)
	{
		for (USHORT usSiteNo = 0; usSiteNo < m_usMaxSiteCount; ++usSiteNo)
		{
			if (nullptr != m_ppDataWritten[usSiteNo])
			{
				delete[] m_ppDataWritten[usSiteNo];
				m_ppDataWritten[usSiteNo] = nullptr;
			}
		}
		delete[] m_ppDataWritten;
		m_ppDataWritten = nullptr;
	}
	m_usMaxSiteCount = 0;
}

XT_TEST(FunctionRunningTimeTest, TestDCMWriteDataFunctionModuleRunningTime)
{
	string strFuncName;
	GetFunctionName(this->GetName(), strFuncName, RUNNING_TIME);
	CTimeReport TimeReport(strFuncName.c_str(), "RunningTimeTest");

	int nRetVal = 0;
	map<BYTE, USHORT> mapSlot;
	nRetVal = GetBoardInfo(mapSlot, g_lpszVectorFilePath);

	if (0 == mapSlot.size())
	{
		//No board is inserted.
		XT_EXPECT_TRUE(FALSE);
		TimeReport.SetNoBoardValid();
		TimeReport.Print(this, g_lpszReportFilePath);
		return;
	}

	XT_EXPECT_EQ(nRetVal, 0);
	if (0 != nRetVal)
	{
		TimeReport.addMsg("Load vector file(%s) fail, the vector file maybe not right.", g_lpszVectorFilePath);
		TimeReport.Print(this, g_lpszReportFilePath);
		return;
	}

	SaveBoardSN(TimeReport, mapSlot);

	dcm.LoadVectorFile(g_lpszVectorFilePath, FALSE);
	const USHORT usMaxSiteCount = 4;
	const int nMaxDataByteCount = 4;

	TimeReport.SetAdditionItemTittle("PTE");
	CWriteDataModulePTE PTECalculate(dcm, g_lpszVectorFilePath);
	PTECalculate.SetMaxSiteCount(usMaxSiteCount);
	PTECalculate.SetLineInfo("TEST_RAM_ST", 0, 16);

	BYTE abyWaveData[usMaxSiteCount][nMaxDataByteCount] = {0};
	for (USHORT usSiteNo = 0; usSiteNo < usMaxSiteCount; ++usSiteNo)
	{
		for (int nDataIndex = 0; nDataIndex < nMaxDataByteCount; ++nDataIndex)
		{
			abyWaveData[usSiteNo][nDataIndex] = 0xAA + (usSiteNo << 4) + nDataIndex;
		}
	}

	double dPTE = 0;
	///<One Controller
	PTECalculate.SetSiteData(ALL_SITE, abyWaveData[0]);
	dPTE = PTECalculate.GetPTE(1, FALSE, 4, 4, 10);
	TimeReport.AdditionItem("Qual Sites in 1 Controller with 4 Pins, Write 16 Lines Same Data to BRAM", dPTE);

	for (USHORT usSiteNo = 0; usSiteNo < usMaxSiteCount; ++usSiteNo)
	{
		PTECalculate.SetSiteData(usSiteNo, abyWaveData[usSiteNo]);
	}
	dPTE = PTECalculate.GetPTE(1, FALSE, 4, 4, 10);
	TimeReport.AdditionItem("Qual Sites in 1 Controller with 4 Pins, Write 16 Lines Different Data to BRAM", dPTE);

	PTECalculate.SetLineInfo("TEST_WRITE_ST", 10, 16);
	PTECalculate.SetSiteData(ALL_SITE, abyWaveData[0]);
	dPTE = PTECalculate.GetPTE(1, FALSE, 4, 4, 10);
	TimeReport.AdditionItem("Qual Sites in 1 Controller with 4 Pins, Write 16 Lines Same Data to DRAM", dPTE);

	for (USHORT usSiteNo = 0; usSiteNo < usMaxSiteCount; ++usSiteNo)
	{
		PTECalculate.SetSiteData(usSiteNo, abyWaveData[usSiteNo]);
	}
	dPTE = PTECalculate.GetPTE(1, FALSE, 4, 4, 10);
	TimeReport.AdditionItem("Qual Sites in 1 Controller with 4 Pins, Write 16 Lines Different Data to DRAM", dPTE);

	///<Unparallel
	PTECalculate.SetLineInfo("TEST_RAM_ST", 0, 16);
	PTECalculate.SetSiteData(ALL_SITE, abyWaveData[0]);
	dPTE = PTECalculate.GetPTE(4, FALSE, 4, 4, 10);
	TimeReport.AdditionItem("Qual Sites in 4 Controllers with 4 Pins Unparallel, Write 16 Lines Same Data to BRAM", dPTE);

	for (USHORT usSiteNo = 0; usSiteNo < usMaxSiteCount; ++usSiteNo)
	{
		PTECalculate.SetSiteData(usSiteNo, abyWaveData[usSiteNo]);
	}
	dPTE = PTECalculate.GetPTE(4, FALSE, 4, 4, 10);
	TimeReport.AdditionItem("Qual Sites in 4 Controllers with 4 Pins Unparallel, Write 16 Lines Different Data to BRAM", dPTE);

	PTECalculate.SetLineInfo("TEST_WRITE_ST", 10, 16);
	PTECalculate.SetSiteData(ALL_SITE, abyWaveData[0]);
	dPTE = PTECalculate.GetPTE(4, FALSE, 4, 4, 10);
	TimeReport.AdditionItem("Qual Sites in 4 Controllers with 4 Pins Unparallel, Write 16 Lines Same Data to DRAM", dPTE);

	for (USHORT usSiteNo = 0; usSiteNo < usMaxSiteCount; ++usSiteNo)
	{
		PTECalculate.SetSiteData(usSiteNo, abyWaveData[usSiteNo]);
	}
	dPTE = PTECalculate.GetPTE(4, FALSE, 4, 4, 10);
	TimeReport.AdditionItem("Qual Sites in 4 Controllers with 4 Pins Unparallel, Write 16 Lines Different Data to DRAM", dPTE);

	///<Parallel
	PTECalculate.SetLineInfo("TEST_RAM_ST", 0, 16);
	PTECalculate.SetSiteData(ALL_SITE, abyWaveData[0]);
	dPTE = PTECalculate.GetPTE(4, TRUE, 4, 4, 10);
	TimeReport.AdditionItem("Qual Sites in 4 Controllers with 4 Pins in Parallel, Write 16 Lines Same Data to BRAM", dPTE);

	for (USHORT usSiteNo = 0; usSiteNo < usMaxSiteCount; ++usSiteNo)
	{
		PTECalculate.SetSiteData(usSiteNo, abyWaveData[usSiteNo]);
	}
	dPTE = PTECalculate.GetPTE(4, TRUE, 4, 4, 10);
	TimeReport.AdditionItem("Qual Sites in 4 Controllers with 4 Pins in Parallel, Write 16 Lines Different Data to BRAM", dPTE);

	PTECalculate.SetLineInfo("TEST_WRITE_ST", 10, 16);
	PTECalculate.SetSiteData(ALL_SITE, abyWaveData[0]);
	dPTE = PTECalculate.GetPTE(4, TRUE, 4, 4, 10);
	TimeReport.AdditionItem("Qual Sites in 4 Controllers with 4 Pins in Parallel, Write 16 Lines Same Data to DRAM", dPTE);

	for (USHORT usSiteNo = 0; usSiteNo < usMaxSiteCount; ++usSiteNo)
	{
		PTECalculate.SetSiteData(usSiteNo, abyWaveData[usSiteNo]);
	}
	dPTE = PTECalculate.GetPTE(4, TRUE, 4, 4, 10);
	TimeReport.AdditionItem("Qual Sites in 4 Controllers with 4 Pins in Parallel, Write 16 Lines Different Data to DRAM", dPTE);

	///<Parallel and Preread Vector
	PTECalculate.SetLineInfo("TEST_RAM_ST", 0, 16);
	PTECalculate.PrereadVector(TRUE, "TEST_RAM_ST", "TEST_RAM_SP");
	PTECalculate.SetSiteData(ALL_SITE, abyWaveData[0]);
	dPTE = PTECalculate.GetPTE(4, TRUE, 4, 4, 10);
	TimeReport.AdditionItem("Qual Sites in 4 Controllers with 4 Pins in Parallel and Preread, Write 16 Lines Same Data to BRAM", dPTE);

	for (USHORT usSiteNo = 0; usSiteNo < usMaxSiteCount; ++usSiteNo)
	{
		PTECalculate.SetSiteData(usSiteNo, abyWaveData[usSiteNo]);
	}
	dPTE = PTECalculate.GetPTE(4, TRUE, 4, 4, 10);
	TimeReport.AdditionItem("Qual Sites in 4 Controllers with 4 Pins in Parallel and Preread, Write 16 Lines Different Data to BRAM", dPTE);

	PTECalculate.SetLineInfo("TEST_WRITE_ST", 10, 16);
	PTECalculate.PrereadVector(TRUE, "TEST_WRITE_ST", "TEST_WRITE_SP");
	PTECalculate.SetSiteData(ALL_SITE, abyWaveData[0]);
	dPTE = PTECalculate.GetPTE(4, TRUE, 4, 4, 10);
	TimeReport.AdditionItem("Qual Sites in 4 Controllers with 4 Pins in Parallel and Preread, Write 16 Lines Same Data to DRAM", dPTE);

	for (USHORT usSiteNo = 0; usSiteNo < usMaxSiteCount; ++usSiteNo)
	{
		PTECalculate.SetSiteData(usSiteNo, abyWaveData[usSiteNo]);
	}
	dPTE = PTECalculate.GetPTE(4, TRUE, 4, 4, 10);
	TimeReport.AdditionItem("Qual Sites in 4 Controllers with 4 Pins in Parallel and Preread, Write 16 Lines Different Data to DRAM", dPTE);

	TimeReport.Print(this, g_lpszReportFilePath);

	dcm_CloseFile();
}
