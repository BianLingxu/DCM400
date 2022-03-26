#pragma once
#include "HardwareInfo.h"
#include "StdAfx.h"
#include <vector>
#include <string>
class CEdge
{
public:
	CEdge();
	CEdge(const CEdge& Edge);
	~CEdge();
	void SetPin(const std::vector<std::string>& vecPin);
	int SetEdge(double* pdEdge);
	void SetFormat(WAVE_FORMAT WaveFormat, IO_FORMAT IOFormat, COMPARE_MODE CompareMode);
	void GetPin(std::vector<std::string>& vecPinName) const;
	int GetFormat(WAVE_FORMAT* pWaveFormat, IO_FORMAT* pIOFormat, COMPARE_MODE* pCompareMode) const;
	int GetEdge(double* pdEdge) const;
private:
	double m_dEdge[EDGE_COUNT];
	std::vector<std::string> m_vecPin;
	WAVE_FORMAT m_WaveFormat;
	IO_FORMAT m_IOFormat;
	COMPARE_MODE m_CompareMode;
};

class CTimeset
{
public:
	CTimeset(const char* lpszName, BYTE byID, double dPeriod);
	~CTimeset();
	BYTE ID();
	void AddEdge(CEdge& Edge);
	int GetEdge(int nIndex, CEdge& Edge);
	int GetEdgeCount();
	void ClearEdge();
	double GetPeriod();
	const std::string& Name();
private:
	double m_dPeriod;
	std::string m_strName;
	BYTE m_byID;
	std::vector<CEdge*> m_vecEdge;
};

