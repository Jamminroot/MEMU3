#pragma once

struct Configuration {
    int scan_width=400;
    int scan_height=300;
    int scan_horizontal_offset=-200;
    int scan_vertical_offset=-200;
    int close_offset_x=50;
    int close_offset_y=60;
    int far_offset_x=65;
    int far_offset_y=65;
    float sensitivity=2.0f;
    float strength=3.0f;
};