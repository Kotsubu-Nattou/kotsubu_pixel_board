/**************************************************************************************************
【ヘッダオンリークラス】kotsubu_color_picker v1.0
要ライブラリ: OpenSiv3D, kotsubu_pixel_board

◎概要
色の選択をダイアログウィンドウで行う。HSV形式カラーピッカー。
明示的な解放は不要。


◎あらすじ
    1. 色の変数を用意。MyColor（s3d::ColorF型またはs3d::HSV型）とする
    2. ダイアログを表示。このときMyColorのアドレスを渡してバインドする
    3. OKボタン     ---  MyColorの色を書き換えて閉じる
       Cancelボタン ---  何もせず閉じる


◎注意点
・処理フローについて
    ダイアログの描画処理は、クライアントウィンドウと「共通」のため、
    利用者が描いた図形などに隠れてしまうことがある。
    また、ダイアログの入力操作処理も「共通」のため、ダイアログウィンドウの
    後ろに隠れた、クライアントウィンドウのボタンなどに反応してしまう。
    <解決策>
        1. 描画の被り --- メインループの「最後」で.process()を実行する
        2. 操作の被り --- クライアント側の操作判定を.isMouseOver()や.isOpen()で囲う

・s3d::ColorF型の運用について
    ・色相を0にしたときの挙動
        色相は色を360度範囲で表すので、360°は0°に修正されてしまう。

    ・明度を0にしたときの挙動
        変数の代入などにより、RGB変換処理が一度でも発生するとR=G=B=0となり、色相と彩度の
        情報が消滅してしまう。これを再度HSVに変換すると、色相=0(赤)、彩度=0となる

    ・これらが問題となる場合はs3d::HSV型で運用する。かつRGB変換を引き起こす処理を避ける


◎実例（Main.cppにて）
#include <Siv3D.hpp>
#include "kotsubu_color_picker.h"
KotsubuColorPicker picker;
s3d::ColorF col;

メインループ
    if (!picker.isMouseOver()) {
        // ここはマウスカーソルがカラーピッカー上にあるときは処理しない
        クライアントの入力操作判定など...
    }
    クライアントの図形描画など...
    if (SimpleGUI::Button(U"Open.", Vec2(100, 100))) picker.open(&col);  // カラーピッカーを開く
    picker.process();                                                    // カラーピッカーのメイン
**************************************************************************************************/

#pragma once
#include <Siv3D.hpp>
#include "kotsubu_pixel_board.h"



class KotsubuColorPicker
{
public:
    // 【コンストラクタ】
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



    // 【メソッド】ダイアログを開く
    // 引数に色を受け取りたい変数のアドレスを渡してバインドする。
    // OKボタンでバインドした変数に色が返る。Cancelの場合は何もしない
    void open(s3d::ColorF* target)
    {
        mBindColorF = target;
        mBindHSV    = nullptr;
        mCurrentHsv = *target;
        mOpened     = true;
        render();
    }

    // 【メソッド】ダイアログを開く
    // 引数に色を受け取りたい変数のアドレスを渡してバインドする。
    // OKボタンでバインドした変数に色が返る。Cancelの場合は何もしない
    void open(s3d::ColorF* target, s3d::Point pos)
    {
        mPos = pos;
        open(target);
    }

    // 【メソッド】ダイアログを開く
    // 引数に色を受け取りたい変数のアドレスを渡してバインドする。
    // OKボタンでバインドした変数に色が返る。Cancelの場合は何もしない
    void open(s3d::HSV* target)
    {
        mBindColorF = nullptr;
        mBindHSV    = target;
        mCurrentHsv = *target;
        mOpened     = true;
        render();
    }

    // 【メソッド】ダイアログを開く
    // 引数に色を受け取りたい変数のアドレスを渡してバインドする。
    // OKボタンでバインドした変数に色が返る。Cancelの場合は何もしない
    void open(s3d::HSV* target, s3d::Point pos)
    {
        mPos = pos;
        open(target);
    }



    // 【メソッド】ダイアログを閉じる
    // Cancelボタンと同様。何もせず閉じる
    void close()
    {
        mOpened = false;
    }



