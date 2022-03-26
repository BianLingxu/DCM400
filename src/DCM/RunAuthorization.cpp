#include "RunAuthorization.h"

CRunAuthorization* CRunAuthorization::Instance()
{
	static CRunAuthorization RunAuthorization;
	return &RunAuthorization;
}

void CRunAuthorization::Apply()
{
	EnterCriticalSection(&m_criRun);
}

void CRunAuthorization::Release()
{
	LeaveCriticalSection(&m_criRun);
}

CRunAuthorization::~CRunAuthorization()
{
	DeleteCriticalSection(&m_criRun);
}

CRunAuthorization::CRunAuthorization()
{
	InitializeCriticalSection(&m_criRun);
}
