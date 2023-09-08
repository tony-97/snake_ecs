#include "game.hpp"
#include <chrono>
#include <raylib.h>
#include <vector>

// void draw_quad_tree_node(const auto& qd_tree_node)
//{
//     DrawRectangleLinesEx(qd_tree_node.mArea, 1.0f, GREEN);
//     for (auto&& leaf : qd_tree_node.mLeafs) {
//         DrawRectangleLinesEx(leaf.mArea, 1.0f, GREEN);
//         draw_quad_tree_node(leaf);
//     }
// }
// void draw_quad_tree(const auto& qd_tree)
//{
//     draw_quad_tree_node(qd_tree.mRoot);
// }

int
main()
{
  Game_t game{};
  game.run();
  // unsigned screen_width  = 1366;
  // unsigned screen_height = 766;
  // QuadTree_t<Rectangle, 2> quad_tree { Rectangle{ 0.0f, 0.0f, screen_width * 1.0f, screen_height * 1.0f }  };
  // std::vector<Rectangle> vr {  };
  // for (int i {}; i < 2000000; ++i) {
  //     Rectangle r = { GetRandomValue(0, screen_width) * 1.0f, GetRandomValue(0, screen_height) * 1.0f,
  //     GetRandomValue(10, 30) * 1.0f, GetRandomValue(10, 30) * 1.0f }; vr.push_back(r); quad_tree.insert(r, r);
  // }
  // Rectangle r { 0.0f, 0.0f, 100.0f, 100.0f };
  // bool use_qd_tree { true };
  // InitWindow(screen_width, screen_height, "quad tree");
  // SetConfigFlags(FLAG_FULLSCREEN_MODE);
  // while (!WindowShouldClose()) {
  //     r.x = GetMouseX();
  //     r.y = GetMouseY();
  //     if (IsKeyPressed(KEY_A)) {
  //         r.width -= 10.0f;
  //         r.height -= 10.0f;
  //     }
  //     if (IsKeyPressed(KEY_D)) {
  //         r.width += 10.0f;
  //         r.height += 10.0f;
  //     }
  //     BeginDrawing();
  //     ClearBackground(BLACK);
  //     if (IsKeyPressed(KEY_SPACE)) {
  //         use_qd_tree = !use_qd_tree;
  //     }
  //     if (use_qd_tree) {
  //         //for (auto&& e : quad_tree.search(Rectangle{0.0f, 0.0f, screen_width * 1.0f, screen_height * 1.0f })) {
  //         DrawPixel(quad_tree[e].x, quad_tree[e].y, DARKGRAY); }
  //         //draw_quad_tree(quad_tree);
  //         for (auto&& e : quad_tree.search(r)) { DrawPixel(quad_tree[e].x, quad_tree[e].y, LIGHTGRAY); }
  //     } else {
  //         //for (auto&& e : vr) { DrawPixel(e.x, e.y, DARKBLUE); }
  //         for (auto&& e : vr) {
  //             if (QuadTree_t<Rectangle>::overlaps(r, e)) {
  //                 DrawPixel(e.x, e.y, SKYBLUE);
  //             }
  //         }
  //     }
  //     DrawRectangleLinesEx(r, 2.0f, RED);
  //     DrawFPS(10, 10);
  //     EndDrawing();
  // }
  // CloseWindow();
  return 0;
}

// #include "raylib.h"
// #include <vector>
// #include <algorithm>
// #include <iostream>
// #include <cstdlib>
// #include <ctime>
// #include <execution>
//
//// Function to interleave bits of x and y
// unsigned int interleaveBits(unsigned int x, unsigned int y) {
//     unsigned int z = 0;
//     for (unsigned int i = 0; i < sizeof(x) * 8; ++i) {
//         z |= ((x & (1 << i)) << i) | ((y & (1 << i)) << (i + 1));
//     }
//     return z;
// }
//
// struct Point {
//     unsigned int x, y;
//     Point(unsigned int x, unsigned int y) : x(x), y(y) {}
// };
//
// struct MortonPoint {
//     unsigned int code;
//     Point point;
//
//     MortonPoint(unsigned int code, Point point) : code(code), point(point) {}
// };
//
// bool compareMortonPoints(const MortonPoint& a, const MortonPoint& b) {
//     return a.code < b.code;
// }
//
// struct LinearQuadtree {
//     std::vector<MortonPoint> mortonPoints;
//
//     void insert(Point point) {
//         unsigned int mortonCode = interleaveBits(point.x, point.y);
//         mortonPoints.push_back(MortonPoint(mortonCode, point));
//     }
//
//     void build() {
//         std::sort(std::execution::par_unseq,mortonPoints.begin(), mortonPoints.end(), compareMortonPoints);
//     }
//
//     std::vector<Point> search(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2) {
//         unsigned int lower = interleaveBits(x1, y1);
//         unsigned int upper = interleaveBits(x2, y2);
//
//         auto lowerBound = std::lower_bound(mortonPoints.begin(), mortonPoints.end(), MortonPoint(lower, Point(0, 0)),
//         compareMortonPoints); auto upperBound = std::upper_bound(mortonPoints.begin(), mortonPoints.end(),
//         MortonPoint(upper, Point(0, 0)), compareMortonPoints);
//
//         std::vector<Point> results;
//         for (auto it = lowerBound; it != upperBound; ++it) {
//             if (it->point.x >= x1 && it->point.x <= x2 && it->point.y >= y1 && it->point.y <= y2) {
//                 results.push_back(it->point);
//             }
//             //results.push_back(it->point);
//         }
//         return results;
//     }
// };
//
// int main() {
//     const int screenWidth = 1366;
//     const int screenHeight = 768;
//
//     InitWindow(screenWidth, screenHeight, "Linear Quadtree Visualization");
//     SetWindowState(FLAG_FULLSCREEN_MODE);
//
//     LinearQuadtree lq;
//     std::vector<Vector2> v {  };
//     std::srand(std::time(nullptr));
//
//     for (int i = 0; i < 1000000; ++i) {
//         unsigned int x = std::rand() % screenWidth;
//         unsigned int y = std::rand() % screenHeight;
//         lq.insert(Point(x, y));
//         v.emplace_back(x, y);
//     }
//
//     lq.build();
//     bool use_tree { false };
//     Rectangle mouseRect = { 0.0f, 0.0f, 100.0f, 100.0f };
//     while (!WindowShouldClose()) {
//         if (IsKeyPressed(KEY_SPACE)) {
//             use_tree = !use_tree;
//         }
//         BeginDrawing();
//         ClearBackground(RAYWHITE);
//         if (use_tree)
//         for (const auto& point : lq.search(mouseRect.x, mouseRect.y, mouseRect.x + 100, mouseRect.y + 100)) {
//             DrawPixel(point.x, point.y, RED);
//         } else
//             for (const auto& point : v) {
//                 if (CheckCollisionPointRec(point, mouseRect))
//                 DrawPixel(point.x, point.y, RED);
//             }
//         DrawRectangleLines(mouseRect.x, mouseRect.y, mouseRect.width, mouseRect.height, GREEN);
//         DrawFPS(10, 10);
//         EndDrawing();
//         Vector2 mousePos = GetMousePosition();
//         mouseRect = { mousePos.x - 50, mousePos.y - 50, 100.0f, 100.0f };
//     }
//
//     CloseWindow();
//     return 0;
// }
