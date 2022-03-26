#include "pch.h"
#include "PatternCMD.h"
#include "CMDCode.h"
#define EACH_CMD_ELEMENT_COUT 2
using namespace std;
CPatternCMD::CPatternCMD(CDriverAlarm* pAlarm)
	: m_pAlarm(pAlarm)
{
	memset(m_aulGeneral, 0, sizeof(m_aulGeneral));
	m_usSpecified = 0;
}

int CPatternCMD::SetCommandInfo(UINT uLineNo, USHORT usTimeSet, const std::vector<CMD_INFO>& vecCurLineCMD, const std::map<UINT, CLineCMD>& mapOtherLinesCMD, std::set<UINT>& setLineNeeded)
{
	setLineNeeded.clear();
	m_usSpecified = 0;
	memset(m_aulGeneral, 0, sizeof(m_aulGeneral));
	if (DCM400_MAX_PATTERN_COUNT <= uLineNo)
	{
		///<The line number is over range
		return -1;
	}
	if (TIME_SET_MAX_COUNT <= usTimeSet)
	{
		///<The time set is over range
		return -2;
	}
	m_nLineNo = uLineNo;
	vector<CMD_INFO> vecCMD = vecCurLineCMD;
	int nRetVal = ParseSpecifiedCMD(vecCMD);
	if (0 != nRetVal)
	{
		return -3;
	}
	for (auto& CMD : vecCMD)
	{
		int nResult = SaveFirstTwoCMD(CMD, mapOtherLinesCMD, setLineNeeded);
		if (0 != nResult)
		{
			switch (nResult)
			{
			case -1:
				///<The command is not supported
				nRetVal = -3;
				break;
			case -2:
				///<The other lines needed
				nRetVal = -4;
				break;
			case -3:
				///<The time set is over range
				nRetVal = -5;
				break;
			case -4:
				///<The operand is over range
				nRetVal = -6;
			default:
				break;
			}
		}
		nResult = SaveLastTwoCMD(CMD, mapOtherLinesCMD, setLineNeeded);
		if (0 != nResult)
		{
			switch (nResult)
			{
			case -1:
				///<The command is not supported
				nRetVal = -3;
				break;
			case -2:
				///<The other lines needed
				nRetVal = -4;
				break;
			case -3:
				///<The time set is over range
				nRetVal = -5;
				break;
			case -4:
				///<The operand is over range
				nRetVal = -6;
			default:
				break;
			}
		}
	}
	return nRetVal;
}

const ULONG* CPatternCMD::GetGeneralCMDData() const
{
	return m_aulGeneral;
}

const USHORT CPatternCMD::GetSpecifiedCMDData() const
{
	return m_usSpecified;
}

int CPatternCMD::ParseSpecifiedCMD(std::vector<CMD_INFO>& vecCMD)
{
	m_bScan = FALSE;
	m_usSpecified = 0;

	BOOL bFind = FALSE;
	CCMDCode CMDcode;
	map<int, int> mapCMD;
	mapCMD.insert(make_pair(CMDcode.GetCMDCode("STORE_DSIO"), 0));
	mapCMD.insert(make_pair(CMDcode.GetCMDCode("CAP_SHIFT_DSIO"), 1));
	mapCMD.insert(make_pair(CMDcode.GetCMDCode("SEND_DSIO"), 2));
	mapCMD.insert(make_pair(CMDcode.GetCMDCode("SRC_SHIFT_DSIO"), 3));
	mapCMD.insert(make_pair(CMDcode.GetCMDCode("SOURCE_SELECT"), 4));
	mapCMD.insert(make_pair(CMDcode.GetCMDCode("STV_START"), 8));
	int nSpecialCode = CMDcode.GetCMDCode("SOURCE_SELECT");
	mapCMD.insert(make_pair(CMDcode.GetCMDCode("STV_END"), 9));
	auto iterCMD = mapCMD.begin();
	set<CMD_INFO> setDelete;
	for (auto& CMD : vecCMD)
	{
		iterCMD = mapCMD.find(CMD.m_nCode);
		if (mapCMD.end() == iterCMD)
		{
			continue;
		}
		setDelete.insert(CMD);
		if (nSpecialCode != iterCMD->first)
		{
			m_usSpecified |= 1 << iterCMD->second;
			m_bScan = TRUE;
			continue;
		}
		int nMinOperand = 0;
		int nMaxOperand = CMDcode.GetCMDOperandRange(nSpecialCode, nMinOperand);
		if (CMD.m_nOperand > nMaxOperand || CMD.m_nOperand < nMinOperand)
		{
			if (nullptr != m_pAlarm)
			{
				m_pAlarm->SetAlarmMsg("The operand of command(%s) in line %d is over range[%d,%d].", CMD.m_nCode, CMDcode.GetCMDName(CMD.m_nCode), m_nLineNo + 1, 
					nMinOperand, nMaxOperand);
				m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmFatal);
 				m_pAlarm->SetAlarmID(ALARM_ID::ALARM_OPERAND_ERROR);
			}
			m_usSpecified = 0;
			return -1;
		}
		m_usSpecified |= CMD.m_nOperand << iterCMD->second;
	}
	return 0;
}

