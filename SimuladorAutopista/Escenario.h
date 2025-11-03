#pragma once
#include <allegro5/allegro_primitives.h>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>

// --- Estructura del vehículo ---
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

// --- Espacio de nombres del escenario ---
namespace Escenario {
    const int SCREEN_W = 800;
    const int SCREEN_H = 600;

    std::vector<Vehiculo> autos; // autos activos en pantalla

    // Dibuja la carretera, los carriles y las cabinas del peaje
    void draw_base_scene() {
        // Fondo (césped)
        al_clear_to_color(al_map_rgb(34, 139, 34));

        // Carretera
        al_draw_filled_rectangle(150, 0, 650, SCREEN_H, al_map_rgb(50, 50, 50));

        // Líneas blancas laterales
        al_draw_line(150, 0, 150, SCREEN_H, al_map_rgb(255, 255, 255), 3);
        al_draw_line(650, 0, 650, SCREEN_H, al_map_rgb(255, 255, 255), 3);

        // Línea amarilla central (divide los carriles)
        al_draw_line(400, 0, 400, SCREEN_H, al_map_rgb(255, 255, 0), 3);

        // Zona de peaje
        al_draw_filled_rectangle(150, 500, 650, 550, al_map_rgb(105, 105, 105));

        // Cabinas de peaje (3)
        al_draw_filled_rectangle(200, 450, 270, 500, al_map_rgb(180, 180, 180));
        al_draw_filled_rectangle(365, 450, 435, 500, al_map_rgb(180, 180, 180));
        al_draw_filled_rectangle(530, 450, 600, 500, al_map_rgb(180, 180, 180));
    }

    // Genera un auto nuevo en un carril aleatorio
    void generar_auto() {
        int carril = rand() % 2; // 0 o 1 (dos carriles)
        float x = (carril == 0) ? 300 : 500; // posiciones de carril
        ALLEGRO_COLOR color = al_map_rgb(rand() % 255, rand() % 255, rand() % 255);
        autos.emplace_back(x, -30, 100 + rand() % 80, color);
    }

    // Actualiza la posición de todos los autos
    void actualizar_autos(float delta) {
        for (auto& a : autos) a.actualizar(delta);

        // Eliminar autos que salen de pantalla
        autos.erase(
            std::remove_if(autos.begin(), autos.end(),
                [](const Vehiculo& a) { return a.y > SCREEN_H + 50; }),
            autos.end()
        );
    }

    // Dibuja todos los autos activos
    void dibujar_autos() {
        for (auto& a : autos) a.dibujar();
    }
}
