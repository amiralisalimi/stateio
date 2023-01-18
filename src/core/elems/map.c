#include <SDL2/SDL.h>
#include <stdlib.h>
#include "map.h"
#include "player.h"
#include "area.h"
#include "potion.h"
#include "../video.h"
#include "../log.h"

enum ELE_MapConstants {
    MAX_PLAYER_CNT = 15,
    MAX_AREA_CNT = 31,
    TROOP_RADIUS = 6
};

Map* ELE_CreateMap(
    int id, Player **players, int player_cnt,
    Area **areas, int area_cnt
) {
    if (player_cnt > MAX_PLAYER_CNT) {
        LogInfo("Players too much");
        return NULL;
    }
    if (area_cnt > MAX_AREA_CNT) {
        LogInfo("Areas too much");
        return NULL;
    }
    Map *new_map = malloc(sizeof(Map));
    new_map->id = id;
    new_map->player_cnt = player_cnt;
    new_map->area_cnt = area_cnt;
    new_map->troops_head = NULL;
    if (player_cnt) {
        new_map->players = malloc(sizeof(Player*) * player_cnt);
        memcpy(new_map->players, players, sizeof(Player*) * player_cnt);
    }
    if (area_cnt) {
        new_map->areas = malloc(sizeof(Area*) * area_cnt);
        memcpy(new_map->areas, areas, sizeof(Area*) * area_cnt);
    }
    return new_map;
}

void ELE_DestroyMap(Map *map) {
    if (map == NULL) return;
    for (int i = 0; i < map->area_cnt; i++) ELE_DestroyArea(map->areas[i]);
    Troop *head;
    for (head = map->troops_head; head != NULL; head = head->next) {
        if (head->prev != NULL) {
            ELE_DestroyTroop(head->prev);
        }
        if (head->next == NULL) {
            ELE_DestroyTroop(head);
            break;
        }
    }
    for (int i = 0; i < map->player_cnt; i++) {
        map->players[i]->area_cnt = 0;
        map->players[i]->troop_cnt = 0;
        map->players[i]->attack_delay = 0;
        map->players[i]->applied_potion = NULL;
    }
    free(map->areas);
    free(map->players);
    free(map);
}

Area* ELE_GetAreaById(Map *map, int id) {
    if (map == NULL) return NULL;
    for (int i = 0; i < map->area_cnt; i++) {
        if (map->areas[i]->id == id) {
            return map->areas[i];
        }
    }
    return NULL;
}

int ELE_SaveMap(Map *map, int lastmap, Potion **potions_onmap, int potion_cnt) {
    if (map == NULL) return 0;
    char filename[24];
    if (lastmap)
        sprintf(filename, "bin/data/lastmap.bin");
    else
        sprintf(filename, "bin/data/map%d.bin", map->id);
    SDL_RWops *map_file = SDL_RWFromFile(filename, "w+b");
    if (map_file == NULL && !lastmap) {
        sprintf(filename, "bin/data/map%d", map->id);
        map_file = SDL_RWFromFile(filename, "w+b");
    }
    if (map_file != NULL) {
        SDL_RWwrite(map_file, &map->id, sizeof(int), 1);
        if (lastmap) {
            SDL_RWwrite(map_file, &map->player_cnt, sizeof(int), 1);
            for (int i = 0; i < map->player_cnt; i++) {
                Player *player = map->players[i];
                SDL_RWwrite(map_file, &player->id, sizeof(int), 1);
                SDL_RWwrite(map_file, &player->area_cnt, sizeof(int), 1);
                SDL_RWwrite(map_file, &player->troop_cnt, sizeof(int), 1);
                SDL_RWwrite(map_file, &player->troop_rate, sizeof(int), 1);
                SDL_RWwrite(map_file, &player->attack_delay, sizeof(int), 1);
                Potion *pt = player->applied_potion;
                int haspt = (pt != NULL);
                SDL_RWwrite(map_file, &haspt, sizeof(int), 1);
                if (haspt) SDL_RWwrite(map_file, pt, sizeof(Potion), 1);
            }
            SDL_RWwrite(map_file, &potion_cnt, sizeof(int), 1);
            for (int i = 0; i < potion_cnt; i++) {
                if (potions_onmap[i] != NULL)
                    SDL_RWwrite(map_file, potions_onmap[i], sizeof(Potion), 1);
                else
                    SDL_RWwrite(map_file, &(Potion){0}, sizeof(Potion), 1);
            }
        }
        SDL_RWwrite(map_file, &map->area_cnt, sizeof(int), 1);
        for (int i = 0; i < map->area_cnt; i++) {
            Area *area = map->areas[i];
            SDL_RWwrite(map_file, &area->id, sizeof(int), 1);
            int conq_id = (area->conqueror ? area->conqueror->id : -1);
            SDL_RWwrite(map_file, &conq_id, sizeof(int), 1);
            SDL_RWwrite(map_file, &area->capacity, sizeof(int), 1);
            SDL_RWwrite(map_file, &area->troop_cnt, sizeof(int), 1);
            SDL_RWwrite(map_file, &area->troop_rate, sizeof(int), 1);
            SDL_RWwrite(map_file, &area->troop_inc_delay, sizeof(int), 1);
            SDL_RWwrite(map_file, &area->center, sizeof(SDL_Point), 1);
            SDL_RWwrite(map_file, &area->radius, sizeof(int), 1);
            SDL_RWwrite(map_file, &area->vertex_cnt, sizeof(int), 1);
            for (int i = 0; i < area->vertex_cnt; i++) {
                SDL_RWwrite(map_file, &area->vertices[i], sizeof(SDL_Point), 1);
            }
        }
        if (lastmap) {
            for (int i = 0; i < map->area_cnt; i++) {
                int att_id = (map->areas[i]->attack ? map->areas[i]->attack->id : -1);
                SDL_RWwrite(map_file, &att_id, sizeof(int), 1);
                SDL_RWwrite(map_file, &map->areas[i]->attack_delay, sizeof(int), 1);
                SDL_RWwrite(map_file, &map->areas[i]->attack_cnt, sizeof(int), 1);
            }
            int troop_cnt = ELE_GetMapTroopCnt(map);
            SDL_RWwrite(map_file, &troop_cnt, sizeof(int), 1);
            for (Troop *troop = map->troops_head; troop != NULL; troop = troop->next) {
                SDL_RWwrite(map_file, &troop->id, sizeof(int), 1);
                SDL_RWwrite(map_file, &troop->player->id, sizeof(int), 1);
                SDL_RWwrite(map_file, &troop->x, sizeof(double), 1);
                SDL_RWwrite(map_file, &troop->y, sizeof(double), 1);
                SDL_RWwrite(map_file, &troop->src->id, sizeof(int), 1);
                SDL_RWwrite(map_file, &troop->dst->id, sizeof(int), 1);
            }
        }
        SDL_RWclose(map_file);
    } else {
        LogError("Unable to r/w maps: %s");
        return -1;
    }
    LogInfo("Map save successful: %s", filename);
    return 0;
}

