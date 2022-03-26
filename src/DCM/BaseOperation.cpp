#include "BaseOperation.h"
#include "STSSP8201.h"
#pragma comment(lib, "STSSP8201.lib")

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

void CBaseOperation::SetFunctionRegisterAddress(BYTE byFunctionAddr, USHORT usREGAddr)
{
	usREGAddr &= 0x3FF;
	byFunctionAddr &= 0x07;
	ULONG ulFullREGAddr = (byFunctionAddr << 10) | usREGAddr;
	m_ulAddress = (m_bySlotNo << 18) | (m_byModuleAddress << 15) | (ulFullREGAddr << 2);
}

ULONG CBaseOperation::BaseRead()
{
	ULONG regValue;

	regValue = read_dw(m_ulAddress);

#if BASE_OP_DEBUG
	ULONG slotAddr, fpgaAddr, regAddr;

	slotAddr = (m_ulAddress >> 18) & 0x3F;
	fpgaAddr = (m_ulAddress >> 15) & 0x07;
	regAddr = (m_ulAddress >> 2) & 0x1FFF;

	printf("Read  register Slot[%d] Fpga[%d] Addr[%04X] Value[%08X]\n", slotAddr, fpgaAddr, regAddr, regValue);
#endif

	return regValue;
}

void CBaseOperation::BaseWrite(ULONG ulData)
{
	write_dw(m_ulAddress, ulData);

#if BASE_OP_DEBUG
	ULONG slotAddr, fpgaAddr, regAddr;

	slotAddr = (m_ulAddress >> 18) & 0x3F;
	fpgaAddr = (m_ulAddress >> 15) & 0x07;
	regAddr = (m_ulAddress >> 2) & 0x1FFF;

	printf("Write register Slot[%d] Fpga[%d] Addr[%04X] Value[%08X]\n", slotAddr, fpgaAddr, regAddr, ulData);
#endif
}
