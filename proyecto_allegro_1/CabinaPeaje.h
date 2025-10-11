#pragma once
#include <queue>
#include "Carro.h"

class CabinaPeaje {
private:
    int idCabina;

    std::queue<Vehiculo> cola;
    bool ocupada;
    float tiempoRestanteServicio;  // En segundos

    // Estadísticas internas
    float tiempoOcupadoAcumulado;  // Para calcular utilización
    int totalVehiculosProcesados;

public:
    CabinaPeaje(int id_);

    // Gestión de cola
    void agregarVehiculo(const Vehiculo& v, float tiempoSimulado);
    void actualizar(float dt, float tiempoSimulado);  // Procesa servicio

    // Acceso
    bool estaOcupada() const;
    int tamanoCola() const;
    int getTotalProcesados() const;
    float getTiempoOcupado() const;

    // Utilidad
    bool estaLibre() const;
    Vehiculo getVehiculoEnServicio() const;  // Solo si ocupada
};