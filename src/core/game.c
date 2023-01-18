#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <time.h>
#include "game.h"
#include "video.h"
#include "log.h"
#include "elems/player.h"
#include "elems/area.h"
#include "elems/potion.h"
#include "elems/map.h"

#define RGBAColor(color) color.r, color.g, color.b, color.a

enum GME_GameConstants {
    MAX_PLAYER_CNT = 11,
    MAX_AREA_CNT = 31
};

int GME_Init() {
    srand(time(NULL));
    Uint32 flags = SDL_INIT_VIDEO;
    if (SDL_Init(flags) != 0) {
        LogError("Unable to init sdl: %s");
        return -1;
    }
    if (VDO_Init() != 0) {
        return -1;
    }
    if (TTF_Init() != 0) {
        return -1;
    }
    flags = IMG_INIT_PNG;
    if (IMG_Init(flags) == 0) {
        LogInfo("IMG_Error: %s", IMG_GetError());
        return -1;
    }
    return 0;
}

void GME_Quit() {
    LogInfo("Gracefully quitting game...");
    Player **players = GME_GetPlayers();
    Player player_arr[MAX_PLAYER_CNT] = {0};
    for (int i = 0; i < MAX_PLAYER_CNT; i++) {
        if (players[i] != NULL) {
            player_arr[i] = *players[i];
            ELE_DestroyPlayer(players[i]);
        } else {
            break;
        }
    }
    ELE_SavePlayers(player_arr, GME_GetPlayerCnt());
    VDO_Quit();
    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
    LogInfo("Done.");
}

int map_cnt = 0;

int GME_Start() {
    if (GME_RetrievePlayers() != 0) {
        return -1;
    }
    for (int i = 0; i < 9; i++) {
        char buffer[25];
        sprintf(buffer, "bin/data/map%d.bin", i);
        SDL_RWops *file = SDL_RWFromFile(buffer, "rb");
        if (file == NULL) {
            sprintf(buffer, "bin/data/map%d", i);
            file = SDL_RWFromFile(buffer, "rb");
            if (file == NULL) break;
            else {
                ++map_cnt;
                SDL_RWclose(file);
            }
        } else {
            ++map_cnt;
            SDL_RWclose(file);
        }
    }
    switch (GME_GetCurPlayer()) {
        case 1:
            return 0;
        case -1:
            return -1;
    }
    return GME_Menu();
}

const SDL_Color g_BackgroundColor = (SDL_Color){229, 229, 229, 255};
const SDL_Color g_GreyColor = (SDL_Color){147, 147, 147, 255};
const SDL_Color g_BlackColor = (SDL_Color){0, 0, 0, 255};
const SDL_Color g_LightBlackColor = (SDL_Color){50, 50, 50, 255};
const SDL_Color g_WhiteColor = (SDL_Color){255, 255, 255, 255};
const SDL_Color g_BlueColor = (SDL_Color){0, 120, 230, 255};
const SDL_Color g_PlayerColors[MAX_PLAYER_CNT] = {
    (SDL_Color){.r =  80, .g = 215, .b = 185, .a = 255},
    (SDL_Color){.r =  75, .g = 115, .b = 215, .a = 255},
    (SDL_Color){.r = 255, .g = 130, .b = 115, .a = 255},
    (SDL_Color){.r = 225, .g = 140, .b = 210, .a = 255},
    (SDL_Color){.r = 245, .g =  65, .b =  40, .a = 255},
    (SDL_Color){.r = 250, .g = 180, .b =  60, .a = 255},
    (SDL_Color){.r = 160, .g = 130, .b =  95, .a = 255},
    (SDL_Color){.r =   0, .g =  80, .b =  50, .a = 255},
    (SDL_Color){.r =  80, .g = 185, .b = 100, .a = 255},
    (SDL_Color){.r = 215, .g = 220, .b =  30, .a = 255},
    (SDL_Color){.r =  75, .g =   0, .b = 206, .a = 255}
};
const SDL_Color g_PotionColors[4] = {
    (SDL_Color){0, 216, 64, 255},
    (SDL_Color){241, 214, 0, 255},
    (SDL_Color){174, 0, 128, 255},
    (SDL_Color){165, 0, 0, 255}
};

Player *g_Players[MAX_PLAYER_CNT];
Area *g_Areas[MAX_AREA_CNT];

Player *g_CurPlayer;

Map *g_CurMap;

Potion **g_Potions;
int potion_cnt;
int potion_size;

SDL_Texture *g_PotionTextures[4];

int GME_Scoreboard() {
    int quit = 0;
    int sdl_quit = 0;
    int w, h;
    VDO_GetWindowSize(&w, &h);
    SDL_Event e;
    SDL_Renderer *renderer = VDO_GetRenderer();
    TTF_Font *font = TTF_OpenFont("bin/fonts/SourceCodePro.ttf", 24);
    int table_width = 500, table_height = 600;
    SDL_Rect table = {w / 2 - table_width / 2, h / 2 - table_height / 2 - 30, table_width, table_height};
    int back_btn_sz = 70;
    SDL_Rect back_btn = {30, h - 25 - back_btn_sz, back_btn_sz, back_btn_sz};
    int entry_margin = 15, entry_height = 20, name_width = 350, score_width = 120;
    int player_cnt = GME_GetPlayerCnt();
    Player **players = malloc(sizeof(Player*) * player_cnt);
    memcpy(players, g_Players, sizeof(Player*) * player_cnt);
    ELE_SortPlayersByScore(players, player_cnt);
    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = 1;
                sdl_quit = 1;
            } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                int x, y;
                SDL_GetMouseState(&x, &y);
                if (back_btn.x <= x && x <= back_btn.x + back_btn.w &&
                    back_btn.y <= y && y <= back_btn.y + back_btn.h) {
                    quit = 1;
                    break;
                }
            }
        }
        boxRGBA(renderer, 0, 0, w, h, RGBAColor(g_BackgroundColor));
        roundedBoxRGBA(renderer, table.x, table.y, table.x + table.w, table.y + table.h, 10,
            RGBAColor(g_GreyColor));
        GME_WriteTTF(renderer, font, "Player", g_WhiteColor, table.x + entry_margin + name_width / 2,
            table.y + entry_margin + entry_height / 2);
        GME_WriteTTF(renderer, font, "Score", g_WhiteColor, table.x + table.w - entry_margin - score_width / 2,
            table.y + entry_margin + entry_height / 2);
        lineRGBA(renderer, table.x + entry_margin, table.y + 2 * entry_margin + entry_height,
            table.x + entry_margin + name_width - 4, table.y + 2 * entry_margin + entry_height,
            RGBAColor(g_WhiteColor));
        lineRGBA(renderer, table.x + table.w - entry_margin - score_width + 4,
            table.y + 2 * entry_margin + entry_height,
            table.x + table.w - entry_margin - 2, table.y + 2 * entry_margin + entry_height,
            RGBAColor(g_WhiteColor));
        for (int i = 0; i < player_cnt; i++) {
            Player *player = players[i];
            GME_WriteTTF(renderer, font, player->name, g_WhiteColor,
                table.x + entry_margin + name_width / 2,
                table.y + (i + 3) * entry_margin + (i + 1) * entry_height + entry_height / 2);
            char buffer[10];
            sprintf(buffer, "%d", player->score);
            GME_WriteTTF(renderer, font, buffer, g_WhiteColor,
                table.x + table.w - entry_margin - score_width / 2,
                table.y + (i + 3) * entry_margin + (i + 1) * entry_height + entry_height / 2);
        }
        roundedBoxRGBA(renderer, back_btn.x, back_btn.y, back_btn.x + back_btn.w,
            back_btn.y + back_btn.h, 10, RGBAColor(g_GreyColor));
        filledTrigonRGBA(renderer, back_btn.x + 20, back_btn.y + back_btn.h / 2,
            back_btn.x + back_btn.w - 20, back_btn.y + 15,
            back_btn.x + back_btn.w - 20, back_btn.y + back_btn.h - 15,
            RGBAColor(g_BackgroundColor));
        char buffer[50];
        sprintf(buffer, "Player: %s", g_CurPlayer->name);
        roundedRectangleRGBA(renderer, w - 370, h - 75, w - 20, h - 25, 10, RGBAColor(g_BlackColor));
        GME_WriteTTF(renderer, font, buffer, g_BlackColor,
            w - 200, h - 50);
        SDL_RenderPresent(renderer);
    }
    TTF_CloseFont(font);
    free(players);
    if (sdl_quit) return 1;
    return 0;
}

