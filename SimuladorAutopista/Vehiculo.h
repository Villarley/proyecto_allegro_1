#pragma once
#include <allegro5/allegro_primitives.h>

struct Vehiculo {
    float x;
    float y;
    float velocidad;
    ALLEGRO_COLOR color;

    Vehiculo(float xPos, float yPos, float vel, ALLEGRO_COLOR col)
        : x(xPos), y(yPos), velocidad(vel), color(col) {
    }

    void actualizar(float delta) {
        y += velocidad * delta;
    }

    void dibujar() const {
        al_draw_filled_rectangle(x - 15, y - 25, x + 15, y + 25, color);
    }
};
