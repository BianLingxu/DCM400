#include "DiagnosisPMU.h"
#include "..\HDModule.h"
#include "IHDReportDevice.h"
#include <iterator>
using namespace std;
CDiagnosisPMU::CDiagnosisPMU()
{
}

CDiagnosisPMU::~CDiagnosisPMU()
{
}

int CDiagnosisPMU::QueryInstance(const char* lpszName, void** ppPoint)
{
    return -1;
}

void CDiagnosisPMU::Release()
{
}

const char* CDiagnosisPMU::Name() const
{
    return "PMU Diagnosis";
}

int CDiagnosisPMU::GetChildren(STSVector<IHDDoctorItem*>& vecChildren) const
{
    return 0;
}

bool CDiagnosisPMU::IsUserCheck() const
{
    switch (m_UserRole)
    {
    case CDiagnosisItem::PROCUCTION:
        return true;
        break;
	case CDiagnosisItem::DEVELOPER:
		return true;
        break;
    case CDiagnosisItem::USER:
#ifdef _DEBUG
        return true;
#endif // _DEBUG

        return false;
        break;
    default:
        break;
    }
    return false;
}

int CDiagnosisPMU::Doctor(IHDReportDevice* pReportDevice)
{
    StartTimer();
    m_pReportDevice = pReportDevice;

	double dTimeConsume = 0;
	char lpszTimeUnits[4] = { 0 };
	int nRetVal = 0;
	const char* lpszBaseIndent = IndentFormat();
	std::string strFirstIndent = IndentFormat() + IndentChar();
	std::string strSecondeIndent = strFirstIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<PMUDiagnosis>\n", lpszBaseIndent);
	CheckDiagnosable();
	BYTE bySlotNo = 0;
	BYTE byBoardControllerIndex = 0;
	for (auto& Undiagnosable : m_mapUndiagnosableController)
	{
		bySlotNo = HDModule::Instance()->ID2Board(Undiagnosable.first, byBoardControllerIndex);
		m_pReportDevice->PrintfToUi(IHDReportDevice::Fail);
		m_pReportDevice->PrintfToUi("\t Slot %d, Controller %d\n", bySlotNo, byBoardControllerIndex);

		bySlotNo = HDModule::Instance()->ID2Board(Undiagnosable.second, byBoardControllerIndex);
		m_pReportDevice->PrintfToUi("\t\t Undiagnosable for slot %d controller %d is not existed.", bySlotNo, byBoardControllerIndex);

		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Controller value='%d, slot value=%d'>\n", IndentFormat());
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Undiagnosable for slot %d controller %d is not existed./>\n", lpszFirstIndent,
			bySlotNo, byBoardControllerIndex);
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</Controller>\n", lpszBaseIndent);
	}

	int nFailItem = 0;
	int nItemResult = 0;

	Connect();

	if (0 == m_vecEnableController.size())
	{
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Item>No controller valid to be diagnosed</Item>\n", lpszFirstIndent);
	}
	else
	{
		do
		{
			nItemResult = Diagnosis(lpszFirstIndent, TRUE);
			if (0 != nItemResult)
			{
				++nFailItem;
			}
			nItemResult = Diagnosis(lpszFirstIndent, FALSE);
			if (0 != nItemResult)
			{
				++nFailItem;
			}
		} while (false);
	}
	Disconnect();

	if (0 == nFailItem)
	{
		nRetVal = 0;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszFirstIndent);
	}
	else
	{
		nRetVal = -1;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszFirstIndent);
	}

	dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszFirstIndent, dTimeConsume, lpszTimeUnits);

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</PMUDiagnosis>\n", lpszBaseIndent);

    return nRetVal;
}

