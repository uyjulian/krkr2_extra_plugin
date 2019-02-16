/*
 * copyright (c)2009 http://wamsoft.jp
 * zlib license
 */
#ifndef __SQTHREAD_H__
#define __SQTHREAD_H__

#ifndef SQTHREAD
#define SQTHREAD Thread
#define SQTHREADNAME _SC("Thread")
#endif

#include <stdio.h>
#include "sqobjectclass.h"

/**
 * �t�@�C����񓯊��ɊJ��
 * @param filename �X�N���v�g�t�@�C����
 * @return �t�@�C���n���h��
 */
extern void *sqobjOpenFile(const SQChar *filename, bool binary=false);

/**
 * �t�@�C�����J���ꂽ���ǂ����̃`�F�b�N
 * @param handler �t�@�C���n���h��
 * @param dataPtr �f�[�^�i�[��A�h���X(�o��)
 * @param dataSize �f�[�^�T�C�Y(�o��)
 * @return ���[�h�������Ă����� true
 */
extern bool sqobjCheckFile(void *handler, const char **dataAddr, int *dataSize);

/**
 * �t�@�C�������
 * @param handler �t�@�C���n���h��
 */
extern void sqobjCloseFile(void *handler);

namespace sqobject {

class Thread : public Object {

protected:
	long _currentTick; ///< ���̃X���b�h�̎��s����

	ObjectInfo _scriptName; ///< �X�N���v�g��
	
	void *_fileHandler; ///< ���̃X���b�h���J�����Ƃ��Ă���t�@�C��

	// �X���b�h�f�[�^
	ObjectInfo _thread;

	// ���s�X�N���v�g
	ObjectInfo _func;

	// �������X�g
	ObjectInfo _args;

	// system�p�҂�
	ObjectInfo _waitSystem;
	// �҂��Ώ�
	ObjectInfo _waitList;
	// �҂�����
	SQInteger _waitTimeout;

	// �҂��̌���
	ObjectInfo _waitResult;
	
	/// �I���R�[�h
	ObjectInfo _exitCode;
	
	/**
	 * �X���b�h���
	 */
	enum ThreadStatus {
		THREAD_NONE,           // ��������
		THREAD_LOADING_FILE,   // �t�@�C�����[�h��
		THREAD_LOADING_FUNC,   // �֐����[�h��
		THREAD_STOP,   // ��~
		THREAD_RUN,    // ���쒆
		THREAD_WAIT,   // �҂���
	} _status;

	/**
	 * @return �����҂�����
	 */
	bool isWait();

	/**
	 * @return �Y���X���b�h�ƌ��݊Ǘ����̃X���b�h����v���Ă�� true
	 */
	bool isSameThread(HSQUIRRELVM v);

public:
	// �R���X�g���N�^
	Thread();

	// �R���X�g���N�^
	Thread(HSQUIRRELVM v);
	
	// �f�X�g���N�^
	~Thread();

protected:
	/**
	 * �X���b�h��񏉊���
	 */
	void _init();

	/**
	 * �I�u�W�F�N�g�ɑ΂���҂������N���A����
	 * @param status �L�����Z���̏ꍇ�� true
	 */
	void _clearWait();

	/**
	 * ���j��
	 */
	void _clear();

	/**
	 * exit��������
	 */
	void _exit();
	
	// ------------------------------------------------------------------
	//
	// Object ����̐���p
	//
	// ------------------------------------------------------------------
	
public:

	/**
	 * �g���K�ɑ΂���҂���������������
	 * @param name �g���K��
	 * @return �Y���I�u�W�F�N�g��҂��Ă��ꍇ�� true
	 */
	bool notifyTrigger(const SQChar *name);

	/**
	 * �I�u�W�F�N�g�ɑ΂���҂���������������
	 * @param target �҂��Ώ�
	 * @return �Y���I�u�W�F�N�g��҂��Ă��ꍇ�� true
	 */
	bool notifyObject(ObjectInfo &target);
	
	// ------------------------------------------------------------------
	//
	// ���\�b�h
	//
	// ------------------------------------------------------------------

protected:

	/**
	 * �����p:fork �����B�X���b�h���P�������� VM��PUSH����
	 * @param v squirrelVM
	 * @return ���������� true
	 */
	static bool _fork(HSQUIRRELVM v);

	/**
	 * �����p: wait����
	 * @param v squirrelVM
	 * @param idx �Y�� idx �ȍ~�ɂ�����̂�҂�
	 */
	void _wait(HSQUIRRELVM v, int idx=2);

	/**
	 * �����p: system�����̑҂��B�X�^�b�N�擪�ɂ���X���b�h��҂�
	 * @param v squirrelVM
	 */
	void _system(HSQUIRRELVM v);

	
	/**
	 * �����p: exec����
	 * @param v squirrelVM
	 * @param idx ���̃C���f�b�N�X�����ɂ�����̂����s�J�n����B������Ȃ�X�N���v�g�A�t�@���N�V�����Ȃ璼��
	 */
	void _exec(HSQUIRRELVM v, int idx=2);

	/**
	 * ���݂̃I�u�W�F�N�g�����s�X���b�h�Ƃ��ēo�^
	 */
	void _entryThread(HSQUIRRELVM v);
	
public:
	/**
	 * �҂��o�^
	 */
	SQRESULT wait(HSQUIRRELVM v);

	/**
	 * wait�̃L�����Z��
	 */
	void cancelWait();

	/**
	 * ���s�J�n
	 * @param func ���s�Ώۃt�@���N�V�����B������̏ꍇ�Y���X�N���v�g��ǂݍ���
	 */
	SQRESULT exec(HSQUIRRELVM v);

	/**
	 * ���s�I��
	 * @param exitCode �I���R�[�h
	 */
	SQRESULT exit(HSQUIRRELVM v);

	/**
	 * exitCode�擾
	 */
	SQRESULT getExitCode(HSQUIRRELVM v);
	
	/**
	 * ���s��~
	 */
	void stop();

	/**
	 * ���s�ĊJ
	 */
	void run();

	/**
	 * @return ���s�X�e�[�^�X
	 */
	int getStatus();

	/**
	 * @return ���ݎ���
	 */
	int getCurrentTick() {
		return _currentTick;
	}

	// ------------------------------------------------------------------
	//
	// ���s����
	//
	// ------------------------------------------------------------------
	
protected:

	/**
	 * �X���b�h�̃G���[���̕\��
	 */
	void printError();

	/**
	 * �X���b�h�̃��C������
	 * @param diff �o�ߎ���
	 * @return �X���b�h���s�I���Ȃ� true
	 */
	bool _main(long diff);

public:

	/**
	 * ����X���b�h������
	 */
	static void init();

	/*
	 * ���ԍX�V
	 * @param diff �o�ߎ���
	 */
	static void update(long diff);

	/**
	 * �X���b�h�����p�R�[���o�b�N
	 * @param th �X���b�h�I�u�W�F�N�g
	 * @param userData ���[�U�f�[�^
	 */
	typedef void ThreadCallback(ObjectInfo th, void *userData);
	
	/*
	 * ���s�������C�����[�v
	 * ���ݑ��݂���X���b�h�𑍂Ȃ߂łP�x�������s����B
	 * �V�X�e���{�̂̃��C�����[�v(�C�x���g�����{�摜����)
	 * ����1�x�����Ăяo�����Ƃŋ@�\����B���ꂼ��̃X���b�h�́A
	 * �������疾���I�� suspend() �܂��� wait�n�̃��\�b�h���Ăяo���ď�����
	 * ���̃X���b�h�ɈϏ�����K�v������B
	 * @param onThreadDone �X���b�h�I�����ɌĂяo�����R�[���o�b�N
	 * @param userData �R�[���o�b�N�ɓn�����[�U�f�[�^����
	 * @return ���쒆�̃X���b�h�̐�
	 */
	static int main(ThreadCallback *onThreadDone=NULL, void *userData=NULL);

	/**
	 * �X�N���v�g���s�J�n�p
	 * @param scriptName �X�N���v�g��
	 * @param argc �����̐�
	 * @param argv ����
	 * @return �����Ȃ� true
	 */
	static bool fork(const SQChar *scriptName, int argc=0, const SQChar **argv=NULL);

