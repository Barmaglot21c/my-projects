#include "CApp.h"
#include "log/window_event_out.h"

CApp::CApp() {
    Running = true;
    MWindow = nullptr;
    //Surf_Display = nullptr;
    Texture_Grid = nullptr;
    Texture_X = nullptr;
    Texture_O = nullptr;
    Texture_Think = nullptr;
    Texture_WinLine = nullptr;
    Texture_WinLineX1 = nullptr;
    Texture_WinLineX2 = nullptr;
    Texture_Winner = nullptr;
    Texture_Loser = nullptr;
    MRenderer = nullptr;
    CurrentPlayer = 0;
    is_win = 0;
    field_xbias = 150;
}

int CApp::OnExecute() {
    if(OnInit() == false){
        return -1;
    }

    SDL_Event Event;

    while(Running){
        while(SDL_PollEvent(&Event)){ //Рекомендуют использовать SDL_WaitEvent
            OnEvent(&Event);
            window_event_out(&Event);
        }

        OnLoop();
        OnRender();
    }

    OnCleanup();
    system("pause");

    return 0;
}

void CApp::SetCell(int ID[2], int Type) {
    if(ID[0] < 0 || ID[0] >= 3) return;
    if(ID[1] < 0 || ID[1] >= 3) return;
    if(Type < 0 || Type > GRID_TYPE_O) return;
 
    Grid[ID[0]][ID[1]] = Type;
}

void CApp::Reset() {
    for(int i = 0; i<3; i++) {
        for(int j = 0; j<3; j++) Grid[i][j] = GRID_TYPE_NONE;
        is_win = 0;
        CurrentPlayer = 0;
    }
}

int main(int argc, char* argv[]){
    CApp theApp;
    return theApp.OnExecute();
}