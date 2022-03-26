#include "ClassDistribution.h"
#include "DriverAlarm.h"
using namespace std;
template<typename T> CClassDistribution<T>* CClassDistribution<T>::Instance()
{
	static CClassDistribution<T> ClassDistribution;
	return &ClassDistribution;
}

template<typename T> CClassDistribution<T>::~CClassDistribution()
{
	for (auto& Instance : m_mapClass)
	{
		if (nullptr != Instance.second)
		{
			delete Instance.second;
			Instance.second = nullptr;
		}
	}
	m_mapClass.clear();
	m_mapClassAuthorized.clear();
	DeleteCriticalSection(&m_criAuthority);

}

template<typename T> void CClassDistribution<T>::SetInstanceID(int nInstanceID)
{
	DistributeClass(nInstanceID, nullptr, false);
}

template<typename T> T* CClassDistribution<T>::GetClass(int* pnInstanceID, int nInstanceID)
{
	nInstanceID = 0 > nInstanceID ? m_nAuthorizedInstanceID : nInstanceID;
	return DistributeClass(nInstanceID, pnInstanceID, TRUE);
}

template<typename T> T* CClassDistribution<T>::Initialize(int nInstanceID, BOOL bDelete /*= FALSE*/)
{
	T* pInstance = nullptr;
	EnterCriticalSection(&m_criAuthority);
	auto iterInstance = m_mapClass.find(nInstanceID);
	if (m_mapClass.end() != iterInstance)
	{
		if (bDelete)
		{
			if (m_nAuthorizedInstanceID == nInstanceID)
			{
				m_nAuthorizedInstanceID = 0;
			}
			m_mapClass.erase(iterInstance);
		}
		else
		{
			pInstance = iterInstance->second;
		}
	}
	else if(!bDelete)
	{
		m_nAuthorizedInstanceID = nInstanceID;
		CDriverAlarm* pDriverAlarm = CAlarmManage::Instance()->GetAlarm(m_nAuthorizedInstanceID);
		pInstance = new T(pDriverAlarm);
		m_mapClass.insert(make_pair(m_nAuthorizedInstanceID, pInstance));
	}
	LeaveCriticalSection(&m_criAuthority);
	return pInstance;
}

template<typename T> void CClassDistribution<T>::GetInstanceValid(std::vector<int>& vecInstancID)
{
	vecInstancID.clear();
	EnterCriticalSection(&m_criAuthority);
	for (auto& iterInstance : m_mapClass)
	{
		vecInstancID.push_back(iterInstance.first);
	}
	LeaveCriticalSection(&m_criAuthority);
}

template<typename T> CClassDistribution<T>::CClassDistribution()
	: m_nAuthorizedInstanceID(0)
{
	InitializeCriticalSection(&m_criAuthority);
}

template<typename T> T* CClassDistribution<T>::DistributeClass(int nInstanceID, int* pnInstanceID, BOOL bDelete)
{
	EnterCriticalSection(&m_criAuthority);
	T* pInstance = nullptr;
	DWORD dwThreadID = GetCurrentThreadId();
	auto iterInstance = m_mapClassAuthorized.find(dwThreadID);
	if (m_mapClassAuthorized.end() != iterInstance)
	{
		if (bDelete)
		{
			pInstance = iterInstance->second.m_pClass;
			if (nullptr != pnInstanceID)
			{
				*pnInstanceID = iterInstance->second.m_nInstanceID;
			}
			m_mapClassAuthorized.erase(iterInstance);
		}
		else
		{
			m_nAuthorizedInstanceID = nInstanceID;
			auto iterCurInstance = m_mapClass.find(m_nAuthorizedInstanceID);
			if (m_mapClass.end() != iterCurInstance)
			{
				pInstance = iterCurInstance->second;
			}
			if (nullptr != pnInstanceID)
			{
				*pnInstanceID = m_nAuthorizedInstanceID;
			}

			INSTANCE_INFO InstanceInfo;
			InstanceInfo.m_nInstanceID = m_nAuthorizedInstanceID;
			InstanceInfo.m_pClass = pInstance;

			m_mapClassAuthorized.insert(make_pair(dwThreadID, InstanceInfo));
		}
	}
	else
	{
		if (!bDelete)
		{
			m_nAuthorizedInstanceID = nInstanceID;
		}
		int nCurInstanceID = m_nAuthorizedInstanceID;
		if (m_nAuthorizedInstanceID != nInstanceID)
		{
			nCurInstanceID = nInstanceID;
		}
		if (nullptr != pnInstanceID)
		{
			*pnInstanceID = nCurInstanceID;
		}

		auto iterCurInstance = m_mapClass.find(nCurInstanceID);
		if (m_mapClass.end() != iterCurInstance)
		{
			pInstance = iterCurInstance->second;
			INSTANCE_INFO InstanceInfo;
			InstanceInfo.m_nInstanceID = nCurInstanceID;
			InstanceInfo.m_pClass = pInstance;
			if (!bDelete)
			{
				m_mapClassAuthorized.insert(make_pair(dwThreadID, InstanceInfo));
			}
		}
	}

	LeaveCriticalSection(&m_criAuthority);
	return pInstance;
}
