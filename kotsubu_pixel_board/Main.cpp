/////////////////////////////////////////////////////////////////////////////////////
//
// ピクセルボードクラスの利用例
//
/////////////////////////////////////////////////////////////////////////////////////

#include <Siv3D.hpp>
#include "kotsubu_pixel_board.h"



//void Main()
//{
//    size_t width = 200, height = 150;
//    double scale = 15.0;
//    KotsubuPixelBoard board(width, height, scale);
//    bool isScroll = false;
//    Vec2 scrollStartPos;
//    bool isDrawing = false;
//    Point drawingStartPos;
//    Font font = Font(24);
//    int  guiAreaLeft = Window::Width() - 220;
//    size_t blendMode = (size_t)KotsubuPixelBoard::EnumBlendMode::Default;
//
//
//
//    while (System::Update())
//    {
//        // 右ドラッグでスクロール
//        if (MouseR.down() && (Cursor::Pos().x < guiAreaLeft)) {
//            isScroll = true;
//            scrollStartPos = board.mBoardPos - Cursor::Pos();
//        }
//
//        if (MouseR.up())
//            isScroll = false;
//        
//        if (isScroll)
//            board.mBoardPos = scrollStartPos + Cursor::Pos();
//
//
//
//        // 左ドラッグで線分をレンダリング
//        if (MouseL.down() && (Cursor::Pos().x < guiAreaLeft)) {
//            isDrawing = true;
//            // カーソル座標をイメージ座標に変換
//            drawingStartPos = board.toImagePos(Cursor::Pos());
//        }
//
//        if (MouseL.up() && isDrawing) {
//            Point pos = board.toImagePos(Cursor::Pos());
//            // 有効範囲ならレンダリング
//            if (board.checkRange(pos))
//                board.renderLineAA(drawingStartPos, pos, ColorF(0.3, 1.0, 0.7, 0.7));
//            isDrawing = false;
//        }
//        
//        
//        
//        // ピクセルボードをドロー
//        s3d::RenderStateBlock2D renderState(s3d::BlendState::Default, s3d::SamplerState::ClampNearest);
//        board.draw();
//
//
//
//        // GUI関係
//        renderState = { s3d::BlendState::Default, s3d::SamplerState::Default2D };
//
//        Rect(guiAreaLeft, 0, Window::Width() - guiAreaLeft, Window::Height()).draw(Palette::Dimgray);
//
//        if (SimpleGUI::Button(U"Clear", Vec2(Window::Width() - 160, 10)))
//            board.clear();
//
//        font(U"Pos: ", (int)board.mBoardPos.x, U", ", (int)board.mBoardPos.y).draw(Vec2(Window::Width() - 210, 50));
//
//        double w = width, h = height;  // スライダー用に型を変換
//        font(U"Width: ", width).draw(Vec2(Window::Width() - 210, 80));
//        SimpleGUI::Slider(w, 0.0, 400.0, Vec2(Window::Width() - 210, 110), 200);
//
//        font(U"Height: ", height).draw(Vec2(Window::Width() - 210, 150));
//        SimpleGUI::Slider(h, 0.0, 300.0, Vec2(Window::Width() - 210, 180), 200);
//
//        width = w; height = h;
//        board.changeSize(width, height);
//
//        font(U"Scale: ", scale).draw(Vec2(Window::Width() - 210, 220));
//        if (SimpleGUI::Slider(scale, 0.0, 100.0, Vec2(Window::Width() - 210, 250), 200))
//            board.setScale(scale);
//
//        if (SimpleGUI::RadioButtons(blendMode, { U"Default", U"Alpha", U"Add", U"AddSoft", U"Mul" }, Vec2(Window::Width() - 210, 300)))
//            board.blendMode((KotsubuPixelBoard::EnumBlendMode)blendMode);
//    }
//}





void Main()
{
    size_t width = 800, height = 600;
    double scale = 1.0;
    KotsubuPixelBoard board(width, height, scale);
    Font font = Font(64, s3d::Typeface::Bold);
    Stopwatch time;
    int max = 100000000;
    board.blendMode(KotsubuPixelBoard::EnumBlendMode::Default);


    while (System::Update())
    {
        if (s3d::Key1.pressed())
            board.blendMode(KotsubuPixelBoard::EnumBlendMode::Default);

        if (s3d::Key2.pressed())
            board.blendMode(KotsubuPixelBoard::EnumBlendMode::Alpha);

        if (s3d::Key3.pressed())
            board.blendMode(KotsubuPixelBoard::EnumBlendMode::Additive);

        if (s3d::Key4.pressed())
            board.blendMode(KotsubuPixelBoard::EnumBlendMode::AdditiveSoft);

        if (s3d::Key5.pressed())
            board.blendMode(KotsubuPixelBoard::EnumBlendMode::Multiple);

        board.clear();
        ColorF col = { RandomColorF(), Random(1.0) };


        // 【テストメモ】
        // ◎条件: Release, 最適化フル, 800x600, max=100000000
        //
        // board.mImg[pos].set(col);
        //      約4600ms
        //
        // board.renderDot(pos, col);
        //      約4600ms （なぜか直代入と変わらなかった）
        //      ・renderDotBlended()の引数にファンクタのインスタンスでは
        //        無く、FuncBlender_no()を指定  ---  変わらない
        //      ・ファンクタの内部を、mImgの参照を受け取って操作するものから
        //        「色を受け取り混ぜて返す」ものにした  ---  約4800ms （少し遅くなった）
        //        「上書き」の性質上、ファンクタ引数の「元の色」は無駄となる。
        //        加算合成などであれば結果が異なる可能性あり
        //      ・ファンクションラッパー版  ---  約4750ms
        //      ・クラス版  ---  約4850ms
        //        各関数は「インターフェースクラス」を継承する。その型のポインタで各関数の実態を指す構造。
        //        試しに、無駄であるが std::function<インターフェースクラス> でやってみたが速度は変わらなかった。
        //        よって、ファンクションラッパーが速いのではなく、クラスの構造がシンプルであるほど速い？
        //
        // board.renderDot_add2(pos, col);
        //      約6800ms
        //      ・ファンクタの内部を、mImgの参照を受け取って操作するものから
        //        「色を受け取り混ぜて返す」ものにした  ---  変わらない
        //
        // 最終テスト:
        // 上記は実践的ではなく、使用する関数が決め打ちのため、最適化が容易と思われる。
        // これを、ブレンドモードメソッドを実装し、ユーザー切り替え出来るようにした  ---  約+500ms
        time.restart();
        for (int i = 0; i < max; ++i) {
            Point pos = board.randomPos();
            // 【テストコード】
            //board.mImg[pos].set(col);
            board.renderDot(pos, col);
        }
        time.pause();


        // ピクセルボードをドロー
        RenderStateBlock2D renderState(BlendState::Default, SamplerState::ClampNearest);
        board.draw();
        font(time.ms(), U" ms").draw(Point(10, 0));
    }
}
