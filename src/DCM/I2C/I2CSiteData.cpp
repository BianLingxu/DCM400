#include "I2CSiteData.h"
using namespace std;
CI2CSiteData::CI2CSiteData(CDriverAlarm* pAlarm)
{
	m_uDataByteCount = 0;
	m_pAlarm = pAlarm;
	m_bSiteMD5 = FALSE;
	MD5_Init(&m_MD5Context);
}

CI2CSiteData& CI2CSiteData::operator=(const CI2CSiteData& SiteData)
{
	CI2CSiteData TempSiteData(m_pAlarm);
	TempSiteData.SetDataByteCount(m_uDataByteCount);

	for (auto& CurSiteData : SiteData.m_mapSiteData)
	{
		TempSiteData.SetSiteData(CurSiteData.first, CurSiteData.second, SiteData.m_uDataByteCount);
	}

	for (auto& CurSiteData : m_mapSiteData)
	{
		if (nullptr != CurSiteData.second)
		{
			delete[] CurSiteData.second;
			CurSiteData.second = nullptr;
		}
	}
	m_mapSiteData.clear();

	for (auto& CurSiteData : TempSiteData.m_mapSiteData)
	{
		m_mapSiteData.insert(make_pair(CurSiteData.first, CurSiteData.second));
		CurSiteData.second = nullptr;
	}

	m_mapSameData.clear();

	for (auto& CurSiteData : SiteData.m_mapSameData)
	{
		m_mapSameData.insert(make_pair(CurSiteData.first, CurSiteData.second));
	}
	m_uDataByteCount = SiteData.m_uDataByteCount;

	return *this;
}
CI2CSiteData::~CI2CSiteData()
{
	for (auto& SiteData : m_mapSiteData)
	{
		if (nullptr != SiteData.second)
		{
			delete[] SiteData.second;
			SiteData.second = nullptr;
		}
	}
	m_mapSiteData.clear();
	m_mapSameData.clear();
	m_uDataByteCount = 0;
}

int CI2CSiteData::SetDataByteCount(UINT uDataByteCount)
{
	if (m_uDataByteCount != uDataByteCount)
	{
		if (0 != m_uDataByteCount && 0 != m_mapSiteData.size())
		{
			return -1;
		}
	}
	m_uDataByteCount = uDataByteCount;
	return 0;
}

UINT CI2CSiteData::GetDataByteCount()
{
	return m_uDataByteCount;
}

int CI2CSiteData::SetSiteData(USHORT usSiteNo, const BYTE* pbyData, UINT uDataByteCount)
{
	if (nullptr == pbyData || 0 == uDataByteCount)
	{
		return -1;
	}
	if (0 == m_uDataByteCount)
	{
		m_uDataByteCount = uDataByteCount;
	}
	else if(uDataByteCount != m_uDataByteCount)
	{
		return -2;
	}
	auto iterSiteData = m_mapSiteData.find(usSiteNo);
	if (m_mapSiteData.end() != iterSiteData)
	{
		return -3;
	}

	if (m_mapSameData.end() != m_mapSameData.find(usSiteNo))
	{
		return -3;
	}
	if (m_bSiteMD5)
	{
		MD5_Init(&m_MD5Context);
		m_bSiteMD5 = FALSE;
	}
	
	BOOL bFindData = FALSE;
	for (auto& SiteData : m_mapSiteData)
	{
		if (0 == memcmp(SiteData.second, pbyData, uDataByteCount))
		{
			bFindData = TRUE;
			m_mapSameData.insert(make_pair(usSiteNo, SiteData.first));
			break;
		}
	}
	if (bFindData)
	{
		return 0;
	}
	BYTE* pbySaveData = nullptr;
	try
	{
		pbySaveData = new BYTE[m_uDataByteCount];
	}
	catch (const std::exception&)
	{
		m_pAlarm->SetAlarmID(ALARM_ID::ALARM_ALLOCTE_MEMORY_ERROR);
		m_pAlarm->SetAlarmMsg("Allocate %d bytes memory fail.", m_uDataByteCount * sizeof(BYTE));
		return -4;
	}
	memcpy_s(pbySaveData, m_uDataByteCount, pbyData, m_uDataByteCount);
	m_mapSiteData.insert(make_pair(usSiteNo, pbySaveData));
	return 0;
}

int CI2CSiteData::GetSiteData(USHORT usSiteNo, UINT uDataByteIndex) const
{
	BYTE* pData = nullptr;
	   	 
	auto iterSiteData = m_mapSiteData.find(usSiteNo);
	if (m_mapSiteData.end() != iterSiteData)
	{
		pData = iterSiteData->second;
	}
	else
	{
		auto iterSameData = m_mapSameData.find(usSiteNo);
		if (m_mapSameData.end() != iterSameData)
		{
			iterSiteData = m_mapSiteData.find(iterSameData->second);
			assert(m_mapSiteData.end() != iterSiteData);
			pData = iterSiteData->second;
		}
		else
		{
			return -1;
		}
	}
	if (uDataByteIndex >= m_uDataByteCount)
	{
		return -2;
	}

	return pData[uDataByteIndex];
}

void CI2CSiteData::SetNACK(USHORT usSiteNo, int nNACKIndex)
{
	auto iterSite = m_mapNACK.find(usSiteNo);
	if (m_mapNACK.end() == iterSite)
	{
		m_mapNACK.insert(make_pair(usSiteNo, -1));
		iterSite = m_mapNACK.find(usSiteNo);
	}
	iterSite->second = nNACKIndex;
}

