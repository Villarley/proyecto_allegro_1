#include <allegro5/allegro.h>
int main() {
    if (!al_init()) return -1;
    ALLEGRO_DISPLAY* display = al_create_display(800, 600);
    al_clear_to_color(al_map_rgb(0, 0, 0));
    al_flip_display();
    al_rest(2.0);
    al_destroy_display(display);
    return 0;
}
