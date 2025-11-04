#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include "Escenario.h"

int main() {
    using namespace Escenario;

    al_init();
    al_install_keyboard();
    al_init_primitives_addon();
    ALLEGRO_DISPLAY* display = al_create_display(SCREEN_W, SCREEN_H);
    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60.0);
    ALLEGRO_FONT* font = al_create_builtin_font();

    al_register_event_source(queue, al_get_display_event_source(display));
    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_timer_event_source(timer));

    bool running = true, redraw = true;
    double simTime = 0.0;
    double speedMult = 1.0;     // 1x / 2x / 5x
    float  spawnP = 0.6f;       // probabilidad por tick

    const double SIM_LIMIT = 300.0; // 5 minutos
    al_start_timer(timer);

    while (running) {
        ALLEGRO_EVENT ev;
        al_wait_for_event(queue, &ev);

        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) running = false;

        if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
            switch (ev.keyboard.keycode) {
            case ALLEGRO_KEY_ESCAPE: running = false; break;
            case ALLEGRO_KEY_1: speedMult = 1.0; break;
            case ALLEGRO_KEY_2: speedMult = 2.0; break;
            case ALLEGRO_KEY_3: speedMult = 5.0; break;
            case ALLEGRO_KEY_EQUALS: case ALLEGRO_KEY_PAD_PLUS:
                spawnP = std::min(0.95f, spawnP + 0.05f); break;
            case ALLEGRO_KEY_MINUS: case ALLEGRO_KEY_PAD_MINUS:
                spawnP = std::max(0.05f, spawnP - 0.05f); break;
            default: break;
            }
        }

        if (ev.type == ALLEGRO_EVENT_TIMER) {
            double dt = (1.0 / 60.0) * speedMult;
            simTime += dt;

            Escenario::probabilistic_spawn(simTime, spawnP);
            Escenario::update((float)dt, simTime);

            redraw = true;
            if (simTime >= SIM_LIMIT) running = false;
        }

        if (redraw && al_is_event_queue_empty(queue)) {
            redraw = false;
            Escenario::draw(font, simTime, speedMult, spawnP);
        }
    }

    // === Fin de simulación: exportar CSV y mostrar pantalla de estadísticas ===
    const char* CSV = "toll_simulation_log.csv";
    auto logs = Escenario::build_logs_snapshot();
    Escenario::export_csv(CSV, logs);
    auto st = Escenario::compute_stats_from_csv(CSV, simTime);

    bool statsRunning = true;
    while (statsRunning) {
        ALLEGRO_EVENT ev;
        while (al_get_next_event(queue, &ev)) {
            if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) { statsRunning = false; break; }
            if (ev.type == ALLEGRO_EVENT_KEY_DOWN && ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
                statsRunning = false; break;
            }
        }
        Escenario::draw_stats_screen(font, st);
        al_rest(1.0 / 30.0);
    }

    al_destroy_font(font);
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);
    al_destroy_display(display);
    return 0;
}
