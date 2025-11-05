#pragma once
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <list>
#include <vector>
#include <deque>
#include <random>
#include <ctime>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <cmath>

// Se agregan los includes de las clases separadas
#include "Vehiculo.h"
#include "CabinaPeaje.h"

// ---------- Util ---------- //
template <typename T>
T clamp_value(T v, T lo, T hi) { return (v < lo) ? lo : (v > hi ? hi : v); }

namespace Escenario {

    // ---------- Config ---------- //
    const int   SCREEN_W = 900, SCREEN_H = 600;
    const float ROAD_L = 150.0f, ROAD_R = 750.0f, TOLL_Y = 520.0f;
    const int   LANES = 3, BOOTHS = 5, MAX_VEHICLES = 20;

    const float MIN_SERVICE = 2.0f, MAX_SERVICE = 5.0f;
    const float DECISION_Y = 380.0f;       // línea roja
    const float QUEUE_SPACING = 65.0f;
    const float SAFE_DIST = 30.0f;
    const float STOP_OFFSET = 20.0f;
    const float EXIT_Y = SCREEN_H + 50.0f;
    const float CLEAR_TIME = 2.0f;         // tiempo de limpieza tras servicio

    // ---------- Métricas ---------- //
    struct Registro {
        int id;
        double tCreacion;
        double tLlegadaCola;
        double tInicioServicio;
        double tSalidaCabina;
    };

    struct Stats {
        int totalProcesados = 0;
        double promEspera = 0.0;
        double promTotalSistema = 0.0;
        double flujoPorMin = 0.0;
        std::vector<double> utilizacionPorCabina;
    };

    // ---------- Estado global ---------- //
    static std::list<Vehiculo> vehiculos;
    static std::vector<Cabina> cabinas;
    static std::vector<Registro> bitacora;

    static std::mt19937 rng((unsigned)time(NULL));
    static std::uniform_real_distribution<float> prob(0.0f, 1.0f);
    static std::uniform_real_distribution<float> Uservice(MIN_SERVICE, MAX_SERVICE);

    inline float road_width() { return ROAD_R - ROAD_L; }
    inline float lane_center_x(int i) { return ROAD_L + (road_width() / LANES) * (i + 0.5f); }
    inline float booth_center_x(int i) { return ROAD_L + (road_width() / BOOTHS) * (i + 0.5f); }

    inline void reset(int cabinasPersonalizadas = BOOTHS) {
        vehiculos.clear();
        cabinas.clear();
        for (int i = 0; i < cabinasPersonalizadas; ++i)
            cabinas.emplace_back(i, booth_center_x(i), TOLL_Y);
    }

    inline void init_once() {
        if (!cabinas.empty()) return;
        reset();
    }

