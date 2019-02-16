#ifndef _layExCairo_hpp_
#define _layExCairo_hpp_

#include <windows.h>
#include "layerExBase.hpp"
#include "cairo.h"

/*
 * Cairo �`��p���C��
 */
struct layerExCairo : public layerExBase
{
protected:
	GeometryT width;
	GeometryT height;
	BufferT buffer;
	PitchT pitch;
	cairo_surface_t *surface;
	cairo_t * cairo;
	bool reseted; //< ���Z�b�g�������ꂽ���ǂ���
	
public:
	layerExCairo(DispatchT obj);
	~layerExCairo();
	
public:
	/**
	 * ���C����񃊃Z�b�g����
	 */
	virtual void reset();
};

#endif
