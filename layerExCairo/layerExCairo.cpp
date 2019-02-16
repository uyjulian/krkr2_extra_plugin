#include "layerExCairo.hpp"

/**
 * �R���X�g���N�^
 */
layerExCairo::layerExCairo(DispatchT obj) : layerExBase(obj)
{
	width = 0;
	height = 0;
	buffer = 0;
	pitch = 0;
	surface = NULL;
	cairo = NULL;
}

/**
 * �f�X�g���N�^
 */
layerExCairo::~layerExCairo()
{
	cairo_destroy(cairo);
	cairo_surface_destroy(surface);
}

/**
 * ���Z�b�g����
 */
void
layerExCairo::reset()
{
	// ��{����
	layerExBase::reset();

	// ���C���̏��ύX�����������ǂ���
	reseted = (width  != _width ||
			   height != _height ||
			   buffer != _buffer ||
			   pitch  != _pitch);

	if (reseted) {
		width  = _width;
		height = _height;
		buffer = _buffer;
		pitch  = _pitch;
		// cairo �p�R���e�L�X�g�̍Đ���
		cairo_destroy(cairo);
		cairo_surface_destroy(surface);
		surface = cairo_image_surface_create_for_data((BYTE*)_buffer, CAIRO_FORMAT_ARGB32, width, height, pitch);
		cairo = cairo_create(surface);
		if (cairo_status(cairo) != CAIRO_STATUS_SUCCESS) {
			TVPThrowExceptionMessage(L"can't create cairo context");
			cairo_destroy(cairo);
			cairo = NULL;
		}
	}
}
