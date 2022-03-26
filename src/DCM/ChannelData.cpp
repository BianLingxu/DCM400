#include "ChannelData.h"
#include "HardwareFunction.h"
CChannelData::CChannelData(CHardwareFunction &HardwareFunction)
{
	m_pHardwareFunction = &HardwareFunction;
	m_pusBRAMFM = nullptr;
	m_pusBRAMMM = nullptr;
	m_pusDRAMFM = nullptr;
	m_pusDRAMMM = nullptr;
	m_uBRAMStartLine = 0;
	m_uDRAMStartLine = 0;
	m_uBRAMLineCount = 0;
	m_uDRAMLineCount = 0;
	m_bPreread = FALSE;
	memset(m_bBRAMSame, 0, sizeof(m_bBRAMSame));
	memset(m_bDRAMSame, 0, sizeof(m_bDRAMSame));
}

CChannelData::~CChannelData()
{
	m_uBRAMStartLine = 0;
	m_uDRAMStartLine = 0;
	m_uBRAMLineCount = 0;
	m_uDRAMLineCount = 0;
	if (nullptr != m_pusBRAMFM)
	{
		delete[] m_pusBRAMFM;
		m_pusBRAMFM = nullptr;
	}
	if (nullptr != m_pusBRAMMM)
	{
		delete[] m_pusBRAMMM;
		m_pusBRAMMM = nullptr;
	}
	if (nullptr != m_pusDRAMFM)
	{
		delete[] m_pusDRAMFM;
		m_pusDRAMFM = nullptr;
	}
	if (nullptr != m_pusDRAMMM)
	{
		delete[] m_pusDRAMMM;
		m_pusDRAMMM = nullptr;
	}
}

int CChannelData::SetVectorInfo(UINT uStartLineIndex, UINT uLineCount, MEM_TYPE MemType)
{
	USHORT **ppuFM = nullptr;
	USHORT **ppuMM = nullptr;
	UINT uMaxLineCount = DCM_BRAM_PATTERN_LINE_COUNT;
	UINT *puStartLine = 0;
	UINT *puLineCount = 0;
	if (MEM_TYPE::BRAM == MemType)
	{
		ppuFM = &m_pusBRAMFM;
		ppuMM = &m_pusBRAMMM;
		puStartLine = &m_uBRAMStartLine;
		puLineCount = &m_uBRAMLineCount;
	}
	else
	{
		uMaxLineCount = DCM_DRAM_PATTERN_LINE_COUNT;
		ppuFM = &m_pusDRAMFM;
		ppuMM = &m_pusDRAMMM;
		MemType = MEM_TYPE::DRAM;
		puStartLine = &m_uDRAMStartLine;
		puLineCount = &m_uDRAMLineCount;
	}
	if (nullptr != *ppuFM)
	{
		if (*puStartLine <= uStartLineIndex && *puStartLine + *puLineCount >= uStartLineIndex + uLineCount)
		{
			return 0;
		}
		return -1;
	}
	if (uMaxLineCount <= uStartLineIndex)
	{
		return -2;
	}
	else if (uMaxLineCount <= uStartLineIndex + uLineCount)
	{
		return -3;
	}
	if (MAX_PREREAD_LINE_COUT < uLineCount)
	{
		uLineCount = MAX_PREREAD_LINE_COUT;
	}
	*puStartLine = uStartLineIndex;
	*puLineCount = uLineCount;
	int nRetVal = AllocateMemory();
	if (0 != nRetVal)
	{
		return -4;
	}
	m_pHardwareFunction->ReadDataMemory(MemType, DATA_TYPE::FM, uStartLineIndex, uLineCount, *ppuFM);
	m_pHardwareFunction->ReadDataMemory(MemType, DATA_TYPE::MM, uStartLineIndex, uLineCount, *ppuMM);

	m_bPreread = TRUE;

	return 0;
}