int CPatternCMD::SaveFirstTwoCMD(const CMD_INFO& CMD, const std::map<UINT, CLineCMD>& mapOtherLinesCMD, std::set<UINT>& setLineNeeded)
{
	memset(&m_aulGeneral, 0, EACH_CMD_ELEMENT_COUT * 2 * sizeof(ULONG));
	CCMDCode CMDCode;
	set<int> setGeneralCMD;
	CMDCode.GetGeneralCMDCode(setGeneralCMD);
	if (setGeneralCMD.end() == setGeneralCMD.find(CMD.m_nCode))
	{
		///<The command is not supported
		return -1;
	}
	BOOL bFail = FALSE;
	set<int> setJumpCMD;
	set<int> setCondtionalCMD;
	CMDCode.GetJumpCMDCode(setJumpCMD);
	CMDCode.GetConditionalCode(setCondtionalCMD);
	if (m_bScan)
	{
		setJumpCMD.insert(CMDCode.GetCMDCode("INC"));
	}

	if (setJumpCMD.end() != setJumpCMD.find(CMD.m_nCode))
	{
		///<Jump command, check conditional command
		if (setCondtionalCMD.end() == setCondtionalCMD.find(CMD.m_nCode))
		{
			///<Not conditional command
			return 0;
		}
	}

	auto iterCMD = mapOtherLinesCMD.find(m_nLineNo + 1);
	if (mapOtherLinesCMD.end() == iterCMD)
	{
		///<The line command not provided
		setLineNeeded.insert(m_nLineNo + 1);

		bFail = TRUE;
		return -2;
	}

	CMD_INFO TargetCMD;
	USHORT usTimeSet = 0;
	auto ConvertAndSave = [&](BOOL bFirstCMD)
	{
		GetGeneralCMD(iterCMD->second, usTimeSet, TargetCMD);;

		int nRetVal = SaveCMD(bFirstCMD ? 0 : 1, usTimeSet, TargetCMD.m_nCode, TargetCMD.m_nOperand, iterCMD->first);
		if (0 != nRetVal)
		{
			switch (nRetVal)
			{
			case -1:
				///<The command index is over range, not will happen
				break;
			case -2:
				///<The time set is over range
				nRetVal = -3;
				break;
			case -3:
				///<The command is not supported
				nRetVal = -1;
				break;
			case -4:
				///<The operand is over range
				nRetVal = -4;
				break;
			default:
				break;
			}
		}
		return nRetVal;
	};

	GetGeneralCMD(iterCMD->second, usTimeSet, TargetCMD);

	if (setJumpCMD.end() != setJumpCMD.find(TargetCMD.m_nCode))
	{
		///<Jump command, check conditional command
		if (setCondtionalCMD.end() != setCondtionalCMD.find(TargetCMD.m_nCode))
		{
			///<Conditional command
			memset(&m_aulGeneral[EACH_CMD_ELEMENT_COUT], 0, EACH_CMD_ELEMENT_COUT * sizeof(ULONG));

			iterCMD = mapOtherLinesCMD.find(m_nLineNo + 2);
			if (mapOtherLinesCMD.end() == iterCMD)
			{
				setLineNeeded.insert(m_nLineNo + 2);
				///<Set the default value is 0, in case of the line number is the latest line
				memset(&m_aulGeneral, 0, EACH_CMD_ELEMENT_COUT * sizeof(ULONG));
				bFail = TRUE;
			}
			else
			{
				return ConvertAndSave(TRUE);				
			}
		}
		else
		{
			memset(&m_aulGeneral, 0, EACH_CMD_ELEMENT_COUT * sizeof(ULONG));

			iterCMD = mapOtherLinesCMD.find(TargetCMD.m_nOperand);
			if (mapOtherLinesCMD.end() == iterCMD)
			{
				setLineNeeded.insert(TargetCMD.m_nOperand);
				///<Set the default value is 0, in case of the line number is the latest line
				memset(&m_aulGeneral[EACH_CMD_ELEMENT_COUT], 0, EACH_CMD_ELEMENT_COUT * sizeof(ULONG));
				bFail = TRUE;
			}
			else
			{
				return ConvertAndSave(FALSE);
			}
		}
	}
	else
	{
		memset(&m_aulGeneral[EACH_CMD_ELEMENT_COUT], 0, EACH_CMD_ELEMENT_COUT * sizeof(ULONG));

		iterCMD = mapOtherLinesCMD.find(m_nLineNo + 2);
		if (mapOtherLinesCMD.end() == iterCMD)
		{
			setLineNeeded.insert(m_nLineNo + 2);
			///<Set the default value is 0, in case of the line number is the latest line
			memset(&m_aulGeneral, 0, EACH_CMD_ELEMENT_COUT * sizeof(ULONG));
			bFail = TRUE;
		}
		else
		{
			return ConvertAndSave(TRUE);
		}
	}
	if (bFail)
	{
		return -2;
	}
	return 0;
}

