/**************************************************************************************************
【ヘッダオンリークラス】kotsubu_pixel_board

・概要
ドットのお絵かきボードを提供するクラス（OpenSiv3D専用）
基本的に「画面いっぱい」の1枚のドット領域としての利用を想定。
ボードの容量は、現在のウィンドウのサイズを「解像度スケール」で
割った分しか確保しない（クリアの高速化や簡略化のため）
上記理由により、基本的に画面外にレンダリングはできない。また、ズームアウトや
スクロールした際に「ボードの領域外」が露呈することがある。
レンダリングは、クラスの公開フィールド s3d::Image mImg に対して直接書き込む（高速化というか手抜き）
明示的な解放は不要。

・使い方
#include <Siv3D.hpp>
#include "kotsubu_pixel_board.h"
KotsubuPixelBoard board(8.0);  // 解像度スケールが8.0（ドットの大きさ）のお絵かきフィールドを生成
メインループ
    board.clear();                              // ボードを白紙にする
    int w = board.mImg.width();                 // 公開メンバmImgはボード内容（s3d::Image型）
    s3d::Point point = カーソル等の座標をボード座標にしたもの;
    board.mImg[point].set(s3d::Palette::Cyan);  // 点をレンダリング（添え字範囲に注意！）
    Circle(point, 3.0).overwrite(board.mImg, Palette::Red);  // mImgは通常のs3d::Image型に対する処理が可能
    board.mPos = { 0.0, 5.0 };                  // ボードをずらす（公開メンバmPos）
    board.setDrawScale(2.0);                    // ドロー時のズーム（縮小時はボードが見切れる）
    board.draw();                               // ドロー
    if (flag) setResoScale(20.0);               // 解像度スケールを変更（ボードは白紙になる）
**************************************************************************************************/

#pragma once
#include <Siv3D.hpp>



class KotsubuPixelBoard
{
    // 【内部フィールド】
    double              mDrawScale;
    double              mResoScale;
    s3d::Image          mBlankImg;
    s3d::DynamicTexture mTex;



public:
    // 【公開フィールド】
    s3d::Vec2  mPos;  // ピクセルボードの左上位置
    s3d::Image mImg;  // 描画イメージ。これに直接.set()などで書き込む



    // 【コンストラクタ】
    KotsubuPixelBoard(double resoScale)
    {
        setDrawScale(1.0);
        setResoScale(resoScale);
    }



    // 【セッタ】描画スケール
    void setDrawScale(double scale)
    {
        if (scale < 0.0) scale = 0.0;
        mDrawScale = scale;
    }



    // 【セッタ】解像度スケール
    // 新しいスケールが適応されると、描画イメージはクリア。
    // ＜注意＞ この関数は負荷が高く、連続でコールするとエラーすることがある
    void setResoScale(double scale)
    {
        static double oldScale = -1;
        if (scale < 1.0) scale = 1.0;
        if (scale == oldScale) return;
        mResoScale = scale;

        // 新しいサイズのブランクイメージを作る
        double rate = 1.0 / mResoScale;
        mBlankImg = s3d::Image(static_cast<size_t>(s3d::Window::Width()  * rate),
                               static_cast<size_t>(s3d::Window::Height() * rate));

        // 動的テクスチャは「同じサイズ」のイメージを供給しないと更新されないため一旦解放。
        // ＜補足＞ テクスチャやイメージのrelease()やclear()と、draw()が別所の場合、
        // 「無い物」のアクセス発生に注意する。また、テクスチャ登録などの重い処理を
        // 連続で行った場合に、エラーすることがあるので注意する。
        mTex.release();

        // 描画イメージをクリア。
        // 描画スケールの変更では内容は変えない方が自然。解像度スケールの
        // 変更ではリセットする方が、ロジックが簡単になってよい
        mImg = mBlankImg;

        oldScale = mResoScale;
    }



    // 【メソッド】描画イメージを白紙に戻す
    // 現在の解像度スケールに応じた高速なクリア。また、似たような用途として
    // s3d::Imageのclear()があるが内容が破棄されてしまう。fill()は負荷が高い
    void clear()
    {
        // 描画イメージをブランクイメージで置き換える（この方法が高速）
        mImg = mBlankImg;
    }



    // 【メソッド】ドロー
    void draw()
    {
        // 動的テクスチャを更新（同じ大きさでないと更新されない）
        mTex.fill(mImg);

        // 動的テクスチャをスケーリングしてドロー
        mTex.scaled(mResoScale * mDrawScale).draw(mPos);
    }
};