int CChannelData::SetLineInfo(UINT uStartLine, UINT uLineCount, MEM_TYPE MemType)
{
	m_setSetChannel.clear();
	UINT uTempStartLine = m_uBRAMStartLine;
	UINT uTempLineCount = m_uBRAMLineCount;
	UINT* puStartLine = &m_uCurBRAMStartLine;
	UINT* puLineCount = &m_uCurBRAMLineCount;
	UINT uMaxLineCount = DCM_BRAM_PATTERN_LINE_COUNT;

	USHORT** ppusFM = &m_pusBRAMFM;

	if (MEM_TYPE::BRAM != MemType)
	{
		uTempStartLine = m_uDRAMStartLine;
		uTempLineCount = m_uDRAMLineCount;
		puStartLine = &m_uCurDRAMStartLine;
		puLineCount = &m_uCurDRAMLineCount;
		ppusFM = &m_pusDRAMFM;

		uMaxLineCount = DCM_DRAM_PATTERN_LINE_COUNT;
	}
	if (nullptr != *ppusFM)
	{
		if (uStartLine < uTempStartLine || uStartLine + uLineCount > uTempStartLine + uTempLineCount)
		{
			///<No preread
			if (m_bPreread)
			{
				return -1;
			}

			USHORT** ppusMM = nullptr;
			if (MEM_TYPE::BRAM == MemType)
			{
				ppusMM = &m_pusBRAMMM;
			}
			else
			{
				ppusMM = &m_pusDRAMMM;
			}
			if (nullptr != *ppusFM)
			{
				delete[] * ppusFM;
				*ppusFM = nullptr;
			}

			if (nullptr != *ppusMM)
			{
				delete[] * ppusMM;
				*ppusMM = nullptr;
			}
		}
		else
		{
			*puStartLine = uStartLine;
			*puLineCount = uLineCount;
			return 0;
		}
	}
	if (uMaxLineCount <= uStartLine)
	{
		return -2;
	}
	else if (uMaxLineCount < uStartLine + uLineCount)
	{
		return -3;
	}
	*puStartLine = uStartLine;
	*puLineCount = uLineCount;

	puStartLine = &m_uBRAMStartLine;
	puLineCount = &m_uBRAMLineCount;
	if (MEM_TYPE::BRAM != MemType)
	{
		puStartLine = &m_uDRAMStartLine;
		puLineCount = &m_uDRAMLineCount;
	}
	*puStartLine = uStartLine;
	*puLineCount = uLineCount;
	int nRetVal = AllocateMemory();
	if (0 != nRetVal)
	{
		return -4;
	}
	return 0;
}

int CChannelData::SetChannelData(USHORT usChannel, MEM_TYPE MemType, const BYTE *pbyData)
{
	if (DCM_CHANNELS_PER_CONTROL <= usChannel)
	{
		return -1;
	}
	USHORT *pusFM = nullptr;
	USHORT *pusMM = nullptr;
	UINT uTempStartLine = m_uBRAMStartLine;
	UINT uTempLineCount = m_uBRAMLineCount;
	UINT *puStartLine = &m_uCurBRAMStartLine;
	UINT *puLineCount = &m_uCurBRAMLineCount;
	if (MEM_TYPE::BRAM == MemType)
	{
		pusFM = m_pusBRAMFM;
		pusMM = m_pusBRAMMM;
	}
	else
	{
		pusFM = m_pusDRAMFM;
		pusMM = m_pusDRAMMM;
		uTempStartLine = m_uDRAMStartLine;
		uTempLineCount = m_uDRAMLineCount;
		puStartLine = &m_uCurDRAMStartLine;
		puLineCount = &m_uCurDRAMLineCount;
	}
	
	if (nullptr == pbyData)
	{
		return -2;
	}
	int nCurByteBiteCount = 8;
	UINT uCurLineIndex = *puStartLine - uTempStartLine;
	for (UINT uLineIndex = 0; uLineIndex < *puLineCount; ++uLineIndex)
	{
		int nCurByteIndex = uLineIndex / 8;
		
		int nValidBitCount = *puLineCount - nCurByteIndex * 8;
		nValidBitCount = nValidBitCount > 8 ? 8 : nValidBitCount;

		BYTE byCurBit = pbyData[nCurByteIndex] >> (nValidBitCount - (uLineIndex % 8) - 1) & 0x01;
		SetBit(&pusFM[uCurLineIndex], usChannel, byCurBit);
		SetBit(&pusMM[uCurLineIndex++], usChannel, byCurBit);
	}
	m_setSetChannel.insert(usChannel);
	return 0;
}

