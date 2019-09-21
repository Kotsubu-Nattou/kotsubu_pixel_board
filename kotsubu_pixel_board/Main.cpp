#include <Siv3D.hpp>
#include "kotsubu_pixel_board.h"



void Main()
{
    Font font = Font(24);
    double resoScale = 30.0, drawScale = 1.0;
    KotsubuPixelBoard board(resoScale);
   


	while (System::Update())
	{
        // ピクセルボードをクリア
        if (MouseR.down()) {
            board.clear();
        }
        

        // ピクセルボードをレンダリング
        if (MouseL.pressed()) {
            // カーソル位置 ⇒ イメージの添え字
            Vec2 pos = Cursor::Pos() - board.mPos;  // カーソル座標にピクセルボード位置を適応
            pos /= resoScale;                       // 解像度スケールを適応
            pos /= drawScale;                       // 描画スケールを適応
            s3d::Point point(pos.asPoint());        // イメージの添え字

            // 有効な添え字なら、イメージに点を書き込む
            if (point.x >= 0 && point.x < board.mImg.width() &&
                point.y >= 0 && point.y < board.mImg.height())
                board.mImg[point].set(Palette::Cyan);
        }
        

        // ピクセルボードをドロー
        s3d::RenderStateBlock2D renderState(s3d::BlendState::Additive, s3d::SamplerState::ClampNearest);
        board.draw();


        // GUI
        renderState = { s3d::BlendState::Default, s3d::SamplerState::Default2D };
        font(U"mPos.x:", board.mPos.x).draw(Vec2(Window::Width() - 200, 0));
        s3d::SimpleGUI::Slider(board.mPos.x, -200.0, 200.0, Vec2(Window::Width() - 200, 30), 200);
        font(U"mPos.y:", board.mPos.y).draw(Vec2(Window::Width() - 200, 60));
        s3d::SimpleGUI::Slider(board.mPos.y, -200.0, 200.0, Vec2(Window::Width() - 200, 90), 200);
        font(U"drawScale:", drawScale).draw(Vec2(Window::Width() - 200, 120));
        if (s3d::SimpleGUI::Slider(drawScale, 0.0, 10.0, Vec2(Window::Width() - 200, 150), 200))
            board.setDrawScale(drawScale);
        if (s3d::SimpleGUI::Button(U"Reset", Vec2(Window::Width() -150, 185))) {
            drawScale = 1.0;
            board.setDrawScale(drawScale);
        }
        font(U"resoScale:", resoScale).draw(Vec2(Window::Width() - 200, 220));
        if (s3d::SimpleGUI::Slider(resoScale, 1.0, 100.0, Vec2(Window::Width() - 200, 250), 200))
            board.setResoScale(resoScale);  // 連続的にコールされたときのエラーはsiv3dの仕様
    }
}
