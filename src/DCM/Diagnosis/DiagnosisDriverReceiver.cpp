#include "DiagnosisDriverReceiver.h"
#include "IHDReportDevice.h"
#include "..\HDModule.h"
#include "..\Pattern.h"
using namespace std;
CDiagnosisDriverReceiver::CDiagnosisDriverReceiver()
{}

CDiagnosisDriverReceiver::~CDiagnosisDriverReceiver()
{
	m_setFailController.clear();
}

int CDiagnosisDriverReceiver::QueryInstance(const char * name, void ** ptr)
{
    return -1;
}

void CDiagnosisDriverReceiver::Release()
{}

const char * CDiagnosisDriverReceiver::Name() const
{
    return "Driver / Receiver Functionality Diagnosis";
}

int CDiagnosisDriverReceiver::GetChildren(STSVector<IHDDoctorItem*>& vecChildren) const
{
	return 0;
}

int CDiagnosisDriverReceiver::Doctor(IHDReportDevice * pReportDevice)
{
	StartTimer();
	m_pReportDevice = pReportDevice;
	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };
	
	const char* lpszBaseIndent = IndentFormat();
	string strNextIndent = lpszBaseIndent + IndentChar();
	const char* lpszNextIndent = strNextIndent.c_str();

    m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<DriverReceiverFunctionalityDiagnosis>\n", lpszBaseIndent);
    int nFailCount = 0;
    do
    {
        if (0 != CheckDriverReceiver(lpszNextIndent))
        {
            ++nFailCount;
			if (1 != m_pReportDevice->IsFailContinue())
			{
				break;
			}
        }
    } while (false);
    int nRet = -1;
    if (0 == nFailCount)
    {
        nRet = 0;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszNextIndent);
    }
    else
    {
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszNextIndent);
    }

	USHORT usControllerCount = m_vecEnableController.size();

	BYTE bySlotNo = 0;
	BYTE byBoardControllerIndex = 0;
	for (auto uControllerID : m_vecEnableController)
	{
		bySlotNo = HDModule::Instance()->ID2Board(uControllerID, byBoardControllerIndex);
		if (m_setFailController.end() == m_setFailController.find(uControllerID))
		{
			m_pReportDevice->PrintfToUi(IHDReportDevice::Pass);
		}
		else
		{
			m_pReportDevice->PrintfToUi(IHDReportDevice::Fail);
		}
		m_pReportDevice->PrintfToUi("\t Slot %d, Controller %d\n", bySlotNo, byBoardControllerIndex);
	}

	dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszNextIndent, dTimeConsume, lpszTimeUnits);

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</DriverReceiverFunctionalityDiagnosis>\n", lpszBaseIndent);
    return nRet;
}

