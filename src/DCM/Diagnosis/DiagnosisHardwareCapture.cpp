#include "DiagnosisHardwareCapture.h"
#include "..\HDModule.h"
#include "..\Pattern.h"
extern bool operator==(const CHardwareFunction::DATA_RESULT& LeftResult, const CHardwareFunction::DATA_RESULT& RightResult);
using namespace std;
CDiagnosisHardwareCapture::CDiagnosisHardwareCapture()
{
}

CDiagnosisHardwareCapture::~CDiagnosisHardwareCapture()
{
}

int CDiagnosisHardwareCapture::QueryInstance(const char* lpszName, void** ppPoint)
{
	return -1;
}

void CDiagnosisHardwareCapture::Release()
{

}

const char* CDiagnosisHardwareCapture::Name() const
{
	return "Hardware Capture Diagnosis";
}

int CDiagnosisHardwareCapture::GetChildren(STSVector<IHDDoctorItem*>& vecChildren) const
{
	return 0;
}

int CDiagnosisHardwareCapture::Doctor(IHDReportDevice* pReportDevice)
{
	m_pReportDevice = pReportDevice;
	int nRetVal = -1;

	StartTimer();
	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };
	const char* lpszBaseIndent = IndentFormat();
	string strFirstIndent = lpszBaseIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<HardwareCaptureDiagnosis>\n", lpszBaseIndent);
	std::string strNextIndent = IndentFormat() + IndentChar();
	const char* lpszNextIndent = strNextIndent.c_str();
	CheckMutualDiagnosable();
	BYTE bySlotNo = 0;
	BYTE byBoardControllerIndex = 0;
	for (auto& Undiagnosable : m_mapUndiagnosableController)
	{
		bySlotNo = HDModule::Instance()->ID2Board(Undiagnosable.first, byBoardControllerIndex);
		m_pReportDevice->PrintfToUi(IHDReportDevice::Fail);
		m_pReportDevice->PrintfToUi("\t Slot %d, Controller %d\n", bySlotNo, byBoardControllerIndex);
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Controller value='%d, slot value=%d'>\n", IndentFormat(), bySlotNo, byBoardControllerIndex);

		bySlotNo = HDModule::Instance()->ID2Board(Undiagnosable.second, byBoardControllerIndex);
		m_pReportDevice->PrintfToUi("\t\t Undiagnosable for slot %d controller %d is not existed.", bySlotNo, byBoardControllerIndex);

		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Info>Undiagnosable for slot %d controller %d is not existed.</Info>\n", bySlotNo, byBoardControllerIndex,
			lpszNextIndent, bySlotNo, byBoardControllerIndex);
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</Controller>\n", IndentFormat());
	}
	int nFailItemCount = 0;
	int nItemResult = 0;

	Connect();

	if (0 == m_vecEnableController.size())
	{
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Item>No controller valid to be diagnosed</Item>\n", lpszNextIndent);
	}
	else
	{
		do
		{
			m_pReportDevice->PrintfToUi(" BRAM\n");
			nItemResult = BRAMDiagnosis(lpszFirstIndent);
			if (0 != nItemResult)
			{
				++nFailItemCount;
			}
			if (1 == m_pReportDevice->IsStop())
			{
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='Next=DRAMDiagnosis'/>\n", lpszFirstIndent);
				break;
			}
			m_pReportDevice->PrintfToUi(" DRAM\n");
			nItemResult = DRAMDiagnosis(lpszFirstIndent);
			if (0 != nItemResult)
			{
				++nFailItemCount;
			}
		} while (false);
	}
	Disconnect();

	if (0 == nFailItemCount)
	{
		nRetVal = 0;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszNextIndent);
	}
	else
	{
		nRetVal = -1;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszNextIndent);
	}

	dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszNextIndent, dTimeConsume, lpszTimeUnits);

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</HardwareCaptureDiagnosis>\n", lpszBaseIndent);

	return nRetVal;
}

bool CDiagnosisHardwareCapture::IsUserCheck() const
{
	switch (m_UserRole)
	{
	case CDiagnosisItem::PROCUCTION:
	case CDiagnosisItem::DEVELOPER:
		return true;
		break;
	case CDiagnosisItem::USER:
	default:
		return false;
		break;
	}
}

