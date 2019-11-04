/**************************************************************************************************
�y�w�b�_�I�����[�N���X�zkotsubu_color_picker
�v���C�u����: OpenSiv3D, kotsubu_pixel_board

���T�v
�F���_�C�A���O�E�B���h�E�őI���B�J���[�s�b�J�[�B
�����I�ȉ���͕s�v�B

�E�ϐ��uMyColor�is3d::ColorF�^�j�v�̐F��ݒ肷���
    1. �_�C�A���O��\���B���̂Ƃ�MyColor�̃A�h���X��n���ăo�C���h
    2. OK�{�^����MyColor�̐F�������ς��ďI��
    3. Cancel�{�^���̏ꍇ�͉��������I��


�����ӓ_
�_�C�A���O�̕`��d�l�́A���C���E�B���h�E�̕`��Ɓu���ʁv�̂��߁A
���p�҂��`�����}�`�ȂǂɉB��Ă��܂����Ƃ�����B
�܂��A�_�C�A���O�̑���n�����u���ʁv�̂��߁A�_�C�A���O�E�B���h�E�ɉB�ꂽ�A
���C���E�B���h�E�̃{�^����N���b�N�Ȃǂɔ������Ă��܂��B
�E��L2�̉�����
    ����̔�� --- ���C���E�B���h�E���̑��씻���.isMouseOver()��.isOpen()�𗘗p���ău���b�N
    �`��̔�� --- ���C�����[�v�̍Ō��.process()�����s


���g�����iMain.cpp�ɂāj
#include <Siv3D.hpp>
#include "kotsubu_color_picker.h"
���C�����[�v
**************************************************************************************************/

#pragma once
#include <Siv3D.hpp>
#include "kotsubu_pixel_board.h"



class KotsubuColorPicker
{
public:
    // �y�R���X�g���N�^�z
    KotsubuColorPicker(s3d::Vec2 pos, s3d::ColorF bgColor = s3d::ColorF(0.15, 0.85)) :
        mScale(2.0), mPos(pos), mOpened(false), mColorHandle(nullptr), mCurrentColor(),
        mBgColor(bgColor), mBgFlameColor(s3d::ColorF(0.1, 1.0)),
        mSelectorColor(s3d::ColorF(0.0, 1.0)), mSelectorSize(6),
        mSliderColor(s3d::ColorF(0.03, 0.85)),
        mSliderKnobColor(s3d::ColorF(0.35, 1.0)), mSliderKnobSize(10),
        mButtonColor(s3d::ColorF(0.35, 1.0)),
        mArea(0, 0, 160, 160),
        mAreaSV(10, 15, mArea.width() - 10, mArea.height() / 2),
        mAreaH(10, mAreaSV.bottom + 10, mArea.width() - 10, mAreaSV.bottom + 25),
        mAreaA(10, mAreaH.bottom + 10, mArea.width() - 40, mAreaH.bottom + 20),
        mAreaAText(mAreaA.right + 10, mAreaA.top, mArea.width() - 10, mAreaA.bottom),
        mAreaCancel(10, mArea.height() - 25, mArea.width() / 2 - 7, mArea.height() - 10),
        mAreaOk(mArea.width() / 2 + 7, mArea.height() - 25, mArea.width() - 10, mArea.height() - 10),
        mFontNumeral(17, s3d::Typeface::Default, s3d::FontStyle::Default),
        mFontNumeralColor(s3d::ColorF(0.8, 1.0)),
        mFontLetter(21, s3d::Typeface::Default, s3d::FontStyle::Default),
        mFontLetterColor(s3d::ColorF(0.9, 1.0)),
        mBoard(mArea.width(), mArea.height(), mScale)
    {   
        mBoard.mBgColor = mBgColor;
    }



    // �y���\�b�h�z�_�C�A���O���J��
    // �����Ƀo�C���h����ϐ����w��B
    // OK�{�^���Ńo�C���h�����ϐ��ɐF���Ԃ�BCancel�̏ꍇ�͉������Ȃ�
    void open(s3d::ColorF* target)
    {
        mColorHandle = target;
        mCurrentColor = *mColorHandle;
        mOpened = true;
        render();
    }



