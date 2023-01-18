#include <SDL2/SDL.h>
#include <stdlib.h>
#include <string.h>
#include "player.h"
#include "../log.h"

enum ELE_PlayerConstants {
    MAX_NAME_LEN = 15,
    DEFAULT_TROOP_RATE = 60 /* Frame */
};

Player* ELE_CreatePlayer(
    int id, const char *name, SDL_Color color, int score) {
    if (strlen(name) > MAX_NAME_LEN) {
        LogInfo("Player name too long");
        return NULL;
    }
    Player *new_player = malloc(sizeof(Player));
    new_player->id = id;
    strcpy(new_player->name, name);
    new_player->score = 0;
    new_player->troop_cnt = 0;
    new_player->troop_rate = DEFAULT_TROOP_RATE;
    new_player->area_cnt = 0;
    new_player->color = color;
    new_player->score = score;
    new_player->attack_delay = 0;
    new_player->applied_potion = NULL;
    return new_player;
}

void ELE_DestroyPlayer(Player *player) {
    free(player);
}

int ELE_CmpPlayersByScore(const void *first, const void *second) {
    Player *f = *(Player**)first;
    Player *s = *(Player**)second;
    if (f->score < s->score) return 1;
    if (f->score > s->score) return -1;
    return 0;
}

void ELE_SortPlayersByScore(Player **players, int player_cnt) {
    qsort(players, player_cnt, sizeof(Player*), ELE_CmpPlayersByScore);
}

int ELE_SavePlayers(Player *players, int player_cnt) {
    const char *players_filename = "bin/data/players.bin";
    SDL_RWops *players_file = SDL_RWFromFile(players_filename, "w+b");
    if (players_file != NULL) {
        SDL_RWwrite(players_file, players, sizeof(Player), player_cnt);
        SDL_RWclose(players_file);
    } else {
        LogError("Unable to r/w players: %s");
        return -1;
    }
    return 0;
}