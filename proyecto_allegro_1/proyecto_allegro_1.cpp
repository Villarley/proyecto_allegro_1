#include <allegro5/allegro.h>
#include <iostream>
#include "Simulador.h"

int main() {
    //Inicialización de Allegro
    if (!al_init()) {
        std::cerr << "Error al inicializar Allegro.\n";
        return -1;
    }

    //Temporizador de simulación (60 ticks por segundo)
    const float FPS = 60.0f;
    ALLEGRO_TIMER* timer = al_create_timer(1.0 / FPS);
    ALLEGRO_EVENT_QUEUE* queue = al_create_event_queue();

    if (!timer || !queue) {
        std::cerr << "Error al crear temporizador o cola de eventos.\n";
        return -1;
    }

    al_register_event_source(queue, al_get_timer_event_source(timer));

    //Crear simulador con parámetros configurables
    Simulador simulador(
        3,     // Número de carriles
        5,     // Número de cabinas
        800.0f, // Longitud del carril
        50.0f,  // Distancia mínima entre vehículos
        1.0f    // Intervalo promedio de generación de vehículos (segundos)
    );

    // ▶Iniciar simulación
    al_start_timer(timer);
    bool corriendo = true;

    while (corriendo) {
        ALLEGRO_EVENT evento;
        al_wait_for_event(queue, &evento);

        if (evento.type == ALLEGRO_EVENT_TIMER) {
            //Actualizar simulador
            simulador.tick(1.0f / FPS);

            //Mostrar estadísticas parciales cada segundo
            if (static_cast<int>(simulador.getTiempoSimulado()) % 60 == 0) {
                std::cout << "Tiempo: " << simulador.getTiempoSimulado() << "s\n";
                std::cout << "Vehículos procesados: " << simulador.totalVehiculosProcesados() << "\n";
                std::cout << "Utilización promedio cabinas: " << simulador.utilizacionPromedioCabinas() << "%\n";
                std::cout << "Vehículos en sistema: " << simulador.totalVehiculosEnSistema() << "\n";
                std::cout << "-----------------------------\n";
            }

            //Detener tras 5 minutos simulados
            if (simulador.getTiempoSimulado() >= 300.0f) {
                corriendo = false;
            }
        }
    }

    //Limpieza
    al_destroy_timer(timer);
    al_destroy_event_queue(queue);

    std::cout << "Simulación finalizada.\n";
    return 0;
}