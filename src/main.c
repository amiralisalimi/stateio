#include "core/game.h"

int main() {
    if (GME_Init() != 0) {
        GME_Quit();
        return 1;
    }
    atexit(GME_Quit);
    GME_Start();
    return 0;
}