#include "DiagnosisFailSelected.h"
#include <set>
#include "..\HDModule.h"
#include "..\Pattern.h"
using namespace std;

CDiagnosisFailSelected::CDiagnosisFailSelected()
{

}

CDiagnosisFailSelected::~CDiagnosisFailSelected()
{

}

int CDiagnosisFailSelected::QueryInstance(const char* lpszName, void** ppPoint)
{
	return -1;
}

void CDiagnosisFailSelected::Release()
{

}

const char* CDiagnosisFailSelected::Name() const
{
	return "Saving Fail Selected Diagnosis";
}

int CDiagnosisFailSelected::GetChildren(STSVector<IHDDoctorItem*>& vecChildren) const
{
	return 0;
}

int CDiagnosisFailSelected::Doctor(IHDReportDevice* pReportDevice)
{
	m_pReportDevice = pReportDevice;
	int nRetVal = -1;

	StartTimer();
	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };
	const char* lpszBaseIndent = IndentFormat(); 
	string strFirstIndent = lpszBaseIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<SaveFailSelectedDiagnosis>\n", lpszBaseIndent);
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

		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Info>Undiagnosable for slot %d controller %d is not existed.</Info>\n", lpszNextIndent, bySlotNo, byBoardControllerIndex);
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</Controller>\n", IndentFormat());
	}

	///<Check supported
	set<UINT> setUnsupportedController;
	CHardwareFunction* pHardware = nullptr;
	BYTE byMutualContrller = 0;
	UINT uRelatedControllerID = 0;
	for (auto Controller : m_vecEnableController)
	{
		bySlotNo = HDModule::Instance()->ID2Board(Controller, byBoardControllerIndex);
		pHardware = GetHardware(Controller);
		if (pHardware->IsSupportSaveFailSelected())
		{
			///<Check whether the controller support saving the fail information of selected line
			if (0 == byBoardControllerIndex % 2)
			{
				byMutualContrller = byBoardControllerIndex + 1;
			}
			else
			{
				byMutualContrller = byBoardControllerIndex - 1;
			}
			uRelatedControllerID = HDModule::Instance()->GetID(bySlotNo, byMutualContrller);
			pHardware = GetHardware(uRelatedControllerID);
			if (pHardware->IsSupportSaveFailSelected())
			{
				continue;
			}
			m_pReportDevice->PrintfToUi(IHDReportDevice::Fail);
			m_pReportDevice->PrintfToUi("\t Slot %d, Controller %d\n", bySlotNo, byBoardControllerIndex);
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Controller value='%d, slot value=%d'>\n", IndentFormat(), bySlotNo, byBoardControllerIndex);

			m_pReportDevice->PrintfToUi("\t\t Undiagnosable for slot %d controller %d is not supported.", bySlotNo, byMutualContrller);
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Undiagnosable for slot %d controller %d is not supported./>\n", lpszNextIndent, bySlotNo, byBoardControllerIndex);
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</Controller>\n", IndentFormat());
			continue;
		}
		setUnsupportedController.insert(Controller);
	}

	if (0 != setUnsupportedController.size())
	{
		vector<UINT> vecEnable = m_vecEnableController;
		m_vecEnableController.clear();
		for (auto Controller : vecEnable)
		{
			if (setUnsupportedController.end() != setUnsupportedController.find(Controller))
			{
				continue;
			}
			m_vecEnableController.push_back(Controller);
		}
	}

	if (0 == m_vecEnableController.size())
	{
		m_pReportDevice->PrintfToUi("\t All controllers are not supported\n", bySlotNo, byBoardControllerIndex);
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Item>All controllers are not supported</Item>\n", lpszNextIndent);
		dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszNextIndent, dTimeConsume, lpszTimeUnits);

		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</SaveFailSelectedDiagnosis>\n", lpszBaseIndent);
		return 0;
	}

	auto EnableFailSelected = [&](BOOL bEnable)
	{
		Bind(m_vecEnableController, m_vecEnableController[0]);
		pHardware = GetHardware(m_vecEnableController[0]);
		pHardware->EnableSaveSelectedFail(bEnable);
		ClearBind();
	};

	int nFailItemCount = 0;
	int nItemResult = 0;
	EnableFailSelected(TRUE);
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
	EnableFailSelected(FALSE);

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

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</SaveFailSelectedDiagnosis>\n", lpszBaseIndent);

	return nRetVal;
}

