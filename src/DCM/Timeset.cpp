#include "Timeset.h"

CTimeset::CTimeset(const char* lpszName, BYTE byID, double dPeriod)
{
	if (nullptr != lpszName)
	{
		m_strName = lpszName;
	}
	m_byID = byID;
	m_dPeriod = dPeriod;
}

CTimeset::~CTimeset()
{
	ClearEdge();
}

BYTE CTimeset::ID()
{
	return m_byID;
}

void CTimeset::AddEdge(CEdge& Edge)
{
	CEdge* pEdge = new CEdge(Edge);
	m_vecEdge.push_back(pEdge);
}

int CTimeset::GetEdge(int nIndex, CEdge& Edge)
{
	if (m_vecEdge.size() <= nIndex)
	{
		return -1;
	}
	Edge = *m_vecEdge[nIndex];
	return 0;
}

int CTimeset::GetEdgeCount()
{
	return m_vecEdge.size();
}

void CTimeset::ClearEdge()
{
	int nEdgeCount = m_vecEdge.size();

	for (auto& Edge : m_vecEdge)
	{
		if (nullptr != Edge)
		{
			delete Edge;
			Edge = nullptr;
		}
	}
	m_vecEdge.clear();
}

double CTimeset::GetPeriod()
{
	return m_dPeriod;
}

const std::string& CTimeset::Name()
{
	// TODO: insert return statement here
	return m_strName;
}

CEdge::CEdge()
{
	memset(m_dEdge, 0, sizeof(m_dEdge));
	m_WaveFormat = WAVE_FORMAT::NRZ;
	m_IOFormat = IO_FORMAT::NRZ;
	m_CompareMode = COMPARE_MODE::EDGE;
}

CEdge::CEdge(const CEdge& Edge)
{
	Edge.GetEdge(m_dEdge);
	Edge.GetFormat(&m_WaveFormat, &m_IOFormat, &m_CompareMode);
	Edge.GetPin(m_vecPin);
}

CEdge::~CEdge()
{
	m_vecPin.clear();
}

void CEdge::SetPin(const std::vector<std::string>& vecPinID)
{
	m_vecPin = vecPinID;
}

int CEdge::SetEdge(double* pdEdge)
{
	if (nullptr == pdEdge)
	{
		return -1;
	}
	memcpy_s(m_dEdge, sizeof(m_dEdge), pdEdge, sizeof(m_dEdge));
	return 0;
}

void CEdge::SetFormat(WAVE_FORMAT WaveFormat, IO_FORMAT IOFormat, COMPARE_MODE CompareMode)
{
	m_WaveFormat = WaveFormat;
	m_IOFormat = IOFormat;
	m_CompareMode = CompareMode;
}


void CEdge::GetPin(std::vector<std::string>& vecPinName) const
{
	vecPinName = m_vecPin;
}

int CEdge::GetFormat(WAVE_FORMAT* pWaveFormat, IO_FORMAT* pIOFormat, COMPARE_MODE* pCompareMode) const
{
	if (nullptr == pWaveFormat || nullptr == pIOFormat || nullptr == pCompareMode)
	{
		return -1;
	}
	*pWaveFormat = m_WaveFormat;
	*pIOFormat = m_IOFormat;
	*pCompareMode = m_CompareMode;
	return 0;
}


int CEdge::GetEdge(double* pdEdge) const
{
	if (nullptr == pdEdge)
	{
		return -1;
	}
	memcpy_s(pdEdge, sizeof(m_dEdge), m_dEdge, sizeof(m_dEdge));
	return 0;
}
