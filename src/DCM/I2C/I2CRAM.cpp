#include "I2CRAM.h"
#include "HardwareInfo.h"
using namespace std;

//#define TEST_FREE_LINE 1

#define ODD_COMMON_START_LINE 3
#define EVEN_COMMON_STOP_LINE 2
#define COMMON_STOP_LINE 2
CI2CRAM::CI2CRAM()
{
	m_nBRAMBaseLine = 0;
	m_nDRAMBaseValidLine = 0;
	m_nCommonLineCount = ODD_COMMON_START_LINE + EVEN_COMMON_STOP_LINE + COMMON_STOP_LINE * 2;
	m_nBRAMBaseValidLine = 0;
	m_nTotalBRAMValidLineCount = 0;
	m_nTotalDRAMValidLineCount = 0;
}

int CI2CRAM::SetBaseRAM(UINT uBRAMBaseLine, UINT uDRAMBaseLine)
{
	if (0 != m_mapBRAMLine.size() || 0 != m_mapDRAMLine.size())
	{
		return -1;
	}
	m_nBRAMBaseLine = uBRAMBaseLine;
	m_nBRAMBaseValidLine = m_nBRAMBaseLine + m_nCommonLineCount;
	m_nDRAMBaseValidLine = uDRAMBaseLine;
	m_nTotalBRAMValidLineCount = DCM_BRAM_PATTERN_LINE_COUNT - m_nBRAMBaseValidLine;
	m_nTotalDRAMValidLineCount = DCM_DRAM_PATTERN_LINE_COUNT - m_nDRAMBaseValidLine;

#ifdef TEST_FREE_LINE
	m_nTotalBRAMValidLineCount = 100;
	m_nTotalDRAMValidLineCount = 100;
#endif // TEST_FREE_LINE


	m_mapBRAMLine.insert(make_pair(m_nBRAMBaseValidLine, m_nTotalBRAMValidLineCount));
	m_mapDRAMLine.insert(make_pair(m_nDRAMBaseValidLine, m_nTotalDRAMValidLineCount));
	return 0;
}

int CI2CRAM::AllocateLine(UINT uLineCount, UINT* puBRAMStartLine, UINT* puBRAMLineCount, UINT* puDRAMStartLine, UINT* puDRAMLineCount)
{
	if (nullptr == puBRAMStartLine || nullptr == puBRAMLineCount || nullptr == puDRAMStartLine || nullptr == puDRAMLineCount)
	{
		return -1;
	}
	*puBRAMStartLine = -1;
	*puBRAMLineCount = 0;
	*puDRAMStartLine = -1;
	*puDRAMLineCount = 0;
	UINT uStartLine = 0;
	int nRetVal = AllocateLine(TRUE, uLineCount, &uStartLine);
	if (0 == nRetVal)
	{
		*puBRAMStartLine = uStartLine;
		*puBRAMLineCount = uLineCount;
		return uLineCount;
	}
	if (0 == (uLineCount % 2))
	{
		uLineCount -= EVEN_COMMON_STOP_LINE + COMMON_STOP_LINE;
	}
	else
	{
		uLineCount -= ODD_COMMON_START_LINE + COMMON_STOP_LINE;
	}
	nRetVal = AllocateLine(FALSE, uLineCount, &uStartLine);
	if (0 == nRetVal)
	{
		*puDRAMStartLine = uStartLine;
		*puDRAMLineCount = uLineCount;
		return uLineCount;
	}
	return -2;
}

int CI2CRAM::FreeLine(UINT uBRAMStartLine, UINT uBRAMLineCount, UINT uDRAMStartLine, UINT uDRAMLineCount)
{
	int nRetVal = 0;
	if (0 != uBRAMLineCount)
	{
		nRetVal = FreeLine(TRUE, uBRAMStartLine, uBRAMLineCount);
		if (0 != nRetVal)
		{
			return -1;
		}
		return uBRAMLineCount;
	}
	else if (0 != uDRAMLineCount)
	{
		nRetVal = FreeLine(FALSE, uDRAMStartLine, uDRAMLineCount);
		if (0 != nRetVal)
		{
			return -2;
		}
		return uBRAMLineCount;
	}
	return 0;
}

int CI2CRAM::GetCommonLine(UINT* puOddStartLine, UINT* puOddStopLine, UINT* puEvenStartLine, UINT* puEvenStopLine)
{
	if (nullptr == puOddStartLine || nullptr == puOddStopLine || nullptr == puEvenStartLine || nullptr == puEvenStopLine)
	{
		return -1;
	}
	*puOddStartLine = m_nBRAMBaseLine;
	*puOddStopLine = *puOddStartLine + ODD_COMMON_START_LINE + COMMON_STOP_LINE - 1;
	*puEvenStartLine = *puOddStopLine + 1;
	*puEvenStopLine = *puEvenStartLine + EVEN_COMMON_STOP_LINE + COMMON_STOP_LINE - 1;
	return 0;
}

int CI2CRAM::GetCommonLineCount()
{
	return m_nCommonLineCount;
}

