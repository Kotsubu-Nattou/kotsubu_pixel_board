/**************************************************************************************************
�y�w�b�_�I�����[�N���X�zkotsubu_pixel_board

�E�T�v
�h�b�g�̂��G�����t�B�[���h��񋟂���N���X�iOpenSiv3D��p�j
����͕s�v�i�A�v���P�[�V�����I�����Ɏ����j

�E�g����
#include <Siv3D.hpp>
#include "kotsubu_pixel_board.h"
KotsubuPixelBoard board(8.0);  // �𑜓x�X�P�[����8.0�i�h�b�g�̑傫���j�̂��G�����t�B�[���h�𐶐�
���C�����[�v
    if (flag) setResoScale(20.0);               // �𑜓x�X�P�[����ύX�i�t�B�[���h�͔����ɖ߂�j
    board.clear();                              // �t�B�[���h�𔒎��ɖ߂�
    int w = board.mImg.width();                 // �����omImg�̓t�B�[���h�̓��e�Bs3d::Image�^
    s3d::Point point = �J�[�\�����̍��W���t�B�[���h���W�ɂ�������;
    board.mImg[point].set(s3d::Palette::Cyan);  // �_�������_�����O�i�Y�����͈͂ɒ��ӁI�j
    board.mPos = { 0.0, 5.0 };                  // �t�B�[���h�����炷�Bs3d::Vec2�^
    board.setDrawScale(2.0);                    // �h���[���̃Y�[���i�k�����̓t�B�[���h�����؂��j
    board.draw();                               // �h���[
**************************************************************************************************/

#pragma once
#include <Siv3D.hpp>



class KotsubuPixelBoard
{
    // �y�����t�B�[���h�z
    double              mDrawScale;
    double              mResoScale;
    s3d::Image          mBlankImg;
    s3d::DynamicTexture mTex;



public:
    // �y���J�t�B�[���h�z
    s3d::Vec2  mPos;  // �s�N�Z���{�[�h�̍���ʒu
    s3d::Image mImg;  // �`��C���[�W�B����ɒ���.set()�Ȃǂŏ�������



    // �y�R���X�g���N�^�z
    KotsubuPixelBoard(double resoScale)
    {
        setDrawScale(1.0);
        setResoScale(resoScale);
    }



    // �y���\�b�h�z�`��C���[�W�𔒎��ɖ߂�
    // ���݂̉𑜓x�X�P�[���ɉ����������ȃN���A�B�܂��A�����悤�ȗp�r�Ƃ���
    // s3d::Image��clear()�����邪���e���j������Ă��܂��Bfill()�͕��ׂ�����
    void clear()
    {
        // �`��C���[�W���u�����N�C���[�W�Œu��������i���̕��@�������j
        mImg = mBlankImg;
    }



    // �y�Z�b�^�z�𑜓x�X�P�[��
    void setResoScale(double scale)
    {
        static double oldScale = -1;
        if (scale < 1.0) scale = 1.0;
        if (scale == oldScale) return;
        mResoScale = scale;

        // �V�����T�C�Y�̃u�����N�C���[�W�����
        double rate = 1.0 / mResoScale;
        mBlankImg = s3d::Image(static_cast<size_t>(s3d::Window::Width()  * rate),
                               static_cast<size_t>(s3d::Window::Height() * rate));

        // ���I�e�N�X�`���́u�����T�C�Y�v�̃C���[�W���������Ȃ��ƍX�V����Ȃ����߈�U����B
        // ���⑫�� �e�N�X�`����C���[�W��release()��clear()�ƁAdraw()���ʏ��̏ꍇ�A
        // �u�������v�̃A�N�Z�X�����ɒ��ӂ���B�܂��A�e�N�X�`���o�^�Ȃǂ̏d��������
        // �A���ōs�����ꍇ�ɃG���[���邱�Ƃ�����̂Œ��ӂ���B
        mTex.release();

        // �`��C���[�W���N���A�B
        // �`��X�P�[���̕ύX�ł͓��e�͕ς��Ȃ��������R�B�𑜓x�X�P�[����
        // �ύX�ł̓��Z�b�g�����������W�b�N���ȒP�ɂȂ��Ă悢
        mImg = mBlankImg;

        oldScale = mResoScale;
    }



    // �y�Z�b�^�z�`��X�P�[��
    void setDrawScale(double scale)
    {
        if (scale < 0.0) scale = 0.0;
        mDrawScale = scale;
    }



    // �y���\�b�h�z�h���[
    void draw()
    {
        // ���I�e�N�X�`�����X�V�i�����傫���łȂ��ƍX�V����Ȃ��j
        mTex.fill(mImg);

        // ���I�e�N�X�`�����X�P�[�����O���ăh���[
        mTex.scaled(mResoScale * mDrawScale).draw(mPos);
    }
};
