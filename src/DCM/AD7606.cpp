#include "AD7606.h"
#define CONTROL_REG (0x822 & 0x3FF)
#define STATUS_REG  (0x823 & 0x3FF)
#define MUTIPLE_REG (0x828 & 0x3FF)
#define SINGLE_REG  (0x824 & 0x3FF)
#define SAMPLE_RATE (0x8FE & 0x3FF)

#define USE_MULTIPLE_MODE (1)

CAD7606::CAD7606(BYTE bySlotNo) : CRegister(bySlotNo)
{
	SetModuleType(MODULE_TYPE::FUN_MODULE);
}

int CAD7606::Write(USHORT usRegisterAddress, ULONG ulData)
{
	CRegister::Write(CONTROL_REG, ulData);
	return 0;
}

int CAD7606::Read(USHORT usChannel, UINT uSampDepth, ULONG* pulDataBuff)
{
#if USE_MULTIPLE_MODE
	ULONG ulRegValue = 0, maxTime = 0;
	USHORT index = 0;
	int nTemp = 0;

	ulRegValue = ((uSampDepth - 1) << 8) + (1 << 2) + (1 << 0);

	// get max wait time
	ulRegValue = CRegister::Read(SAMPLE_RATE);
	switch (ulRegValue) {
	case 0x00:
		maxTime = uSampDepth * 5;
		break;
	case 0x01:
		maxTime = uSampDepth * 10;
		break;
	case 0x02:
		maxTime = uSampDepth * 20;
		break;
	case 0x03:
		maxTime = uSampDepth * 40;
		break;
	case 0x04:
		maxTime = uSampDepth * 80;
		break;
	case 0x05:
		maxTime = uSampDepth * 160;
		break;
	case 0x06:
		maxTime = uSampDepth * 320;
		break;
	default:
		maxTime = uSampDepth * 320;
		break;
	}
	maxTime = maxTime * 3 / 2;
	if (maxTime < 1000) {
		maxTime = 1000;
	}
	// wait for sample finish
	for (index = 0; index < maxTime; index++)
	{
		ulRegValue = CRegister::Read(STATUS_REG);
		if (0 != (ulRegValue & 0x02)) 
		{
			break;
		}
	}

	if (0 == (ulRegValue & 0x02)) 
	{
		return -1;
	}

	for (index = 0; index < 8; index++)
	{
		if (0 == (usChannel & (1 << index)))
		{
			continue;
		}

		ulRegValue = CRegister::Read(MUTIPLE_REG + index);
		nTemp = (int)ulRegValue;
		nTemp /= (int)uSampDepth;

		pulDataBuff[index] = (ULONG)nTemp;
	}
#else
	ULONG ulRegValue;
	USHORT index;

	ulRegValue = 0x01;
	CRegister::Write(CONTROL_REG, ulRegValue);

	for (index = 0; index < 1000; index++) {
		ulRegValue = CRegister::Read(STATUS_REG);
		if (0 != (ulRegValue & 0x02)) {
			break;
		}
	}
	if (0 == (ulRegValue & 0x02)) {
		return -1;
	}

	for (index = 0; index < 8; index+=2) {
		if (0 == (usChannel & (3 << index))) {
			continue;
		}

		ulRegValue = CRegister::Read(SINGLE_REG + index/2);
		if (0 != (usChannel & (1 << index))) {
			pulDataBuff[index + 0] = ulRegValue & 0xFFFF;
		}
		if (0 != (usChannel & (2 << index))) {
			pulDataBuff[index + 1] = ulRegValue >> 16;
		}
	}
#endif
	return 0;
}