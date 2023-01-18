#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <stdlib.h>
#include "../video.h"
#include "troop.h"
#include "../log.h"

Troop* ELE_CreateTroop(
    int id, Player *player, double x, double y,
    Area *src, Area *dst, Troop *next, Troop *prev
) {
    Troop *new_troop = malloc(sizeof(Troop));
    new_troop->id = id;
    new_troop->player = player;
    ++player->troop_cnt;
    new_troop->x = x;
    new_troop->y = y;
    new_troop->src = src;
    new_troop->dst = dst;
    new_troop->sx = src->center.x;
    new_troop->sy = src->center.y;
    new_troop->dx = dst->center.x;
    new_troop->dy = dst->center.y;
    new_troop->next = next;
    new_troop->prev = prev;
    return new_troop;
}

void ELE_DestroyTroop(Troop *troop) {
    --troop->player->troop_cnt;
    free(troop);
}