#ifndef _GAME_H
#define _GAME_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include "elems/map.h"

extern int GME_Init(void);
extern void GME_Quit(void);
extern int GME_Start(void);

extern int GME_Menu(void);

extern Map* GME_GetCurMap(void);
extern Player** GME_GetPlayers(void);
extern Area** GME_GetAreas(void);
extern int GME_GetPlayerCnt(void);
extern int GME_GetAreaCnt(void);

extern int GME_GetCurPlayer(void);

extern int GME_WriteTTF(SDL_Renderer *renderer, TTF_Font *font, const char *s, SDL_Color color,
        int x, int y);

extern int GME_MapStart(Map *map);
extern void GME_MapQuit(Map *map);

extern int GME_RetrievePlayers(void);
extern int GME_RetrieveMap(int id);

extern int GME_GenerateTestArena(void);
extern void GME_BuildRandMap(void);

extern int GME_RenderGame(void);

#endif /* _GAME_H */