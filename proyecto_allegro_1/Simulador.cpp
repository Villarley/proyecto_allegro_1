#include "Simulador.h"
#include <cstdlib>

Simulador::Simulador(int numCarriles, int numCabinas, float longitudCarril, float distanciaSeguridad, float intervaloGeneracion)
    : tiempoSimulado(0.0f), tiempoEntreVehiculos(intervaloGeneracion),
    acumuladorTiempo(0.0f), contadorVehiculos(0) {

    for (int i = 0; i < numCarriles; ++i) {
        carriles.emplace_back(i, longitudCarril, distanciaSeguridad);
    }

    for (int j = 0; j < numCabinas; ++j) {
        cabinas.emplace_back(j);
    }
}

void Simulador::tick(float dt) {
    tiempoSimulado += dt;
    acumuladorTiempo += dt;

    // Generación probabilística de vehículos
    if (acumuladorTiempo >= tiempoEntreVehiculos) {
        acumuladorTiempo = 0.0f;

        int carrilSeleccionado = rand() % carriles.size();
        float velocidad = 50.0f + static_cast<float>(rand()) / RAND_MAX * 30.0f;  // entre 50 y 80
        Vehiculo nuevo(contadorVehiculos++, 0.0f, velocidad, tiempoSimulado);
        carriles[carrilSeleccionado].agregarVehiculo(nuevo);
    }

    // Actualizar carriles
    for (Carril& c : carriles) {
        c.actualizar(dt, tiempoSimulado);
        c.eliminarVehiculosSalidos();
    }

    // Asignar vehículos a cabinas si cruzan el final
    for (Carril& c : carriles) {
        for (Vehiculo& v : c.obtenerVehiculos()) {
            if (v.getEstado() == EstadoVehiculo::SALIDO) {
                // Asignar a cabina más corta
                int cabinaElegida = 0;
                int minCola = cabinas[0].tamanoCola();
                for (size_t i = 1; i < cabinas.size(); ++i) {
                    if (cabinas[i].tamanoCola() < minCola) {
                        cabinaElegida = static_cast<int>(i);
                        minCola = cabinas[i].tamanoCola();
                    }
                }
                cabinas[cabinaElegida].agregarVehiculo(v, tiempoSimulado);
            }
        }
    }

    // Actualizar cabinas
    for (CabinaPeaje& cab : cabinas) {
        cab.actualizar(dt, tiempoSimulado);
    }
}

float Simulador::getTiempoSimulado() const {
    return tiempoSimulado;
}

int Simulador::totalVehiculosProcesados() const {
    int total = 0;
    for (const CabinaPeaje& cab : cabinas) {
        total += cab.getTotalProcesados();
    }
    return total;
}

float Simulador::utilizacionPromedioCabinas() const {
    float totalTiempo = 0.0f;
    for (const CabinaPeaje& cab : cabinas) {
        totalTiempo += cab.getTiempoOcupado();
    }
    return (totalTiempo / (cabinas.size() * tiempoSimulado)) * 100.0f;
}

int Simulador::totalVehiculosEnSistema() const {
    int total = 0;