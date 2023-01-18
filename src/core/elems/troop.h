#ifndef _TROOP_H
#define _TROOP_H

#include <SDL2/SDL.h>
#include "player.h"
#include "area.h"

struct Troop {
    int id;
    Player *player;
    double x, y;
    int sx, sy;
    int dx, dy;
    Area *src, *dst;
    struct Troop *next;
    struct Troop *prev;
};
typedef struct Troop Troop;

extern Troop* ELE_CreateTroop(
    int id, Player *player, double x, double y,
    Area *src, Area *dst, Troop *next, Troop *prev
);
extern void ELE_DestroyTroop(Troop *troop);

#endif /* _TROOP_H */