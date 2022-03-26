#include "COM.h"
#include "Bind.h"
#include "Register.h"

using namespace std;
#define BURST_SIZE (256)

CCOM::CCOM(BYTE bySlotNo)
	: CFPGAA(bySlotNo)
{
	memset(m_usSelectAddress, 0, sizeof(m_usSelectAddress));
	memset(m_uDataAddress, 0, sizeof(m_uDataAddress));
	m_usRequestAddress = 0;
	m_usStatusAddress = 0;
	m_usQueryTime = 0;
	m_ulStartAddresss = 0;
	SetModuleType(MODULE_TYPE::COM_MODULE);
}

void CCOM::SetSelectAddress(USHORT usSelectAddressRead, USHORT usSelectAddressWrite)
{
	m_usSelectAddress[0] = usSelectAddressRead;
	m_usSelectAddress[1] = usSelectAddressWrite;
}

void CCOM::SetDataAddress(USHORT usReadAddress, USHORT usWriteAddress)
{
	m_uDataAddress[0] = usReadAddress;
	m_uDataAddress[1] = usWriteAddress;
}

void CCOM::SetRequestAddress(USHORT usRequestAddress)
{
	m_usRequestAddress = usRequestAddress;
}

void CCOM::SetStatusAddress(USHORT usStatusAddress, USHORT usQueryTime)
{
	m_usStatusAddress = usStatusAddress;
	m_usQueryTime = usQueryTime;
}

void CCOM::SetStartAddress(ULONG ulAddress)
{
	m_ulStartAddresss = ulAddress;
}

int CCOM::Read(ULONG ulReadDataCount, ULONG* pulData)
{
	if (0 == ulReadDataCount || nullptr == pulData)
	{
		return -1;
	}

	if (0 == m_usQueryTime)
	{
		CFPGAA::Write(m_usSelectAddress[0], m_ulStartAddresss);
		for (ULONG ulDataIndex = 0; ulDataIndex < ulReadDataCount; ++ulDataIndex)
		{
			*pulData = CFPGAA::Read(m_uDataAddress[0]);
			pulData++;
		}
	}
	else
	{
		ULONG ulStatus = 0;
		UINT uLeftLine = ulReadDataCount;
		UINT uStartAddr = m_ulStartAddresss;
		UINT uCurReadLine = 0;
		while (0 < uLeftLine)
		{
			uCurReadLine = (uLeftLine > BURST_SIZE) ? BURST_SIZE : uLeftLine;
			// send address
			CFPGAA::Write(m_usSelectAddress[0], uStartAddr);

			// send request
			CFPGAA::Write(m_usRequestAddress, (uCurReadLine * 4) | (1 << 17));

			// check status
			int nReady = WaitReady(0x02);
			if (0 != nReady)
			{
				return -2;
			}

			// read data
			for (ULONG ulDataIndex = 0; ulDataIndex < uCurReadLine; ++ulDataIndex)
			{
				*pulData = CFPGAA::Read(m_uDataAddress[0]);
				pulData++;
			}

			uStartAddr += uCurReadLine;
			uLeftLine -= uCurReadLine;
		}
	}
	return 0;
}

int CCOM::Write(ULONG ulWriteCount, const ULONG* pulData)
{
	if (0 == ulWriteCount || nullptr == pulData)
	{
		return -1;
	}
	if (0 == m_usQueryTime)
	{
		CFPGAA::Write(m_usSelectAddress[1], m_ulStartAddresss);
		for (ULONG ulDataIndex = 0; ulDataIndex < ulWriteCount; ++ulDataIndex)
		{
			CFPGAA::Write(m_uDataAddress[1], *pulData);
			pulData++;
		}
	}
	else
	{
		ULONG ulStatus = 0;
		UINT uLeftLine = ulWriteCount;
		UINT uStartAddr = m_ulStartAddresss;
		UINT uCurReadLine = 0;
		while (uLeftLine > 0)
		{
			uCurReadLine = (uLeftLine > BURST_SIZE) ? BURST_SIZE : uLeftLine;

			// send address
			CFPGAA::Write(m_usSelectAddress[1], uStartAddr);

			// send data
			for (ULONG ulDataIndex = 0; ulDataIndex < uCurReadLine; ++ulDataIndex)
			{
				CFPGAA::Write(m_uDataAddress[1], *pulData);
				pulData++;
			}

			// send request
			CFPGAA::Write(m_usRequestAddress, (uCurReadLine * 4) | (1 << 16));

			// check status
			int nRetVal = WaitReady(0x01);
			if (0 != nRetVal)
			{
				return -2;
			}
			uStartAddr += uCurReadLine;
			uLeftLine -= uCurReadLine;
		}
	}
	return 0;
}

inline int CCOM::WaitReady(ULONG ulStatusBit)
{
	int nQueryTimes = 0;
	ULONG ulStatus = 0;
	BOOL bReady = TRUE;
	if (CBindInfo::Instance()->IsBind())
	{
		///<Check all controller binded to current controller
		BOOL bTimeout = FALSE;
		set<BYTE> setSlot;
		set<BYTE> setController;
		BYTE byTargetSlot = CBindInfo::Instance()->GetBindInfo(setSlot, setController);
		auto iterSlot = setSlot.begin();
		while (setSlot.end() != iterSlot)
		{
			CRegister Controller(*iterSlot);
			Controller.SetRegisterType(CRegister::COM_REG);

			auto iterController = setController.begin();
			while (setController.end() != iterController)
			{
				Controller.SetControllerIndex(*iterController);
				for (;nQueryTimes < m_usQueryTime;++ nQueryTimes)
				{
					bReady = FALSE;
					ulStatus = Controller.Read(m_usStatusAddress);
					if (0 == (ulStatus & ulStatusBit))
					{
						bReady = TRUE;
						break;
					}
				}
				if (nQueryTimes >= m_usQueryTime)
				{
					bTimeout = TRUE;
					break;
				}
				++iterController;
			}
			if (bTimeout)
			{
				break;
			}
			++iterSlot;
		}
		CBindInfo::Instance()->Bind(setSlot, setController, byTargetSlot);
	}
	else
	{
		bReady = FALSE;
		for (; nQueryTimes < m_usQueryTime; ++nQueryTimes)
		{
			ulStatus = CFPGAA::Read(m_usStatusAddress);
			if (0 == (ulStatus & ulStatusBit))
			{
				bReady = TRUE;
				break;
			}
		}
	}
	if (bReady)
	{
		return 0;
	}
	return -1;
}
