/**************************************************************************************************
【ヘッダオンリークラス】kotsubu_pixel_board v1.4

◎概要
ドット感あふれる「お絵かきボード」を提供するクラス（OpenSiv3D専用）
1つのインスタンスにつき、1つのイメージを内包した構造。
実用的な速度で、イメージのクリアや「描き変え」に対応。
これによって、1フレームごとのアニメーションなどにも利用できる。
複数のインスタンスを重ねて利用すれば、多重スクロールなども表現できる。
明示的な解放は不要。

・座標系
  1. クライアントの左上を原点とした「ボードの位置」 --- s3d::Vec2型
  2. ボードの左上を原点とした「イメージの座標」     --- s3d::Point型

・レンダリング
  図形の種類別にrender〇〇メソッドを用意。事前にブレンドモードを指定してレンダリングを行う。
  なるべく速度優先のため、レンダリングメソッドで座標の範囲チェックは行わないので注意。
  また、イメージ（s3d::Image mImg）は「公開」しており、通常のs3d::Imageと同等の操作が可能。
  最後にdrawメソッドの実行で、次フレームにて表示。


◎使い方（Main.cppにて）
#include <Siv3D.hpp>
#include "kotsubu_pixel_board.h"
KotsubuPixelBoard board(32, 24, 10.0);                // 32x24ドット、ズーム率10のお絵かきボードを生成
std::vector<Point> vtx = { {0, 0}, {8, 4}, {0, 8} };  // レンダリングする多角形の定義

メインループ
    board.clear();                          // イメージをクリア
    int w = board.mImg.width();             // イメージの幅を取得（通常のs3d::Imageと同等の操作が可能）
    board.blendMode(KotsubuPixelBoard::EnumBlendMode::Alpha); // ブレンドモードを指定（enum定数を利用）

    s3d::Point pos = board.toImagePos(s3d::Cursor::Pos());    // カーソル座標をイメージ座標に変換
    if (board.checkRange(pos)) {                              // イメージの座標範囲内かどうかをチェック
        board.renderDot(pos, Palette::Blue);                  // 点をレンダリング（座標範囲に注意）
        board.renderPolygon(vtx, pos, Palette::Cyan);         // 多角形をレンダリング
        board.mImg[pos].set(Palette::Green);                  // mImgに直接書き込むことも可能
        Circle(pos, 3.0).overwrite(board.mImg, Palette::Red); // 他のs3dメソッドとの組み合わせ
    }
    board.setBoardScale(2.0);                                    // ズーム
    board.mBoardPos = { 0.0, 5.0 };                              // ボードをスクロール
    board.mBoardSamplerState = s3d::SamplerState::ClampNearest;  // ドット感を強調する（デフォルト）
    board.mGlowEffect = true;                                    // グロー効果をON
    board.draw();                                                // ドロー

    board.resize(48, 36);                   // サイズを変更（ボードは白紙になる。高負荷注意）
    board.mVisible = false;                 // 非表示にする
**************************************************************************************************/

#pragma once
#include <vector>
#include <random>
#include <functional>
#include <Siv3D.hpp>



class KotsubuPixelBoard
{
public:
    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    //  【公開】定数、フィールド、コンストラクタ、一般メソッド
    //
    
    // 【公開定数】
    static enum class EnumShape { Dot, Line, LineAA, LineFadein, Polygon };  // レンダリング図形の種類（機能的な意味無し。便宜上用意）
    static enum class EnumBlendMode { Default, Alpha, Additive, AdditiveSoft, AdditiveHard, Multiple };  // ブレンドモード

    
    // 【公開フィールド】
    s3d::Image        mImg;                // 描画用イメージ。直接操作が可能
    s3d::Vec2         mBoardPos;           // ピクセルボードの左上位置
    s3d::BlendState   mBoardBlendState;    // ボードのブレンドステート
    s3d::SamplerState mBoardSamplerState;  // ボードのサンプラーステート
    bool              mVisible;            // 表示非表示の切り替え
    bool              mGlowEffect;         // グロー効果の切り替え