int CI2CSiteData::GetNACK(USHORT usSiteNo)
{
	auto iterSite = m_mapNACK.find(usSiteNo);
	if (m_mapNACK.end() == iterSite)
	{
		return -1;
	}
	return iterSite->second;
}

BOOL CI2CSiteData::IsDataAllSame()
{
	if (1 == m_mapSiteData.size())
	{
		return TRUE;
	}
	return FALSE;
}

int CI2CSiteData::GetSiteData(USHORT usSiteNo, std::vector<BYTE>& vecData)
{
	vecData.clear();

	const BYTE* pbyData = nullptr;

	auto iterSiteData = m_mapSiteData.find(usSiteNo);
	if (m_mapSiteData.end() != iterSiteData)
	{
		pbyData = iterSiteData->second;
	}
	else
	{
		auto iterSameData = m_mapSameData.find(usSiteNo);
		if (m_mapSameData.end() != iterSameData)
		{
			iterSiteData = m_mapSiteData.find(iterSameData->second);
			assert(m_mapSiteData.end() != iterSiteData);
			pbyData = iterSiteData->second;
		}
		else
		{
			return -1;
		}
	}
	for (UINT uDataIndex = 0; uDataIndex < m_uDataByteCount;++uDataIndex)
	{
		vecData.push_back(pbyData[uDataIndex]);
	}
	return 0;
}

int CI2CSiteData::GetDataKey(std::string& strKey)
{
	strKey.clear();

	USHORT usSiteCount = m_mapSiteData.size() + m_mapSameData.size();
	if (0 == usSiteCount)
	{
		return -1;
	}

	char lpszKey[256] = { 0 };
	BOOL bAllDataSame = (1 == m_mapSiteData.size() ? TRUE : FALSE);
	sprintf_s(lpszKey, sizeof(lpszKey), "%d|", bAllDataSame);
	strKey += lpszKey;
	auto iterData = m_mapSiteData.begin();
	auto iterSameData = m_mapSameData.begin();
	const BYTE* pbyData = nullptr;
	
	for (USHORT usSiteNo = 0; usSiteNo < usSiteCount; ++usSiteNo)
	{
		if (bAllDataSame)
		{
			usSiteNo = m_mapSiteData.begin()->first;
		}

		iterData = m_mapSiteData.find(usSiteNo);
		if (m_mapSiteData.end() != iterData)
		{
			pbyData = iterData->second;
		}
		else
		{
			iterSameData = m_mapSameData.find(usSiteNo);
			if (m_mapSameData.end() == iterSameData)
			{
				continue;
			}
			iterData = m_mapSiteData.find(iterSameData->second);
			if (m_mapSiteData.end() == iterData)
			{
				continue;
			}
			pbyData = iterData->second;
		}
		sprintf_s(lpszKey, sizeof(lpszKey), "%d:", usSiteNo);
		strKey += lpszKey;

		for (UINT uDataIndex = 0; uDataIndex < m_uDataByteCount; ++uDataIndex)
		{
			sprintf_s(lpszKey, sizeof(lpszKey), "%02X", pbyData[uDataIndex]);
			strKey += lpszKey;
		}
		if (bAllDataSame)
		{
			break;
		}
		else if (usSiteNo + 1 != usSiteCount)
		{
			strKey += "|";
		}
	}
	strKey += ";";
	return 0;
}

void CI2CSiteData::Reset()
{
	for (auto& SiteData : m_mapSiteData)
	{
		if (nullptr != SiteData.second)
		{
			delete[] SiteData.second;
			SiteData.second = nullptr;
		}
	}
	m_mapSiteData.clear();
	m_mapSameData.clear();
	m_mapNACK.clear();
	m_uDataByteCount = 0;
}

int CI2CSiteData::SetMD5Data(const unsigned char* pucData, int nDataSize)
{
	if (nullptr == pucData || 0 >= nDataSize)
	{
		return -1;
	}
	MD5_Update(&m_MD5Context, pucData, nDataSize);
	return 0;
}

void CI2CSiteData::GetMD5(std::string& strData)
{
	strData.clear();
	
	GetSiteDataMD5();
	unsigned char ucMD5[16] = { 0 };
	MD5_Final(&m_MD5Context, ucMD5);
	char lpszData[8] = { 0 };
	for (int nDataIndex = 0; nDataIndex < 16; ++nDataIndex)
	{
		sprintf_s(lpszData, sizeof(lpszData), "%02X", ucMD5[nDataIndex]);
		strData += lpszData;
	}
}


void CI2CSiteData::GetSiteDataMD5()
{
	const BYTE* pbySiteData = nullptr;
	auto iterSiteData = m_mapSiteData.begin();
	auto iterSameData = m_mapSameData.begin();
	USHORT usSiteCount = m_mapSameData.size() + m_mapSiteData.size();
	for (USHORT usSiteNo = 0; usSiteNo < usSiteCount;++usSiteNo)
	{
		iterSiteData = m_mapSiteData.find(usSiteNo);
		if (m_mapSiteData.end() == iterSiteData)
		{
			iterSameData = m_mapSameData.find(usSiteNo);
			if (m_mapSameData.end() == iterSameData)
			{
				continue;
			}
			iterSiteData = m_mapSiteData.find(iterSameData->second);
			if (m_mapSiteData.end() == iterSiteData)
			{
				continue;
			}
		}
		pbySiteData = iterSiteData->second;
		SetMD5Data(pbySiteData, m_uDataByteCount);
	}
	m_bSiteMD5 = TRUE;
}
