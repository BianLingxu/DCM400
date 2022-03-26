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

CPeriod::~CPeriod()
{
    for (auto& Period : m_mapPeriod)
    {
        if (nullptr != Period.second)
        {
            delete[] Period.second;
            Period.second = nullptr;
        }
    }
    m_mapPeriod.clear();
}

double CPeriod::GetPeriod(BYTE bySlotNo, BYTE byController, BYTE byTimeset)
{
    UINT uControllerID = GetControllerID(bySlotNo, byController);
    auto iterController = m_mapPeriod.find(uControllerID);
    if (m_mapPeriod.end() == iterController)
    {
        return -1;
    }
    else if (TIMESET_COUNT <= byTimeset)
    {
        return -2;
    }
    return iterController->second[byTimeset];
}

inline UINT CPeriod::GetControllerID(BYTE bySlotNo, BYTE byController)
{
    return bySlotNo << 24 | byController;
}

int CPeriod::SetPeriod(BYTE bySlotNo, BYTE byController, BYTE byTimeset, float fPeriod)
{
    if (TIMESET_COUNT <= byTimeset)
    {
        return -1;
    }
	UINT uControllerID = GetControllerID(bySlotNo, byController);
	auto iterController = m_mapPeriod.find(uControllerID);
	if (m_mapPeriod.end() == iterController)
	{
        float* pfPeriod = nullptr;
        try
        {
            pfPeriod = new float[TIMESET_COUNT];
            memset(pfPeriod, -1, TIMESET_COUNT * sizeof(float));
        }
        catch (const std::exception&)
        {
            return -2;
        }
        m_mapPeriod.insert(make_pair(uControllerID, pfPeriod));
        iterController = m_mapPeriod.find(uControllerID);
	}
    iterController->second[byTimeset] = fPeriod;
	return 0;
}