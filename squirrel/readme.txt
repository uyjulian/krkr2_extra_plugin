Title: Squirrel Plugin
Author: �킽�Ȃׂ���

������͂ȂɁH

Squirrel (http://squirrel-lang.org/) �̋g���g���o�C���h�ł��B

Squirrel �g�ݍ��ݗp�I�u�W�F�N�g�w������ł��B
���@�I�ɂ� C ���ꕗ�ŁATJS2 �ƍ\�����T�O���悭���Ă��܂��B

Squirrel �́A�����X���b�h�i�R���[�`���j���T�|�[�g���Ă���A
�X�N���v�g�̎��s������C�ӂ̃^�C�~���O�Œ��f�ł��邽�߁A
�Q�[���p�̃��W�b�N��g�ނ̂ɔ��ɓK���Ă��܂��B

���V�X�e���T�v

�����O���

�ESquirrel �̃O���[�o����Ԃ͋g���g���S�̂ɑ΂��ĂP�������݂��܂��B
�@
�@Squirrel �p�̃X�N���v�g�̎��s�͂��̃O���[�o����ԏ�ł����Ȃ��A
�@��`���ꂽ�t�@���N�V������N���X�����̃O���[�o����Ԃɓo�^����Ă����܂��B

�ETJS2 �̃O���[�o����Ԃ� Squirrel ������ "::krkr" �ŎQ�Ƃł��܂��B

�ESquirrel �̃O���[�o����Ԃ� TJS2 ������ "sqglobal" �ŎQ�Ƃł��܂��B

�EI/O ��Ԃ� OS���ڂł͂Ȃ��ATJS �̃X�g���[�W��Ԃ��Q�Ƃ���܂��B

�@�t�@�C������ TJS �̃X�g���[�W���ɂȂ�܂��B
�@stdin/stdout/stderr �͗��p�ł��܂���

��TJS2/Squirrel�l�ϊ����[��

�E�����A�����A������Ȃǂ̃v���~�e�B�u�l�͒l�n���ɂȂ�܂��B

�ETJS2 �� void �� squirrel �� null �ƑΉ����܂�

�ETJS�� null �� squirrel �ł͒l0�� userpointer �ƑΉ����܂�

  ���̒l�� squirrel �ł̓O���[�o���ϐ� tjsNull �ŎQ�Ɖ\�ł��B

�ETJS2�I�u�W�F�N�g(iTJSDispatch2*) �́ASquirrel �ł� userData �Ƃ��ĎQ�Ɖ\�ł�

�@���^���\�b�h exist/get/set/call ��ʂ��đ���\�ł��B
�@�N���X�I�u�W�F�N�g�� call �����ꍇ�́ATJS2 ����
�@�C���X�^���X���쐬���ꂻ��� UserData �ŎQ�Ƃ������̂��A��܂�
  Dictionary/Array�̑��݂��Ȃ������o�Q�Ƃ̓G���[�ɂȂ�܂�

�Esquirrel �I�u�W�F�N�g�́ATJS2 ���ł� iTJSDispatch2 �Ƃ��ĎQ�Ɖ\�ł�

  PropGet/PropGetByNum/PropSet/PropSetByNum/FuncCall/CreateNew 
  ��ʂ��đ���\�ł��Bincontextof �w��͖�������܂��B
  table / array �̑��݂��Ȃ������o���Q�Ƃ����ꍇ�� void ���A��܂�
  table / array �ɑ΂��鏑�����݂� create �����ɂȂ�܂�

�EcreateTJSClass()�ŁATJS�̃N���X�� Squirrel�N���X�Ƃ��Ĉ������Ƃ��ł��܂�

  - ���̃N���X������ꂽ squirrel �C���X�^���X�� TJS2���ɓn�鎞��
�@�@����� TJS �C���X�^���X�̎Q�Ƃ��n����܂�

  - TJS2 ���o�R���� Squirrel ���ɒl���߂�Ƃ��́A
�@�@���̂܂܌��� Squirrel�C���X�^���X���A��܂��B

  - tjsOverride() �Ő������ꂽTJS�C���X�^���X���ɒ��ڃ��\�b�h��o�^�ł��܂�

  - TJS�C���X�^���X���� callSQ() �Ƃ��đΉ����� squirrel �C���X�^���X��
�@  ���\�b�h�𖾎��I�ɌĂяo�����߂��g������܂��B

  - TJS�C���X�^���X���ł� missing �@�\���ݒ肳��A���݂��Ȃ������o��
�@  �Q�Ƃ��ꂽ�ꍇ�� squirrel �C���X�^���X�̓��������o���Q�Ƃ���܂��B
    TJS�C���X�^���X��������̃C�x���g�Ăяo���ɂ����ꂪ�K�p����邽�߁A
�@  TJS�C���X�^���X���ɒ�`���Ȃ���Ύ����I�� squirrel �C���X�^���X��
�@  ���ꂪ�Ăяo����܂�
  
  - ���̌`�ō��ꂽTJS�C���X�^���X�� squirrel �C���X�^���X���j�������
    �Ƃ��� invalidate ����܂�

�EScripts.registerSQ() �� TJS2 �̒l�� squirrel ���ɓo�^�ł��܂��B

���W�����C�u����

�ESquirrel �W�����C�u�����̂����ȉ��̂��̂����p�\�ł�

  - I/O
  - blob
  - math
  - string

���g�p���@

��Scripts �g��

Squirrel �X�N���v�g�̎��s�@�\��A�I�u�W�F�N�g�� Squirrel �̏�����
�����񉻂�����A�t�@�C���ɕۑ������肷�郁�\�b�h�� Scripts �N���X
�Ɋg������܂��B�ڍׂ� manual.tjs ���Q�Ƃ��Ă�������

��SQFunction �g��

Squirrel ��global�t�@���N�V�����𒼐ڌĂяo����悤�ɕێ�����N���X�ł��B
TJS2 ���b�s���O�ɂ��]���ȕ��ׂȂ��ɌĂяo���������s�����Ƃ��ł��܂��B
�ڍׂ� manual.tjs ���Q�Ƃ��Ă��������B

��SQContinous �g��

Squirrel ��global�t�@���N�V�����𒼐ڌĂяo�� Continuous Handler ��
�ێ�����N���X�ł��B
TJS2 ���b�s���O�ɂ��]���ȕ��ׂȂ��ɌĂяo���������s�����Ƃ��ł��܂��B
�ڍׂ� manual.tjs ���Q�Ƃ��Ă��������B

���g���g���N���X�� squirrel�N���X��

�g���g���̃N���X�� squirrel �̃N���X�Ƃ��Čp���\�ȏ�Ԃ�
�������Ƃ��ł��܂��B�ڍׂ� manual.nut ���Q�Ƃ��Ă��������B

���X���b�h�g����

squirrel�ɂ�镡���̃X���b�h�̕�����s��������������Ă��܂��B
���̂��߂ɗ��p�ł�������N���X Object / Thread ����`����Ă��܂��B
�ڍׂ� squirrel/sqobject/manual.nut ���Q�Ƃ��Ă��������B

���̃X���b�h�������ғ�������ꍇ�Acontinuous handler �Ȃǂ���
����I�� Scripts.driveSQ() ���Ăяo���K�v������܂��B
�܂��A�����̋������f�͂Ȃ����߁A�ʂ̃X���b�h�Œ���I�� wait() ��
�s��Ȃ�������t���[�Y��ԂƂȂ�̂Œ��ӂ���K�v������܂��B
��TJS���l�A�e�Ղ� busy loop �������N�����܂�

���N�����X�N���v�g���s

���S�� squirrel �ŋg���g���𐧌䂳����ꍇ�́A
�ȉ��̂悤�� startup.tjs ���������܂��B
startup.nut ���X���b�h�N������A����ȍ~ squirrel 
�X���b�h�������Ȃ�܂œ�����p�����܂��B

-----------------------------------------------------------------------------
Plugins.link("squirrel.dll");
System.exitOnNoWindowStartup = false; // �N�����E�C���h�E�����I���̗}��
System.exitOnWindowClose = false; // ���C���E�C���h�E�����鎞�̏I���̗}��

// ���C�����[�v�o�^
var prevTick = System.getTickCount();
function main(tick)
{
	if (Scripts.driveSQ(tick - prevTick) == 0) {
		// squirrel �X���b�h���S�ďI��������~�߂�
		System.terminate();
	}
	prevTick = tick;
}
System.addContinuousHandler(main);

// ����������
var argc = 0;
var args = [];
var arg;
while ((arg = System.getArgument("-arg" + argc)) !== void) {
	args.add(arg);
	argc++;
}
// squirrel �̃X�N���v�g���N��
Scripts.forkStorageSQ("startup.nut", args*);
-----------------------------------------------------------------------------

��System.exitOnNoWindowStartup �� ���r�W����4577�ȍ~�̋g���g���ł̂ݎg���܂�

�����C�Z���X

Squirrel �� ������ zlib/libpng�X�^�C�����C�Z���X�ł��B

Copyright (c) 2003-2009 Alberto Demichelis

This software is provided 'as-is', without any 
express or implied warranty. In no event will the 
authors be held liable for any damages arising from 
the use of this software.

Permission is granted to anyone to use this software 
for any purpose, including commercial applications, 
and to alter it and redistribute it freely, subject 
to the following restrictions:

		1. The origin of this software must not be 
		misrepresented; you must not claim that 
		you wrote the original software. If you 
		use this software in a product, an 
		acknowledgment in the product 
		documentation would be appreciated but is 
		not required.

		2. Altered source versions must be plainly 
		marked as such, and must not be 
		misrepresented as being the original 
		software.

		3. This notice may not be removed or 
		altered from any source distribution.
-----------------------------------------------------
END OF COPYRIGHT

���̃v���O�C�����̂̃��C�Z���X�͋g���g���{�̂ɏ������Ă��������B
