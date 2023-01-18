#ifndef _PLAYER_H
#define _PLAYER_H

#include <SDL2/SDL.h>
#include "potion.h"

struct Player {
    int id;
    char name[32];
    int score;

    int area_cnt;
    int troop_cnt;
    int troop_rate;
    SDL_Color color;

    int attack_delay;

    Potion *applied_potion;
};
typedef struct Player Player;

extern Player* ELE_CreatePlayer(
    int id, const char *name, SDL_Color color, int score);
extern void ELE_DestroyPlayer(Player *player);

extern void ELE_SortPlayersByScore(Player **players, int player_cnt);

extern int ELE_SavePlayers(Player *players, int player_cnt);

#endif /* _PLAYER_H */