#ifndef __LayerExImage__
#define __LayerExImage__

#include "../layerExDraw/LayerExBase.hpp"

/**
 * ���C���g�� �C���[�W����p�l�C�e�B�u�C���X�^���X
 */
class layerExImage : public layerExBase
{
public:
	// �R���X�g���N�^
	layerExImage(DispatchT obj) : layerExBase(obj) {}

	virtual void reset();
	
	/**
	 * ���b�N�A�b�v�e�[�u�����f
	 * @param pLut lookup table
	 */
	void lut(BYTE* pLut);
	
	/**
	 * ���x�ƃR���g���X�g
	 * @param brightness ���x -255 �` 255, �����̏ꍇ�͈Â��Ȃ�
	 * @param contrast �R���g���X�g -100 �`100, 0 �̏ꍇ�ω����Ȃ�
	 */
	void light(int brightness, int contrast);

	/**
	 * �F���ƍʓx��K��
	 * @param hue �F��
	 * @param sat �ʓx
	 * @param blend �u�����h 0 (���ʂȂ�) �` 1 (full effect)
	 */
	void colorize(int hue, int sat, double blend);

	/**
	 * �F���ƍʓx�ƋP�x����
	 * @param hue �F�� -180�`180 (�x)
	 * @param saturation �ʓx -100�`100 (%)
	 * @param luminance �P�x -100�`100 (%)
	 */
	void modulate(int hue, int saturation, int luminance);
	
	/**
	 * �m�C�Y�ǉ�
	 * @param level �m�C�Y���x�� 0 (no noise) �` 255 (lot of noise).
	 */
	void noise(int level);

	/**
	 * �m�C�Y�����i���̉摜�𖳎����ăO���[�X�P�[���̃z���C�g�m�C�Y��`��^�����͈ێ��j
	 */
	void generateWhiteNoise();

	/**
	 * �K�E�X�ڂ���
	 * @param radius �ڂ����x����
	 */
	void gaussianBlur(float radius);
};

#endif