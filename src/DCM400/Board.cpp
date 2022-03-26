#include "pch.h"
#include "Board.h"
#include "DCM400HardwareInfo.h"
#include "STSMD5.h"
#include "FlashInfo.h"
using namespace std;

CBoard::CBoard(BYTE bySlotNo, CDriverAlarm* pAlarm)
	: m_pHardwareFunction(nullptr)
	, m_pAlarm(pAlarm)
{
	m_usChannelCount = 0;
	m_bExisted = FALSE;
	m_bySlotNo = bySlotNo;

	m_pHardwareFunction = new CHardwareFunction(m_bySlotNo, m_pAlarm);
	m_bExisted = m_pHardwareFunction->IsBoardExisted();
	if (m_bExisted)
	{
		GetChannelCount(TRUE);
	}
}

CBoard::~CBoard()
{
	if (0 == m_mapController.size() && nullptr != m_pHardwareFunction)
	{
		delete m_pHardwareFunction;
		m_pHardwareFunction = nullptr;
	}
	m_pHardwareFunction = nullptr;
	for (auto& Control : m_mapController)
	{
		if (nullptr != Control.second)
		{
			delete Control.second;
			Control.second = nullptr;
		}
	}
	m_mapController.clear();
}

// USHORT CBoard::GetFPGARevision(BYTE byControllerIndex)
// {
// 	if (DCM400_MAX_CONTROLLERS_PRE_BOARD <= byControllerIndex)
// 	{
// 		return 0xFFFF;
// 	}
// 	auto iterController = m_mapController.find(byControllerIndex);
// 	if (m_mapController.end() == iterController || nullptr == iterController->second)
// 	{
// 		return 0xFFFF;
// 	}
// 
// 	return iterController->second->GetFPGARevision();
// }
// 
// USHORT CBoard::GetFPGARevision()
// {
// 	CHardwareFunction HardwareFunction(m_bySlotNo, m_pAlarm);
// 	return HardwareFunction.GetBoardLogicRevision();
// }
// 
BOOL CBoard::IsExisted() const
{
	return m_bExisted;
}

int CBoard::SetPeriodSeries(const std::vector<USHORT>& vecChannel, USHORT usSeriesIndex, double dPeriod)
{
	if (TIME_SERIES_MAX_COUNT <= usSeriesIndex)
	{
		return -1;
	}
	int nRetVal = 0;
	BOOL bNoBoard = TRUE;

	set<BYTE> setController;
	m_ChannelClassifiy.GetController(vecChannel, setController);
	for (auto& Controller : setController)
	{
		auto iterController = m_mapController.find(Controller);
		if (m_mapController.end() == iterController)
		{
			continue;
		}
		bNoBoard = FALSE;
		nRetVal = iterController->second->SetPeriodSeries(usSeriesIndex, dPeriod);
		if (0 != nRetVal)
		{
			switch (nRetVal)
			{
			case -1:
				///<The series index is over range
				nRetVal = -1;
				break;
			case -2:
				///<The period is over range
				nRetVal = -2;
				break;
			default:
				break;
			}
			break;
		}
	}
	if (bNoBoard)
	{
		return -3;
	}
	return nRetVal;
}

int CBoard::SetEdgeSeries(const std::vector<USHORT>& vecChannel, USHORT usSeriesIndex, const double* pdEdge)
{
	if (TIME_SERIES_MAX_COUNT <= usSeriesIndex)
	{
		return -1;
	}
	int nRetVal = 0;
	BOOL bNoBoard = TRUE;
	vector<USHORT> vecControllerChannel;
	m_ChannelClassifiy.SetChannel(vecChannel);
	for (auto& Controller : m_mapController)
	{
		m_ChannelClassifiy.GetChannel(Controller.first, vecControllerChannel);
		if (0 == vecControllerChannel.size())
		{
			continue;
		}
		bNoBoard = FALSE;
		nRetVal = Controller.second->SetEdgeSeries(vecChannel, usSeriesIndex,  pdEdge);
		if (0 != nRetVal)
		{
			switch (nRetVal)
			{
			case -1:
				///<The series is over range
				nRetVal = -1;
				break;
			case -2:
				///<The the point of edge is nullptr
				nRetVal = -2;
				break;
			default:
				break;
			}
		}
	}
	return 0;
}

int CBoard::SetFormatSeries(const std::vector<USHORT>& vecChannel, USHORT usSeriesIndex, const WAVE_FORMAT& WaveFormat, const IO_FORMAT& IOFormat, const COMPARE_MODE& CompareMode)
{
	if (TIME_SERIES_MAX_COUNT <= usSeriesIndex)
	{
		return -1;
	}
	int nRetVal = 0;
	BOOL bNoBoard = TRUE;
	vector<USHORT> vecControllerChannel;
	m_ChannelClassifiy.SetChannel(vecChannel);
	for (auto& Controller : m_mapController)
	{
		m_ChannelClassifiy.GetChannel(Controller.first, vecControllerChannel);
		if (0 == vecControllerChannel.size())
		{
			continue;
		}
		bNoBoard = FALSE;
		nRetVal = Controller.second->SetFormatSeries(vecChannel, usSeriesIndex, WaveFormat, IOFormat, CompareMode);
		if (0 != nRetVal)
		{
			switch (nRetVal)
			{
			case -1:
				///<The series is over range
				nRetVal = -1;
				break;
			case -2:
				///<The the point of edge is nullptr
				nRetVal = -2;
				break;
			default:
				break;
			}
		}
	}
	return 0;
}

int CBoard::SetTimeSet(const std::vector<USHORT>& vecChannel, USHORT usTimesetIndex, BYTE byPeriodSeries, const BYTE* pbyEdgeSeries, BYTE byFomratSeries)
{
	int nRetVal = 0;
	BOOL bNoBoard = TRUE;
	vector<USHORT> vecControllerChannel;
	m_ChannelClassifiy.SetChannel(vecChannel);
	for (auto& Controller : m_mapController)
	{
		m_ChannelClassifiy.GetChannel(Controller.first, vecControllerChannel);
		if (0 == vecControllerChannel.size())
		{
			continue;
		}
		bNoBoard = FALSE;
		nRetVal = Controller.second->SetTimeSet(vecChannel, usTimesetIndex, byPeriodSeries, pbyEdgeSeries, byFomratSeries);
		if (0 != nRetVal)
		{
			switch (nRetVal)
			{
			case -1:
				///<
			default:
				break;
			}
		}
	}
	return 0;
}

int CBoard::SetVector(USHORT usChannel, UINT uPatternLine, char cPattern, const CPatternCMD& PatternCMD)
{
	if (DCM400_MAX_CHANNELS_PER_BOARD <= usChannel)
	{
		return -1;
	}

	auto iterController = m_mapController.find(usChannel / DCM400_CHANNELS_PER_CONTROL);
	if (m_mapController.end() == iterController || nullptr == iterController->second)
	{
		return -2;
	}
	int nRetVal = iterController->second->AddChannelPattern(usChannel % DCM400_CHANNELS_PER_CONTROL, uPatternLine, cPattern, PatternCMD);
	if (0 != nRetVal)
	{
		m_setPatternController.insert(iterController->first);
		switch (nRetVal)
		{
		case -1:
			///<Channel over range, not will happpen
			nRetVal = -1;
			break;
		case -2:
			///<The pattern line number is over range
			nRetVal = -3;
			break;
		case -3:
			///<Allocate momory fail
			nRetVal = -4;
			break;
		default:
			break;
		}
	}
	return nRetVal;
}

int CBoard::LoadVector()
{
	for (auto& Controller : m_mapController)
	{
		Controller.second->LoadVector();
	}
	return 0;
}

void CBoard::SetVectorValid(BOOL bValid)
{
	for (auto Controller : m_setPatternController)
	{
		auto iterController = m_mapController.find(Controller);
		if (m_mapController.end() == iterController || nullptr == iterController->second)
		{
			continue;
		}
		iterController->second->SetVectorValid(bValid);
	}
}

BOOL CBoard::IsVectorValid(const std::vector<USHORT>& vecChannel)
{
	set<BYTE> setController;
	m_ChannelClassifiy.GetController(vecChannel, setController);
	for (auto& Controller : setController)
	{
		auto iterController = m_mapController.find(Controller);
		if (m_mapController.end() == iterController || nullptr == iterController->second)
		{
			continue;
		}
		if (!iterController->second->IsVectorValid())
		{
			return FALSE;
		}
	}
	return TRUE;
}

