#include <SDL2/SDL.h>
#include <stdlib.h>
#include "potion.h"

Potion* ELE_CreatePotion(
    int id, int type, int frames_onmap, int frames_applied,
    SDL_Point center
) {
    Potion *new_potion = malloc(sizeof(Potion));
    new_potion->id = id;
    new_potion->type = type;
    new_potion->frames_onmap = frames_onmap;
    new_potion->frames_applied = frames_applied;
    new_potion->center = center;
    return new_potion;
}

void ELE_DestroyPotion(Potion *potion) {
    free(potion);
}