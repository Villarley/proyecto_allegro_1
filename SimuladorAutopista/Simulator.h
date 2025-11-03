#pragma once
#include <vector>
#include <random>
#include <algorithm>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include "Vehicle.h"

class Simulator {
public:
    int WIDTH = 900;
    int HEIGHT = 600;
    int lanes = 3;
    int booths = 5;

    float tollHeight = 110.0f;
    float laneGap = 0.0f;

    float spawnPPerSec = 0.7f;
    float spawnAccum = 0.0f;
    std::mt19937 rng{ std::random_device{}() };
    std::uniform_real_distribution<float> U{ 0.0f, 1.0f };

    std::vector<Vehicle> vehicles;

    void init() {
        laneGap = (HEIGHT - tollHeight) / static_cast<float>(lanes);
    }

    float boothCenterX(int i) const {
        float w = static_cast<float>(WIDTH) / booths;
        return (i + 0.5f) * w;
    }

    float laneCenterY(int laneIndex, float marginTop = 40.0f) const {
        return marginTop + laneIndex * laneGap + laneGap * 0.5f;
    }

    float laneCenterX(int laneIndex) const {
        return WIDTH * 0.5f;
    }

    void drawScene() const {
        al_clear_to_color(al_map_rgb(200, 200, 200));

        float y0 = HEIGHT - tollHeight;
        al_draw_filled_rectangle(0, y0, WIDTH, HEIGHT, al_map_rgb(160, 160, 160));

        for (int i = 0; i < booths; ++i) {
            float w = WIDTH / static_cast<float>(booths);
            float x1 = i * w + w * 0.10f;
            float x2 = (i + 1) * w - w * 0.10f;
            al_draw_filled_rectangle(x1, y0 + 10, x2, HEIGHT - 10, al_map_rgb(100, 100, 255));
        }

        for (int i = 1; i < lanes; ++i) {
            float y = i * laneGap;
            al_draw_line(0, y, WIDTH, y, al_map_rgb(100, 100, 100), 3.0f);
        }
    }

    void maybeSpawn(float dt) {
        spawnAccum += dt;
        const float checkEvery = 1.0f;

        if (spawnAccum >= checkEvery) {
            spawnAccum -= checkEvery;
            if (U(rng) < spawnPPerSec) {
                int L = std::uniform_int_distribution<int>(0, lanes - 1)(rng);
                Vehicle v;
                v.lane = L;
                v.x = laneCenterX(L);
                v.y = -30.0f;
                v.speed = std::uniform_real_distribution<float>(90.f, 150.f)(rng);
                vehicles.push_back(v);
            }
        }
    }

    void update(float dt) {
        for (auto& v : vehicles) {
            float peajeY = HEIGHT - tollHeight;
            if (v.y + v.h * 0.5f >= peajeY) {
                v.state = VehicleState::Slowing;
                v.y += v.speed * 0.35f * dt;
            }
            else {
                v.state = VehicleState::Moving;
                v.update(dt);
            }
        }

        vehicles.erase(
            std::remove_if(vehicles.begin(), vehicles.end(),
                [&](const Vehicle& v) { return v.y - v.h > HEIGHT; }),
            vehicles.end()
        );
    }

    void drawVehicles() const {
        for (const auto& v : vehicles) v.draw();
    }
};
#pragma once