int GME_ChooseMap() {
    int quit = 0;
    int sdl_quit = 0;
    int w, h;
    VDO_GetWindowSize(&w, &h);
    SDL_Event e;
    SDL_Renderer *renderer = VDO_GetRenderer();
    int back_btn_sz = 70;
    SDL_Rect back_btn = {30, h - 25 - back_btn_sz, back_btn_sz, back_btn_sz};
    int btn_w = 150, btn_h = 180, btn_marg = 20;
    SDL_Rect btn[9];
    for (int i = 0; i < 9; i++) {
        int ti = i % 3, tj = i / 3;
        btn[i].x = w / 2 - (1 - ti) * (btn_w + btn_marg) - btn_w / 2;
        btn[i].y = h / 2 - (1 - tj) * (btn_h + btn_marg) - btn_h / 2 - 50;
        btn[i].w = btn_w; btn[i].h = btn_h;
    }
    SDL_Rect rnd = {back_btn.x + back_btn.w + 20, h - 95, btn_w + 100, 70};
    SDL_Rect tst = {rnd.x + rnd.w + 20, h - 95, btn_w + 60, 70};
    TTF_Font *font = TTF_OpenFont("bin/fonts/SourceCodePro.ttf", 24);
    int mapid = -10;
    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = sdl_quit = 1;
            } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                int x, y;
                SDL_GetMouseState(&x, &y);
                if (back_btn.x <= x && x <= back_btn.x + back_btn.w &&
                    back_btn.y <= y && y <= back_btn.y + back_btn.h) {
                    quit = 1;
                    break;
                }
                if (rnd.x <= x && x <= rnd.x + rnd.w &&
                    rnd.y <= y && y <= rnd.y + rnd.h) {
                    mapid = -1;
                    quit = 1;
                }
                if (tst.x <= x && x <= tst.x + tst.w &&
                    tst.y <= y && y <= tst.y + tst.h) {
                    mapid = 100;
                    quit = 1;
                }
                for (int i = 0; i < 9; i++) {
                    if (i == map_cnt) break;
                    if (btn[i].x <= x && x <= btn[i].x + btn[i].w &&
                        btn[i].y <= y && y <= btn[i].y + btn[i].h) {
                        mapid = i;
                        quit = 1;
                        break;
                    }
                }
            }
        }
        boxRGBA(renderer, 0, 0, w, h, RGBAColor(g_BackgroundColor));
        for (int i = 0; i < 9; i++) {
            roundedRectangleRGBA(renderer, btn[i].x, btn[i].y, btn[i].x + btn[i].w, btn[i].y + btn[i].h,
                10, RGBAColor(g_GreyColor));
            if (i < map_cnt) {
                roundedBoxRGBA(renderer, btn[i].x, btn[i].y, btn[i].x + btn[i].w, btn[i].y + btn[i].h,
                    10, RGBAColor(g_GreyColor));
                char buffer[10];
                sprintf(buffer, "Map %d", i);
                GME_WriteTTF(renderer, font, buffer, g_WhiteColor, btn[i].x + btn[i].w / 2, btn[i].y + btn[i].h / 2);
            }
        }
        roundedBoxRGBA(renderer, rnd.x, rnd.y, rnd.x + rnd.w, rnd.y + rnd.h, 10,
            RGBAColor(g_GreyColor));
        GME_WriteTTF(renderer, font, "Generate Random", g_WhiteColor, rnd.x + rnd.w / 2, h - 60);
        roundedBoxRGBA(renderer, tst.x, tst.y, tst.x + tst.w, tst.y + tst.h, 10,
            RGBAColor(g_GreyColor));
        GME_WriteTTF(renderer, font, "Test Arena", g_WhiteColor, tst.x + tst.w / 2, h - 60);
        char buffer[50];
        sprintf(buffer, "Player: %s", g_CurPlayer->name);
        roundedRectangleRGBA(renderer, w - 370, h - 75, w - 20, h - 25, 10, RGBAColor(g_BlackColor));
        GME_WriteTTF(renderer, font, buffer, g_BlackColor,
            w - 200, h - 50);
        roundedBoxRGBA(renderer, back_btn.x, back_btn.y, back_btn.x + back_btn.w,
            back_btn.y + back_btn.h, 10, RGBAColor(g_GreyColor));
        filledTrigonRGBA(renderer, back_btn.x + 20, back_btn.y + back_btn.h / 2,
            back_btn.x + back_btn.w - 20, back_btn.y + 15,
            back_btn.x + back_btn.w - 20, back_btn.y + back_btn.h - 15,
            RGBAColor(g_BackgroundColor));
        SDL_RenderPresent(renderer);
    }
    TTF_CloseFont(font);
    if (sdl_quit) return 1;
    if (mapid == -1) {
        if (GME_MapStart(0) == 1) sdl_quit = 1;
        GME_MapQuit(GME_GetCurMap());
    } else if (mapid == 100) {
        if (GME_GenerateTestArena() < 0) return -1;
        if (GME_MapStart(GME_GetCurMap()) == 1) sdl_quit = 1;
        GME_MapQuit(GME_GetCurMap());
    } else if (mapid >= 0) {
        if (GME_RetrieveMap(mapid) < 0) return -1;
        if (GME_MapStart(GME_GetCurMap()) == 1) sdl_quit = 1;
        GME_MapQuit(GME_GetCurMap());
    }
    if (sdl_quit) return 1;
    return 0;
}

