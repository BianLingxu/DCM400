#ifndef __A3CEBD0B_E666_428C_8F74_F9CEB183BC84_HDDEBUGTOOLITEMPMUFORCE_H__
#define __A3CEBD0B_E666_428C_8F74_F9CEB183BC84_HDDEBUGTOOLITEMPMUFORCE_H__

#include "HDDebugToolItem.h"

class HDDebugToolItemPmuForce : public HDDebugToolItem
{
public :
	HDDebugToolItemPmuForce(HDDebugTool * debugTool);
	~HDDebugToolItemPmuForce();

	virtual int Type() const;
	virtual const char * Name() const;

	virtual int GetData(int site, int logicChannel, STSVariant & data, STSVariant & mark) const;
};

#endif /*__A3CEBD0B_E666_428C_8F74_F9CEB183BC84_HDDEBUGTOOLITEMPMUFORCE_H__*/