int ELE_GetMapAreaCntSum(Map *map) {
    int sum = 0;
    for (int i = 0; i < map->player_cnt; i++) {
        sum += map->players[i]->area_cnt;
    }
    return sum;
}

int ELE_GetMapTroopCnt(Map *map) {
    int troop_cnt = 0;
    if (map == NULL) return troop_cnt;
    for (Troop *troop = map->troops_head; troop != NULL; troop = troop->next) {
        ++troop_cnt;
    }
    return troop_cnt;
}

void ELE_AddTroopToMap(Map *map, Troop *troop) {
    if (troop == NULL) return;
    troop->next = troop->prev = NULL;
    if (map->troops_head == NULL) {
        map->troops_head = troop;
    } else {
        troop->next = map->troops_head;
        map->troops_head->prev = troop;
        map->troops_head = troop;
    }
}

Troop* ELE_RemoveTroopFromMap(Map *map, Troop *troop) {
    if (troop == NULL) return NULL;
    if (troop->prev != NULL)
        troop->prev->next = troop->next;
    if (troop->next != NULL)
        troop->next->prev = troop->prev;
    if (troop == map->troops_head)
        map->troops_head = troop->next;
    Troop *ret = troop->next;
    ELE_DestroyTroop(troop);
    return ret;
}

int ELE_Collide(Troop *first, Troop *second) {
    if (first == NULL || second == NULL) return 0;
    if (first->player == second->player) return 0;
    int x1 = first->x, y1 = first->y;
    int x2 = second->x, y2 = second->y;
    return (abs(x1 - x2) + abs(y1 - y2) < 2 * TROOP_RADIUS);
}

void ELE_HandleCollisions(Map *map) {
    int w, h;
    VDO_GetWindowSize(&w, &h);
    for (Troop *troop = map->troops_head; troop != NULL;) {
        if (troop->x < 0 || troop->y < 0 || troop->x > w || troop->y > h) {
            troop = ELE_RemoveTroopFromMap(map, troop);
        } else if (abs(troop->x - troop->dx) + abs(troop->y - troop->dy) < 40) {
            Area *src = troop->src, *dst = troop->dst;
            if (dst->conqueror == troop->player) {
                ++dst->troop_cnt;
            } else if (dst->troop_cnt == 0) {
                ++dst->troop_cnt;
                ELE_AreaConquer(dst, troop->player);
            } else {
                --dst->troop_cnt;
            }
            dst->troop_inc_delay = 10;
            troop = ELE_RemoveTroopFromMap(map, troop);
        } else {
            int found = 0;
            for (Troop *troop2 = troop->next; troop2 != NULL;) {
                if (ELE_Collide(troop, troop2)) {
                    troop2 = ELE_RemoveTroopFromMap(map, troop2);
                    found = 1;
                } else {
                    troop2 = troop2->next;
                }
            }
            if (found) {
                troop = ELE_RemoveTroopFromMap(map, troop);
            } else {
                troop = troop->next;
            }
        }
    }
}