int GME_Menu() {
    int quit = 0;
    int w, h;
    VDO_GetWindowSize(&w, &h);
    SDL_Event e;
    SDL_Renderer *renderer = VDO_GetRenderer();
    TTF_Font *font = TTF_OpenFont("bin/fonts/Aaargh.ttf", 32);
    TTF_Font *font_small = TTF_OpenFont("bin/fonts/SourceCodePro.ttf", 24);
    int button_width = 400, button_height = 80, button_margin = 30;
    SDL_Rect new_game_btn = {w / 2 - button_width / 2, h / 2 - 3 * button_height / 2 - button_margin,
        button_width, button_height};
    SDL_Rect cont_game_btn = {w / 2 - button_width / 2, h / 2 - button_height / 2,
        button_width, button_height};
    SDL_Rect scoreboard_btn = {w / 2 - button_width / 2, h / 2 + button_height / 2 + button_margin,
        button_width, button_height};
    while (!quit) {
        int next = -1;
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = 1;
            } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                int x, y;
                SDL_GetMouseState(&x, &y);
                if (new_game_btn.x <= x && x <= new_game_btn.x + new_game_btn.w &&
                    new_game_btn.y <= y && y <= new_game_btn.y + new_game_btn.h) {
                    next = 0;
                } else if (cont_game_btn.x <= x && x <= cont_game_btn.x + cont_game_btn.w &&
                    cont_game_btn.y <= y && y <= cont_game_btn.y + cont_game_btn.h) {
                    next = 1;
                } else if (scoreboard_btn.x <= x && x <= scoreboard_btn.x + scoreboard_btn.w &&
                    scoreboard_btn.y <= y && y <= scoreboard_btn.y + scoreboard_btn.h) {
                    next = 2;
                }
            }
        }
        if (next == 0) {
            if (GME_ChooseMap() == 1) {
                quit = 1;
            }
        } else if (next == 1) {
            if (GME_RetrieveMap(-1) == 0) {
                if (GME_MapStart(GME_GetCurMap()) == 1) {
                    quit = 1;
                }
                GME_MapQuit(GME_GetCurMap());
            }
        } else if (next == 2) {
            if (GME_Scoreboard() == 1) {
                quit = 1;
            }
        }
        boxRGBA(renderer, 0, 0, w, h, RGBAColor(g_BackgroundColor));
        roundedBoxRGBA(renderer, new_game_btn.x, new_game_btn.y,
            new_game_btn.x + new_game_btn.w, new_game_btn.y + new_game_btn.h,
            10, RGBAColor(g_GreyColor));
        GME_WriteTTF(renderer, font, "New Game", g_WhiteColor, new_game_btn.x + new_game_btn.w / 2,
            new_game_btn.y + new_game_btn.h / 2);
        roundedBoxRGBA(renderer, cont_game_btn.x, cont_game_btn.y,
            cont_game_btn.x + cont_game_btn.w, cont_game_btn.y + cont_game_btn.h,
            10, RGBAColor(g_GreyColor));
        GME_WriteTTF(renderer, font, "Continue Last Game", g_WhiteColor, cont_game_btn.x + cont_game_btn.w / 2,
            cont_game_btn.y + cont_game_btn.h / 2);
        roundedBoxRGBA(renderer, scoreboard_btn.x, scoreboard_btn.y,
            scoreboard_btn.x + scoreboard_btn.w, scoreboard_btn.y + scoreboard_btn.h,
            10, RGBAColor(g_GreyColor));
        GME_WriteTTF(renderer, font, "Scoreboard", g_WhiteColor, scoreboard_btn.x + scoreboard_btn.w / 2,
            scoreboard_btn.y + scoreboard_btn.h / 2);
        char buffer[50];
        sprintf(buffer, "Player: %s", g_CurPlayer->name);
        roundedRectangleRGBA(renderer, w - 370, h - 75, w - 20, h - 25, 10, RGBAColor(g_BlackColor));
        GME_WriteTTF(renderer, font_small, buffer, g_BlackColor,
            w - 200, h - 50);
        SDL_RenderPresent(renderer);
    }
    TTF_CloseFont(font);
    TTF_CloseFont(font_small);
    return 0;
}

Map* GME_GetCurMap() {
    return g_CurMap;
}

Player** GME_GetPlayers() {
    return g_Players;
}

Area** GME_GetAreas() {
    return g_Areas;
}

int GME_GetPlayerCnt() {
    int player_cnt = MAX_PLAYER_CNT;
    for (int i = 0; i < MAX_PLAYER_CNT; i++) {
        if (g_Players[i] == NULL) {
            player_cnt = i;
            break;
        }
    }
    return player_cnt;
}

int GME_GetAreaCnt() {
    int area_cnt = MAX_AREA_CNT;
    for (int i = 0; i < MAX_AREA_CNT; i++) {
        if (g_Areas[i] == NULL) {
            area_cnt = i;
            break;
        }
    }
    return area_cnt;
}

int GME_WriteTTF(SDL_Renderer *renderer, TTF_Font *font, const char *s, SDL_Color color,
        int x, int y) {
    SDL_Surface *text = TTF_RenderText_Solid(font, s, color);
    SDL_Texture *text_texture = SDL_CreateTextureFromSurface(renderer, text);
    SDL_Rect obox = {x - text->w / 2, y - text->h / 2, text->w, text->h};
    SDL_Rect ibox = {0, 0, text->w, text->h};
    if (text == NULL || text_texture == NULL) {
        SDL_FreeSurface(text);
        SDL_DestroyTexture(text_texture);
        LogInfo("TTF_Error: %s", TTF_GetError());
        return -1;
    }
    SDL_RenderCopy(renderer, text_texture, &ibox, &obox);
    SDL_FreeSurface(text);
    SDL_DestroyTexture(text_texture);
    return 0;
}

