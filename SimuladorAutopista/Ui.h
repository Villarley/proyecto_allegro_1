#pragma once
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <string>

class UIButton {
public:
    float x, y, w, h;
    std::string text;
    bool hover = false;

    UIButton(float _x, float _y, float _w, float _h, const std::string& _text)
        : x(_x), y(_y), w(_w), h(_h), text(_text) {
    }

    bool isInside(float mx, float my) const {
        return (mx >= x && mx <= x + w && my >= y && my <= y + h);
    }

    void updateHover(float mx, float my) {
        hover = isInside(mx, my);
    }

    void draw(ALLEGRO_FONT* font) const {
        // Fondo del botón
        ALLEGRO_COLOR colorFondo = hover
            ? al_map_rgb(60, 130, 255)  // Azul brillante al pasar el mouse
            : al_map_rgb(40, 40, 40);   // Gris oscuro por defecto

        al_draw_filled_rectangle(x, y, x + w, y + h, colorFondo);

        // Borde
        al_draw_rectangle(x, y, x + w, y + h, al_map_rgb(255, 255, 255), 2.0f);

        // Texto centrado
        al_draw_text(font, al_map_rgb(255, 255, 255),
            x + w / 2, y + (h / 2) - 6, ALLEGRO_ALIGN_CENTER, text.c_str());
    }
};
