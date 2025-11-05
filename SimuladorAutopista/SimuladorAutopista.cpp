#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include "Escenario.h"
#include <string>
#include <fstream>

enum EstadoApp { MENU, SIMULACION, CONFIG, ESTADISTICAS, SALIR };

int main() {
    using namespace Escenario;

    // --- Inicialización Allegro ---
    al_init();
    al_install_keyboard();
    al_init_primitives_addon();
    al_init_font_addon();
    al_init_ttf_addon();

    ALLEGRO_DISPLAY* display = al_create_display(SCREEN_W, SCREEN_H);
    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();
    ALLEGRO_TIMER* timer = al_create_timer(1.0 / 60.0);
    ALLEGRO_FONT* font = al_create_builtin_font();

    al_register_event_source(queue, al_get_display_event_source(display));
    al_register_event_source(queue, al_get_keyboard_event_source());
    al_register_event_source(queue, al_get_timer_event_source(timer));

    EstadoApp estadoActual = MENU;
    bool running = true;
    bool redraw = true;

    int cabinasConfig = BOOTHS;
    double simTime = 0.0;
    double speedMult = 1.0;
    float spawnP = 0.6f;
    const double SIM_LIMIT = 300.0; // 5 minutos simulados

    std::vector<Registro> logs;
    Stats st;

    al_start_timer(timer);

    // Ruta del CSV (junto al .vcxproj)
    const char* CSV_PATH = "C:\\Temp\\toll_simulation_log.csv";

    // --- Funciones visuales ---
    auto draw_menu = [&](ALLEGRO_FONT* font, int seleccion) {
        al_clear_to_color(al_map_rgb(20, 20, 20));
        al_draw_textf(font, al_map_rgb(255, 255, 255), SCREEN_W / 2, 100, ALLEGRO_ALIGN_CENTER, "SIMULADOR DE TRAFICO Y PEAJE");
        const char* opciones[] = { "Iniciar simulacion", "Configuracion", "Ver estadisticas", "Salir" };
        for (int i = 0; i < 4; i++) {
            ALLEGRO_COLOR c = (i == seleccion) ? al_map_rgb(255, 255, 0) : al_map_rgb(180, 180, 180);
            al_draw_textf(font, c, SCREEN_W / 2, 200 + i * 50, ALLEGRO_ALIGN_CENTER, opciones[i]);
        }
        al_draw_textf(font, al_map_rgb(255, 255, 0), SCREEN_W / 2, SCREEN_H - 40, ALLEGRO_ALIGN_CENTER, "[ENTER] Seleccionar | [ESC] Salir");
        al_flip_display();
        };

    auto draw_config = [&](ALLEGRO_FONT* font, int cabinas) {
        al_clear_to_color(al_map_rgb(30, 30, 30));
        al_draw_textf(font, al_map_rgb(255, 255, 255), SCREEN_W / 2, 150, ALLEGRO_ALIGN_CENTER, "CONFIGURACION");
        al_draw_textf(font, al_map_rgb(200, 200, 200), SCREEN_W / 2, 250, ALLEGRO_ALIGN_CENTER,
            "Numero de cabinas: %d", cabinas);
        al_draw_textf(font, al_map_rgb(255, 255, 0), SCREEN_W / 2, SCREEN_H - 60, ALLEGRO_ALIGN_CENTER,
            "[Flechas Arriba/Abajo] Cambiar  |  [M] Menu");
        al_flip_display();
        };

    int seleccion = 0;

    // --- Bucle principal ---
    while (running) {
        ALLEGRO_EVENT ev;
        al_wait_for_event(queue, &ev);

        if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) running = false;

        // === MENU PRINCIPAL ===
        if (estadoActual == MENU) {
            if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
                switch (ev.keyboard.keycode) {
                case ALLEGRO_KEY_UP:
                    seleccion = (seleccion + 3) % 4; break;
                case ALLEGRO_KEY_DOWN:
                    seleccion = (seleccion + 1) % 4; break;
                case ALLEGRO_KEY_ENTER:
                    if (seleccion == 0) {  // iniciar simulación
                        reset(cabinasConfig);
                        simTime = 0.0;
                        speedMult = 1.0;
                        spawnP = 0.6f;
                        estadoActual = SIMULACION;
                    }
                    else if (seleccion == 1) estadoActual = CONFIG;
                    else if (seleccion == 2) estadoActual = ESTADISTICAS;
                    else if (seleccion == 3) running = false;
                    break;
                case ALLEGRO_KEY_ESCAPE:
                    running = false; break;
                }
            }
            draw_menu(font, seleccion);
        }

        // === CONFIGURACION ===
        else if (estadoActual == CONFIG) {
            if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
                if (ev.keyboard.keycode == ALLEGRO_KEY_UP)
                    cabinasConfig = std::min(10, cabinasConfig + 1);
                if (ev.keyboard.keycode == ALLEGRO_KEY_DOWN)
                    cabinasConfig = std::max(1, cabinasConfig - 1);
                if (ev.keyboard.keycode == ALLEGRO_KEY_M)
                    estadoActual = MENU;
            }
            draw_config(font, cabinasConfig);
        }

        // === SIMULACION ===
        else if (estadoActual == SIMULACION) {
            if (ev.type == ALLEGRO_EVENT_TIMER) {
                double dt = (1.0 / 60.0) * speedMult;
                simTime += dt;
                probabilistic_spawn(simTime, spawnP);
                update((float)dt, simTime);
                redraw = true;

                // Guardar CSV al terminar
                if (simTime >= SIM_LIMIT) {
                    logs = build_logs_snapshot();

                    // --- Escritura manual sin filesystem ---
                    std::ofstream out(CSV_PATH);
                    if (out.is_open()) {
                        out << "id,tCreacion,tLlegadaCola,tInicioServicio,tSalidaCabina\n";
                        for (size_t i = 0; i < logs.size(); i++) {
                            out << logs[i].id << ","
                                << logs[i].tCreacion << ","
                                << logs[i].tLlegadaCola << ","
                                << logs[i].tInicioServicio << ","
                                << logs[i].tSalidaCabina << "\n";
                        }
                        out.close();
                    }

                    st = compute_stats_from_csv(CSV_PATH, simTime);

                    al_clear_to_color(al_map_rgb(0, 0, 0));
                    al_draw_textf(font, al_map_rgb(0, 255, 0), SCREEN_W / 2, SCREEN_H / 2 - 10, ALLEGRO_ALIGN_CENTER,
                        "Simulacion finalizada y guardada.");
                    al_draw_textf(font, al_map_rgb(200, 200, 200), SCREEN_W / 2, SCREEN_H / 2 + 20, ALLEGRO_ALIGN_CENTER,
                        "Archivo: toll_simulation_log.csv");
                    al_flip_display();
                    al_rest(2.0);

                    estadoActual = ESTADISTICAS;
                }
            }

            if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
                switch (ev.keyboard.keycode) {
                case ALLEGRO_KEY_ESCAPE:
                case ALLEGRO_KEY_M:
                    estadoActual = MENU; break;
                case ALLEGRO_KEY_1: speedMult = 1.0; break;
                case ALLEGRO_KEY_2: speedMult = 2.0; break;
                case ALLEGRO_KEY_3: speedMult = 5.0; break;
                case ALLEGRO_KEY_EQUALS: case ALLEGRO_KEY_PAD_PLUS:
                    spawnP = std::min(0.95f, spawnP + 0.05f); break;
                case ALLEGRO_KEY_MINUS: case ALLEGRO_KEY_PAD_MINUS:
                    spawnP = std::max(0.05f, spawnP - 0.05f); break;
                }
            }

            if (redraw && al_is_event_queue_empty(queue)) {
                redraw = false;
                draw(font, simTime, speedMult, spawnP);
            }
        }

        // === ESTADISTICAS ===
        else if (estadoActual == ESTADISTICAS) {
            std::ifstream test(CSV_PATH);
            if (!test.good()) {
                al_clear_to_color(al_map_rgb(0, 0, 0));
                al_draw_textf(font, al_map_rgb(255, 100, 100), SCREEN_W / 2, SCREEN_H / 2 - 20, ALLEGRO_ALIGN_CENTER,
                    "No hay datos registrados aun.");
                al_draw_textf(font, al_map_rgb(200, 200, 200), SCREEN_W / 2, SCREEN_H / 2 + 20, ALLEGRO_ALIGN_CENTER,
                    "Ejecuta una simulacion primero.");
                al_draw_textf(font, al_map_rgb(255, 255, 0), SCREEN_W / 2, SCREEN_H - 40, ALLEGRO_ALIGN_CENTER,
                    "[M] Volver al menu");
                al_flip_display();

                if (ev.type == ALLEGRO_EVENT_KEY_DOWN && ev.keyboard.keycode == ALLEGRO_KEY_M)
                    estadoActual = MENU;
            }
            else {
                test.close();
                st = compute_stats_from_csv(CSV_PATH, simTime);
                draw_stats_screen(font, st);
                if (ev.type == ALLEGRO_EVENT_KEY_DOWN && ev.keyboard.keycode == ALLEGRO_KEY_M)
                    estadoActual = MENU;
            }
        }
    }

    // --- Limpieza ---
    al_destroy_font(font);
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);
    al_destroy_display(display);
    return 0;
}