int CDiagnosisDriverReceiver::CheckDriverReceiver(const char* lpszBaseIndent)
{
	StartTimer();
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<DriverReceiver>\n", lpszBaseIndent);
	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };
	
	double dPeriod = 1000;
	double dEdge[EDGE_COUNT] = { 10, dPeriod / 2 + 10, 10, dPeriod / 2 + 10, dPeriod / 2, dPeriod * 3 / 4 };

	std::string strFirstIndent = lpszBaseIndent + IndentChar();
	string strSecondIndent = strFirstIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();
	const char* lpszSecondIndent = strSecondIndent.c_str();
	int nRetVal = -1;

	Bind(m_vecEnableController, m_vecEnableController[0]);
	CHardwareFunction* pHardware = GetHardware(m_vecEnableController[0]);

	///<Disconnect the function relay firstly, in case of the relay is connected outside
	vector<USHORT> vecChannel;
	for (USHORT usChannel = 0; usChannel < HDModule::ChannelCountPerBoard;++usChannel)
	{
		vecChannel.push_back(usChannel);
	}
	pHardware->SetFunctionRelay(vecChannel, FALSE);
	vecChannel.clear();

	
	for (USHORT usChannel = 0; usChannel < HDModule::ControlCountPerBoard; usChannel++)
	{
		vecChannel.push_back(usChannel);
	}
	pHardware->SetPeriod(0, 1000);
	pHardware->SetEdge(vecChannel, 0, dEdge, WAVE_FORMAT::NRZ, IO_FORMAT::NRZ, COMPARE_MODE::EDGE);

	CPattern Pattern(*pHardware);
	int nLineCount = 0;
	///<The vector for checking status word '0'
	Pattern.AddPattern(nLineCount++, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, FALSE); ///<0
	Pattern.AddPattern(nLineCount++, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, FALSE);///<1;
	Pattern.AddPattern(nLineCount++, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, FALSE);///<2;
	Pattern.AddPattern(nLineCount++, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, FALSE);///<3;

	///<The vector for checking status word '1'
	Pattern.AddPattern(nLineCount++, TRUE, "1111111111111111", 0, "INC", "", 0, FALSE, FALSE);///<4;
	Pattern.AddPattern(nLineCount++, TRUE, "1111111111111111", 0, "INC", "", 0, FALSE, FALSE);///<5;
	Pattern.AddPattern(nLineCount++, TRUE, "1111111111111111", 0, "INC", "", 0, FALSE, FALSE);///<6;
	Pattern.AddPattern(nLineCount++, TRUE, "1111111111111111", 0, "INC", "", 0, FALSE, FALSE);///<7;

	///<The vector for checking status word 'S' after '0'
	Pattern.AddPattern(nLineCount++, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, FALSE);///<8;
	Pattern.AddPattern(nLineCount++, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, FALSE);///<9;
	Pattern.AddPattern(nLineCount++, TRUE, "0000000000000000", 0, "INC", "", 0, FALSE, FALSE);///<10;
	Pattern.AddPattern(nLineCount++, TRUE, "SSSSSSSSSSSSSSSS", 0, "INC", "", 0, FALSE, FALSE);///<11;

	///<The vector for checking status word 'S' after '1'
	Pattern.AddPattern(nLineCount++, TRUE, "1111111111111111", 0, "INC", "", 0, FALSE, FALSE);///<12;
	Pattern.AddPattern(nLineCount++, TRUE, "1111111111111111", 0, "INC", "", 0, FALSE, FALSE);///<13;
	Pattern.AddPattern(nLineCount++, TRUE, "1111111111111111", 0, "INC", "", 0, FALSE, FALSE);///<14;
	Pattern.AddPattern(nLineCount++, TRUE, "SSSSSSSSSSSSSSSS", 0, "INC", "", 0, FALSE, FALSE);///<15;

	Pattern.Load();
	ClearBind();

	BOOL bAllPass = TRUE;
	BOOL bCurCheckPass = TRUE;
	do
	{
		if (0 != CheckStatusWord(lpszFirstIndent, '0', 0, 3))
		{
			bAllPass = FALSE;
			break;
		}
		if (0 != CheckStatusWord(lpszFirstIndent, '1', 4, 7))
		{
			bAllPass = FALSE;
			break;
		}

		///<Check the status word 'S'
		bCurCheckPass = TRUE;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<StatusWord value='S'>\n", lpszFirstIndent);
		
		StartTimer();
		if (0 != CheckStatusWord(lpszSecondIndent, '0', 8, 11))
		{
			bAllPass = FALSE;
			bCurCheckPass = FALSE;
		}
		if (0 != CheckStatusWord(lpszSecondIndent, '1', 12, 15))
		{
			bAllPass = FALSE;
			bCurCheckPass = FALSE;
		}
		if (bCurCheckPass)
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszSecondIndent);
		}
		else
		{
			bAllPass = FALSE;
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszSecondIndent);
		}
		dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszSecondIndent, dTimeConsume, lpszTimeUnits);

		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</StatusWord>\n", lpszFirstIndent);
	} while (false);

	if (bAllPass)
	{
		nRetVal = 0;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszFirstIndent);
	}
	else
	{
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszFirstIndent);
		nRetVal = -1;
	}
	dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszFirstIndent, dTimeConsume, lpszTimeUnits);

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</DriverReceiver>\n", lpszBaseIndent);

	return nRetVal;
}

