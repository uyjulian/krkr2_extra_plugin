#include <stdio.h>
#include <windows.h>
#include <list>
#include <map>
using namespace std;

#include "tp_stub.h"

static const char *copyright = 
"----- AntiGrainGeometry Copyright START -----\n"
"Anti-Grain Geometry - Version 2.3\n"
"Copyright (C) 2002-2005 Maxim Shemanarev (McSeem)\n"
"\n"
"Permission to copy, use, modify, sell and distribute this software\n"
"is granted provided this copyright notice appears in all copies. \n"
"This software is provided \"as is\" without express or implied\n"
"warranty, and with no claim as to its suitability for any purpose.\n"
"----- AntiGrainGeometry Copyright END -----\n";

/**
 * ���O�o�͗p
 */
static void log(const tjs_char *format, ...)
{
	va_list args;
	va_start(args, format);
	tjs_char msg[1024];
	_vsnwprintf(msg, 1024, format, args);
	TVPAddLog(msg);
	va_end(args);
}

#include "agg_basics.h"
#include "agg_rendering_buffer.h"
#include "agg_pixfmt_rgba.h"
#include "agg_renderer_base.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_u.h"
#include "agg_renderer_scanline.h"
#include "agg_conv_transform.h"
#include "agg_trans_perspective.h"
#include "agg_span_interpolator_linear.h"
#include "agg_span_interpolator_trans.h"
#include "agg_span_subdiv_adaptor.h"
#include "agg_span_image_filter_rgba.h"

typedef agg::pixfmt_bgra32 pixfmt;
typedef agg::rgba8 color_type;
typedef agg::renderer_base<pixfmt> renderer_base;
typedef agg::renderer_scanline_aa_solid<renderer_base> renderer_solid;
typedef agg::pixfmt_bgra32_pre         pixfmt_pre;
typedef agg::renderer_base<pixfmt_pre> renderer_base_pre;

agg::rasterizer_scanline_aa<> g_rasterizer;
agg::scanline_u8 g_scanline;

#include "LayerExBase.h"

/**
 * �����ϊ��R�s�[
 * @param src
 * @param sleft
 * @param stop
 * @param swidth
 * @param sheight
 * @param x1 �����
 * @param y1
 * @param x2 �E���
 * @param y2
 * @param x3 ������
 * @param y3
 * @param x4 �E����
 * @param y4
 */
