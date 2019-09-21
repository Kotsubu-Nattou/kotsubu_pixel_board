/**************************************************************************************************
【ヘッダオンリークラス】kotsubu_pixel_board

・概要
ドットのお絵かきフィールドを提供するクラス（OpenSiv3D専用）
基本的に「画面いっぱい」の1枚のドットフィールドとしての利用を想定（ズームは可能）
フィールドの容量は、現在のウィンドウのサイズを「解像度スケール」で割った分だけ確保。
上記の理由により、ズームアウトした際に「領域外」が露呈することがある。
明示的な解放は不要。


・使い方
#include <Siv3D.hpp>
#include "kotsubu_pixel_board.h"
KotsubuPixelBoard board(8.0);  // 解像度スケールが8.0（ドットの大きさ）のお絵かきフィールドを生成
メインループ
    if (flag) setResoScale(20.0);               // 解像度スケールを変更（フィールドは白紙に戻る）
    board.clear();                              // フィールドを白紙に戻す
    int w = board.mImg.width();                 // メンバmImgはフィールドの内容。s3d::Image型
    s3d::Point point = カーソル等の座標をフィールド座標にしたもの;
    board.mImg[point].set(s3d::Palette::Cyan);  // 点をレンダリング（添え字範囲に注意！）
    board.mPos = { 0.0, 5.0 };                  // フィールドをずらす。s3d::Vec2型
    board.setDrawScale(2.0);                    // ドロー時のズーム（縮小時はフィールドが見切れる）
    board.draw();                               // ドロー
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



    // 【メソッド】描画イメージを白紙に戻す
    // 現在の解像度スケールに応じた高速なクリア。また、似たような用途として
    // s3d::Imageのclear()があるが内容が破棄されてしまう。fill()は負荷が高い
    void clear()
    {
        // 描画イメージをブランクイメージで置き換える（この方法が高速）
        mImg = mBlankImg;
    }



    // 【セッタ】解像度スケール
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



    // 【セッタ】描画スケール
    void setDrawScale(double scale)
    {
        if (scale < 0.0) scale = 0.0;
        mDrawScale = scale;
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
