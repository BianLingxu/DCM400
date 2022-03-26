#include "pch.h"
#include "ShieldAlaram.h"
using namespace std;

CShieldPin::CShieldPin(const char* lpszPinName)
{
	if (nullptr != lpszPinName)
	{
		m_strPinName = lpszPinName;
	}
}

void CShieldPin::ShieldSite(USHORT usSiteNo, const CHANNEL_INFO& Channel, BOOL bShield)
{
	auto iterSite = m_mapShieldSite.find(usSiteNo);
	if (m_mapShieldSite.end() == iterSite)
	{
		if (!bShield)
		{
			return;
		}
		m_mapShieldSite.insert(make_pair(usSiteNo, Channel));
	}
	else
	{
		if (!bShield)
		{
			m_mapShieldSite.erase(iterSite);
		}
	}
}

void CShieldPin::GetShieldChannel(std::vector<CHANNEL_INFO>& vecChannel, BOOL bAppend)
{
	if (!bAppend)
	{
		vecChannel.clear();
	}
	for (auto& Channel : m_mapShieldSite)
	{
		vecChannel.push_back(Channel.second);
	}
}

BOOL CShieldPin::IsShield(USHORT usSiteNo)
{
	if (m_mapShieldSite.end() != m_mapShieldSite.find(usSiteNo))
	{
		return TRUE;
	}
	return FALSE;
}

CShieldFunction::CShieldFunction(const char* lpszShieldFunction)
{
	if (nullptr != lpszShieldFunction)
	{
		m_strFunciton = lpszShieldFunction;
	}
}

int CShieldFunction::ShieldPin(const char* lpszPinName, USHORT usSiteNo, const CHANNEL_INFO& Channel, BOOL bShielded)
{
	if (nullptr == lpszPinName)
	{
		return -1;
	}
	auto iterShieldPin = m_mapShieldPin.find(lpszPinName);
	if (m_mapShieldPin.end() == iterShieldPin)
	{
		if (!bShielded)
		{
			return 0;
		}
		CShieldPin* pShieldPin = new CShieldPin(lpszPinName);
		pShieldPin->ShieldSite(usSiteNo, Channel, TRUE);
		m_mapShieldPin.insert(make_pair(lpszPinName, pShieldPin));
	}
	else
	{
		iterShieldPin->second->ShieldSite(usSiteNo, Channel, bShielded);
	}
	return 0;
}

void CShieldFunction::ShieldID(UINT uAlarmID, BOOL bShield)
{
	auto iterShield = m_setID.find(uAlarmID);
	if (m_setID.end() == iterShield)
	{
		if (!bShield)
		{
			return;
		}
		m_setID.insert(uAlarmID);
	}
	else if(!bShield)
	{
		m_setID.erase(iterShield);
	}
}

void CShieldFunction::GetShieldChannel(vector<CHANNEL_INFO>& vecChannel, UINT uAlaramID)
{
	vecChannel.clear();
	if (m_setID.end() == m_setID.find(uAlaramID))
	{
		return;
	}
	for (auto& ShieldPin : m_mapShieldPin)
	{
		if (nullptr != ShieldPin.second)
		{
			ShieldPin.second->GetShieldChannel(vecChannel, TRUE);
		}
	}
}

BOOL CShieldFunction::IsShield(const char* lpszPinName, USHORT usSiteNo, UINT uAlarmID)
{
	if (nullptr == lpszPinName)
	{
		return FALSE;
	}
	if (m_setID.end() == m_setID.find(uAlarmID))
	{
		return FALSE;
	}
	auto iterShieldPin = m_mapShieldPin.find(lpszPinName);
	if (m_mapShieldPin.end() == iterShieldPin || nullptr == iterShieldPin->second)
	{
		return FALSE;
	}
	
	return iterShieldPin->second->IsShield(usSiteNo);
}
