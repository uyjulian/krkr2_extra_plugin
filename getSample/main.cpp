#include "ncbind/ncbind.hpp"

//------------------------------------------------------------------------------------------------
// �������i�݊��̂��߂Ɏc����Ă��܂��j

/**
 * �T���v���l�̎擾�i�������j
 * ���݂̍Đ��ʒu����w�萔�̃T���v�����擾���Ă��̕��ϒl��Ԃ��܂��B
 * �l�����̃T���v���l�͖�������܂��B
 * @param n �擾����T���v���̐��B�ȗ������ 100
 */
tjs_error
getSample(tTJSVariant *result,tjs_int numparams, tTJSVariant **param, iTJSDispatch2 *objthis)
{
	tjs_error ret = TJS_S_OK;
	tjs_int n = numparams > 0 ? (tjs_int)*param[0] : 100;
	if (n > 0 && result) {
		short *buf = (short*)malloc(n * sizeof(*buf));
		if (buf) {
			tTJSVariant buffer     = (tjs_int)buf;
			tTJSVariant numsamples = n;
			tTJSVariant channel    = 1;
			tTJSVariant *p[3] = {&buffer, &numsamples, &channel};
			if (TJS_SUCCEEDED(ret = objthis->FuncCall(0, L"getVisBuffer", NULL, NULL, 3, p, objthis))) {
				int c=0;
				int sum = 0;
				for (int i=0;i<n;i++) {
					if (buf[i] >= 0) {
						sum += buf[i]; c++;
					}
				}
				*result = c>0 ? sum / c : 0;
			}
			free(buf);
		}
	}
	return ret;
}

NCB_ATTACH_FUNCTION(getSample, WaveSoundBuffer, getSample);


//------------------------------------------------------------------------------------------------
// �V�����̊g���v���p�e�B�E���\�b�h

class WaveSoundBufferAdd {
protected:
	iTJSDispatch2 *objthis; //< �I�u�W�F�N�g���̎Q��
	int counts, aheads;
	tjs_uint32 hint;
	tTJSVariant vBuffer, vNumSamples, vChannel, vAheads, *params[4];
	short *buf;

	static int defaultCounts, defaultAheads;
public:
	WaveSoundBufferAdd(iTJSDispatch2 *objthis)
		:   objthis(objthis),
			counts(defaultCounts),
			aheads(defaultAheads),
			hint(0)
	{
		buf = new short[counts];
		// useVisBuffer = true; �ɂ���
		tTJSVariant val(1);
		tjs_error r = objthis->PropSet(0, TJS_W("useVisBuffer"), NULL, &val, objthis);
		if (r != TJS_S_OK)
			TVPAddLog(ttstr(TJS_W("useVisBuffer=1 failed: ")) + ttstr(r));

		// getVisBuffer�p�̈��������
		vBuffer     = (tjs_int)buf;
		vChannel    = 1;
		vNumSamples = counts;
		vAheads     = aheads;
		params[0] = &vBuffer;
		params[1] = &vNumSamples;
		params[2] = &vChannel;
		params[3] = &vAheads;
	}
	~WaveSoundBufferAdd() {
		delete[] buf;
	}

	/**
	 * �T���v���l�̎擾�i�V�����j
	 * getVisBuffer(buf, sampleCount, 1, sampleAhead)�ŃT���v�����擾���C
	 * (value/32768)^2�̍ő�l���擾���܂��B(0�`1�̎����ŕԂ�܂�)
	 * �����̃v���p�e�B��ǂݏo���ƈÖق�useVisBuffer=true�ɐݒ肳��܂�
	 */
	double getSampleValue() {
		memset(buf, 0, counts*sizeof(short));
		tTJSVariant result;
		tjs_error r = objthis->FuncCall(0, TJS_W("getVisBuffer"), &hint, &result, 4, params, objthis);

		if (r != TJS_S_OK) TVPAddLog(ttstr(TJS_W("getVisBuffer failed: "))+ttstr(r));

		int cnt = (int)result.AsInteger();
		if (cnt > counts || cnt < 0) cnt = counts;

		// �T���v���̓�撆�̍ő�l��Ԃ�
		double max = 0;
		for (int i=cnt-1;i>=0;i--) {
			double s = ((double)buf[i]) / +32768.0;
			s *= s;
			if (max < s) max = s;
		}
		return max;
	}

	/**
	 * �o�b�t�@�擾�p�p�����[�^�v���p�e�B�isampleValue���Q�Ɓj
	 * �f�t�H���g��setDefaultCounts/setDefaultAheads�Ō��肳��܂�
	 */
	int  getSampleCount() const  { return counts; }
	void setSampleCount(int cnt) {
		counts = cnt;
		delete[] buf;
		buf = new short[counts];
		vBuffer     = (tjs_int)buf;
		vNumSamples = counts;
	}
	int  getSampleAhead() const  { return aheads; }
	void setSampleAhead(int ahd) {
		vAheads     = (aheads = ahd);
	}

	/**
	 * �V�����̃f�t�H���g�̃p�����[�^�ݒ�p�֐�
	 * �ȍ~�Ő��������WaveSoundBuffer�̃C���X�^���X�ɂ���
	 * sampleCount/sampleAhead�̃f�t�H���g�l��ݒ�ł��܂�
	 */
	static void setDefaultCounts(int cnt) { defaultCounts = cnt; }
	static void setDefaultAheads(int ahd) { defaultAheads = ahd; }
};

int WaveSoundBufferAdd::defaultCounts = 100;
int WaveSoundBufferAdd::defaultAheads = 0;

// �C���X�^���X�Q�b�^
NCB_GET_INSTANCE_HOOK(WaveSoundBufferAdd)
{
	NCB_INSTANCE_GETTER(objthis) { // objthis �� iTJSDispatch2* �^�̈����Ƃ���
		ClassT* obj = GetNativeInstance(objthis);	// �l�C�e�B�u�C���X�^���X�|�C���^�擾
		if (!obj) {
			obj = new ClassT(objthis);				// �Ȃ��ꍇ�͐�������
			SetNativeInstance(objthis, obj);		// objthis �� obj ���l�C�e�B�u�C���X�^���X�Ƃ��ēo�^����
		}
		return obj;
	}
};

// �o�^
NCB_ATTACH_CLASS_WITH_HOOK(WaveSoundBufferAdd, WaveSoundBuffer) {
	Property(L"sampleValue", &Class::getSampleValue, (int)0);
	Property(L"sampleCount", &Class::getSampleCount, &Class::setSampleCount);
	Property(L"sampleAhead", &Class::getSampleAhead, &Class::setSampleAhead);
}
NCB_ATTACH_FUNCTION(setDefaultCounts, WaveSoundBuffer, WaveSoundBufferAdd::setDefaultCounts);
NCB_ATTACH_FUNCTION(setDefaultAheads, WaveSoundBuffer, WaveSoundBufferAdd::setDefaultAheads);