int CDiagnosisHardwareCapture::BRAMRateDiagnosis(const char* lpszBaseIndent)
{
	StartTimer();
	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };

	string strFirstIndent = lpszBaseIndent + IndentChar();
	string strSecondIndent = lpszBaseIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();
	const char* lpszSecondIndent = strSecondIndent.c_str();

	int nRetVal = 0;
	int nFailItemCount = 0;

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<MutiRateDiagnosis>\n", lpszBaseIndent);

	vector<double> vecPeriod;
	vecPeriod.push_back(32);
	vecPeriod.push_back(100);
	vecPeriod.push_back(1e3);
	vecPeriod.push_back(2e3);
	vecPeriod.push_back(5e3);
	set<UINT> setStartLine;
	set<UINT> setStopLine;
	set<UINT> setBRAMOut;
	set<UINT> setDRAMIn;
	int nStopLineNo = 0;
	setStartLine.insert(0);
	setStopLine.insert(100);
	nStopLineNo = LoadPattern(setStartLine, setStopLine, setBRAMOut, setDRAMIn);
	vector<USHORT> vecChannel;
	for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL; ++usChannel)
	{
		vecChannel.push_back(usChannel);
	}

	Bind(m_vecEnableController, m_vecEnableController[0]);
	CHardwareFunction* pHardware = GetHardware(m_vecEnableController[0]);
	pHardware->SetPinLevel(vecChannel, 3, 0, 1.5, 1.5, 0.8);
	ClearBind();

	for (auto Period : vecPeriod)
	{
		if (1 == m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='Next=Period=%.0f'/>\n", lpszFirstIndent, Period);
			break;
		}

		StartTimer();
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Period value=%.0f>\n", lpszFirstIndent, Period);
		///<Run vector
		Bind(m_vecEnableController, m_vecEnableController[0]);
		pHardware = GetHardware(m_vecEnableController[0]);
		pHardware->SetPeriod(0, Period);
		double adEdge[EDGE_COUNT] = { 0, Period / 2, 0, Period / 2, Period / 2, Period * 4 / 5 };
		pHardware->SetEdge(vecChannel, 0, adEdge, WAVE_FORMAT::NRZ, IO_FORMAT::NRZ, COMPARE_MODE::EDGE);
		pHardware->SetRunParameter(0, nStopLineNo, FALSE, 0);
		pHardware->SynRun();
		ClearBind();

		WaitStop();

		nRetVal = ResultAnalyze(lpszSecondIndent);
		if (0 != nRetVal)
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszSecondIndent);
			++nFailItemCount;
		}
		else
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszSecondIndent);
		}
		dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszSecondIndent, dTimeConsume, lpszTimeUnits);

		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</Period>\n", lpszFirstIndent);
	}

	dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
	if (nullptr != m_pReportDevice)
	{
		if (0 != nFailItemCount)
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszFirstIndent);
		}
		else
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszFirstIndent);
		}
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszFirstIndent, dTimeConsume, lpszTimeUnits);
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</MutiRateDiagnosis>\n", lpszBaseIndent);
	}

	if (0 != nFailItemCount)
	{
		return -1;
	}
	return 0;
}

int CDiagnosisHardwareCapture::BRAMDiagnosis(const char* lpszBaseIndent)
{
	StartTimer();
	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };

	string strFirstIndent = lpszBaseIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();

	BOOL nItemResult = FALSE;
	int nFailItemCount = 0;

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<BRAM>\n", lpszBaseIndent);

	do
	{
		///<Section type diagnosis
		nItemResult = BRAMSectionDiagnosis(lpszFirstIndent);
		if (0 != nItemResult)
		{
			++nFailItemCount;
		}

		if (1 == m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='Next=RateDiagnosis'/>\n", lpszFirstIndent);
			break;
		}
		nItemResult = BRAMRateDiagnosis(lpszFirstIndent);
		if (0 != nItemResult)
		{
			++nFailItemCount;
		}
		if (1 == m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='Next=SaveSelectedFailDiagnosis'/>\n", lpszFirstIndent);
			break;
		}

		nItemResult = BRAMSaveSelectedFailDiagnosis(lpszFirstIndent);
		if (0 != nItemResult)
		{
			++nFailItemCount;
		}
	} while (FALSE);


	dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
	if (nullptr != m_pReportDevice)
	{
		if (0 != nFailItemCount)
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszFirstIndent);
		}
		else
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszFirstIndent);
		}
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszFirstIndent, dTimeConsume, lpszTimeUnits);
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</BRAM>\n", lpszBaseIndent);
	}
	ShowUIResult();
	if (0 != nFailItemCount)
	{
		return -1;
	}
	return 0;
}

