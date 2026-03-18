#include "CApp.h"
 
void CApp::OnCleanup() {
    //SDL_FreeSurface(Surf_Display);
    SDL_DestroyTexture(Texture_Grid);
    SDL_DestroyTexture(Texture_X);
    SDL_DestroyTexture(Texture_O);
    SDL_DestroyTexture(Texture_Think);
    SDL_DestroyTexture(Texture_Winner);
    SDL_DestroyTexture(Texture_Loser);
    SDL_DestroyTexture(Texture_WinLine);
    SDL_DestroyTexture(Texture_WinLineX1);
    SDL_DestroyTexture(Texture_WinLineX2);
    SDL_DestroyRenderer(MRenderer);
    SDL_DestroyWindow(MWindow);
    SDL_Quit();
}