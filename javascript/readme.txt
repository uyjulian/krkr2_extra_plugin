Title: Javascript Plugin
Author: �킽�Ȃׂ���

������͂ȂɁH

V8 JavaScript Engine (http://code.google.com/p/v8/) �̋g���g���o�C���h�ł��B

Google �{�Ƃ̃R�[�h�͌݊����ɖ�肪����̂ŁANode.js (http://nodejs.org/) ��
�\�[�X (https://github.com/joyent/node) �ɂ��� V8 ���g���č\�z���Ă���܂�

���R���p�C�����@

(1) c:\node �� node.js ���擾���āAVisual Studio �Ńr���h

c:\node\build �ȉ��ɐ�������� v8 ���C�u�����݂̂��g���܂��̂ŁA
�S�̂��\�z����K�v�͂���܂���B

 > vcbuild clean

�ŁA�v���W�F�N�g�t�@�C���͂�����̂ŁAnode.sln ���J���� v8 �̕����������Ηǂ��ł�

(2) javascript.sln ���J���ăR���p�C��

c:\node ���Q�Ƃ��ăv���O�C������������܂�

���V�X�e���T�v

����̏ڍׂ� manual.tjs / manual.js ���Q�Ƃ��Ă��������B

�����O���

�EJavascript �̃O���[�o����Ԃ͋g���g���S�̂ɑ΂��ĂP�������݂��܂��B
�@
�@Javascript �p�̃X�N���v�g�̎��s�͂��̃O���[�o����ԏ�ł����Ȃ��A
�@��`���ꂽ�t�@���N�V������N���X�����̃O���[�o����Ԃɓo�^����Ă����܂��B

�ETJS2 �̃O���[�o����Ԃ� Javascript ������ "krkr" �ŎQ�Ƃł��܂��B

�EJavascript �̃O���[�o����Ԃ� TJS2 ������ "jsglobal" �ŎQ�Ƃł��܂��B

�E�����A�����A������Ȃǂ̃v���~�e�B�u�l�͒l�n���ɂȂ�܂��B
�ETJS2 �� void �� javascript �� undefined �ƑΉ����܂�
�ETJS2 �� null �� javascript �� null �ƑΉ����܂�
�E�I�u�W�F�N�g�͑��݂ɌĂяo���\�ł�

��Scripts �g��

Javascript �̎��s�p���\�b�h�� Scripts �N���X�Ɋg������܂��B
����ɂ��O���� Javascript �t�@�C����ǂݍ���Ŏ��s�\�ɂȂ�܂�

���g���g���N���X�� javascript�v���g�^�C�v�N���X��

�g���g���̃N���X�� javascript �̃v���g�^�C�v�N���X�Ƃ���
�p���\�ȏ�Ԃň������Ƃ��ł��܂��B

�EcreateTJSClass()�ŁATJS�̃N���X������I�ɕێ����� 
�@Javascript�N���X�����֐����쐬���邱�Ƃ��ł��܂��B

  tjsOverride() ��TJS�C���X�^���X�ɑ΂��Ē��� javascript ���\�b�h��
  �o�^�ł��܂�

  TJS�C���X�^���X���� callJS() �Ƃ��� javascript �C���X�^���X��
�@���\�b�h�𖾎��I�ɌĂяo�����߂��g������܂��B

  TJS�C���X�^���X���ł� missing �@�\���ݒ肳��A���݂��Ȃ������o��
�@�Q�Ƃ��ꂽ�ꍇ�� javascript �C���X�^���X�̓��������o���Q�Ƃ���܂��B
  TJS�C���X�^���X��������̃C�x���g�Ăяo���ɂ����ꂪ�K�p����邽�߁A
�@TJS�C���X�^���X���ɒ�`���Ȃ���Ύ����I�� javascript �C���X�^���X��
�@���ꂪ�Ăяo����܂�

  ��squirrel �����Ƃ͈قȂ肱�̋@�\�Ŏ擾�����g���g���N���X��
  �g���g�����Ő������ꂽ�C���X�^���X���Ԃ����ꍇ�̃��b�s���O������
�@�s���܂���̂ł����ӂ�������

���f�o�b�K�@�\

  enableDebugJS() �Ńf�o�b�K��L���ɂ���ƁA
�@�O������� TCP/IP�ڑ��Ń����[�g�f�o�b�O�\�ɂȂ�܂��B
�@�������[�g�f�o�b�K�Ƃ��� V8 �t���� d8.exe ���g���܂��B

 d8 �̋N�����@
  > d8.exe --remote_debugger --debugger_port=5858

�@�f�o�b�K�ɂ���� Javascript �̎��s�����f���Ă���Ԃ͋g���g���S�̂�
�@��~��ԂɂȂ�A��ʂ̍X�V��C�x���g�����Ȃǂ���~����̂Œ��ӂ��K�v�ł��B

�@�t�� Javascript ���s��ԂłȂ��Ԃ́A���̂܂܂ł̓f�o�b�K�ɑ΂���
�@�������A��܂���BTJS ���ŁAprocessDebugJS() �����I�ɌĂяo����
�@�ʐM������������K�v������̂Œ��ӂ��Ă��������B

�����C�Z���X

���̃v���O�C�����̂̃��C�Z���X�͋g���g���{�̂ɏ������Ă��������B

Node's license follows:

====

Copyright Joyent, Inc. and other Node contributors. All rights reserved.
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.

====

This license applies to all parts of Node that are not externally
maintained libraries. The externally maintained libraries used by Node are:

- V8, located at deps/v8. V8's license follows:
  """
    This license applies to all parts of V8 that are not externally
    maintained libraries.  The externally maintained libraries used by V8
    are:

      - PCRE test suite, located in
        test/mjsunit/third_party/regexp-pcre.js.  This is based on the
        test suite from PCRE-7.3, which is copyrighted by the University
        of Cambridge and Google, Inc.  The copyright notice and license
        are embedded in regexp-pcre.js.

      - Layout tests, located in test/mjsunit/third_party.  These are
        based on layout tests from webkit.org which are copyrighted by
        Apple Computer, Inc. and released under a 3-clause BSD license.

      - Strongtalk assembler, the basis of the files assembler-arm-inl.h,
        assembler-arm.cc, assembler-arm.h, assembler-ia32-inl.h,
        assembler-ia32.cc, assembler-ia32.h, assembler-x64-inl.h,
        assembler-x64.cc, assembler-x64.h, assembler-mips-inl.h,
        assembler-mips.cc, assembler-mips.h, assembler.cc and assembler.h.
        This code is copyrighted by Sun Microsystems Inc. and released
        under a 3-clause BSD license.

      - Valgrind client API header, located at third_party/valgrind/valgrind.h
        This is release under the BSD license.

    These libraries have their own licenses; we recommend you read them,
    as their terms may differ from the terms below.

    Copyright 2006-2012, the V8 project authors. All rights reserved.
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
          notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above
          copyright notice, this list of conditions and the following
          disclaimer in the documentation and/or other materials provided
          with the distribution.
        * Neither the name of Google Inc. nor the names of its
          contributors may be used to endorse or promote products derived
          from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
    A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
