#pragma once
#include <vector>
#include "Carro.h"

class Carril {
private:
    int idCarril;
    float longitud;  // Longitud total del carril en unidades lógicas
    float distanciaSeguridad;  // Mínima distancia entre vehículos

    std::vector<Vehiculo> vehiculos;

public:
    Carril(int id, float longitud_, float distanciaSeguridad_);

    // Gestión de vehículos
    void agregarVehiculo(const Vehiculo& v);
    void actualizar(float dt, float tiempoSimulado);  // Mueve vehículos y aplica lógica de frenado

    // Acceso
    std::vector<Vehiculo>& obtenerVehiculos() ;
    int cantidadVehiculos() const;

    // Utilidad
    void eliminarVehiculosSalidos();  // Limpia vehículos que ya salieron del sistema
};