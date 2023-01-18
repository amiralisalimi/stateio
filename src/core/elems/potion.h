#ifndef _POTION_H
#define _POTION_H

#include <SDL2/SDL.h>

enum POTION_TYPES {
    TROOP_SPEED_X2,
    TROOP_FREEZE_OTHERS,
    AREA_BEYOND_CAPACITY,
    AREA_SHIELD
};

struct Potion {
    int id;
    int type;
    int frames_onmap;
    int frames_applied;

    SDL_Point center;
};
typedef struct Potion Potion;

extern Potion* ELE_CreatePotion(
    int id, int type, int frames_onmap, int frames_applied,
    SDL_Point center
);
extern void ELE_DestroyPotion(Potion *potion);

#endif /* _POTION_H */