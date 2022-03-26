//#include "pch.h"
#include "I2CRAM.h"
#include "DCM400HardwareInfo.h"
using namespace std;

//#define TEST_FREE_LINE 1

CI2CRAM::CI2CRAM()
{
	m_nBaseLine = 0;
	m_nTotalValidLineCount = 0;
}

int CI2CRAM::SetBaseLine(UINT uBaseLine)
{
	if (0 != m_mapPatternLine.size())
	{
		return -1;
	}
	m_nBaseLine = uBaseLine;
	m_nTotalValidLineCount = DCM400_MAX_PATTERN_COUNT - m_nBaseLine;

#ifdef TEST_FREE_LINE
	m_nTotalValidLineCount = 100;
	m_nTotalDRAMValidLineCount = 100;
#endif // TEST_FREE_LINE

	m_mapPatternLine.insert(make_pair(uBaseLine, m_nTotalValidLineCount));
	return 0;
}

int CI2CRAM::FreeLine(UINT uStartLine, UINT uLineCount)
{
	int nRetVal = 0;
	if (0 != uLineCount)
	{
		nRetVal = FreeLine(uStartLine, uLineCount);
		if (0 != nRetVal)
		{
			return -1;
		}
		return uLineCount;
	}
	return 0;
}

void CI2CRAM::Reset()
{
	m_nBaseLine = 0;
	m_mapPatternLine.clear();
}

void CI2CRAM::Clear()
{
	m_mapPatternLine.clear();
	SetBaseLine(m_nBaseLine);
}

int CI2CRAM::AllocateLine(UINT uLineCount, UINT* puStartLine)
{
	if (nullptr == puStartLine)
	{
		return -1;
	}	
	auto iterPattern = m_mapPatternLine.begin();
	while (m_mapPatternLine.end() != iterPattern)
	{
		if (uLineCount <= iterPattern->second)
		{
			break;
		}
		++iterPattern;
	}
	if (m_mapPatternLine.end() != iterPattern)
	{
		m_nTotalValidLineCount -= uLineCount;
		*puStartLine = iterPattern->first;
		UINT uNewStartLine = iterPattern->first + uLineCount;
		UINT uLeftLineCount = iterPattern->second - uLineCount;
		m_mapPatternLine.erase(iterPattern);
		if (0 == uLeftLineCount)
		{
			return 0;
		}
		m_mapPatternLine.insert(make_pair(uNewStartLine, uLeftLineCount));
		return 0;
	}
	return -1;
}

int CI2CRAM::FreeLine(int nStartLine, int nLineCount)
{
	int nMaxLineCount = DCM400_MAX_PATTERN_COUNT - m_nBaseLine;
	int nMaxLineNo = DCM400_MAX_PATTERN_COUNT - 1;

	if (m_nBaseLine > nStartLine || nStartLine > nMaxLineNo)
	{
		return -1;
	}
	if (nLineCount + nStartLine > nMaxLineNo || nLineCount + m_nTotalValidLineCount > nMaxLineCount)
	{
		return -2;
	}
	m_nTotalValidLineCount += nLineCount;

	BOOL bAddHead = TRUE;
	auto iterRAM = m_mapPatternLine.begin();
	while (m_mapPatternLine.end() != iterRAM)
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
	if (m_mapPatternLine.end() != iterRAM)
	{
		UINT uNewLineCount = iterRAM->second + nLineCount;
		UINT uNewStartLine = 0;
		if (bAddHead)
		{
			uNewStartLine = iterRAM->first - nLineCount;
			m_mapPatternLine.erase(iterRAM);
			m_mapPatternLine.insert(pair<UINT, UINT>(uNewStartLine, uNewLineCount));
			iterRAM = m_mapPatternLine.begin();
			while (m_mapPatternLine.end() != iterRAM)
			{
				if (iterRAM->first + iterRAM->second == uNewStartLine)
				{
					break;
				}
				else if (iterRAM->first > uNewStartLine)
				{
					iterRAM = m_mapPatternLine.end();
					break;
				}
				++iterRAM;
			}

			if (m_mapPatternLine.end() != iterRAM)
			{
				auto iterTempRAM = iterRAM;
				++iterTempRAM;
				iterRAM->second += uNewLineCount;
				m_mapPatternLine.erase(iterTempRAM);
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
				m_mapPatternLine.erase(iterRAM);
				iterRAM = m_mapPatternLine.find(uNewStartLine);
				iterRAM->second += uNewLineCount;
			}
		}
		return 0;
	}
	m_mapPatternLine.insert(make_pair(nStartLine, nLineCount));
	return 0;
}
