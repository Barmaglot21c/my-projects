#include "CApp.h"
 
void CApp::OnRender(){
    SDL_RenderClear(MRenderer);

    CSurface::OnTextureDraw(MRenderer, Texture_Grid, field_xbias, 0);
    
    for(int i = 0; i < 3; i++){
        for(int j = 0; j < 3; j++){
            int X = i*100+field_xbias;
            int Y = j*100;
 
            if(Grid[i][j] == GRID_TYPE_X){
                CSurface::OnTextureDraw(MRenderer, Texture_X, X, Y, 100, 0, 100, 100);
            }
            else if(Grid[i][j] == GRID_TYPE_O){
                CSurface::OnTextureDraw(MRenderer, Texture_O, X, Y, 0, 0, 100, 100);
            }
        }
    }
    
    if(is_win == 0){
        if (CurrentPlayer == 0) {
            CSurface::OnTextureDraw(MRenderer, Texture_Think, 0, 50, 150, 150*800/600, 0, 0, 600, 800, SDL_FLIP_HORIZONTAL);
            CSurface::OnTextureDraw(MRenderer, Texture_Think, field_xbias+300+15, 50+15, 120, 120*800/600);
        }
        else if (CurrentPlayer == 1) {
            CSurface::OnTextureDraw(MRenderer, Texture_Think, 0+15, 50+15, 120, 120*800/600, 0, 0, 600, 800, SDL_FLIP_HORIZONTAL);
            CSurface::OnTextureDraw(MRenderer, Texture_Think, field_xbias+300, 50, 150, 150*800/600);
        }
    }
    else if(CurrentPlayer == 1){
        CSurface::OnTextureDraw(MRenderer, Texture_Winner, 0, 75, 150, 150);
        CSurface::OnTextureDraw(MRenderer, Texture_Loser, field_xbias+300, 60, 150, 150*729/600);
    }
    else{
        CSurface::OnTextureDraw(MRenderer, Texture_Loser, 0, 60, 150, 150*729/600, 0, 0, 600, 729, SDL_FLIP_HORIZONTAL);
        CSurface::OnTextureDraw(MRenderer, Texture_Winner, field_xbias+300, 75, 150, 150, 0, 0, 1200, 1200, SDL_FLIP_HORIZONTAL);
    }

    if(is_win != 0) {
        for (short i = 0; i < 8; i++){
            short mask = 1<<i;
            if(is_win & mask){
                switch(i){
                    case 0:
                    case 1:
                    case 2: {
                        CSurface::OnTextureDraw(MRenderer, Texture_WinLine, field_xbias, i*100);
                        break;
                    }
                    case 3:
                    case 4:
                    case 5: {
                        CSurface::OnTextureDraw(MRenderer, Texture_WinLine, field_xbias+(i-3+1)*100, 0, 300, 100, 0, 0, 300, 100, SDL_FLIP_NONE, 90, 0, 0);
                        break;
                    }
                    case 6: {
                        CSurface::OnTextureDraw(MRenderer, Texture_WinLineX1, field_xbias, 0);
                        break;
                    }
                    case 7: {
                        CSurface::OnTextureDraw(MRenderer, Texture_WinLineX2, field_xbias, 0);
                        break;
                    }
                }
            }
        }
    }

    SDL_RenderPresent(MRenderer);
}