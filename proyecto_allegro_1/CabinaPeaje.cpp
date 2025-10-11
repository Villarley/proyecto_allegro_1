#include "CabinaPeaje.h"

CabinaPeaje::CabinaPeaje(int id_)
    : idCabina(id_), ocupada(false), tiempoRestanteServicio(0.0f),
    tiempoOcupadoAcumulado(0.0f), totalVehiculosProcesados(0) {
}

void CabinaPeaje::agregarVehiculo(const Vehiculo& v, float tiempoSimulado) {
    Vehiculo copia = v;
    copia.setEstado(EstadoVehiculo::EN_COLA);
    copia.setTiempoLlegadaCola(tiempoSimulado);
    cola.push(copia);
}

void CabinaPeaje::actualizar(float dt, float tiempoSimulado) {
    if (ocupada) {
        tiempoRestanteServicio -= dt;
        tiempoOcupadoAcumulado += dt;

        if (tiempoRestanteServicio <= 0.0f) {
            // Vehículo termina servicio
            Vehiculo v = cola.front();
            cola.pop();

            v.setEstado(EstadoVehiculo::SALIDO);
            v.setTiempoSalidaCabina(tiempoSimulado);
            totalVehiculosProcesados++;

            ocupada = false;
        }
    }

    if (!ocupada && !cola.empty()) {
        // Iniciar servicio al siguiente vehículo
        Vehiculo& v = cola.front();
        v.setEstado(EstadoVehiculo::EN_SERVICIO);
        tiempoRestanteServicio = 2.0f + static_cast<float>(rand()) / RAND_MAX * 3.0f;  // entre 2 y 5 s
        ocupada = true;
    }
}

bool CabinaPeaje::estaOcupada() const {
    return ocupada;
}

bool CabinaPeaje::estaLibre() const {
    return !ocupada;
}

int CabinaPeaje::tamanoCola() const {
    return static_cast<int>(cola.size());
}

int CabinaPeaje::getTotalProcesados() const {
    return totalVehiculosProcesados;
}

float CabinaPeaje::getTiempoOcupado() const {
    return tiempoOcupadoAcumulado;
}

Vehiculo CabinaPeaje::getVehiculoEnServicio() const {
    return cola.front();  // Solo válido si ocupada == true