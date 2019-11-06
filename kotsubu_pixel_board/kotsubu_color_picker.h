/**************************************************************************************************
�y�w�b�_�I�����[�N���X�zkotsubu_color_picker v1.0
�v���C�u����: OpenSiv3D, kotsubu_pixel_board

���T�v
�F�̑I�����_�C�A���O�E�B���h�E�ōs���BHSV�`���J���[�s�b�J�[�B
�����I�ȉ���͕s�v�B


�����炷��
    1. �F�̕ϐ���p�ӁBMyColor�is3d::ColorF�^�܂���s3d::HSV�^�j�Ƃ���
    2. �_�C�A���O��\���B���̂Ƃ�MyColor�̃A�h���X��n���ăo�C���h����
    3. OK�{�^��     ---  MyColor�̐F�����������ĕ���
       Cancel�{�^�� ---  ������������


�����ӓ_
�E�����t���[�ɂ���
    �_�C�A���O�̕`�揈���́A�N���C�A���g�E�B���h�E�Ɓu���ʁv�̂��߁A
    ���p�҂��`�����}�`�ȂǂɉB��Ă��܂����Ƃ�����B
    �܂��A�_�C�A���O�̓��͑��쏈�����u���ʁv�̂��߁A�_�C�A���O�E�B���h�E��
    ���ɉB�ꂽ�A�N���C�A���g�E�B���h�E�̃{�^���Ȃǂɔ������Ă��܂��B
    <������>
        1. �`��̔�� --- ���C�����[�v�́u�Ō�v��.process()�����s����
        2. ����̔�� --- �N���C�A���g���̑��씻���.isMouseOver()��.isOpen()�ň͂�

�Es3d::ColorF�^�̉^�p�ɂ���
    �E�F����0�ɂ����Ƃ��̋���
        �F���͐F��360�x�͈͂ŕ\���̂ŁA360����0���ɏC������Ă��܂��B

    �E���x��0�ɂ����Ƃ��̋���
        �ϐ��̑���Ȃǂɂ��ARGB�ϊ���������x�ł����������R=G=B=0�ƂȂ�A�F���ƍʓx��
        ��񂪏��ł��Ă��܂��B������ēxHSV�ɕϊ�����ƁA�F��=0(��)�A�ʓx=0�ƂȂ�

    �E����炪���ƂȂ�ꍇ��s3d::HSV�^�ŉ^�p����B����RGB�ϊ��������N���������������


������iMain.cpp�ɂāj
#include <Siv3D.hpp>
#include "kotsubu_color_picker.h"
KotsubuColorPicker picker;
s3d::ColorF col;

���C�����[�v
    if (!picker.isMouseOver()) {
        // �����̓}�E�X�J�[�\�����J���[�s�b�J�[��ɂ���Ƃ��͏������Ȃ�
        �N���C�A���g�̓��͑��씻��Ȃ�...
    }
    �N���C�A���g�̐}�`�`��Ȃ�...
    if (SimpleGUI::Button(U"Open.", Vec2(100, 100))) picker.open(&col);  // �J���[�s�b�J�[���J��
    picker.process();                                                    // �J���[�s�b�J�[�̃��C��
**************************************************************************************************/

#pragma once
#include <Siv3D.hpp>
#include "kotsubu_pixel_board.h"



class KotsubuColorPicker
{
public:
    // �y�R���X�g���N�^�z
    KotsubuColorPicker(s3d::ColorF bgColor = s3d::ColorF(0.15, 0.85)) :
        mScale(2.0), mOpened(false), mBindColorF(nullptr), mBindHSV(nullptr), mCurrentHsv(),
        mBgColor(bgColor), mBgFlameColor(s3d::ColorF(0.1, 1.0)),
        mSelectorColor(s3d::ColorF(0.0, 1.0)), mSelectorSize(7),
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
        mFontNumeral(16, s3d::Typeface::Default, s3d::FontStyle::Default),
        mFontNumeralColor(s3d::ColorF(0.7, 1.0)),
        mFontLetter(20, s3d::Typeface::Default, s3d::FontStyle::Default),
        mFontLetterColor(s3d::ColorF(0.9, 1.0)),
        mBoard(mArea.width(), mArea.height(), mScale)
    {   
        mBoard.mBgColor = mBgColor;
        mPos = { s3d::Window::Size() / 2 - mBoard.getSizeAtClient() / 2 };
    }