int CDiagnosisHardwareCapture::BRAMSectionDiagnosis(const char* lpszBaseIndent)
{
	StartTimer();
	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };

	string strFirstIndent = lpszBaseIndent + IndentChar();
	string strSecondIndent = lpszBaseIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();
	const char* lpszSecondIndent = strSecondIndent.c_str();

	int nRetVal = 0;
	int nFailItemCount = 0;
	double dPeriod = 100;
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<SectionDiagnosis value='Period=%.0f'>\n", lpszBaseIndent, dPeriod);
	set<UINT> setStartLineNo;
	set<UINT> setStopLineNo;
	set<UINT> setBRAMOut;
	set<UINT> setDRAMIn;
	int nStopLineNo = 0;
	CHardwareFunction* pHardware = nullptr;
	vector<USHORT> vecChannel;
	for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL; ++usChannel)
	{
		vecChannel.push_back(usChannel);
	}

	Bind(m_vecEnableController, m_vecEnableController[0]);
	pHardware = GetHardware(m_vecEnableController[0]);
	pHardware->SetPeriod(0, dPeriod);
	pHardware->SetPinLevel(vecChannel, 3, 0, 1.5, 1.5, 0.8);
	double adEdge[EDGE_COUNT] = { 0, dPeriod / 2, 0, dPeriod / 2, dPeriod / 2, dPeriod * 4 / 5 };
	pHardware->SetEdge(vecChannel, 0, adEdge, WAVE_FORMAT::NRZ, IO_FORMAT::NRZ, COMPARE_MODE::EDGE);
	ClearBind();


	auto Diagnosis = [&](const char* lpszItemName)
	{
		StartTimer();
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<%s>\n", lpszFirstIndent, lpszItemName);
		nStopLineNo = LoadPattern(setStartLineNo, setStopLineNo, setBRAMOut, setDRAMIn);
		///<Run vector
		Bind(m_vecEnableController, m_vecEnableController[0]);
		pHardware = GetHardware(m_vecEnableController[0]);
		pHardware->SetRunParameter(0, nStopLineNo, FALSE, 0);
		pHardware->SynRun();
		ClearBind();

		WaitStop();

		int nRetVal = ResultAnalyze(lpszSecondIndent);
		if (0 != nRetVal)
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszSecondIndent);
			++nFailItemCount;
		}
		else
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszSecondIndent);
		}
		dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszSecondIndent, dTimeConsume, lpszTimeUnits);
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</%s>\n", lpszFirstIndent, lpszItemName);
	};

	do
	{
		///<No capture line number saved
		setStartLineNo.clear();
		setStopLineNo.clear();
		Diagnosis("NoCapture");
		setStartLineNo.clear();
		setStopLineNo.clear();

		if (1 == m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='Next=OneSection'/>\n", lpszFirstIndent);
			break;
		}

		///<One Section
		setStartLineNo.insert(0);
		setStopLineNo.insert(100);
		Diagnosis("OneSection");

		if (1 == m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='Next=OneLineSaved'/>\n", lpszFirstIndent);
			break;
		}

		///<One line each sector
		setStartLineNo.insert(0);
		setStopLineNo.insert(0);
		setStartLineNo.insert(10);
		setStopLineNo.insert(10);
		Diagnosis("OneLineSaved");
		setStartLineNo.clear();
		setStopLineNo.clear();
		if (1 == m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='Next=MultiSection'/>\n", lpszFirstIndent);
			break;
		}

		///<MultiSection
		for (int nSectionIndex = 0; nSectionIndex < BRAM_MAX_SAVE_FAIL_LINE_COUNT; ++nSectionIndex)
		{
			int nStartLine = nSectionIndex * 4;
			setStartLineNo.insert(nStartLine);
			setStopLineNo.insert(nStartLine);
		}
		Diagnosis("MultiSection");
		setStartLineNo.clear();
		setStopLineNo.clear();

		if (1 == m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='Next=FullMemorySaved'/>\n", lpszFirstIndent);
			break;
		}

		///<Full Memory Saved
		setStartLineNo.insert(0);
		setStopLineNo.insert(2000);
		Diagnosis("FullMemorySaved");
		setStartLineNo.clear();
		setStopLineNo.clear();

	} while (FALSE);

	dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
	if (nullptr != m_pReportDevice)
	{
		if (0 != nFailItemCount)
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszFirstIndent);
		}
		else
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszFirstIndent);
		}
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszFirstIndent, dTimeConsume, lpszTimeUnits);
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</BRAM>\n", lpszBaseIndent);
	}

	if (0 != nFailItemCount)
	{
		return -1;
	}
	return 0;
}

int CDiagnosisHardwareCapture::BRAMSaveSelectedFailDiagnosis(const char* lpszBaseIndent)
{
	StartTimer();
	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };
	double dPeriod = 100;
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<SaveSelectedFailDiagnosis value='Period=%.0f'>\n", lpszBaseIndent, dPeriod);

	string strFirstIndent = lpszBaseIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();

	set<UINT> setBRAMOut;
	set<UINT> setDRAMIn;

	int nRetVal = SaveSelectedFailDiagnosis(setBRAMOut, setDRAMIn, dPeriod, lpszFirstIndent);
	if (0 != nRetVal)
	{
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszFirstIndent);
	}
	else
	{
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszFirstIndent);
	}
	dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszFirstIndent, dTimeConsume, lpszTimeUnits);
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</SaveSelectedFailDiagnosis>\n", lpszBaseIndent);
	if (0 != nRetVal)
	{
		return -1;
	}
	return 0;
}

int CDiagnosisHardwareCapture::DRAMDiagnosis(const char* lpszBaseIndent)
{
	StartTimer();
	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };

	string strFirstIndent = lpszBaseIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();

	BOOL nItemResult = FALSE;
	int nFailItemCount = 0;

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<DRAM>\n", lpszBaseIndent);

	do
	{
		///<No fail line number saved
		nItemResult = DRAMSectionDiagnosis(lpszFirstIndent);
		if (0 != nItemResult)
		{
			++nFailItemCount;
		}
		if (1 == m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='Next=SaveSelectedFailDiagnosis'/>\n", lpszFirstIndent);
			break;
		}
		nItemResult = DRAMSaveSelectedFailDiagnosis(lpszFirstIndent);
		if (0 != nItemResult)
		{
			++nFailItemCount;
		}

	} while (FALSE);


	dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
	if (nullptr != m_pReportDevice)
	{
		if (0 != nFailItemCount)
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszFirstIndent);
		}
		else
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszFirstIndent);
		}
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszFirstIndent, dTimeConsume, lpszTimeUnits);
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</DRAM>\n", lpszBaseIndent);
	}
	ShowUIResult();
	if (0 != nFailItemCount)
	{
		return -1;
	}
	return 0;
}

