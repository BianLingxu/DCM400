#include "pch.h"
#include "Controller.h"

CController::CController(BYTE bySlotNo, BYTE byIndex, CDriverAlarm* pAlarm)
	: m_pAlarm(pAlarm)
	, m_Hardware(bySlotNo, pAlarm)
	, m_Pattern(m_Hardware)
	, m_bySlotNo(bySlotNo)
	, m_byIndex(byIndex)
{

}

BOOL CController::IsExist()
{
	return m_Hardware.IsControllerExist();
}

CHardwareFunction* CController::GetHardwareFunction()
{
	return &m_Hardware;
}

BOOL CController::IsVectorValid()
{
	return m_Hardware.IsVectorValid();
}

int CController::AddChannelPattern(USHORT usChannel, UINT uPatternLine, char cPattern, const CPatternCMD& PatternCMD)
{
	return m_Pattern.AddChannelPattern(usChannel, uPatternLine, cPattern, PatternCMD);
}

int CController::LoadVector()
{
	return m_Pattern.Load();
}

void CController::SetVectorValid(BOOL bValid)
{

}

int CController::SetPeriodSeries(BYTE bySeries, double dPeriod)
{
	return m_Hardware.SetPeriodSeries(bySeries, dPeriod);
}

int CController::SetEdgeSeries(const std::vector<USHORT>& vecChannel, BYTE bySeries, const double* pdEdge)
{
	return m_Hardware.SetEdgeSeries(vecChannel, bySeries, pdEdge);
}

int CController::SetFormatSeries(const std::vector<USHORT>& vecChannel, BYTE bySeries, WAVE_FORMAT WaveFormat, IO_FORMAT IOFormat, COMPARE_MODE CompareMode)
{
	return m_Hardware.SetFormatSeries(vecChannel, bySeries, WaveFormat, IOFormat, CompareMode);
}

int CController::SetTimeSet(const vector<USHORT>& vecChannel, USHORT usTimeSetIndex, BYTE byPeriodSeries, const BYTE* pbyEdgeSeries, BYTE byFormatSeries)
{
	return m_Hardware.SetTimeSet(vecChannel, usTimeSetIndex, byPeriodSeries, pbyEdgeSeries, byFormatSeries);
}