    // �y���\�b�h�z�_�C�A���O�����
    // Cancel�{�^���Ɠ��l�B������������
    void close()
    {
        mOpened = false;
    }



    // �y���\�b�h�z�_�C�A���O���J���Ă��邩��Ԃ�
    bool isOpen()
    {
        return mOpened;
    }



    // �y���\�b�h�z�_�C�A���O��Ƀ}�E�X�J�[�\�������邩��Ԃ�
    bool isMouseOver()
    {
        if (mOpened)
            return mBoard.isMouseOver();
        else
            return false;
    }



    // �y���\�b�h�z�_�C�A���O�̃��C�����[�`��
    // �����ŕ`��Ƒ��씻����s���B�_�C�A���O�𗘗p����ꍇ�́A���t���[�����s���邱�ƁB
    // �_�C�A���O���J���Ă��Ȃ��ꍇ�̕��ׂ͂قږ���
    void process()
    {
        if (!mOpened) return;
        static bool isScroll = false;
        static bool isDragSV = false;
        static bool isDragH  = false;
        static bool isDragA  = false;
        static s3d::Point cursorBegin;
        static s3d::HSV   colorBegin;
        s3d::Point cursor = mBoard.toImagePos(s3d::Cursor::Pos());


        // �}�E�X���_�E��
        if (s3d::MouseL.down()) {
            cursorBegin = cursor;
            colorBegin  = mCurrentColor;

            if (mAreaSV.isHit(cursor))
                isDragSV = true;

            else if (mAreaH.isHit(cursor))
                isDragH = true;

            else if (mAreaA.isHit(cursor))
                isDragA = true;

            else if (mAreaCancel.isHit(cursor))
                // ������������
                close();

            else if (mAreaOk.isHit(cursor)) {
                // �F��Ԃ��ĕ���
                *mColorHandle = mCurrentColor;
                close();
            }

            else if (mArea.isHit(cursor))
                // �X�N���[�����J�n
                isScroll = true;
        }


        // �h���b�O�ƃX�N���[�����̏���
        if (s3d::MouseL.up())
            isScroll = isDragSV = isDragH = isDragA = false;

        if (isDragSV) {
            s3d::Vec2 cursorDist = cursor - cursorBegin;
            mCurrentColor.s = colorBegin.s + cursorDist.x / mAreaSV.width();
            mCurrentColor.v = colorBegin.v - cursorDist.y / mAreaSV.height();
            if (mCurrentColor.s < 0.0) mCurrentColor.s = 0.0;
            if (mCurrentColor.s > 1.0) mCurrentColor.s = 1.0;
            if (mCurrentColor.v < 0.0) mCurrentColor.v = 0.0;
            if (mCurrentColor.v > 1.0) mCurrentColor.v = 1.0;
            render();
        }

        else if (isDragH) {
            s3d::Vec2 cursorDist = cursor - cursorBegin;
            mCurrentColor.h = colorBegin.h + cursorDist.x / mAreaH.width() * 360.0;
            if (mCurrentColor.h < 0.0)   mCurrentColor.h = 0.0;
            if (mCurrentColor.h > 360.0) mCurrentColor.h = 360.0;
            render();
        }

        else if (isDragA) {
            s3d::Vec2 cursorDist = cursor - cursorBegin;
            mCurrentColor.a = colorBegin.a + cursorDist.x / mAreaA.width();
            if (mCurrentColor.a < 0.0) mCurrentColor.a = 0.0;
            if (mCurrentColor.a > 1.0) mCurrentColor.a = 1.0;
            render();
        }

        else if (isScroll) {
            mPos += s3d::Cursor::Delta();
            int limit;
            if (mPos.x < (limit = -mArea.width() * mScale + 20))  mPos.x = limit;
            if (mPos.x > (limit = s3d::Window::Width() - 20))     mPos.x = limit;
            if (mPos.y < (limit = -mArea.height() * mScale + 20)) mPos.y = limit;
            if (mPos.y > (limit = s3d::Window::Height() - 20))    mPos.y = limit;
        }


        // �E�B���h�E���h���[
        mBoard.mPos = mPos;
        mBoard.draw();


        // �e�L�X�g���h���[
        drawText();
    }





private:
    // �y�����t�B�[���h�z
    double       mScale;
    s3d::Vec2    mPos;
    s3d::ColorF* mColorHandle;
    s3d::HSV     mCurrentColor;
    bool         mOpened;
    // ���C�A�E�g�p
    s3d::ColorF  mBgColor;
    s3d::ColorF  mBgFlameColor;
    s3d::ColorF  mButtonColor;
    s3d::ColorF  mSelectorColor;
    int          mSelectorSize;
    s3d::ColorF  mSliderColor;
    s3d::ColorF  mSliderKnobColor;
    int          mSliderKnobSize;
    s3d::Font    mFontNumeral, mFontLetter;
    s3d::ColorF  mFontNumeralColor, mFontLetterColor;
    KotsubuPixelBoard::Point2 mArea, mAreaSV, mAreaH, mAreaA, mAreaAText, mAreaCancel, mAreaOk;
    // �s�N�Z���{�[�h
    KotsubuPixelBoard mBoard;



