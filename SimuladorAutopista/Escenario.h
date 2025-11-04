#pragma once
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <deque>
#include <vector>
#include <random>
#include <ctime>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <cmath>
#include <limits>

template <typename T>
T clamp_value(T v, T lo, T hi) {
    return (v < lo) ? lo : (v > hi ? hi : v);
}

namespace Escenario {

    const int SCREEN_W = 900, SCREEN_H = 600;
    const float ROAD_L = 150.0f, ROAD_R = 750.0f, TOLL_Y = 520.0f;
    const int LANES = 3, BOOTHS = 5, MAX_VEHICLES = 20;
    const float MIN_SERVICE = 2.0f, MAX_SERVICE = 5.0f;
    const float DECISION_Y = 380.0f;
    const float QUEUE_SPACING = 65.0f;
    const float SAFE_DIST = 30.0f;
    const float MIN_DISTANCE = 15.0f; // ⭐ Distancia mínima en fila
    const float STOP_OFFSET = 20.0f;
    const float EXIT_Y = SCREEN_H + 50.0f;

    enum Estado { EnCarretera, EnCola, EnServicio, Saliendo, Salio };

    struct Vehiculo {
        int id;
        float x, y, velocidad;
        int carrilActual;
        Estado estado;
        int indicePila = -1; // ⭐ Posición en la pila
        float tiempoServicio = 0, restanteServicio = 0;
        double tCreacion = 0, tLlegadaCola = -1, tInicioServicio = -1, tSalida = -1;
        ALLEGRO_COLOR color;

        Vehiculo(int _id, float _x, float _y, float _vel, int _carril, double simTime)
            : id(_id), x(_x), y(_y), velocidad(_vel),
            carrilActual(_carril), estado(EnCarretera),
            tCreacion(simTime),
            color(al_map_rgb(rand() % 255, rand() % 255, rand() % 255)) {
        }
    };

    struct PilaCarril {
        int id;
        float x;
        std::deque<Vehiculo*> vehiculos;

        PilaCarril(int _id, float _x) : id(_id), x(_x) {}

        void agregar(Vehiculo* v) {
            v->x = x;
            v->carrilActual = id;
            v->indicePila = (int)vehiculos.size(); // ⭐ Al final
            vehiculos.push_back(v);
        }

        void remover(Vehiculo* v) {
            vehiculos.erase(std::remove(vehiculos.begin(), vehiculos.end(), v), vehiculos.end());
            // ⭐ Reindexar después de remover
            for (size_t i = 0; i < vehiculos.size(); ++i) {
                vehiculos[i]->indicePila = (int)i;
            }
        }

        Vehiculo* obtenerPorIndice(int idx) {
            if (idx >= 0 && idx < (int)vehiculos.size()) {
                return vehiculos[idx];
            }
            return nullptr;
        }
    };

    struct Cabina {
        int id;
        float x, y;
        std::deque<Vehiculo*> fila; // ⭐ PILA ORDENADA
        Vehiculo* enServicio = nullptr;
        int procesados = 0;
        double cooldownHasta = 0.0;

        Cabina(int _id, float _x, float _y) : id(_id), x(_x), y(_y) {}

        float stopY() const { return y - STOP_OFFSET; }

        bool puedeAtender(double simTime) const {
            return enServicio == nullptr && simTime >= cooldownHasta;
        }

        void agregarAFila(Vehiculo* v, double simTime) {
            v->estado = EnCola;
            v->carrilActual = id;
            v->x = x;
            v->tLlegadaCola = simTime;
            v->indicePila = (int)fila.size(); // ⭐ AL FINAL siempre
            fila.push_back(v);
        }

        void reindexar() {
            for (size_t i = 0; i < fila.size(); ++i) {
                fila[i]->indicePila = (int)i;
            }
        }

        Vehiculo* obtenerPorIndice(int idx) {
            if (idx >= 0 && idx < (int)fila.size()) {
                return fila[idx];
            }
            return nullptr;
        }
    };

    static std::deque<Vehiculo> vehiculos;
    static std::vector<PilaCarril> carriles;
    static std::vector<Cabina> cabinas;

    static std::mt19937 rng((unsigned)time(NULL));
    static std::uniform_real_distribution<float> prob(0.0f, 1.0f);
    static std::uniform_real_distribution<float> Uservice(MIN_SERVICE, MAX_SERVICE);

    float road_width() { return ROAD_R - ROAD_L; }
    float lane_center_x(int i) { return ROAD_L + (road_width() / LANES) * (i + 0.5f); }
    float booth_center_x(int i) { return ROAD_L + (road_width() / BOOTHS) * (i + 0.5f); }

    void init() {
        if (!carriles.empty()) return;
        carriles.clear();
        for (int i = 0; i < LANES; ++i) carriles.emplace_back(i, lane_center_x(i));
        cabinas.clear();
        for (int i = 0; i < BOOTHS; ++i) cabinas.emplace_back(i, booth_center_x(i), TOLL_Y);
    }

