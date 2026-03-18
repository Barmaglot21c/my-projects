#ifndef _CAPP_H_
    #define _CAPP_H_
 
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "CEvent.h"
#include "CSurface.h"

#include <iostream>
#include <fstream>
#include <cmath>

//#include "log/sendlog.h" //в подпапке почему то не пашет
 
class CApp : public CEvent{
    private:
    bool Running;
    int Grid[3][3];
    int field_xbias;
    int CurrentPlayer;
    unsigned short is_win;

    SDL_Window* MWindow;
    //SDL_Surface* Surf_Display;
    SDL_Texture* Texture_Grid;
    SDL_Texture* Texture_X;
    SDL_Texture* Texture_O;
    SDL_Texture* Texture_Think;
    SDL_Texture* Texture_WinLine;
    SDL_Texture* Texture_WinLineX1;
    SDL_Texture* Texture_WinLineX2;
    SDL_Texture* Texture_Winner;
    SDL_Texture* Texture_Loser;
    SDL_Renderer* MRenderer;

    enum{
        GRID_TYPE_NONE = 0,
        GRID_TYPE_X,
        GRID_TYPE_O
    };

    public:
    CApp();
    void OnLButtonDown(int mX, int mY);
    void OnKeyDown(SDL_Keycode sym, Uint16 mod);

    int OnExecute();
    void SetCell(int ID[2], int Type);
    void Reset();

    bool OnInit();
    void OnEvent(SDL_Event* Event);
    void OnLoop();
    void OnRender();
    void OnCleanup();  

    void OnExit();
};

#endif