    void render()
    {
        mBoard.clear();        

        // BG�̘g
        mBoard.renderRectFlame(0, 0, mArea.width() - 1, mArea.height() - 1, mBgFlameColor);


        // SV��`
        double h = mCurrentColor.h;
        double stepS = 1.0 / mAreaSV.width();
        double stepV = 1.0 / mAreaSV.height();
        double v = 1.0;
        for (int y = mAreaSV.top; y <= mAreaSV.bottom; ++y) {
            double s = 0.0;
            for (int x = mAreaSV.left; x <= mAreaSV.right; ++x) {
                mBoard.renderDot(x, y, s3d::HSV(h, s, v));
                s += stepS;
            }
            v -= stepV;
        }


        // H�o�[
        double stepH = 1.0 / mAreaH.width() * 360.0;
        h = 0.0;
        for (int x = mAreaH.left; x <= mAreaH.right; ++x) {
            KotsubuPixelBoard::Point2 line = { x, mAreaH.top, x, mAreaH.bottom };
            mBoard.renderLine(line.begin(), line.end(), s3d::HSV(h, 1.0, 1.0));
            h += stepH;
        }


        // A�X���C�_�[
        mBoard.renderRect(mAreaA, mSliderColor);
        int left, top, right, bottom;
        left  = mAreaA.left + mCurrentColor.a * (mAreaA.width() - mSliderKnobSize);
        right = left + mSliderKnobSize;
        mBoard.renderRect(left, mAreaA.top, right, mAreaA.bottom, mSliderKnobColor);


        // SV�Z���N�^�[
        double selectorHalf = mSelectorSize * 0.5;
        left   = mAreaSV.left + mCurrentColor.s * mAreaSV.width() - selectorHalf;
        top    = mAreaSV.top  + (1.0 - mCurrentColor.v) * mAreaSV.height() - selectorHalf;
        right  = left + mSelectorSize;
        bottom = top  + mSelectorSize;
        mBoard.renderRectFlame(left, top, right, bottom, mSelectorColor);
        mBoard.renderRectFlame(left - 1, top - 1, right + 1, bottom + 1, mSelectorColor);


        // H�Z���N�^�[
        left   = mAreaH.left + mCurrentColor.h / 360.0 * mAreaH.width() - selectorHalf;
        right  = left + mSelectorSize;
        mBoard.renderRectFlame(left, mAreaH.top, right, mAreaH.bottom, mSelectorColor);
        mBoard.renderRectFlame(left - 1, mAreaH.top, right + 1, mAreaH.bottom, mSelectorColor);


        // �{�^��
        mBoard.renderRect(mAreaCancel, mButtonColor);
        mBoard.renderRect(mAreaOk, mButtonColor);
    }



    void drawText()
    {
        mFontNumeral(U"A  {:.2f}"_fmt(mCurrentColor.a)).drawAt(mBoard.toClientPos(mAreaAText.center()), mFontNumeralColor);
        mFontLetter(U"Cancel").drawAt(mBoard.toClientPos(mAreaCancel.center()), mFontLetterColor);
        mFontLetter(U"O K").drawAt(mBoard.toClientPos(mAreaOk.center()), mFontLetterColor);
    }
};