class tPerspectiveCopy : public tTJSDispatch
{
protected:
public:
	tjs_error TJS_INTF_METHOD FuncCall(
		tjs_uint32 flag, const tjs_char * membername, tjs_uint32 *hint,
		tTJSVariant *result,
		tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis) {

		if (numparams < 13) return TJS_E_BADPARAMCOUNT;

		NI_LayerExBase *dest;
		if ((dest = NI_LayerExBase::getNative(objthis)) == NULL) return TJS_E_NATIVECLASSCRASH;
		dest->reset(objthis);
		
		NI_LayerExBase *src;
		if ((src = NI_LayerExBase::getNative(param[0]->AsObjectNoAddRef())) == NULL) return TJS_E_NATIVECLASSCRASH;
		src->reset(param[0]->AsObjectNoAddRef());
		
		double g_x1 = param[1]->AsReal();
		double g_y1 = param[2]->AsReal();
		double g_x2 = g_x1 + param[3]->AsReal();
		double g_y2 = g_y1 + param[4]->AsReal();

		double quad[8];
        quad[0] = param[5]->AsReal(); // ����
        quad[1] = param[6]->AsReal();
        quad[2] = param[7]->AsReal(); // �E��
        quad[3] = param[8]->AsReal();
		quad[4] = param[11]->AsReal(); // �E��
        quad[5] = param[12]->AsReal(); 
        quad[6] = param[9]->AsReal(); // ����
        quad[7] = param[10]->AsReal();


		{
			/// �\�[�X�̏���
			unsigned char *buffer = src->_buffer;
			// AGG �p�ɐ擪�ʒu�ɕ␳
			if (src->_pitch < 0) {
				buffer += int(src->_height - 1) * src->_pitch;
			}
			agg::rendering_buffer rbuf_src(buffer, src->_width, src->_height, src->_pitch);

			/// �����_�����O�p�o�b�t�@
			buffer = dest->_buffer;
			// AGG �p�ɐ擪�ʒu�ɕ␳
			if (dest->_pitch < 0) {
				buffer += int(dest->_height - 1) * dest->_pitch;
			}
			agg::rendering_buffer rbuf(buffer, dest->_width, dest->_height, dest->_pitch);

			// �����_���̏���
			pixfmt_pre  pixf_pre(rbuf);
			renderer_base_pre rb_pre(pixf_pre);
			
			g_rasterizer.clip_box(0, 0, dest->_width, dest->_height);
			g_rasterizer.reset();
			g_rasterizer.move_to_d(quad[0], quad[1]);
			g_rasterizer.line_to_d(quad[2], quad[3]);
			g_rasterizer.line_to_d(quad[4], quad[5]);
			g_rasterizer.line_to_d(quad[6], quad[7]);
			
			// �ό`�R�s�[
			agg::trans_perspective tr(quad, g_x1, g_y1, g_x2, g_y2);
			if(tr.is_valid()) {
				typedef agg::span_interpolator_linear<agg::trans_perspective> interpolator_type;
				typedef agg::span_subdiv_adaptor<interpolator_type> subdiv_adaptor_type;
				interpolator_type interpolator(tr);
				subdiv_adaptor_type subdiv_adaptor(interpolator);
				
				typedef agg::span_image_filter_rgba_2x2<color_type,
				agg::order_bgra,
				subdiv_adaptor_type> span_gen_type;
				typedef agg::renderer_scanline_aa<renderer_base_pre, span_gen_type> renderer_type;

				typedef agg::span_allocator<agg::rgba8> span_alloc_type;
				span_alloc_type sa;
				agg::image_filter_hermite filter_kernel;
				agg::image_filter_lut filter(filter_kernel, false);
				
				span_gen_type sg(sa, 
								 rbuf_src, 
								 agg::rgba_pre(0, 0, 0, 0),
								 subdiv_adaptor,
								 filter);
				
				renderer_type ri(rb_pre, sg);
				agg::render_scanlines(g_rasterizer, g_scanline, ri);
			}

		}

		dest->redraw(objthis);
		return TJS_S_OK;
	}
};

//---------------------------------------------------------------------------
#pragma argsused
int WINAPI DllEntryPoint(HINSTANCE hinst, unsigned long reason,
	void* lpReserved)
{
	return 1;
}

static void
addMethod(iTJSDispatch2 *dispatch, const tjs_char *methodName, tTJSDispatch *method)
{
	tTJSVariant var = tTJSVariant(method);
	method->Release();
	dispatch->PropSet(
		TJS_MEMBERENSURE, // �����o���Ȃ������ꍇ�ɂ͍쐬����悤�ɂ���t���O
		methodName, // �����o�� ( ���Ȃ炸 TJS_W( ) �ň͂� )
		NULL, // �q���g ( �{���̓����o���̃n�b�V���l�����ANULL �ł��悢 )
		&var, // �o�^����l
		dispatch // �R���e�L�X�g
		);
}

static void
delMethod(iTJSDispatch2 *dispatch, const tjs_char *methodName)
{
	dispatch->DeleteMember(
		0, // �t���O ( 0 �ł悢 )
		methodName, // �����o��
		NULL, // �q���g
		dispatch // �R���e�L�X�g
		);
}

