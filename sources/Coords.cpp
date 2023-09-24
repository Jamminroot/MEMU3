#include "../headers/Coords.h"
#include <cmath>

void Coords::set(const int pX, const int pY) {
    x = pX;
    y = pY;
    recalculate_vector_length();
}

void Coords::recalculate_vector_length() {
    vector_length = sqrtf(powf((float) x, 2) + powf((float) y, 2));
}

Coords::Coords(const int &pX, const int &pY) : x(pX), y(pY) {
    recalculate_vector_length();
}

int Coords::as_offset(const int &width) const {
    return x + y * width;
}

Coords::Coords() : x(0), y(0), vector_length(0) {

}