int CDiagnosisHardwareCapture::DRAMSectionDiagnosis(const char* lpszBaseIndent)
{
	StartTimer();
	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };

	string strFirstIndent = lpszBaseIndent + IndentChar();
	string strSecondIndent = lpszBaseIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();
	const char* lpszSecondIndent = strSecondIndent.c_str();

	int nRetVal = 0;
	int nFailItemCount = 0;
	double dPeriod = 100;
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<SectionDiagnosis, value='Period=%.0f'>\n", lpszBaseIndent, dPeriod);
	set<UINT> setStartLineNo;
	set<UINT> setStopLineNo;
	set<UINT> setBRAMOut;
	set<UINT> setDRAMIn;
	int nStopLineNo = 0;
	CHardwareFunction* pHardware = nullptr;
	vector<USHORT> vecChannel;
	for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL; ++usChannel)
	{
		vecChannel.push_back(usChannel);
	}

	Bind(m_vecEnableController, m_vecEnableController[0]);
	pHardware = GetHardware(m_vecEnableController[0]);
	pHardware->SetPeriod(0, dPeriod);
	pHardware->SetPinLevel(vecChannel, 3, 0, 1.5, 1.5, 0.8);
	double adEdge[EDGE_COUNT] = { 0, dPeriod / 2, 0, dPeriod / 2, dPeriod / 2, dPeriod * 4 / 5 };
	pHardware->SetEdge(vecChannel, 0, adEdge, WAVE_FORMAT::NRZ, IO_FORMAT::NRZ, COMPARE_MODE::EDGE);
	ClearBind();


	auto Diagnosis = [&](const char* lpszItemName)
	{
		StartTimer();
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<%s>\n", lpszFirstIndent, lpszItemName);
		nStopLineNo = LoadPattern(setStartLineNo, setStopLineNo, setBRAMOut, setDRAMIn);
		///<Run vector
		Bind(m_vecEnableController, m_vecEnableController[0]);
		pHardware = GetHardware(m_vecEnableController[0]);
		pHardware->SetRunParameter(0, nStopLineNo, TRUE, 0);
		pHardware->SynRun();
		ClearBind();

		WaitStop();

		int nRetVal = ResultAnalyze(lpszSecondIndent);
		if (0 != nRetVal)
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszSecondIndent);
			++nFailItemCount;
		}
		else
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszSecondIndent);
		}
		dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszSecondIndent, dTimeConsume, lpszTimeUnits);
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</%s>\n", lpszFirstIndent, lpszItemName);
	};

	do
	{
		///<No caprure saved
		setStartLineNo.clear();
		setStopLineNo.clear();
		setBRAMOut.insert(10);
		setDRAMIn.insert(100);
		setBRAMOut.insert(201);
		setDRAMIn.insert(251);
		Diagnosis("NoCapture");
		setStartLineNo.clear();
		setStopLineNo.clear();
		setBRAMOut.clear();
		setDRAMIn.clear();
		if (1 == m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='Next=BRAMSwitchLine'/>\n", lpszFirstIndent);
			break;
		}

		///<Capture the line BRAM switch to DRAM
		setStartLineNo.insert(10);
		setStopLineNo.insert(10);
		setBRAMOut.insert(10);
		setDRAMIn.insert(100);
		Diagnosis("BRAMSwitchLine");
		setStartLineNo.clear();
		setStopLineNo.clear();
		setBRAMOut.clear();
		setDRAMIn.clear();
		if (1 == m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='Next=DRAMFirstLine'/>\n", lpszFirstIndent);
			break;
		}

		///<Capture DRAM First Line
		setStartLineNo.insert(11);
		setStopLineNo.insert(11);
		setBRAMOut.insert(10);
		setDRAMIn.insert(100);
		Diagnosis("DRAMFirstLine");
		setStartLineNo.clear();
		setStopLineNo.clear();
		setBRAMOut.clear();
		setDRAMIn.clear();

		if (1 == m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='Next=DRAMLastLine'/>\n", lpszFirstIndent);
			break;
		}

		///<Capture the line switch from DRAM to BRAM
		setStartLineNo.insert(100);
		setStopLineNo.insert(100);
		setBRAMOut.insert(10);
		setDRAMIn.insert(100);
		Diagnosis("DRAMLastLine");
		setStartLineNo.clear();
		setStopLineNo.clear();
		setBRAMOut.clear();
		setDRAMIn.clear();

		if (1 == m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='Next=DRAMMiddleSave'/>\n", lpszFirstIndent);
			break;
		}

		///<DRAMMiddleSave
		setStartLineNo.insert(50);
		setStopLineNo.insert(200);
		setBRAMOut.insert(10);
		setDRAMIn.insert(1000);
		Diagnosis("DRAMMiddleSave");
		setStartLineNo.clear();
		setStopLineNo.clear();
		setBRAMOut.clear();
		setDRAMIn.clear();
		if (1 == m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='Next=MultiSection'/>\n", lpszFirstIndent);
			break;
		}

		setBRAMOut.insert(10);
		setDRAMIn.insert(DRAM_MAX_SAVE_FAIL_LINE_COUNT * 4 + 100);
		for (int nSectionIndex = 0; nSectionIndex < DRAM_MAX_SAVE_FAIL_LINE_COUNT; ++nSectionIndex)
		{
			int nStartLine = nSectionIndex * 4 + 10;
			setStartLineNo.insert(nStartLine);
			setStopLineNo.insert(nStartLine);
		}
		Diagnosis("MultiSection");
		setStartLineNo.clear();
		setStopLineNo.clear();
		setBRAMOut.clear();
		setDRAMIn.clear();
		if (1 == m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='Next=DRAMFullMemory'/>\n", lpszFirstIndent);
			break;
		}

		///<DRAM Full Memory Saved
		setStartLineNo.insert(5);
		setStopLineNo.insert(4000);
		setBRAMOut.insert(10);
		setDRAMIn.insert(2000);
		Diagnosis("DRAMFullMemory");
		setStartLineNo.clear();
		setStopLineNo.clear();
		setBRAMOut.clear();
		setDRAMIn.clear();
		if (1 == m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='Next=AllFullMemory'/>\n", lpszFirstIndent);
			break;
		}

		///<DRAM and BRAM memory all occupied
		setStartLineNo.insert(10);
		setStopLineNo.insert(4000);
		setBRAMOut.insert(10);
		setDRAMIn.insert(2000);
		Diagnosis("AllFullMemory");
		setStartLineNo.clear();
		setStopLineNo.clear();
		setBRAMOut.clear();
		setDRAMIn.clear();
		if (1 == m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='Next=AllFullMemory'/>\n", lpszFirstIndent);
			break;
		}

	} while (FALSE);

	dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
	if (nullptr != m_pReportDevice)
	{
		if (0 != nFailItemCount)
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszFirstIndent);
		}
		else
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszFirstIndent);
		}
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszFirstIndent, dTimeConsume, lpszTimeUnits);
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</SectionDiagnosis>\n", lpszBaseIndent);
	}

	if (0 != nFailItemCount)
	{
		return -1;
	}
	return 0;
}

