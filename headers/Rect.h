#pragma once

struct Rect {
    Rect() {};
    Rect(const int &, const int &, const int &, const int &);
    int width;
    int height;
    int top;
    int left;
};

