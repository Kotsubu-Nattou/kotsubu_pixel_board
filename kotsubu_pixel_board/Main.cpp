/////////////////////////////////////////////////////////////////////////////////////
//
// ピクセルボードクラスの使用例
//
/////////////////////////////////////////////////////////////////////////////////////

#include <Siv3D.hpp>
#include "kotsubu_pixel_board.h"



void Main()
{
    size_t width = 40, height = 30;
    double scale = 20.0;
    KotsubuPixelBoard board(width, height, scale);
    Font font = Font(24);
    bool isDrag = false;
    Vec2 dragStartPos;
    bool isCirclePen = false;



	while (System::Update())
	{
        // ピクセルボードをドラッグしてスクロール
        if (MouseR.down()) {
            isDrag = true;
            dragStartPos = board.mPos - Cursor::Pos();
        }

        if (MouseR.up())
            isDrag = false;
        
        if (isDrag)
            board.mPos = dragStartPos + Cursor::Pos();



        // ピクセルボードをレンダリング
        if (MouseL.pressed()) {
            // カーソル位置 ⇒ イメージの添え字
            Vec2 pos = Cursor::Pos() - board.mPos;  // カーソル座標にピクセルボード位置を適応
            pos /= scale;                           // 描画スケールを適応
            s3d::Point point(pos.asPoint());        // イメージの添え字に変換

            // 有効な添え字ならレンダリング
            if (point.x >= 0 && point.x < board.mImg.width() &&
                point.y >= 0 && point.y < board.mImg.height()) {
                if (isCirclePen) {
                    // mImgは通常のs3d::Image型と同じことが可能
                    Circle(point, 2.0).overwrite(board.mImg, Palette::Darkorange);
                }
                else
                    // クラスの公開メンバに直接書き込み
                    board.mImg[point].set(s3d::Palette::Cyan);
            }
        }
        


        // ピクセルボードをドロー
        s3d::RenderStateBlock2D renderState(s3d::BlendState::Additive, s3d::SamplerState::ClampNearest);
        board.draw();



        // GUI処理
        renderState = { s3d::BlendState::Default, s3d::SamplerState::Default2D };

        if (s3d::SimpleGUI::Button(U"Clear", Vec2(Window::Width() - 160, 20)))
            board.clear();

        font(U"Pos: ", (int)board.mPos.x, U", ", (int)board.mPos.y).draw(Vec2(Window::Width() - 210, 70));

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
