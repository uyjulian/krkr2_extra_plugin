/*
 * copyright (c)2009 http://wamsoft.jp
 * zlib license
 */
#ifndef __SQCONT_H__
#define __SQCONT_H__

/**
 * continuous handler �p����
 * function(currentTick, diffTick) �̌`�Œ���I�ɌĂяo����郁�\�b�h�Q��o�^����@�\
 */
namespace sqobject {

/// �@�\�o�^
void registerContinuous();
/// �n���h�������Ăяo���BThread::main �̑O�ŌĂяo���K�v������
void beforeContinuous();
/// �n���h�������Ăяo���BThread::main �̌�ŌĂяo���K�v������
void afterContinuous();
/// �@�\�I��
void doneContinuous();

};

#endif
