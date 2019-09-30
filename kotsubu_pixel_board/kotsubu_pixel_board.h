/**************************************************************************************************
【ヘッダオンリークラス】kotsubu_pixel_board v1.2

・概要
ドットのお絵かきボードを提供するクラス（OpenSiv3D専用）
1つのボードにつき、1つの描画イメージを内包する。
座標系       --- クライアント左上を原点とするボードの位置, ボード左上を原点とする描画イメージの座標
レンダリング --- クラスの公開フィールド s3d::Image mImg に対して直接書き込む（高速化というか手抜き）
明示的な解放は不要。

・使い方
#include <Siv3D.hpp>
#include "kotsubu_pixel_board.h"
KotsubuPixelBoard board(32, 24, 10.0);         // 32x24ドット、ズーム率10のお絵かきボードを生成
メインループ
    board.clear();                             // ボードを白紙にする
    int w = board.mImg.width();                // 公開メンバmImgはボードの描画内容（s3d::Image型）
    s3d::Point pos = board.toImagePos(Cursor::Pos());         // カーソル座標をイメージ座標に変換
    if (board.checkRange(pos)) {                              // イメージの範囲内かどうかをチェック
        board.mImg[pos].set(s3d::Palette::Cyan);              // 点をレンダリング（添え字範囲に注意）
        Circle(pos, 3.0).overwrite(board.mImg, Palette::Red); // mImgはs3d::Image型と同じ扱いが可能
    }
    board.mBoardPos = { 0.0, 5.0 };            // ボードをスクロール
    board.setScale(2.0);                       // ズーム
    board.draw();                              // ドロー
    board.setSize(48, 36);                     // ドットサイズを変更（ボードは白紙になる。高負荷注意）
    board.mVisible = false;                    // 非表示にする
**************************************************************************************************/

#pragma once
#include <Siv3D.hpp>



class KotsubuPixelBoard
{
    // 【内部フィールド】
    double              mScale;
    s3d::Image          mBlankImg;
    s3d::DynamicTexture mTex;



public:
    // 【公開フィールド】
    s3d::Vec2  mBoardPos;  // ピクセルボードの左上位置
    s3d::Image mImg;       // 描画用イメージ。これに直接.set()などで書き込んで.draw()
    bool       mVisible;   // 表示非表示の切り替え



    // 【コンストラクタ】
    KotsubuPixelBoard() : KotsubuPixelBoard(s3d::Window::Width(), s3d::Window::Height(), 1.0)
    {}

    KotsubuPixelBoard(size_t width, size_t height, double scale = 1.0)
    {
        mVisible = true;
        setScale(scale);
        setSize(width, height);
    }



    // 【セッタ】ズーム率
    void setScale(double scale)
    {
        if (scale < 0.0) scale = 0.0;
        mScale = scale;
    }



    // 【ゲッタ】ズーム率
    double getScale()
    {
        return mScale;
    }



    // 【セッタ】サイズ（ドット単位）
    // 設定したサイズが以前のサイズから更新した場合、描画イメージはクリアされる。
    // ＜注意＞ この関数は負荷が高く、連続的に異なるサイズを設定するとエラーすることがある
    void setSize(size_t width, size_t height)
    {
        static size_t oldWidth  = -1;
        static size_t oldHeight = -1;
        if (width  < 1) width  = 1;
        if (height < 1) height = 1;
        if ((width == oldWidth) && (height == oldHeight)) return;

        // 新しいサイズのブランクイメージを作る
        mBlankImg = s3d::Image(width, height);

        // 動的テクスチャは「同じサイズ」のイメージを供給しないと更新されないため一旦解放。
        // ＜補足＞ テクスチャやイメージのrelease()やclear()と、draw()が別所の場合、
        // 「無い物」のアクセス発生に注意する。また、テクスチャ登録などの重い処理を
        // 連続で行った場合に、エラーすることがあるので注意する。
        mTex.release();

        // 描画用イメージをクリア
        mImg = mBlankImg;

        oldWidth  = width;
        oldHeight = height;
    }



    // 【メソッド】イメージを白紙に戻す
    // 現在のイメージサイズに応じた高速なクリア。また、似たような用途として
    // s3d::Imageのclear()があるが内容が破棄されてしまう。fill()は負荷が高い
    void clear()
    {
        // 描画用イメージをブランクイメージで置き換える（この方法が高速）
        mImg = mBlankImg;
    }



    // 【メソッド】ドロー
    void draw()
    {
        if (mVisible) {
            // 動的テクスチャを更新（同じ大きさでないと更新されない）
            mTex.fill(mImg);

            // 動的テクスチャをスケーリングしてドロー
            mTex.scaled(mScale).draw(mBoardPos);
        }
    }



    // 【メソッド】クライアント座標をイメージ座標に変換
    // カーソル座標などから、スクロール位置やズーム率を考慮したイメージ座標に変換。
    // イメージ配列の添え字として利用できる（範囲チェック等は行わないので慎重に）
    s3d::Point toImagePos(s3d::Point clientPos)
    {
        return ((clientPos - mBoardPos) / mScale).asPoint();
    }



    // 【メソッド】イメージ座標をクライアント座標に変換
    // イメージ座標から、スクロール位置やズーム率を考慮したクライアント座標に変換
    s3d::Point toClientPos(s3d::Point imagePos)
    {
        return (imagePos * mScale + mBoardPos).asPoint();
    }



    // 【メソッド】イメージ座標の範囲内かどうかを返す
    bool checkRange(s3d::Point imagePos)
    {
        return (imagePos.x >= 0) && (imagePos.x < mImg.width()) &&
               (imagePos.y >= 0) && (imagePos.y < mImg.height());
    }

    bool checkRange(s3d::Vector2D<int> imagePos)
    {
        return checkRange(imagePos.asPoint());
    }
};