    // 【コンストラクタ】
    KotsubuPixelBoard() : KotsubuPixelBoard(1, 1)  // デフォルトは、最もミニマムな設定（縦横1ドット）
    {}

    KotsubuPixelBoard(size_t width, size_t height, double boardScale = 1.0) :
        mBoardBlendState(s3d::BlendState::Default), mBoardSamplerState(s3d::SamplerState::ClampNearest),
        mVisible(true), mGlowEffect(false)
    {
        mRnd.seed(mRndSeedGen());
        setBoardScale(boardScale);
        resize(width, height);
        blendMode(EnumBlendMode::Default);
    }



    // 【セッタ】ピクセルボードの表示スケール
    void setBoardScale(double boardScale)
    {
        if (boardScale < 0.0) boardScale = 0.0;
        mBoardScale = boardScale;
    }



    // 【ゲッタ】ピクセルボードの表示スケールを返す
    double getBoardScale()
    {
        return mBoardScale;
    }



    // 【メソッド】イメージのサイズを変更（ドット単位。高負荷）
    // 変化がなかった場合は何もしない。変化した場合は描画イメージはクリアされる
    // ＜注意＞ 連続的に異なるサイズを設定すると、高負荷のためエラー落ちする
    void resize(size_t width, size_t height)
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
        mTexFront.release();
        mTexBack.release();

        // 描画用イメージをクリア
        mImg = mBlankImg;