void CChannelData::Write()
{
	if (!m_bPreread)
	{
		memset(m_bBRAMSame, 0, sizeof(m_bBRAMSame));
		memset(m_bDRAMSame, 0, sizeof(m_bDRAMSame));
	}
	int nChannelCount = m_setSetChannel.size();
	if (0 == nChannelCount)
	{
		m_uCurBRAMLineCount = 0;
		m_uCurDRAMLineCount = 0;
		return;
	}
	if (!m_bPreread)
	{
		if (1 == nChannelCount)
		{
			int nRetVal = WriteChannelData();
			if (0 == nRetVal)
			{
				return;
			}
		}
		if (DCM_CHANNELS_PER_CONTROL != nChannelCount)
		{
			ReadCombineData();
		}
	}
	BOOL (*pbSame)[2] = { 0 };
	UINT uCurStartLine = 0;
	UINT uCurLineCount = 0;
	UINT uDataStartLine = 0;
	USHORT *pusFM = nullptr;
	USHORT *pusMM = nullptr;
	MEM_TYPE MemType = MEM_TYPE::BRAM;
	for (int nIndex = 0; nIndex < 2;++nIndex)
	{
		if (0 == nIndex)
		{
			uCurStartLine = m_uCurBRAMStartLine;
			uCurLineCount = m_uCurBRAMLineCount;
			uDataStartLine = m_uBRAMStartLine;
			MemType = MEM_TYPE::BRAM;
			pusFM = m_pusBRAMFM;
			pusMM = m_pusBRAMMM;
			pbSame = &m_bBRAMSame;
		}
		else
		{
			uCurStartLine = m_uCurDRAMStartLine;
			uCurLineCount = m_uCurDRAMLineCount;
			uDataStartLine = m_uDRAMStartLine;
			MemType = MEM_TYPE::DRAM;
			pusFM = m_pusDRAMFM;
			pusMM = m_pusDRAMMM;
			pbSame = &m_bDRAMSame;
		}
		if (0 != uCurLineCount)
		{
			int nOffset = uCurStartLine - uDataStartLine;
			if (!(*pbSame)[0])
			{
				m_pHardwareFunction->WriteDataMemory(MemType, DATA_TYPE::FM, uCurStartLine, uCurLineCount, &pusFM[nOffset]);
			}
			if (!(*pbSame)[1])
			{
				m_pHardwareFunction->WriteDataMemory(MemType, DATA_TYPE::MM, uCurStartLine, uCurLineCount, &pusMM[nOffset]);
			}
		}
		memset(*pbSame, 0, sizeof(m_bDRAMSame));
	}

	m_uCurDRAMLineCount = 0;
	m_uCurBRAMLineCount = 0;
}

int CChannelData::GetPrereadType()
{
	if (!m_bPreread)
	{
		return -1;
	}
	if (0 != m_uBRAMLineCount)
	{
		return 0;
	}
	return 1;
}

inline void CChannelData::SetBit(USHORT *pusData, int nBitIndex, BOOL bSet)
{
	if (nullptr == pusData || 16 <= nBitIndex)
	{
		return;
	}
	USHORT usData = 1 << nBitIndex;
	if (bSet)
	{
		*pusData |= usData;
	}
	else
	{
		*pusData &= ~usData;
	}
}

