Title: getSample�v���O�C��
Author: �킽�Ȃׂ����Cmiahmie

������͂ȂɁH

WaveSoundBuffer �Ɍ��p�N�A�j���p�̂��߂̃T���v���擾�̋@�\��ǉ����܂��B


���g����

manual.tjs �Q��


���g�p��i�V�����j

	// soundBuffer : ����炵�Ă���T�E���h�o�b�t�@
	var voiceLevel; // ���p�N���x��
	var a = soundBuffer.sampleValue;
	//dm("�{�C�X�l: "+soundBuffer.position+" : %0.3f".sprintf(a));
	if      (a > 0.3 ) voiceLevel = 2;
	else if (a > 0.03) voiceLevel = 1;
	else               voiceLevel = 0;
	// voiceLevel�ɂ��킹�Č��̃A�j����ݒ�


�����C�Z���X

���̃v���O�C���̃��C�Z���X�͋g���g���{�̂ɏ������Ă��������B
