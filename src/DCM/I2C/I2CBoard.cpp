#include "I2CBoard.h"
#include "..\ChannelClassify.h"
#include <iterator>
using namespace std;

#ifdef RECORD_TIME
#include "..\Timer.h"
#endif // RECORD_TIME

CI2CBoard::CI2CBoard(BYTE bySlotNo, CDriverAlarm* pAlarm)
    : m_pAlarm(pAlarm)
    , m_bEnableCompareShield(TRUE)
{
	m_bySlotNo = bySlotNo;
	m_bBRAM = TRUE;
    m_byTimeset = TIMESET_COUNT - 1;
    m_pSite = nullptr;
    m_uBRAMStartLine = 0x7FFFFFFF;
 }

CI2CBoard::~CI2CBoard()
{
    DelteMemory(m_mapHardwareFunction);
    DelteMemory(m_mapPattern);
    DelteMemory(m_mapControllerData);
}

void CI2CBoard::SetSite(const CI2CSite& I2CSite)
{
    m_pSite = &I2CSite;
}

void CI2CBoard::SetPatternRAM(BOOL bBRAM)
{
	m_bBRAM = bBRAM;
}

int CI2CBoard::SetChannelPattern(USHORT usChannel, UINT uLineIndex, char cPatternSign, const char* lpszCMD, USHORT usCMDOperand, USHORT bCapture, BOOL bSwitch)
{
    if (DCM_MAX_CHANNELS_PER_BOARD < usChannel)
    {
        return -1;
    }
	BYTE byControllerIndex = usChannel / DCM_CHANNELS_PER_CONTROL;
	USHORT usChannelOffset = usChannel % DCM_CHANNELS_PER_CONTROL;
    CHardwareFunction* pHardware = GetHardware(byControllerIndex);
    if (nullptr == pHardware)
    {
        return -2;
    }
    auto iterPattern = m_mapPattern.find(byControllerIndex);
    if (m_mapPattern.end() == iterPattern)
    {
        CPattern* pPattern = new CPattern(*pHardware, nullptr);
        pPattern->SetDefaultPattern(CPattern:: PATTERN_SIGN::PAT_S);
        m_mapPattern.insert(make_pair(byControllerIndex, pPattern));
        iterPattern = m_mapPattern.find(byControllerIndex);
    }
    iterPattern->second->AddChannelPattern(usChannelOffset, m_bBRAM, uLineIndex, cPatternSign, m_byTimeset, lpszCMD, "", usCMDOperand, bCapture, bSwitch);
	
    return 0;
}

void CI2CBoard::Load()
{
#ifdef RECORD_TIME
	CTimer::Instance()->Start("CI2CBoard::Load");
#endif // RECORD_TIME

    if (0 != m_mapPattern.size())
    {
        for (auto& Pattern : m_mapPattern)
        {
            if (nullptr != Pattern.second)
			{
#ifdef RECORD_TIME
				CTimer::Instance()->Start("Load:%d", Pattern.first);
#endif // RECORD_TIME
                Pattern.second->Load();
                delete Pattern.second;
                Pattern.second = nullptr;

#ifdef RECORD_TIME
				CTimer::Instance()->Stop();
#endif // RECORD_TIME
            }
        }
    }
    m_mapPattern.clear();

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
#endif // RECORD_TIME
}

