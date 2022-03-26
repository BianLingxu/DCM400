#include "pch.h"
#include "BaseOperation.h"
#include "STSSP8201.h"
#include <stdio.h>

#pragma comment(lib, "STSSP8201.lib")

/*--------------------------------------------------------------------------------------------*/
/* BitMap:   [22~16]        [15~13]         [12~11]       [10~0]                              */
/*         m_bySlotNo    m_byModuleAddr    m_byFpgaAddr    usRegAddr                          */
/*--------------------------------------------------------------------------------------------*/
/* Above description based on FPGA design document, to make full address that operated on PCIE*/
/* bar, above bit mapping should be left shift 2 bit.                                         */
/*--------------------------------------------------------------------------------------------*/

#define BASE_OP_DEBUG (0)

CBaseOperation::CBaseOperation(BYTE bySlotNo)
{
	m_bySlotNo = bySlotNo;
	m_ulAddress = 0;
	m_byModuleAddress = -1;
}

void CBaseOperation::WaitUs(double dWaitUs)
{
	read_dw(m_bySlotNo << 18);
	if (1 >= dWaitUs)
	{
		return;
	}
	dWaitUs -= 1;

	LARGE_INTEGER CurrTicks, TicksCount;

	QueryPerformanceFrequency(&TicksCount);
	QueryPerformanceCounter(&CurrTicks);

	TicksCount.QuadPart = ((LONGLONG)(TicksCount.QuadPart * dWaitUs)) / 1000000i64;
	TicksCount.QuadPart += CurrTicks.QuadPart;

	while (CurrTicks.QuadPart < TicksCount.QuadPart)
	{
		QueryPerformanceCounter(&CurrTicks);
	}
}

void CBaseOperation::SetModuleAddress(BYTE byModuleAddress)
{
	byModuleAddress &= 0x07;
	m_byModuleAddress = byModuleAddress;
}

void CBaseOperation::SetDetailAddress(BYTE byFPGAAddr, USHORT usDetailAddress)
{
	byFPGAAddr &= 0x03;
	usDetailAddress &= 0x3FF;
    m_ulAddress = (m_bySlotNo << 16) | (m_byModuleAddress << 13) | (byFPGAAddr << 11) | usDetailAddress;
    m_ulAddress <<= 2;
}

ULONG CBaseOperation::BaseRead()
{
	ULONG ulData;

	ulData = read_dw(m_ulAddress);

#if BASE_OP_DEBUG
	ULONG ulDbgAddr;

	ulDbgAddr = m_ulAddress >> 2;
    printf("Read  Slot[%d] Ctrl[%d] Fpga[%d] ADDR[%04X], DATA[%08X]\n", (ulDbgAddr >> 16) & 0x7F, (ulDbgAddr >> 13) & 0x07, (ulDbgAddr >> 11) & 0x03, ulDbgAddr & 0x3FF, ulData);
#endif

    return ulData;
}

void CBaseOperation::BaseWrite(ULONG ulData)
{
	write_dw(m_ulAddress, ulData);

#if BASE_OP_DEBUG
	ULONG ulDbgAddr;

	ulDbgAddr = m_ulAddress >> 2;
	printf("Write Slot[%d] Ctrl[%d] Fpga[%d] ADDR[%04X], DATA[%08X]\n", (ulDbgAddr >> 16) & 0x7F, (ulDbgAddr >> 13) & 0x07, (ulDbgAddr >> 11) & 0x03, ulDbgAddr & 0x3FF, ulData);
#endif
}
