#ifndef _layExSWF_hpp_
#define _layExSWF_hpp_

#include "layerExCairo.hpp"
#include "SWFMovie.hpp"

/*
 * SWF �`��p���C��
 */
struct layerExSWF : public layerExCairo
{
public:
	layerExSWF(DispatchT obj);
	~layerExSWF();

	/**
	 * SWF �`��
	 */
	void drawSWF(SWFMovie *swf);
};

#endif