bool CDiagnosisFailSelected::IsUserCheck() const
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

int CDiagnosisFailSelected::BRAMRateDiagnosis(const char* lpszBaseIndent)
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
		pHardware->EnableSaveSelectedFail(TRUE);
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

int CDiagnosisFailSelected::BRAMDiagnosis(const char* lpszBaseIndent)
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
		///<No fail line number saved
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

int CDiagnosisFailSelected::BRAMSectionDiagnosis(const char* lpszBaseIndent)
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


	auto Diagnosis = [&](const char* lpszItemName, BOOL bAllFailPattern)
	{
		StartTimer();
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<%s>\n", lpszFirstIndent, lpszItemName);
		nStopLineNo = LoadPattern(setStartLineNo, setStopLineNo, setBRAMOut, setDRAMIn, bAllFailPattern);
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
		///<No fail line number saved
		setStartLineNo.clear();
		setStopLineNo.clear();
		Diagnosis("NoFailSaved", FALSE);
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
		Diagnosis("OneSection", FALSE);
		
		if (1 == m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='Next=OneLineSaved'/>\n", lpszFirstIndent);
			break;
		}

		///<One line each sector
		setStartLineNo.insert(0);
		setStopLineNo.insert(1);
		setStartLineNo.insert(10);
		setStopLineNo.insert(11);
		Diagnosis("OneLineSaved", TRUE);
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
			setStopLineNo.insert(nStartLine + 1);
		}
		Diagnosis("MultiSection", TRUE);
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
		Diagnosis("FullMemorySaved", FALSE);
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

int CDiagnosisFailSelected::DRAMDiagnosis(const char* lpszBaseIndent)
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

	return 0;
}

int CDiagnosisFailSelected::DRAMSectionDiagnosis(const char* lpszBaseIndent)
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


	auto Diagnosis = [&](const char* lpszItemName, BOOL bAllFailPattern)
	{
		StartTimer();
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<%s>\n", lpszFirstIndent, lpszItemName);
		nStopLineNo = LoadPattern(setStartLineNo, setStopLineNo, setBRAMOut, setDRAMIn, bAllFailPattern);
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
		///<No fail line number saved
		setStartLineNo.clear();
		setStopLineNo.clear();
		setBRAMOut.insert(10);
		setDRAMIn.insert(100);
		setBRAMOut.insert(201);
		setDRAMIn.insert(251);
		Diagnosis("NoFailSaved", FALSE);
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
		Diagnosis("DRAMMiddleSave", TRUE);
		setStartLineNo.clear();
		setStopLineNo.clear();
		setBRAMOut.clear();
		setDRAMIn.clear();
		if (1 == m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='Next=DRAMFirstLineStart'/>\n", lpszFirstIndent);
			break;
		}


		///<DRAM First Line Start
		setStartLineNo.insert(11);
		setStopLineNo.insert(12);
		setBRAMOut.insert(10);
		setDRAMIn.insert(100);
		Diagnosis("DRAMFirstLineStart", TRUE);
		setStartLineNo.clear();
		setStopLineNo.clear();
		setBRAMOut.clear();
		setDRAMIn.clear();
		if (1 == m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='Next=DRAMFirstLineStop'/>\n", lpszFirstIndent);
			break;
		}

		///<DRAM First Line Stop
		setStartLineNo.insert(10);
		setStopLineNo.insert(11);
		setBRAMOut.insert(10);
		setDRAMIn.insert(100);
		Diagnosis("DRAMFirstLineStop", TRUE);
		setStartLineNo.clear();
		setStopLineNo.clear();
		setBRAMOut.clear();
		setDRAMIn.clear();

		if (1 == m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='Next=DRAMLastLineStart'/>\n", lpszFirstIndent);
			break;
		}

		///<DRAM Last Line Start
		setStartLineNo.insert(100);
		setStopLineNo.insert(101);
		setBRAMOut.insert(10);
		setDRAMIn.insert(100);
		Diagnosis("DRAMLastLineStart", TRUE);
		setStartLineNo.clear();
		setStopLineNo.clear();
		setBRAMOut.clear();
		setDRAMIn.clear();
		if (1 == m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='Next=DRAMLastLineStop'/>\n", lpszFirstIndent);
			break;
		}

		///<DRAM Last Line Stop
		setStartLineNo.insert(99);
		setStopLineNo.insert(100);
		setBRAMOut.insert(10);
		setDRAMIn.insert(100);
		Diagnosis("DRAMLastLineStop", TRUE);
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
			setStopLineNo.insert(nStartLine + 1);
		}
		Diagnosis("MultiSection", TRUE);
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
		Diagnosis("DRAMFullMemory", FALSE);
		setStartLineNo.clear();
		setStopLineNo.clear();
		setBRAMOut.clear();
		setDRAMIn.clear();
		if (1 == m_pReportDevice->IsStop())
		{
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<UserStop value='Next=AllFullMemory'/>\n", lpszFirstIndent);
			break;
		}

		///<All Fail Memory saved
		setStartLineNo.insert(10);
		setStopLineNo.insert(4000);
		setBRAMOut.insert(10);
		setDRAMIn.insert(2000);
		Diagnosis("AllFullMemory", FALSE);
		setStartLineNo.clear();
		setStopLineNo.clear();
		setBRAMOut.clear();
		setDRAMIn.clear();

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

