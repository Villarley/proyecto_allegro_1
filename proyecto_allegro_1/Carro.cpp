#include "Carro.h"

Vehiculo::Vehiculo(int id_, float posicionInicial, float velocidadInicial, float tiempoSimulado)
    : id(id_), posicionX(posicionInicial), velocidad(velocidadInicial),
      estado(EstadoVehiculo::EN_TRANSITO),
      tiempoGenerado(tiempoSimulado),
      tiempoLlegadaCola(-1.0f), tiempoSalidaCabina(-1.0f) {}

int Vehiculo::getId() const { return id; }
float Vehiculo::getPosicionX() const { return posicionX; }
float Vehiculo::getVelocidad() const { return velocidad; }
EstadoVehiculo Vehiculo::getEstado() const { return estado; }

float Vehiculo::getTiempoGenerado() const { return tiempoGenerado; }
float Vehiculo::getTiempoLlegadaCola() const { return tiempoLlegadaCola; }
float Vehiculo::getTiempoSalidaCabina() const { return tiempoSalidaCabina; }

void Vehiculo::setVelocidad(float nuevaVelocidad) { velocidad = nuevaVelocidad; }
void Vehiculo::setEstado(EstadoVehiculo nuevoEstado) { estado = nuevoEstado; }
void Vehiculo::setTiempoLlegadaCola(float tiempo) { tiempoLlegadaCola = tiempo; }
void Vehiculo::setTiempoSalidaCabina(float tiempo) { tiempoSalidaCabina = tiempo; }

void Vehiculo::avanzar(float dt) {
    if (estado == EstadoVehiculo::EN_TRANSITO) {
        posicionX += velocidad * dt;
    }
}

std::string Vehiculo::estadoComoTexto() const {
    switch (estado) {
        case EstadoVehiculo::EN_TRANSITO: return "En tránsito";
        case EstadoVehiculo::EN_COLA: return "En cola";
        case EstadoVehiculo::EN_SERVICIO: return "En servicio";
        case EstadoVehiculo::SALIDO: return "Salido";
        default: return "Desconocido";
    }
}