int CChannelData::AllocateMemory()
{
	if (0 == m_uBRAMLineCount && nullptr != m_pusBRAMFM)
	{
		return -1;
	}
	else if (0 == m_uDRAMLineCount && nullptr != m_pusDRAMFM)
	{
		return -2;
	}
	if (0 == m_uBRAMLineCount && 0 == m_uDRAMLineCount)
	{
		return 0;
	}
	UINT uLineCount = 0;
	for (int nIndex = 0; nIndex < 2; nIndex++)
	{
		USHORT **ppusFM = nullptr;
		USHORT **ppusMM = nullptr;
		if (0 == nIndex)
		{
			ppusFM = &m_pusBRAMFM;
			ppusMM = &m_pusBRAMMM;
			uLineCount = m_uBRAMLineCount;
		}
		else
		{
			ppusFM = &m_pusDRAMFM;
			ppusMM = &m_pusDRAMMM;
			uLineCount = m_uDRAMLineCount;
		}
		if (0 == uLineCount || nullptr != *ppusFM)
		{
			continue;
		}
		try
		{
			*ppusFM = new USHORT[uLineCount];
			*ppusMM = new USHORT[uLineCount];
			memset(*ppusFM, 0, uLineCount * sizeof(USHORT));
			memset(*ppusMM, 0, uLineCount * sizeof(USHORT));
		}
		catch (const std::exception&)
		{
			return -3;
		}
	}
	return 0;
}

int CChannelData::ReadCombineData()
{
	USHORT usChannelCount = m_setSetChannel.size();
	USHORT *pusFM = nullptr;
	USHORT *pusMM = nullptr;
	UINT uStartLine = 0;
	UINT uLineCount = 0;
	USHORT **ppusCurFM = nullptr;
	USHORT **ppusCurMM = nullptr;
	BOOL (*pbSame)[2] = { 0 };
	MEM_TYPE MemType = MEM_TYPE::BRAM;
	for (int nMemIndex = 0; nMemIndex < 2; ++nMemIndex)
	{
		if (0 == nMemIndex)
		{
			MemType = MEM_TYPE::BRAM;
			uStartLine = m_uCurBRAMStartLine;
			uLineCount = m_uCurBRAMLineCount;
			ppusCurFM = &m_pusBRAMFM;
			ppusCurMM = &m_pusBRAMMM;
			pbSame = &m_bBRAMSame;
		}
		else
		{
			MemType = MEM_TYPE::DRAM;
			uStartLine = m_uCurDRAMStartLine;
			uLineCount = m_uCurDRAMLineCount;
			ppusCurFM = &m_pusDRAMFM;
			ppusCurMM = &m_pusDRAMMM;
			pbSame = &m_bDRAMSame;
		}
		if (0 == uLineCount)
		{
			continue;
		}
		try
		{
			pusFM = new USHORT[uLineCount];
			pusMM = new USHORT[uLineCount];
			memset(pusFM, 0, uLineCount * sizeof(USHORT));
			memset(pusMM, 0, uLineCount * sizeof(USHORT));
		}
		catch (const std::exception&)
		{
			return -1;
		}
		m_pHardwareFunction->ReadDataMemory(MemType, DATA_TYPE::FM, uStartLine, uLineCount, pusFM);
		m_pHardwareFunction->ReadDataMemory(MemType, DATA_TYPE::MM, uStartLine, uLineCount, pusMM);
		if (0 == memcmp(pusFM, ppusCurFM, uLineCount * sizeof(USHORT)))
		{
			(*pbSame)[0] = TRUE;
		}
		if (0 == memcmp(pusMM, ppusCurMM, uLineCount * sizeof(USHORT)))
		{
			(*pbSame)[1] = TRUE;
		}
		if (!(*pbSame)[0] || !(*pbSame)[1])
		{
			for (UINT uLineIndex = 0; uLineIndex < uLineCount; ++uLineIndex)
			{
				auto iterChannel = m_setSetChannel.begin();
				while (m_setSetChannel.end() != iterChannel)
				{
					USHORT usCurChannel = *iterChannel;
					if (!(*pbSame)[0])
					{
						SetBit(&pusFM[uLineIndex], usCurChannel, ((*ppusCurFM)[uLineIndex] >> usCurChannel) & 0x01);
					}
					if (!(*pbSame)[1])
					{
						SetBit(&pusMM[uLineIndex], usCurChannel, ((*ppusCurMM)[uLineIndex] >> usCurChannel) & 0x01);
					}
					++iterChannel;
				}				
			}
		}
		if (!(*pbSame)[0])
		{
			memcpy_s(*ppusCurFM, uLineCount * sizeof(USHORT), pusFM, uLineCount * sizeof(USHORT));
		}
		if (!(*pbSame)[1])
		{
			memcpy_s(*ppusCurMM, uLineCount * sizeof(USHORT), pusMM, uLineCount * sizeof(USHORT));
		}
		
		if (uLineCount != m_uCurDRAMLineCount)
		{
			if (nullptr != pusFM)
			{
				delete[] pusFM;
				pusFM = nullptr;
			}
			if (nullptr != pusMM)
			{
				delete[] pusMM;
				pusMM = nullptr;
			}
		}
	}
	if (nullptr != pusFM)
	{
		delete[] pusFM;
		pusFM = nullptr;
	}
	if (nullptr != pusMM)
	{
		delete[] pusMM;
		pusMM = nullptr;
	}
	return 0;
}

