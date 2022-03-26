#include "I2CManage.h"
#include "I2C.h"
#include "I2CRAM.h"
#include "I2CBoardManage.h"
using namespace std;
CI2CManage* CI2CManage::Instance()
{
	static CI2CManage I2CManage;
	return &I2CManage;
}

CI2CRAM* CI2CManage::GetRAMManage(const CI2C* const pI2C)
{
	DWORD dwAddress = (DWORD)pI2C;
	return (CI2CRAM*)DistributeClass(dwAddress, TRUE, FALSE);
}

CI2CBoardManage* CI2CManage::GetBoardManage(const CI2C* const pI2C)
{
	DWORD dwAddress = (DWORD)pI2C;
	return (CI2CBoardManage*)DistributeClass(dwAddress, FALSE, FALSE);
}

void CI2CManage::DeleteInstance(CI2C* pI2C)
{
	DWORD dwAddress = (DWORD)pI2C;
	DistributeClass(dwAddress, TRUE, TRUE);
	DistributeClass(dwAddress, FALSE, TRUE);
}

CI2CManage::~CI2CManage()
{
	DeleteCriticalSection(&m_criAllocate);
}

CI2CManage::CI2CManage()
{
	InitializeCriticalSection(&m_criAllocate);
}

void* CI2CManage::DistributeClass(DWORD dwAddress, BOOL bRAM, BOOL bDelete)
{
	EnterCriticalSection(&m_criAllocate);
	void* pClass = nullptr;
	if (bRAM)
	{
		auto iterClass = m_mapRAM.find(dwAddress);
		if (m_mapRAM.end() != iterClass)
		{
			if (bDelete)
			{
				delete iterClass->second;
				iterClass->second = nullptr;
				m_mapRAM.erase(iterClass);
			}
			else
			{
				pClass = iterClass->second;
			}
		}
		else
		{
			if (!bDelete)
			{
				pClass = new CI2CRAM;
				m_mapRAM.insert(make_pair(dwAddress, (CI2CRAM*)pClass));
			}
		}
	}
	else
	{
		auto iterClass = m_mapBoard.find(dwAddress);
		if (m_mapBoard.end() != iterClass)
		{
			if (bDelete)
			{
				delete iterClass->second;
				iterClass->second = nullptr;
				m_mapBoard.erase(iterClass);
			}
			else
			{
				pClass = iterClass->second;
			}
		}
		else
		{
			if (!bDelete)
			{
				pClass = new CI2CBoardManage;
				m_mapBoard.insert(make_pair(dwAddress, (CI2CBoardManage*)pClass));
			}
		}
	}
	LeaveCriticalSection(&m_criAllocate);
	return pClass;
}