int GME_GetCurPlayer() {
    g_CurPlayer = NULL;
    int w, h;
    VDO_GetWindowSize(&w, &h);
    SDL_Renderer *renderer = VDO_GetRenderer();
    char name[40] = " ";
    int name_ptr = 0;
    int quit = 0;
    SDL_Event e;
    SDL_Color text_color = g_GreyColor;
    SDL_StartTextInput();
    SDL_Surface *icon = IMG_Load("bin/images/icon.png");
    if (icon == NULL) {
        LogInfo("IMG_Error: %s", IMG_GetError());
        return -1;
    }
    TTF_Font *font = TTF_OpenFont("bin/fonts/SourceCodePro.ttf", 24);
    SDL_Texture *icon_texture = SDL_CreateTextureFromSurface(renderer, icon);
    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_BACKSPACE && name_ptr > 1) {
                    name[--name_ptr] = 0;
                } else if (e.key.keysym.sym == SDLK_RETURN && name_ptr > 1) {
                    quit = 1;
                }
            } else if (e.type == SDL_TEXTINPUT) {
                if (name_ptr < 15)
                    strcat(name, e.text.text);
                name_ptr = strlen(name);
            } else if (e.type == SDL_QUIT) {
                SDL_StopTextInput();
                SDL_DestroyTexture(icon_texture);
                SDL_FreeSurface(icon);
                TTF_CloseFont(font);
                return 1;
            }
        }
        boxRGBA(renderer, 0, 0, w, h, RGBAColor(g_BackgroundColor));
        int icon_w = SDL_min(icon->w, 200);
        int icon_h = SDL_min(icon->h, 200);
        SDL_Rect icon_src = {0, 0, icon->w, icon->h};
        SDL_Rect icon_box = {w / 2 - icon_w / 2, h / 3 - icon_h / 2 - 50, icon_w, icon_h};
        SDL_RenderCopy(renderer, icon_texture, &icon_src, &icon_box);
        roundedBoxRGBA(renderer, w / 2 - 250, h / 2 - 20, w / 2 + 250, h / 2 + 20, 5,
            RGBAColor(g_GreyColor));
        GME_WriteTTF(renderer, font, name, g_WhiteColor, w / 2 - 5, h / 2);
        SDL_RenderPresent(renderer);
    }
    SDL_StopTextInput();
    SDL_DestroyTexture(icon_texture);
    SDL_FreeSurface(icon);
    TTF_CloseFont(font);
    int player_cnt = MAX_PLAYER_CNT;
    for (int i = 0; i < MAX_PLAYER_CNT; i++) {
        if (g_Players[i] != NULL && !strcmp(g_Players[i]->name, name + 1)) {
            g_CurPlayer = g_Players[i];
            break;
        }
        if (g_Players[i] == NULL) {
            player_cnt = i;
            break;
        }
    }
    if (g_CurPlayer == NULL) {
        if (player_cnt >= MAX_PLAYER_CNT) {
            LogInfo("Maximum number of players reached");
            return 1;
        }
        g_CurPlayer = ELE_CreatePlayer(player_cnt, name + 1, g_PlayerColors[player_cnt], 0);
        g_Players[player_cnt] = g_CurPlayer;
    }
    LogInfo("Player id %d", g_CurPlayer->id);
    return 0;
}

Player* GME_GetPlayerById(int id) {
    int player_cnt = GME_GetPlayerCnt();
    for (int i = 0; i < player_cnt; i++) {
        if (g_Players[i]->id == id) {
            return g_Players[i];
        }
    }
    return NULL;
}

Area* GME_GetAreaById(int id) {
    int area_cnt = GME_GetAreaCnt();
    for (int i = 0; i < area_cnt; i++) {
        if (g_Areas[i]->id == id) {
            return g_Areas[i];
        }
    }
    return NULL;
}

int GME_MapStart(Map *map) {
    LogInfo("Starting map...");
    if (map == NULL) {
        GME_BuildRandMap();
        Player *players[5];
        players[0] = g_Players[0];
        players[1] = g_Players[1];
        players[2] = g_Players[2];
        players[3] = g_Players[3];
        players[4] = g_CurPlayer;
        int area_cnt = MAX_AREA_CNT;
        for (int i = 0; i < MAX_AREA_CNT; i++) {
            if (g_Areas[i] == NULL) {
                area_cnt = i;
                break;
            }
        }
        g_CurMap = ELE_CreateMap(map_cnt, players, 5, g_Areas, area_cnt);
        for (int i = 0; i < g_CurMap->player_cnt; i++) {
            int start_area = rand() % g_CurMap->area_cnt;
            for (int i = 0; i < 20; i++) {
                if (g_CurMap->areas[start_area]->conqueror == NULL) break;
                start_area = rand() % g_CurMap->area_cnt;
            }
            if (g_CurMap->areas[start_area]->conqueror != NULL) return -1;
            ELE_AreaConquer(g_CurMap->areas[start_area], g_CurMap->players[i]);
        }
        g_Potions = malloc(sizeof(Potion*));
        potion_cnt = 0;
        potion_size = 1;
    } else {
        g_CurMap = map;
        if (map->players == NULL) {
            map->player_cnt = 5;
            map->players = malloc(sizeof(Player*) * map->player_cnt);
            for (int i = 0; i < map->player_cnt - 1; i++) {
                map->players[i] = g_Players[i];
            }
            map->players[map->player_cnt - 1] = g_CurPlayer;
            for (int i = 0; i < map->area_cnt; i++) {
                map->areas[i]->troop_cnt = 30;
                map->areas[i]->troop_inc_delay = 0;
                map->areas[i]->troop_rate = 60;
            }
            for (int i = 0; i < map->player_cnt; i++) {
                map->players[i]->applied_potion = NULL;
                map->players[i]->area_cnt = 0;
                map->players[i]->attack_delay = 0;
                map->players[i]->troop_cnt = 0;
                map->players[i]->troop_rate = 60;
                int start_area = rand() % map->area_cnt;
                for (int i = 0; i < 20; i++) {
                    if (map->areas[start_area]->conqueror == NULL) break;
                    start_area = rand() % map->area_cnt;
                }
                if (map->areas[start_area]->conqueror != NULL) return -1;
                ELE_AreaConquer(map->areas[start_area], map->players[i]);
            }
        }
        if (g_Potions == NULL) {
            g_Potions = malloc(sizeof(Potion*));
            potion_cnt = 0;
            potion_size = 1;
        }
    }
    SDL_Surface *surf;
    SDL_Renderer *renderer = VDO_GetRenderer();
    surf = IMG_Load("bin/images/TroopSpeedX2.png");
    g_PotionTextures[0] = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    surf = IMG_Load("bin/images/TroopFreezeOthers.png");
    g_PotionTextures[1] = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    surf = IMG_Load("bin/images/AreaBeyondCapacity.png");
    g_PotionTextures[2] = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    surf = IMG_Load("bin/images/AreaShield.png");
    g_PotionTextures[3] = SDL_CreateTextureFromSurface(renderer, surf);
    SDL_FreeSurface(surf);
    LogInfo("Map id %d", g_CurMap->id);
    return GME_RenderGame();
}

void GME_MapQuit(Map *map) {
    if (g_Potions != NULL) {
        for (int i = 0; i < potion_cnt; i++) {
            if (g_Potions[i] != NULL) {
                ELE_DestroyPotion(g_Potions[i]);
            }
        }
    }
    free(g_Potions);
    g_Potions = NULL;
    for (int i = 0; i < map->player_cnt; i++) {
        if (map->players[i]->applied_potion != NULL) {
            ELE_DestroyPotion(map->players[i]->applied_potion);
        }
    }
    for (int i = 0; i < 4; i++) {
        if (g_PotionTextures[i] == NULL) continue;
        SDL_DestroyTexture(g_PotionTextures[i]);
        g_PotionTextures[i] = NULL;
    }
    potion_cnt = 0;
    potion_size = 0;
    ELE_DestroyMap(map);
}