    void draw_scene() {
        al_clear_to_color(al_map_rgb(34, 139, 34));
        al_draw_filled_rectangle(ROAD_L, 0, ROAD_R, SCREEN_H, al_map_rgb(50, 50, 50));
        for (int i = 1; i < LANES; ++i) {
            float x = ROAD_L + road_width() * i / LANES;
            al_draw_line(x, 0, x, SCREEN_H, al_map_rgb(255, 255, 0), 2);
        }
        al_draw_line(ROAD_L, DECISION_Y, ROAD_R, DECISION_Y, al_map_rgb(255, 0, 0), 3);
        al_draw_filled_rectangle(ROAD_L, TOLL_Y - 40, ROAD_R, TOLL_Y + 40, al_map_rgb(105, 105, 105));
        for (auto& cab : cabinas)
            al_draw_filled_rectangle(cab.x - 35, TOLL_Y - 70, cab.x + 35, TOLL_Y - 20, al_map_rgb(180, 180, 180));
    }

    void draw(ALLEGRO_FONT* font, double simTime, double speedMult, double spawnP) {
        init();
        draw_scene();

        for (auto& v : vehiculos) {
            if (v.estado == Salio) continue;
            ALLEGRO_COLOR col =
                (v.estado == EnCarretera) ? al_map_rgb(0, 255, 0) :
                (v.estado == EnCola) ? al_map_rgb(255, 215, 0) :
                (v.estado == EnServicio) ? al_map_rgb(255, 0, 0) :
                (v.estado == Saliendo) ? al_map_rgb(150, 150, 150) : v.color;
            al_draw_filled_rectangle(v.x - 15, v.y - 10, v.x + 15, v.y + 10, col);
            al_draw_textf(font, al_map_rgb(255, 255, 255), v.x, v.y - 5, ALLEGRO_ALIGN_CENTER, "%d", v.id);
        }

        for (auto& c : cabinas) {
            bool libre = c.puedeAtender(simTime);
            al_draw_textf(font, libre ? al_map_rgb(0, 255, 0) : al_map_rgb(255, 0, 0),
                c.x, TOLL_Y - 105, ALLEGRO_ALIGN_CENTER, libre ? "LIBRE" : "OCUPADO");
            al_draw_textf(font, al_map_rgb(255, 255, 255), c.x, TOLL_Y - 90, ALLEGRO_ALIGN_CENTER,
                "F:%d P:%d", (int)c.fila.size(), c.procesados);
        }

        al_draw_textf(font, al_map_rgb(255, 255, 255), 20, 10, 0,
            "Tiempo: %.1f | Vehiculos: %d | x%.1f | P=%.2f",
            simTime, (int)vehiculos.size(), speedMult, spawnP);
        al_flip_display();
    }

