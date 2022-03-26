#include "pch.h"
#include "ConfigFile.h"
#include <fstream>
#include <windows.h>
using namespace std;
CConfigFile::CConfigFile(const char* lpszFile)
{
	if (nullptr == lpszFile)
	{
		return;
	}
	m_strFile = lpszFile;
	
	ifstream ConfigFile(m_strFile.c_str());
	if (!ConfigFile.is_open())
	{
		return;
	}
	CSection* pSection = nullptr;
	int nPos = 0;
	string strLine;
	while (getline(ConfigFile, strLine))
	{
		nPos = strLine.find("[");
		if (-1 != nPos)
		{
			if (nullptr != pSection)
			{
				m_mapSection.insert(make_pair(pSection->Name(), pSection));
				pSection = nullptr;
			}
			strLine.erase(0, nPos + 1);
			nPos = strLine.find("]");
			if (-1 != nPos)
			{
				strLine.erase(nPos);
			}
			pSection = new CSection(strLine.c_str());
			continue;
		}
		nPos = strLine.find("=");
		if (nullptr != pSection && -1 != nPos)
		{
			string strKey = strLine.substr(0, nPos);
			pSection->SetKey(strLine.substr(0, nPos).c_str(), strLine.substr(nPos + 1).c_str());
		}
	}
	if (nullptr != pSection)
	{
		m_mapSection.insert(make_pair(pSection->Name(), pSection));
	}
	ConfigFile.close();
}

CConfigFile::~CConfigFile()
{
	for (auto& Section : m_mapSection)
	{
		if (nullptr != Section.second)
		{
			delete Section.second;
			Section.second = nullptr;
		}
	}
}

int CConfigFile::SetValue(const char* lpszSection, const char* lpszKey, const char* lpszFormat, ...)
{
	if (nullptr == lpszSection)
	{
		return -1;
	}
	if (nullptr == lpszKey)
	{
		return -2;
	}
	if (nullptr == lpszFormat)
	{
		return -3;
	}
	CSection* pSection = nullptr;
	auto iterSection = m_mapSection.find(lpszSection);
	if (m_mapSection.end() == iterSection)
	{
		pSection = new CSection(lpszSection);
		m_mapSection.insert(make_pair(lpszSection, pSection));
	}
	else
	{
		pSection = iterSection->second;
	}

	char lpszData[512] = { 0 };
	int nBuffSize = sizeof(lpszData);
	char* lpszBuff = lpszData;
	int nFormatSize = strlen(lpszFormat);
	BOOL bAllocate = FALSE;
	if (nBuffSize < nFormatSize)
	{
		bAllocate = TRUE;
		nBuffSize = nFormatSize + 10;
		lpszBuff = new char[nBuffSize];
		memset(lpszBuff, 0, nBuffSize);
	}
	va_list ap;
	va_start(ap, lpszFormat);
	vsprintf_s(lpszBuff, nBuffSize, lpszFormat, ap);
	va_end(ap);
	pSection->SetKey(lpszKey, lpszBuff);
	if (bAllocate && nullptr != lpszBuff)
	{
		delete[] lpszBuff;
		lpszBuff = nullptr;
	}
	return 0;
}

const char* CConfigFile::GetValue(const char* lpszSection, const char* lpszKey)
{
	if (nullptr == lpszSection || nullptr == lpszKey)
	{
		return nullptr;
	}
	auto iterSection = m_mapSection.find(lpszSection);
	if (m_mapSection.end() == iterSection || nullptr == iterSection->second)
	{
		return nullptr;
	}
	return iterSection->second->GetValue(lpszKey);
}

int CConfigFile::ClearSection(const char* lpszSection)
{
	if (nullptr == lpszSection)
	{
		return -1;
	}
	auto iterSection = m_mapSection.find(lpszSection);
	if (m_mapSection.end() == iterSection)
	{
		return 0;
	}
	if (nullptr != iterSection->second)
	{
		delete iterSection->second;
		iterSection->second = nullptr;
	}
	m_mapSection.erase(iterSection);
	return 0;
}

int CConfigFile::Save()
{
	if (0 == m_strFile.size())
	{
		return -1;
	}
	ofstream ConfigFile(m_strFile.c_str());
	if (!ConfigFile.is_open())
	{
		return -2;
	}
	vector<string> vecKey;
	for (auto& Section : m_mapSection)
	{
		ConfigFile << "[" << Section.first.c_str() << "]"  << endl;
		Section.second->GetKey(vecKey);
		for (auto& Key : vecKey)
		{
			ConfigFile << Key.c_str() << "=" << Section.second->GetValue(Key.c_str()) << endl;
		}
	}
	ConfigFile.close();
	return 0;
}

void CConfigFile::Reset()
{
	for (auto& Section : m_mapSection)
	{
		if (nullptr != Section.second)
		{
			delete Section.second;
			Section.second = nullptr;
		}
	}
	m_mapSection.clear();
}

CSection::CSection(const char* lpszName)
{
	if (nullptr == lpszName)
	{
		return;
	}
	m_strName = lpszName;
}

int CSection::SetKey(const char* lpszKey, const char* lpszValue)
{
	if (nullptr == lpszKey)
	{
		return -1;
	}
	if (nullptr == lpszValue)
	{
		return -2;
	}
	auto iterKey = m_mapKeyValue.find(lpszKey);
	if (m_mapKeyValue.end() == iterKey)
	{
		m_mapKeyValue.insert(make_pair(lpszKey, string()));
		iterKey = m_mapKeyValue.find(lpszKey);
	}
	iterKey->second = lpszValue;
	return 0;
}

const char* CSection::Name() const
{
	return m_strName.c_str();
}

const char* CSection::GetValue(const char* lpszKey) const
{
	if (nullptr == lpszKey)
	{
		return nullptr;
	}
	auto iterKey = m_mapKeyValue.find(lpszKey);
	if (m_mapKeyValue.end() == iterKey)
	{
		return nullptr;
	}
	return iterKey->second.c_str();
}

void CSection::GetKey(std::vector<std::string>& vecKey) const
{
	vecKey.clear();
	for (auto& Key : m_mapKeyValue)
	{
		vecKey.push_back(Key.first);
	}
}