int CI2CRAM::GetCommonLineInfo(BOOL bOddStart, BYTE* pbyHeadLineCount, BYTE* pbyEndLineCount)
{
	if (nullptr == pbyHeadLineCount || nullptr == pbyEndLineCount)
	{
		return -1;
	}
	if (bOddStart)
	{
		*pbyHeadLineCount = ODD_COMMON_START_LINE;
	}
	else
	{
		*pbyHeadLineCount = EVEN_COMMON_STOP_LINE;
	}
	*pbyEndLineCount = COMMON_STOP_LINE;
	return 0;
}

void CI2CRAM::Reset()
{
	m_nBRAMBaseLine = 0;
	m_nDRAMBaseValidLine = 0;
	m_nBRAMBaseValidLine = 0;
	m_mapBRAMLine.clear();
	m_mapDRAMLine.clear();
}

void CI2CRAM::Clear()
{
	m_mapBRAMLine.clear();
	m_mapDRAMLine.clear();
	SetBaseRAM(m_nBRAMBaseLine, m_nDRAMBaseValidLine);
}

int CI2CRAM::AllocateLine(BOOL bBRAM, UINT uLineCount, UINT* puStartLine)
{
	if (nullptr == puStartLine)
	{
		return -1;
	}
	int* puTotalValidLineCount = &m_nTotalBRAMValidLineCount;
	auto pmapRAM = &m_mapBRAMLine;
	if (!bBRAM)
	{
		pmapRAM = &m_mapDRAMLine;
		puTotalValidLineCount = &m_nTotalDRAMValidLineCount;
	}

	auto iterRAM = pmapRAM->begin();
	while (pmapRAM->end() != iterRAM)
	{
		if (uLineCount <= iterRAM->second)
		{
			break;
		}
		++iterRAM;
	}
	if (pmapRAM->end() != iterRAM)
	{
		puTotalValidLineCount -= uLineCount;
		*puStartLine = iterRAM->first;
		UINT uNewStartLine = iterRAM->first + uLineCount;
		UINT uLeftLineCount = iterRAM->second - uLineCount;
		pmapRAM->erase(iterRAM);
		if (0 == uLeftLineCount)
		{
			return 0;
		}
		pmapRAM->insert(make_pair(uNewStartLine, uLeftLineCount));
		return 0;
	}
	return -1;
}

int CI2CRAM::FreeLine(BOOL bBRAM, int nStartLine, int nLineCount)
{
	auto pmapRAM = &m_mapBRAMLine;
	int nBaseValidLine = m_nBRAMBaseValidLine;
	int* pnTotalValidLineCount = &m_nTotalBRAMValidLineCount;
	int nMaxLineCount = DCM_BRAM_PATTERN_LINE_COUNT - m_nBRAMBaseValidLine;
	int nMaxLineNo = DCM_BRAM_PATTERN_LINE_COUNT - 1;
	if (!bBRAM)
	{
		pmapRAM = &m_mapDRAMLine;
		nBaseValidLine = m_nDRAMBaseValidLine;
		pnTotalValidLineCount = &m_nTotalDRAMValidLineCount;
		nMaxLineCount = DCM_DRAM_PATTERN_LINE_COUNT - m_nDRAMBaseValidLine;
		nMaxLineNo = DCM_DRAM_PATTERN_LINE_COUNT - 1;
	}
	if (nBaseValidLine > nStartLine || nStartLine > nMaxLineNo)
	{
		return -1;
	}
	if (nLineCount + nStartLine > nMaxLineNo || nLineCount + *pnTotalValidLineCount > nMaxLineCount)
	{
		return -2;
	}
	*pnTotalValidLineCount += nLineCount;

	BOOL bAddHead = TRUE;
	auto iterRAM = pmapRAM->begin();
	while (pmapRAM->end() != iterRAM)
	{
		if (iterRAM->first + iterRAM->second == nStartLine)
		{
			bAddHead = FALSE;
			break;
		}
		else if(iterRAM->first == nStartLine + nLineCount)
		{
			bAddHead = TRUE;
			break;
		}
		++iterRAM;
	}
	if (pmapRAM->end() != iterRAM)
	{
		UINT uNewLineCount = iterRAM->second + nLineCount;
		UINT uNewStartLine = 0;
		if (bAddHead)
		{
			uNewStartLine = iterRAM->first - nLineCount;
			pmapRAM->erase(iterRAM);
			pmapRAM->insert(pair<UINT, UINT>(uNewStartLine, uNewLineCount));
			iterRAM = pmapRAM->begin();
			while (pmapRAM->end() != iterRAM)
			{
				if (iterRAM->first + iterRAM->second == uNewStartLine)
				{
					break;
				}
				else if (iterRAM->first > uNewStartLine)
				{
					iterRAM = pmapRAM->end();
					break;
				}
				++iterRAM;
			}

			if (pmapRAM->end() != iterRAM)
			{
				auto iterTempRAM = iterRAM;
				++iterTempRAM;
				iterRAM->second += uNewLineCount;
				pmapRAM->erase(iterTempRAM);
			}
		}
		else
		{
			iterRAM->second += nLineCount;
			uNewStartLine = iterRAM->first;
			++iterRAM;
			if (uNewStartLine + uNewLineCount == iterRAM->first)
			{
				uNewLineCount += iterRAM->second;
				pmapRAM->erase(iterRAM);
				iterRAM = pmapRAM->find(uNewStartLine);
				iterRAM->second += uNewLineCount;
			}
		}
		return 0;
	}
	pmapRAM->insert(make_pair(nStartLine, nLineCount));
	return 0;
}
