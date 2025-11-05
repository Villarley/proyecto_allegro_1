#pragma once
#include <algorithm>
#include <cmath>

namespace Escenario {

    enum Estado { EnCarretera, EnCola, EnServicio, Saliendo, Salio };

    class Vehiculo {
    public:
        int id;
        float x, y;
        float velocidad;
        float vSalida;
        int cabinaElegida;
        int carrilInicial;
        Estado estado;

        float tiempoServicio;
        float restanteServicio;

        double tCreacion;
        double tLlegadaCola;
        double tInicioServicio;
        double tSalidaCabina;

        Vehiculo(int _id, float _x, float _y, float _vel, int _lane, double simTime)
            : id(_id), x(_x), y(_y), velocidad(_vel),
            vSalida(std::max(_vel * 0.65f, 120.0f)),
            cabinaElegida(-1), carrilInicial(_lane), estado(EnCarretera),
            tiempoServicio(0.0f), restanteServicio(0.0f),
            tCreacion(simTime), tLlegadaCola(-1), tInicioServicio(-1), tSalidaCabina(-1)
        {
        }
    };
}
