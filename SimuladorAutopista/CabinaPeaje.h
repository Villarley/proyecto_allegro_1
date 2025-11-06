#pragma once
#include <deque>
#include "Vehiculo.h"

namespace Escenario {
	//clase que representa una cabina de peaje
    class Cabina {
    public:
		//atributos de la cabina
        int id;
        float x, y;
        std::deque<Vehiculo*> fila;
        Vehiculo* enServicio;

        double tOcupadaAcum;
        double tOcupadaHasta;
        double tBloqueadaHasta;

        int procesados;
		// constructor de la cabina
        Cabina(int _id, float _x, float _y)
            : id(_id), x(_x), y(_y), enServicio(nullptr),
            tOcupadaAcum(0.0), tOcupadaHasta(0.0),
            tBloqueadaHasta(0.0), procesados(0)
        {
        }

        float stopY() const { return y - 20.0f; } // usamos directamente el valor, no la constante global
    };
}
