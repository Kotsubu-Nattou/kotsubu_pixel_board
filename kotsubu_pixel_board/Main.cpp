#include <Siv3D.hpp>
#include "kotsubu_pixel_board.h"



void Main()
{
    Font font = Font(24);
    double resoScale = 30.0, drawScale = 1.0;
    KotsubuPixelBoard board(resoScale);
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
            pos /= resoScale;                       // 解像度スケールを適応
            pos /= drawScale;                       // 描画スケールを適応
            s3d::Point point(pos.asPoint());        // イメージの添え字

            // 有効な添え字ならレンダリング
            if (point.x >= 0 && point.x < board.mImg.width() &&
                point.y >= 0 && point.y < board.mImg.height()) {
                if (isCirclePen) {
                    // 通常のs3d::Imageに対する処理が可能
                    Circle(point, 3.0).overwrite(board.mImg, Palette::Darkorange);
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

        if (s3d::SimpleGUI::Button(U"Clear", Vec2(Window::Width() - 150, 20)))
            board.clear();

        font(U"mPos: ", (int)board.mPos.x, U", ", (int)board.mPos.y).draw(Vec2(Window::Width() - 200, 70));

        font(U"drawScale:", drawScale).draw(Vec2(Window::Width() - 200, 120));
        if (s3d::SimpleGUI::Slider(drawScale, 0.0, 10.0, Vec2(Window::Width() - 200, 150), 200))
            board.setDrawScale(drawScale);
        if (s3d::SimpleGUI::Button(U"Reset", Vec2(Window::Width() -150, 190))) {
            drawScale = 1.0;
            board.setDrawScale(drawScale);
        }

        font(U"resoScale:", resoScale).draw(Vec2(Window::Width() - 200, 240));
        if (s3d::SimpleGUI::Slider(resoScale, 1.0, 100.0, Vec2(Window::Width() - 200, 270), 200))
            board.setResoScale(resoScale);  // 連続的にコールされたときのエラーはsiv3dの仕様

        s3d::SimpleGUI::CheckBox(isCirclePen, U"Circle Pen", Vec2(Window::Width() - 200, 320));
    }
}