    void update(float dt, double simTime) {
        init();

        // ===== FASE 1: MOVIMIENTO EN CARRETERA =====
        for (auto& carril : carriles) {
            for (size_t i = 0; i < carril.vehiculos.size(); ++i) {
                Vehiculo* v = carril.vehiculos[i];
                if (!v || v->estado != EnCarretera) continue;

                // ⭐ Buscar vehículo DELANTE en la pila (índice mayor)
                Vehiculo* adelante = nullptr;
                float minDist = std::numeric_limits<float>::infinity();

                for (size_t j = 0; j < carril.vehiculos.size(); ++j) {
                    Vehiculo* otro = carril.vehiculos[j];
                    // Solo considerar los que están delante (mayor Y)
                    if (otro != v && otro->y > v->y) {
                        float dist = otro->y - v->y;
                        if (dist < minDist) {
                            minDist = dist;
                            adelante = otro;
                        }
                    }
                }

                // ⭐ REGLA: No acercarse a menos de SAFE_DIST
                if (adelante && minDist < SAFE_DIST) {
                    continue; // NO avanzar
                }

                // Avanzar
                v->y += v->velocidad * dt;

                // PUNTO DE DECISIÓN
                if (v->y >= DECISION_Y) {
                    // Elegir cabina con menor fila
                    int mejorCabina = 0;
                    size_t menorFila = cabinas[0].fila.size();
                    for (size_t j = 1; j < cabinas.size(); ++j) {
                        if (cabinas[j].fila.size() < menorFila) {
                            mejorCabina = (int)j;
                            menorFila = cabinas[j].fila.size();
                        }
                    }

                    // ⭐ CAMBIO DE PILA: Sacar de carril, agregar AL FINAL de cabina
                    carril.remover(v);
                    cabinas[mejorCabina].agregarAFila(v, simTime);
                }
            }
        }

        // ===== FASE 2: GESTIÓN DE CABINAS =====
        for (auto& cab : cabinas) {
            float stopY = cab.stopY();

            // Procesar vehículo en servicio
            if (cab.enServicio) {
                Vehiculo* v = cab.enServicio;

                // Alineación forzada
                v->x = cab.x;
                v->y = stopY;
                v->estado = EnServicio;

                v->restanteServicio -= dt;

                if (v->restanteServicio <= 0.0f) {
                    v->estado = Saliendo;
                    v->tSalida = simTime;
                    cab.procesados++;
                    cab.enServicio = nullptr;
                    cab.cooldownHasta = simTime + 1.5;
                }
            }

            // ⭐ MOVER LA FILA CON ORDEN ESTRICTO
            for (int i = (int)cab.fila.size() - 1; i >= 0; --i) {
                Vehiculo* v = cab.fila[i];
                if (!v) continue;

                // ⭐ Alineación forzada en X
                v->x = cab.x;

                // ⭐ Calcular posición objetivo basada en ÍNDICE
                float targetY;
                if (i == 0) {
                    // El primero va a la posición de entrada
                    targetY = stopY;
                }
                else {
                    // Los demás van DETRÁS del anterior con QUEUE_SPACING
                    targetY = stopY - (float)i * QUEUE_SPACING;
                }

                // ⭐ REGLA CRÍTICA: No puede adelantar al de adelante
                if (i > 0) {
                    Vehiculo* adelante = cab.fila[i - 1];
                    float distanciaAlFrente = adelante->y - v->y;

                    // ⭐ Si está muy cerca del de adelante, NO avanzar
                    if (distanciaAlFrente < MIN_DISTANCE) {
                        continue; // NO moverse
                    }

                    // ⭐ Ajustar targetY para no sobrepasar
                    float maxY = adelante->y - MIN_DISTANCE;
                    if (targetY > maxY) {
                        targetY = maxY;
                    }
                }

                // ⭐ CASO ESPECIAL: El primero intenta entrar
                if (i == 0 && cab.puedeAtender(simTime)) {
                    if (v->y >= stopY - 5.0f) {
                        // ⭐ ENTRAR A SERVICIO
                        if (cab.enServicio == nullptr) {
                            v->estado = EnServicio;
                            v->tiempoServicio = Uservice(rng);
                            v->restanteServicio = v->tiempoServicio;
                            v->tInicioServicio = simTime;
                            v->y = stopY;
                            v->x = cab.x;

                            cab.enServicio = v;
                            cab.fila.pop_front();
                            cab.reindexar(); // ⭐ Reindexar después de quitar el primero
                            break; // Salir del loop de esta cabina
                        }
                    }
                }

                // ⭐ Movimiento suave hacia targetY
                float dy = targetY - v->y;
                if (fabs(dy) > 1.0f) {
                    float move = v->velocidad * 0.4f * dt;
                    v->y += clamp_value(dy, -move, move);
                }
                else {
                    v->y = targetY;
                }
            }
        }

        // ===== FASE 3: SALIDA =====
        for (auto& v : vehiculos) {
            if (v.estado != Saliendo) continue;
            v.y += v.velocidad * 1.2f * dt;
            if (v.y > EXIT_Y) v.estado = Salio;
        }

        // Limpieza
        vehiculos.erase(
            std::remove_if(vehiculos.begin(), vehiculos.end(),
                [](const Vehiculo& v) { return v.estado == Salio; }),
            vehiculos.end()
        );
    }

    void probabilistic_spawn(double simTime, float spawnProb) {
        init();
        static int nextId = 1;
        static double nextSpawn = 0.0;
        if ((int)vehiculos.size() >= MAX_VEHICLES) return;
        if (prob(rng) >= spawnProb) return;
        if (simTime < nextSpawn) return;

        const float SPAWN_Y = -20.0f;
        const float MIN_GAP = SAFE_DIST * 2.0f;
        std::vector<int> disponibles;

        for (int i = 0; i < LANES; ++i) {
            bool libre = true;
            for (auto* v : carriles[i].vehiculos) {
                if (v->y > SPAWN_Y && (v->y - SPAWN_Y) < MIN_GAP) {
                    libre = false;
                    break;
                }
            }
            if (libre) disponibles.push_back(i);
        }

        if (disponibles.empty()) return;

        int lane = disponibles[rand() % disponibles.size()];
        float vel = 100.0f + (rand() % 40);
        vehiculos.emplace_back(nextId++, carriles[lane].x, SPAWN_Y, vel, lane, simTime);
        carriles[lane].agregar(&vehiculos.back());
        nextSpawn = simTime + 0.3;
    }

    inline void export_csv(const char* path) {
        std::ofstream out(path);
        out << "ID,Creacion,LlegadaCola,InicioServicio,Salida\n";
        for (auto& v : vehiculos) {
            if (v.tSalida >= 0) {
                out << v.id << "," << v.tCreacion << "," << v.tLlegadaCola << ","
                    << v.tInicioServicio << "," << v.tSalida << "\n";
            }
        }
        out.close();
    }

} // namespace Escenario