int CDiagnosisFailSelected::ResultAnalyze(const char* lpszBaseIndent)
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
	vector<CHardwareFunction::DATA_RESULT> vecBRAMFailData;
	vector<CHardwareFunction::DATA_RESULT> vecDRAMFailData;
	BOOL bItemFail = FALSE;
	for (auto Controller : m_vecEnableController)
	{
		StartTimer();
		BOOL bControllerFail = FALSE;
		pHardware = GetHardware(Controller);
		pHardware->GetFailData(vecBRAMFailData, vecDRAMFailData);

		bySlotNo = HDModule::Instance()->ID2Board(Controller, byBoardControllerIndex);
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<controller value='%d, slot value = %d'>\n", lpszFirstIndent, byBoardControllerIndex, bySlotNo);

		int nExpectedIndex = Controller % 2;
		char lpszValue[128] = { 0 };
		///<Check BRAM
		int nErrorCount = 0;
		const char* lpszRAMType[2] = { "BRAM", "DRAM" };
		for (int nRAMType = 0; nRAMType < 2; ++nRAMType)
		{
			const auto& vecExpected = 0 == nRAMType ? m_avecBRAMFailExpected[nExpectedIndex] : m_avecDRAMFailExpected[nExpectedIndex];
			auto& vecReal = 0 == nRAMType ? vecBRAMFailData : vecDRAMFailData;
			if (vecExpected == vecReal)
			{
				continue;
			}
			nErrorCount = 0;
			bControllerFail = TRUE;
			auto iterReal = vecReal.begin();
			auto iterExpected = vecExpected.begin();
			while (vecReal.end() != iterReal && vecExpected.end() != iterExpected)
			{
				if (iterReal->m_nLineNo != iterExpected->m_nLineNo || iterReal->m_usData != iterExpected->m_usData)
				{
					if (ERROR_PRINT < nErrorCount++)
					{
						break;
					}
					sprintf_s(lpszValue, sizeof(lpszValue), "value='%s expect addr='0x%X' Real addr=0x%X Expect='0x%X' Real=0x%X'",
						lpszRAMType[nRAMType], iterExpected->m_nLineNo, iterReal->m_nLineNo, iterExpected->m_usData, iterReal->m_usData);
					m_pReportDevice->PrintfToDataLog(IHDReportDevice::Error, "%s<data retValue='false' %s/>\n", lpszSecondIndent, lpszValue);
				}
				++iterReal;
				++iterExpected;
			}
			if (ERROR_PRINT >= nErrorCount && vecReal.size() != vecExpected.size())
			{
				while (vecReal.end() != iterReal)
				{
					if (ERROR_PRINT < nErrorCount++)
					{
						break;
					}
					sprintf_s(lpszValue, sizeof(lpszValue), "value='%s expect addr=- Real addr=0x%X Expect=- Real=0x%X'",
						lpszRAMType[nRAMType], iterReal->m_nLineNo, iterReal->m_usData);
					m_pReportDevice->PrintfToDataLog(IHDReportDevice::Error, "%s<data retValue='false' %s/>\n", lpszSecondIndent, lpszValue);
					++iterReal;
				}
				while (vecExpected.end() != iterExpected)
				{
					if (ERROR_PRINT < nErrorCount++)
					{
						break;
					}
					sprintf_s(lpszValue, sizeof(lpszValue), "value='%s expect addr=0x%X Real addr=- Expect=0x%X Real=-'",
						lpszRAMType[nRAMType], iterExpected->m_nLineNo, iterExpected->m_usData);
					m_pReportDevice->PrintfToDataLog(IHDReportDevice::Error, "%s<data retValue='false' %s/>\n", lpszSecondIndent, lpszValue);
					++iterExpected;
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

void CDiagnosisFailSelected::ShowUIResult()
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

int CDiagnosisFailSelected::LoadPattern(const std::set<UINT>& setStartLineNo, const std::set<UINT>& setStopLineNo, std::set<UINT>& setBRAMOut, std::set<UINT>& setDRAMIn, BOOL bAllFail)
{
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

	const char* lpszParallelCMD = nullptr;
	char lpszPattern[17] = { 0 };

	auto SaveResult = [&](int nControllerType, BOOL bBRAM, int nLineNo)
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

	BOOL bSRAM = TRUE;
	vector<UINT> vecCurController;
	CHardwareFunction* pHardware = nullptr;
	int nBRAMLineNo = 0;
	int nDRAMLineNo = 0;
	int* pnCurLineNo = &nBRAMLineNo;
	BOOL bSaveFail = FALSE;
	int nVectorType = bAllFail ? 6 : 8;
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
		bSRAM = TRUE;
		bSaveFail = FALSE;
		nBRAMLineNo = 0;
		nDRAMLineNo = 0;
		pnCurLineNo = &nBRAMLineNo;
		BOOL bPass = FALSE;
		BOOL bSwitch = FALSE;
		for (int nLineIndex = 0; nLineIndex < nLineCount; ++nLineIndex)
		{
			bSwitch = FALSE;
			bPass = FALSE;
			lpszParallelCMD = "";
			if (setStartLineNo.end() != setStartLineNo.find(nLineIndex))
			{
				lpszParallelCMD = "FAIL_ON";
				bSaveFail = TRUE;
			}
			else if (setStopLineNo.end() != setStopLineNo.find(nLineIndex))
			{
				lpszParallelCMD = "FAIL_OFF";
				bSaveFail = FALSE;
			}
			switch (nLineIndex % nVectorType)
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
					strcpy_s(lpszPattern, sizeof(lpszPattern), "0000000000000000");
				}
				else
				{
					strcpy_s(lpszPattern, sizeof(lpszPattern), "LLLLLLLLLLLLLLLL");
				}
				bPass = TRUE;
				break;
			case 7:
				if (1 == nControllerType)
				{
					strcpy_s(lpszPattern, sizeof(lpszPattern), "0000000000000000");
				}
				else
				{
					strcpy_s(lpszPattern, sizeof(lpszPattern), "LLLLLLLLLLLLLLLL");
				}
				bPass = TRUE;
				break;
			default:
				break;
			}
			if (bSaveFail && !bPass)
			{
				SaveResult(nControllerType, bSRAM, *pnCurLineNo);
			}
			if (setBRAMOut.end() != setBRAMOut.find(nLineIndex) || setDRAMIn.end() != setDRAMIn.find(nLineIndex))
			{
				bSwitch = TRUE;
			}
			Pattern.AddPattern(*pnCurLineNo, bSRAM, lpszPattern, 0, "INC", lpszParallelCMD, 0, FALSE, bSwitch);
			++* pnCurLineNo;
			if (bSwitch)
			{
				if (setBRAMOut.end() != setBRAMOut.find(nLineIndex))
				{
					bSRAM = FALSE;
					pnCurLineNo = &nDRAMLineNo;
				}
				else if (setDRAMIn.end() != setDRAMIn.find(nLineIndex))
				{
					bSRAM = TRUE;
					pnCurLineNo = &nBRAMLineNo;
				}
			}
		}
		Pattern.Load();
		ClearBind();
	}
	return nBRAMLineNo - 1;
}