        oldWidth  = width;
        oldHeight = height;
    }

    // 【メソッド】イメージのサイズを変更（指定サイズを1とする逆倍。高負荷）
    // dotScale --- 最小1.0から。1で指定サイズそのまま、2で指定サイズの半分となる（このとき、
    // setBoardScaleメソッドで、表示サイズを2倍（同値）することにより、
    // 表示サイズを保ったままドットを拡大できる）
    // 変化がなかった場合は何もしない。変化した場合は描画イメージはクリアされる
    // ＜注意＞ 連続的に異なるサイズを設定すると、高負荷のためエラー落ちする
    void resize(size_t width, size_t height, double dotScale)
    {
        if (dotScale < 1.0) dotScale = 1.0;
        double rate = 1.0 / dotScale;
        resize(width * rate, height * rate);
    }

    // 【メソッド】イメージのサイズを変更（現在のサイズを1とする逆倍。高負荷）
    // dotScale --- 最小1.0から。1で変化なし、2でサイズが半分となる（このとき、
    // setBoardScaleメソッドで、表示サイズを2倍（同値）することにより、
    // 表示サイズを保ったままドットを拡大できる）
    // 変化がなかった場合は何もしない。変化した場合は描画イメージはクリアされる
    // ＜注意＞ 連続的に異なるサイズを設定すると、高負荷のためエラー落ちする
    void resize(double dotScale)
    {
        resize(mImg.width(), mImg.height(), dotScale);
    }



    // 【メソッド】イメージをクリア
    // 現在のイメージサイズに応じた高速なクリアを行う。似たような用途として
    // s3d::Imageのclear()があるがイメージ自体が破棄される。fill()は負荷が高い
    void clear()
    {
        // 描画用イメージをブランクイメージで置き換える（この方法が高速）
        mImg = mBlankImg;
    }



    // 【メソッド】ドロー
    void draw()
    {
        if (!mVisible) return;
        s3d::RenderStateBlock2D state(mBoardBlendState, mBoardSamplerState);

        // 後面のグロー効果処理（前面に描くと期待通りにならない）
        if (mGlowEffect) {
            s3d::RenderStateBlock2D state(s3d::SamplerState::ClampLinear);
            mTexBack.fill(mImg.brightened(50).gaussianBlurred(5));
            mTexBack.scaled(mBoardScale).draw(mBoardPos);
            mTexBack.scaled(mBoardScale).draw(mBoardPos);
        }

        // 前面の動的テクスチャを更新（同じ大きさでないと更新されない）
        mTexFront.fill(mImg);

        // 前面の動的テクスチャをドロー
        mTexFront.scaled(mBoardScale).draw(mBoardPos);
        //if (mGlowEffect) {
        //    s3d::RenderStateBlock2D state(s3d::BlendState::Additive);
        //    mTexFront.scaled(mBoardScale).draw(mBoardPos);
        //}
    }



    // 【メソッド】クライアント座標をイメージ座標に変換
    // カーソル座標などから、スクロール位置やズーム率を考慮したイメージ座標に変換。
    // イメージ配列の添え字として利用できる（範囲チェック等は行わないので慎重に）
    s3d::Point toImagePos(const s3d::Point& clientPos)
    {
        return ((clientPos - mBoardPos) / mBoardScale).asPoint();
    }



    // 【メソッド】イメージ座標をクライアント座標に変換
    // イメージ座標から、スクロール位置やズーム率を考慮したクライアント座標に変換
    s3d::Point toClientPos(const s3d::Point& imagePos)
    {
        return (imagePos * mBoardScale + mBoardPos).asPoint();
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



    // 【メソッド】ランダム座標を返す（イメージの範囲内。高速）
    s3d::Point randomPos()
    {
        int id = mImg.num_pixels() * (mRnd() / (static_cast<double>(mRnd.max()) + 1));
        return { id % mImg.width(),
                 id / mImg.width() };
    }



    // 【メソッド】レンダーのブレンドモード
    void blendMode(KotsubuPixelBoard::EnumBlendMode blendMode)
    {
        switch (blendMode) {
        case EnumBlendMode::Alpha:
            mFuncBlender = FuncBlender_alpha();        break;

        case EnumBlendMode::Additive:
            mFuncBlender = FuncBlender_additive();     break;

        case EnumBlendMode::AdditiveSoft:
            mFuncBlender = FuncBlender_additiveSoft(); break;

        case EnumBlendMode::AdditiveHard:
            mFuncBlender = FuncBlender_additiveHard(); break;

        case EnumBlendMode::Multiple:
            mFuncBlender = FuncBlender_multiple();     break;

        default:
            mFuncBlender = FuncBlender_default();      break;
        }
    }



    // 【メソッド】指定座標の色を返す
    // 具体的には、イメージの1点の参照を返す（s3d::Color&型）
    // これを、ColorF等で受け取れば単純な色のコピー。ColorF&で受け取ればその座標にアタッチできる
    s3d::Color& spoit(const s3d::Point& pos)
    {
        return mImg[pos];
    }





    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    //  【公開】レンダリング系メソッド
    //

    // 【メソッド】点をレンダリング
    void renderDot(const s3d::Point& pos, const s3d::ColorF& col)
    {
        mFuncBlender(mImg, pos, col);
    }



    // 【メソッド】線分をレンダリング
    void renderLine(s3d::Point startPos, s3d::Point endPos, const s3d::ColorF& col)
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
            int e = dist.x;  // 誤差の初期値（四捨五入のために閾値/2とする）
            for (;;) {
                // 現在位置に点を描く
                mFuncBlender(mImg, now, col);

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
            int e = dist.y;
            for (;;) {
                mFuncBlender(mImg, now, col);

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
    void renderLineAA(s3d::Point startPos, s3d::Point endPos, const s3d::ColorF& col,
                      double aaColorRate = 0.5)
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
            int e = dist.x;  // 誤差の初期値（四捨五入のために閾値/2とする）
            for (;;) {
                // 現在位置に点を描く
                mFuncBlender(mImg, now, col);

                // 始点なら終了
                if (now.x == startPos.x) break;

                // xを「1ドット」移動
                now.x += step.x;

                // 誤差を蓄積
                e += dist2.y;

                // 誤差がたまったら
                if (e >= dist2.x) {
                    mFuncBlender(mImg, now, aaCol);                        // 疑似AA

                    // yを「1ドット」移動
                    now.y += step.y;

                    mFuncBlender(mImg, { now.x - step.x, now.y }, aaCol);  // 疑似AA

                    // 誤差をリセット。超過分を残すのがミソ
                    e -= dist2.x;
                }
            }
        }

        else {
            // y基準
            int e = dist.y;
            for (;;) {
                mFuncBlender(mImg, now, col);

                if (now.y == startPos.y) break;
                now.y += step.y;
                e += dist2.x;

                if (e >= dist2.y) {
                    mFuncBlender(mImg, now, aaCol);
                    now.x += step.x;

                    mFuncBlender(mImg, { now.x, now.y - step.y }, aaCol);
                    e -= dist2.y;
                }
            }
        }
    }



    // 【メソッド】線分をレンダリング（始点からフェードインする。疑似アンチエイリアシング付き）
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
            int e        = dist.x;  // 誤差の初期値（四捨五入のために閾値/2とする）
            int decayLen = (endPos.x - startPos.x) * fadingSectionRate;  // フェード区間の長さ
            int splitX   = startPos.x + decayLen;                        // 分割点x

            // ◎ 終点xから分割点xまでループ（通常のAA付き線分の処理）
            for (;;) {
                // 現在位置に点を描く
                mFuncBlender(mImg, now, col);

                // 分割点ならループを抜ける
                if (now.x == splitX) break;

                // xを「1ドット」移動
                now.x += step.x;

                // 誤差を蓄積
                e += dist2.y;

                // 誤差がたまったら
                if (e >= dist2.x) {
                    mFuncBlender(mImg, now, aaCol);                        // 疑似AA

                    // yを「1ドット」移動
                    now.y += step.y;

                    mFuncBlender(mImg, { now.x - step.x, now.y }, aaCol);  // 疑似AA

                    // 誤差をリセット。超過分を残すのがミソ
                    e -= dist2.x;
                }
            }

            // 始点なら終了
            if (now.x == startPos.x) return;

            // ◎ 分割点xから始点xまでループ（ここがフェードする）
            double alphaFadeVol = col.a / (1 + (double)std::abs(decayLen));  // アルファのフェード量
            for (;;) {
                // 初回の重複描画を避けるためフローを変更
                now.x += step.x;
                e += dist2.y;

                col.a -= alphaFadeVol;  // アルファをフェードアウト

                if (e >= dist2.x) {
                    mFuncBlender(mImg, now, ColorF(col, col.a * aaColorRate));
                    now.y += step.y;
                    mFuncBlender(mImg, { now.x - step.x, now.y }, ColorF(col, col.a * aaColorRate));
                    e -= dist2.x;
                }

                mFuncBlender(mImg, now, col);
                if (now.x == startPos.x) break;
            }
        }

        else {
            // ◎◎ y基準
            int e        = dist.y;
            int decayLen = (endPos.y - startPos.y) * fadingSectionRate;
            int splitY   = startPos.y + decayLen;

            for (;;) {
                mFuncBlender(mImg, now, col);
                if (now.y == splitY) break;

                now.y += step.y;
                e += dist2.x;

                if (e >= dist2.y) {
                    mFuncBlender(mImg, now, aaCol);
                    now.x += step.x;
                    mFuncBlender(mImg, { now.x, now.y - step.y }, aaCol);
                    e -= dist2.y;
                }
            }
            if (now.y == startPos.y) return;
        
            double alphaFadeVol = col.a / (1 + (double)std::abs(decayLen));
            for (;;) {
                now.y += step.y;
                e += dist2.x;

                col.a -= alphaFadeVol;

                if (e >= dist2.y) {
                    mFuncBlender(mImg, now, ColorF(col, col.a * aaColorRate));
                    now.x += step.x;
                    mFuncBlender(mImg, { now.x, now.y - step.y }, ColorF(col, col.a * aaColorRate));
                    e -= dist2.y;
                }

                mFuncBlender(mImg, now, col);
                if (now.y == startPos.y) break;
            }
        }
    }



    // 【メソッド】凸多角形をレンダリング（すべての内角は180°以下）
    // ＜引数＞ vertices --- 多角形を構成する頂点を格納した配列。vector<Point>
    // 図形は閉じていても無くても可。頂点の右回り左回りはどちらでも可。頂点数が3未満なら何もしない。
    // このメソッドのみ座標の範囲チェック等を行うため安全
    void renderPolygon(std::vector<s3d::Point> vertices, s3d::Point pos, s3d::ColorF col)
    {
        // @ アルゴリズム
        // 1. 図形の高さ分の「ラスタ配列」を用意。これには図形外周の左Xと右Xの座標が入る
        // 2. エッジの数だけループ
        // 3.     ラスタ配列[エッジY].左or右 = エッジX
        // 4. 図形の上から高さ分ループ
        // 5.     ラスタ配列[Y].左から、ラスタ配列[Y].右を塗りつぶす
        // このアルゴリズムでは、左か右を向くパックマンは描けるが、上と下では口が塗りつぶされてしまう。
        // 角度によっては凹多角形も表示できるが、便宜上、メソッドの説明文は凸多角形とした。

        // 頂点数のチェック
        if (vertices.size() < 3) return;

        // 変数等の準備
        int modelLeft = INT32_MAX, modelRight = INT32_MIN, modelTop = INT32_MAX, modelBottom = INT32_MIN;
        for (auto& vtx : vertices) {
            if (vtx.x < modelLeft)        modelLeft   = vtx.x;
            else if (vtx.x > modelRight)  modelRight  = vtx.x;
            if (vtx.y < modelTop)         modelTop    = vtx.y;
            else if (vtx.y > modelBottom) modelBottom = vtx.y;
        }
        if ((pos.x + modelLeft >= mImg.width()) || (pos.x + modelRight < 0)) return;

        int renderStartY = pos.y + modelTop;
        if (renderStartY < 0) renderStartY = 0;
        else if (renderStartY >= mImg.height()) return;

        int renderEndY = pos.y + modelBottom;
        if (renderEndY < 0) return;
        else if (renderEndY >= mImg.height()) renderEndY = mImg.height() - 1;

        int modelHeight = modelBottom - modelTop + 1;

        // 1行に左右のモデルX座標を収める構造の「ラスタ配列」を定義。添え字がモデルの高さを表す
        std::vector<std::vector<int>> rasters(modelHeight);


        // モデル上端を0にする
        for (auto& vtx : vertices)
            vtx.y -= modelTop;

        // 図形を閉じる
        if (vertices.back() != vertices.front())
            vertices.emplace_back(vertices.front());


        // 「ラスタ配列」にエッジを書き込む
        for (int i = 0, edgeQty = vertices.size() - 1; i < edgeQty; ++i)
            writeEdge(rasters, vertices[i], vertices[i + 1]);


        // ラスタ処理
        int imgRight = mImg.width() - 1;
        int fixId    = pos.y + modelTop;
        for (int y = renderStartY; y <= renderEndY ; ++y) {
            int renderStartX  = pos.x + rasters[y - fixId].front();
            if (renderStartX < 0) renderStartX = 0;

            int renderEndX = pos.x + rasters[y - fixId].back();
            if (renderEndX > imgRight) renderEndX = imgRight;

            // 1行分の点をレンダリング
            for (int x = renderStartX; x <= renderEndX; ++x)
                mFuncBlender(mImg, { x, y }, col);
        }
    }





