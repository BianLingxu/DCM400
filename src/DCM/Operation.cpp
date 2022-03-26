#include "Operation.h"
#include "ATE305.h"
#include "AD7606.h"
#include "BRAM.h"
#include "DRAM.h"
#include "Register.h"
#include "FPGAE.h"
COperation::COperation(BYTE bySlotNo)
{
	m_bySlotNo = bySlotNo;
	m_byControllerIndex = 0;

	CBaseOperation* pOperation = new CBRAM(bySlotNo);
	m_mapOperation.insert(std::pair<BYTE, CBaseOperation*>(BRAM, pOperation)); 

	pOperation = new CDRAM(bySlotNo);
	m_mapOperation.insert(std::pair<BYTE, CBaseOperation*>(DRAM, pOperation));

	pOperation = new CATE305(bySlotNo);
	m_mapOperation.insert(std::pair<BYTE, CBaseOperation*>(ATE305_TYPE, pOperation));

	pOperation = new CAD7606(bySlotNo);
	m_mapOperation.insert(std::pair<BYTE, CBaseOperation*>(AD7606_TYPE, pOperation));

	pOperation = new CRegister(bySlotNo);
	pOperation->SetRegisterType((USHORT)CRegister::REG_TYPE::COM_REG);
	m_mapOperation.insert(std::pair<BYTE, CBaseOperation*>(COM_TYPE, pOperation));

	pOperation = new CRegister(bySlotNo);
	pOperation->SetRegisterType((USHORT)CRegister::REG_TYPE::SYS_REG);
	m_mapOperation.insert(std::pair<BYTE, CBaseOperation*>(SYS_REG_TYPE, pOperation));

	pOperation = new CRegister(bySlotNo);
	pOperation->SetRegisterType((USHORT)CRegister::REG_TYPE::FUNC_REG);
	m_mapOperation.insert(std::pair<BYTE, CBaseOperation*>(FUNC_REG_TYPE, pOperation));

	pOperation = new CRegister(bySlotNo);
	pOperation->SetRegisterType((USHORT)CRegister::REG_TYPE::TMU_REG);
	m_mapOperation.insert(std::pair<BYTE, CBaseOperation*>(TMU_REG_TYPE, pOperation));

	pOperation = new CRegister(bySlotNo);
	pOperation->SetRegisterType((USHORT)CRegister::REG_TYPE::CAL_REG);
	m_mapOperation.insert(std::pair<BYTE, CBaseOperation*>(CAL_REG_TYPE, pOperation));

	pOperation = new CFPGAE(bySlotNo);
	m_mapOperation.insert(std::pair<BYTE, CBaseOperation*>(BOARD_TYPE, pOperation));
}

COperation::~COperation()
{
	for (auto& Operation : m_mapOperation)
	{
		if (nullptr != Operation.second)
		{
			delete Operation.second;
			Operation.second = nullptr;
		}
	}
	m_mapOperation.clear();
}

BYTE COperation::GetSlotNo() const
{
	return m_bySlotNo;
}

int COperation::SetControllerIndex(BYTE byControllerIndex)
{
	int nRetVal = 0;
	m_byControllerIndex = byControllerIndex;
	for (auto& Operation : m_mapOperation)
	{
		if (nullptr != Operation.second)
		{
			nRetVal = Operation.second->SetControllerIndex(m_byControllerIndex);
			if (0 != nRetVal)
			{
				break;;
			}
		}
	}
	return nRetVal;
}

BYTE COperation::GetControllerIndex() const
{
	return m_byControllerIndex;
}

int COperation::ReadRAM(BRAM_TYPE RAMType, USHORT usRAMAddress, UINT uReadDataCount, ULONG* pulDataBuff)
{
	OPERATION_TYPE OperationType = BRAM;
	switch (RAMType)
	{
	case BRAM_TYPE::IMM1:
		break;
	case BRAM_TYPE::IMM2:
		break;
	case BRAM_TYPE::IMM3:
		break;
	case BRAM_TYPE::FM:
		break;
	case BRAM_TYPE::MM:
		break;
	case BRAM_TYPE::IOM:
		break;
	case BRAM_TYPE::PMU_BUF:
		break;
	case BRAM_TYPE::MEM_PERIOD:
		break;
	case BRAM_TYPE::MEM_RSU:
		break;
	case BRAM_TYPE::MEM_HIS:
		break;
	case BRAM_TYPE::MEM_TIMING:
		break;
	default:
		return -1;
		break;
	}
	if (0 == uReadDataCount || nullptr == pulDataBuff)
	{
		return -2;
	}
	CBaseOperation* pRAMOperation = GetOperation(OperationType);
	if (nullptr == pRAMOperation)
	{
		return 0x80000000;
	}
	pRAMOperation->SetAddress((int)RAMType, usRAMAddress);

	return pRAMOperation->Read(uReadDataCount, pulDataBuff);
}

