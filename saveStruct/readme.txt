Title: saveStruct Plugin
Author: �킽�Ȃׂ���, miahmie

������͂ȂɁH

Array/Dictionary �� saveStruct �̏������A
Unicode �ł͂Ȃ��A���݂̃R�[�h�y�[�W�܂���UTF-8 �ŏo�͉\�ɂ�����̂ł�

toStructString�Ō��ʂ𕶎���Ŏ擾���邱�Ƃ��ł��܂�

���g�p���@

manual.tjs �Q��

�V���ɐ��`�p�I�v�V�����l���ǉ�����܂����B

�EssoIndent : �����E�z��̊K�w���C���f���g�ł킩��₷���o�͂��܂�
�EssoConst  : �����E�z���(const)�����ďo�͂��܂�
�EssoSort   : �����̃L�[���\�[�g���ďo�͂��܂�
�EssoHidden : �B�������o���o�͂��܂�

�ߋ��o�[�W�����̌݊��̂��߁iScripts.toStructString�������j
ssoConst/ssoIndent�̓f�t�H���g�ł͎w�肳��܂���̂Œ��ӂ��Ă��������B

ssoHidden�͉B�������o���擾����悤�ɂȂ�܂����A�B�������o�̋@�\���́A
simplebinder���g�����ꕔ�̃v���O�C�����ł����g�p����Ă��Ȃ��悤�ł��B
�i�p�r�Ƃ��Ă͖w�ǎg�������Ȃ����Ǝv���܂��j


�����̑�

{Array/Dictionary}.toStructString��newline�I�v�V������
�f�t�H���g�l���ύX����Ă��܂��B(\r\n �� \n)
�����split�Ȃǂōs�����������s���Ă���ꍇ��
����݊��ɉe�����ł܂��̂Œ��ӂ��Ă��������B


Array/Dictionary �� saveStruct�ƈႢ�A
�����^�z��ȊO�̃I�u�W�F�N�g�̓��e���ۑ��ΏۂɂȂ�܂��B
�iEnumMembers�ɑΉ����ĂȂ��I�u�W�F�N�g�͕ۑ��ł��܂���j
�������A���ʂ̓��e�͏�Ɏ����Ƃ��ċL������邽�߁A
�֐���v���p�e�B����%[]�Ƃ��ĕۑ�����邱�ƂɂȂ�܂��B

�Ⴆ��

(Dictionary.saveStruct2 incontextof global)("global.tjs",,, ssoIndent|ssoSort);

�Ƃ���ƁAglobal����̃I�u�W�F�N�g�c���[�����邱�Ƃ��ł��܂��B
�iArray�N���X�����͗�O�I��[]�ŕۑ�����Ă��܂��܂����d�l�ł��j


Scripts.toStructString�ŃI�u�W�F�N�g�ȊO�̃v���~�e�B�u�Ȓl�������񉻂ł��܂��B
�i�������n����""�ň͂܂�ăG�X�P�[�v���ꂽ���̂��Ԃ�܂��j
������̃��\�b�h��option�̃f�t�H���g�l��
Array/Dictioanary�̂���ƈႤ�̂Œ��ӂ��Ă��������B

�g�p��FDebug.message("any_variables="+Scripts.toStructString(any_variables));


�����C�Z���X

���C�Z���X�͋g���g���{�̂ɏ������Ă��������B
