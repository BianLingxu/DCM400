#ifndef __550F01E4_46B4_48AF_A8F0_55E42E82505D_HDDEBUGTOOLITEMPMUINTERVAL_H__
#define __550F01E4_46B4_48AF_A8F0_55E42E82505D_HDDEBUGTOOLITEMPMUINTERVAL_H__

#include "HDDebugToolItem.h"

class HDDebugToolItemPmuInterval : public HDDebugToolItem
{
public :
	HDDebugToolItemPmuInterval(HDDebugTool * debugTool);
	~HDDebugToolItemPmuInterval();

	virtual int Type() const;
	virtual const char * Name() const;

	virtual int GetData(int site, int logicChannel, STSVariant & data, STSVariant & mark) const;
};

#endif /*__550F01E4_46B4_48AF_A8F0_55E42E82505D_HDDEBUGTOOLITEMPMUINTERVAL_H__*/