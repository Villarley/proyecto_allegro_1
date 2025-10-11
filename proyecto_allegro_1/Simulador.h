#pragma once
#include <vector>
#include "Carril.h"
#include "CabinaPeaje.h"
#include "Carro.h"
class Simulador {
private:
    float tiempoSimulado;       // Tiempo total en segundos
    float tiempoEntreVehiculos; // Intervalo promedio entre vehículos
    float acumuladorTiempo;     // Para controlar generación probabilística

    int contadorVehiculos;      // Para asignar IDs únicos

    std::vector<Carril> carriles;
    std::vector<CabinaPeaje> cabinas;

public:
    Simulador(int numCarriles, int numCabinas, float longitudCarril, float distanciaSeguridad, float intervaloGeneracion);

    void tick(float dt);  // Actualiza todo el sistema
    float getTiempoSimulado() const;

    // Acceso a estadísticas
    int totalVehiculosProcesados() const;
    float utilizacionPromedioCabinas() const;
    int totalVehiculosEnSistema() const;
};