    // ---------- Dibujo ---------- //
    inline void draw_scene() {
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

    inline void draw(ALLEGRO_FONT* font, double simTime, double speedMult, float spawnP) {
        init_once();
        draw_scene();

        for (auto& v : vehiculos) {
            if (v.estado == Salio) continue;
            ALLEGRO_COLOR col =
                (v.estado == EnCarretera) ? al_map_rgb(0, 255, 0) :
                (v.estado == EnCola) ? al_map_rgb(255, 215, 0) :
                (v.estado == EnServicio) ? al_map_rgb(0, 150, 255) :
                (v.estado == Saliendo) ? al_map_rgb(150, 150, 150) :
                al_map_rgb(255, 255, 255);
            al_draw_filled_rectangle(v.x - 15, v.y - 10, v.x + 15, v.y + 10, col);
            al_draw_textf(font, al_map_rgb(255, 255, 255), v.x, v.y - 14, ALLEGRO_ALIGN_CENTER, "%d", v.id);
        }

        for (auto& c : cabinas) {
            bool libre = (c.enServicio == nullptr);
            al_draw_textf(font, libre ? al_map_rgb(0, 255, 0) : al_map_rgb(255, 0, 0),
                c.x, TOLL_Y - 105, ALLEGRO_ALIGN_CENTER, libre ? "LIBRE" : "OCUPADA");
            al_draw_textf(font, al_map_rgb(255, 255, 255), c.x, TOLL_Y - 90, ALLEGRO_ALIGN_CENTER,
                "Cola:%d  Proc:%d", (int)c.fila.size(), c.procesados);
        }

        al_draw_textf(font, al_map_rgb(255, 255, 255), 20, 10, 0,
            "Tiempo: %.1fs | Activos: %d | Vel x%.1f | p=%.2f",
            simTime, (int)vehiculos.size(), speedMult, spawnP);
        al_draw_textf(font, al_map_rgb(200, 200, 200), 20, 30, 0,
            "[1]=1x [2]=2x [3]=5x  +/- cambia p  [ESC]=fin");
        al_flip_display();
    }

    // ---------- Update ---------- //
    inline void update(float dt, double simTime) {
        init_once();
        const float FIXED_SPACING = 25.0f;

        // (1) Movimiento en carretera
        for (auto& v : vehiculos) {
            if (v.estado != EnCarretera) continue;

            auto same_lane = [&](const Vehiculo& a, const Vehiculo& b) {
                return std::abs(a.x - b.x) < (road_width() / LANES) * 0.1f;
                };

            float menorDist = 1e9f;
            for (auto& o : vehiculos) {
                if (&o == &v) continue;
                if (o.estado == EnCarretera && same_lane(o, v) && o.y > v.y)
                    menorDist = std::min(menorDist, o.y - v.y);
            }
            if (menorDist > SAFE_DIST)
                v.y += v.velocidad * dt;

            if (v.y >= DECISION_Y) {
                int mejor = 0;
                size_t mejorLen = cabinas[0].fila.size();
                for (int i = 1; i < BOOTHS; ++i) {
                    size_t len = cabinas[i].fila.size();
                    if (len < mejorLen) { mejor = i; mejorLen = len; }
                }
                if (v.cabinaElegida != -1 && cabinas[v.cabinaElegida].fila.size() <= mejorLen)
                    mejor = v.cabinaElegida;

                if ((int)cabinas[mejor].fila.size() < 5) {
                    v.cabinaElegida = mejor;
                    v.x = cabinas[mejor].x;
                    v.estado = EnCola;
                    v.tLlegadaCola = simTime;
                    float baseY = DECISION_Y - 10.0f;
                    if (cabinas[mejor].fila.empty())
                        v.y = baseY;
                    else
                        v.y = std::min(baseY, cabinas[mejor].fila.back()->y - FIXED_SPACING);
                    cabinas[mejor].fila.push_back(&v);
                }
                else v.y = DECISION_Y - 10;
            }
        }

        // (2) Colas y servicio
        for (auto& cab : cabinas) {
            const float baseStop = cab.stopY();
            const float holdY = DECISION_Y - 10.0f;

            for (size_t i = 0; i < cab.fila.size(); ++i) {
                Vehiculo* v = cab.fila[i];
                if (!v) continue;
                v->x = cab.x;

                bool puedeCruzar = (i == 0) && (cab.enServicio == nullptr) && (simTime >= cab.tBloqueadaHasta);
                float targetY = puedeCruzar ? baseStop : (holdY - (float)i * FIXED_SPACING);

                if (i > 0) {
                    Vehiculo* adelante = cab.fila[i - 1];
                    float maxY = adelante->y - FIXED_SPACING;
                    if (targetY > maxY) targetY = maxY;
                }

                float dy = targetY - v->y;
                if (std::fabs(dy) > 0.5f) {
                    float step = v->velocidad * 0.3f * dt;
                    if (!puedeCruzar && v->y < DECISION_Y && (v->y + step) > DECISION_Y)
                        step = DECISION_Y - v->y;
                    v->y += clamp_value(dy, -step, step);
                }
                else v->y = targetY;
            }

            if (!cab.enServicio && !cab.fila.empty() && simTime >= cab.tBloqueadaHasta) {
                Vehiculo* f0 = cab.fila.front();
                if (std::fabs(f0->y - baseStop) < 1.0f) {
                    f0->y = baseStop;
                    cab.enServicio = f0;
                    f0->estado = EnServicio;
                    f0->tiempoServicio = Uservice(rng);
                    f0->restanteServicio = f0->tiempoServicio;
                    f0->tInicioServicio = simTime;
                    cab.tOcupadaHasta = simTime;
                    cab.fila.pop_front();
                }
            }

            if (cab.enServicio) {
                Vehiculo* s = cab.enServicio;
                s->x = cab.x;
                s->y = baseStop;
                s->restanteServicio -= dt;
                if (s->restanteServicio <= 0.0f) {
                    s->estado = Saliendo;
                    s->tSalidaCabina = simTime;
                    cab.procesados++;
                    cab.tOcupadaAcum += (simTime - cab.tOcupadaHasta);
                    cab.enServicio = nullptr;
                    cab.tBloqueadaHasta = simTime + CLEAR_TIME;
                    bitacora.push_back({ s->id, s->tCreacion, s->tLlegadaCola, s->tInicioServicio, s->tSalidaCabina });
                }
            }
        }

        // (3) Salida
        for (auto& v : vehiculos) {
            if (v.estado != Saliendo) continue;
            v.y += v.vSalida * dt;
            if (v.y > EXIT_Y) v.estado = Salio;
        }

        vehiculos.remove_if([](const Vehiculo& v) {
            return v.estado == Salio;
            });
    }

    // ---------- Spawn ---------- //
    inline void probabilistic_spawn(double simTime, float spawnProb) {
        init_once();
        static int nextId = 1;
        if ((int)vehiculos.size() >= MAX_VEHICLES) return;
        if (prob(rng) >= spawnProb) return;

        const float SPAWN_Y = -20.0f;
        const float MIN_GAP = SAFE_DIST * 2.0f;
        std::vector<int> libres;
        for (int lane = 0; lane < LANES; ++lane) {
            float x = lane_center_x(lane);
            bool ok = true;
            for (auto& v : vehiculos) {
                if (v.estado == EnCarretera && std::fabs(v.x - x) < 1.0f) {
                    if ((v.y - SPAWN_Y) < MIN_GAP) { ok = false; break; }
                }
            }
            if (ok) libres.push_back(lane);
        }
        if (libres.empty()) return;

        int lane = libres[rng() % libres.size()];
        float x = lane_center_x(lane);
        float vel = 100.0f + (rng() % 45);
        vehiculos.emplace_back(nextId++, x, SPAWN_Y, vel, lane, simTime);
    }

    // ---------- CSV + Stats ---------- //
    inline void export_csv(const char* path, const std::vector<Registro>& logs) {
        std::ofstream out(path);
        out << "id,tCreacion,tLlegadaCola,tInicioServicio,tSalidaCabina\n";
        for (auto& r : logs)
            out << r.id << "," << r.tCreacion << "," << r.tLlegadaCola << ","
            << r.tInicioServicio << "," << r.tSalidaCabina << "\n";
    }

    inline std::vector<Registro> build_logs_snapshot() {
        return bitacora;
    }

    inline Stats compute_stats_from_csv(const char* path, double simDurationSecs) {
        Stats st;
        st.utilizacionPorCabina.resize(BOOTHS, 0.0);
        double totalTime = simDurationSecs > 0 ? simDurationSecs : 1.0;
        for (int i = 0; i < BOOTHS; ++i)
            st.utilizacionPorCabina[i] = cabinas[i].tOcupadaAcum / totalTime;

        std::ifstream in(path);
        std::string line;
        if (!in.good()) return st;
        std::getline(in, line);
        double sumEspera = 0.0, sumTotal = 0.0; int n = 0;
        while (std::getline(in, line)) {
            if (line.empty()) continue;
            std::stringstream ss(line);
            std::string s; int id; char comma;
            double tC, tLC, tIS, tSC;
            ss >> id >> comma >> tC >> comma >> tLC >> comma >> tIS >> comma >> tSC;
            if (tSC >= 0 && tIS >= 0 && tLC >= 0 && tC >= 0) {
                double espera = tIS - tLC;
                double total = tSC - tC;
                sumEspera += espera;
                sumTotal += total;
                n++;
            }
        }
        st.totalProcesados = n;
        if (n > 0) {
            st.promEspera = sumEspera / n;
            st.promTotalSistema = sumTotal / n;
            st.flujoPorMin = (double)n / (totalTime / 60.0);
        }
        return st;
    }

    inline void draw_stats_screen(ALLEGRO_FONT* font, const Stats& st) {
        al_clear_to_color(al_map_rgb(20, 20, 20));
        al_draw_textf(font, al_map_rgb(255, 255, 255), SCREEN_W / 2, 40, ALLEGRO_ALIGN_CENTER, "Estadisticas finales");
        al_draw_textf(font, al_map_rgb(200, 200, 200), 60, 100, 0, "Vehiculos procesados: %d", st.totalProcesados);
        al_draw_textf(font, al_map_rgb(200, 200, 200), 60, 130, 0, "Promedio espera (cola): %.2fs", st.promEspera);
        al_draw_textf(font, al_map_rgb(200, 200, 200), 60, 160, 0, "Promedio total en sistema: %.2fs", st.promTotalSistema);
        al_draw_textf(font, al_map_rgb(200, 200, 200), 60, 190, 0, "Flujo promedio: %.2f veh/min", st.flujoPorMin);
        for (int i = 0; i < BOOTHS; ++i) {
            al_draw_textf(font, al_map_rgb(180, 180, 255), 60, 230 + i * 24, 0,
                "Cabina %d utilizacion: %d%%", i + 1, (int)std::round(100.0 * st.utilizacionPorCabina[i]));
        }
        al_draw_textf(font, al_map_rgb(255, 255, 0), SCREEN_W / 2, SCREEN_H - 40, ALLEGRO_ALIGN_CENTER, "[Esc] para salir");
        al_flip_display();
    }

} // namespace Escenario
