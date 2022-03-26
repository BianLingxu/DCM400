#include "pch.h"
#include "ATE318.h"
#define ACCESS_CTRL (0x830 & 0x3FF)
#define STATUS_ADDR (0x831 & 0x3FF)
#define WDTATA01_ADDR (0x840 & 0x3FF)
#define RDTATA01_ADDR (0x848 & 0x3FF)

#define WDTATA0_SINGLE_ADDR (0x850 & 0x3FF)
#define RDTATA0_SINDLE_ADDR (0x860 & 0x3FF)

#define USE_BURST_MODE (0)

CATE318::CATE318(BYTE bySlotNo)
	: CRegister(bySlotNo)
{
	//SetModuleType(MODULE_TYPE::FUN_MODULE);
}

ULONG CATE318::GetStatus()
{
	return CRegister::Read(STATUS_ADDR);
}

int CATE318::Read(BYTE byAddress, std::map<USHORT, ULONG>& mapChannelData)
{
	if (0 == mapChannelData.size())
	{
		return -2;
	}

	byAddress &= 0x1F;

#if !USE_BURST_MODE
	auto iterChannel = mapChannelData.begin();
	while (mapChannelData.end() != iterChannel)
	{
		iterChannel->second = ReadData(byAddress, (BYTE)(iterChannel->first));
		++iterChannel;
	}

	return 0;
#else
	if (mapChannelData.size() <= 2)
	{
		auto iterChannel = mapChannelData.begin();
		while (mapChannelData.end() != iterChannel)
		{
			iterChannel->second = ReadData(byAddress, (BYTE)(iterChannel->first));
			++iterChannel;
		}
		return 0;
	}

	USHORT uReadChannel = 0;
	auto iterChannel = mapChannelData.begin();
	while (mapChannelData.end() != iterChannel)
	{
		uReadChannel |= 1 << iterChannel->first;
		++iterChannel;
	}
	ULONG ulData = GetCTRLData(byAddress, uReadChannel, TRUE, FALSE);
	CRegister::Write(ACCESS_CTRL, ulData);

	BYTE byCurReadChannel = 0;
	for (BYTE byIndex = 0; byIndex < 8;++byIndex)
	{
		byCurReadChannel = (uReadChannel >> (byIndex * 2)) & 0x03;
		if (0 == byCurReadChannel)
		{
			continue;
		}
		ulData = CRegister::Read(RDTATA01_ADDR + byIndex);
		for (int nChannelIndex = 0; nChannelIndex < 2;++nChannelIndex)
		{
			iterChannel = mapChannelData.find(byIndex * 2 + nChannelIndex);
			if (mapChannelData.end() == iterChannel)
			{
				continue;
			}
			iterChannel->second = ulData >> (nChannelIndex * 8);
		}
	}
#endif

	return 0;
}

int CATE318::Write(BYTE byAddress, std::map<USHORT, ULONG>& mapChannelData)
{
	if (0 == mapChannelData.size())
	{
		return -2;
	}

	byAddress &= 0x1F;
#if !USE_BURST_MODE
	for (auto & Data : mapChannelData)
	{
		WriteData(byAddress, Data.first, Data.second);
	}
	WaitUs(15);

#else
	if (mapChannelData.size() <= 2) {
		auto iterChannel = mapChannelData.begin();
		while (mapChannelData.end() != iterChannel)
		{
			WriteData(byAddress, (BYTE)(iterChannel->first), (USHORT)(iterChannel->second));
			++iterChannel;
		}

		return 0;
	}

	USHORT uWriteChannel = 0;
	BOOL bAllDataSame = TRUE;
	iterChannel = mapChannelData.begin();
	BYTE byWriteData = iterChannel->second;
	++iterChannel;
	while (mapChannelData.end() != iterChannel)
	{
		if (byWriteData != iterChannel->second)
		{
			bAllDataSame = FALSE;
		}
		uWriteChannel |= 1 << iterChannel->first;
		++iterChannel;
	}

	BYTE byLow3Bits = 3;
	ULONG ulData = 0;
	if (bAllDataSame)
	{
		byLow3Bits = 3;
		CRegister::Write(WDTATA01_ADDR, byWriteData);
	}
	else
	{
		byLow3Bits = 7;
		BYTE byCurChannel = 0;
		for (BYTE byChipIndex = 0; byChipIndex < 8; ++byChipIndex)
		{
			ulData = 0;
			for (BYTE byOffset = 0; byOffset < 2;++byOffset)
			{
				byCurChannel = byChipIndex * 2 + byOffset;
				iterChannel = mapChannelData.find(byCurChannel);
				if (mapChannelData.end() == iterChannel)
				{
					continue;
				}
				ulData = iterChannel->second << (byOffset * 16);
			}
			CRegister::Write(WDTATA01_ADDR + byChipIndex, ulData);
		}
	}
	ulData = GetCTRLData(byAddress, uWriteChannel, FALSE, bAllDataSame);
	CRegister::Write(ACCESS_CTRL, ulData);
#endif

	return 0;
}

int CATE318::SetAddress(ULONG ulAddress)
{
	return -1;
}

int CATE318::SetAddress(int nType, USHORT usAddress)
{
	return -1;
}

int CATE318::SetStartLineNo(int nType, ULONG ulStartLineNo)
{
	return -1;
}

ULONG CATE318::Read(int nAddress)
{
	return -1;
}

int CATE318::Write(int nAddress, ULONG ulData)
{
	return -1;
}

int CATE318::Read(UINT uReadDataCount, ULONG* pulDataBuff)
{
	return -1;
}

int CATE318::Write(UINT uWriteDataCount, const ULONG* pulData)
{
	return -1;
}

int CATE318::Read(USHORT usAddress, UINT uReadDataCount, ULONG* pulDataBuff)
{
	return -1;
}

int CATE318::Write(USHORT usAddress, UINT uWriteDataCount, const ULONG* pulData)
{
	return -1;
}

inline ULONG CATE318::GetCTRLData(BYTE byAddress, USHORT uEnableChannel, BOOL bRead, BOOL bSame)
{
	BYTE byLow3Bit = bRead ? 1 : (bSame ? 3 : 7);
	return (uEnableChannel << 16) | (byAddress << 3) | byLow3Bit;
}

ULONG CATE318::ReadData(BYTE byAddress, BYTE byChannel)
{
	ULONG ulData = (1 << 21) | (byAddress << 16);
	ULONG ulAddress = RDTATA0_SINDLE_ADDR + byChannel;
	CRegister::Write(ulAddress, ulData);
	WaitUs(2.5);
	ulData = CRegister::Read(ulAddress);
	return ulData & 0xFFFF;
}

void CATE318::WriteData(BYTE byAddress, BYTE byChannel, USHORT uData)
{
	ULONG ulData = (1 << 21) | (byAddress << 16) | uData;
	CRegister::Write(WDTATA0_SINGLE_ADDR + byChannel, ulData);
}