int CPatternCMD::SaveLastTwoCMD(const CMD_INFO& CMD, const std::map<UINT, CLineCMD>& mapOtherLinesCMD, std::set<UINT>& setLineNeeded)
{
	memset(&m_aulGeneral[EACH_CMD_ELEMENT_COUT * 2], 0, EACH_CMD_ELEMENT_COUT * 2 * sizeof(ULONG));
	CCMDCode CMDCode;
	set<int> setGeneralCMD;
	CMDCode.GetGeneralCMDCode(setGeneralCMD);
	if (setGeneralCMD.end() == setGeneralCMD.find(CMD.m_nCode))
	{
		///<The command is not supported
		return -1;
	}
	BOOL bFail = FALSE;
	set<int> setJumpCMD;
	CMDCode.GetJumpCMDCode(setJumpCMD);
	if (m_bScan)
	{
		setJumpCMD.insert(CMDCode.GetCMDCode("INC"));
	}

	if (setJumpCMD.end() == setJumpCMD.find(CMD.m_nCode))
	{
		///<Not jump command
		return 0;
	}

	///<Jump command
	auto iterCMD = mapOtherLinesCMD.find(CMD.m_nOperand);
	if (mapOtherLinesCMD.end() == iterCMD)
	{
		setLineNeeded.insert(CMD.m_nOperand);
		return -2;
	}
	USHORT usTimeSet = 0;
	CMD_INFO TargetCMD;

	auto ConvertAndSave = [&](BOOL bFrontCMD)
	{
		int nCMDIndex = bFrontCMD ? 2 : 3;
		GetGeneralCMD(iterCMD->second, usTimeSet, TargetCMD);
		int nRetVal = SaveCMD(bFrontCMD ? 2 : 3, usTimeSet, TargetCMD.m_nCode, TargetCMD.m_nOperand, iterCMD->first);
		if (0 != nRetVal)
		{
			switch (nRetVal)
			{
			case -1:
				///<The command index is over range, not will happen
				break;
			case -2:
				///<The time set is over range
				nRetVal = -3;
				break;
			case -3:
				///<The command is not supported
				nRetVal = -1;
				break;
			case -4:
				///<The operand is over range
				nRetVal = -4;
				break;
			default:
				break;
			}
		}
		return nRetVal;
	};


	GetGeneralCMD(iterCMD->second, usTimeSet, TargetCMD);
	if (setJumpCMD.end() == setJumpCMD.find(TargetCMD.m_nCode))
	{
		///<Not jump command
		memset(&m_aulGeneral[EACH_CMD_ELEMENT_COUT * 3], 0, EACH_CMD_ELEMENT_COUT * sizeof(ULONG));
		iterCMD = mapOtherLinesCMD.find(TargetCMD.m_nOperand + 1);
		if (mapOtherLinesCMD.end() == iterCMD)
		{
			setLineNeeded.insert(TargetCMD.m_nOperand + 1);
			///<Set the default value is 0, in case of the line number is the latest line
			memset(&m_aulGeneral[EACH_CMD_ELEMENT_COUT * 2], 0, EACH_CMD_ELEMENT_COUT * sizeof(ULONG));
			return -2;
		}
		return ConvertAndSave(TRUE);
	}

	memset(&m_aulGeneral[EACH_CMD_ELEMENT_COUT * 2], 0, EACH_CMD_ELEMENT_COUT * sizeof(ULONG));
	
	GetGeneralCMD(iterCMD->second, usTimeSet, TargetCMD);
	iterCMD = mapOtherLinesCMD.find(TargetCMD.m_nOperand);
	if (mapOtherLinesCMD.end() == iterCMD)
	{
		setLineNeeded.insert(TargetCMD.m_nOperand);

		///<Set the default value is 0, in case of the line number is the latest line
		memset(&m_aulGeneral[EACH_CMD_ELEMENT_COUT * 3], 0, EACH_CMD_ELEMENT_COUT * sizeof(ULONG));
		return -2;
	}

	return ConvertAndSave(FALSE);
}