int GME_RetrievePlayers() {
    memset(g_Players, 0, sizeof(g_Players));
    const char *players_filename = "bin/data/players.bin";
    SDL_RWops *players_file = SDL_RWFromFile(players_filename, "rb");
    Player players[MAX_PLAYER_CNT] = {0};
    if (players_file == NULL) {
        LogInfo("Unable to open players data, going with the defaults");
        // Default Players
        g_Players[0] = ELE_CreatePlayer(0, "ArshiA", g_PlayerColors[0], 0);
        g_Players[1] = ELE_CreatePlayer(1, "AArshiAA", g_PlayerColors[1], 0);
        g_Players[2] = ELE_CreatePlayer(2, "AAArshiAAA", g_PlayerColors[2], 0);
        g_Players[3] = ELE_CreatePlayer(3, "IArshiAI", g_PlayerColors[3], 0);
    } else {
        SDL_RWread(players_file, players, sizeof(Player), MAX_PLAYER_CNT);
        SDL_RWclose(players_file);
        for (int i = 0; i < MAX_PLAYER_CNT; i++) {
            if (players[i].name[0] == 0) break;
            g_Players[i] = ELE_CreatePlayer(
                players[i].id, players[i].name, players[i].color,
                players[i].score);
        }
    }
    LogInfo("Player Retrieve Done");
    return 0;
}

int GME_RetrieveMap(int id) {
    char filename[24];
    if (id == -1)
        sprintf(filename, "bin/data/lastmap.bin");
    else
        sprintf(filename, "bin/data/map%d.bin", id);
    SDL_RWops *file = SDL_RWFromFile(filename, "rb");
    if (file == NULL && id != -1) {
        sprintf(filename, "bin/data/map%d", id);
        file = SDL_RWFromFile(filename, "rb");
    }
    if (file != NULL) {
        int mapid;
        SDL_RWread(file, &mapid, sizeof(int), 1);
        Map *map = ELE_CreateMap(mapid, NULL, 0, NULL, 0);
        if (id == -1) {
            SDL_RWread(file, &map->player_cnt, sizeof(int), 1);
            map->players = malloc(sizeof(Player*) * map->player_cnt);
            for (int i = 0; i < map->player_cnt; i++) {
                int player_id;
                SDL_RWread(file, &player_id, sizeof(int), 1);
                map->players[i] = GME_GetPlayerById(player_id);
                SDL_RWread(file, &map->players[i]->area_cnt, sizeof(int), 1);
                SDL_RWread(file, &map->players[i]->troop_cnt, sizeof(int), 1);
                SDL_RWread(file, &map->players[i]->troop_rate, sizeof(int), 1);
                SDL_RWread(file, &map->players[i]->attack_delay, sizeof(int), 1);
                map->players[i]->area_cnt = 0;
                map->players[i]->troop_cnt = 0;
                int haspt;
                SDL_RWread(file, &haspt, sizeof(int), 1);
                if (haspt) {
                    Potion pt;
                    SDL_RWread(file, &pt, sizeof(Potion), 1);
                    map->players[i]->applied_potion = ELE_CreatePotion(
                        pt.id, pt.type, pt.frames_onmap, pt.frames_applied, pt.center
                    );
                }
            }
            SDL_RWread(file, &potion_cnt, sizeof(int), 1);
            potion_size = potion_cnt + 1;
            g_Potions = malloc(sizeof(Potion*) * potion_size);
            for (int i = 0; i < potion_cnt; i++) {
                Potion pt;
                SDL_RWread(file, &pt, sizeof(Potion), 1);
                if (pt.frames_onmap > 0) {
                    g_Potions[i] = ELE_CreatePotion(
                        pt.id, pt.type, pt.frames_onmap, pt.frames_applied, pt.center
                    );
                } else g_Potions[i] = NULL;
            }
        }
        SDL_RWread(file, &map->area_cnt, sizeof(int), 1);
        map->areas = malloc(sizeof(Area*) * map->area_cnt);
        for (int i = 0; i < map->area_cnt; i++) {
            int area_id;
            SDL_RWread(file, &area_id, sizeof(int), 1);
            int conq_id;
            SDL_RWread(file, &conq_id, sizeof(int), 1);
            int area_cap, area_tcnt, area_trate, area_tincdelay;
            SDL_RWread(file, &area_cap, sizeof(int), 1);
            SDL_RWread(file, &area_tcnt, sizeof(int), 1);
            SDL_RWread(file, &area_trate, sizeof(int), 1);
            SDL_RWread(file, &area_tincdelay, sizeof(int), 1);
            SDL_Point area_center;
            SDL_RWread(file, &area_center, sizeof(SDL_Point), 1);
            int area_radius;
            SDL_RWread(file, &area_radius, sizeof(int), 1);
            Player *player = GME_GetPlayerById(conq_id);
            Area *area = ELE_CreateArea(
                area_id, (id == -1 ? player : NULL),
                ELE_GetAreaCapacityByRadius(area_radius), area_tcnt,
                area_trate, area_center, area_radius, NULL, 0
            );
            if (player) player->area_cnt++;
            int vertex_cnt;
            SDL_RWread(file, &vertex_cnt, sizeof(int), 1);
            area->vertex_cnt = vertex_cnt;
            area->vertices = malloc(sizeof(SDL_Point) * vertex_cnt);
            for (int i = 0; i < vertex_cnt; i++) {
                SDL_RWread(file, &area->vertices[i], sizeof(SDL_Point), 1);
            }
            map->areas[i] = area;
        }
        if (id == -1) {
            for (int i = 0; i < map->area_cnt; i++) {
                int att_id;
                SDL_RWread(file, &att_id, sizeof(int), 1);
                map->areas[i]->attack = ELE_GetAreaById(map, att_id);
                SDL_RWread(file, &map->areas[i]->attack_delay, sizeof(int), 1);
                SDL_RWread(file, &map->areas[i]->attack_cnt, sizeof(int), 1);
            }
            int troop_cnt;
            SDL_RWread(file, &troop_cnt, sizeof(int), 1);
            for (int i = 0; i < troop_cnt; i++) {
                int troop_id;
                SDL_RWread(file, &troop_id, sizeof(int), 1);
                int player_id;
                SDL_RWread(file, &player_id, sizeof(int), 1);
                double x, y;
                SDL_RWread(file, &x, sizeof(double), 1);
                SDL_RWread(file, &y, sizeof(double), 1);
                int src_id, dst_id;
                SDL_RWread(file, &src_id, sizeof(int), 1);
                SDL_RWread(file, &dst_id, sizeof(int), 1);
                Player *player = GME_GetPlayerById(player_id);
                Troop *troop = ELE_CreateTroop(
                    troop_id, player, x, y,
                    ELE_GetAreaById(map, src_id), ELE_GetAreaById(map, dst_id),
                    NULL, NULL
                );
                ELE_AddTroopToMap(map, troop);
            }
        }
        g_CurMap = map;
        SDL_RWclose(file);
    } else {
        LogInfo("Unable to read map files");
        return -1;
    }
    LogInfo("Map file read successful");
    return 0;
}

