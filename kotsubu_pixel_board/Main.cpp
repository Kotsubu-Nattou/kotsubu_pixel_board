/////////////////////////////////////////////////////////////////////////////////////
//
// ピクセルボードクラスの利用例
//
/////////////////////////////////////////////////////////////////////////////////////

#include <Siv3D.hpp>
#include "kotsubu_pixel_board.h"
#include "kotsubu_color_picker.h"



void Main()
{
    s3d::Window::Resize(1000, 800);
    size_t width = 200, height = 150;
    double scale = 15.0;
    KotsubuPixelBoard  board(width, height, scale);
    std::vector<Point> vtx = { {0, -5}, {5, 5}, {-5, 5} };
    KotsubuColorPicker picker;
    ColorF color = { 0.3, 1.0, 0.8, 0.7 };
    bool   isScrolling = false;
    bool   isDragDrawing = false;
    Point  drawingStartPos;
    Font   font = Font(24);
    int    guiAreaLeft = Window::Width() - 220;
    bool   isGlowEffect = false;
    size_t shape     = (size_t)KotsubuPixelBoard::EnumShape::Dot;
    size_t blendMode = (size_t)KotsubuPixelBoard::EnumBlendMode::Default;



    while (System::Update())
    {
        // マウス右ドラッグで、スクロール
        if (MouseR.down() && (Cursor::Pos().x < guiAreaLeft)) isScrolling = true;
        if (MouseR.up()) isScrolling = false;
        if (isScrolling) board.mPos += Cursor::Delta();



        // マウス操作で、レンダリング
        if (!picker.isMouseOver()) {
            // 図形の種類に応じて分岐
            switch ((KotsubuPixelBoard::EnumShape)shape) {
            case KotsubuPixelBoard::EnumShape::Dot:
                // 点図形
                if (MouseL.pressed() && (Cursor::Pos().x < guiAreaLeft)) {
                    static Point oldPos(-1, -1);
                    // カーソル座標をイメージ座標に変換
                    Point pos = board.toImagePos(Cursor::Pos());
                    // 座標更新、かつ有効範囲ならレンダリング
                    if (pos != oldPos && board.checkRange(pos)) {
                        board.renderDot(pos, color);
                        oldPos = pos;
                    }
                }
                break;


            case KotsubuPixelBoard::EnumShape::Polygon:
                // 多角形
                if (MouseL.down() && (Cursor::Pos().x < guiAreaLeft))
                    board.renderPolygon(vtx, board.toImagePos(Cursor::Pos()), color);
                break;


            default:
                // ライン類。ドラッグ処理を行う
                if (MouseL.down() && (Cursor::Pos().x < guiAreaLeft)) {
                    isDragDrawing = true;
                    drawingStartPos = board.toImagePos(Cursor::Pos());
                }

                if (MouseL.up() && isDragDrawing) {
                    // カーソル座標をイメージ座標に変換
                    Point pos = board.toImagePos(Cursor::Pos());
                    // 有効範囲ならレンダリング
                    if (board.checkRange(pos)) {
                        // ラインの種類に応じて分岐
                        switch ((KotsubuPixelBoard::EnumShape)shape) {
                        case KotsubuPixelBoard::EnumShape::Line:
                            board.renderLine(drawingStartPos, pos, color);       break;

                        case KotsubuPixelBoard::EnumShape::LineAA:
                            board.renderLineAA(drawingStartPos, pos, color);     break;

                        case KotsubuPixelBoard::EnumShape::LineFadein:
                            board.renderLineFadein(drawingStartPos, pos, color); break;
                        }
                    }
                    isDragDrawing = false;
                }
                break;
            }
        }
        
        

        // ピクセルボードをドロー
        board.draw();



        // GUI関係
        Rect(guiAreaLeft, 0, Window::Width() - guiAreaLeft, Window::Height()).draw(Palette::Dimgray);

        if (SimpleGUI::Button(U"Clear", Vec2(Window::Width() - 160, 10)))
            board.clear();

        font(U"Scale: ", scale).draw(Vec2(Window::Width() - 210, 50));
        if (SimpleGUI::Slider(scale, 0.0, 100.0, Vec2(Window::Width() - 210, 80), 200))
            board.setScale(scale);

        if (SimpleGUI::Button(U"Draw color", Vec2(Window::Width() - 210, 130)))
            picker.open(&color);

        if (SimpleGUI::CheckBox(isGlowEffect, U"Glow effect", Vec2(Window::Width() - 210, 180)))
            board.mGlowEffect = isGlowEffect;

        SimpleGUI::RadioButtons(shape, { U"Dot", U"Line", U"Line AA", U"Line fadein", U"Polygon" }, Vec2(Window::Width() - 210, 230));

        if (SimpleGUI::RadioButtons(blendMode, { U"Default", U"Alpha", U"Add", U"AddSoft", U"AddHard", U"Mul" }, Vec2(Window::Width() - 210, 440)))
            board.blendMode((KotsubuPixelBoard::EnumBlendMode)blendMode);



        // カラーピッカーの処理
        picker.process();
    }
}
