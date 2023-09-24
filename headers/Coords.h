#pragma once

class Coords {
public:
    Coords(const int &pX, const int &pY);
    Coords();
    int x, y;
    float vector_length;
    void set(const int pX, const int pY);
    int as_offset(const int &width) const;
private:
    void recalculate_vector_length();
};