int CPatternCMD::SaveCMD(int nCMDIndex, USHORT usTimeSet, int nCode, ULONG ulOperand, UINT uLineNo)
{
	if (CMD_INFO_COUNT / EACH_CMD_ELEMENT_COUT <= nCMDIndex)
	{
		return -1;
	}
	if (TIME_SET_MAX_COUNT <= usTimeSet)
	{
		return -2;
	}
	CCMDCode CMDCode;
	set<int> setCMD;
	CMDCode.GetGeneralCMDCode(setCMD);
	if (setCMD.end() == setCMD.find(nCode))
	{
		///<The command is not existed
		return -3;
	}
	int nMinOperand = 0;
	int nMaxOperand = CMDCode.GetCMDOperandRange(nCode, nMinOperand);
	if (-1 != nMaxOperand && (ulOperand > nMaxOperand || ulOperand < nMinOperand))
	{
		///<The operand is over range
		if (nullptr != m_pAlarm)
		{
			m_pAlarm->SetAlarmMsg("The operand of command(%s) in line %d is over range[%d,%d].", ulOperand, CMDCode.GetCMDName(nCode), m_nLineNo + 1, nMinOperand, nMaxOperand);
			m_pAlarm->SetAlarmLevel(CDriverAlarm::ALARM_LEVEL::AlarmFatal);
			m_pAlarm->SetAlarmID(ALARM_ID::ALARM_OPERAND_ERROR);
		}
		return -4;
	}
	ULONG* pulCMD = &m_aulGeneral[EACH_CMD_ELEMENT_COUT * nCMDIndex];
	memset(pulCMD, 0, EACH_CMD_ELEMENT_COUT * sizeof(ULONG));
	pulCMD[0] |= nCode & 0xFF;
	pulCMD[0] |= (usTimeSet & 0xFF) << 8;
	pulCMD[0] |= (ulOperand & 0xFFFF) << 8;
	pulCMD[1] |= ulOperand >> 16 & 0xFFFF;
	return 0;
}

void CPatternCMD::GetGeneralCMD(const CLineCMD& LineCMD, USHORT usTimeSet, CMD_INFO& CMDInfo)
{
	CCMDCode CMDCode;
	set<int> setTargetCMD;
	CMDCode.GetGeneralCMDCode(setTargetCMD);
	vector<CMD_INFO> vecCMD;
	LineCMD.GetCMD(usTimeSet, vecCMD);
	for (const auto& CMD : vecCMD)
	{
		if (setTargetCMD.end() != setTargetCMD.find(CMD.m_nCode))
		{
			CMDInfo.m_nCode = CMD.m_nCode;
			CMDInfo.m_nOperand = CMD.m_nOperand;
			break;
		}
	}
}

CLineCMD::CLineCMD()
	: m_usTimeSet(0)
{
}

void CLineCMD::SetCMD(USHORT usTimeSet, const std::vector<CMD_INFO>& vecCMD)
{
	m_usTimeSet = usTimeSet;
	m_vecCMD = vecCMD;
}

void CLineCMD::GetCMD(USHORT& usTimeset, std::vector<CMD_INFO>& vecCMD) const
{
	usTimeset = m_usTimeSet;
	vecCMD = m_vecCMD;
}

bool operator<(const CMD_INFO& CMD1, const CMD_INFO& CMD2)
{
	if (CMD1.m_nCode < CMD1.m_nCode)
	{
		return true;
	}
	return false;
}
