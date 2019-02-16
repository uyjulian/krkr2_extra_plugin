#include "Primitive.hpp"
#include "agg_svg_parser.h"

/**
 * SVG �ێ��p
 */
class AGGSVG : public AGGPrimitive
{
public:
	static const tjs_char *getTypeName() { return L"SVG"; }

protected:
	/// SVG �p�p�X���
	agg::svg::path_renderer _path;

	// �o�E���f�B���O
	double _min_x;
	double _min_y;
    double _max_x;
    double _max_y;

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
		selfMtx *= agg::trans_affine_translation((_min_x + _max_x) * -0.5, (_min_y + _max_y) * -0.5);
		selfMtx *= agg::trans_affine_scaling(_scale);
		selfMtx *= agg::trans_affine_rotation(agg::deg2rad(_rotate));
		selfMtx *= agg::trans_affine_translation((_min_x + _max_x) * 0.5 + _x, (_min_y + _max_y) * 0.5 + _y);

		// �S�̕ό`
		selfMtx *= mtx;

		// ���̊g��
		_path.expand(_expand);

		// �`��
		_path.render(ras, sl, ren, selfMtx, rb.clip_box(), 1.0);
	}
	
public:
	
	/**
	 * �摜�̃p�[�X
	 */
	void parse(const ttstr &name) {

		// �摜�ǂݍ���
		IStream *in = TVPCreateIStream(name, TJS_BS_READ);
		if(!in) {
			TVPThrowExceptionMessage((ttstr(TJS_W("cannot open : ")) + ttstr(name)).c_str());
		}
		try	{
			// �����_�����O����
			agg::svg::parser p(_path);
			p.parse(in);
			_path.arrange_orientations();
			_path.bounding_rect(&_min_x, &_min_y, &_max_x, &_max_y);
		} catch(agg::svg::exception& e) {
			in->Release();
			TVPThrowExceptionMessage(ttstr(e.msg()).c_str());
		} catch(...) {
			in->Release();
			throw;
		}
		in->Release();

		redraw();
	}

public:
	/// �R���X�g���N�^
	AGGSVG(NI_AGGPrimitive *owner, tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *tjs_obj) : AGGPrimitive(owner) {
		_min_x = 0.0;
		_min_y = 0.0;
		_max_x = 0.0;
		_max_y = 0.0;

		if (numparams > 0) {
			parse(*param[0]);
		}

		// tjs_obj �Ƀ��\�b�h�ǉ� XXX
	}
};

static RegistTypeFactory<AGGSVG> regist;