int CChannelData::WriteChannelData()
{
	USHORT usChannelCount = m_setSetChannel.size();
	if (1 < usChannelCount)
	{
		///<The channel count which need to be written is more than one
		return -1;
	}
	if (0 == usChannelCount)
	{
		return 0;
	}

	if (0 == m_uCurBRAMLineCount)
	{
		///<The line count in BRAM is 0
		if (0 == m_uCurDRAMLineCount)
		{
			return 0;
		}
		///<Not write DRAM, for not supported
		return 1;
	}
	USHORT usChannel = *m_setSetChannel.begin();
	int nDWDataCount = (m_uCurBRAMLineCount + 31) / 32;
	ULONG* pulFMDataLine = nullptr;
	ULONG* pulMMDataLine = nullptr;
	try
	{
		pulFMDataLine = new ULONG[nDWDataCount];
		pulMMDataLine = new ULONG[nDWDataCount];
		memset(pulFMDataLine, 0, nDWDataCount * sizeof(ULONG));
		memset(pulMMDataLine, 0, nDWDataCount * sizeof(ULONG));
	}
	catch (const std::exception&)
	{
		return -2;
	}
	int nBaseLine = m_uCurBRAMStartLine - m_uBRAMStartLine;
	for (UINT uDataIndex = 0; uDataIndex < m_uCurBRAMLineCount; ++uDataIndex)
	{
		pulFMDataLine[uDataIndex / 32] |= (m_pusBRAMFM[nBaseLine + uDataIndex] >> usChannel & 0x01) << (uDataIndex % 32);
		pulMMDataLine[uDataIndex / 32] |= (m_pusBRAMMM[nBaseLine + uDataIndex] >> usChannel & 0x01) << (uDataIndex % 32);
	}

	int nRetVal = 0;
	nRetVal = m_pHardwareFunction->WriteChannelDataMemory(MEM_TYPE::BRAM, DATA_TYPE::FM, usChannel, m_uCurBRAMStartLine, m_uCurBRAMLineCount, pulFMDataLine);
	nRetVal = m_pHardwareFunction->WriteChannelDataMemory(MEM_TYPE::BRAM, DATA_TYPE::MM, usChannel, m_uCurBRAMStartLine, m_uCurBRAMLineCount, pulMMDataLine);
	m_uCurBRAMLineCount = 0;
	if (nullptr != pulFMDataLine)
	{
		delete[] pulFMDataLine;
		pulFMDataLine = nullptr;
	}
	if (nullptr != pulMMDataLine)
	{
		delete[] pulMMDataLine;
		pulMMDataLine = nullptr;
	}
	if (0 != nRetVal)
	{
		nRetVal = -3;
	}
	if (0 == nRetVal && 0 != m_uCurDRAMLineCount)
	{
		return 1;
	}
	return nRetVal;
}
