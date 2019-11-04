/**************************************************************************************************
【ヘッダオンリークラス】kotsubu_color_picker
要ライブラリ: OpenSiv3D, kotsubu_pixel_board

◎概要
色をダイアログウィンドウで選択。カラーピッカー。
明示的な解放は不要。

・変数「MyColor（s3d::ColorF型）」の色を設定する例
    1. ダイアログを表示。このときMyColorのアドレスを渡してバインド
    2. OKボタンでMyColorの色を書き変えて終了
    3. Cancelボタンの場合は何もせず終了


◎注意点
ダイアログの描画仕様は、メインウィンドウの描画と「共通」のため、
利用者が描いた図形などに隠れてしまうことがある。
また、ダイアログの操作系統も「共通」のため、ダイアログウィンドウに隠れた、
メインウィンドウのボタンやクリックなどに反応してしまう。
・上記2つの解決策
    操作の被り --- メインウィンドウ側の操作判定は.isMouseOver()や.isOpen()を利用してブロック
    描画の被り --- メインループの最後で.process()を実行


◎使い方（Main.cppにて）
#include <Siv3D.hpp>
#include "kotsubu_color_picker.h"
メインループ
**************************************************************************************************/

#pragma once
#include <Siv3D.hpp>
#include "kotsubu_pixel_board.h"



class KotsubuColorPicker
{
public:
    // 【コンストラクタ】
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



    // 【メソッド】ダイアログを開く
    // 引数にバインドする変数を指定。
    // OKボタンでバインドした変数に色が返る。Cancelの場合は何もしない
    void open(s3d::ColorF* target)
    {
        mColorHandle = target;
        mCurrentColor = *mColorHandle;
        mOpened = true;
        render();
    }



    // 【メソッド】ダイアログを閉じる
    // Cancelボタンと同様。何もせず閉じる
    void close()
    {
        mOpened = false;
    }



    // 【メソッド】ダイアログが開いているかを返す
    bool isOpen()
    {
        return mOpened;
    }



    // 【メソッド】ダイアログ上にマウスカーソルがあるかを返す
    bool isMouseOver()
    {
        if (mOpened)
            return mBoard.isMouseOver();
        else
            return false;
    }



    // 【メソッド】ダイアログのメインルーチン
    // 内部で描画と操作判定を行う。ダイアログを利用する場合は、毎フレーム実行すること。
    // ダイアログが開いていない場合の負荷はほぼ無し
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


        // マウス左ダウン
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
                // 何もせず閉じる
                close();

            else if (mAreaOk.isHit(cursor)) {
                // 色を返して閉じる
                *mColorHandle = mCurrentColor;
                close();
            }

            else if (mArea.isHit(cursor))
                // スクロールを開始
                isScroll = true;
        }


        // ドラッグとスクロール時の処理
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


        // ウィンドウをドロー
        mBoard.mPos = mPos;
        mBoard.draw();


        // テキストをドロー
        drawText();
    }





private:
    // 【内部フィールド】
    double       mScale;
    s3d::Vec2    mPos;
    s3d::ColorF* mColorHandle;
    s3d::HSV     mCurrentColor;
    bool         mOpened;
    // レイアウト用
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
    // ピクセルボード
    KotsubuPixelBoard mBoard;



    void render()
    {
        mBoard.clear();        

        // BGの枠
        mBoard.renderRectFlame(0, 0, mArea.width() - 1, mArea.height() - 1, mBgFlameColor);


        // SV矩形
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


        // Hバー
        double stepH = 1.0 / mAreaH.width() * 360.0;
        h = 0.0;
        for (int x = mAreaH.left; x <= mAreaH.right; ++x) {
            KotsubuPixelBoard::Point2 line = { x, mAreaH.top, x, mAreaH.bottom };
            mBoard.renderLine(line.begin(), line.end(), s3d::HSV(h, 1.0, 1.0));
            h += stepH;
        }


        // Aスライダー
        mBoard.renderRect(mAreaA, mSliderColor);
        int left, top, right, bottom;
        left  = mAreaA.left + mCurrentColor.a * (mAreaA.width() - mSliderKnobSize);
        right = left + mSliderKnobSize;
        mBoard.renderRect(left, mAreaA.top, right, mAreaA.bottom, mSliderKnobColor);


        // SVセレクター
        double selectorHalf = mSelectorSize * 0.5;
        left   = mAreaSV.left + mCurrentColor.s * mAreaSV.width() - selectorHalf;
        top    = mAreaSV.top  + (1.0 - mCurrentColor.v) * mAreaSV.height() - selectorHalf;
        right  = left + mSelectorSize;
        bottom = top  + mSelectorSize;
        mBoard.renderRectFlame(left, top, right, bottom, mSelectorColor);
        mBoard.renderRectFlame(left - 1, top - 1, right + 1, bottom + 1, mSelectorColor);


        // Hセレクター
        left   = mAreaH.left + mCurrentColor.h / 360.0 * mAreaH.width() - selectorHalf;
        right  = left + mSelectorSize;
        mBoard.renderRectFlame(left, mAreaH.top, right, mAreaH.bottom, mSelectorColor);
        mBoard.renderRectFlame(left - 1, mAreaH.top, right + 1, mAreaH.bottom, mSelectorColor);


        // ボタン
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