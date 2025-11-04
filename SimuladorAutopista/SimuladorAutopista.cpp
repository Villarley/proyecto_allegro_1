#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_color.h>
#include <iostream>
#include <cstdlib>
#include "Escenario.h"

using namespace Escenario;

int main() {
    // Inicializar Allegro
    if (!al_init()) {
        std::cerr << "Error: No se pudo inicializar Allegro.\n";
        return -1;
    }
    al_init_primitives_addon();
    al_install_mouse();
    al_init_font_addon();
    al_init_ttf_addon();

    ALLEGRO_DISPLAY* display = al_create_display(SCREEN_W, SCREEN_H);
    if (!display) {
        std::cerr << "Error al crear display.\n";
        return -1;
    }

    ALLEGRO_FONT* font = al_create_builtin_font();
    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60.0);
    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();

    al_register_event_source(queue, al_get_display_event_source(display));
    al_register_event_source(queue, al_get_mouse_event_source());
    al_register_event_source(queue, al_get_timer_event_source(timer));

    bool running = true;
    bool redraw = true;
    bool menuOpen = false;

    double simTime = 0.0;
    float speedMult = 1.0f;
    float spawnProb = 0.02f;

    struct UIButton {
        float x, y, w, h;
        const char* label;
    };
    UIButton btnMenu = { SCREEN_W - 100.0f, 15.0f, 80.0f, 30.0f, "MENU" };

    al_start_timer(timer);

    while (running) {
        ALLEGRO_EVENT ev;
        al_wait_for_event(queue, &ev);

        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            running = false;
        }
        else if (ev.type == ALLEGRO_EVENT_TIMER) {
            simTime += (1.0 / 60.0) * speedMult;
            probabilistic_spawn(simTime, spawnProb);
            update((1.0f / 60.0f) * speedMult, simTime);
            redraw = true;
        }
        else if (ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
            float mx = ev.mouse.x;
            float my = ev.mouse.y;

            if (mx > btnMenu.x && mx < btnMenu.x + btnMenu.w &&
                my > btnMenu.y && my < btnMenu.y + btnMenu.h) {
                menuOpen = !menuOpen;
            }

            if (menuOpen) {
                if (mx > btnMenu.x - 150 && mx < btnMenu.x &&
                    my > btnMenu.y + 40 && my < btnMenu.y + 70) {
                    speedMult = std::max(0.2f, speedMult - 0.2f);
                }
                if (mx > btnMenu.x - 150 && mx < btnMenu.x &&
                    my > btnMenu.y + 80 && my < btnMenu.y + 110) {
                    speedMult = std::min(3.0f, speedMult + 0.2f);
                }
                if (mx > btnMenu.x - 150 && mx < btnMenu.x &&
                    my > btnMenu.y + 120 && my < btnMenu.y + 150) {
                    std::cout << "Guardando CSV...\n";
                    export_csv("toll_log.csv");
                }
            }
        }

        if (redraw && al_is_event_queue_empty(queue)) {
            redraw = false;

            draw(font, simTime, speedMult, spawnProb);

            // Botón MENU visible
            al_draw_filled_rectangle(
                btnMenu.x, btnMenu.y,
                btnMenu.x + btnMenu.w, btnMenu.y + btnMenu.h,
                al_map_rgb(30, 144, 255)
            );
            al_draw_rectangle(
                btnMenu.x, btnMenu.y,
                btnMenu.x + btnMenu.w, btnMenu.y + btnMenu.h,
                al_map_rgb(255, 255, 255), 2
            );
            al_draw_text(font, al_map_rgb(255, 255, 255),
                btnMenu.x + 10, btnMenu.y + 8, 0, btnMenu.label);

            // Menú desplegable
            if (menuOpen) {
                al_draw_filled_rectangle(
                    btnMenu.x - 160, btnMenu.y + 35,
                    btnMenu.x, btnMenu.y + 160,
                    al_map_rgb(25, 25, 25)
                );
                al_draw_text(font, al_map_rgb(255, 255, 255),
                    btnMenu.x - 145, btnMenu.y + 45, 0, "Bajar velocidad");
                al_draw_text(font, al_map_rgb(255, 255, 255),
                    btnMenu.x - 145, btnMenu.y + 85, 0, "Subir velocidad");
                al_draw_text(font, al_map_rgb(255, 255, 255),
                    btnMenu.x - 145, btnMenu.y + 125, 0, "Guardar CSV");
            }

            al_flip_display();
        }
    }

    export_csv("toll_log.csv");
    std::cout << "Simulación terminada. Datos guardados en toll_log.csv\n";

    al_destroy_display(display);
    al_destroy_font(font);
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);

    return 0;
}
