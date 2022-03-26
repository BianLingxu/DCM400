#include "pch.h"
#include "CMDCode.h"
#include "DCM400HardwareInfo.h"
using namespace std;
CCMDCode::CCMDCode()
{
	DETAIL Detail;
	auto AddCMD = [&](const char* lpszName, int nCode, int nMinOperand, int nMaxiOperand, BOOL bConditional, BOOL bJump, BOOL bSpecified)
	{
		if (nullptr == lpszName)
		{
			return;
		}
		Detail.m_strName = lpszName;
		Detail.m_nMaxOperand = nMaxiOperand;
		Detail.m_nMinOperand = nMinOperand;
		m_mapCMD.insert(make_pair(nCode, Detail));
		m_mapCode.insert(make_pair(lpszName, nCode));
		if (bConditional)
		{
			m_setConditionalCode.insert(nCode);
		}
		if (bJump)
		{
			m_setJUMPCode.insert(nCode);
		}
		if (bSpecified)
		{
			m_setSpecified.insert(nCode);
		}
	};
	AddCMD("INC", 0x00, -1, -1, FALSE, FALSE, FALSE);
	AddCMD("REPEAT", 0x01, 1, 65535, FALSE, TRUE, FALSE);
	AddCMD("JUMP", 0x02, 0, DCM400_MAX_PATTERN_COUNT - 1, TRUE, TRUE, FALSE);
	AddCMD("CALL", 0x03, 0, DCM400_MAX_PATTERN_COUNT - 1, FALSE, TRUE, FALSE);
	AddCMD("RETURN", 0x04, 0, DCM400_MAX_PATTERN_COUNT - 1, FALSE, TRUE, FALSE);
	AddCMD("SET_LOOPA", 0x05, 1, 65535, FALSE, FALSE, FALSE);
	AddCMD("END_LOOPA", 0x06, 0, DCM400_MAX_PATTERN_COUNT - 1, FALSE, TRUE, FALSE);
	AddCMD("SET_LOOPB", 0x07, 1, 65535, FALSE, FALSE, FALSE);
	AddCMD("END_LOOPB", 0x08, 0, DCM400_MAX_PATTERN_COUNT - 1, FALSE, TRUE, FALSE);
	AddCMD("SET_LOOPC", 0x09, 1, 65535, FALSE, FALSE, FALSE);
	AddCMD("END_LOOPC", 0x0A, 0, DCM400_MAX_PATTERN_COUNT - 1, FALSE, TRUE, FALSE);
	AddCMD("EXIT_LOOPA", 0x0B, -1, -1, FALSE, TRUE, FALSE);
	AddCMD("SIG_STOP", 0x0C, -1, -1, FALSE, FALSE, FALSE);
	AddCMD("EXIT", 0x0D, -1, -1, FALSE, FALSE, FALSE);
	AddCMD("TEST_FAIL", 0x0E, -1, -1, FALSE, FALSE, FALSE);
	AddCMD("MJUMP", 0x0F, -1, -1, TRUE, TRUE, FALSE);
	AddCMD("FJUMP", 0x10, 0, DCM400_MAX_PATTERN_COUNT - 1, TRUE, TRUE, FALSE);
	AddCMD("PJUMP", 0x11, 0, DCM400_MAX_PATTERN_COUNT - 1, TRUE, TRUE, FALSE);
	AddCMD("FCALL", 0x12, 0, DCM400_MAX_PATTERN_COUNT - 1, TRUE, TRUE, FALSE);
	AddCMD("PCALL", 0x13, 0, DCM400_MAX_PATTERN_COUNT - 1, TRUE, TRUE, FALSE);
	AddCMD("FRETURN", 0x14, 0, DCM400_MAX_PATTERN_COUNT - 1, TRUE, TRUE, FALSE);
	AddCMD("SET_GLO", 0x15, 0, DCM400_MAX_PATTERN_COUNT - 1, FALSE, FALSE, FALSE);
	AddCMD("MATCH", 0x16, 1, 65535, TRUE, TRUE, FALSE);
	AddCMD("MATCH_JUMP", 0x17, 0, DCM400_MAX_PATTERN_COUNT - 1, TRUE, TRUE, FALSE);
	AddCMD("SET_FAIL", 0x18, 1, 255, FALSE, FALSE, FALSE);
	AddCMD("CLEAR_FAIL", 0x19, -1, -1, FALSE, FALSE, FALSE);
	AddCMD("SET_FLAGA", 0x1A, -1, -1, FALSE, FALSE, FALSE);
	AddCMD("SET_FLAGB", 0x1B, -1, -1, FALSE, FALSE, FALSE);
	AddCMD("TRIG_OUT", 0x1C, -1, -1, FALSE, FALSE, FALSE);
	AddCMD("SCAN_IN", 0x1D, -1, -1, FALSE, FALSE, FALSE);
	AddCMD("SCAN_OUT", 0x1E, -1, -1, FALSE, FALSE, FALSE);
	AddCMD("TMU_START", 0x1F, -1, -1, FALSE, FALSE, FALSE);
	AddCMD("TMU_SET", 0x20, 0, 15, FALSE, FALSE, FALSE);

	AddCMD("HLT", 0x23, -1, -1, FALSE, FALSE, FALSE);

	///<The command saved to vector data
	AddCMD("STORE_DSIO", 100, -1, -1, FALSE, FALSE, TRUE);
	AddCMD("CAP_SHIFT_DSIO", 101, -1, -1, FALSE, TRUE, TRUE);
	AddCMD("SEND_DSIO", 102, -1, -1, FALSE, FALSE, TRUE);
	AddCMD("SRC_SHIFT_DSIO", 103, -1, -1, FALSE, FALSE, TRUE);
	AddCMD("SOURCE_SELECT", 104, 0, DCM400_CHANNELS_PER_CONTROL - 1, FALSE, FALSE, TRUE);
	AddCMD("STV_START", 105,-1, -1, FALSE, FALSE, TRUE);

}

int CCMDCode::GetCMDCode(const char* lpszCMD)
{
	if (nullptr == lpszCMD)
	{
		return -1;
	}
	auto iterCode= m_mapCode.find(lpszCMD);
	if (m_mapCode.end() == iterCode)
	{
		return -2;
	}
	return iterCode->second;
}

const char* CCMDCode::GetCMDName(int nCode)
{
	auto iterCMD = m_mapCMD.find(nCode);
	if (m_mapCMD.end() == iterCMD)
	{
		return nullptr;
	}
	return iterCMD->second.m_strName.c_str();
}

int CCMDCode::GetCMDOperandRange(int nCode, int& nMinOperand)
{
	auto iterCMD = m_mapCMD.find(nCode);
	if (m_mapCMD.end() == iterCMD)
	{
		return -1;
	}
	nMinOperand = iterCMD->second.m_nMinOperand;
	return iterCMD->second.m_nMaxOperand;
}

void CCMDCode::GetConditionalCode(std::set<int>& setCode) const
{
	setCode = m_setConditionalCode;
}

void CCMDCode::GetGeneralCMDCode(std::set<int>& setCode) const
{
	setCode.clear();
	for (auto& CMD : m_mapCMD)
	{
		if (m_setSpecified.end() != m_setSpecified.find(CMD.first))
		{
			continue;
		}
		setCode.insert(CMD.first);
	}
}

void CCMDCode::GetSpecifiedCMDCode(std::set<int>& setSpecified) const
{
	setSpecified = m_setSpecified;
}

void CCMDCode::GetJumpCMDCode(std::set<int>& setJumpCMD) const
{
	setJumpCMD = m_setJUMPCode;
}
