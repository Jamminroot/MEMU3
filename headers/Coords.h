#pragma once
class Coords {
public:
    Coords(const int& pX, const int& pY);
    Coords();
    int x, y;
    float length;
    void set(const int pX, const int pY);
private:
    void recalculate_length();
};
