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
    float x_multiplier=0.1f;
    float y_multiplier=0.05f;
    int point_a_distance = 25;
    int point_a_b_distance = 25;
    int point_b_c_distance = 50;
    float multiplier_at_closest=0.01f;
    float multiplier_point_at_point_a=0.4f;
    float multiplier_point_at_point_b=1.0f;
    float multiplier_point_at_point_c=0.0f;
    float multiplier_at_furthest=0.0f;
};