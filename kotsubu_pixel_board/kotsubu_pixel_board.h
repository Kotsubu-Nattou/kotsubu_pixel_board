/**************************************************************************************************
【ヘッダオンリークラス】kotsubu_pixel_board v1.3

◎概要
2Dドットのお絵かきボードを提供するクラス（OpenSiv3D専用）
1つのインスタンスにつき、1つのイメージを内包する構造。
明示的な解放は不要。

・座標系
  1. クライアントの左上を原点とした「ボードの位置」 --- s3d::Vec2型
  2. ボードの左上を原点とした「イメージの座標」     --- s3d::Point型
  また、カーソル座標をイメージ座標に変換するメソッド等も用意。

・レンダリング
  図形の種類別にrender〇〇メソッドを用意。事前にブレンドモードを指定してレンダリングを行う。
  また、イメージ（s3d::Image mImg）は「公開」しており、通常のイメージに対する操作が可能。


◎使い方
#include <Siv3D.hpp>
#include "kotsubu_pixel_board.h"
KotsubuPixelBoard board(32, 24, 10.0);      // 32x24ドット、ズーム率10のお絵かきボードを生成

メインループ
    board.clear();                          // ボードを白紙にする
    int w = board.mImg.width();             // イメージの幅を取得（通常のs3d::Imageと同様の操作が可能）
    board.blendMode(KotsubuPixelBoard::EnumBlendMode::Alpha); // ブレンドモードを指定（enum定数を利用）

    s3d::Point pos = board.toImagePos(Cursor::Pos());         // カーソル座標をイメージ座標に変換
    if (board.checkRange(pos)) {                              // イメージの座標範囲内かどうかをチェック
        board.renderDot(pos, Palette::Blue);                  // 点をレンダリング（座標範囲に注意）
        board.mImg[pos].set(Palette::Green);                  // mImgに直接書き込むことも可能
        Circle(pos, 3.0).overwrite(board.mImg, Palette::Red); // 他のs3dメソッドとの組み合わせ
    }
    board.mBoardPos = { 0.0, 5.0 };         // ボードをスクロール
    board.setScale(2.0);                    // ズーム
    board.draw();                           // ドロー

    board.changeSize(48, 36);               // ドットサイズを変更（ボードは白紙になる。高負荷注意）
    board.mVisible = false;                 // 非表示にする
**************************************************************************************************/

#pragma once
#include <functional>
#include <Siv3D.hpp>



class KotsubuPixelBoard
{
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    //  【公開】定数、フィールド、コンストラクタ、一般メソッド
    //

    // 【公開定数】ブレンドモード
    static enum class EnumBlendMode { Default, Alpha, Additive, AdditiveSoft, Multiple };

    
    // 【公開フィールド】
    s3d::Vec2  mBoardPos;  // ピクセルボードの左上位置
    s3d::Image mImg;       // 描画用イメージ。直接操作が可能
    bool       mVisible;   // 表示非表示の切り替え


    // 【コンストラクタ】
    KotsubuPixelBoard() : KotsubuPixelBoard(s3d::Window::Width(), s3d::Window::Height(), 1.0)
    {}