//---------------------------------------------------------------------------
static tjs_int GlobalRefCountAtInit = 0;
extern "C" HRESULT _stdcall _export V2Link(iTVPFunctionExporter *exporter)
{
	// �X�^�u�̏�����(�K���L�q����)
	TVPInitImportStub(exporter);

	TVPAddImportantLog(ttstr(copyright));
	
	// �N���X�I�u�W�F�N�g�`�F�b�N
	if ((NI_LayerExBase::classId = TJSFindNativeClassID(L"LayerExBase")) <= 0) {
		NI_LayerExBase::classId = TJSRegisterNativeClass(L"LayerExBase");
	}
	
	{
		// TJS �̃O���[�o���I�u�W�F�N�g���擾����
		iTJSDispatch2 * global = TVPGetScriptDispatch();

		// Layer �N���X�I�u�W�F�N�g���擾
		tTJSVariant varScripts;
		TVPExecuteExpression(TJS_W("Layer"), &varScripts);
		iTJSDispatch2 *dispatch = varScripts.AsObjectNoAddRef();
		if (dispatch) {
			// �v���p�e�B������
			NI_LayerExBase::init(dispatch);

			// ��p���\�b�h�̒ǉ�
			addMethod(dispatch, L"perspectiveCopy", new tPerspectiveCopy());
		}

		global->Release();
	}
			
	// ���̎��_�ł� TVPPluginGlobalRefCount �̒l��
	GlobalRefCountAtInit = TVPPluginGlobalRefCount;
	// �Ƃ��čT���Ă����BTVPPluginGlobalRefCount �͂��̃v���O�C������
	// �Ǘ�����Ă��� tTJSDispatch �h���I�u�W�F�N�g�̎Q�ƃJ�E���^�̑��v�ŁA
	// ������ɂ͂���Ɠ������A����������Ȃ��Ȃ��ĂȂ��ƂȂ�Ȃ��B
	// �����Ȃ��ĂȂ���΁A�ǂ����ʂ̂Ƃ���Ŋ֐��Ȃǂ��Q�Ƃ���Ă��āA
	// �v���O�C���͉���ł��Ȃ��ƌ������ƂɂȂ�B

	return S_OK;
}
//---------------------------------------------------------------------------
extern "C" HRESULT _stdcall _export V2Unlink()
{
	// �g���g��������A�v���O�C����������悤�Ƃ���Ƃ��ɌĂ΂��֐��B

	// �������炩�̏����Ńv���O�C��������ł��Ȃ��ꍇ��
	// ���̎��_�� E_FAIL ��Ԃ��悤�ɂ���B
	// �����ł́ATVPPluginGlobalRefCount �� GlobalRefCountAtInit ����
	// �傫���Ȃ��Ă���Ύ��s�Ƃ������Ƃɂ���B
	if(TVPPluginGlobalRefCount > GlobalRefCountAtInit) return E_FAIL;
		// E_FAIL ���A��ƁAPlugins.unlink ���\�b�h�͋U��Ԃ�

	// �v���p�e�B�J��
	NI_LayerExBase::unInit();
	
	// - �܂��ATJS �̃O���[�o���I�u�W�F�N�g���擾����
	iTJSDispatch2 * global = TVPGetScriptDispatch();

	// - global �� DeleteMember ���\�b�h��p���A�I�u�W�F�N�g���폜����
	if (global) {

		// Layer �N���X�I�u�W�F�N�g���擾
		tTJSVariant varScripts;
		TVPExecuteExpression(TJS_W("Layer"), &varScripts);
		iTJSDispatch2 *dispatch = varScripts.AsObjectNoAddRef();
		if (dispatch) {
			delMethod(dispatch, L"perspectiveCopy");
		}

		// TJS ���̂����ɉ������Ă����Ƃ��Ȃǂ�
		// global �� NULL �ɂȂ蓾��̂� global �� NULL �łȂ�
		// ���Ƃ��`�F�b�N����
		global->Release();
	}

	// �X�^�u�̎g�p�I��(�K���L�q����)
	TVPUninitImportStub();

	return S_OK;
}
