#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <iostream>
#include "Escenario.h"

int main() {
    srand(time(nullptr));

    if (!al_init()) return -1;
    if (!al_init_primitives_addon()) return -1;

    ALLEGRO_DISPLAY* display = al_create_display(Escenario::SCREEN_W, Escenario::SCREEN_H);
    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60);
    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
    al_register_event_source(queue, al_get_display_event_source(display));
    al_register_event_source(queue, al_get_timer_event_source(timer));

    al_start_timer(timer);

    double tiempo_generacion = 0.0;
    bool salir = false;
    ALLEGRO_EVENT ev;

    while (!salir) {
        al_wait_for_event(queue, &ev);

        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE)
            salir = true;
        else if (ev.type == ALLEGRO_EVENT_TIMER) {
            tiempo_generacion += 1.0 / 60.0;

            if (tiempo_generacion >= 1.5) { // cada 1.5 s aparece un auto
                Escenario::generar_auto();
                tiempo_generacion = 0.0;
            }

            Escenario::actualizar_autos(1.0 / 60.0);

            Escenario::draw_base_scene();
            Escenario::dibujar_autos();
            al_flip_display();
        }
    }

    al_destroy_timer(timer);
    al_destroy_event_queue(queue);
    al_destroy_display(display);
    return 0;
}