// 
// int CBoard::GetControllerCount() const
// {
// 	return m_mapController.size();
// }
// 
// void CBoard::GetExistController(std::vector<BYTE>& vecController) const
// {
// 	vecController.clear();
// 	for (auto& Controller : m_mapController)
// 	{
// 		vecController.push_back(Controller.first);
// 	}
// }
// 
// int CBoard::IsChannelExisted(USHORT usChannel)
// {
// 	if (DCM400_MAX_CHANNELS_PER_BOARD <= usChannel)
// 	{
// 		return -1;
// 	}
// 	if (m_mapController.end() != m_mapController.find(usChannel / DCM400_CHANNELS_PER_CONTROL))
// 	{
// 		return 1;
// 	}
// 	return 0;
// }
// 
// int CBoard::SetPeriod(BYTE byTimesetIndex, double dPeriod)
// {
// 	int nRetVal = 0;
// 	for (auto& Controller : m_mapController)
// 	{
// 		nRetVal = Controller.second->SetPeriod(byTimesetIndex, dPeriod);
// 		if (0 != nRetVal)
// 		{
// 			//The period is over range
// 			nRetVal = -1;
// 		}
// 	}
// 	return nRetVal;
// }
// 
// int CBoard::SetPeriod(const std::vector<USHORT>& vecChannel, BYTE byTimesetIndex, double dPeriod)
// {
// 	set<BYTE> setController;
// 	m_ChannelClassifiy.GetController(vecChannel, setController);
// 	int nRetVal = 0;
// 	for (auto& Controller : m_mapController)
// 	{
// 		if (setController.end() == setController.find(Controller.first))
// 		{
// 			continue;
// 		}
// 		nRetVal = Controller.second->SetPeriod(byTimesetIndex, dPeriod);
// 		if (0 != nRetVal)
// 		{
// 			//The period is over range
// 			nRetVal = -1;
// 		}
// 	}
// 	return nRetVal;
// }
// 
// double CBoard::GetPeriod(BYTE byControllerIndex, BYTE byTimesetIndex)
// {
// 	if (DCM400_MAX_CONTROLLERS_PRE_BOARD <= byControllerIndex)
// 	{
// 		return -1;
// 	}
// 	double dPeriod = 0;
// 	BOOL bNoBoard = TRUE;
// 	auto iterController = m_mapController.find(byControllerIndex);
// 	if (m_mapController.end() == iterController || nullptr == iterController->second)
// 	{
// 		return -2;
// 	}
// 	dPeriod = iterController->second->GetPeriod(byTimesetIndex);
// 	if (0 > dPeriod)
// 	{
// 		dPeriod = -3;
// 	}
// 	return dPeriod;
// }
// 
// int CBoard::SetEdge(std::vector<USHORT>& vecChannel, BYTE byTimeset, double* pdEdge, WAVE_FORMAT WaveFormat, IO_FORMAT IOFormat, COMPARE_MODE CompareMode)
// {
// 	int nRetVal = 0;
// 	BOOL bNoBoard = TRUE;
// 	m_ChannelClassifiy.SetChannel(vecChannel);
// 	vector<USHORT> vecControllerChannel;
// 
// 	for (auto& Controller : m_mapController)
// 	{
// 		if (nullptr == Controller.second)
// 		{
// 			continue;
// 		}
// 		m_ChannelClassifiy.GetChannel(Controller.first, vecControllerChannel);
// 		if (0 == vecControllerChannel.size())
// 		{
// 			continue;
// 		}
// 		bNoBoard = FALSE;
// 		nRetVal = Controller.second->SetEdge(vecControllerChannel, byTimeset, pdEdge, WaveFormat, IOFormat, CompareMode);
// 		if (0 != nRetVal)
// 		{
// 			switch (nRetVal)
// 			{
// 			case -1:
// 				///<The timeset is over range
// 				nRetVal = -1;
// 				break;
// 			case -2:
// 				///<Wave format error
// 				nRetVal = -2;
// 				break;
// 			case -3:
// 				///<The point of edge value is nullptr
// 				nRetVal = -3;
// 			case -4:
// 				///<The value is error
// 				nRetVal = -4;
// 				break;
// 			default:
// 				break;
// 			}
// 			break;
// 		}
// 	}
// 	if (bNoBoard)
// 	{
// 		nRetVal = -5;
// 	}
// 	return nRetVal;
// }
// 
// int CBoard::GetEdge(USHORT usChannel, BYTE byTimesetIndex, double* pdEdge, WAVE_FORMAT& WaveFormat, IO_FORMAT& IOFormat, COMPARE_MODE& CompareMode)
// {
// 	if (DCM400_MAX_CHANNELS_PER_BOARD <= usChannel)
// 	{
// 		return -1;
// 	}
// 	auto iterController = m_mapController.find(usChannel / DCM400_CHANNELS_PER_CONTROL);
// 	if (m_mapController.end() == iterController)
// 	{
// 		//The controller is not existed;
// 		return -2;
// 	}
// 	int nRetVal = iterController->second->GetEdge(usChannel % DCM400_CHANNELS_PER_CONTROL, byTimesetIndex, pdEdge, WaveFormat, IOFormat, CompareMode);
// 	if (0 != nRetVal)
// 	{
// 		switch (nRetVal)
// 		{
// 		case -1:
// 			///<Channel number is over range, not will happen
// 			break;
// 		case -2:
// 			///<The timeset index is over range
// 			nRetVal = -3;
// 			break;
// 		case -3:
// 			///<The point of the parameter is nullptr
// 			nRetVal = -4;
// 			break;
// 		default:
// 			break;
// 		}
// 	}
// 	return nRetVal;
// }
// 
// void CBoard::GetSN(std::string &strSN)
// {
// 	STS_HARDINFO HardInfo;
// 	GetHardInfo(&HardInfo, 1);
// 	strSN = HardInfo.moduleInfo.moduleSN;
// }
// 
// int CBoard::SetVector(USHORT usChannel, BOOL bBRAM, UINT uPatternLine, char cPatternSign, BYTE byTimeset, const char* lpszCMD, const char* lpszParallelCMD,
// 	USHORT usOperand, BOOL bCapture, BOOL bSwitch)
// {
// 	if (DCM400_MAX_CHANNELS_PER_BOARD <= usChannel)
// 	{
// 		return -1;
// 	}
// 	BYTE byControllerIndex = usChannel / DCM400_CHANNELS_PER_CONTROL;
// 	if (DCM400_MAX_CONTROLLERS_PRE_BOARD <= byControllerIndex)
// 	{
// 		return -2;
// 	}
// 	auto iterController = m_mapController.find(usChannel / DCM400_CHANNELS_PER_CONTROL);
// 	if (m_mapController.end() == iterController)
// 	{
// 		//The controller is not existed;
// 		return -3;
// 	}
// 	int nRetVal = iterController->second->SetVector(usChannel % DCM400_CHANNELS_PER_CONTROL, bBRAM, uPatternLine, cPatternSign, byTimeset, lpszCMD, lpszParallelCMD, usOperand, bCapture, bSwitch);
// 	if (0 != nRetVal)
// 	{
// 		switch (nRetVal)
// 		{
// 		case -2:
// 			//The pattern line is over range
// 			nRetVal = -4;
// 			break;
// 		case -3:
// 			///<The pattern sign is not supported
// 			nRetVal = -5;
// 			break;
// 		case -4:
// 			///<The timeset is over range
// 			nRetVal = -6;
// 			break;
// 		case -5:
// 			///<The point of command is nullptr
// 			nRetVal = -7;
// 			break;
// 		case -6:
// 			///<The command is not supported
// 			nRetVal = -8;
// 			break;
// 		case -7:
// 			///<The operand is over range
// 			nRetVal = -9;
// 			break;
// 		default:
// 			break;
// 		}
// 	}
// 	return nRetVal;
// }
// 
// int CBoard::LoadVector()
// {
// 	int nRetVal = 0;
// 	for (auto& Controller : m_mapController)
// 	{
// 		if (nullptr != Controller.second)
// 		{
// 			nRetVal = Controller.second->LoadVector();
// 			if (0 != nRetVal)
// 			{
// 				return -1;
// 			}
// 		}
// 	}
// 	return 0;
// }
// 
// void CBoard::SetVectorValid(BOOL bValid)
// {
// 	if (nullptr == m_pHardwareFunction)
// 	{
// 		return;
// 	}
// 	m_pHardwareFunction->SetVectorValid(bValid);
// }
// 
// BOOL CBoard::IsVectorValid()
// {
// 	if (nullptr == m_pHardwareFunction)
// 	{
// 		return FALSE;
// 	}
// 
// 	return m_pHardwareFunction->IsVectorValid();
// }
// 
// int CBoard::SetOperand(const std::vector<USHORT>& vecChannel, UINT uBRAMLineNo, USHORT usOperand, BOOL bCheckRange)
// {
// 	int nRetVal = 0;
// 	BOOL bNoBoard = TRUE;
// 	set<BYTE> setControllerIndex;
// 	m_ChannelClassifiy.GetController(vecChannel, setControllerIndex);
// 	for (auto Controller : setControllerIndex)
// 	{
// 		auto iterController = m_mapController.find(Controller);
// 		if (m_mapController.end() == iterController || nullptr == iterController->second)
// 		{
// 			continue;
// 		}
// 		bNoBoard = FALSE;
// 		nRetVal = iterController->second->SetOperand(uBRAMLineNo, usOperand, bCheckRange);
// 		if (0 != nRetVal)
// 		{
// 			switch (nRetVal)
// 			{
// 			case -1:
// 				///<The line number is over range
// 				nRetVal = -1;
// 				break;
// 			case -2:
// 				///<The operand is over range
// 				nRetVal = -2;
// 				break;
// 			default:
// 				break;
// 			}
// 			break;
// 		}
// 	}
// 	
// 	if (bNoBoard)
// 	{
// 		nRetVal = -3;
// 	}
// 	return nRetVal;
// }
// 
// int CBoard::SetInstruction(const std::vector<USHORT>& vecChannel, UINT uBRAMLineNo, const char* lpszInstruction, USHORT usOperand)
// {
// 	int nRetVal = 0;
// 	BOOL bNoBoard = TRUE;
// 	set<BYTE> setControllerIndex;
// 	m_ChannelClassifiy.GetController(vecChannel, setControllerIndex);
// 	for (auto Controller : setControllerIndex)
// 	{
// 		auto iterController = m_mapController.find(Controller);
// 		if (m_mapController.end() == iterController || nullptr == iterController->second)
// 		{
// 			continue;
// 		}
// 		bNoBoard = FALSE;
// 		nRetVal = iterController->second->SetInstruction(uBRAMLineNo, lpszInstruction, usOperand);
// 		if (0 != nRetVal)
// 		{
// 			switch (nRetVal)
// 			{
// 			case -1:
// 				///<The line number is over range
// 				nRetVal = -1;
// 				break;
// 			case -2:
// 				///<The instruction is nullptr
// 				nRetVal = -2;
// 				break;
// 			case -3:
// 				///<The instruction is not supported
// 				nRetVal = -3;
// 				break;
// 			case -4:
// 				///<The operand is over range
// 				nRetVal = -4;
// 				break;
// 			default:
// 				break;
// 			}
// 			break;
// 		}
// 	}
// 
// 	if (bNoBoard)
// 	{
// 		nRetVal = -5;
// 	}
// 	return nRetVal;
// }
// 
// int CBoard::SetSaveSelectFail(const std::vector<USHORT>& vecChannel, UINT uRAMLineNo, BOOL bStartSave, BOOL bBRAM, BOOL bDelete /*= FALSE*/)
// {
// 	int nRetVal = 0;
// 	BOOL bNoBoard = TRUE;
// 	set<BYTE> setControllerIndex;
// 	m_ChannelClassifiy.GetController(vecChannel, setControllerIndex);
// 	for (auto Controller : setControllerIndex)
// 	{
// 		auto iterController = m_mapController.find(Controller);
// 		if (m_mapController.end() == iterController || nullptr == iterController->second)
// 		{
// 			continue;
// 		}
// 		bNoBoard = FALSE;
// 		nRetVal = iterController->second->SetSaveSelectFail(uRAMLineNo, bStartSave, bBRAM, bDelete);
// 		if (0 != nRetVal)
// 		{
// 			nRetVal = -1;
// 			return nRetVal;
// 		}
// 	}
// 
// 	if (bNoBoard)
// 	{
// 		nRetVal = -2;
// 	}
// 	return nRetVal;
// }
// 
// int CBoard::GetInstruction(BYTE byController, UINT uBRAMLineNo, char* lpszInstruction, int nBuffSize)
// {
// 	if (DCM400_MAX_CONTROLLERS_PRE_BOARD <= byController)
// 	{
// 		return -1;
// 	}
// 	auto iterController = m_mapController.find(byController);
// 	if (m_mapController.end() == iterController || nullptr == iterController->second)
// 	{
// 		return -2;
// 	}
// 	int nRetVal = iterController->second->GetInstruction(uBRAMLineNo, lpszInstruction, nBuffSize);
// 	if (0 != nRetVal)
// 	{
// 		switch (nRetVal)
// 		{
// 		case -1:
// 			///<The line number is over range
// 			nRetVal = -3;
// 			break;
// 		case -2:
// 			///<The instruction is nullptr
// 			nRetVal = -4;
// 			break;
// 		case -3:
// 			///<The buff size is too small
// 			nRetVal = -5;
// 			break;
// 		default:
// 			break;
// 		}
// 	}
// 	return nRetVal;
// }
// 
// int CBoard::GetOperand(BYTE byController, UINT uBRAMLineNo)
// {
// 	if (DCM400_MAX_CONTROLLERS_PRE_BOARD <= byController)
// 	{
// 		return -1;
// 	}
// 	auto iterController = m_mapController.find(byController);
// 	if (m_mapController.end() == iterController)
// 	{
// 		return -2;
// 	}
// 	int nRetVal = iterController->second->GetOperand(uBRAMLineNo);
// 	if (0 > nRetVal)
// 	{
// 		nRetVal = -3;
// 	}
// 	return nRetVal;
// }
// 
// int CBoard::Connect(const std::vector<USHORT>& vecChannel, BOOL bConnect)
// {
// 	if (!m_bExisted || nullptr == m_pHardwareFunction)
// 	{
// 		return -1;
// 	}
// 	if (0 == vecChannel.size())
// 	{
// 		return 0;
// 	}
// 	int nRetVal = m_pHardwareFunction->SetFunctionRelay(vecChannel, bConnect);
// 	if (0 != nRetVal)
// 	{
// 		return -2;
// 	}
// 	return 0;
// }
// 
// int CBoard::SetCalibrationRelay(USHORT usChannel, BOOL bConnect)
// {
// 	if (DCM400_MAX_CHANNELS_PER_BOARD <= usChannel)
// 	{
// 		return -1;
// 	}
// 	m_pHardwareFunction->SetCalibrationRelay(usChannel, bConnect);
// 	return 0;
// }
// 
// int CBoard::GetConnectChannel(std::vector<USHORT>& vecChannel, RELAY_TYPE RelayType)
// {
// 	if (!m_bExisted || nullptr == m_pHardwareFunction)
// 	{
// 		return -1;
// 	}
// 	m_pHardwareFunction->GetConnectChannel(vecChannel, RelayType);
// 	return 0;
// }
// 
// int CBoard::InitMCU(const std::vector<USHORT>& vecChannel)
// {
// 	m_ChannelClassifiy.SetChannel(vecChannel);
// 	vector<USHORT> vecControllerChannel;
// 
// 	for (auto& Controller : m_mapController)
// 	{
// 		if (nullptr == Controller.second)
// 		{
// 			continue;
// 		}
// 		m_ChannelClassifiy.GetChannel(Controller.first, vecControllerChannel);
// 		if (0 == vecControllerChannel.size())
// 		{
// 			continue;
// 		}
// 		Controller.second->InitMCU(vecControllerChannel);
// 	}
// 	return 0;
// }
// 
// 
// int CBoard::InitPMU(const std::vector<USHORT>& vecChannel)
// {
// 	m_ChannelClassifiy.SetChannel(vecChannel);
// 	vector<USHORT> vecControllerChannel;
// 
// 	for (auto& Controller : m_mapController)
// 	{
// 		if (nullptr == Controller.second)
// 		{
// 			continue;
// 		}
// 		m_ChannelClassifiy.GetChannel(Controller.first, vecControllerChannel);
// 		if (0 == vecControllerChannel.size())
// 		{
// 			continue;
// 		}
// 		Controller.second->InitPMU(vecControllerChannel);
// 	}
// 	return 0;
// }
// 
// int CBoard::SetPinLevel(std::vector<USHORT>& vecChannel, double dVIH, double dVIL, double dVOH, double dVOL)
// {
// 	int nRetVal = 0;
// 	m_ChannelClassifiy.SetChannel(vecChannel);
// 	vector<USHORT> vecControllerChannel;
// 
// 	for (auto& Controller : m_mapController)
// 	{
// 		if (nullptr == Controller.second)
// 		{
// 			continue;
// 		}
// 		m_ChannelClassifiy.GetChannel(Controller.first, vecControllerChannel);
// 		if (0 == vecControllerChannel.size())
// 		{
// 			continue;
// 		}
// 		nRetVal = Controller.second->SetPinLevel(vecControllerChannel, dVIH, dVIL, dVOH, dVOL);
// 		if (0 != nRetVal)
// 		{
// 			nRetVal = -1;
// 			break;
// 		}
// 	}
// 	return nRetVal;
// }
// 
// int CBoard::SetChannelStatus(const std::vector<USHORT>& vecChannel, CHANNEL_OUTPUT_STATUS ChannelStatus)
// {
// 	int nRetVal = 0;
// 	BOOL bNoChannel = TRUE;
// 	m_ChannelClassifiy.SetChannel(vecChannel);
// 	vector<USHORT> vecControllerChannel;
// 
// 	for (auto& Controller : m_mapController)
// 	{
// 		if (nullptr == Controller.second)
// 		{
// 			continue;
// 		}
// 		m_ChannelClassifiy.GetChannel(Controller.first, vecControllerChannel);
// 		if (0 == vecControllerChannel.size())
// 		{
// 			continue;
// 		}
// 		bNoChannel = FALSE;
// 		nRetVal = Controller.second->SetChannelStatus(vecControllerChannel, ChannelStatus);
// 		if (0 != nRetVal)
// 		{
// 			switch (nRetVal)
// 			{
// 			case -1:
// 				nRetVal = -1;
// 				break;
// 			case -2:
// 				///<Not happened
// 			default:
// 				break;
// 			}
// 			break;
// 		}
// 	}
// 	if (bNoChannel)
// 	{
// 		nRetVal = -2;
// 	}
// 	return nRetVal;
// }
// 
// int CBoard::GetChannelMode(USHORT usChannel)
// {
// 	if (DCM400_MAX_CHANNELS_PER_BOARD <= usChannel)
// 	{
// 		return -1;
// 	}
// 	auto iterController = m_mapController.find(usChannel / DCM400_CHANNELS_PER_CONTROL);
// 	if (m_mapController.end() == iterController)
// 	{
// 		return -2;
// 	}
// 	int nRetVal = iterController->second->GetChannelMode(usChannel % DCM400_CHANNELS_PER_CONTROL);
// 	if (0 > nRetVal)
// 	{
// 		///<Channel number is over range, not will happened
// 	}
// 	return nRetVal;
// }
// 
// void CBoard::UpdateChannelMode()
// {
// 	for (auto& Controller : m_mapController)
// 	{
// 		if (nullptr != Controller.second)
// 		{
// 			Controller.second->UpdateChannelMode();
// 		}
// 	}
// }
// 
// int CBoard::GetChannelStatus(USHORT usChannel)
// {
// 	if (DCM400_MAX_CHANNELS_PER_BOARD <= usChannel)
// 	{
// 		return -1;
// 	}
// 	auto iterController = m_mapController.find(usChannel / DCM400_CHANNELS_PER_CONTROL);
// 	if (m_mapController.end() == iterController || nullptr == iterController->second)
// 	{
// 		return -2;
// 	}
// 
// 	return iterController->second->GetChannelStatus(usChannel % DCM400_CHANNELS_PER_CONTROL);
// }
// 
// int CBoard::SetRunParam(std::vector<USHORT>& vecChannel, UINT uStartLineNumber, UINT uStopLineNumber, BOOL bWithDRAM, UINT uDRAMStartLine, BOOL bEnableStart)
// {
// 	m_ChannelClassifiy.SetChannel(vecChannel);
// 	vector<USHORT> vecControllerChannel;
// 
// 	int nRetVal = 0;
// 	for (auto& Controller : m_mapController)
// 	{
// 		if (nullptr == Controller.second)
// 		{
// 			continue;
// 		}
// 		m_ChannelClassifiy.GetChannel(Controller.first, vecControllerChannel);
// 		if (0 == vecControllerChannel.size())
// 		{
// 			continue;
// 		}
// 		Controller.second->SetComparedChannel(vecControllerChannel);
// 		Controller.second->SetChannelMCUMode(vecControllerChannel);
// 		nRetVal = Controller.second->SetRunParam(uStartLineNumber, uStopLineNumber, bWithDRAM, uDRAMStartLine, bEnableStart);
// 		if (0 != nRetVal)
// 		{
// 			break;
// 		}
// 	}
// 	return nRetVal;
// }
// 
// int CBoard::SynRun(USHORT usChannel)
// {
// 	m_mapFailNoFilter.clear();
// 	if (DCM400_MAX_CHANNELS_PER_BOARD <= usChannel)
// 	{
// 		return -1;
// 	}
// 	auto iterController = m_mapController.find(usChannel / DCM400_CHANNELS_PER_CONTROL);
// 
// 	if (m_mapController.end() == iterController || nullptr == iterController->second)
// 	{
// 		return -2;
// 	}
// 	iterController->second->SynRun();
// 	return 0;
// }
// 
// int CBoard::StopVector(std::vector<USHORT>& vecChannel)
// {
// 	set<BYTE> setControllerIndex;
// 	int nRetVal = 0;
// 	BOOL bNoChannelExisted = TRUE;
// 	m_ChannelClassifiy.GetController(vecChannel, setControllerIndex);
// 
// 	for (auto Controller : setControllerIndex)
// 	{
// 		auto iterController = m_mapController.find(Controller);
// 		if (m_mapController.end() != iterController && nullptr != iterController->second)
// 		{
// 			iterController->second->Stop();
// 			bNoChannelExisted = FALSE;
// 		}
// 	}
// 	if (bNoChannelExisted)
// 	{
// 		return -1;
// 	}
// 	return 0;
// }
// 
// void CBoard::EnableStart(std::vector<USHORT>& vecChannel, BOOL bEnable)
// {
// 	set<BYTE> setControllerIndex;
// 	int nRetVal = 0;
// 	m_ChannelClassifiy.GetController(vecChannel, setControllerIndex);
// 
// 	for (auto Controller : setControllerIndex)
// 	{
// 		auto iterController = m_mapController.find(Controller);
// 		if (m_mapController.end() != iterController && nullptr != iterController->second)
// 		{
// 			iterController->second->EnableStart(bEnable);
// 		}
// 	}
// }
// 
// int CBoard::WaitStop(std::vector<USHORT>& vecChannel, UINT uTimeout)
// {
// 	UINT uWaitTimes = uTimeout * 100;
// 	UINT uCurTimes = 0;
// 	set<BYTE> setControllerIndex;
// 	int nRetVal = 0;
// 	m_ChannelClassifiy.GetController(vecChannel, setControllerIndex);
// 	for (auto Controller : setControllerIndex)
// 	{
// 		auto iterController = m_mapController.find(Controller);
// 		if (m_mapController.end() != iterController && nullptr != iterController->second)
// 		{
// 			nRetVal = 0;///<Running
// 			while (0 == nRetVal && uWaitTimes > uCurTimes)
// 			{
// 				if (0 != uCurTimes++)
// 				{
// 					Wait(10);
// 				}
// 				nRetVal = iterController->second->GetRunningStatus();
// 			}
// 		}
// 	}
// 	if (uWaitTimes <= uCurTimes)
// 	{
// 		return -1;
// 	}
// 	return 0;
// }
// 
// int CBoard::GetMCUResult(const std::vector<USHORT>& vecChannel)
// {
// 	int nRetVal = 0;
// 	m_ChannelClassifiy.SetChannel(vecChannel);
// 	vector<USHORT> vecControllerChannel;
// 	BOOL bHasBoard = FALSE;
// 	for (auto& Controller : m_mapController)
// 	{
// 		if (nullptr == Controller.second)
// 		{
// 			continue;
// 		}
// 		m_ChannelClassifiy.GetChannel(Controller.first, vecControllerChannel);
// 		if (0 == vecControllerChannel.size())
// 		{
// 			continue;
// 		}
// 		bHasBoard = TRUE;
// 		nRetVal = Controller.second->GetMCUResult(vecControllerChannel);
// 		if (0 <= nRetVal)
// 		{
// 			if (1 == nRetVal)
// 			{
// 				break;
// 			}
// 			continue;
// 		}
// 		switch (nRetVal)
// 		{
// 		case -1:
// 			///<Not ran vector before
// 			m_pAlarm->SetAlarmMsg("The board(slot%d) not ran vector before.", m_bySlotNo);
// 			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_NOT_RAN_VECTOR);
// 			m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
// 			nRetVal = -1;
// 			break;
// 		case -2:
// 			///<Vector running
// 			m_pAlarm->SetAlarmMsg("The board(slot%d) is running now.", m_bySlotNo);
// 			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_VECTOR_RUNNING);
// 			m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmError);
// 			nRetVal = -2;
// 			break;
// 		case -3:
// 			///<Channel is over range
// 			///<Not will happen
// 		default:
// 			break;
// 		}
// 		break;
// 	}
// 	if (!bHasBoard)
// 	{
// 		nRetVal = -3;
// 	}
// 	return nRetVal;
// }
// 
// int CBoard::GetChannelResult(USHORT usChannel)
// {
// 	if (DCM400_MAX_CHANNELS_PER_BOARD <= usChannel)
// 	{
// 		return -1;
// 	}
// 	int nControllerIndex = usChannel / DCM400_CHANNELS_PER_CONTROL;
// 	auto iterController = m_mapController.find(nControllerIndex);
// 	if (m_mapController.end() == iterController || nullptr == iterController->second)
// 	{
// 		return -2;
// 	}
// 	int nRetVal = iterController->second->GetChannelResult(usChannel % DCM400_CHANNELS_PER_CONTROL);
// 	if (0 > nRetVal)
// 	{
// 		switch (nRetVal)
// 		{
// 		case -1:
// 			///<Channel is over range
// 			nRetVal = -1;
// 			break;
// 		case -2:
// 			///<Not ran vector before
// 			nRetVal = -3;
// 			break;
// 		case -3:
// 			///<Vector running
// 			nRetVal = -4;
// 			break;
// 		default:
// 			nRetVal = -4;
// 			break;
// 		}
// 	}
// 	return nRetVal;
// }
// 
// int CBoard::GetRunningStatus(USHORT usChannel)
// {
// 	if (DCM400_MAX_CHANNELS_PER_BOARD <= usChannel)
// 	{
// 		return -1;
// 	}
// 	int nControllerIndex = usChannel / DCM400_CHANNELS_PER_CONTROL;
// 	auto iterController = m_mapController.find(nControllerIndex);
// 	if (m_mapController.end() == iterController || nullptr == iterController->second)
// 	{
// 		return -2;
// 	}
// 	int nRetVal = iterController->second->GetRunningStatus();
// 	if (0 > nRetVal)
// 	{
// 		nRetVal = 2;
// 	}
// 	return nRetVal;
// }
// 
// int CBoard::GetFailCount(USHORT usChannel)
// {
// 	if (DCM400_MAX_CHANNELS_PER_BOARD <= usChannel)
// 	{
// 		return -1;
// 	}
// 	int nControllerIndex = usChannel / DCM400_CHANNELS_PER_CONTROL;
// 	auto iterController = m_mapController.find(nControllerIndex);
// 	if (m_mapController.end() == iterController || nullptr == iterController->second)
// 	{
// 		return -2;
// 	}
// 	int nRetVal = iterController->second->GetFailCount(usChannel % DCM400_CHANNELS_PER_CONTROL);
// 	if (0 > nRetVal)
// 	{
// 		switch (nRetVal)
// 		{
// 		case -1:
// 			nRetVal = -1;
// 			break;
// 		case -2:
// 			///<Not ran before
// 			nRetVal = -3;
// 			break;
// 		case -3:
// 			///<Vector running
// 			nRetVal = -4;
// 			break;
// 		default:
// 			break;
// 		}
// 	}
// 	return nRetVal;
// }
// 
// int CBoard::GetFailLineNo(USHORT usChannel, UINT uGetMaxFailCount, std::vector<int>& vecBRAMLineNo, std::vector<int>& vecDRAMLineNo, BOOL bForceRefresh)
// {
// 	if (DCM400_MAX_CHANNELS_PER_BOARD <= usChannel)
// 	{
// 		return -1;
// 	}
// 
// 	auto iterControl = m_mapController.find(usChannel / DCM400_CHANNELS_PER_CONTROL);
// 	if (m_mapController.end() == iterControl || nullptr == iterControl->second)
// 	{
// 		return -2;
// 	}
// 	int nRetVal = iterControl->second->GetFailLineNo(usChannel % DCM400_CHANNELS_PER_CONTROL, uGetMaxFailCount, vecBRAMLineNo, vecDRAMLineNo, bForceRefresh);
// 	if (0 > nRetVal)
// 	{
// 		switch (nRetVal)
// 		{
// 		case -1:
// 			///<The channel is over range, will not happened
// 			nRetVal = -2;
// 			break;
// 		case -2:
// 			///<No ran
// 			nRetVal = -3;
// 			break;
// 		case -3:
// 			///<Vector running
// 			nRetVal = -4;
// 			break;
// 		default:
// 			break;
// 		}
// 	}
// 	return nRetVal;
// }
// 
// int CBoard::GetFailLineCountFilter(USHORT usChannel)
// {
// 	if (DCM400_MAX_CHANNELS_PER_BOARD <= usChannel)
// 	{
// 		return -1;
// 	}
// 	BYTE byControllerIndex = usChannel / DCM400_CHANNELS_PER_CONTROL;
// 	auto iterControl = m_mapController.find(usChannel / DCM400_CHANNELS_PER_CONTROL);
// 	if (m_mapController.end() == iterControl || nullptr == iterControl->second)
// 	{
// 		return -2;
// 	}
// 	auto iterFilterCount = m_mapFailNoFilter.find(byControllerIndex);
// 	if (m_mapFailNoFilter.end() == iterFilterCount)
// 	{
// 		return -3;
// 	}
// 	return iterFilterCount->second;
// }
// 
// int CBoard::GetMCUFailLineNo(USHORT usChannel, std::vector<int>& vecBRAMLineNo, std::vector<int>& vecDRAMLineNo, BOOL bForceRefresh /*= FALSE*/)
// {
// 	if (DCM400_MAX_CHANNELS_PER_BOARD <= usChannel)
// 	{
// 		return -1;
// 	}
// 
// 	auto iterControl = m_mapController.find(usChannel / DCM400_CHANNELS_PER_CONTROL);
// 	if (m_mapController.end() == iterControl || nullptr == iterControl->second)
// 	{
// 		return -2;
// 	}
// 	int nRetVal = iterControl->second->GetMCUFailLineNo(vecBRAMLineNo, vecDRAMLineNo, bForceRefresh);
// 	if (0 > nRetVal)
// 	{
// 		switch (nRetVal)
// 		{
// 		case -1:
// 			///<The channel is over range, will not happened
// 			nRetVal = -2;
// 			break;
// 		case -2:
// 			///<No ran
// 			nRetVal = -3;
// 			break;
// 		case -3:
// 			///<Vector running
// 			nRetVal = -4;
// 			break;
// 		default:
// 			break;
// 		}
// 	}
// 	return nRetVal;
// }
// 
// int CBoard::DeleteControllerFailLine(USHORT usChannel, int nBRAMDeleteCount, int nDRAMDeleteCount)
// {
// 	if (DCM400_MAX_CHANNELS_PER_BOARD <= usChannel)
// 	{
// 		return -1;
// 	}
// 	BYTE byControllerIndex = usChannel / DCM400_CHANNELS_PER_CONTROL;
// 	auto iterControl = m_mapController.find(byControllerIndex);
// 	if (m_mapController.end() == iterControl || nullptr == iterControl->second)
// 	{
// 		return -2;
// 	}
// 	iterControl->second->DeleteFailLine(nBRAMDeleteCount, nDRAMDeleteCount);
// 	m_mapFailNoFilter.insert(make_pair(byControllerIndex, nBRAMDeleteCount + nDRAMDeleteCount));
// 	return 0;
// }
// 
// int CBoard::GetLastCertainResultLineNo(USHORT usChannel, int& nBRAMLineNo, BOOL& bBRAMLineFail, int& nDRAMLineNo, BOOL& bDRAMLineFail)
// {
// 	if (DCM400_MAX_CHANNELS_PER_BOARD <= usChannel)
// 	{
// 		return -1;
// 	}
// 	int nControllerIndex = usChannel / DCM400_CHANNELS_PER_CONTROL;
// 	auto iterControl = m_mapController.find(nControllerIndex);
// 	if (m_mapController.end() == iterControl || nullptr == iterControl->second)
// 	{
// 		return -2;
// 	}
// 	iterControl->second->GetLastCertainResultLineNo(usChannel % DCM400_CHANNELS_PER_CONTROL, nBRAMLineNo, bBRAMLineFail, nDRAMLineNo, bDRAMLineFail);
// 	
// 	return 0;
// }
// 
// int CBoard::PreloadFailLineNo(const std::vector<USHORT>& vecChannel, UINT uGetMaxFailCount)
// {
// 	int nRetVal = 0;
// 	BOOL bNoBoard = TRUE;
// 	m_ChannelClassifiy.SetChannel(vecChannel);
// 	vector<USHORT> vecControllerChannel;
// 
// 	for (auto& Controller : m_mapController)
// 	{
// 		if (nullptr == Controller.second)
// 		{
// 			continue;
// 		}
// 		m_ChannelClassifiy.GetChannel(Controller.first, vecControllerChannel);
// 		if (0 == vecControllerChannel.size())
// 		{
// 			continue;
// 		}
// 		bNoBoard = FALSE;
// 		nRetVal = Controller.second->PreloadFailLineNo(uGetMaxFailCount);
// 		if (0 != nRetVal)
// 		{
// 			switch (nRetVal)
// 			{
// 			case -1:
// 				///<Not ran
// 				nRetVal = -1;
// 				break;
// 			case -2:
// 				///<Running
// 				nRetVal = -2;
// 				break;
// 			default:
// 				break;
// 			}
// 			return nRetVal;
// 		}
// 	}
// 	if (bNoBoard)
// 	{
// 		return -3;
// 	}
// 	return 0;
// }
// 
// int CBoard::GetCapture(USHORT usChannel, std::vector<LINE_DATA>& vecBRAM, std::vector<LINE_DATA>& vecDRAM)
// {
// 	if (DCM400_MAX_CHANNELS_PER_BOARD <= usChannel)
// 	{
// 		///<Channel number over range
// 		return -1;
// 	}
// 	BYTE byController = usChannel / DCM400_CHANNELS_PER_CONTROL;
// 	auto iterController = m_mapController.find(byController);
// 	if (m_mapController.end() == iterController || nullptr == iterController->second)
// 	{
// 		///<Channel number is not existed
// 		return -2;
// 	}
// 
// 	int nRetVal = iterController->second->GetCapture(usChannel % DCM400_CHANNELS_PER_CONTROL, vecBRAM, vecDRAM);
// 	if (0 != nRetVal)
// 	{
// 		switch (nRetVal)
// 		{
// 		case -1:
// 			///<Channel is over range, will not happened
// 			break;
// 		case -2:
// 			///<Not ran vector
// 			nRetVal = -3;
// 			break;
// 		case -3:
// 			///<Vector running
// 			nRetVal = -4;
// 			break;
// 		default:
// 			break;
// 		}
// 		return nRetVal;
// 	}
// 	return 0;
// }
// 
// int CBoard::GetStopLineNo(USHORT usChannel)
// {
// 	if (DCM400_MAX_CHANNELS_PER_BOARD <= usChannel)
// 	{
// 		return -1;
// 	}
// 	int nControllerIndex = usChannel / DCM400_CHANNELS_PER_CONTROL;
// 	auto iterControl = m_mapController.find(nControllerIndex);
// 	if (m_mapController.end() == iterControl || nullptr == iterControl->second)
// 	{
// 		return -2;
// 	}
// 	int nRetVal = iterControl->second->GetStopLineNo();
// 	if (0 > nRetVal)
// 	{
// 		switch (nRetVal)
// 		{
// 		case -1:
// 			///<The channel is over range, will not happened
// 			nRetVal = -2;
// 			break;
// 		case -2:
// 			///<No ran
// 			nRetVal = -3;
// 			break;
// 		case -3:
// 			///<Vector running
// 			nRetVal = -4;
// 			break;
// 		default:
// 			break;
// 		}
// 	}
// 	return nRetVal;
// }
// 
// int CBoard::GetRunLineCount(USHORT usChannel, ULONG& ulLineCount)
// {
// 	ulLineCount = -1;
// 	if (DCM400_MAX_CHANNELS_PER_BOARD <= usChannel)
// 	{
// 		return -1;
// 	}
// 	auto iterController = m_mapController.find(usChannel / DCM400_CHANNELS_PER_CONTROL);
// 	if (m_mapController.end() == iterController || nullptr == iterController->second)
// 	{
// 		return -2;
// 	}
// 	ulLineCount = iterController->second->GetRunLineCount();
// 	return 0;
// }
// 
// int CBoard::SetCalibrationInfo(BYTE byControllerIndex, STS_CALINFO* pCalInfo, BYTE *pbyChannelStatus, int nElementCount)
// {
// 	if (DCM400_MAX_CONTROLLERS_PRE_BOARD <= byControllerIndex)
// 	{
// 		return -1;
// 	}
// 	auto iterController = m_mapController.find(byControllerIndex);
// 	if (m_mapController.end() == iterController || nullptr == iterController->second)
// 	{
// 		return -2;
// 	}
// 	int nRetVal = iterController->second->SetCalibrationInfo(pCalInfo, pbyChannelStatus, nElementCount);
// 	if (0 != nRetVal)
// 	{
// 		return nRetVal - 2;
// 	}
// 
// 	return 0;
// }
// 
// int CBoard::GetCalibrationInfo(BYTE byControllerIndex, STS_CALINFO* pCalInfo, int nElementCount)
// {
// 	if (DCM400_MAX_CONTROLLERS_PRE_BOARD <= byControllerIndex)
// 	{
// 		return -1;
// 	}
// 	auto iterController = m_mapController.find(byControllerIndex);
// 	if (m_mapController.end() == iterController || nullptr == iterController->second)
// 	{
// 		return -2;
// 	}
// 	int nRetVal = iterController->second->GetCalibrationInfo(pCalInfo, nElementCount);
// 	if (0 != nRetVal)
// 	{
// 		return nRetVal - 3;
// 	}
// 	return 0;
// }
// 
// int CBoard::GetCalibrationInfo(USHORT usChannel, STS_CALINFO &CalibrationInfo)
// {
// 	if (DCM400_MAX_CHANNELS_PER_BOARD <= usChannel)
// 	{
// 		return -1;
// 	}
// 	BYTE byControllerIndex = usChannel / DCM400_CHANNELS_PER_CONTROL;
// 	auto iterController = m_mapController.find(byControllerIndex);
// 	if (m_mapController.end() == iterController || nullptr == iterController->second)
// 	{
// 		return -2;
// 	}
// 	int nRetVal = iterController->second->GetCalibrationInfo(usChannel % DCM400_CHANNELS_PER_CONTROL, CalibrationInfo);
// 	if (0 != nRetVal)
// 	{
// 		nRetVal = -3;
// 	}
// 	return nRetVal;
// }
// 
// int CBoard::SetCalibrationData(BYTE byControllerIndex, DCM400_CAL_DATA *pCalibrationData, int nElementCount)
// {
// 	if (DCM400_MAX_CONTROLLERS_PRE_BOARD <= byControllerIndex)
// 	{
// 		return -1;
// 	}
// 	auto iterController = m_mapController.find(byControllerIndex);
// 	if (m_mapController.end() == iterController || nullptr == iterController->second)
// 	{
// 		return -2;
// 	}
// 	int nRetVal = iterController->second->SetCalibrationData(pCalibrationData, nElementCount);
// 	if (0 != nRetVal)
// 	{
// 		nRetVal -= 2;
// 	}
// 	return nRetVal;
// }
// 
// int CBoard::ResetCalibrationData(BYTE byControllerIndex)
// {
// 	if (DCM400_MAX_CONTROLLERS_PRE_BOARD <= byControllerIndex)
// 	{
// 		return -1;
// 	}
// 	auto iterController = m_mapController.find(byControllerIndex);
// 	if (m_mapController.end() == iterController || nullptr == iterController->second)
// 	{
// 		return -2;
// 	}
// 	int nRetVal = iterController->second->ResetCalibrationData();
// 	if (0 != nRetVal)
// 	{
// 		nRetVal = -3;
// 	}
// 	return nRetVal;
// }
// 
// int CBoard::GetCalibrationData(BYTE byControllerIndex, DCM400_CAL_DATA* pCalibrationData, int nElementCount)
// {
// 	if (DCM400_MAX_CONTROLLERS_PRE_BOARD <= byControllerIndex)
// 	{
// 		return -1;
// 	}
// 	auto iterController = m_mapController.find(byControllerIndex);
// 	if (m_mapController.end() == iterController || nullptr == iterController->second)
// 	{
// 		return -2;
// 	}
// 	if (nullptr == pCalibrationData)
// 	{
// 		return -3;
// 	}
// 	if (DCM400_CHANNELS_PER_CONTROL > nElementCount)
// 	{
// 		return -4;
// 	}
// 	int nRetVal = iterController->second->GetCalibrationData(pCalibrationData, nElementCount);
// 	if (0 > nRetVal)
// 	{
// 		switch (nRetVal)
// 		{
// 		case -1:
// 			nRetVal = -3;
// 			break;
// 		case -2:
// 			nRetVal = -4;
// 			break;
// 		case -3:
// 			nRetVal = -5;
// 			break;
// 		case -4:
// 			nRetVal = -6;
// 		default:
// 			break;
// 		}
// 	}
// 	return nRetVal;
// }
// 
// int CBoard::ReadCalibrationData(BYTE byControllerIndex)
// {
// 	if (DCM400_MAX_CONTROLLERS_PRE_BOARD <= byControllerIndex)
// 	{
// 		return -1;
// 	}
// 	auto iterController = m_mapController.find(byControllerIndex);
// 	if (m_mapController.end() == iterController || nullptr == iterController->second)
// 	{
// 		return -2;
// 	}
// 	
// 	int nRetVal = iterController->second->ReadCalibrationData();
// 	if (0 > nRetVal)
// 	{
// 		switch (nRetVal)
// 		{
// 		case -1:
// 			nRetVal = -3;
// 			break;
// 		case -2:
// 			nRetVal = -4;
// 			break;
// 		case -3:
// 			nRetVal = -5;
// 			break;
// 		default:
// 			break;
// 		}
// 	}
// 	return nRetVal;
// }
// 
// int CBoard::SetHardInfo(STS_HARDINFO* pHardInfo, int nModuleCount)
// {
// 	if (!m_bExisted || nullptr == m_pHardwareFunction)
// 	{
// 		return -1;
// 	}
// 	if (nullptr == pHardInfo || 0 >= nModuleCount)
// 	{
// 		return -2;
// 	}
// 	map<STS_BOARD_MODULE, STS_MODULEINFO> mapHardInfo;
// 	for (int nModuleIndex = 0; nModuleIndex < nModuleCount;++nModuleIndex)
// 	{
// 		mapHardInfo.insert(make_pair(pHardInfo[nModuleIndex].module, pHardInfo[nModuleIndex].moduleInfo));
// 	}
// 	CHardInfo HardInfo(*m_pHardwareFunction);
// 	int nRetVal = HardInfo.SetHardInfo(mapHardInfo);
// 	if (!nRetVal)
// 	{
// 		return -3;
// 	}
// 	return 0;
// }
// 
// int CBoard::GetHardInfo(STS_HARDINFO* pHardInfo, int nElementCount)
// {
// 	if (!m_bExisted || nullptr == m_pHardwareFunction)
// 	{
// 		return -1;
// 	}
// 	map<STS_BOARD_MODULE, STS_MODULEINFO> mapHardInfo;
// 	CHardInfo HardInfo(*m_pHardwareFunction);
// 	mapHardInfo = HardInfo.GetHardInfo();
// 	int nModuleCount = mapHardInfo.size();
// 	int nModuleIndex = 0;
// 	for (auto& HardInfo : mapHardInfo)
// 	{
// 		if (nullptr == pHardInfo || nModuleIndex >= nElementCount)
// 		{
// 			break;
// 		}
// 		pHardInfo[nModuleIndex].module = HardInfo.first;
// 		pHardInfo[nModuleIndex++].moduleInfo = HardInfo.second;
// 	}
// 	return nModuleCount;
// }
// 
int CBoard::GetChannelCount(BOOL bForceRead)
{
	if (!m_bExisted || nullptr == m_pHardwareFunction)
	{
		return -1;
	}
	if (!bForceRead && 0 != m_usChannelCount)
	{
		return m_usChannelCount;
	}

	if (!m_pHardwareFunction->CheckFlashID())
	{
		return -2;
	}
	DWORD dwDataLength = 0;
	BYTE *byDataLength = (BYTE*)(&dwDataLength);
	DWORD dwMaxDataLength = 256;
	int nRetVal = 0;
	BYTE *pbyReadData = nullptr;

	const int nSectorCount = 2;
	BYTE bySector[nSectorCount] = { CONTROL_COUNT_SAVE_SECTOR, BACKUP_CONTROL_COUNT_SECTOR };

	BOOL bOriSector = FALSE;
	UCHAR cCheckCode[16] = { 0 };
	UCHAR cReadCheckCode[16] = { 0 };
	for (int nIndex = 0; nIndex < nSectorCount; ++nIndex)
	{
		nRetVal = m_pHardwareFunction->ReadFlash(bySector[nIndex], CONTROL_COUNT_PAGE, 0, 4, byDataLength);
		if (0 == dwDataLength || (DWORD)-1 == dwDataLength || dwDataLength >= dwMaxDataLength)
		{
			nRetVal = -2;
			continue;
		}
		int nDataLeft = dwDataLength;
		int	nStartIndex = 0;
		BOOL bMD5Success = TRUE;
		int nMD5FailTimes = 0;
		BYTE byOffset = 0;
		if (nullptr == pbyReadData)
		{
			try
			{
				pbyReadData = new BYTE[dwDataLength];
				memset(pbyReadData, 0, dwDataLength * sizeof(BYTE));
			}
			catch (const std::exception&)
			{
				return -3;
			}
		}
		do
		{
			//Read the flash data
			nRetVal = m_pHardwareFunction->ReadFlash(bySector[nIndex], CONTROL_COUNT_PAGE, 4, (BYTE)dwDataLength, pbyReadData);

			memcpy(cReadCheckCode, pbyReadData, sizeof(cReadCheckCode));
			//Calculate the MD5 check code of the data read.
			STSMD5Context context;
			STSMD5_Init(&context);
			STSMD5_Update(&context, &pbyReadData[sizeof(cCheckCode)], dwDataLength - sizeof(cCheckCode));
			STSMD5_Final(&context, cCheckCode);
			if (0 == memcmp(cCheckCode, cCheckCode, sizeof(cCheckCode)))
			{
				bMD5Success = TRUE;
			}
			else
			{
				bMD5Success = FALSE;
				++nMD5FailTimes;
			}
		} while (!bMD5Success && 2 >= nMD5FailTimes);
		if (bMD5Success)
		{
			if (nSectorCount == nIndex + 1)
			{
				bOriSector = TRUE;
			}
			nRetVal = 0;
			break;
		}
	}
	if (0 != nRetVal)
	{
		if (nullptr != pbyReadData)
		{
			delete[] pbyReadData;
			pbyReadData = nullptr;
		}
		return -4;
	}
	DWORD dwControlNum = 0;
	memcpy(&dwControlNum, &pbyReadData[sizeof(cCheckCode)], 4);
	if (nullptr != pbyReadData)
	{
		delete[] pbyReadData;
		pbyReadData = nullptr;
	}
	BYTE byControllerCount = 0;
	for (int nControllerIndex = 0; nControllerIndex < DCM400_MAX_CONTROLLERS_PRE_BOARD;++nControllerIndex)
	{
		if (0 != (dwControlNum >> nControllerIndex & 0x01))
		{
			++byControllerCount;
		}
	}
	m_usChannelCount = byControllerCount * DCM400_CHANNELS_PER_CONTROL;

	InitController();

	return m_usChannelCount;
}