void GME_BuildRandMap() {
    memset(g_Areas, 0, sizeof(g_Areas));
    int w, h;
    VDO_GetWindowSize(&w, &h);
    int wsqcnt = 21;
    int sqsize = w / wsqcnt;
    int hsqcnt = h / sqsize;
    int min_radius = sqsize;
    int max_radius = 2 * sqsize - 15;
    int radius = (min_radius + max_radius) / 2;
    int amp = (max_radius - min_radius) / 2;
    int area_cnt = 0;
    double PI = acos(-1);
    for (int i = 2; i < wsqcnt - 4; i += 3) {
        for (int j = 2; j < hsqcnt - 2; j += 3) {
            if (rand() % 5 == 0) continue;
            int wst = i * sqsize;
            int hst = j * sqsize;
            SDL_Point center;
            center.x = rand() % sqsize + wst;
            center.y = rand() % sqsize + hst;
            int vertex_cnt = 360;
            SDL_Point vertices[vertex_cnt];
            /* Generate vertices */
            int wave_cnt = 30;
            double amps[wave_cnt];
            double phases[wave_cnt];
            for (int vi = 0; vi < wave_cnt; vi++) {
                amps[vi] = rand() * 1.0 / RAND_MAX / pow(vi + 1, 1.5) * 2 * amp;
                phases[vi] = rand() * 1.0 / RAND_MAX * 2 * PI;
            }
            for (int vi = 0; vi < vertex_cnt; vi++) {
                double alpha = 2 * PI * vi / vertex_cnt;
                double cur_radius = radius;
                for (int vj = 0; vj < wave_cnt; vj++) {
                    cur_radius += amps[vj] * cos((vj + 1) * alpha + phases[vj]);
                }
                vertices[vi].x = center.x + cos(alpha) * cur_radius;
                vertices[vi].y = center.y + sin(alpha) * cur_radius;
            }
            Area *area = ELE_CreateArea(
                area_cnt,
                NULL,
                ELE_GetAreaCapacityByRadius(radius),
                30,
                30,
                center,
                radius,
                vertices,
                vertex_cnt
            );
            g_Areas[area_cnt++] = area;
        }
    }
    LogInfo("Random Area Generation Done");
}

int GME_GenerateTestArena() {
    int area_cnt;
    do {
        GME_BuildRandMap();
        area_cnt = GME_GetAreaCnt();
    } while (area_cnt < 12);
    int opp_area = rand() % area_cnt;
    Player *players[2];
    players[0] = g_Players[3];
    players[1] = g_CurPlayer;
    g_CurMap = ELE_CreateMap(map_cnt, players, 2, g_Areas, area_cnt);
    for (int i = 0; i < g_CurMap->area_cnt; i++) {
        if (i == opp_area) {
            ELE_AreaConquer(g_CurMap->areas[i], g_CurMap->players[0]);
        } else {
            ELE_AreaConquer(g_CurMap->areas[i], g_CurMap->players[1]);
        }
    }
    return 0;
}

SDL_Color GME_ChangeAlpha(SDL_Color color, Uint8 alpha) {
    return (SDL_Color){color.r, color.g, color.b, alpha};
}

void GME_Move(double x, double y, double size, int sx, int sy, int dx, int dy, double *nx, double *ny) {
    double PI = acos(-1), theta;
    if (sx != dx) {
        theta = atan(1.0 * (sy - dy) / (sx - dx));
    } else {
        theta = (sy < dy ? PI / 2 : -PI / 2);
    }
    if (sx > dx) theta += PI;
    double sinus = sin(theta);
    double cosinus = cos(theta);
    *nx = x + size * cosinus, *ny = y + size * sinus;
}

void GME_Move2(double x, double y, double size, double theta, double *nx, double *ny) {
    double sinus = sin(theta);
    double cosinus = cos(theta);
    *nx = x + size * cosinus, *ny = y + size * sinus;
}

int GME_PutRandomPotion() {
    int type = rand() % 4;
    SDL_Point center;
    int area_cnt = g_CurMap->area_cnt;
    SDL_assert(area_cnt > 0);
    int from = rand() % area_cnt;
    int to = rand() % area_cnt;
    Area *src = g_CurMap->areas[from], *dst = g_CurMap->areas[to];
    center.x = src->center.x; center.y = src->center.y;
    int size = rand() % SDL_max(abs(src->center.x - dst->center.x) + 1, abs(src->center.y - dst->center.y) + 1);
    double X, Y;
    GME_Move(center.x, center.y, size, src->center.x, src->center.y,
        dst->center.x, dst->center.y, &X, &Y);
    center.x = X; center.y = Y;
    g_Potions[potion_cnt] = ELE_CreatePotion(potion_cnt, type, 1500, 1500, center);
    ++potion_cnt;
    if (potion_cnt == potion_size) {
        potion_size *= 2;
        g_Potions = realloc(g_Potions, sizeof(Potion*) * potion_size);
    }
}