int COperation::WriteRAM(BRAM_TYPE RAMType, USHORT usRAMAddress, UINT uWriteDataCount, const ULONG* pulData)
{
	OPERATION_TYPE OperationType = BRAM;
	switch (RAMType)
	{
	case BRAM_TYPE::IMM1:
		break;
	case BRAM_TYPE::IMM2:
		break;
	case BRAM_TYPE::IMM3:
		break;
	case BRAM_TYPE::FM:
		break;
	case BRAM_TYPE::MM:
		break;
	case BRAM_TYPE::IOM:
		break;
	case BRAM_TYPE::PMU_BUF:
		break;
	case BRAM_TYPE::MEM_PERIOD:
		break;
	case BRAM_TYPE::MEM_RSU:
		break;
	case BRAM_TYPE::MEM_HIS:
		break;
	case BRAM_TYPE::MEM_TIMING:
		break;
	default:
		return -1;
		break;
	}
	if (0 == uWriteDataCount || nullptr == pulData)
	{
		return -2;
	}

	CBaseOperation* pRAMOperation = GetOperation(OperationType);
	if (nullptr == pRAMOperation)
	{
		return 0x80000000;
	}
	pRAMOperation->SetAddress((int)RAMType, usRAMAddress);

	return pRAMOperation->Write(uWriteDataCount, pulData);
}

int COperation::ReadDRAM(DRAM_TYPE DRAMType, UINT uStartLineNo, UINT uReadDataCount, ULONG* pulDataBuff)
{
	OPERATION_TYPE OperationType = DRAM;
	switch (DRAMType)
	{
	case DRAM_TYPE::FM:
		break;
	case DRAM_TYPE::MM:
		break;
	case DRAM_TYPE::IOM:
		break;
	case DRAM_TYPE::CMD:
		break;
	case DRAM_TYPE::DRAM5:
		break;
	case DRAM_TYPE::DRAM6:
		break;
	default:
		return -1;
		break;
	}
	if (0 == uReadDataCount || nullptr == pulDataBuff)
	{
		return -2;
	}

	CBaseOperation* pDRAMOperation = GetOperation(OperationType);
	if (nullptr == pDRAMOperation)
	{
		return 0x80000000;
	}
	int nRetVal = pDRAMOperation->SetStartLineNo((int)DRAMType, uStartLineNo);
	if (0 != nRetVal)
	{
		return -3;
	}

	nRetVal = pDRAMOperation->Read(uReadDataCount, pulDataBuff);

	switch (nRetVal)
	{
	case 0:
		break;
	case -3:
		nRetVal = -4;
		break;
	default:
		nRetVal = 0x80000001;
		break;
	}
	return nRetVal;
}

int COperation::WriteDRAM(DRAM_TYPE DRAMType, UINT uStartLineNo, UINT uWriteDataCount, ULONG* pulData)
{
	OPERATION_TYPE OperationType = DRAM;
	switch (DRAMType)
	{
	case DRAM_TYPE::FM:
		break;
	case DRAM_TYPE::MM:
		break;
	case DRAM_TYPE::IOM:
		break;
	case DRAM_TYPE::CMD:
		break;
	case DRAM_TYPE::DRAM5:
		break;
	case DRAM_TYPE::DRAM6:
		break;
	default:
		return -1;
		break;
	}
	if (0 == uWriteDataCount || nullptr == pulData)
	{
		return -2;
	}

	CBaseOperation* pDRAMOperation = GetOperation(OperationType);
	if (nullptr == pDRAMOperation)
	{
		return 0x80000000;
	}
	int nRetVal = pDRAMOperation->SetStartLineNo((int)DRAMType, uStartLineNo);
	if (0 != nRetVal)
	{
		return -3;
	}

	nRetVal = pDRAMOperation->Write(uWriteDataCount, pulData);
	switch (nRetVal)
	{
	case 0:
		break;
	case -3:
		nRetVal = -4;
		break;
	default:
		nRetVal = 0x80000001;
		break;
	}
	return nRetVal;
}

int COperation::IsDRAMReady()
{
	CBaseOperation* pCom = GetOperation(COM_TYPE);
	if (nullptr == pCom)
	{
		return 1;
	}
	DWORD dwData = pCom->Read(0x0406);
	if (0 != dwData)
	{
		return 0;
	}
	return 1;
}

int COperation::Get305Status()
{
	CBaseOperation* pATE305 = GetOperation(ATE305_TYPE);
	if (nullptr == pATE305)
	{
		return 0x80000000;
	}
	return pATE305->GetStatus();
}

int COperation::Read305(BYTE byAddress, std::map<USHORT, ULONG>& mapChannelData)
{
	CBaseOperation* pATE305 = GetOperation(ATE305_TYPE);
	if (nullptr == pATE305)
	{
		return 0x80000000;
	}
	return pATE305->Read(byAddress, mapChannelData);
}

