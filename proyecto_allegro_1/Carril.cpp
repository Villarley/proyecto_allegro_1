#include "Carril.h"

Carril::Carril(int id, float longitud_, float distanciaSeguridad_)
    : idCarril(id), longitud(longitud_), distanciaSeguridad(distanciaSeguridad_) {
}

void Carril::agregarVehiculo(const Vehiculo& v) {
    vehiculos.push_back(v);
}

void Carril::actualizar(float dt, float tiempoSimulado) {
    for (size_t i = 0; i < vehiculos.size(); ++i) {
        Vehiculo& actual = vehiculos[i];

        // Si está en tránsito, verificar distancia con el vehículo anterior
        if (actual.getEstado() == EstadoVehiculo::EN_TRANSITO) {
            bool debeFrenar = false;

            if (i > 0) {
                const Vehiculo& anterior = vehiculos[i - 1];
                float distancia = anterior.getPosicionX() - actual.getPosicionX();

                if (distancia < distanciaSeguridad) {
                    actual.setVelocidad(0.0f);  // Frenar
                    debeFrenar = true;
                }
            }

            if (!debeFrenar) {
                actual.avanzar(dt);
            }

            // Si supera la longitud del carril, marcar como salido
            if (actual.getPosicionX() >= longitud) {
                actual.setEstado(EstadoVehiculo::SALIDO);
                actual.setTiempoSalidaCabina(tiempoSimulado);  // Marca salida lógica
            }
        }
    }
}

std::vector<Vehiculo>& Carril::obtenerVehiculos() {
    return vehiculos;
}

int Carril::cantidadVehiculos() const {
    return static_cast<int>(vehiculos.size());
}

void Carril::eliminarVehiculosSalidos() {
    vehiculos.erase(
        std::remove_if(vehiculos.begin(), vehiculos.end(),
            [](const Vehiculo& v) {
                return v.getEstado() == EstadoVehiculo::SALIDO;
            }),
        vehiculos.end()
    );
}