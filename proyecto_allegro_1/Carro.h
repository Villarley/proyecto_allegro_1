#pragma once
#include <string>

enum class EstadoVehiculo {
    EN_TRANSITO,
    EN_COLA,
    EN_SERVICIO,
    SALIDO
};

class Vehiculo {
private:
    int id;
    float posicionX;
    float velocidad;
    EstadoVehiculo estado;

    // Tiempos en segundos desde el inicio de la simulación
    float tiempoGenerado;
    float tiempoLlegadaCola;
    float tiempoSalidaCabina;

public:
    Vehiculo(int id_, float posicionInicial, float velocidadInicial, float tiempoSimulado);

    // Getters
    int getId() const;
    float getPosicionX() const;
    float getVelocidad() const;
    EstadoVehiculo getEstado() const;

    float getTiempoGenerado() const;
    float getTiempoLlegadaCola() const;
    float getTiempoSalidaCabina() const;

    // Setters
    void setVelocidad(float nuevaVelocidad);
    void setEstado(EstadoVehiculo nuevoEstado);
    void setTiempoLlegadaCola(float tiempo);
    void setTiempoSalidaCabina(float tiempo);

    // Movimiento
    void avanzar(float dt);  // dt en segundos

    // Utilidad
    std::string estadoComoTexto() const;
};
