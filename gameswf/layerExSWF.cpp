// ---------------------------------------------------------------
// SWF ���[�r�[�`�惌�C��
// ---------------------------------------------------------------

#include "layerExSWF.hpp"

/**
 * �R���X�g���N�^
 */
layerExSWF::layerExSWF(DispatchT obj) : layerExCairo(obj)
{
}

/**
 * �f�X�g���N�^
 */
layerExSWF::~layerExSWF()
{
}

// �`��^�[�Q�b�g
extern cairo_t *ctarget;


/**
 * @param swf ���[�r�[
 * @param advance �o�ߎ���(ms)
 */
void
layerExSWF::drawSWF(SWFMovie *swf)
{
	ctarget = cairo;
	swf->draw(_width, _height);
}