    // �y���\�b�h�z�_�C�A���O���J��
    // �����ɐF���󂯎�肽���ϐ��̃A�h���X��n���ăo�C���h����B
    // OK�{�^���Ńo�C���h�����ϐ��ɐF���Ԃ�BCancel�̏ꍇ�͉������Ȃ�
    void open(s3d::ColorF* target)
    {
        mBindColorF = target;
        mBindHSV    = nullptr;
        mCurrentHsv = *target;
        mOpened     = true;
        render();
    }

    // �y���\�b�h�z�_�C�A���O���J��
    // �����ɐF���󂯎�肽���ϐ��̃A�h���X��n���ăo�C���h����B
    // OK�{�^���Ńo�C���h�����ϐ��ɐF���Ԃ�BCancel�̏ꍇ�͉������Ȃ�
    void open(s3d::ColorF* target, s3d::Point pos)
    {
        mPos = pos;
        open(target);
    }

    // �y���\�b�h�z�_�C�A���O���J��
    // �����ɐF���󂯎�肽���ϐ��̃A�h���X��n���ăo�C���h����B
    // OK�{�^���Ńo�C���h�����ϐ��ɐF���Ԃ�BCancel�̏ꍇ�͉������Ȃ�
    void open(s3d::HSV* target)
    {
        mBindColorF = nullptr;
        mBindHSV    = target;
        mCurrentHsv = *target;
        mOpened     = true;
        render();
    }

    // �y���\�b�h�z�_�C�A���O���J��
    // �����ɐF���󂯎�肽���ϐ��̃A�h���X��n���ăo�C���h����B
    // OK�{�^���Ńo�C���h�����ϐ��ɐF���Ԃ�BCancel�̏ꍇ�͉������Ȃ�
    void open(s3d::HSV* target, s3d::Point pos)
    {
        mPos = pos;
        open(target);
    }



    // �y���\�b�h�z�_�C�A���O�����
    // Cancel�{�^���Ɠ��l�B������������
    void close()
    {
        mOpened = false;
    }



    // �y���\�b�h�z�_�C�A���O���J���Ă����true
    bool isOpen()
    {
        return mOpened;
    }



    // �y���\�b�h�z�_�C�A���O��Ƀ}�E�X�J�[�\���������true
    bool isMouseOver()
    {
        if (mOpened)
            return mBoard.isMouseOver();
        else
            return false;
    }



    // �y���\�b�h�z�_�C�A���O�̃��C�������B���씻��ƕ`����s��
    // �_�C�A���O�𗘗p����ꍇ�́A���t���[�����s���邱�Ɓi�J���Ă��Ȃ��Ƃ��̕��ׂ͂قږ����j
    void process()
    {
        if (!mOpened) return;
        static bool isScroll = false, isDragSV = false, isDragH  = false, isDragA  = false;
        static s3d::Point cursorBegin;
        static s3d::HSV   hsvBegin;
        s3d::Point cursor = mBoard.toImagePos(s3d::Cursor::Pos());


        // �}�E�X���_�E��
        if (s3d::MouseL.down()) {
            cursorBegin = cursor;
            hsvBegin  = mCurrentHsv;

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
                if (mBindColorF != nullptr)
                    *mBindColorF = mCurrentHsv;
                else
                    *mBindHSV    = mCurrentHsv;
                close();
            }

            else if (mArea.isHit(cursor))
                isScroll = true;
        }


        // �h���b�O�ƃX�N���[�����̏���
        if (s3d::MouseL.up())
            isScroll = isDragSV = isDragH = isDragA = false;