int CDiagnosisHardwareCapture::DRAMSaveSelectedFailDiagnosis(const char* lpszBaseIndent)
{
	StartTimer();
	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };
	double dPeriod = 100;
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<SaveSelectedFailDiagnosis value='Period=%.0f'>\n", lpszBaseIndent, dPeriod);

	string strFirstIndent = lpszBaseIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();

	set<UINT> setBRAMOut = { 4, 80 };
	set<UINT> setDRAMIn = { 20, 150 };
	int nRetVal = SaveSelectedFailDiagnosis(setBRAMOut, setDRAMIn, dPeriod, lpszFirstIndent);
	if (0 != nRetVal)
	{
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszFirstIndent);
	}
	else
	{
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszFirstIndent);
	}
	dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszFirstIndent, dTimeConsume, lpszTimeUnits);
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</SaveSelectedFailDiagnosis>\n", lpszBaseIndent);
	if (0 != nRetVal)
	{
		return -1;
	}
	return 0;
}

int CDiagnosisHardwareCapture::SaveSelectedFailDiagnosis(const std::set<UINT>& setBRAMOut, const std::set<UINT>& setDRAMIn, double dPeriod, const char* lpszBaseIndent)
{
	set<BYTE> setUnsupported;
	vector<UINT> vecSupportedController;
	BYTE byControllerIndex = 0;
	BYTE bySlotNo = 0;
	CHardwareFunction* pHardware = nullptr;
	for (auto Controller : m_vecEnableController)
	{
		pHardware = GetHardware(Controller);
		if (pHardware->IsSupportSaveFailSelected())
		{
			vecSupportedController.push_back(Controller);
			continue;
		}
		bySlotNo = HDModule::Instance()->ID2Board(Controller, byControllerIndex);
		setUnsupported.insert(bySlotNo);
	}
	vector<UINT> vecDiagnosisController;
	for (auto Controller : vecSupportedController)
	{
		bySlotNo = HDModule::Instance()->ID2Board(Controller, byControllerIndex);
		if (setUnsupported.end() != setUnsupported.find(bySlotNo))
		{
			continue;
		}
		vecDiagnosisController.push_back(Controller);
	}
	if (0 == vecDiagnosisController.size())
	{
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Info>Unsupport to save selected fail.</Info>\n", lpszBaseIndent);
		return 0;
	}

	vector<UINT> vecBackup = m_vecEnableController;
	m_vecEnableController = vecDiagnosisController;

	set<UINT> setStartLineNo = { 10,100 };
	set<UINT> setStopLineNo = { 50, 200 };
	
	int nSectionCount = setBRAMOut.size();
	BOOL bWithDRAM = 0 != nSectionCount;

	if (nSectionCount != setDRAMIn.size())
	{
		return -2;
	}
	if (0 != nSectionCount)
	{
		setStartLineNo.clear();
		setStopLineNo.clear();
		int nFrontRAM = 0;
		auto iterBRAM = setBRAMOut.begin();
		auto iterDRAM = setDRAMIn.begin();
		while (setBRAMOut.end() != iterBRAM)
		{
			setStartLineNo.insert((nFrontRAM + *iterBRAM) / 2);
			setStopLineNo.insert((*iterBRAM + *iterDRAM) / 2);
			nFrontRAM = *iterDRAM;
			++iterBRAM;
			++iterDRAM;
		}
	}

	int nStopLineNo = 0;
	vector<USHORT> vecChannel;
	for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL; ++usChannel)
	{
		vecChannel.push_back(usChannel);
	}

	int nFailItemCount = 0;
	nStopLineNo = LoadPattern(setStartLineNo, setStopLineNo, setBRAMOut, setDRAMIn, TRUE);
	///<Run vector
	Bind(m_vecEnableController, m_vecEnableController[0]);
	pHardware = GetHardware(m_vecEnableController[0]);
	pHardware->SetPeriod(0, dPeriod);
	pHardware->SetPinLevel(vecChannel, 3, 0, 1.5, 1.5, 0.8);
	double adEdge[EDGE_COUNT] = { 0, dPeriod / 2, 0, dPeriod / 2, dPeriod / 2, dPeriod * 4 / 5 };
	pHardware->SetEdge(vecChannel, 0, adEdge, WAVE_FORMAT::NRZ, IO_FORMAT::NRZ, COMPARE_MODE::EDGE);
	pHardware->EnableSaveSelectedFail(TRUE);
	pHardware->SetRunParameter(0, nStopLineNo, bWithDRAM, 0);
	pHardware->SynRun();
	ClearBind();

	WaitStop();

	///<Disable saving select fail
	Bind(m_vecEnableController, m_vecEnableController[0]);
	pHardware = GetHardware(m_vecEnableController[0]);
	pHardware->EnableSaveSelectedFail(FALSE);
	ClearBind();

	int nRetVal = ResultAnalyze(lpszBaseIndent);
	m_vecEnableController = vecBackup;

	if (0 != nRetVal)
	{
		return -1;
	}
	return 0;
}

