/////////////////////////////////////////////////////////////////////////////////////
//
// ピクセルボードクラスの利用例
//
/////////////////////////////////////////////////////////////////////////////////////

#include <Siv3D.hpp>
#include "kotsubu_pixel_board.h"



void Main()
{
    size_t width = 40, height = 30;
    double scale = 20.0;
    KotsubuPixelBoard board(width, height, scale);
    bool isScroll = false;
    Vec2 scrollStartPos;
    bool isCirclePen = false;
    Font font = Font(24);
    int  guiAreaLeft = Window::Width() - 220;



    while (System::Update())
    {
        // ピクセルボードをドラッグしてスクロール
        if (MouseR.down() &&
            Cursor::Pos().x < guiAreaLeft) {
            isScroll = true;
            scrollStartPos = board.mBoardPos - Cursor::Pos();
        }

        if (MouseR.up())
            isScroll = false;
        
        if (isScroll)
            board.mBoardPos = scrollStartPos + Cursor::Pos();



        // ピクセルボードをレンダリング
        if (MouseL.pressed() &&
            Cursor::Pos().x < guiAreaLeft) {
            // カーソル座標をイメージ座標に変換
            Point pos = board.toImagePos(Cursor::Pos());
            // 有効な座標なら
            if (board.checkRange(pos)) {
                if (isCirclePen)
                    // mImgは通常のs3d::Image型と同じことが可能
                    Circle(pos, 2.0).overwrite(board.mImg, Palette::Darkorange);
                else
                    // クラスの公開メンバに直接書き込み
                    board.mImg[pos].set(s3d::Palette::Cyan);
            }
        }
        


        // ピクセルボードをドロー
        s3d::RenderStateBlock2D renderState(s3d::BlendState::Additive, s3d::SamplerState::ClampNearest);
        board.draw();



        // GUI関係
        renderState = { s3d::BlendState::Default, s3d::SamplerState::Default2D };

        Rect(guiAreaLeft, 0, Window::Width() - guiAreaLeft, Window::Height()).draw(Palette::Dimgray);

        if (s3d::SimpleGUI::Button(U"Clear", Vec2(Window::Width() - 160, 20)))
            board.clear();

        font(U"Pos: ", (int)board.mBoardPos.x, U", ", (int)board.mBoardPos.y).draw(Vec2(Window::Width() - 210, 70));

        double w = width, h = height;  // スライダー用に型を変換
        font(U"Width: ", width).draw(Vec2(Window::Width() - 210, 110));
        s3d::SimpleGUI::Slider(w, 0.0, 100.0, Vec2(Window::Width() - 210, 140), 200);

        font(U"Height: ", height).draw(Vec2(Window::Width() - 210, 180));
        s3d::SimpleGUI::Slider(h, 0.0, 100.0, Vec2(Window::Width() - 210, 210), 200);

        width = w; height = h;
        board.setSize(width, height);  // 連続的に異なるサイズを適用すると負荷が高く、エラーするので注意

        font(U"Scale: ", scale).draw(Vec2(Window::Width() - 210, 250));
        if (s3d::SimpleGUI::Slider(scale, 0.0, 100.0, Vec2(Window::Width() - 210, 280), 200))
            board.setScale(scale);

        s3d::SimpleGUI::CheckBox(board.mVisible, U"Visible", Vec2(Window::Width() - 210, 340));

        s3d::SimpleGUI::CheckBox(isCirclePen, U"Circle Pen", Vec2(Window::Width() - 210, 400));

    }
}