        if (isDragSV) {
            s3d::Vec2 cursorDist = cursor - cursorBegin;
            mCurrentHsv.s = hsvBegin.s + cursorDist.x / mAreaSV.width();
            mCurrentHsv.v = hsvBegin.v - cursorDist.y / mAreaSV.height();
            if (mCurrentHsv.s < 0.0) mCurrentHsv.s = 0.0;
            if (mCurrentHsv.s > 1.0) mCurrentHsv.s = 1.0;
            if (mCurrentHsv.v < 0.0) mCurrentHsv.v = 0.0;
            if (mCurrentHsv.v > 1.0) mCurrentHsv.v = 1.0;
            render();
        }

        else if (isDragH) {
            s3d::Vec2 cursorDist = cursor - cursorBegin;
            mCurrentHsv.h = hsvBegin.h + cursorDist.x / mAreaH.width() * 360.0;
            if (mCurrentHsv.h < 0.0)   mCurrentHsv.h = 0.0;
            if (mCurrentHsv.h > 360.0) mCurrentHsv.h = 360.0;
            render();
        }

        else if (isDragA) {
            s3d::Vec2 cursorDist = cursor - cursorBegin;
            mCurrentHsv.a = hsvBegin.a + cursorDist.x / mAreaA.width();
            if (mCurrentHsv.a < 0.0) mCurrentHsv.a = 0.0;
            if (mCurrentHsv.a > 1.0) mCurrentHsv.a = 1.0;
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


        // �_�C�A���O�̐}�`���h���[
        mBoard.mPos = mPos;
        mBoard.draw();


        // �_�C�A���O�̕�������h���[
        drawText();
    }





private:
    // �y�����t�B�[���h�z
    s3d::Vec2    mPos;
    double       mScale;
    s3d::ColorF* mBindColorF;
    s3d::HSV*    mBindHSV;
    s3d::HSV     mCurrentHsv;
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



    // �y�����֐��z�_�C�A���O�̐}�`�������_�����O
    void render()
    {
        mBoard.clear();        

        // BG�̘g
        mBoard.renderRectFlame(0, 0, mArea.width() - 1, mArea.height() - 1, mBgFlameColor);


        // SV��`
        double h = mCurrentHsv.h;
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
        left  = mAreaA.left + mCurrentHsv.a * (mAreaA.width() - mSliderKnobSize);
        right = left + mSliderKnobSize;
        mBoard.renderRect(left, mAreaA.top, right, mAreaA.bottom, mSliderKnobColor);


        // SV�Z���N�^�[
        double selectorHalf = mSelectorSize * 0.5;
        left   = mAreaSV.left + mCurrentHsv.s * mAreaSV.width() - selectorHalf;
        top    = mAreaSV.top  + (1.0 - mCurrentHsv.v) * mAreaSV.height() - selectorHalf;
        right  = left + mSelectorSize;
        bottom = top  + mSelectorSize;
        mBoard.renderRectFlame(left, top, right, bottom, mSelectorColor);
        mBoard.renderRectFlame(left - 1, top - 1, right + 1, bottom + 1, mSelectorColor);


        // H�Z���N�^�[
        left   = mAreaH.left + mCurrentHsv.h / 360.0 * mAreaH.width() - selectorHalf;
        right  = left + mSelectorSize;
        mBoard.renderRectFlame(left, mAreaH.top, right, mAreaH.bottom, mSelectorColor);
        mBoard.renderRectFlame(left - 1, mAreaH.top, right + 1, mAreaH.bottom, mSelectorColor);


        // �{�^��
        mBoard.renderRect(mAreaCancel, mButtonColor);
        mBoard.renderRect(mAreaOk, mButtonColor);
    }



    // �y�����֐��z�_�C�A���O�̕�������h���[
    void drawText()
    {
        s3d::RenderStateBlock2D state(s3d::BlendState::Default, s3d::SamplerState::Default2D);
        mFontNumeral(U"A  {:.2f}"_fmt(mCurrentHsv.a)).drawAt(mBoard.toClientPos(mAreaAText.center()), mFontNumeralColor);
        mFontLetter(U"Cancel").drawAt(mBoard.toClientPos(mAreaCancel.center()), mFontLetterColor);
        mFontLetter(U"O K").drawAt(mBoard.toClientPos(mAreaOk.center()), mFontLetterColor);
    }
};