int CDiagnosisHardwareCapture::ResultAnalyze(const char* lpszBaseIndent)
{
	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };

	string strFirstIndent = lpszBaseIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();
	string strSecondIndent = strFirstIndent + IndentChar();
	const char* lpszSecondIndent = strSecondIndent.c_str();
	BYTE bySlotNo = 0;
	byte byBoardControllerIndex = 0;
	CHardwareFunction* pHardware = nullptr;
	vector<CHardwareFunction::DATA_RESULT> vecBRAMCaptureData;
	vector<CHardwareFunction::DATA_RESULT> vecDRAMCaptureData;
	vector<CHardwareFunction::DATA_RESULT> vecBRAMFailData;
	vector<CHardwareFunction::DATA_RESULT> vecDRAMFailData;
	BOOL bItemFail = FALSE;
	for (auto Controller : m_vecEnableController)
	{
		StartTimer();
		BOOL bControllerFail = FALSE;
		pHardware = GetHardware(Controller);
		pHardware->GetCapture(vecBRAMCaptureData, vecDRAMCaptureData);
		pHardware->GetFailData(vecBRAMFailData, vecDRAMFailData);

		bySlotNo = HDModule::Instance()->ID2Board(Controller, byBoardControllerIndex);
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<controller value='%d, slot value = %d'>\n", lpszFirstIndent, byBoardControllerIndex, bySlotNo);

		int nExpectedIndex = Controller % 2;
		char lpszValue[128] = { 0 };
		const char* lpszCheckype[2] = { "Capture", "Fail" };
		const char* lpszRAMType[2] = { "BRAM", "DRAM" };
		for (int nCheckType = 0; nCheckType < 2; ++nCheckType)
		{
			for (int nRAMType = 0; nRAMType < 2; ++nRAMType)
			{
				const auto* pvecExpected = &m_avecBRAMExpected[0];
				const auto* pvecReal = &vecBRAMCaptureData;
				if (0 == nCheckType)
				{
					if (0 == nRAMType)
					{
						pvecExpected = &m_avecBRAMExpected[nExpectedIndex];
						pvecReal = &vecBRAMCaptureData;
					}
					else
					{
						pvecExpected = &m_avecDRAMExpected[nExpectedIndex];
						pvecReal = &vecDRAMCaptureData;
					}
				}
				else
				{
					if (0 == nRAMType)
					{
						pvecExpected = &m_avecBRAMFailExpected[nExpectedIndex];
						pvecReal = &vecBRAMFailData;
					}
					else
					{
						pvecExpected = &m_avecDRAMFailExpected[nExpectedIndex];
						pvecReal = &vecDRAMFailData;
					}
				}
				if (*pvecExpected == *pvecReal)
				{
					///<Pass
					continue;
				}

				bControllerFail = TRUE;
				int nErrorCount = 0;
				auto iterReal = pvecReal->begin();
				auto iterExpected = pvecExpected->begin();
				while (pvecReal->end() != iterReal && pvecExpected->end() != iterExpected)
				{
					if (iterReal->m_nLineNo != iterExpected->m_nLineNo || iterReal->m_usData != iterExpected->m_usData)
					{
						if (ERROR_PRINT < nErrorCount++)
						{
							break;
						}
						sprintf_s(lpszValue, sizeof(lpszValue), "value='%s %s expect addr='0x%X' Real addr=0x%X Expect='0x%X' Real=0x%X'",
							lpszCheckype[nCheckType], lpszRAMType[nCheckType], iterExpected->m_nLineNo, iterReal->m_nLineNo, iterExpected->m_usData, iterReal->m_usData);
						m_pReportDevice->PrintfToDataLog(IHDReportDevice::Error, "%s<data retValue='false' %s/>\n", lpszSecondIndent, lpszValue);
					}
					++iterReal;
					++iterExpected;
				}
				if (ERROR_PRINT >= nErrorCount && pvecReal->size() != pvecExpected->size())
				{
					while (pvecReal->end() != iterReal)
					{
						if (ERROR_PRINT < nErrorCount++)
						{
							break;
						}
						sprintf_s(lpszValue, sizeof(lpszValue), "value='%s %s expect addr=- Real addr=0x%X Expect=- Real=0x%X'",
							lpszCheckype[nCheckType], lpszRAMType[nCheckType], iterReal->m_nLineNo, iterReal->m_usData);
						m_pReportDevice->PrintfToDataLog(IHDReportDevice::Error, "%s<data retValue='false' %s/>\n", lpszSecondIndent, lpszValue);
						++iterReal;
					}
					while (pvecExpected->end() != iterExpected)
					{
						if (ERROR_PRINT < nErrorCount++)
						{
							break;
						}
						sprintf_s(lpszValue, sizeof(lpszValue), "value='%s %s expect addr=0x%X Real addr=- Expect=0x%X Real=-'",
							lpszCheckype[nCheckType], lpszRAMType[nCheckType], iterExpected->m_nLineNo, iterExpected->m_usData);
						m_pReportDevice->PrintfToDataLog(IHDReportDevice::Error, "%s<data retValue='false' %s/>\n", lpszSecondIndent, lpszValue);
						++iterExpected;
					}
				}
			}
		}
		if (bControllerFail)
		{
			m_setFailController.insert(Controller);
			bItemFail = TRUE;
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszSecondIndent);
		}
		else
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszSecondIndent);
		}
		dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
		if (nullptr != m_pReportDevice)
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszSecondIndent, dTimeConsume, lpszTimeUnits);
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</controller>\n", lpszFirstIndent);
		}
	}
	if (bItemFail)
	{
		return -1;
	}
	return 0;
}

