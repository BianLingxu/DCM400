#include "Timer.h"
#include <fstream>
using namespace std;
CTimer::CTimer()
{
	QueryPerformanceFrequency(&m_TickFreq);
}

CTimer* CTimer::Instance()
{
	static CTimer CurTimer;
	return &CurTimer;
}

void CTimer::Reset()
{
	while (!m_stackSign.empty())
	{
		m_stackSign.pop();
	}
	while (!m_stackStartTick.empty())
	{
		m_stackStartTick.pop();
	}
	m_vecTime.clear();
	m_vecSign.clear();
}

void CTimer::Start(const char* lpszFormat, ...)
{
	string strMsg = "nullptr";
	if (nullptr != lpszFormat)
	{
		char lpszMsg[256] = { 0 };
		va_list Args;
		va_start(Args, lpszFormat);
		vsprintf_s(lpszMsg, sizeof(lpszMsg), lpszFormat, Args);
		strMsg = lpszMsg;
		va_end(Args);
	}
	m_stackSign.push(strMsg);
	LARGE_INTEGER TickStart;
	QueryPerformanceCounter(&TickStart);
	m_stackStartTick.push(TickStart);
}

double CTimer::Stop()
{
	if (m_stackStartTick.empty())
	{
		return 0;
	}
	LARGE_INTEGER StopTick;
	QueryPerformanceCounter(&StopTick);
	double dTime = double(StopTick.QuadPart - m_stackStartTick.top().QuadPart) / m_TickFreq.QuadPart * 1e6;
	m_stackStartTick.pop();

	m_vecTime.push_back(dTime);
	m_vecSign.push_back(m_stackSign.top());
	m_stackSign.pop();

	return dTime;
}

int CTimer::Print(const char* lpszFilePath)
{
	if (nullptr == lpszFilePath)
	{
		return -1;
	}
	fstream LogFile(lpszFilePath, ios::out | ios::app);
	if (!LogFile.is_open())
	{
		return -2;
	}
	LogFile << "Name, Time/us" << endl;

	char lpszTime[32] = { 0 };

	int nTimerIndex = 0;
	for (auto& strTimer : m_vecSign)
	{
		LogFile << strTimer.c_str() << ",";
		sprintf_s(lpszTime, sizeof(lpszTime), "%.3f", m_vecTime[nTimerIndex++]);
		LogFile << lpszTime << endl;
	}
	LogFile << endl << endl;
	LogFile.close();
	m_vecTime.clear();
	m_vecSign.clear();

	while (!m_stackSign.empty())
	{
		m_stackSign.pop();
	}
	while (!m_stackStartTick.empty())
	{
		m_stackStartTick.pop();
	}
	return 0;
}