int CBoard::SetChannelCount(USHORT usChannelCount)
{
	if (!m_bExisted && nullptr != m_pHardwareFunction)
	{
		return -1;
	}
	if (DCM400_MAX_CHANNELS_PER_BOARD < usChannelCount)
	{
		return -2;
	}
	BYTE byControllerCount = usChannelCount / DCM400_CHANNELS_PER_CONTROL;
	BYTE byControllerStatus = 0;
	for (int nIndex = 0; nIndex < byControllerCount; ++nIndex)
	{
		byControllerStatus |= 1 << nIndex;
	}

	if (!m_pHardwareFunction->CheckFlashID())
	{
		return -3;
	}

	const BYTE bySectorCount = 2;
	BYTE bySector[bySectorCount] = { CONTROL_COUNT_SAVE_SECTOR, BACKUP_CONTROL_COUNT_SECTOR };

	UCHAR ucCheckCode[16] = { 0 };
	DWORD dwSumSize = sizeof(ucCheckCode) + sizeof(byControllerStatus);
	BYTE* byData = STS_NULL;
	BYTE* bySize = (BYTE*)(&dwSumSize);
	int nRewriteTimes = 0;//The times of write initialization data to flash after write fail.
	BOOL bWriteSuccess = TRUE;
	for (int nSectorIndex = 0; nSectorIndex < bySectorCount; ++nSectorIndex)
	{
		do
		{
			DWORD dwOffset = 0;
			bWriteSuccess = TRUE;
			m_pHardwareFunction->EraseFlash(bySector[nSectorIndex]);//Erase the sector.

			//Write the length of the initialization data per SE8212 board.
			int nValue = m_pHardwareFunction->WriteFlash(bySector[nSectorIndex], CONTROL_COUNT_PAGE, (BYTE)dwOffset, 4, bySize);
			dwOffset += 4;
			if (0 != nValue)
			{
				++nRewriteTimes;
				bWriteSuccess = FALSE;
				break;
			}

			//Write MD5 check code
			STSMD5Context context;
			STSMD5_Init(&context);

			STSMD5_Update(&context, (unsigned char*)(&byControllerStatus), sizeof(byControllerStatus));


			STSMD5_Final(&context, ucCheckCode);

			nValue = m_pHardwareFunction->WriteFlash(bySector[nSectorIndex], CONTROL_COUNT_PAGE, (BYTE)dwOffset, sizeof(ucCheckCode), ucCheckCode);
			dwOffset += sizeof(ucCheckCode);
			if (0 != nValue)
			{
				++nRewriteTimes;
				bWriteSuccess = FALSE;
				break;
			}

			byData = &byControllerStatus;

			//Write the initialization data into flash.
			nValue = m_pHardwareFunction->WriteFlash(bySector[nSectorIndex], CONTROL_COUNT_PAGE, (BYTE)dwOffset, sizeof(byControllerStatus), byData);
			if (0 != nValue)
			{
				++nRewriteTimes;
				bWriteSuccess = FALSE;
				break;
			}

		} while (!bWriteSuccess && 2 >= nRewriteTimes);
		if (!bWriteSuccess)
		{
			break;
		}
	}
	if (!bWriteSuccess)
	{
		return -3;
	}
	m_usChannelCount = usChannelCount;
	InitController();
	return 0;
}