int CDiagnosisDriverReceiver::CheckStatusWord(const char* lpszBaseIndent, char cStatusWard, int nStartLine, int nStopLine)
{
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<StatusWord value='%c'>\n", lpszBaseIndent, cStatusWard);
	StartTimer();

	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };
	string strFirstIndent = lpszBaseIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();

	double dPinLevel[5] = { 0 };
	switch (cStatusWard)
	{
	case '0':
		dPinLevel[0] = -1;
		dPinLevel[1] = 0.4;
		dPinLevel[2] = 1.0;
		dPinLevel[3] = 1.8;
		break;
	case '1':
		dPinLevel[0] = 0.4;
		dPinLevel[1] = 2.4;
		dPinLevel[2] = 4;
		dPinLevel[3] = 5;
		break;
	default:
		break;
	}

	BOOL bAllPass = FALSE;
	do
	{
		if (0 != CheckPinLevel(lpszFirstIndent, 3.3, 0.8, dPinLevel[3], dPinLevel[2], nStartLine, nStopLine, 0x0000))
		{
			break;
		}
		if (0 != CheckPinLevel(lpszFirstIndent, 3.3, 0.8, dPinLevel[3], dPinLevel[0], nStartLine, nStopLine, 0xffff))
		{
			break;
		}
		if (0 != CheckPinLevel(lpszFirstIndent, 3.3, 0.8, dPinLevel[1], dPinLevel[0], nStartLine, nStopLine, 0xffffffff))
		{
			break;
		}
		bAllPass = TRUE;
	} while (false);

	int nRetVal = 0;
	if (bAllPass)
	{
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszFirstIndent);
	}
	else
	{
		nRetVal = -1;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszFirstIndent);
	}
	dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszFirstIndent, dTimeConsume, lpszTimeUnits);

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</StatusWord>\n", lpszBaseIndent);
	
	return nRetVal;
}

int CDiagnosisDriverReceiver::CheckPinLevel(const char* lpszBaseIndent, double dVIH, double dVIL, double dVOH, double dVOL, int nStartLine, int nStopLine, unsigned long ulExpectStatus)
{
	if (0 >= m_vecEnableController.size())
	{
		return -2;
	}

	StartTimer();

	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };

	string strFirstIndent = lpszBaseIndent + IndentChar();
	string strSecondIndent = strFirstIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();
	const char* lpszSecondIndent = strSecondIndent.c_str();

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<PinLevel value='VIH=%.1f, VOH=%.1f, VIH=%.1f, VIL=%.1f'>\n", lpszBaseIndent, dVIH, dVIL, dVOH, dVOL);

	vector<USHORT> vecChannel;
	for (USHORT usChannel = 0; usChannel < HDModule::ChannelCountPerControl; usChannel++)
	{
		vecChannel.push_back(usChannel);
	}

	double dVT = (dVIH + dVIL) / 2;
	double dClampHigh = 7.5;
	double dClampLow = -2.5;

	Bind(m_vecEnableController, m_vecEnableController[0]);
	CHardwareFunction* pHardware = GetHardware(m_vecEnableController[0]);
	pHardware->SetPinLevel(vecChannel, dVIH, dVIL, dVT, dVOH,dVOL);
	pHardware->SetRunParameter(nStartLine, nStopLine);
	ClearBind();

	pHardware->SynRun();

	UINT uPatternCount = nStopLine - nStartLine + 1;
	BOOL bAllPass = TRUE;
	ULONG ulChannelStatus = 0;
	BYTE bySlotNo = 0;
	BYTE byBoardControllerIndex = 0;
	for (auto uControllerID : m_vecEnableController)
	{
		StartTimer();
		bySlotNo = HDModule::Instance()->ID2Board(uControllerID, byBoardControllerIndex);
		
		pHardware = GetHardware(uControllerID);
		
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<controller value='%d, slot value = %d'>\n", lpszFirstIndent, byBoardControllerIndex, bySlotNo);
		
		ulChannelStatus = pHardware->GetChannelStatus();

		if (ulExpectStatus == ulChannelStatus)
		{
			UINT uPatternLineCount = pHardware->GetRanPatternCount();
			if (uPatternLineCount != uPatternCount)
			{
				bAllPass = FALSE;
				SetFailController(uControllerID);

				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<data value='lineCount=0x%X real=0x%X'/>\n", lpszSecondIndent, uPatternCount, uPatternLineCount);
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszSecondIndent);
			}
			else
			{
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszSecondIndent);
			}
		}
		else
		{
			bAllPass = FALSE;
			SetFailController(uControllerID);

			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<data value='cdata=0x%X data=0x%X'/>\n", lpszSecondIndent, ulExpectStatus, ulChannelStatus);
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszSecondIndent);

		}
		dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszSecondIndent, dTimeConsume, lpszTimeUnits);
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</controller>\n", lpszFirstIndent);
	}
	int nRetVal = 0;
	if (bAllPass)
	{
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszFirstIndent);
		nRetVal = 0;
	}
	else
	{
		nRetVal = -1;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszFirstIndent);
	}

	dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszFirstIndent, dTimeConsume, lpszTimeUnits);

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</PinLevel>\n", lpszBaseIndent);

	return nRetVal;
}

inline void CDiagnosisDriverReceiver::SetFailController(UINT uControllerID)
{
	if (m_setFailController.end() == m_setFailController.find(uControllerID))
	{
		m_setFailController.insert(uControllerID);
	}
}