int CI2CBoard::SetRunParameter(UINT uStartLine, UINT uStopLine, BOOL bWithDRAM, UINT uSDARMStartLine)
{
    if (DCM_BRAM_PATTERN_LINE_COUNT < uStartLine || DCM_BRAM_PATTERN_LINE_COUNT < uStopLine)
    {
        return -1;
    }
    if (bWithDRAM && DCM_DRAM_PATTERN_LINE_COUNT < uSDARMStartLine)
    {
        return -2;
    }
    if (bWithDRAM)
	{
		m_uBRAMStartLine = 0x7FFFFFFF;
    }
    else
	{
        m_uBRAMStartLine = uStartLine;
    }
	m_bBRAM = !bWithDRAM;
    vector<USHORT> vecChannel;
    m_pSite->GetBoardChannel(m_bySlotNo, vecChannel);
    CBoardChannelClassify Classify;
    Classify.SetChannel(vecChannel);
    set<USHORT> setChannel;
    int nRetVal = 0;
    BOOL bNoBoard = TRUE;
    m_vecRunController.clear();
    for (auto& Controller : m_mapHardwareFunction)
    {
        Classify.GetChannel(Controller.first, vecChannel);
        if (0 == vecChannel.size())
        {
            continue;
        }
        bNoBoard = FALSE;
        m_vecRunController.push_back(Controller.second);
        if (m_bEnableCompareShield)
        {
            setChannel.clear();
            copy(vecChannel.begin(), vecChannel.end(), inserter(setChannel, setChannel.begin()));
            Controller.second->SetComparedChannel(setChannel);
        }
        Controller.second->SetChannelMCUMode(vecChannel);
        nRetVal = Controller.second->SetRunParameter(uStartLine, uStopLine, bWithDRAM, uSDARMStartLine);
        if (0 != nRetVal)
        {
            switch (nRetVal)
            {
            case -1:
                ///<The start line number in BRAM is over range
                nRetVal = -1;
                break;
            case -2:
                ///<The stop line number in BRAM is over range
                nRetVal = -2;
                break;
            case -3:
                ///<The stop line number is before start line number
                nRetVal = -3;
                break;
            case -4:
                ///<The start line number in DRAM is over range
                nRetVal = -4;
                break;
            default:
                break;
            }
            break;
        }
    }
    if (0 == nRetVal && bNoBoard)
    {
        nRetVal = -5;
    }
    return nRetVal;
}

void CI2CBoard::EnableRun(BOOL bEnable)
{
    for (auto& Controller : m_vecRunController)
    {
        Controller->EnableStart(bEnable);
    }
}

void CI2CBoard::Run()
{
    if (0 == m_vecRunController.size())
    {
        return;
    }
    m_vecRunController[0]->SynRun();
}

int CI2CBoard::SetPeriod(double dPeriod, BOOL bOnlyValidSite)
{
    if (nullptr == m_pSite)
    {
        return -1;
    }
    CHardwareFunction* pHardware = nullptr;
    set<BYTE> setController;
    vector<USHORT> vecChannel;
    m_pSite->GetBoardChannel(m_bySlotNo, vecChannel, bOnlyValidSite);
    CBoardChannelClassify Classify;
    Classify.GetController(vecChannel, setController);
	int nRetVal = 0;
	BOOL bNoBoard = TRUE;
    for (auto Controller : setController)
    {
        pHardware = GetHardware(Controller);
        if (nullptr == pHardware)
        {
            continue;
        }
        bNoBoard = FALSE;
        nRetVal = pHardware->SetPeriod(m_byTimeset, dPeriod);
        if (0 != nRetVal)
        {
            return -3;
        }
    }
    if (bNoBoard)
    {
        return -2;
    }
    return 0;
}