	/**
	 * �S�X���b�h�ւ̃g���K�ʒm
	 * @param name �����҂��g���K��
	 */
	static void trigger(const SQChar *name);
	
	/**
	 * ����X���b�h�̔j��
	 */
	static void done();

	/**
	 * ����X���b�h��
	 */
	static int getThreadCount();

	// -------------------------------------------------------------
	// �X���b�h�����p
	// -------------------------------------------------------------

public:
	static long currentTick;  ///< ����̌Ăяo������
	static long diffTick;     ///< �����Ăяo������
	
protected:
	static ObjectInfo *threadList; ///< �X���b�h�ꗗ
	static ObjectInfo *newThreadList; ///< �X���b�h�ꗗ

	// -------------------------------------------------------------
	// �O���[�o�����\�b�h�p
	// -------------------------------------------------------------

	/**
	 * ���ݎ����̎擾
	 */
	static SQRESULT global_getCurrentTick(HSQUIRRELVM v);

	/**
	 * ���������̎擾
	 */
	static SQRESULT global_getDiffTick(HSQUIRRELVM v);
	
	/*
	 * @return ���݂̃X���b�h��Ԃ�
	 */
	static SQRESULT global_getCurrentThread(HSQUIRRELVM v);
	
	/*
	 * @return ���݂̃X���b�h�ꗗ��Ԃ�
	 */
	static SQRESULT global_getThreadList(HSQUIRRELVM v);

	/*
	 * �X�N���v�g��V�����X���b�h�Ƃ��Ď��s����
	 * �� return Thread(func); ����
	 * @param func �X���b�h�Ŏ��s����t�@���N�V����
	 * @return �V�X���b�h
	 */
	static SQRESULT global_fork(HSQUIRRELVM v);

	/**
	 * @return ���ݎ��s���̃X���b�h���I�u�W�F�N�g(Thread*)
	 */
	static Thread *getCurrentThread(HSQUIRRELVM v);

	/**
	 * �X�N���v�g��؂�ւ���
	 * @param func �X���b�h�Ŏ��s����t�@���N�V����
	 */
	static SQRESULT global_exec(HSQUIRRELVM v);

	/**
	 * ���s���X���b�h�̏I��
	 * @param exitCode �I���R�[�h
	 */
	static SQRESULT global_exit(HSQUIRRELVM v);

	/**
	 * �X�N���v�g�����s���Ă��̏I����҂�
	 * @param func �X���b�h�Ŏ��s����t�@���N�V����
	 * @return �X�N���v�g�̏I���R�[�h
	 */
	static SQRESULT global_system(HSQUIRRELVM v);
	
	/**
	 * ���s���X���b�h�̏����҂�
	 * @param target int:���ԑ҂�(ms), string:�g���K�҂�, obj:�I�u�W�F�N�g�҂�
	 * @param timeout �^�C���A�E�g(�ȗ����͖����ɑ҂�)
	 * @return �҂����L�����Z�����ꂽ�� true
	 */
	static SQRESULT global_wait(HSQUIRRELVM v);

	/**
	 * �S�X���b�h�ւ̃g���K�ʒm
	 * @param name �����҂��g���K��
	 */
	static SQRESULT global_trigger(HSQUIRRELVM v);

	/**
	 * �x�[�XVM��ŃX�N���v�g�����s����B
	 * ���̌Ăяo���̓X���b�h�ɂ����̂ł͂Ȃ����߁A�������� suspend() / wait() ��
	 * �ĂԂƃG���[�ɂȂ�̂Œ��ӂ��Ă��������B�K��1�x�ŌĂт������̂�n���K�v������܂��B
	 * @param func �O���[�o���֐��B���t�@�C���͎w��ł��܂���
	 */
	static SQRESULT global_execOnBase(HSQUIRRELVM v);
	
public:
	/**
	 * �O���[�o�����\�b�h�̓o�^
	 */
	static void registerGlobal();

	/**
	 * �N���X�̓o�^
	 */
	static void registerClass();

};

};

#endif