// 
// 
// int CBoard::SetDynamicLoad(const std::vector<USHORT>& vecChannel, BOOL bEnable, double dIOH, double dIOL, double dVTValue, double dClmapHigh, double dClampLow)
// {
// 	BOOL bHaveChannel = FALSE;
// 	int nRetVal = 0;
// 	m_ChannelClassifiy.SetChannel(vecChannel);
// 	vector<USHORT> vecControllerChannel;
// 
// 	for (auto& Controller : m_mapController)
// 	{
// 		if (nullptr == Controller.second)
// 		{
// 			continue;
// 		}
// 		m_ChannelClassifiy.GetChannel(Controller.first, vecControllerChannel);
// 		if (0 == vecControllerChannel.size())
// 		{
// 			continue;
// 		}
// 		bHaveChannel = TRUE;
// 		nRetVal = Controller.second->SetDynamicLoad(vecControllerChannel, bEnable, dIOH, dIOL, dVTValue, dClmapHigh, dClampLow);
// 		if (0 != nRetVal)
// 		{
// 			switch (nRetVal)
// 			{
// 			case -2:
// 				///<The output current is over range
// 				nRetVal = -2;
// 				break;
// 			case -3:
// 				///<The VT is over range
// 				nRetVal = -3;
// 				break;
// 			case -4:
// 				///<The clamp is over range
// 				nRetVal = -4;
// 				break;
// 			default:
// 				break;
// 			}
// 			break;
// 		}
// 	}
// 	if (!bHaveChannel)
// 	{
// 		return -1;
// 	}
// 	return nRetVal;
// }
// 
// int CBoard::GetDynamicLoad(USHORT usChannel, BOOL& bEnable, double& dIOH, double& dIOL)
// {
// 	if (DCM400_MAX_CHANNELS_PER_BOARD  <= usChannel)
// 	{
// 		return -1;
// 	}
// 	auto iterController = m_mapController.find(usChannel / DCM400_CHANNELS_PER_CONTROL);
// 	if (m_mapController.end() == iterController || nullptr == iterController->second)
// 	{
// 		///<The controller is not existed
// 		return -2;
// 	}
// 	int nRetVal = 0;
// 	nRetVal = iterController->second->GetDynamicLoad(usChannel % DCM400_CHANNELS_PER_CONTROL, bEnable, dIOH, dIOL);
// 
// 	if (0 != nRetVal)
// 	{
// 		///<No will happen
// 	}
// 	return 0;
// }
// 
// int CBoard::SetPMUMode(const std::vector<USHORT>& vecChannel, PMU_MODE PMUMode, PMU_IRANGE Range, double dSetValue, double dClmapHigh, double dClampLow)
// {
// 	BOOL bHaveChannel = FALSE;
// 	int nRetVal = 0;
// 	m_ChannelClassifiy.SetChannel(vecChannel);
// 	vector<USHORT> vecControllerChannel;
// 
// 	for (auto& Controller : m_mapController)
// 	{
// 		if (nullptr == Controller.second)
// 		{
// 			continue;
// 		}
// 		m_ChannelClassifiy.GetChannel(Controller.first, vecControllerChannel);
// 		if (0 == vecControllerChannel.size())
// 		{
// 			continue;
// 		}
// 		bHaveChannel = TRUE;
// 		Controller.second->SetPMUMode(vecControllerChannel, PMUMode, Range, dSetValue, dClmapHigh, dClampLow);
// 	}
// 	if (!bHaveChannel)
// 	{
// 		return -1;
// 	}
// 	return nRetVal;
// }
// 
// void CBoard::GetPMUClampChannel(const std::vector<USHORT>& vecChannel, map<USHORT, UCHAR>& mapClampChannel)
// {
// 	mapClampChannel.clear();
// 
// 	m_ChannelClassifiy.SetChannel(vecChannel);
// 	vector<USHORT> vecControllerChannel;
// 	map<USHORT, UCHAR> mapControllerClamp;
// 	for (auto& Controller : m_mapController)
// 	{
// 		if (nullptr == Controller.second)
// 		{
// 			continue;
// 		}
// 		m_ChannelClassifiy.GetChannel(Controller.first, vecControllerChannel);
// 		if (0 == vecControllerChannel.size())
// 		{
// 			continue;
// 		}
// 		Controller.second->GetClampChannel(vecControllerChannel, mapControllerClamp);
// 		if (0 != mapControllerClamp.size())
// 		{
// 			USHORT usChannelOffset = DCM400_CHANNELS_PER_CONTROL * Controller.first;
// 			for (auto& ClampChannel : mapControllerClamp)
// 			{
// 				mapClampChannel.insert(make_pair(ClampChannel.first + usChannelOffset, ClampChannel.second));
// 			}
// 		}
// 	}
// }
// 
// int CBoard::EnablePMUClampFlag(USHORT usChannel, BOOL bEnable)
// {
// 	if (DCM400_MAX_CHANNELS_PER_BOARD <= usChannel)
// 	{
// 		return -1;
// 	}
// 	auto iterController = m_mapController.find(usChannel / DCM400_CHANNELS_PER_CONTROL);
// 	if (m_mapController.end() == iterController)
// 	{
// 		return -2;
// 	}
// 	vector<USHORT> vecChannel;
// 	vecChannel.push_back(usChannel % DCM400_CHANNELS_PER_CONTROL);
// 	iterController->second->EnablePMUClampFlag(vecChannel, bEnable);
// 	return 0;
// }
// 
// void CBoard::EnableAllPMUClampFlag()
// {
// 	vector<USHORT> vecChannel;
// 	for (USHORT usChannel = 0; usChannel < DCM400_CHANNELS_PER_CONTROL;++usChannel)
// 	{
// 		vecChannel.push_back(usChannel);
// 	}
// 	for (auto& Control : m_mapController)
// 	{
// 		if (nullptr == Control.second)
// 		{
// 			continue;
// 		}
// 		Control.second->EnablePMUClampFlag(vecChannel, TRUE);
// 	}
// }
// 
// int CBoard::PMUMeasure(const std::vector<USHORT>& vecChannel, int nSampleTimes, double dSamplePeriod)
// {
// 	m_setWaitPMUStartControlller.clear();
// 	BOOL bHaveChannel = FALSE;
// 	int nRetVal = 0;
// 	m_ChannelClassifiy.SetChannel(vecChannel);
// 	vector<USHORT> vecControllerChannel;
// 
// 	BOOL bFail = FALSE;
// 	for (auto& Controller : m_mapController)
// 	{
// 		if (nullptr == Controller.second)
// 		{
// 			continue;
// 		}
// 		m_ChannelClassifiy.GetChannel(Controller.first, vecControllerChannel);
// 		if (0 == vecControllerChannel.size())
// 		{
// 			continue;
// 		}
// 		bHaveChannel = TRUE;
// 		nRetVal = Controller.second->PMUMeasure(vecControllerChannel, nSampleTimes, dSamplePeriod);
// 		if (0 != nRetVal)
// 		{
// 			bFail = TRUE;
// 			switch (nRetVal)
// 			{
// 			case -1:
// 				///<Measurement error
// 				nRetVal = -2;
// 				break;
// 			case -3:
// 				///<Wait for stopping and start another
// 				m_setWaitPMUStartControlller.insert(Controller.first);
// 				bFail = FALSE;
// 				break;
// 			default:
// 				break;
// 			}
// 			if (bFail)
// 			{
// 				break;
// 			}
// 		}
// 	}
// 	if (!bHaveChannel)
// 	{
// 		return -1;
// 	}
// 	if (bFail)
// 	{
// 		return nRetVal;
// 	}
// 	if (0 != m_setWaitPMUStartControlller.size())
// 	{
// 		return -3;
// 	}
// 	return 0;
// }
// 
// int CBoard::StartPMU()
// {
// 	BOOL bStartPMU = FALSE;
// 	int nRetVal = 0;
// 	auto iterController = m_mapController.begin();
// 	for (auto Controller : m_setWaitPMUStartControlller)
// 	{
// 		iterController = m_mapController.find(Controller);
// 		if (m_mapController.end() == iterController || nullptr == iterController->second)
// 		{
// 			continue;
// 		}
// 		bStartPMU = TRUE;
// 		nRetVal = iterController->second->StartPMU();
// 		if (0 != nRetVal)
// 		{
// 			nRetVal = -1;
// 		}
// 	}
// 	if (!bStartPMU)
// 	{
// 		nRetVal = -1;
// 	}
// 	return nRetVal;
// }
// 
// int CBoard::WaitPMUFinish()
// {
// 	BOOL bStartPMU = FALSE;
// 	int nRetVal = 0;
// 	auto iterController = m_mapController.begin();
// 	for (auto Controller : m_setWaitPMUStartControlller)
// 	{
// 		iterController = m_mapController.find(Controller);
// 		if (m_mapController.end() == iterController || nullptr == iterController->second)
// 		{
// 			continue;
// 		}
// 		bStartPMU = TRUE;
// 		nRetVal = iterController->second->WaitPMUFinish();
// 		if (0 != nRetVal)
// 		{
// 			nRetVal = -1;
// 		}
// 	}
// 	if (!bStartPMU)
// 	{
// 		nRetVal = -1;
// 	}
// 	return nRetVal;
// }
// 
// double CBoard::GetPMUMeasureResult(USHORT usChannel, int nSampleTimes)
// {
// 	if (DCM400_MAX_CHANNELS_PER_BOARD < usChannel)
// 	{
// 		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CHANNEL_OVER_RANGE);
// 		m_pAlarm->SetAlarmType(CDriverAlarm::ALARM_TYPE::PARAMETER_OVERRANGE);
// 		m_pAlarm->SetAlarmMsg("The channel(S%d_%d) is over rang[0, %d].", m_bySlotNo, usChannel, DCM400_MAX_CHANNELS_PER_BOARD - 1);
// 		return MAX_MEASURE_VALUE;
// 	}
// 	BYTE byControllerIndex = usChannel / DCM400_CHANNELS_PER_CONTROL;
// 	auto iterController = m_mapController.find(byControllerIndex);
// 	if (m_mapController.end() == iterController || nullptr == iterController->second)
// 	{
// 		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_CHANNEL_NOT_EXISTED);
// 		m_pAlarm->SetAlarmMsg("The channel(S%d_%d) is not existed.", m_bySlotNo, usChannel);
// 		m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmWarning);
// 		return MAX_MEASURE_VALUE;
// 	}
// 	return iterController->second->GetPMUMeasureResult(usChannel % DCM400_CHANNELS_PER_CONTROL, nSampleTimes);
// }
// 
// double CBoard::GetPMUMode(USHORT usChannel, PMU_MODE& PMUMode, PMU_IRANGE& PMURange)
// {
// 	if (DCM400_MAX_CHANNELS_PER_BOARD <= usChannel)
// 	{
// 		return 1e16;
// 	}
// 	BYTE byControllerIndex = usChannel / DCM400_CHANNELS_PER_CONTROL;
// 	auto iterController = m_mapController.find(byControllerIndex);
// 	if (m_mapController.end() == iterController || nullptr == iterController->second)
// 	{
// 		return MAX_MEASURE_VALUE;
// 	}
// 	double dValue = iterController->second->GetPMUMode(usChannel % DCM400_CHANNELS_PER_CONTROL, PMUMode, PMURange);
// 	if (1e10 < dValue)
// 	{
// 		return MAX_MEASURE_VALUE;
// 	}
// 	return dValue;
// }
// 
// int CBoard::SetVTMode(const std::vector<USHORT>& vecChannel, double dVTVoltValue, VT_MODE VTMode)
// {
// 	BOOL bHaveChannel = FALSE;
// 	int nRetVal = 0;
// 	m_ChannelClassifiy.SetChannel(vecChannel);
// 	vector<USHORT> vecControllerChannel;
// 
// 	for (auto& Controller : m_mapController)
// 	{
// 		if (nullptr == Controller.second)
// 		{
// 			continue;
// 		}
// 		m_ChannelClassifiy.GetChannel(Controller.first, vecControllerChannel);
// 		if (0 == vecControllerChannel.size())
// 		{
// 			continue;
// 		}
// 		bHaveChannel = TRUE;
// 		nRetVal = Controller.second->SetVTMode(vecControllerChannel, dVTVoltValue, VTMode);
// 		if (0 != nRetVal)
// 		{
// 			switch (nRetVal)
// 			{
// 			case -2:
// 				///<The voltage is over range
// 				nRetVal = -1;
// 				break;
// 			case -3:
// 				///<The mode is error
// 				nRetVal = -2;
// 				break;
// 			default:
// 				break;
// 			}
// 			break;
// 		}
// 	}
// 	if (!bHaveChannel)
// 	{
// 		nRetVal = -3;
// 	}
// 	return nRetVal;
// }
// 
// int CBoard::GetVTMode(USHORT usChannel, VT_MODE& VTMode)
// {
// 	if (DCM400_MAX_CHANNELS_PER_BOARD <= usChannel)
// 	{
// 		return -1;
// 	}
// 	auto iterController = m_mapController.find(usChannel / DCM400_CHANNELS_PER_CONTROL);
// 	if (m_mapController.end() == iterController || nullptr == iterController->second)
// 	{
// 		///<The controller is not existed
// 		return -2;
// 	}
// 	int nRetVal = 0;
// 	nRetVal = iterController->second->GetVTMode(usChannel % DCM400_CHANNELS_PER_CONTROL, VTMode);
// 
// 	if (0 != nRetVal)
// 	{
// 		///<No will happen
// 	}
// 	return 0;
// }
// 
// int CBoard::SetPrereadLine(const std::vector<USHORT>& vecChannel, UINT uStartLine, UINT uLineCount)
// {
// 	set<BYTE> setControllerIndex;
// 	m_ChannelClassifiy.GetController(vecChannel, setControllerIndex);
// 	BOOL bNoBoard = TRUE;
// 	int nRetVal = 0;
// 
// 	for (auto Controller : setControllerIndex)
// 	{
// 		auto iterController = m_mapController.find(Controller);
// 		if (m_mapController.end() == iterController || nullptr == iterController->second)
// 		{
// 			continue;
// 		}
// 		bNoBoard = FALSE;
// 		nRetVal = iterController->second->SetPrereadLine(uStartLine, uLineCount);
// 		if (0 != nRetVal)
// 		{
// 			switch (nRetVal)
// 			{
// 			case -1:
// 				///<The prread line count have reached to maximum limited
// 				nRetVal = -1;
// 				break;
// 			case -2:
// 				///<The start line is over range
// 				nRetVal = -2;
// 				break;
// 			case -3:
// 				///<The line count is over range
// 				nRetVal = -3;
// 				break;
// 			case -4:
// 				///<Allocate memory fail
// 				///<The line count is over maximum line count limited
// 				nRetVal = -4;
// 			default:
// 				break;
// 			}
// 			break;
// 		}
// 	}
// 	if (bNoBoard)
// 	{
// 		nRetVal = -6;
// 	}
// 	return nRetVal;
// }
// 
// int CBoard::SetLineInfo(const std::vector<USHORT>& vecChannel, UINT uStartLine, UINT uLineCount)
// {
// 	m_vecModifiedChannel = vecChannel;
// 
// 	set<BYTE> setControllerIndex;
// 	m_ChannelClassifiy.GetController(vecChannel, setControllerIndex);
// 	BOOL bNoBoard = TRUE;
// 	int nRetVal = 0;
// 
// 	for (auto Controller : setControllerIndex)
// 	{
// 		auto iterController = m_mapController.find(Controller);
// 		if (m_mapController.end() == iterController || nullptr == iterController->second)
// 		{
// 			continue;
// 		}
// 		bNoBoard = FALSE;
// 		nRetVal = iterController->second->SetLineInfo(uStartLine, uLineCount);
// 		if (0 != nRetVal)
// 		{
// 			switch (nRetVal)
// 			{
// 			case -1:
// 				///<The start line is over range
// 				nRetVal = -1;
// 				break;
// 			case -2:
// 				///<The line count is over range
// 				nRetVal = -2;
// 				break;
// 			case -3:
// 				///<Allocate memory fail
// 				nRetVal = -3;
// 				break;
// 			default:
// 				break;
// 			}
// 			break;
// 		}
// 	}
// 	if (0 != nRetVal)
// 	{
// 		m_vecModifiedChannel.clear();
// 	}
// 	if (bNoBoard)
// 	{
// 		nRetVal = -4;
// 	}
// 	return nRetVal;
// }
// 
// int CBoard::SetWaveData(const std::vector<USHORT>& vecChannel, const BYTE* pbyData)
// {	
// 	int nRetVal = 0;
// 	BOOL bNoBoard = TRUE;
// 	m_ChannelClassifiy.SetChannel(vecChannel);
// 	vector<USHORT> vecControllerChannel;
// 
// 	for (auto& Controller : m_mapController)
// 	{
// 		if (nullptr == Controller.second)
// 		{
// 			continue;
// 		}
// 		m_ChannelClassifiy.GetChannel(Controller.first, vecControllerChannel);
// 		if (0 == vecControllerChannel.size())
// 		{
// 			continue;
// 		}
// 		bNoBoard = FALSE;
// 		nRetVal = Controller.second->SetWaveData(vecControllerChannel, pbyData);
// 		if (0 != nRetVal)
// 		{
// 			switch (nRetVal)
// 			{
// 			case -1:
// 				///<Not set the channel line information
// 				nRetVal = -1;
// 				break;
// 			case -2:
// 				///<Not happened
// 				break;
// 			case-3:
// 				///<The point of data is nullptr
// 				nRetVal = -2;
// 				break;
// 			default:
// 				break;
// 			}
// 			break;
// 		}
// 	}
// 
// 	if (bNoBoard)
// 	{
// 		nRetVal = -3;
// 	}
// 	return nRetVal;
// }
// 
// int CBoard::WriteData()
// {
// 	set<BYTE> setController;
// 	m_ChannelClassifiy.GetController(m_vecModifiedChannel, setController);
// 	BOOL bNoBoard = TRUE;
// 	for (auto Controller : setController)
// 	{
// 		auto iterController = m_mapController.find(Controller);
// 		if (m_mapController.end() != iterController && nullptr != iterController->second)
// 		{
// 			bNoBoard = FALSE;
// 			iterController->second->WriteData();
// 		}
// 	}
// 	if (bNoBoard)
// 	{
// 		return -1;
// 	}
// 	return 0;
// }
// 
// int CBoard::GetVector(BYTE byControllerIndex, BOOL bBRAM, UINT uStartLine, UINT uLineCount, char(*lpszPattern)[17])
// {
// 	if (DCM400_MAX_CONTROLLERS_PRE_BOARD <= byControllerIndex)
// 	{
// 		return -1;
// 	}
// 	auto iterController = m_mapController.find(byControllerIndex);
// 	if (m_mapController.end() == iterController || nullptr == iterController->second)
// 	{
// 		return -2;
// 	}
// 	int nRetVal = iterController->second->GetPattern(bBRAM, uStartLine, uLineCount, lpszPattern);
// 	if (0 != nRetVal)
// 	{
// 		switch (nRetVal)
// 		{
// 		case -1:
// 			///<Start line is over range
// 			nRetVal = -3;
// 			break;
// 		case -2:
// 			///<The line count is over range
// 			nRetVal = -4;
// 			break;
// 		case -3:
// 			///<The point of the pattern is nullptr
// 			nRetVal = -5;
// 			break;
// 		case -4:
// 			///<Allocate memory fail
// 			nRetVal = -6;
// 			break;
// 		default:
// 			break;
// 		}
// 	}
// 	return nRetVal;
// }
// 
// int CBoard::GetMemory(BYTE byControllerIndex, BOOL bBRAM, DATA_TYPE DataType, UINT uStartLine, UINT uLineCount, USHORT* pusData)
// {
// 	if (DCM400_MAX_CONTROLLERS_PRE_BOARD <= byControllerIndex)
// 	{
// 		return -1;
// 	}
// 	auto iterController = m_mapController.find(byControllerIndex);
// 	if (m_mapController.end() == iterController || nullptr == iterController->second)
// 	{
// 		return -2;
// 	}
// 	int nRetVal = 0;
// 	nRetVal = iterController->second->GetMemory(bBRAM, DataType, uStartLine, uLineCount, pusData);
// 	if (0 != nRetVal)
// 	{
// 		switch (nRetVal)
// 		{
// 		case -1:
// 			///<The memory type is error, will not happened
// 			break;
// 		case -2:
// 			///<The data type is not supported
// 			nRetVal = -3;
// 			break;
// 		case -3:
// 			///<The start line is over range
// 			nRetVal = -4;
// 			break;
// 		case -4:
// 			///<The data count is over range
// 			nRetVal = -5;
// 			break;
// 		case -5:
// 			///<The point of data is nullptr or the read data count is 0
// 			nRetVal = -6;
// 			break;
// 		default:
// 			break;
// 		}
// 	}
// 	return nRetVal;
// }
// 
// int CBoard::SetMemory(USHORT usChannel, BOOL bRAM, DATA_TYPE DataType, UINT uStartLine, UINT uLineCount, BYTE* pbyData)
// {
// 	if (DCM400_MAX_CHANNELS_PER_BOARD <= usChannel)
// 	{
// 		return -1;
// 	}
// 	auto iterController = m_mapController.find(usChannel / DCM400_CHANNELS_PER_CONTROL);
// 	if (m_mapController.end() == iterController || nullptr == iterController->second)
// 	{
// 		return -2;
// 	}
// 	int nRetVal = iterController->second->SetMemory(usChannel % DCM400_CHANNELS_PER_CONTROL, bRAM, DataType, uStartLine, uLineCount, pbyData);
// 	if (0 != nRetVal)
// 	{
// 		switch (nRetVal)
// 		{
// 		case -1:
// 			///<Not happened
// 			break;
// 		case -2:
// 			///<The data type is not supported
// 			nRetVal = -3;
// 			break;
// 		case -3:
// 			///<Allocate memory fail
// 			nRetVal = -4;
// 			break;
// 		case -4:
// 			///<The start line is over range
// 			nRetVal = -5;
// 			break;
// 		case -5:
// 			///<The line count is over range
// 			nRetVal = -6;
// 			break;
// 		case -6:
// 			///<The line count is 0
// 			nRetVal = -7;
// 			break;
// 		case -7:
// 			///<The point of data is nullptr
// 			nRetVal = -8;
// 			break;
// 		default:
// 			break;
// 		}
// 	}
// 
// 	return nRetVal;
// }
// 
// int CBoard::GetVector(USHORT usChannel, BOOL bBRAM, UINT uStartLine, UINT uLineCount, char* lpszPattern)
// {
// 	if (DCM400_MAX_CHANNELS_PER_BOARD <= usChannel)
// 	{
// 		return -1;
// 	}
// 	BYTE byController = usChannel / DCM400_CHANNELS_PER_CONTROL;
// 	
// 	auto iterController = m_mapController.find(byController);
// 	if (m_mapController.end() == iterController || nullptr == iterController->second)
// 	{
// 		return -2;
// 	}
// 
// 	char(*pControllerPatthern)[17] = nullptr;
// 	try
// 	{
// 		pControllerPatthern = new char[uLineCount][17];
// 	}
// 	catch (const std::exception&)
// 	{
// 		return -3;
// 	}
// 	int nRetVal = GetVector(byController, bBRAM, uStartLine, uLineCount, pControllerPatthern);
// 	if (0 != nRetVal)
// 	{
// 		switch (nRetVal)
// 		{
// 		case -2:
// 			///<Channel is not existed
// 			nRetVal = -2;
// 			break;
// 		case -3:
// 			///<Start line is over range
// 			nRetVal = -4;
// 			break;
// 		case -4:
// 			///<Line count is over range
// 			nRetVal = -5;
// 			break;
// 		case -5:
// 			///<The point of pattern is nullptr
// 			nRetVal = -6;
// 			break;
// 		case -6:
// 			///<Allocate memory fail
// 			nRetVal = -3;
// 			break;
// 		default:
// 			break;
// 		}
// 	}
// 	if (0 != nRetVal)
// 	{
// 		if (nullptr != pControllerPatthern)
// 		{
// 			delete[] pControllerPatthern;
// 			pControllerPatthern = nullptr;
// 		}
// 		return nRetVal;
// 	}
// 	USHORT usByteChannel = usChannel % DCM400_CHANNELS_PER_CONTROL;
// 	for (UINT uLineIndex = 0; uLineIndex < uLineCount; ++uLineIndex)
// 	{
// 		lpszPattern[uLineIndex] = pControllerPatthern[uLineIndex][usByteChannel];
// 	}
// 
// 	if (nullptr != pControllerPatthern)
// 	{
// 		delete[] pControllerPatthern;
// 		pControllerPatthern = nullptr;
// 	}
// 	return 0;
// }
// 
// double CBoard::GetPinLevel(USHORT usChannel, LEVEL_TYPE LevelType)
// {
// 	if (DCM400_MAX_CHANNELS_PER_BOARD <= usChannel)
// 	{
// 		return 1e16;
// 	}
// 	BYTE byControllerIndex = usChannel / DCM400_CHANNELS_PER_CONTROL;
// 	auto iterController = m_mapController.find(byControllerIndex);
// 	if (m_mapController.end() == iterController || nullptr == iterController->second)
// 	{
// 		return 1e16;
// 	}
// 	double dValue = iterController->second->GetPinLevel(usChannel % DCM400_CHANNELS_PER_CONTROL, LevelType);
// 	if (10000 < dValue)
// 	{
// 		dValue = 1e16;
// 	}
// 	return dValue;
// }
// 
// int CBoard::SetTMUUnitChannel(std::vector<USHORT> vecChannel, BYTE byUnitIndex)
// {
// 	int nRetVal = 0;
// 	BOOL bNoBoard = TRUE;
// 	m_ChannelClassifiy.SetChannel(vecChannel);
// 	vector<USHORT> vecControllerChannel;
// 	for (auto& Controller : m_mapController)
// 	{
// 		if (nullptr == Controller.second)
// 		{
// 			continue;
// 		}
// 		m_ChannelClassifiy.GetChannel(Controller.first, vecControllerChannel);
// 		USHORT usChannelCount = vecControllerChannel.size();
// 		if (0 == usChannelCount)
// 		{
// 			continue;
// 		}
// 		else if (1 != usChannelCount)
// 		{
// 			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_TMU_UNIT_CONNECT_CHANNEL_OVER_RANGE);
// 			m_pAlarm->SetAlarmMsg("More than 1 channel in board S%d is connected to TMU unit %d.", m_bySlotNo, byUnitIndex + 1);
// 			return -1;
// 		}
// 		bNoBoard = FALSE;
// 		nRetVal = Controller.second->SetTMUUnitChannel(vecControllerChannel[0] % DCM400_CHANNELS_PER_CONTROL, byUnitIndex);
// 		if (0 != nRetVal)
// 		{
// 			switch (nRetVal)
// 			{
// 			case -1:
// 				///<Channel number is over range, not will happen
// 				break;
// 			case -2:
// 				///<The unit index is over range
// 				nRetVal = -2;
// 				break;
// 			default:
// 				break;
// 			}
// 			break;
// 		}
// 	}
// 	if (bNoBoard)
// 	{
// 		nRetVal = -3;
// 	}
// 	return nRetVal;
// }
// 
// int CBoard::GetTMUConnectUnit(USHORT usChannel)
// {
// 	if (DCM400_MAX_CHANNELS_PER_BOARD <= usChannel)
// 	{
// 		return -1;
// 	}
// 	auto iterController = m_mapController.find(usChannel / DCM400_CHANNELS_PER_CONTROL);
// 	if (m_mapController.end() == iterController || nullptr == iterController->second)
// 	{
// 		return -2;
// 	}
// 	int nRetVal = iterController->second->GetTMUConnectUnit(usChannel % DCM400_CHANNELS_PER_CONTROL);
// 	if (0 > nRetVal)
// 	{
// 		switch (nRetVal)
// 		{
// 		case -1:
// 			///<The channel is over range, not will happen
// 			break;
// 		case -2:
// 			///<The channel is not connected to any TMU unit
// 			nRetVal = -3;
// 			break;
// 		default:
// 			break;
// 		}
// 	}
// 	return nRetVal;
// }
// 
// int CBoard::SetTMUParam(const std::vector<USHORT>& vecChannel, BOOL bRaiseTriggerEdge, UINT uHoldOffTime, UINT uHolfOffNum, BYTE bySpecifiedUnit)
// {
// 	int nRetVal = 0;
// 	BOOL bNoBoard = TRUE;
// 	m_ChannelClassifiy.SetChannel(vecChannel);
// 	vector<USHORT> vecControllerChannel;
// 	for (auto& Controller : m_mapController)
// 	{
// 		if (nullptr == Controller.second)
// 		{
// 			continue;
// 		}
// 		m_ChannelClassifiy.GetChannel(Controller.first, vecControllerChannel);
// 		if (0 == vecControllerChannel.size())
// 		{
// 			continue;
// 		}
// 		bNoBoard = FALSE;
// 		nRetVal = Controller.second->SetTMUParam(vecControllerChannel, bRaiseTriggerEdge, uHoldOffTime, uHolfOffNum, bySpecifiedUnit);
// 		if (0 != nRetVal)
// 		{
// 			switch (nRetVal)
// 			{
// 			case -1:
// 				///<Channel number is over range, not will happen
// 				break;
// 			case -2:
// 				///<The channel count is over range
// 				nRetVal = -2;
// 				break;
// 			case -3:
// 				///<The unit specified is over range
// 				nRetVal = -1;
// 				break;
// 			case -4:
// 				///<The channel is not connected to unit
// 				nRetVal = -3;
// 				break;
// 			default:
// 				break;
// 			}
// 			break;
// 		}
// 	}
// 	if (bNoBoard)
// 	{
// 		nRetVal = -4;
// 	}
// 
// 	return nRetVal;
// }
// 
// int CBoard::GetTMUUnitParameter(BYTE byControllerIndex, BYTE byTMUUnitIndex, BOOL& bRaiseTriggerEdge, USHORT& usHoldOffTime, USHORT& usHoldOffNum)
// {
// 	if (DCM400_MAX_CONTROLLERS_PRE_BOARD <= byControllerIndex)
// 	{
// 		return -1;
// 	}
// 	auto iterController = m_mapController.find(byControllerIndex);
// 	if (m_mapController.end() == iterController || nullptr == iterController->second)
// 	{
// 		return -2;
// 	}
// 	int nRetVal = iterController->second->GetTMUUnitParameter(byTMUUnitIndex, bRaiseTriggerEdge, usHoldOffTime, usHoldOffNum);
// 	if (0 != nRetVal)
// 	{
// 		switch (nRetVal)
// 		{
// 		case -1:
// 			///<The unit is over range
// 			nRetVal = -3;
// 			break;
// 		default:
// 			break;
// 		}
// 	}
// 	return 0;
// }
// 
// int CBoard::GetTMUParameter(USHORT usChannel, BOOL& bRaiseTriggerEdge, USHORT& usHoldOffTime, USHORT& usHoldOffNum)
// {
// 	if (DCM400_MAX_CHANNELS_PER_BOARD <= usChannel)
// 	{
// 		return -1;
// 	}
// 	auto iterController = m_mapController.find(usChannel / DCM400_CHANNELS_PER_CONTROL);
// 	if (m_mapController.end() == iterController || nullptr == iterController->second)
// 	{
// 		return -2;
// 	}
// 	int nRetVal = iterController->second->GetTMUParameter(usChannel % DCM400_CHANNELS_PER_CONTROL, bRaiseTriggerEdge, usHoldOffTime, usHoldOffNum);
// 	if (0 != nRetVal)
// 	{
// 		switch (nRetVal)
// 		{
// 		case -1:
// 			///<Channel is over range, not will happened
// 			break;
// 		case -2:
// 			///<The channel is not connected to TMU unit
// 			nRetVal = -3;
// 			break;
// 		default:
// 			break;
// 		}
// 	}
// 	return nRetVal;
// }
// 
// int CBoard::TMUMeasure(const std::vector<USHORT>& vecChannel, TMU_MEAS_MODE MeasMode, UINT uSampleNum, double dTimeout)
// {
// 	int nRetVal = 0;
// 	BOOL bNoBoard = TRUE;
// 	m_ChannelClassifiy.SetChannel(vecChannel);
// 	vector<USHORT> vecControllerChannel;
// 	for (auto& Controller : m_mapController)
// 	{
// 		if (nullptr == Controller.second)
// 		{
// 			continue;
// 		}
// 		m_ChannelClassifiy.GetChannel(Controller.first, vecControllerChannel);
// 		if (0 == vecControllerChannel.size())
// 		{
// 			continue;
// 		}
// 		bNoBoard = FALSE;
// 		nRetVal = Controller.second->TMUMeasure(vecControllerChannel, MeasMode, uSampleNum, dTimeout);
// 		if (0 != nRetVal)
// 		{
// 			switch (nRetVal)
// 			{
// 			case -1:
// 				///<Channel number is over range, not will happen
// 				break;
// 			case -2:
// 				///<The channel is not connected to unit
// 				nRetVal = -1;
// 				break;
// 			case -3:
// 				///<The measurement mode is not supported
// 				nRetVal = -2;
// 				break;
// 			default:
// 				break;
// 			}
// 			break;
// 		}
// 	}
// 	if (bNoBoard)
// 	{
// 		nRetVal = -3;
// 	}
// 
// 	return nRetVal;
// }
// 
// int CBoard::GetTMUMeasure(USHORT usChannel, TMU_MEAS_MODE& MeasMode, UINT& uSampleNum, double& dTimeout)
// {
// 	if (DCM400_MAX_CHANNELS_PER_BOARD <= usChannel)
// 	{
// 		return -1;
// 	}
// 	auto iterController = m_mapController.find(usChannel / DCM400_CHANNELS_PER_CONTROL);
// 	if (m_mapController.end() == iterController || nullptr == iterController->second)
// 	{
// 		return -2;
// 	}
// 	int nRetVal = iterController->second->GetTMUMeasure(usChannel % DCM400_CHANNELS_PER_CONTROL, MeasMode, uSampleNum, dTimeout);
// 	if (0 != nRetVal)
// 	{
// 		switch (nRetVal)
// 		{
// 		case -1:
// 			///<Channel is over range, not will happened
// 			break;
// 		case -2:
// 			///<The channel is not connected to TMU unit
// 			nRetVal = -3;
// 			break;
// 		default:
// 			break;
// 		}
// 	}
// 	return nRetVal;
// }
// 
// double CBoard::GetTMUMeasureResult(USHORT usChannel, TMU_MEAS_TYPE MeasType, int& nErrorCode)
// {
// 	if (DCM400_MAX_CHANNELS_PER_BOARD <= usChannel)
// 	{
// 		nErrorCode = -1;
// 		return TMU_ERROR;
// 	}
// 	BOOL bNotBoard = TRUE;
// 	auto iterController = m_mapController.find(usChannel / DCM400_CHANNELS_PER_CONTROL);
// 	if (m_mapController.end() == iterController || nullptr == iterController->second)
// 	{
// 		nErrorCode = -2;
// 		return TMU_ERROR;
// 	}
// 	double dMeasResult = iterController->second->GetTMUMeasureResult(usChannel % DCM400_CHANNELS_PER_CONTROL, MeasType, nErrorCode);
// 	if (0 != nErrorCode)
// 	{
// 		switch (nErrorCode)
// 		{
// 		case -1:
// 			///<The channel is over range, not will happen
// 			break;
// 		case -2:
// 			///<The channel is not connect to any TMU unit
// 			nErrorCode = -3;
// 			break;
// 		case -3:
// 			///<The measurement type is not supported
// 			nErrorCode = -4;
// 			break;
// 		case -4:
// 			///<The measurement type is not measured before
// 			nErrorCode = -5;
// 			break;
// 		case -5:
// 			///<The measurement is not stop in timeout
// 			nErrorCode = -6;
// 			break;
// 		case -6:
// 			///<The TMU measurement is timeout
// 			nErrorCode = -7;
// 			break;
// 		case -7:
// 			///<The bind unit of measurement is not stop in timeout
// 			nErrorCode = -8;
// 			break;
// 		case -8:
// 			///<The bind unit is timeout
// 			nErrorCode = -9;
// 			break;
// 		case -9:
// 			///<The edge measurement error
// 			nErrorCode = -10;
// 			break;
// 		default:
// 			break;
// 		}
// 	}
// 	return dMeasResult;
// }
// 
// int CBoard::SetTriggerOut(USHORT usChannel)
// {
// 	if (DCM400_MAX_CHANNELS_PER_BOARD <= usChannel)
// 	{
// 		return -1;
// 	}
// 	auto iterController = m_mapController.find(usChannel / DCM400_CHANNELS_PER_CONTROL);
// 	if (m_mapController.end() == iterController || nullptr == iterController->second)
// 	{
// 		return -2;
// 	}
// 	int nRetVal = iterController->second->SetTriggerOut(usChannel % DCM400_CHANNELS_PER_CONTROL);
// 	if (0 != nRetVal)
// 	{
// 		///<Not will happened
// 	}
// 	return 0;
// }
// 
// int CBoard::SetFailSyn(const std::vector<CChannelGroup>& vecFailSynChannel, int& nConfictIndex)
// {
// 	map<BYTE, BYTE> mapFailSyn;
// 	int nFailSynType = m_pHardwareFunction->GetFailSynType();
// 	if (0 == nFailSynType)
// 	{
// 		int nSize = vecFailSynChannel.size();
// 		if (0 == nSize)
// 		{
// 			for (auto& Controller : m_mapController)
// 			{
// 				if (0 == mapFailSyn.size())
// 				{
// 					mapFailSyn.insert(make_pair(Controller.first, 0));
// 				}
// 				mapFailSyn.begin()->second |= 1 << Controller.first;
// 			}
// 			m_pHardwareFunction->SetFailSyn(mapFailSyn);
// 			return 0;
// 		}
// 		else if (1 < nSize)
// 		{
// 			///<Not supported
// 			return -1;
// 		}
// 	}
// 
// 	set<BYTE> setSynController;
// 	vector<USHORT> vecChannel;
// 	int nIndex = 0;
// 	CBoardChannelClassify Classify;
// 	set<BYTE> setController;
// 	BOOL bSynControllerConfict = FALSE;
// 	for (const auto& ChannelGroup : vecFailSynChannel)
// 	{
// 		ChannelGroup.GetChannel(vecChannel);
// 		Classify.GetController(vecChannel, setController);
// 		BYTE bySynController = 0;
// 		for (auto Controller : setController)
// 		{
// 			bySynController |= 1 << Controller;
// 		}
// 		for (auto SynController : setSynController)
// 		{
// 			if (0 != (bySynController & SynController))
// 			{
// 				bSynControllerConfict = TRUE;
// 				break;
// 			}
// 		}
// 		if (bSynControllerConfict)
// 		{
// 			break;
// 		}
// 		++nIndex;
// 	}
// 	if (bSynControllerConfict)
// 	{
// 		///<The synchronous is conflict
// 		nConfictIndex = nIndex;
// 		return -2;
// 	}
// 
// 	for (auto SynController : setSynController)
// 	{
// 		for (BYTE byControllerIndex = 0; byControllerIndex < DCM400_MAX_CONTROLLERS_PRE_BOARD; ++byControllerIndex)
// 		{
// 			if (0 != (SynController >> byControllerIndex))
// 			{
// 				mapFailSyn.insert(make_pair(byControllerIndex, SynController));
// 			}
// 		}
// 	}
// 	int nRetVal = m_pHardwareFunction->SetFailSyn(mapFailSyn);
// 	if (0 != nRetVal)
// 	{
// 		///<Not supported
// 		return -1;
// 	}
// 	return 0;
// }
// 
// int CBoard::GetFailMemoryFilled(USHORT usChannel, BOOL& bBRAMFilled, BOOL& bDRAMFilled)
// {
// 	if (DCM400_MAX_CHANNELS_PER_BOARD <= usChannel)
// 	{
// 		return -1;
// 	}
// 	auto iterController = m_mapController.find(usChannel / DCM400_CHANNELS_PER_CONTROL);
// 	if (m_mapController.end() == iterController || nullptr == iterController->second)
// 	{
// 		return -2;
// 	}
// 	int nRetVal = iterController->second->GetFailMemoryFilled(bBRAMFilled, bDRAMFilled);
// 	if (0 != nRetVal)
// 	{
// 		///<Not will happened
// 		switch (nRetVal)
// 		{
// 		case -1:
// 			nRetVal = -3;
// 			break;
// 		case -2:
// 			nRetVal = -4;
// 			break;
// 		default:
// 			break;
// 		}
// 	}
// 	return nRetVal;
// }
// 
// int CBoard::EnableSaveSelectedFail(const std::vector<USHORT>& vecChannel, BOOL bEnable)
// {
// 	set<BYTE> setController;
// 	m_ChannelClassifiy.GetController(vecChannel, setController);
// 	BOOL bNoBoard = TRUE;
// 	for (auto Controller : setController)
// 	{
// 		auto iterController = m_mapController.find(Controller);
// 		if (m_mapController.end() != iterController && nullptr != iterController->second)
// 		{
// 			bNoBoard = FALSE;
// 			iterController->second->EnableSaveSelectedFail(bEnable);
// 		}
// 	}
// 	if (bNoBoard)
// 	{
// 		return -1;
// 	}
// 	return 0;
// }
// 
// int CBoard::GetInstructionType(const char* lpszInstruction)
// {
// 	auto iterController = m_mapController.begin();
// 	if (m_mapController.end() == iterController || nullptr == iterController->second)
// 	{
// 		CController Controller(m_bySlotNo, 0, m_pAlarm);
// 		return Controller.GetInstructionType(lpszInstruction);
// 	}
// 	return iterController->second->GetInstructionType(lpszInstruction);
// }
// 
// void CBoard::DeleteController(const std::vector<USHORT>& vecChannelLeft)
// {
// 	set<BYTE> setControllerIndex;
// 	m_ChannelClassifiy.GetController(vecChannelLeft, setControllerIndex);
// 	vector<BYTE> vecControllerDelete;
// 	for (auto& Controller : m_mapController)
// 	{
// 		if (setControllerIndex.end() == setControllerIndex.find(Controller.first))
// 		{
// 			vecControllerDelete.push_back(Controller.first);
// 		}
// 	}
// 	for (auto Controller : vecControllerDelete)
// 	{
// 		auto iterController = m_mapController.find(Controller);
// 		if (m_mapController.end() != iterController)
// 		{
// 			m_mapController.erase(iterController);
// 		}
// 	}
// 	if (0 != m_mapController.size())
// 	{
// 		m_pHardwareFunction = m_mapController.begin()->second->GetHardwareFunction();
// 	}
// 	else
// 	{
// 		m_pHardwareFunction = new CHardwareFunction(m_bySlotNo, m_pAlarm);
// 	}
// }
// 
// void CBoard::ClearPreread()
// {
// 	for (auto& Controller : m_mapController)
// 	{
// 		if (nullptr == Controller.second)
// 		{
// 			continue;
// 		}
// 		Controller.second->ClearPreread();
// 	}
// }
// 
void CBoard::InitController()
{
	if (0 == m_mapController.size() && nullptr != m_pHardwareFunction)
	{
		delete m_pHardwareFunction;
		m_pHardwareFunction = nullptr;
	}
	m_pHardwareFunction = nullptr;

	BYTE byControllerCount = m_usChannelCount / DCM400_CHANNELS_PER_CONTROL;
	auto iterController = m_mapController.begin();
	BYTE byControllerIndex = 0;
	for (byControllerIndex = 0; byControllerIndex < byControllerCount;++byControllerIndex)
	{
		iterController = m_mapController.find(byControllerIndex);
		if (m_mapController.end() == iterController || nullptr == iterController->second)
		{
			CController* pController = new CController(m_bySlotNo, byControllerIndex, m_pAlarm);
			if (!pController->IsExist())
			{
				delete pController;
				pController = nullptr;
				if (m_mapController.end() != iterController)
				{
					m_mapController.erase(iterController);
				}
				continue;
			}
			if (m_mapController.end() == iterController)
			{
				m_mapController.insert(make_pair(byControllerIndex, pController));
				iterController = m_mapController.find(byControllerIndex);
			}
			else
			{
				iterController->second = pController;
			}
		}
	}
	for (;byControllerIndex < DCM400_MAX_CONTROLLERS_PRE_BOARD;++byControllerIndex)
	{
		iterController = m_mapController.find(byControllerIndex);
		if (m_mapController.end() != iterController)
		{
			if (nullptr != iterController->second)
			{
				delete iterController->second;
				iterController->second = nullptr;
			}
			m_mapController.erase(iterController);
		}
	}

	if (m_bExisted)
	{
		if (0 != m_mapController.size())
		{
			m_pHardwareFunction = m_mapController.begin()->second->GetHardwareFunction();
		}
		else
		{
			m_pHardwareFunction = new CHardwareFunction(m_bySlotNo, m_pAlarm);
		}
	}
}
// 
// void CBoard::Wait(UINT uUs)
// {
// 	LARGE_INTEGER TimeCur, TimeStop, TimeFreq;
// 	QueryPerformanceFrequency(&TimeFreq);
// 	QueryPerformanceCounter(&TimeCur);
// 	TimeStop.QuadPart = TimeCur.QuadPart + uUs * TimeFreq.QuadPart * 1e-6;
// 	while (TimeStop.QuadPart > TimeCur.QuadPart)
// 	{
// 		QueryPerformanceCounter(&TimeCur);
// 	}
// }
// 
// inline int CBoard::GetNumber(double dValue)
// {
// 	return (int)(dValue + (0 < dValue ? 0.5 : -0.5));
// }
// 
// template<typename Key, typename Value>
// inline void CBoard::DelteMemory(std::map<Key, Value>& mapParam)
// {
// 	for (auto& Param : mapParam)
// 	{
// 		if (nullptr != Param.second)
// 		{
// 			delete Param.second;
// 			Param.second = nullptr;
// 		}
// 	}
// 	mapParam.clear();
// }

int CBoard::StopVector(std::vector<USHORT>& vecChannel)
{
	return 0;
}

void CBoard::EnableStart(std::vector<USHORT>& vecChannel, BOOL bEnable)
{
}