int CI2CBoard::SetEdge(const std::vector<USHORT>& vecChannel, const double* pdEdge, WAVE_FORMAT WaveFormat, IO_FORMAT IOFormat)
{
#ifdef RECORD_TIME
    CTimer::Instance()->Start("Board SetEdge");
#endif // RECORD_TIME

    CHardwareFunction* pHardware = GetHardware(0);
    if (nullptr == pHardware || !pHardware->IsBoardExisted())
	{
		return -1;
	}
    if (nullptr == pdEdge)
    {
        return -2;
    }
    int nRetVal = 0;

#ifdef RECORD_TIME
   CTimer::Instance()->Start("SetChannel %d", vecChannel.size());
#endif // RECORD_TIME

    CBoardChannelClassify m_ChannelClassifiy;///<The channel classify
    m_ChannelClassifiy.SetChannel(vecChannel);

#ifdef RECORD_TIME
     CTimer::Instance()->Stop();
     CTimer::Instance()->Start("SetControllerEdge");
#endif // RECORD_TIME

    vector<USHORT> vecControllerChannel;
	for (BYTE byControllerIndex = 0; byControllerIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++byControllerIndex)
	{
#ifdef RECORD_TIME
        CTimer::Instance()->Start("GetChannel");
#endif // RECORD_TIME

		m_ChannelClassifiy.GetChannel(byControllerIndex, vecControllerChannel);
        if (0 == vecControllerChannel.size())
		{
#ifdef RECORD_TIME
			CTimer::Instance()->Stop();
#endif // RECORD_TIME

            continue;
		}
#ifdef RECORD_TIME
 		CTimer::Instance()->Stop();
 		CTimer::Instance()->Start("CheckController %d", byControllerIndex);
#endif // RECORD_TIME
        
        CHardwareFunction* pHardware = GetHardware(byControllerIndex);
        if(nullptr == pHardware)
		{
#ifdef RECORD_TIME
			CTimer::Instance()->Stop();
#endif // RECORD_TIME
			continue;
		}

#ifdef RECORD_TIME
 		CTimer::Instance()->Stop();
        CTimer::Instance()->Start("Hardware Edge");
#endif // RECORD_TIME

		nRetVal = pHardware->SetEdge(vecControllerChannel, m_byTimeset, pdEdge, WaveFormat, IOFormat, COMPARE_MODE::EDGE);
        if (0 != nRetVal)
        {
            switch (nRetVal)
            {
            case -1:
                ///<Timeset over range, not happened
                break;
            case -2:
                ///<The format is error
                nRetVal = -3;
                break;
            case -3:
                ///<The point is nullptr, checked
                break;
            case -4:
                ///<The value is error
                nRetVal = -4;
                break;
            default:
                break;
            }
            break;
		}

#ifdef RECORD_TIME
		CTimer::Instance()->Stop();
#endif // RECORD_TIME
	}

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
 	CTimer::Instance()->Stop();
#endif // RECORD_TIME
    return nRetVal;
}

BOOL CI2CBoard::IsBoardExist(BOOL bRefresh)
{
    BOOL bClassExisted = TRUE;
    CHardwareFunction* pHardware = nullptr;
    auto iterBoard = m_mapHardwareFunction.begin();
    if (m_mapHardwareFunction.end() == iterBoard || nullptr == iterBoard->second)
    {
        pHardware = new CHardwareFunction(m_bySlotNo, m_pAlarm);
        bClassExisted = FALSE;
    }
    else
    {
        pHardware = iterBoard->second;
    }
    BOOL bBoardExisted = TRUE;
    if (bRefresh || !bClassExisted)
	{
		bBoardExisted = pHardware->IsBoardExisted();
    }
    if (!bClassExisted)
    {
        delete pHardware;
        pHardware = nullptr;
    }
    return bBoardExisted;
}

int CI2CBoard::WaitStop()
{
    for (auto& ControllerData : m_mapControllerData)
    {
        ControllerData.second->SetDataValid(FALSE);
    }

    BOOL bAllControllerNotRan = TRUE;
    const UINT uTotalCheckTimes = static_cast<unsigned int>(16e6);
    UINT uCheckTimes = 0;
    int nRetVal = 0;

    for (auto& Controller : m_vecRunController)
    {
		uCheckTimes = 0;
		Controller->EnableStart(FALSE);
        do
        {
            if (0 != uCheckTimes++)
            {
                Controller->DelayUs(10);
            }
            nRetVal = Controller->GetRunningStatus();

        } while (0 == nRetVal && uCheckTimes < uTotalCheckTimes);
        if (-1 != nRetVal)
        {
            bAllControllerNotRan = FALSE;
        }
        if (uTotalCheckTimes <= uCheckTimes)
        {
            break;
        }
    }
    if (bAllControllerNotRan)
    {
        return -2;
    }
    if (uTotalCheckTimes <= uCheckTimes)
    {
        return -3;
    }
    return 0;
}

