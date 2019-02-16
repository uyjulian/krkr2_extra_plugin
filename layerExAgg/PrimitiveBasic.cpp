#include "Primitive.hpp"

/**
 * ��{�v���~�e�B�u�ێ��p
 */
class AGGBasic : public AGGPrimitive
{
public:
	static const tjs_char *getTypeName() { return L"Basic"; }

public:
	/**
	 * �`�揈��
	 * @param rb �x�[�X�����_��
	 * @param mtx ��{�A�t�B���ό`
	 */
	void paint(renderer_base &rb, agg::trans_affine &mtx) {

		rasterizer_scanline ras;
		scanline sl;
		renderer_scanline ren(rb);

		// �ό`����
		agg::trans_affine selfMtx;
		selfMtx *= agg::trans_affine_scaling(_scale);
		selfMtx *= agg::trans_affine_rotation(agg::deg2rad(_rotate));
		selfMtx *= agg::trans_affine_translation(_x, _y);

		// �S�̕ό`
		selfMtx *= mtx;

		// ���̊g��
		_path.expand(_expand);

		// �`��
		_path.render(ras, sl, ren, selfMtx, rb.clip_box(), 1.0);
	}


public:
	/// �R���X�g���N�^
	AGGBasic(NI_AGGPrimitive *owner, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj);
	
};

// --------------------------------------------------------------------
// �v���~�e�B�u�ŗL���\�b�h�̒�`
// --------------------------------------------------------------------

PRIMFUNC(tLoadMethod,1,AGGBasic)
PRIMFUNCEND
tLoadMethod loadMethod;

/**
 * �R���X�g���N�^
 * @param owner AGGPrimitive �̃l�C�e�B�u�I�u�W�F�N�g
 * @param numparams �R���X�g���N�^�p�����[�^��
 * @param param �R���X�g���N�^�p�����[�^
 * @param tjs_obj �����C���X�^���X
 */
AGGBasic::AGGBasic(NI_AGGPrimitive *owner, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj) : AGGPrimitive(owner)
{
	// tjs_obj �Ƀ��\�b�h�ǉ� 

	addMember(tjs_obj, L"load", &loadMethod);
}