    // 【メソッド】ダイアログが開いていればtrue
    bool isOpen()
    {
        return mOpened;
    }



    // 【メソッド】ダイアログ上にマウスカーソルがあればtrue
    bool isMouseOver()
    {
        if (mOpened)
            return mBoard.isMouseOver();
        else
            return false;
    }



    // 【メソッド】ダイアログのメイン処理。操作判定と描画を行う
    // ダイアログを利用する場合は、毎フレーム実行すること（開いていないときの負荷はほぼ無し）
    void process()
    {
        if (!mOpened) return;
        static bool isScroll = false, isDragSV = false, isDragH  = false, isDragA  = false;
        static s3d::Point cursorBegin;
        static s3d::HSV   hsvBegin;
        s3d::Point cursor = mBoard.toImagePos(s3d::Cursor::Pos());


        // マウス左ダウン
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
                // 何もせず閉じる
                close();

            else if (mAreaOk.isHit(cursor)) {
                // 色を返して閉じる
                if (mBindColorF != nullptr)
                    *mBindColorF = mCurrentHsv;
                else
                    *mBindHSV    = mCurrentHsv;
                close();
            }

            else if (mArea.isHit(cursor))
                isScroll = true;
        }


        // ドラッグとスクロール時の処理
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


        // ダイアログの図形をドロー
        mBoard.mPos = mPos;
        mBoard.draw();


        // ダイアログの文字列をドロー
        drawText();
    }





private:
    // 【内部フィールド】
    s3d::Vec2    mPos;
    double       mScale;
    s3d::ColorF* mBindColorF;
    s3d::HSV*    mBindHSV;
    s3d::HSV     mCurrentHsv;
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



    // 【内部関数】ダイアログの図形をレンダリング
    void render()
    {
        mBoard.clear();        

        // BGの枠
        mBoard.renderRectFlame(0, 0, mArea.width() - 1, mArea.height() - 1, mBgFlameColor);


        // SV矩形
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
        left  = mAreaA.left + mCurrentHsv.a * (mAreaA.width() - mSliderKnobSize);
        right = left + mSliderKnobSize;
        mBoard.renderRect(left, mAreaA.top, right, mAreaA.bottom, mSliderKnobColor);


        // SVセレクター
        double selectorHalf = mSelectorSize * 0.5;
        left   = mAreaSV.left + mCurrentHsv.s * mAreaSV.width() - selectorHalf;
        top    = mAreaSV.top  + (1.0 - mCurrentHsv.v) * mAreaSV.height() - selectorHalf;
        right  = left + mSelectorSize;
        bottom = top  + mSelectorSize;
        mBoard.renderRectFlame(left, top, right, bottom, mSelectorColor);
        mBoard.renderRectFlame(left - 1, top - 1, right + 1, bottom + 1, mSelectorColor);


        // Hセレクター
        left   = mAreaH.left + mCurrentHsv.h / 360.0 * mAreaH.width() - selectorHalf;
        right  = left + mSelectorSize;
        mBoard.renderRectFlame(left, mAreaH.top, right, mAreaH.bottom, mSelectorColor);
        mBoard.renderRectFlame(left - 1, mAreaH.top, right + 1, mAreaH.bottom, mSelectorColor);


        // ボタン
        mBoard.renderRect(mAreaCancel, mButtonColor);
        mBoard.renderRect(mAreaOk, mButtonColor);
    }



    // 【内部関数】ダイアログの文字列をドロー
    void drawText()
    {
        s3d::RenderStateBlock2D state(s3d::BlendState::Default, s3d::SamplerState::Default2D);
        mFontNumeral(U"A  {:.2f}"_fmt(mCurrentHsv.a)).drawAt(mBoard.toClientPos(mAreaAText.center()), mFontNumeralColor);
        mFontLetter(U"Cancel").drawAt(mBoard.toClientPos(mAreaCancel.center()), mFontLetterColor);
        mFontLetter(U"O K").drawAt(mBoard.toClientPos(mAreaOk.center()), mFontLetterColor);
    }
};