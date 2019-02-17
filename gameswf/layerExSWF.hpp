#ifndef _layExSWF_hpp_
#define _layExSWF_hpp_

#include "layerExCairo.hpp"
#include "SWFMovie.hpp"

/*
 * SWF 描画用レイヤ
 */
struct layerExSWF : public layerExCairo
{
public:
	layerExSWF(DispatchT obj);
	~layerExSWF();

	/**
	 * SWF 描画
	 */
	void drawSWF(SWFMovie *swf);
};

#endif