private:
    ////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    //  【内部】レンダリング用の関数オブジェクト、内部関数、フィールド
    //

    // 【内部関数オブジェクト】イメージの1点に対するブレンド処理群
    static struct FuncBlender_default {
        void operator()(s3d::Image& img, const s3d::Point& pos, const s3d::ColorF& col)
        {
            img[pos].set(col);  // そのまま上書き
        }
    };



    static struct FuncBlender_alpha {
        void operator()(s3d::Image& img, const s3d::Point& pos, const s3d::ColorF& col)
        {
            // 背景の色をスポイト
            s3d::ColorF base = img[pos];

            // 塗る色と合成
            double revColA = 1.0 - col.a;
            s3d::ColorF mix  = { base * revColA + col * col.a,  // RGB成分
                                 base.a * revColA + col.a };    // アルファ成分

            // 混ぜた色をセット
            img[pos].set(mix);
        }
    };



    static struct FuncBlender_additive {
        void operator()(s3d::Image& img, const s3d::Point& pos, const s3d::ColorF& col)
        {
            s3d::ColorF base = img[pos];
            double revColA = 1.0 - col.a;
            s3d::ColorF mix  = { base + col * col.a,
                                 base.a * revColA + col.a };
            img[pos].set(mix);
        }
    };



    static struct FuncBlender_additiveSoft {
        void operator()(s3d::Image& img, const s3d::Point& pos, const s3d::ColorF& col)
        {
            s3d::ColorF base = img[pos];
            s3d::ColorF mix  = { base + col * col.a,
                                (base.a + col.a) * 0.6 };  // 平均より少し上
            img[pos].set(mix);
        }
    };



    static struct FuncBlender_additiveHard {
        void operator()(s3d::Image& img, const s3d::Point& pos, const s3d::ColorF& col)
        {
            s3d::ColorF base = img[pos];
            s3d::ColorF mix  = { base + col * col.a,
                                 base.a + col.a };
            img[pos].set(mix);
        }
    };



    static struct FuncBlender_multiple {
        void operator()(s3d::Image& img, const s3d::Point& pos, const s3d::ColorF& col)
        {
            s3d::ColorF base = img[pos];
            s3d::ColorF mix  = { base   * col,
                                 base.a * col.a };
            img[pos].set(mix);
        }
    };



    // 【内部関数】エッジを「ラスタ配列」に書き込む
    void writeEdge(std::vector<std::vector<int>>& rasters, s3d::Point startPos, s3d::Point endPos)
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
            // ◎ x基準
            int e = dist.x;  // 誤差の初期値（四捨五入のために閾値/2とする）
            pushEdgeX(rasters[now.y], now.x);  // 現在のラスタにX座標を追加
            for (;;) {
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
                    // 現在のラスタにX座標を追加
                    pushEdgeX(rasters[now.y], now.x);
                    // 誤差をリセット。超過分を残すのがミソ
                    e -= dist2.x;
                }
            }
        }

        else {
            // ◎ y基準
            int e = dist.y;
            for (;;) {
                pushEdgeX(rasters[now.y], now.x);
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



    // 【内部関数】ラスタ1行にX座標を追加（最終的に2個のX座標が昇順で並ぶ）
    void pushEdgeX(std::vector<int>& raster, int x)
    {
        static const int Left  = 0;
        static const int Right = 1;

        if (raster.empty())
            raster.emplace_back(x);
        else {
            if (raster.size() == 1) {
                if (x < raster[Left]) {
                    raster.emplace_back(raster[Left]);
                    raster[Left] = x;
                }
                else
                    raster.emplace_back(x);
            }
            else {
                if (x < raster[Left])
                    raster[Left] = x;
                else if (x > raster[Right])
                    raster[Right] = x;
            }
        }
    }



    // 【内部フィールド】
    double               mBoardScale;
    s3d::Image           mBlankImg;
    s3d::DynamicTexture  mTexFront;
    s3d::DynamicTexture  mTexBack;
    std::function<void(s3d::Image&, const s3d::Point&, const s3d::ColorF&)> mFuncBlender;
    static std::random_device mRndSeedGen;
    static std::mt19937       mRnd;
};





////////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  【内部静的フィールドの定義】
//
std::random_device KotsubuPixelBoard::mRndSeedGen;
std::mt19937       KotsubuPixelBoard::mRnd;
