#include "CApp.h"

bool CApp::OnInit() {
    
    if ((SDL_INIT_EVERYTHING) < 0){
        return false;
    }
    
    MWindow = SDL_CreateWindow("Tic Tac Toe v0.1", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 600, 300, 0/*SDL_WINDOW_RESIZABLE*/);
    if(MWindow == NULL){
        std::cout<<"Unable to init window"<<std::endl;
        return false;
    }

    if((MRenderer = SDL_CreateRenderer(MWindow, -1, SDL_RENDERER_ACCELERATED)) == NULL){
        std::cout<<"Renderer could not be created."<<std::endl<<"SDL Error: "<<SDL_GetError()<<std::endl;
        return false;
    }
    else SDL_SetRenderDrawColor(MRenderer, 0xFF, 0xFF, 0xFF, 0xFF);

    if ((Texture_Grid = CSurface::OnTextureLoad(MWindow, MRenderer, "img/grid.png")) == NULL){
        return false;
    }

    if ((Texture_X = CSurface::OnTextureLoad(MWindow, MRenderer, "img/xo.png", 220, 145, 255)) == NULL){
        return false;
    }
    if ((Texture_O = CSurface::OnTextureLoad(MWindow, MRenderer, "img/xo.png", 220, 145, 255)) == NULL){
        return false;
    }
    if ((Texture_Think = CSurface::OnTextureLoad(MWindow, MRenderer, "img/Thinking.png", 255, 0, 255)) == NULL){
        return false;
    }
    if ((Texture_WinLine = CSurface::OnTextureLoad(MWindow, MRenderer, "img/WinlineO.png", 255, 0, 255)) == NULL){
        return false;
    }
    if ((Texture_WinLineX1 = CSurface::OnTextureLoad(MWindow, MRenderer, "img/WinlineX1.png", 255, 0, 255)) == NULL){
        return false;
    }
    if ((Texture_WinLineX2 = CSurface::OnTextureLoad(MWindow, MRenderer, "img/WinlineX2.png", 255, 0, 255)) == NULL){
        return false;
    }
    if ((Texture_Winner = CSurface::OnTextureLoad(MWindow, MRenderer, "img/fkyeah.png", 255, 0, 255)) == NULL){
        return false;
    }
    if ((Texture_Loser = CSurface::OnTextureLoad(MWindow, MRenderer, "img/oh no.png", 255, 0, 255)) == NULL){
        return false;
    }

    Reset();

    return true;
}