void CDiagnosisPMU::CheckDiagnosable()
{
	if (0 == m_vecEnableController.size() && 0 == m_mapUndiagnosableController.size())
	{
		return;
	}

	set<UINT> setController;
	for (auto uControllerID : m_vecEnableController)
	{
		setController.insert(uControllerID);
	}
	m_vecEnableController.clear();

	for (auto& Undiagnosable : m_mapUndiagnosableController)
	{
		setController.insert(Undiagnosable.first);
	}
	m_mapUndiagnosableController.clear();

	BYTE bySlotNo = 0;
	BYTE byBoardControllerIndex = 0;
	BYTE byRelatedController = 0;
	UINT uRelatedControllerID = 0;
	for (auto uControllerID : setController)
	{
		bySlotNo = HDModule::Instance()->ID2Board(uControllerID, byBoardControllerIndex);
		if (0 == byBoardControllerIndex % 2)
		{
			byRelatedController = byBoardControllerIndex + 1;
		}
		else
		{
			byRelatedController = byBoardControllerIndex - 1;
		}
		uRelatedControllerID = HDModule::Instance()->GetID(bySlotNo, byRelatedController);
		if (setController.end() == setController.find(uRelatedControllerID))
		{
			m_mapUndiagnosableController.insert(make_pair(uRelatedControllerID, uControllerID));
		}
	}

	for (auto& undiagnosable : m_mapUndiagnosableController)
	{
		auto iterController = setController.find(undiagnosable.first);
		if (setController.end() != iterController)
		{
			setController.erase(iterController);
		}
	}

	copy(setController.begin(), setController.end(), back_inserter(m_vecEnableController));
}

void CDiagnosisPMU::Connect()
{
	vector<USHORT> vecBoardChannel;
	for (USHORT usChannel = 0; usChannel < DCM_MAX_CHANNELS_PER_BOARD; ++usChannel)
	{
		vecBoardChannel.push_back(usChannel);
	}
	set<BYTE> setSlot;
	BYTE bySlotNo = 0;
	BYTE byController = 0;

	for (auto uControllerID : m_vecEnableController)
	{
		bySlotNo = HDModule::Instance()->ID2Board(uControllerID, byController);
		if (setSlot.end() == setSlot.find(bySlotNo))
		{
			CHardwareFunction* pHardware = GetHardware(uControllerID);
			pHardware->SetFunctionRelay(vecBoardChannel);
			setSlot.insert(bySlotNo);
		}
	}
}

void CDiagnosisPMU::Disconnect()
{
	vector<USHORT> vecBoardChannel;
	for (USHORT usChannel = 0; usChannel < DCM_MAX_CHANNELS_PER_BOARD; ++usChannel)
	{
		vecBoardChannel.push_back(usChannel);
	}
	set<BYTE> setSlot;
	BYTE bySlotNo = 0;
	BYTE byController = 0;

	for (auto uControllerID : m_vecEnableController)
	{
		bySlotNo = HDModule::Instance()->ID2Board(uControllerID, byController);
		if (setSlot.end() == setSlot.find(bySlotNo))
		{
			CHardwareFunction* pHardware = GetHardware(uControllerID);
			pHardware->SetFunctionRelay(vecBoardChannel, FALSE);
			setSlot.insert(bySlotNo);
		}
	}
}

int CDiagnosisPMU::Diagnosis(const char* lpszBaseIndent, BOOL bMV)
{
	StartTimer();
	if (bMV)
	{
		m_pReportDevice->PrintfToUi(" MV\n");
	}
	else
	{
		m_pReportDevice->PrintfToUi(" MI\n");
	}

	const char* lpszName = bMV ? "MV" : "MI";


	double dTimeConsume = 0;
	char lpszTimeUnits[8] = { 0 };

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<%s>\n", lpszBaseIndent, lpszName);

	string strFirstIndent = lpszBaseIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();

	BOOL bAllPass = TRUE;
	int nRetVal = 0;
	vector<USHORT> vecChannel;

	do
	{
		///<Measure all channels same time
		for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL; ++usChannel)
		{
			vecChannel.push_back(usChannel);
		}
		nRetVal = DiagnosisChannel(lpszFirstIndent, vecChannel, bMV, "AllChannel");
		if (0 != nRetVal)
		{
			bAllPass = FALSE;
		}

		///<Measure all even channels one time
		vecChannel.clear();
		for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL; usChannel += 2)
		{
			vecChannel.push_back(usChannel);
		}
		nRetVal = DiagnosisChannel(lpszFirstIndent, vecChannel, bMV, "EvenChannel");
		if (0 != nRetVal)
		{
			bAllPass = FALSE;
		}
		///<Measure all odd channels one time
		vecChannel.clear();
		for (USHORT usChannel = 1; usChannel < DCM_CHANNELS_PER_CONTROL; usChannel += 2)
		{
			vecChannel.push_back(usChannel);
		}
		nRetVal = DiagnosisChannel(lpszFirstIndent, vecChannel, bMV, "OddChannel");
		if (0 != nRetVal)
		{
			bAllPass = FALSE;
		}

		///<Measure odd and even channel in different chip one time
		vecChannel.clear();
		for (USHORT usChannel = 0; usChannel < DCM_CHANNELS_PER_CONTROL; ++usChannel)
		{
			if (usChannel / 2 % 2 != usChannel % 2)
			{
				continue;
			}
			vecChannel.push_back(usChannel);
		}
		nRetVal = DiagnosisChannel(lpszFirstIndent, vecChannel, bMV, "OddEvenMix");
		if (0 != nRetVal)
		{
			bAllPass = FALSE;
		}

	} while (false);

	if (bAllPass)
	{
		nRetVal = 0;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszFirstIndent);
	}
	else
	{
		nRetVal = -1;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszFirstIndent);
	}

	dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszFirstIndent, dTimeConsume, lpszTimeUnits);

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</%s>\n", lpszBaseIndent, lpszName);

	ShowUIResult();
	return nRetVal;
}

