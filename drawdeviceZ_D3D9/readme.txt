������͂ȂɁH

Direct3D9 �x�[�X�œ��삷��g���g�� drawdevice �v���O�C���ł��B
�g���g��Z�R���̃R�[�h���g�p���Ă��܂��B


���g����

1. Window �� drawDevice �ɑ΂��Ďw��\�ł�

-------------------------------------------
Plugins.link("drawdeviceZ.dll");
class MyWindow extends Window {
  function MyWindow() {
    super.Window();
    // drawdevice �������ւ�
    drawDevice = new DrawDeviceZ();

    //...
  }
};
-------------------------------------------

�@�\�ɂ��Ă� manual.tjs ���Q�Ƃ��Ă�������


���R���p�C��

premake4�ɂăv���W�F�N�g���쐬���Ă��������B(vs20xx�t�H���_�쐬�ς݁j
�R���p�C���ɂ�
../tp_stub.*
../drawdevice/BasicDrawDevice.*
../00_simplebinder/*
�̃t�H���_�E�t�@�C�����K�v�ł��B


�����m�̖��

�g���g���R���̗�O�̃v���O�C�����ł̃L���b�`�i�đ��Ȃ��j��
��O�������Ƀ��������[�N���N����\��������܂��B
�i../exceptiontest/Main.cpp �\�[�X�R�����g�Q�Ɓj

try/catch�̗�O�z�����g�p���Ă��鉺�L�������Č����̂��ƁF
	tTVPBasicDrawDevice::EnsureDevice()
	tTVPBasicDrawDevice::SetDestRectangle()
	TVPEnsureDirect3DObject()

	��TVPDoTryBlock�ɂ������ɕύX�iifdef __TP_STUB_H__�ɂ��ꍇ�킯�j


�����C�Z���X

���̃v���O�C���̃��C�Z���X�͋g���g���{�̂���ыg���g��Z�ɏ������Ă��������B

DrawDeviceZ/* �̃t�@�C���͋g���g��Z�̃\�[�X�R�[�h���ꕔ���ς��Ďg�p���Ă��܂��B
Main.cpp ���̈ꕔ�̃R�[�h�iDirect3D9�̏����������Ȃǁj��
�g���g��Z��WindowImpl.cpp���痬�p�E���ς��Ă��܂��B
errmsg.cpp �̃e�L�X�g�͋g���g��Z��string_table_en.rc���痬�p����
���b�Z�[�W�e�L�X�g���g�p���Ă��܂��B


----------------------------------------------------------------------------
�g���g��Z���C�Z���X:

Copyright (c), W.Dee and contributors All rights reserved.
Contributors
 Go Watanabe, Kenjo, Kiyobee, Kouhei Yanagita, mey, MIK, Takenori Imoto, yun
Kirikiri Z Project Contributors
W.Dee, casper, �L�����MCF, Biscrat, �L, nagai, ���[, ���� ��V, �i��,
�����T��, ��傤���i���͖������̐��j, AZ-UME, �� �H�l, 
Katsumasa Tsuneyoshi, ���r��, miahmie, �T�[�N����, �A�U�i�V, �͂�����, 
�I�����쏊, ����ӂ�/waffle, �����\�t�g, TYPE-MOON, �L����ЃG���c�[
----------------------------------------------------------------------------
�\�[�X�R�[�h�`�����o�C�i���`�����A�ύX���邩���Ȃ������킸�A�ȉ��̏�����
�����ꍇ�Ɍ���A�ĔЕz����юg�p��������܂��B

�E�\�[�X�R�[�h���ĔЕz����ꍇ�A��L�̒��쌠�\���A�{�����ꗗ�A����щ��L�Ɛ�
  �������܂߂邱�ƁB
�E�o�C�i���`���ōĔЕz����ꍇ�A�Еz���ɕt���̃h�L�������g���̎����ɁA��L��
  ���쌠�\���A�{�����ꗗ�A����щ��L�Ɛӏ������܂߂邱�ƁB
�E���ʂɂ����ʂ̋��Ȃ��ɁA�{�\�t�g�E�F�A����h���������i�̐�`�܂��͔̔�
  ���i�ɁA�g�D�̖��O�܂��̓R���g���r���[�^�[�̖��O���g�p���Ă͂Ȃ�Ȃ��B

�{�\�t�g�E�F�A�́A���쌠�҂���уR���g���r���[�^�[�ɂ���āu����̂܂܁v��
����Ă���A�����َ����킸�A���ƓI�Ȏg�p�\���A����ѓ���̖ړI�ɑ΂���K
�����Ɋւ���Öق̕ۏ؂��܂߁A�܂�����Ɍ��肳��Ȃ��A�����Ȃ�ۏ؂�����܂�
��B���쌠�҂��R���g���r���[�^�[���A���R�̂�������킸�A���Q�����̌�������
����킸�A���ӔC�̍������_��ł��邩���i�ӔC�ł��邩�i�ߎ����̑��́j�s�@
�s�ׂł��邩���킸�A���ɂ��̂悤�ȑ��Q����������\����m�炳��Ă����Ƃ�
�Ă��A�{�\�t�g�E�F�A�̎g�p�ɂ���Ĕ��������i��֕i�܂��͑�p�T�[�r�X�̒��B�A
�g�p�̑r���A�f�[�^�̑r���A���v�̑r���A�Ɩ��̒��f���܂߁A�܂�����Ɍ��肳���
���j���ڑ��Q�A�Ԑڑ��Q�A�����I�ȑ��Q�A���ʑ��Q�A�����I���Q�A�܂��͌��ʑ��Q��
���āA��ؐӔC�𕉂�Ȃ����̂Ƃ��܂��B
