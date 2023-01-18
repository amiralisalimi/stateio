#ifndef _MAP_H
#define _MAP_H

#include "player.h"
#include "area.h"
#include "troop.h"

struct Map {
    int id;

    Player **players;
    int player_cnt;

    Area **areas;
    int area_cnt;

    Troop *troops_head;
};
typedef struct Map Map;

extern Map* ELE_CreateMap(
    int id, Player **players, int player_cnt,
    Area **areas, int area_cnt
);
extern void ELE_DestroyMap(Map *map);

extern Area* ELE_GetAreaById(Map *map, int id);

extern int ELE_SaveMap(Map *map, int lastmap, Potion **potions_onmap, int potion_cnt);

extern int ELE_GetMapAreaCntSum(Map *map);

extern int ELE_GetMapTroopCnt(Map *map);

extern void ELE_AddTroopToMap(Map *map, Troop *troop);
extern Troop* ELE_RemoveTroopFromMap(Map *map, Troop *troop);

extern void ELE_HandleCollisions(Map *map);

#endif /* _MAP_H */