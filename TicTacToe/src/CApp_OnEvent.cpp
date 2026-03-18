#include "CApp.h"
 
void CApp::OnEvent(SDL_Event* Event){
    CEvent::OnEvent(Event);
}

void CApp::OnExit(){
    Running = false;
}

void CApp::OnLButtonDown(int mX, int mY){
    int ID[2] = {(mX-field_xbias) / 100, (mY / 100)};
 
    if((Grid[ID[0]][ID[1]] != GRID_TYPE_NONE) || (is_win != 0)){
        return;
    }
 
    if(CurrentPlayer == 0){
        SetCell(ID, GRID_TYPE_X);
        CurrentPlayer = 1;
    }
    else{
        SetCell(ID, GRID_TYPE_O);
        CurrentPlayer = 0;
    }

    for(int i=0; i<3; i++){
        if((Grid[0][i] == Grid[1][i]) && (Grid[1][i] == Grid[2][i]) && (Grid[0][i] != GRID_TYPE_NONE)){
            is_win = is_win | (1<<i);//0b00000001..0b00000100
        }
    }
    for(int i=0; i<3; i++){
        if((Grid[i][0] == Grid[i][1]) && (Grid[i][1] == Grid[i][2]) && (Grid[i][0] != GRID_TYPE_NONE)){
            is_win = is_win | (1<<(i+3));//0b00001000..0b00100000
        }
    }
    if ((Grid[0][0] == Grid[1][1]) && (Grid[1][1] == Grid[2][2]) && (Grid[0][0] != GRID_TYPE_NONE)){
        is_win = is_win | 0b01000000;
    }
    if ((Grid[0][2] == Grid[1][1]) && (Grid[1][1] == Grid[2][0]) && (Grid[0][2] != GRID_TYPE_NONE)){
        is_win = is_win | 0b10000000;
    }
}

void CApp::OnKeyDown(SDL_Keycode sym, Uint16 mod){
    switch (sym){
        case SDLK_r:{
            Reset();
            break;
        }
        default: break;
    }
}