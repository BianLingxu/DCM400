#include "pch.h"
#include "Period.h"
#include "Bind.h"
#include "HardwareFunction.h"
using namespace std;

CPeriod::CPeriod()
{
}
CPeriod* CPeriod::Instance()
{
    static CPeriod Period;
    return &Period;
}

int CPeriod::SetMemory(BYTE bySlotNo, BYTE byController, float* pfPeriod)
{
    USHORT usID = GetControllerID(bySlotNo, byController);
    auto iterID = m_mapPeriod.find(usID);
    if (m_mapPeriod.end() != iterID)
    {
        return -1;
    }
    m_mapPeriod.insert(make_pair(usID, pfPeriod));
    return 0;
}

CPeriod::~CPeriod()
{
    m_mapPeriod.clear();
}

double CPeriod::GetPeriod(BYTE bySlotNo, BYTE byController, BYTE byTimeset)
{
   USHORT usControllerID = GetControllerID(bySlotNo, byController);
    auto iterController = m_mapPeriod.find(usControllerID);
    if (m_mapPeriod.end() == iterController)
    {
        return -1;
    }
    else if (TIME_SERIES_MAX_COUNT <= byTimeset)
    {
        return -2;
    }
    return iterController->second[byTimeset];
}

inline USHORT CPeriod::GetControllerID(BYTE bySlotNo, BYTE byController)
{
    return bySlotNo << 8 | byController;
}

int CPeriod::SetPeriod(BYTE bySlotNo, BYTE byController, BYTE byTimesetSeriesIndex, float fPeriod)
{
    if (TIME_SERIES_MAX_COUNT <= byTimesetSeriesIndex)
    {
        return -1;
    }
	UINT usControllerID = GetControllerID(bySlotNo, byController);
	auto iterController = m_mapPeriod.find(usControllerID);
	if (m_mapPeriod.end() == iterController)
	{
        float* pfPeriod = nullptr;
        try
        {
            pfPeriod = new float[TIME_SERIES_MAX_COUNT];
            memset(pfPeriod, -1, TIME_SERIES_MAX_COUNT * sizeof(float));
        }
        catch (const std::exception&)
        {
            return -2;
        }
        m_mapPeriod.insert(make_pair(usControllerID, pfPeriod));
        iterController = m_mapPeriod.find(usControllerID);
	}
    iterController->second[byTimesetSeriesIndex] = fPeriod;
	return 0;
}