void CDiagnosisHardwareCapture::ShowUIResult()
{
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

		m_pReportDevice->PrintfToUi("\t Slot %d, controller %d\n", bySlotNo, byBoardControllerIndex);
	}
	m_setFailController.clear();
}

int CDiagnosisHardwareCapture::LoadPattern(const std::set<UINT>& setStartLineNo, const std::set<UINT>& setStopLineNo, 
	const std::set<UINT>& setBRAMOut, const std::set<UINT>& setDRAMIn, BOOL bSaveSelectFail)
{
	for (auto& Data : m_avecBRAMExpected)
	{
		Data.clear();
	}
	for (auto& Data : m_avecDRAMExpected)
	{
		Data.clear();
	}
	for (auto& Data : m_avecBRAMFailExpected)
	{
		Data.clear();
	}
	for (auto& Data : m_avecDRAMFailExpected)
	{
		Data.clear();
	}

	int nLineCount = 0;
	auto GetLineCount = [&](const set<UINT>& setLineNo)
	{
		if (0 != setLineNo.size())
		{
			nLineCount = nLineCount > *setLineNo.rbegin() ? nLineCount : *setLineNo.rbegin();
		}
	};
	GetLineCount(setStartLineNo);
	GetLineCount(setStopLineNo);
	GetLineCount(setBRAMOut);
	GetLineCount(setDRAMIn);

	nLineCount += 100;

	char lpszPattern[17] = { 0 };

	auto SaveResult = [&](int nContrllerType, BOOL bBRAM, int nLineNo)
	{
		auto& vecResultExpected = bBRAM ? m_avecBRAMExpected[0 == nContrllerType ? 0 : 1] : m_avecDRAMExpected[0 == nContrllerType ? 0 : 1];
		const int nMaxCaptureLineCount = bBRAM ? BRAM_MAX_SAVE_CAPTURE_COUNT : DRAM_MAX_SAVE_CAPTURE_COUNT;
		if (nMaxCaptureLineCount <= vecResultExpected.size())
		{
			return;
		}
		CHardwareFunction::DATA_RESULT DataResult;
		DataResult.m_nLineNo = nLineNo;
		for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL; ++usChannel)
		{
			if ('L' == lpszPattern[usChannel])
			{
				DataResult.m_usData |= 1 << usChannel;
			}
		}
		vecResultExpected.push_back(DataResult);
	};

	auto SaveFailResult = [&](int nControllerType, BOOL bBRAM, int nLineNo)
	{
		if (nullptr == lpszPattern)
		{
			return;
		}
		auto& vecFailExpected = bBRAM ? m_avecBRAMFailExpected[0 == nControllerType ? 0 : 1] : m_avecDRAMFailExpected[0 == nControllerType ? 0 : 1];
		const int nMaxFaiLineCount = bBRAM ? BRAM_MAX_SAVE_FAIL_LINE_COUNT : DRAM_MAX_SAVE_FAIL_LINE_COUNT;
		if (nMaxFaiLineCount <= vecFailExpected.size())
		{
			return;
		}
		CHardwareFunction::DATA_RESULT DataResult;
		DataResult.m_nLineNo = nLineNo;
		for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL; ++usChannel)
		{
			if ('L' == lpszPattern[usChannel])
			{
				DataResult.m_usData |= 1 << usChannel;
			}
		}
		if (0 != DataResult.m_usData)
		{
			vecFailExpected.push_back(DataResult);
		}
	};

	BOOL bBRAM = TRUE;
	vector<UINT> vecCurController;
	CHardwareFunction* pHardware = nullptr;
	int nBRAMLineNo = 0;
	int nDRAMLineNo = 0;
	int* pnCurLineNo = &nBRAMLineNo;
	BOOL bCapture = FALSE;
	
	for (int nControllerType = 0; nControllerType < 2; ++nControllerType)
	{
		vecCurController.clear();
		for (auto Controller : m_vecEnableController)
		{
			if (nControllerType == Controller % 2)
			{
				vecCurController.push_back(Controller);
			}
		}

		Bind(vecCurController, vecCurController[0]);
		pHardware = GetHardware(vecCurController[0]);
		CPattern Pattern(*pHardware);
		bBRAM = TRUE;
		bCapture = FALSE;
		nBRAMLineNo = 0;
		nDRAMLineNo = 0;
		pnCurLineNo = &nBRAMLineNo;
		BOOL bSwitch = FALSE;
		BOOL bFail = TRUE;
		for (int nLineIndex = 0; nLineIndex < nLineCount; ++nLineIndex)
		{
			bSwitch = FALSE;
			bFail = TRUE;
			if (setStartLineNo.end() != setStartLineNo.find(nLineIndex))
			{
				bCapture = TRUE;
			}
			switch (nLineIndex % 8)
			{
			case 0:
				if (0 == nControllerType)
				{
					strcpy_s(lpszPattern, sizeof(lpszPattern), "L1L1L1L1L1L1L1L1");
				}
				else
				{
					strcpy_s(lpszPattern, sizeof(lpszPattern), "1L1L1L1L1L1L1L1L");
				}
				break;
			case 1:
				if (1 == nControllerType)
				{
					strcpy_s(lpszPattern, sizeof(lpszPattern), "L1L1L1L1L1L1L1L1");
				}
				else
				{
					strcpy_s(lpszPattern, sizeof(lpszPattern), "1L1L1L1L1L1L1L1L");
				}
				break;
			case 2:
				if (1 == nControllerType)
				{
					strcpy_s(lpszPattern, sizeof(lpszPattern), "1L1L1L1L1L1L1L1L");
				}
				else
				{
					strcpy_s(lpszPattern, sizeof(lpszPattern), "L1L1L1L1L1L1L1L1");
				}
				break;
			case 3:
				if (0 == nControllerType)
				{
					strcpy_s(lpszPattern, sizeof(lpszPattern), "1L1L1L1L1L1L1L1L");
				}
				else
				{
					strcpy_s(lpszPattern, sizeof(lpszPattern), "L1L1L1L1L1L1L1L1");
				}
				break;
			case 4:
				if (0 == nControllerType)
				{
					strcpy_s(lpszPattern, sizeof(lpszPattern), "1111LLLL1111LLLL");
				}
				else
				{
					strcpy_s(lpszPattern, sizeof(lpszPattern), "LLLL1111LLLL1111");
				}
				break;
			case 5:
				if (1 == nControllerType)
				{
					strcpy_s(lpszPattern, sizeof(lpszPattern), "1111LLLL1111LLLL");
				}
				else
				{
					strcpy_s(lpszPattern, sizeof(lpszPattern), "LLLL1111LLLL1111");
				}
				break;
			case 6:
				if (0 == nControllerType)
				{
					strcpy_s(lpszPattern, sizeof(lpszPattern), "1111111111111111");
				}
				else
				{
					strcpy_s(lpszPattern, sizeof(lpszPattern), "HHHHHHHHHHHHHHHH");
				}
				bFail = FALSE;
				break;
			case 7:
				if (1 == nControllerType)
				{
					strcpy_s(lpszPattern, sizeof(lpszPattern), "1111111111111111");
				}
				else
				{
					strcpy_s(lpszPattern, sizeof(lpszPattern), "HHHHHHHHHHHHHHHH");
				}
				bFail = FALSE;
				break;
			default:
				break;
			}
			if (bCapture)
			{
				SaveResult(nControllerType, bBRAM, *pnCurLineNo);
			}
			if (bFail)
			{
				SaveFailResult(nControllerType, bBRAM, *pnCurLineNo);
			}
			if (setBRAMOut.end() != setBRAMOut.find(nLineIndex) || setDRAMIn.end() != setDRAMIn.find(nLineIndex))
			{
				bSwitch = TRUE;
			}
			string strParallelInstr = "";
			if (bSaveSelectFail && 0 == nLineIndex)
			{
				strParallelInstr = "FAIL_ON";
			}
			Pattern.AddPattern(*pnCurLineNo, bBRAM, lpszPattern, 0, "INC", strParallelInstr.c_str(), 0, bCapture, bSwitch);
			++* pnCurLineNo;
			if (bSwitch)
			{
				if (setBRAMOut.end() != setBRAMOut.find(nLineIndex))
				{
					bBRAM = FALSE;
					pnCurLineNo = &nDRAMLineNo;
				}
				else if (setDRAMIn.end() != setDRAMIn.find(nLineIndex))
				{
					bBRAM = TRUE;
					pnCurLineNo = &nBRAMLineNo;
				}
			}

			if (setStopLineNo.end() != setStopLineNo.find(nLineIndex))
			{
				bCapture = FALSE;
			}
		}
		Pattern.Load();
		ClearBind();
	}
	return nBRAMLineNo - 1;
}