    KotsubuPixelBoard(size_t width, size_t height, double scale = 1.0)
    {
        mVisible = true;
        setScale(scale);
        changeSize(width, height);
        blendMode(EnumBlendMode::Default);
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



    // 【メソッド】イメージのサイズを変更（単位はドット。高負荷）
    // サイズが変わらない場合は何もしない。変わる場合は描画イメージはクリアされる
    // ＜注意＞ 連続的に異なるサイズを設定すると、高負荷のためエラー落ちする
    void changeSize(size_t width, size_t height)
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



    // 【メソッド】ランダム座標を返す（イメージの範囲内）
    s3d::Point randomPos()
    {
        // Random関数は重いため、「Rnd(横),Rnd(縦)」とするより高速
        int id = s3d::Random(mImg.num_pixels() - 1);
        return { id % mImg.width(),
                 id / mImg.width() };
    }



    // 【メソッド】ブレンドモードを指定
    void blendMode(KotsubuPixelBoard::EnumBlendMode blendMode)
    {
        switch (blendMode) {
        case EnumBlendMode::Alpha:
            mFunctor = FuncBlender_alpha();        break;

        case EnumBlendMode::Additive:
            mFunctor = FuncBlender_additive();     break;

        case EnumBlendMode::AdditiveSoft:
            mFunctor = FuncBlender_additiveSoft(); break;

        case EnumBlendMode::Multiple:
            mFunctor = FuncBlender_multiple();     break;

        default:
            mFunctor = FuncBlender_default();      break;
        }
    }



    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    //  【公開】レンダリング系メソッド
    //

    // 【メソッド】点をレンダリング
    void renderDot(s3d::Point pos, s3d::ColorF col)
    {
        mFunctor(mImg, pos, col);
    }



    // 【メソッド】線分をレンダリング
    void renderLine(s3d::Point startPos, s3d::Point endPos, s3d::ColorF col)
    {   
        // ◎ブレゼンハムアルゴリズム
        // 終点を初期位置として始める
        s3d::Point now = endPos;
        // xとyそれぞれの、距離（絶対値）と進むべき方向（正負）を求める
        s3d::Point dist, step;
        if (endPos.x >= startPos.x)
            { dist.x = endPos.x - startPos.x; step.x = -1; }
        else
            { dist.x = startPos.x - endPos.x; step.x = 1; }
        if (endPos.y >= startPos.y)
            { dist.y = endPos.y - startPos.y; step.y = -1; }
        else
            { dist.y = startPos.y - endPos.y; step.y = 1; }
        // 誤差の判定時に四捨五入する、かつ整数で扱うため、関連パラメータを2倍する
        s3d::Point dist2 = dist * 2;


        if (dist.x >= dist.y) {
            // x基準
            s3d::int32 e = dist.x;  // 誤差の初期値（四捨五入のために閾値/2とする）
            for (;;) {
                // 現在位置に点を描く
                mFunctor(mImg, now, col);

                // 始点なら終了
                if (now.x == startPos.x) break;

                // xを「1ドット」移動
                now.x += step.x;

                // 誤差を蓄積
                e += dist2.y;

                // 誤差がたまったら
                if (e >= dist2.x) {
                    // yを「1ドット」移動
                    now.y += step.y;
                    // 誤差をリセット。超過分を残すのがミソ
                    e -= dist2.x;
                }
            }
        }

        else {
            // y基準
            s3d::int32 e = dist.y;
            for (;;) {
                mFunctor(mImg, now, col);

                if (now.y == startPos.y) break;
                now.y += step.y;
                e += dist2.x;

                if (e >= dist2.y) {
                    now.x += step.x;
                    e -= dist2.y;
                }
            }
        }
    }



    // 【メソッド】線分をレンダリング（疑似アンチエイリアシング付き）
    void renderLineAA(s3d::Point startPos, s3d::Point endPos, s3d::ColorF col, double aaColorRate = 0.5)
    {
        // 終点を初期位置として始める
        s3d::Point now = endPos;
        // xとyそれぞれの、距離（絶対値）と進むべき方向（正負）を求める
        s3d::Point dist, step;
        if (endPos.x >= startPos.x)
            { dist.x = endPos.x - startPos.x; step.x = -1; }
        else
            { dist.x = startPos.x - endPos.x; step.x = 1; }
        if (endPos.y >= startPos.y)
            { dist.y = endPos.y - startPos.y; step.y = -1; }
        else
            { dist.y = startPos.y - endPos.y; step.y = 1; }
        // 誤差の判定時に四捨五入する、かつ整数で扱うため、関連パラメータを2倍する
        s3d::Point dist2 = dist * 2;
        // AA部分の通常部分に対する色の割合
        if (aaColorRate < 0.0) aaColorRate = 0.0;
        if (aaColorRate > 1.0) aaColorRate = 1.0;
        // AA部分の色
        s3d::ColorF aaCol = s3d::ColorF(col, col.a * aaColorRate);


        if (dist.x >= dist.y) {
            // x基準
            s3d::int32 e = dist.x;  // 誤差の初期値（四捨五入のために閾値/2とする）
            for (;;) {
                // 現在位置に点を描く
                mFunctor(mImg, now, col);

                // 始点なら終了
                if (now.x == startPos.x) break;

                // xを「1ドット」移動
                now.x += step.x;

                // 誤差を蓄積
                e += dist2.y;

                // 誤差がたまったら
                if (e >= dist2.x) {
                    mFunctor(mImg, now, aaCol);                        // 疑似AA

                    // yを「1ドット」移動
                    now.y += step.y;

                    mFunctor(mImg, { now.x - step.x, now.y }, aaCol);  // 疑似AA

                    // 誤差をリセット。超過分を残すのがミソ
                    e -= dist2.x;
                }
            }
        }

        else {
            // y基準
            s3d::int32 e = dist.y;
            for (;;) {
                mFunctor(mImg, now, col);

                if (now.y == startPos.y) break;
                now.y += step.y;
                e += dist2.x;

                if (e >= dist2.y) {
                    mFunctor(mImg, now, aaCol);
                    now.x += step.x;

                    mFunctor(mImg, { now.x, now.y - step.y }, aaCol);
                    e -= dist2.y;
                }
            }
        }
    }



    // 【関数】線分をレンダリング（始点からフェードインする。疑似アンチエイリアシング付き）
    void renderLineFadein(s3d::Point startPos, s3d::Point endPos, s3d::ColorF col,
                          double fadingSectionRate = 0.5, double aaColorRate = 0.5)
    {
        // 終点を初期位置として始める
        s3d::Point now = endPos;
        // xとyそれぞれの、距離（絶対値）と進むべき方向（正負）を求める
        s3d::Point dist, step;
        if (endPos.x >= startPos.x)
            { dist.x = endPos.x - startPos.x; step.x = -1; }
        else
            { dist.x = startPos.x - endPos.x; step.x = 1; }
        if (endPos.y >= startPos.y)
            { dist.y = endPos.y - startPos.y; step.y = -1; }
        else
            { dist.y = startPos.y - endPos.y; step.y = 1; }
        // 誤差の判定時に四捨五入する、かつ整数で扱うため、関連パラメータを2倍する
        s3d::Point dist2 = dist * 2;
        // AA部分の通常部分に対する色の割合
        if (aaColorRate < 0.0) aaColorRate = 0.0;
        if (aaColorRate > 1.0) aaColorRate = 1.0;
        // AA部分の色
        s3d::ColorF aaCol = s3d::ColorF(col, col.a * aaColorRate);
        // フェード区間の割合
        if (fadingSectionRate < 0.0) fadingSectionRate = 0.0;
        if (fadingSectionRate > 1.0) fadingSectionRate = 1.0;


        if (dist.x >= dist.y) {
            // ◎◎ x基準
            s3d::int32 e        = dist.x;  // 誤差の初期値（四捨五入のために閾値/2とする）
            s3d::int32 decayLen = (endPos.x - startPos.x) * fadingSectionRate;  // フェード区間の長さ
            s3d::int32 splitX   = startPos.x + decayLen;                        // 分割点x

            // ◎ 終点xから分割点xまでループ（通常のAA付き線分の処理）
            for (;;) {
                // 現在位置に点を描く
                mFunctor(mImg, now, col);

                // 分割点ならループを抜ける
                if (now.x == splitX) break;

                // xを「1ドット」移動
                now.x += step.x;

                // 誤差を蓄積
                e += dist2.y;

                // 誤差がたまったら
                if (e >= dist2.x) {
                    mFunctor(mImg, now, aaCol);                        // 疑似AA

                    // yを「1ドット」移動
                    now.y += step.y;

                    mFunctor(mImg, { now.x - step.x, now.y }, aaCol);  // 疑似AA

                    // 誤差をリセット。超過分を残すのがミソ
                    e -= dist2.x;
                }
            }

            // 始点なら終了
            if (now.x == startPos.x) return;

            // ◎ 分割点xから始点xまでループ（ここがフェードする）
            double alphaFadeVol = col.a / (1 + std::abs(decayLen));  // アルファのフェード量
            for (;;) {
                // 初回の重複描画を避けるためフローを変更
                now.x += step.x;
                e += dist2.y;

                col.a -= alphaFadeVol;  // アルファをフェードアウト

                if (e >= dist2.x) {
                    mFunctor(mImg, now, ColorF(col, col.a * aaColorRate));
                    now.y += step.y;
                    mFunctor(mImg, { now.x - step.x, now.y }, ColorF(col, col.a * aaColorRate));
                    e -= dist2.x;
                }

                mFunctor(mImg, now, col);
                if (now.x == startPos.x) break;
            }
        }

        else {
            // ◎◎ y基準
            s3d::int32 e        = dist.y;
            s3d::int32 decayLen = (endPos.y - startPos.y) * fadingSectionRate;
            s3d::int32 splitY   = startPos.y + decayLen;

            for (;;) {
                mFunctor(mImg, now, col);
                if (now.y == splitY) break;

                now.y += step.y;
                e += dist2.x;

                if (e >= dist2.y) {
                    mFunctor(mImg, now, aaCol);
                    now.x += step.x;
                    mFunctor(mImg, { now.x, now.y - step.y }, aaCol);
                    e -= dist2.y;
                }
            }
            if (now.y == startPos.y) return;
        
            double alphaFadeVol = col.a / (1 + std::abs(decayLen));
            for (;;) {
                now.y += step.y;
                e += dist2.x;

                col.a -= alphaFadeVol;

                if (e >= dist2.y) {
                    mFunctor(mImg, now, ColorF(col, col.a * aaColorRate));
                    now.x += step.x;
                    mFunctor(mImg, { now.x, now.y - step.y }, ColorF(col, col.a * aaColorRate));
                    e -= dist2.y;
                }

                mFunctor(mImg, now, col);
                if (now.y == startPos.y) break;
            }
        }
    }





private:
    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    //  【内部】レンダリング用の関数オブジェクト、フィールド
    //

    // 【内部関数オブジェクト】ブレンド種類別、イメージの1点を書き変える処理群
    static struct FuncBlender_default {
        void operator()(s3d::Image& img, s3d::Point pos, s3d::ColorF col)
        {
            img[pos].set(col);  // 色をセット
        }
    };



    static struct FuncBlender_alpha {
        void operator()(s3d::Image& img, s3d::Point pos, s3d::ColorF col)
        {
            s3d::ColorF src = img[pos];                        // 現時点の色をスポイト
            double alpha = 1.0 - col.a;
            s3d::ColorF dst = { src.r * alpha + col.r * col.a, // 塗りたい色と合成
                                src.g * alpha + col.g * col.a,
                                src.b * alpha + col.b * col.a,
                                src.a * alpha + col.a };
            img[pos].set(dst);                                 // 混ぜた色をセット
        }
    };



    static struct FuncBlender_additive {
        void operator()(s3d::Image& img, s3d::Point pos, s3d::ColorF col)
        {
            s3d::ColorF src = img[pos];
            s3d::ColorF dst = { src.r + col.r * col.a,
                                src.g + col.g * col.a,
                                src.b + col.b * col.a,
                                src.a + col.a };
            img[pos].set(dst);
        }
    };



    static struct FuncBlender_additiveSoft {
        void operator()(s3d::Image& img, s3d::Point pos, s3d::ColorF col)
        {
            s3d::ColorF src = img[pos];
            s3d::ColorF dst = { src.r + col.r * col.a,
                                src.g + col.g * col.a,
                                src.b + col.b * col.a,
                               (src.a + col.a) * 0.5 };  // 平均をとってみた
            img[pos].set(dst);
        }
    };



    static struct FuncBlender_multiple {
        void operator()(s3d::Image& img, s3d::Point pos, s3d::ColorF col)
        {
            s3d::ColorF src = img[pos];
            s3d::ColorF dst = { src.r * col.r,
                                src.g * col.g,
                                src.b * col.b,
                                src.a * col.a };
            img[pos].set(dst);
        }
    };



    // 【内部フィールド】
    double               mScale;
    s3d::Image           mBlankImg;
    s3d::DynamicTexture  mTex;
    std::function<void(s3d::Image&, s3d::Point, s3d::ColorF)> mFunctor;
};
