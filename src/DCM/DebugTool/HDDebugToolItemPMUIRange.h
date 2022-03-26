#ifndef __CC7FF89D_BFED_4932_9345_3437950A5662_HDDEBUGTOOLITEMPMUIRANGE_H__
#define __CC7FF89D_BFED_4932_9345_3437950A5662_HDDEBUGTOOLITEMPMUIRANGE_H__

#include "HDDebugToolItem.h"

class HDDebugToolItemPmuIRange : public HDDebugToolItem
{
public :
	HDDebugToolItemPmuIRange(HDDebugTool * debugTool);
	~HDDebugToolItemPmuIRange();

	virtual int Type() const;
	virtual const char * Name() const;

	virtual int GetData(int site, int logicChannel, STSVariant & data, STSVariant & mark) const;
};

#endif /*__CC7FF89D_BFED_4932_9345_3437950A5662_HDDEBUGTOOLITEMPMUIRANGE_H__*/