int GME_RenderGame() {
    LogInfo("Start Render Game");
    int quit = 0;
    int w, h;
    VDO_GetWindowSize(&w, &h);
    SDL_Renderer *renderer = VDO_GetRenderer();
    SDL_Event e;
    int back_btn_sz = 70;
    SDL_Rect back_btn = {30, h - 25 - back_btn_sz, back_btn_sz, back_btn_sz};
    int save_sz = 200;
    SDL_Rect save_btn = {back_btn.x + back_btn.w + 20, back_btn.y, save_sz, back_btn_sz};
    Map *map = g_CurMap;
    Player *player = g_CurPlayer;
    Area **areas = map->areas;
    Area *selected = NULL;
    TTF_Font *font = TTF_OpenFont("bin/fonts/SourceCodeProBold.ttf", 18);
    TTF_Font *font_big = TTF_OpenFont("bin/fonts/SourceCodePro.ttf", 24);
    int troop_cnt = 0;
    int frame = 0;
    int sdl_quit = 0;
    Player *winner = NULL;
    while (!quit) {
        // Check Win
        int players = 0;
        for (int i = 0; i < map->player_cnt; i++) {
            if (map->players[i]->troop_cnt + map->players[i]->area_cnt != 0) {
                winner = map->players[i];
                ++players;
            }
        }
        if (players == 1) {
            break;
        } else {
            winner = NULL;
        }
        ++frame;
        if (selected != NULL && selected->conqueror != g_CurPlayer) selected = NULL;
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = 1;
                sdl_quit = 1;
            } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                int x, y;
                SDL_GetMouseState(&x, &y);
                if (back_btn.x <= x && x <= back_btn.x + back_btn.w &&
                    back_btn.y <= y && y <= back_btn.y + back_btn.h) {
                    quit = 1;
                    break;
                }
                if (save_btn.x <= x && x <= save_btn.x + save_btn.w &&
                    save_btn.y <= y && y <= save_btn.y + save_btn.h) {
                    map->id = map_cnt++;
                    ELE_SaveMap(map, 0, NULL, 0);
                }
                for (int i = 0; i < map->area_cnt; i++) {
                    if (abs(x - areas[i]->center.x) + abs(y - areas[i]->center.y) < 25) {
                        if (selected == NULL && areas[i]->conqueror == player) {
                            selected = areas[i];
                        } else if (selected == areas[i]) {
                            selected = NULL;
                        } else if (selected != NULL) {
                            if (ELE_GetAreaAppliedPotionType(areas[i]) == AREA_SHIELD &&
                                areas[i]->conqueror != selected->conqueror)
                                break;
                            ELE_AreaAttack(selected, areas[i]);
                            selected = NULL;
                        }
                        break;
                    }
                }
            }
        }
        boxRGBA(renderer, 0, 0, w, h, RGBAColor(g_BackgroundColor));
        // Render Player names
        int freeze_is_applied = 0;
        for (int i = 0; i < map->player_cnt; i++) {
            Player *player = map->players[i];
            int x1 = w - 220, y1 = h - 90 - 80 * i;
            int x2 = w - 20, y2 = h - 20 - 80 * i;
            int in_game = (player->troop_cnt + player->area_cnt > 0);
            if (in_game && player->applied_potion != NULL) {
                roundedBoxRGBA(renderer, x1 - 5, y1 - 5, x2 + 5, y2 + 5, 10,
                    RGBAColor(g_PotionColors[player->applied_potion->type]));
                roundedBoxRGBA(renderer, x1, y1, x2, y2, 10, RGBAColor(g_BackgroundColor));
            }
            roundedBoxRGBA(renderer, x1, y1, x2, y2, 10,
                RGBAColor(GME_ChangeAlpha(g_GreyColor, (in_game ? 100 : 20))));
            GME_WriteTTF(renderer, font, player->name, GME_ChangeAlpha(g_LightBlackColor, (in_game ? 255 : 155)),
                (x1 + x2) / 2, y1 + 20);
            int width = x2 - x1 - 40;
            width = 1.0 * width * player->area_cnt / ELE_GetMapAreaCntSum(map);
            roundedBoxRGBA(renderer, x1 + 20, y2 - 25, x1 + 20 + width, y2 - 20, 2,
                RGBAColor(player->color));
            // Player applied potion
            if (player->applied_potion != NULL) {
                if (player->applied_potion->frames_applied <= 0) {
                    ELE_DestroyPotion(player->applied_potion);
                    player->applied_potion = NULL;
                } else {
                    --player->applied_potion->frames_applied;
                }
            }
            if (player->applied_potion != NULL) {
                freeze_is_applied |= (player->applied_potion->type == TROOP_FREEZE_OTHERS);
            }
        }
        // Render Areas
        for (int i = 0; i < map->area_cnt; i++) {
            if (areas[i]->troop_inc_delay > 0) --areas[i]->troop_inc_delay;
            if (frame % areas[i]->troop_rate == 0 /* && areas[i]->attack == NULL  */&&
                areas[i]->conqueror != NULL && areas[i]->troop_inc_delay == 0 &&
                (areas[i]->troop_cnt < areas[i]->capacity ||
                    (ELE_GetAreaAppliedPotionType(areas[i]) == AREA_BEYOND_CAPACITY))) {
                ++areas[i]->troop_cnt;
            }
            if (areas[i]->troop_cnt <= 0) {
                areas[i]->troop_cnt = 0;
                areas[i]->attack_cnt = 0;
            }
            if (areas[i]->attack_cnt == 0) ELE_AreaUnAttack(areas[i]);
            if (areas[i]->attack != NULL) {
                if (freeze_is_applied && ELE_GetAreaAppliedPotionType(areas[i]) != TROOP_FREEZE_OTHERS)
                    ;
                else if (areas[i]->attack_delay > 0) {
                    areas[i]->attack_delay -= (ELE_GetAreaAppliedPotionType(areas[i]) == TROOP_SPEED_X2 ? 2 : 1);
                }
                else if (areas[i]->attack_cnt > 0) {
                    for (int it = 0; it < 5; it++) {
                        if (areas[i]->attack_cnt == 0) {
                            ELE_AreaUnAttack(areas[i]);
                            break;
                        }
                        areas[i]->troop_cnt--;
                        areas[i]->attack_cnt--;
                        int sx = areas[i]->center.x, sy = areas[i]->center.y;
                        int dx = areas[i]->attack->center.x, dy = areas[i]->attack->center.y;
                        double x, y;
                        GME_Move(sx, sy, 10, sx, sy, dx, dy, &x, &y);
                        double vert_theta, PI = acos(-1);
                        if (sy != dy) {
                            vert_theta = SDL_atan(-1.0 * (sx - dx) / (sy - dy));
                        } else {
                            vert_theta = (sx > dx ? PI / 2 : -PI / 2);
                        }
                        if (sy < dy) vert_theta += PI;
                        if (it == 0) GME_Move2(x, y, 22, vert_theta, &x, &y);
                        if (it == 1) GME_Move2(x, y, 11, vert_theta, &x, &y);
                        if (it == 3) GME_Move2(x, y, 11, vert_theta + PI, &x, &y);
                        if (it == 4) GME_Move2(x, y, 22, vert_theta + PI, &x, &y);
                        Troop *troop = ELE_CreateTroop(troop_cnt++, areas[i]->conqueror, x, y, areas[i], areas[i]->attack, NULL, NULL);
                        ELE_AddTroopToMap(map, troop);
                    }
                    areas[i]->attack_delay = 25;
                }
            }
            int area_shield = ELE_GetAreaAppliedPotionType(areas[i]) == AREA_SHIELD;
            int beyond_cap = ELE_GetAreaAppliedPotionType(areas[i]) == AREA_BEYOND_CAPACITY;
            ELE_ColorArea(areas[i], (areas[i] == selected ? g_BlueColor :
                (area_shield ? g_PotionColors[AREA_SHIELD] : 
                (beyond_cap ? g_PotionColors[AREA_BEYOND_CAPACITY] : g_BackgroundColor))),
                (areas[i]->conqueror ? areas[i]->conqueror->color : g_GreyColor),
                (areas[i] == selected ? 5 :
                (area_shield | beyond_cap ? 4 : 2)));
            filledCircleRGBA(renderer, areas[i]->center.x, areas[i]->center.y, 16,
                245, 245, 245, 255);
        }
        for (int i = 0; i < map->area_cnt; i++) {
            char buffer[5];
            sprintf(buffer, "%d", areas[i]->troop_cnt);
            GME_WriteTTF(renderer, font, buffer, g_BlackColor,
                areas[i]->center.x, areas[i]->center.y + 25);
        }
        // Render Potion
        if (rand() % 2400 == 0) {
            GME_PutRandomPotion();
        }
        for (int i = 0; i < potion_cnt; i++) {
            if (g_Potions[i] != NULL) {
                if (g_Potions[i]->frames_onmap <= 0) {
                    ELE_DestroyPotion(g_Potions[i]);
                    g_Potions[i] = NULL;
                    continue;
                }
                --g_Potions[i]->frames_onmap;
                SDL_Texture *potion_texture = g_PotionTextures[g_Potions[i]->type];
                SDL_Point center = g_Potions[i]->center;
                int w, h;
                SDL_QueryTexture(potion_texture, NULL, NULL, &w, &h);
                SDL_Rect src = {0, 0, w, h};
                SDL_Rect dst = {center.x - 30, center.y - 30, 60, 60};
                SDL_RenderCopy(renderer, potion_texture, &src, &dst);
            }
        }
        // Render Troops
        for (Troop *troop = map->troops_head; troop != NULL; troop = troop->next) {
            double size = 0.5;
            if (troop->player->applied_potion != NULL &&
                troop->player->applied_potion->type == TROOP_SPEED_X2)
                size = 1;
            if (!freeze_is_applied ||
                (troop->player->applied_potion != NULL &&
                troop->player->applied_potion->type == TROOP_FREEZE_OTHERS))
                GME_Move(troop->x, troop->y, size, troop->sx, troop->sy, troop->dx, troop->dy, &troop->x, &troop->y);
            filledCircleRGBA(renderer, troop->x, troop->y, 6, RGBAColor(g_BackgroundColor));
            filledCircleRGBA(renderer, troop->x, troop->y, 5, RGBAColor(troop->player->color));
        }
        roundedBoxRGBA(renderer, back_btn.x, back_btn.y, back_btn.x + back_btn.w,
            back_btn.y + back_btn.h, 10, RGBAColor(g_GreyColor));
        filledTrigonRGBA(renderer, back_btn.x + 20, back_btn.y + back_btn.h / 2,
            back_btn.x + back_btn.w - 20, back_btn.y + 15,
            back_btn.x + back_btn.w - 20, back_btn.y + back_btn.h - 15,
            RGBAColor(g_BackgroundColor));
        roundedBoxRGBA(renderer, save_btn.x, save_btn.y, save_btn.x + save_btn.w,
            save_btn.y + save_btn.h, 10, RGBAColor(g_GreyColor));
        GME_WriteTTF(renderer, font_big, "Save Map", g_WhiteColor,
            save_btn.x + save_btn.w / 2, save_btn.y + save_btn.h / 2);
        ELE_HandleCollisions(map);
        for (int i = 0; i < potion_cnt; i++) {
            if (g_Potions[i] == NULL) continue;
            for (Troop *troop = map->troops_head; troop != NULL; troop = troop->next) {
                if (troop->player->applied_potion != NULL) continue;
                SDL_Point X = g_Potions[i]->center;
                SDL_Point Y = {troop->x, troop->y};
                if (abs(X.x - Y.x) + abs(X.y - Y.y) < 40) {
                    troop->player->applied_potion = g_Potions[i];
                    g_Potions[i] = NULL;
                    break;
                }
            }
        }
        // AI
        for (int i = 0; i < map->player_cnt; i++) {
            if (map->players[i] == g_CurPlayer || map->players[i]->area_cnt == 0) continue;
            if (map->players[i]->attack_delay) {
                --map->players[i]->attack_delay;
                continue;
            }
            if (rand() % 240) continue;
            Player *player = map->players[i];
            int from = rand() % player->area_cnt;
            int to = rand() % map->area_cnt;
            Area *src = NULL, *dst = NULL;
            for (int i = 0; i < map->area_cnt; i++) {
                if (map->areas[i]->conqueror == player) {
                    if (from == 0) {
                        src = map->areas[i];
                        break;
                    } else {
                        --from;
                    }
                }
            }
            dst = map->areas[to];
            for (int i = 0; i < 10; i++) {
                if (dst == src ||
                    (ELE_GetAreaAppliedPotionType(dst) == AREA_SHIELD &&
                    dst->conqueror != src->conqueror)) {
                    to = rand() % map->area_cnt;
                    dst = map->areas[to];
                } else {
                    break;
                }
            }
            ELE_AreaAttack(src, dst);
            player->attack_delay = 60;
        }
        SDL_RenderPresent(renderer);
    }
    LogInfo("Quiting game rendering");
    TTF_CloseFont(font);
    TTF_CloseFont(font_big);
    if (sdl_quit) return 1;
    if (winner == NULL) {
        ELE_SaveMap(map, 1, g_Potions, potion_cnt);
        return 0;
    }
    for (int i = 0; i < map->player_cnt; i++) {
        if (map->players[i] == winner) {
            map->players[i]->score += map->player_cnt - 1;
        } else {
            map->players[i]->score -= 1;
        }
    }
    quit = 0;
    sdl_quit = 0;
    font = TTF_OpenFont("bin/fonts/SourceCodeProBold.ttf", 28);
    while (!quit) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                quit = 1;
                sdl_quit = 1;
            } else if (e.type == SDL_MOUSEBUTTONDOWN) {
                int x, y;
                SDL_GetMouseState(&x, &y);
                if (back_btn.x <= x && x <= back_btn.x + back_btn.w &&
                    back_btn.y <= y && y <= back_btn.y + back_btn.h) {
                    quit = 1;
                    break;
                }
            }
        }
        boxRGBA(renderer, 0, 0, w, h, RGBAColor(g_BackgroundColor));
        char buffer[50];
        sprintf(buffer, "%s won the game", winner->name);
        GME_WriteTTF(renderer, font, buffer, g_BlackColor, w / 2, h / 2);
        roundedBoxRGBA(renderer, back_btn.x, back_btn.y, back_btn.x + back_btn.w,
            back_btn.y + back_btn.h, 10, RGBAColor(g_GreyColor));
        filledTrigonRGBA(renderer, back_btn.x + 20, back_btn.y + back_btn.h / 2,
            back_btn.x + back_btn.w - 20, back_btn.y + 15,
            back_btn.x + back_btn.w - 20, back_btn.y + back_btn.h - 15,
            RGBAColor(g_BackgroundColor));
        SDL_RenderPresent(renderer);
    }
    TTF_CloseFont(font);
    if (sdl_quit) return 1;
    return 0;
}