int CI2CBoard::IsChannelExist(USHORT usChannel, BOOL bRefresh)
{
    if (DCM_MAX_CHANNELS_PER_BOARD <= usChannel)
    {
        return 0;
    }

    int nControllerIndex = usChannel / DCM_CHANNELS_PER_CONTROL;

#ifdef RECORD_TIME
    CTimer::Instance()->Start("GetController %d", nControllerIndex);
#endif // RECORD_TIME

    CHardwareFunction* pHardware = GetHardware(nControllerIndex);

    if (nullptr != pHardware)
    {
#ifdef RECORD_TIME
        CTimer::Instance()->Stop();
#endif // RECORD_TIME
        if (bRefresh)
        {
             return pHardware->IsControllerExist() ? 1 : 0;
        }
        return 1;
    }
    else
    {
        return 0;
    }

#ifdef RECORD_TIME
	CTimer::Instance()->Stop();
#endif // RECORD_TIME

    return 0;
}

int CI2CBoard::SetPinLevel( double dVIH, double dVIL, double dVOH, double dVOL, BYTE byChannelType)
{
    vector<BYTE> vecControllerIndex;
    if (nullptr == m_pSite)
    {
        return -1;
    }
    vector<USHORT> vecChannel;
    m_pSite->GetBoardChannel(m_bySlotNo, vecChannel, TRUE, byChannelType);

	CBoardChannelClassify Classify;
    Classify.SetChannel(vecChannel);
    int nRetVal = 0;
    for (BYTE byControllerIndex = 0; byControllerIndex < DCM_MAX_CONTROLLERS_PRE_BOARD;++byControllerIndex)
	{
		Classify.GetChannel(byControllerIndex, vecChannel);
        if (0 == vecChannel.size())
        {
            continue;
        }
		CHardwareFunction* pHardware = GetHardware(byControllerIndex);
        if (nullptr == pHardware)
        {
            continue;
        }
		nRetVal = pHardware->SetPinLevel(vecChannel, dVIH, dVIL, (dVIH + dVIL) / 2, dVOH, dVOL);
		if (0 != nRetVal)
		{
			return -2;
		}
    }
    return 0;
}

int CI2CBoard::GetResult(USHORT usChannel, vector<int>& vecResult)
{
    if (DCM_MAX_CHANNELS_PER_BOARD <= usChannel)
    {
        return -1;
    }
	BYTE byControllerIndex = usChannel / DCM_CHANNELS_PER_CONTROL;
	CHardwareFunction* pHardware = GetHardware(byControllerIndex);
	if (nullptr == pHardware)
	{
		return -2;
	}
    auto iterController = m_mapControllerData.find(byControllerIndex);
    if (m_mapControllerData.end() == iterController)
    {
        CControllerData* pControllerData = new CControllerData(byControllerIndex);
        m_mapControllerData.insert(make_pair(byControllerIndex, pControllerData));
        iterController = m_mapControllerData.find(byControllerIndex);
    }

    if (!iterController->second->IsDataValid())
    {
        CHardwareFunction* pHardware = GetHardware(iterController->first);
       
		vector<CHardwareFunction::DATA_RESULT> vecBRAMFailLineNo;
        vector<CHardwareFunction::DATA_RESULT> vecDRAMFailLineNo;
        pHardware->GetFailData(vecBRAMFailLineNo, vecDRAMFailLineNo);

        int nOffset = 0;
        auto pvecFailLineData = &vecDRAMFailLineNo;
        if (m_bBRAM)
        {
			pvecFailLineData = &vecBRAMFailLineNo;
			nOffset = m_uBRAMStartLine;
        }
        iterController->second->SetFailData(*pvecFailLineData, nOffset);
    }
    iterController->second->GetChannelFailLine(usChannel % DCM_CHANNELS_PER_CONTROL, vecResult);
    return 0;
}