int COperation::Write305(BYTE byAddress, std::map<USHORT, ULONG>& mapChannelData)
{
	CBaseOperation* pATE305 = GetOperation(ATE305_TYPE);
	if (nullptr == pATE305)
	{
		return 0x80000000;
	}
	return pATE305->Write(byAddress, mapChannelData);
}

int COperation::PMUStart(int nSampleDepth)
{
	CBaseOperation* pAD7606 = GetOperation(AD7606_TYPE);
	if (nullptr == pAD7606)
	{
		return 0x80000000;
	}
	return pAD7606->Write(0, (UINT)nSampleDepth);
}

int COperation::Write7606(ULONG ulData)
{
	CBaseOperation* pAD7606 = GetOperation(AD7606_TYPE);
	if (nullptr == pAD7606)
	{
		return 0x80000000;
	}
	return pAD7606->Write(0, ulData);
}

int COperation::Read7606(USHORT usChannel, int sampDepth, ULONG *pBuff)
{
	CBaseOperation* pAD7606 = GetOperation(AD7606_TYPE);
	if (nullptr == pAD7606)
	{
		return 0x80000000;
	}
	return pAD7606->Read(usChannel, (UINT)sampDepth, pBuff);
}

ULONG COperation::ReadRegister(REG_TYPE RegisterType, USHORT usREGAddress)
{
	OPERATION_TYPE OperationType = SYS_REG_TYPE;
	switch (RegisterType)
	{
	case REG_TYPE::SYS_REG:
		OperationType = SYS_REG_TYPE;
		break;
	case REG_TYPE::FUNC_REG:
		OperationType = FUNC_REG_TYPE;
		break;
	case REG_TYPE::TMU_REG:
		OperationType = TMU_REG_TYPE;
		break;
	case REG_TYPE::CAL_REG:
		OperationType = CAL_REG_TYPE;
		break;
	default:
		return -1;
		break;
	}
	CBaseOperation* pREGOperation = GetOperation(OperationType);
	if (nullptr == pREGOperation)
	{
		return -1;
	}
	return pREGOperation->Read(usREGAddress);
}

int COperation::WriteRegister(REG_TYPE RegisterType, USHORT usREGAddress, ULONG ulData)
{
	OPERATION_TYPE OperationType = SYS_REG_TYPE;
	switch (RegisterType)
	{
	case REG_TYPE::SYS_REG:
		OperationType = SYS_REG_TYPE;
		break;
	case REG_TYPE::FUNC_REG:
		OperationType = FUNC_REG_TYPE;
		break;
	case REG_TYPE::TMU_REG:
		OperationType = TMU_REG_TYPE;
		break;
	case REG_TYPE::CAL_REG:
		OperationType = CAL_REG_TYPE;
		break;
	default:
		return -1;
		break;
	}
	CBaseOperation* pREGOperation = GetOperation(OperationType);
	if (nullptr == pREGOperation)
	{
		return 0x80000000;
	}
	return pREGOperation->Write(usREGAddress, ulData);
}

int COperation::ReadBoard(USHORT usAddress, UINT uReadDataCount, ULONG* pulDataBuff)
{
	if (0 == uReadDataCount || nullptr == pulDataBuff)
	{
		return -1;
	}
	OPERATION_TYPE OperationType = BOARD_TYPE;
	CBaseOperation* pREGOperation = GetOperation(OperationType);

	if (nullptr == pREGOperation)
	{
		return 0x80000000;
	}
	int nRetVal = pREGOperation->Read(usAddress, uReadDataCount, pulDataBuff);
	if (0 != nRetVal)
	{
		nRetVal = 0x80000001;
	}
	return nRetVal;
}

int COperation::WriteBoard(USHORT usAddress, UINT uWriteDataCount, ULONG* pulData)
{
	if (0 == uWriteDataCount || nullptr == pulData)
	{
		return -1;
	}
	OPERATION_TYPE OperationType = BOARD_TYPE;
	CBaseOperation* pREGOperation = GetOperation(OperationType);
	if (nullptr == pREGOperation)
	{
		return 0x80000000;
	}
	int nRetVal = pREGOperation->Write(usAddress, uWriteDataCount, pulData);
	if (0 != nRetVal)
	{
		nRetVal = 0x80000001;
	}
	return nRetVal;
}

void COperation::WaitUs(double dUs)
{	
	m_mapOperation.begin()->second->WaitUs(dUs);
}

ULONG COperation::ReadBoard(USHORT usAddress)
{
	ULONG ulData = 0;
	int nRetVal = ReadBoard(usAddress, 1, &ulData);
	if (0 != nRetVal)
	{
		return 0x80000000;
	}
	return ulData;
}

int COperation::WriteBoard(USHORT usAddress, ULONG ulData)
{
	return WriteBoard(usAddress, 1, &ulData);
}

inline CBaseOperation* COperation::GetOperation(OPERATION_TYPE OperationType)
{
	auto iterOperation = m_mapOperation.find(OperationType);
	if (m_mapOperation.end() == iterOperation)
	{
		return nullptr;
	}
	return iterOperation->second;
}