int CDiagnosisPMU::DiagnosisChannel(const char* lpszBaseIndent, const std::vector<USHORT>& vecChannel, BOOL bMV, const char* lpszItemType)
{
	StartTimer();

	double dTimeConsume = 0;
	char lpszTimeUnits[8] = { 0 };
	
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<%s>\n", lpszBaseIndent, lpszItemType);
	int nRetVal = 0;
	string strFirstIndent = lpszBaseIndent + IndentChar();
	string strSecondIndent = strFirstIndent + IndentChar();
	string strThirdIndent = strSecondIndent + IndentChar();
	const char* lpszFirstIndent = strFirstIndent.c_str();
	const char* lpszSecondIndent = strSecondIndent.c_str();
	const char* lpszThirdIndent = strThirdIndent.c_str();

	BOOL bAllPass = TRUE;

	int nTestItemCount = 2;

	int nControllerCount = m_vecEnableController.size();
	CHardwareFunction* pHardware = nullptr;

	int nChannelCount = vecChannel.size();
	map<USHORT, double> mapExpectValue;
	auto iterExpectValue = mapExpectValue.begin();
	
	//double dVoltageStep = 7. / nChannelCount;
	double dBaseValue = bMV ? -1 : -1.8e-3;
	double dStep = bMV ? 7. / nChannelCount : 3.6e-3 / nChannelCount;
	PMU_MODE SetMode = bMV ? PMU_MODE::FVMV : PMU_MODE::FIMI;
	PMU_MODE MeasMode = bMV ? PMU_MODE::FIMV : PMU_MODE::FVMI;
	double dCriteria = bMV ? 0.2 : 100e-3;
	double dMultiple = bMV ? 1 : 1e3;
	double dSetSymbol = bMV ? 1 : -1;

	BYTE bySlotNo = 0;
	BYTE byBoardController = 0;
	double dSetValue = 0;
	vector<UINT> vecBindController;
	for (int nItemIndex = 0; nItemIndex < nTestItemCount;++nItemIndex)
	{
		///<Force controller
		Bind(nItemIndex, vecBindController);
		ClearBind();///<Can't use bind for the calibration data of each controller are not same
		for (auto ControllerID : vecBindController)
		{
			pHardware = GetHardware(ControllerID);
			USHORT usChannelIndex = 0;
			for (auto usChannel : vecChannel)
			{
				vector<USHORT> vecCurChannel;
				vecCurChannel.push_back(usChannel);
				dSetValue = dBaseValue + dStep * usChannelIndex++;
				pHardware->SetPMUMode(vecCurChannel, SetMode, PMU_IRANGE::IRANGE_2MA, dSetValue * dSetSymbol, 7.2, -2);

				iterExpectValue = mapExpectValue.find(usChannel);
				if (mapExpectValue.end() == iterExpectValue)
				{
					mapExpectValue.insert(make_pair(usChannel, 0));
					iterExpectValue = mapExpectValue.find(usChannel);
				}
				iterExpectValue->second = dSetValue * dMultiple;
			}
		}

		///<Measure controller
		UINT uControllerID = Bind(1 - nItemIndex, vecBindController);
		pHardware = GetHardware(uControllerID);
		pHardware->SetPMUMode(vecChannel, MeasMode, PMU_IRANGE::IRANGE_2MA, 0, 7.2, -2);
		pHardware->DelayMs(1);
		pHardware->PMUMeasure(vecChannel, 10, 10);
		ClearBind();

		for (auto uCurControllerID : m_vecEnableController)
		{
			bySlotNo = HDModule::Instance()->ID2Board(uCurControllerID, byBoardController);
			if (nItemIndex != byBoardController % 2)
			{
				///<The controller is force voltage
				continue;
			}

			StartTimer();
			BOOL bControllerPass = TRUE;
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<controller value='%d, slot value = %d'>\n", lpszFirstIndent, byBoardController, bySlotNo);
			pHardware = GetHardware(uCurControllerID);
			for (auto usChannel : vecChannel)
			{
				double dMeasResult = pHardware->GetPMUMeasureResult(usChannel, AVERAGE_RESULT) * dMultiple;
				iterExpectValue = mapExpectValue.find(usChannel);
				if (mapExpectValue.end() == iterExpectValue)
				{
					///<Not will happen
					continue;
				}

				if (dCriteria < fabs(dMeasResult - iterExpectValue->second))
				{
					bAllPass = FALSE;
					bControllerPass = FALSE;
					m_pReportDevice->PrintfToDataLog(IHDReportDevice::Error, "%s<data retValue='channel=%d, expect=%.3f, real=%.3f'/>\n",
						lpszSecondIndent, usChannel, iterExpectValue->second, dMeasResult);
				}
			}
			if (bControllerPass)
			{
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszSecondIndent);
			}
			else
			{
				SaveFailController(uCurControllerID);
				m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszSecondIndent);
			}
			dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszSecondIndent, dTimeConsume, lpszTimeUnits);

			m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</controller>\n", lpszFirstIndent);
		}
	}

	if (bAllPass)
	{
		nRetVal = 0;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='true'/>\n", lpszFirstIndent);
	}
	else
	{
		nRetVal = -1;
		m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<result value='false'/>\n", lpszFirstIndent);
	}

	dTimeConsume = StopTimer(lpszTimeUnits, sizeof(lpszTimeUnits));
	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s<Time value='%.3f%s'/>\n", lpszFirstIndent, dTimeConsume, lpszTimeUnits);

	m_pReportDevice->PrintfToDataLog(IHDReportDevice::Info, "%s</%s>\n", lpszBaseIndent, lpszItemType);
	return nRetVal;
}

