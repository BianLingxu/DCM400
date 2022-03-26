#ifndef __A5EEF54D_E72A_46D1_A20A_BAC97847EBAE_HDDEBUGTOOLITEMPMUSAMPLES_H__
#define __A5EEF54D_E72A_46D1_A20A_BAC97847EBAE_HDDEBUGTOOLITEMPMUSAMPLES_H__

#include "HDDebugToolItem.h"

class HDDebugToolItemPmuSamples : public HDDebugToolItem
{
public :
	HDDebugToolItemPmuSamples(HDDebugTool * debugTool);
	~HDDebugToolItemPmuSamples();

	virtual int Type() const;
	virtual const char * Name() const;

	virtual int GetData(int site, int logicChannel, STSVariant & data, STSVariant & mark) const;
};

#endif /*__A5EEF54D_E72A_46D1_A20A_BAC97847EBAE_HDDEBUGTOOLITEMPMUSAMPLES_H__*/