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

    // ======== CONFIGURACIÓN ========
    const int SCREEN_W = 900, SCREEN_H = 600;
    const float ROAD_L = 150.0f, ROAD_R = 750.0f, TOLL_Y = 520.0f;
    const int LANES = 3, BOOTHS = 5, MAX_VEHICLES = 20;
    const float MIN_SERVICE = 2.0f, MAX_SERVICE = 5.0f;
    const float DECISION_Y = 380.0f;
    const float QUEUE_SPACING = 65.0f;
    const float SAFE_DIST = 30.0f;
    const float STOP_OFFSET = 20.0f;
    const float EXIT_Y = SCREEN_H + 50.0f;
    const float MAX_IDLE_TIME = 5.0f;
    const float TRY_MOVE_TIME = 3.0f;

    enum Estado { EnCarretera, EnCola, EnServicio, Saliendo, Salio };

    struct Vehiculo {
        int id;
        float x, y, velocidad;
        int carrilActual;
        Estado estado;
        float tiempoServicio = 0, restanteServicio = 0;
        double tCreacion = 0, tLlegadaCola = -1, tInicioServicio = -1, tSalida = -1;
        float idleTimer = 0.0f;
        float lastY = 0.0f;
        ALLEGRO_COLOR color;

        Vehiculo(int _id, float _x, float _y, float _vel, int _carril, double simTime)
            : id(_id), x(_x), y(_y), velocidad(_vel),
              carrilActual(_carril), estado(EnCarretera),
              tCreacion(simTime), lastY(_y),
              color(al_map_rgb(rand() % 255, rand() % 255, rand() % 255)) {}
    };

    struct PilaCarril {
        int id;
        float x;
        std::deque<Vehiculo*> vehiculos;

        PilaCarril(int _id, float _x) : id(_id), x(_x) {}

        void agregar(Vehiculo* v) { v->x = x; vehiculos.push_back(v); }

        void remover(Vehiculo* v) {
            vehiculos.erase(std::remove(vehiculos.begin(), vehiculos.end(), v), vehiculos.end());
        }

        void ordenar() {
            std::sort(vehiculos.begin(), vehiculos.end(),
                      [](auto* a, auto* b) { return a->y < b->y; });
        }
    };

    struct Cabina {
        int id;
        float x, y;
        std::deque<Vehiculo*> fila;
        Vehiculo* enServicio = nullptr;
        int procesados = 0;
        double cooldownHasta = 0.0;

        Cabina(int _id, float _x, float _y) : id(_id), x(_x), y(_y) {}

        float stopY() const { return y - STOP_OFFSET; }
        bool puedeAtender(double simTime) const {
            return enServicio == nullptr && simTime >= cooldownHasta;
        }

        void agregarAFila(Vehiculo* v) {
            v->estado = EnCola;
            v->x = x;
            fila.push_back(v);
        }
    };

    // ======== VARIABLES GLOBALES ========
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

    // ======== DIBUJO ========
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
        al_draw_textf(font, al_map_rgb(255, 255, 255), 20, 10, 0,
            "Tiempo: %.1f | Vehiculos: %d | x%.1f | P=%.2f",
            simTime, (int)vehiculos.size(), speedMult, spawnP);

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
        al_flip_display();
    }

    // ======== UPDATE ========
    void update(float dt, double simTime) {
        init();

        // actualizar tiempos de movimiento
        for (auto& v : vehiculos) {
            if (fabs(v.y - v.lastY) < 0.5f) v.idleTimer += dt;
            else { v.lastY = v.y; v.idleTimer = 0.0f; }
        }

        // 1️⃣ MOVER EN CARRETERA
        for (auto& carril : carriles) {
            carril.ordenar();
            for (auto* v : carril.vehiculos) {
                if (!v || v->estado != EnCarretera) continue;

                // buscar vehículo adelante
                Vehiculo* front = nullptr;
                float dist = std::numeric_limits<float>::infinity();
                for (auto* o : carril.vehiculos) {
                    if (o != v && o->y > v->y) {
                        float d = o->y - v->y;
                        if (d < dist) { dist = d; front = o; }
                    }
                }

                if (front && dist < SAFE_DIST) continue;

                v->y += v->velocidad * dt;

                // si se queda mucho tiempo sin moverse -> intenta moverse de carril
                if (v->idleTimer > TRY_MOVE_TIME) {
                    int newLane = v->carrilActual;
                    if (prob(rng) > 0.5f && v->carrilActual < LANES - 1) newLane++;
                    else if (v->carrilActual > 0) newLane--;
                    if (newLane != v->carrilActual) {
                        carril.remover(v);
                        carriles[newLane].agregar(v);
                        v->carrilActual = newLane;
                        v->x = carriles[newLane].x;
                        v->idleTimer = 0.0f;
                    }
                }

                if (v->idleTimer > MAX_IDLE_TIME) {
                    v->estado = Salio; // desaparece sin causar bug
                }

                // pasar a cabina
                if (v->y >= DECISION_Y) {
                    int mejor = 0;
                    size_t menor = cabinas[0].fila.size();
                    for (size_t j = 1; j < cabinas.size(); ++j)
                        if (cabinas[j].fila.size() < menor) { menor = cabinas[j].fila.size(); mejor = j; }

                    carril.remover(v);
                    cabinas[mejor].agregarAFila(v);
                    v->tLlegadaCola = simTime;
                }
            }
        }

        // 2️⃣ CABINAS
        for (auto& cab : cabinas) {
            float stopY = cab.stopY();

            if (cab.enServicio) {
                Vehiculo* v = cab.enServicio;
                v->restanteServicio -= dt;
                if (v->restanteServicio <= 0.0f) {
                    v->estado = Saliendo;
                    v->tSalida = simTime;
                    cab.procesados++;
                    cab.enServicio = nullptr;
                    cab.cooldownHasta = simTime + 1.5;
                }
            }

            for (size_t i = 0; i < cab.fila.size(); ++i) {
                Vehiculo* v = cab.fila[i];
                float targetY = stopY - (float)i * QUEUE_SPACING;
                if (v->y < targetY) v->y += v->velocidad * 0.5f * dt;

                if (i > 0) {
                    Vehiculo* prev = cab.fila[i - 1];
                    if (prev->y - v->y < SAFE_DIST)
                        v->y = prev->y - SAFE_DIST;
                }

                if (i == 0 && v->y >= stopY - 5.0f && cab.puedeAtender(simTime)) {
                    v->estado = EnServicio;
                    v->tiempoServicio = Uservice(rng);
                    v->restanteServicio = v->tiempoServicio;
                    v->tInicioServicio = simTime;
                    cab.enServicio = v;
                    cab.fila.pop_front();
                }
            }
        }

        // 3️⃣ SALIDA
        for (auto& v : vehiculos) {
            if (v.estado != Saliendo) continue;
            v.y += v.velocidad * 1.2f * dt;
            if (v.y > EXIT_Y) v.estado = Salio;
        }

        vehiculos.erase(std::remove_if(vehiculos.begin(), vehiculos.end(),
            [](const Vehiculo& v) { return v.estado == Salio; }), vehiculos.end());
    }

    // ======== SPAWN ========
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
            for (auto* v : carriles[i].vehiculos)
                if (v->y > SPAWN_Y && (v->y - SPAWN_Y) < MIN_GAP)
                    libre = false;
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
        for (auto& v : vehiculos)
            if (v.tSalida >= 0)
                out << v.id << "," << v.tCreacion << "," << v.tLlegadaCola << ","
                    << v.tInicioServicio << "," << v.tSalida << "\n";
        out.close();
    }

} // namespace Escenario
