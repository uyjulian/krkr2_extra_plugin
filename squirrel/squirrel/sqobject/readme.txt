Author: �n粍�(go@wamsoft.jp)
Date: 2009/4/22

���T�v

squirrel �ŋ^���X���b�h�������������郉�C�u�����ł��B

���g����

�ڍׂ� manual.nut ���Q�Ƃ��Ă�������

���g�ݍ���

������g�ݍ��ނ��߂ɂ́A�����̃V�X�e���ɂ��킹��
���b�p�[�������ꕔ�Ǝ���������K�v������܂��B

���������Ǘ������̎���

�{�@�\�̃N���X�͕W���ł͒ʏ��C�̃q�[�v�@�\�������Ă��܂��B
�v���v���Z�b�T�� SQOBJHEAP ���`���邱�ƂŁA�S�� squirrel 
�̕W���@�\ (sq_malloc/sq_free) �������ăq�[�v���m��
����悤�ɂȂ�܂��B

�Ȃ��A���̒�`���̂������ւ������ꍇ�� sqobjectinfo.h �� 
SQHEAPDEFINE �̒�`�������ւ��Ă�������

���񓯊��t�@�C�����[�h�̎���

�t�@�C����񓯊��ɓǂݍ��ޏ������s�����߂Ɉȉ��̃��\�b�h���������Ă��������B

�� squirrel �W���@�\�� sqstd_loadfile ���͑S���g���Ă܂���B
�@ �T���v���� sqfunc / sqratfunc ���̏����������ł� 
   sqstdmath �� sqstdstring �����o�^���Ă��܂��B

   ���[������������̂ŁAdofile() �̗��p�ɂ͂����ӂ��������B

-----------------------------------------------------------------------------
/**
 * �t�@�C����񓯊��ɊJ��
 * @param filename �X�N���v�g�t�@�C����
 * @param binary �o�C�i���w��ŊJ��
 * @return �t�@�C���n���h��
 */
extern void *sqobjOpenFile(const SQChar *filename, bool binary);

/**
 * �t�@�C�����J���ꂽ���ǂ����̃`�F�b�N
 * @param handler �t�@�C���n���h��
 * @param dataPtr �f�[�^�i�[��A�h���X(�o��) (�G���[����NULL)
 * @param dataSize �f�[�^�T�C�Y(�o��)
 * @return ���[�h�������Ă����� true
 */
extern bool sqobjCheckFile(void *handler, const char **dataAddr, int *dataSize);

/**
 * �t�@�C�������
 * @param handler �t�@�C���n���h��
 */
extern void sqobjCloseFile(void *handler);
----------------------------------------------------------------------------

���O���[�o��VM�֌W���\�b�h�̎���

SQObject �� SQThread �́A����̃O���[�o�� Squirrel VM �Ɉˑ����܂��B
������擾���邽�߂̈ȉ��̃��\�b�h���������܂�

----------------------------------------------------------
namespace sqobject{
  extern HSQUIRRELVM�@init();        /// < VM������
  extern void done();                /// < VM�j��
  extern HSQUIRRELVM�@getGlobalVM(); /// < �O���[�o��VM�擾
}
----------------------------------------------------------

���I�u�W�F�N�g�Q�Ə����̎���

�o�C���_�ɉ������l�C�e�B�u�I�u�W�F�N�g�̎Q��(push/get)�������K�v�ɂȂ�܂��B
�W���ł� sqrat ���g���R�[�h�ɂȂ��Ă��܂��B�ʓr�Ǝ������p�̃R�[�h��
�g���ꍇ�͕K�v�ɉ����ăv���v���Z�b�T�ňȉ����`���Ă�������

NOUSESQRAT   sqrat ���o�C���_�Ƃ��Ďg�p���Ȃ�

������`�����ꍇ�͓Ǝ��̊ȈՃo�C���_ (sqfunc.h) �ɂ�鏈���ɂȂ�܂�

���o�^�p�� ObjectInfo �̏��@�\���g�����߈ȉ��̃��\�b�h���K�v�ɂȂ�܂��B

// Object �p���I�u�W�F�N�g�� push
template<typename T>
void pushValue(HSQUIRRELVM v, T *value);

// ���̑��̃I�u�W�F�N�g�p�̔ėp push
template<typename T>
void pushOtherValue(HSQUIRRELVM v, T *value) {

// �I�u�W�F�N�g�̒l�擾
template<typename T>
SQRESULT getValue(HSQUIRRELVM v, T **value, int idx=-1) {

���I�u�W�F�N�g�o�^�����̎���

�I�u�W�F�N�g���N���X�Ƃ��ēo�^���邽�߂̈ȉ��̃��\�b�h���������܂��B
�v���O�����͂���ƁAThread::registGlobal() ���Ăяo�����Ƃ�
���\�b�h��o�^�ł��܂�

-----------------------------------------------
namespace sqobject{
  void Object::registerClass();
  void Thread::registerClass();
}
-----------------------------------------------

���Ǝ��I�u�W�F�N�g�̎����Ɠo�^

sqobject::Object ��P��p������`�ŃI�u�W�F�N�g���쐬���Ă��������B

Object �̓Ǝ��@�\���g���ꍇ�́A�R���X�g���N�^ 
Object(HSQUIRRELVM v, int delegateIdx=2) ���Ăяo�����A
���Ăяo���悤�ɂ��邩�A���邢�́A�o�^��� initSelf(HSQUIRRELVM v, int idx=1)
���g���āA���ȃI�u�W�F�N�g�Q�Ƃ��L�^����K�v������܂��B

���̏������K�v�ȋ@�\
�E�f�X�g���N�^�@�\
�E�f���Q�[�g�@�\
�E�v���p�e�B�@�\
�Ewait�@�\ (wait/notify)
�EC++����̃C�x���g�R�[���o�b�N

Object �N���X�Ɋg������Ă���A�v���p�e�B��f���Q�[�g��
�@�\���g�����Ƃ��ł���ق��A�^���X���b�h�� wait �ΏۂƂ��ăI�u�W�F�N�g
���������Ƃ��ł��܂��B

���I�u�W�F�N�g�̌p���������͓̂Ǝ��Ɏ�������K�v������܂�

�����s�����̎���

�����̏����n�ɑg�ݓ����ꍇ�̊�{�I�ȏ����菇��������܂�

��������

1. sqobject::init() ���Ăяo��
2. print�֐��o�^���K�v�ȏ������s��
3. �N���X�o�^

  �g�ݍ��݋@�\�͈ȉ��̏����ŃO���[�o���ɓǂݍ��܂�܂�

  Object::registerClass();
  Thread::registerClass();
  Thread::registerGlobal();

  ���K�v�ɉ����ăN���X��o�^

�����s��������

�^���X���b�h���ғ�������ɂ̓A�v���̃��C�����[�v��
����ȉ��̏������Ăяo���Ă��������B

-----------------------------------------------
/*
 * ���ԍX�V
 * @param diff �o�ߎ���
 */
int Thread::update(long diff);

/**
 * ���s�������C�����[�v
 * @return ���쒆�̃X���b�h�̐�
 */	
int Thread::main();
-----------------------------------------------

��{�\���͎��̂悤�ɂ���̂��Ó��ł��B

while(true) {
  �C�x���g����
�@Thread::update(���ԍ���)
  beforeContinuous(); // ���Ocontinuous����:��q
�@Thread::main()
  afterContinuous(); // ����continuous����:��q
�@��ʍX�V����
};

���Ԃ̊T�O�̓V�X�e�����ŔC�ӂɑI���ł��܂��B
��ʓI�ɂ̓t���[�������Ams �w����g���܂��B����Ŏw�肵���l��
wait() ���߂ɓn�����l�p�����[�^�̈Ӗ��ɂȂ�܂��B

���I�������̎���

1. Thread::done() �ŃX���b�h�̏��������j��
2. roottable �̏���S�N���A
3. sqobject::done() �Ăяo���� VM �����

��continuous handler �@�\

�X���b�h�@�\�Ƃ͕ʂɁA�P���ɃG���W������������ squirrel 
�X�N���v�g�����I�ɌĂяo���@�\�ł��B

�X���b�h����      : ���[�U�ɂ�鐧�䏈��
continuous handler: ���䂪�I��������Ƃ̎����v�Z����

�Ƃ������g��������z�肵�Ă��܂��Bcontinuous handler��
�Ăяo���́A���ׂẴX���b�h��������U suspend ������ɂȂ�܂��B

��continuous handler �̌Ăяo���́A��ɒʏ�� sq_call ��
  �����̂Ȃ̂ŃX�N���v�g�� suspend() ���ĕ��A���邱�Ƃ͂ł��܂���B

�Esqobject::registerContinuous() ���Ăяo�����Ƃŋ@�\�o�^����܂�

�Esqobject::beforeContinuous() �� sqobject::afterContinuous() ��
  ���ꂼ�� sqobject::Thread::main() �̑O��ŌĂяo���Ă��������B

�E�����I�����ɂ́@sqoject::doneContinuous() ���Ăяo���܂�

�E�X�N���v�g����� addContinuous() / removeContinuous() ��
  �֐���o�^/�����ł��܂��B

�������T���v���R�[�h

 sqfunc.cpp     �V���v���Ȍp��/�����o�֐������̎�����
 sqratfunc.cpp	SQRat���g���ꍇ�̎�����

���X�N���v�g�̌Ăяo��

C++������X�N���v�g���N������ꍇ�́AThread::fork() ���g�����A
squirrel �� API ���g���āA�O���[�o���֐� fork() ���Ăяo���悤�ɂ��Ă��������B

sqrat�ł̗�
---------------------------------------------------------------
Sqrat::Function forkFunc(Sqrat::RootTable(), _SC("fork"));
forkFunc.Evaluate<int>(NULL, _SC("file.nut"));
---------------------------------------------------------------

���X���b�h���s���̒���

�ʂ̃X�N���v�g�́A�҂���ԂɂȂ�܂Ŏ��s�𒆒f���Ȃ����߁A
�e�Ղ� busy loop �ɂȂ�܂��B����I�� wait() �܂��� suspend() 
������ăV�X�e���ɏ�����߂��悤�ɂ���K�v������܂��B
�����Ƃ��ẮA��ʍX�V���K�v�ȃ^�C�~���O�� suspend() 
���s���Ηǂ����ƂɂȂ�܂��B

�����C�Z���X

squirrel ���l zlib���C�Z���X�ɏ]���ė��p���Ă��������B

/*
 * copyright (c)2009 Go Watanabe go@wamsoft.jp
 * zlib license
 */
