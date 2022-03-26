#include "I2CLineInfo.h"
using namespace std;
CI2CLineInfo* CI2CLineInfo::Instance()
{
	static CI2CLineInfo I2CLineInfo;
	return &I2CLineInfo;
}

void CI2CLineInfo::Reset()
{
	m_bRecord = FALSE;
	m_mapLineInfo.clear();
}

void CI2CLineInfo::SetI2CBase(CI2CBase* pI2CRead, CI2CBase* pI2CWrite)
{
	m_pI2CBase[0] = pI2CRead;
	m_pI2CBase[1] = pI2CWrite;
}

void CI2CLineInfo::RecordLine(const std::pair<std::string, CI2CLine*>& pairI2CLine, BOOL bRead)
{
	if (!m_bRecord)
	{
		return;
	}
	if (!pairI2CLine.second->IsLoadVector())
	{
		///<The vector is not loaded, not record
		return;
	}


	UINT uBRAMStartLine = 0;
	UINT uDRAMStartLine = 0;
	BOOL bBRAM = pairI2CLine.second->GetStartLine(&uBRAMStartLine, &uDRAMStartLine);
	UINT uStartLine = bBRAM ? uBRAMStartLine : uDRAMStartLine | 1 << 31;
	int nLineCount = pairI2CLine.second->GetExclusiveLineCount();
	LINE_INFO I2CInfo;
	I2CInfo.m_strKey = pairI2CLine.first;
	I2CInfo.m_nLineCount = nLineCount;
	I2CInfo.m_bRead = static_cast<bool>(bRead);
	m_mapLineInfo.insert(make_pair(uStartLine, I2CInfo));
}

int CI2CLineInfo::FreeLine(int nLineCount)
{
	if (0 >= nLineCount)
	{
		return -1;
	}
	if (!m_bRecord)
	{
		RecordAllLine();
	}
	CI2CBase* pI2CBase = m_pI2CBase[0];
	int nLeftLine = nLineCount;
	auto riterLine = m_mapLineInfo.rbegin();
	while (m_mapLineInfo.rend() != riterLine && 0 < nLeftLine)
	{
		if (riterLine->second.m_bRead)
		{
			pI2CBase = m_pI2CBase[0];
		}
		else
		{
			pI2CBase = m_pI2CBase[1];
		}
		if (nullptr != pI2CBase)
		{
			pI2CBase->FreeI2C(riterLine->second.m_strKey);
			nLeftLine -= riterLine->second.m_nLineCount;
		}
		m_mapLineInfo.erase((++riterLine).base());
		
		riterLine = m_mapLineInfo.rbegin();
	}
	if (0 < nLeftLine)
	{
		return -2;
	}
	return 0;
}

CI2CLineInfo::CI2CLineInfo()
	: m_bRecord(false)
{

}

void CI2CLineInfo::RecordAllLine()
{
	m_bRecord = TRUE;
	for (auto& I2CBase : m_pI2CBase)
	{
		if (nullptr == I2CBase)
		{
			continue;
		}
		I2CBase->RecordLine();
	}
}