void CDiagnosisPMU::ShowUIResult()
{
	BYTE bySlotNo = 0;
	BYTE byBoardController = 0;
	for (auto uControllerID : m_vecEnableController)
	{
		bySlotNo = HDModule::Instance()->ID2Board(uControllerID, byBoardController);
		if (m_setFailController.end() != m_setFailController.find(uControllerID))
		{
			m_pReportDevice->PrintfToUi(IHDReportDevice::Fail);
		}
		else
		{
			m_pReportDevice->PrintfToUi(IHDReportDevice::Pass);
		}
		m_pReportDevice->PrintfToUi("\t Slot %d, Controller %d\n", bySlotNo, byBoardController);
	}
	m_setFailController.clear();
}

inline void CDiagnosisPMU::SaveFailController(UINT uControllerID)
{
	if (m_setFailController.end() == m_setFailController.find(uControllerID))
	{
		m_setFailController.insert(uControllerID);
	}
}

UINT CDiagnosisPMU::Bind(int nEvenController, std::vector<UINT>& vecBindController)
{
	set<BYTE> setSlot;
	set<BYTE> setController;
	BYTE byTargetSlot = 0;
	UINT uTargetControllerID = 0;
	BYTE byCurSlot = 0;
	BYTE byController = 0;
	BOOL bAllController = FALSE;
	if (-1 == nEvenController)
	{
		bAllController = TRUE;
	}
	BYTE byRemainder = nEvenController ? 0 : 1;

	for (auto uControllerID : m_vecEnableController)
	{
		byCurSlot = HDModule::Instance()->ID2Board(uControllerID, byController);
		if (bAllController || byRemainder == byController % 2)
		{
			vecBindController.push_back(uControllerID);
			if (setController.end() == setController.find(byController))
			{
				setController.insert(byController);
			}
			if (setSlot.end() == setSlot.find(byCurSlot))
			{
				setSlot.insert(byCurSlot);
			}
			if (0 == byTargetSlot)
			{
				byTargetSlot = byCurSlot;
				uTargetControllerID = uControllerID;
			}
		}
	}
	CBindInfo::Instance()->Bind(setSlot, setController, byTargetSlot);
	return uTargetControllerID;
}

void CDiagnosisPMU::ClearBind()
{
	CBindInfo::Instance()->ClearBind();
}