int CI2CBoard::SetDynamicLoad(BYTE byChannelType, BOOL bEnable, double dIOH, double dIOL, double dVTVoltValue, double dClampHighVoltValue, double dClampLowVoltValue)
{
	vector<BYTE> vecControllerIndex;
	if (nullptr == m_pSite)
	{
		return -1;
	}
	vector<USHORT> vecChannel;
	m_pSite->GetBoardChannel(m_bySlotNo, vecChannel, TRUE, byChannelType);

	CBoardChannelClassify Classify;
	Classify.SetChannel(vecChannel);
	int nRetVal = 0;
	for (BYTE byControllerIndex = 0; byControllerIndex < DCM_MAX_CONTROLLERS_PRE_BOARD; ++byControllerIndex)
	{
		Classify.GetChannel(byControllerIndex, vecChannel);
		if (0 == vecChannel.size())
		{
			continue;
		}
		CHardwareFunction* pHardware = GetHardware(byControllerIndex);
		if (nullptr == pHardware)
		{
			continue;
		}
		nRetVal = pHardware->SetDynamicLoad(vecChannel, bEnable, dIOH,dIOL, dVTVoltValue, dClampHighVoltValue, dClampLowVoltValue);
		if (0 != nRetVal)
		{
            switch (nRetVal)
            {
            case -1:
                ///<Channel over range, not will happen
                break;
            case -2:
                ///<Current over range
                nRetVal = -2;
                break;
            case -3:
                ///<VT over range
                nRetVal = -3;
                break;
            case -4:
                ///<Clamp voltage over range
                nRetVal = -4;
                break;
            default:
                break;
            }
			return nRetVal;
		}
	}
	return 0;
}

void CI2CBoard::EnableCopmareShield(BOOL bEnable)
{
	m_bEnableCompareShield = bEnable;
}

inline CHardwareFunction* CI2CBoard::GetHardware(BYTE byController)
{
    if (DCM_MAX_CONTROLLERS_PRE_BOARD <= byController)
    {
        return nullptr;
    }
    auto iterController = m_mapHardwareFunction.find(byController);
    if (m_mapHardwareFunction.end() == iterController || nullptr == iterController->second)
    {
		CHardwareFunction* pHardware = new CHardwareFunction(m_bySlotNo, m_pAlarm);
		pHardware->SetControllerIndex(byController);
        if (m_mapHardwareFunction.end() != iterController)
        {
            m_mapHardwareFunction.erase(iterController);
        }
        else if (!pHardware->IsControllerExist())
        {
            delete pHardware;
            pHardware = nullptr;
            return nullptr;
        }
        m_mapHardwareFunction.insert(make_pair(byController, pHardware));
        iterController = m_mapHardwareFunction.find(byController);
    }
    return iterController->second;
}

template<typename Key, typename Value>
inline void CI2CBoard::DelteMemory(std::map<Key, Value>& mapParam)
{
    if (0 != mapParam.size())
    {
        for (auto& Param : mapParam)
        {
            if (nullptr != Param.second)
            {
                delete Param.second;
                Param.second = nullptr;
            }
        }
        mapParam.clear();
    }
}

CControllerData::CControllerData(BYTE byControllerIndex)
{
    m_byIndex = byControllerIndex;
    m_bDataValid = FALSE;
}

void CControllerData::SetFailData(const std::vector<CHardwareFunction::DATA_RESULT>& vecFailData, int nOffset)
{
    m_vecFailData.clear();

    CHardwareFunction::DATA_RESULT DataResult;
    for (auto& Fail : vecFailData)
    {
        DataResult = Fail;
        DataResult.m_nLineNo -= nOffset;
        m_vecFailData.push_back(DataResult);
    }
    m_bDataValid = TRUE;
}

int CControllerData::GetChannelFailLine(USHORT usChannel, std::vector<int>& vecFail)
{
    vecFail.clear();
    if (DCM_CHANNELS_PER_CONTROL <= usChannel)
    {
        return -1;
    }
    USHORT usChannelBit = 1 << usChannel;

    for (auto& Fail : m_vecFailData)
    {
        if (Fail.m_usData & usChannelBit)
        {
            vecFail.push_back(Fail.m_nLineNo);
        }
    }
    return 0;
}

void CControllerData::SetDataValid(BOOL bValid)
{
    m_bDataValid = bValid;
    if (!m_bDataValid)
    {
        m_vecFailData.clear();
    }
}

BOOL CControllerData::IsDataValid()
{
    return m_bDataValid;
}
