#include "pch.h"
#include "PinGroup.h"
using namespace std;
CPinGroup::CPinGroup(const char* lpszPinGroupName)
{
	if (nullptr != lpszPinGroupName)
	{
		m_strName = lpszPinGroupName;
	}
}

CPinGroup::CPinGroup(const CPinGroup& PinGroup)
{
	m_strName = PinGroup.m_strName;
	m_setPin = PinGroup.m_setPin;
}

int CPinGroup::AddPinName(std::string& strPinName)
{
	BOOL bExisted = FALSE;
	UINT uPinNameCount = m_setPin.size();
	if (m_setPin.end() != m_setPin.find(strPinName))
	{
		return -1;
	}
	m_setPin.insert(strPinName);
	return 0;
}

int CPinGroup::SetPinName(std::set<std::string> setPin)
{
	if (0 != m_setPin.size())
	{
		return -1;
	}
	m_setPin = setPin;
	return 0;
}

void CPinGroup::GetPinName(std::set<std::string>& setPinName)
{
	setPinName = m_setPin;
}

BOOL CPinGroup::IsSamePinGroup(const std::set<std::string>& setPinName)
{
	if (setPinName != m_setPin)
	{
		return FALSE;
	}
	return TRUE;
}