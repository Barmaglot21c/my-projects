#include "CSurface.h"

CSurface::CSurface(){
}

SDL_Surface* CSurface::OnLoad(SDL_Window* window, const char* File){
    SDL_Surface* Surf_Temp = NULL;
    SDL_Surface* Surf_Return = NULL;

    if((Surf_Temp = IMG_Load(File)) == NULL){
        std::cout<<"Unable to load image "<<File<<std::endl<<"SDL_image Error: " << IMG_GetError()<<std::endl;
        return NULL;
    }

    Surf_Return = SDL_ConvertSurfaceFormat(Surf_Temp, SDL_GetWindowPixelFormat(window), 0);//Надо изучить по подробней
    SDL_FreeSurface(Surf_Temp);

    return Surf_Return;
}

bool CSurface::OnDraw(SDL_Surface* Surf_Dest, SDL_Surface* Surf_Src, int X, int Y){
    if(Surf_Dest == NULL | Surf_Src == NULL){
        return false;
    }

    SDL_Rect DestR;

    DestR.x = X;
    DestR.y = Y;

    SDL_BlitSurface(Surf_Src, NULL, Surf_Dest, &DestR);

    return true;
}

bool CSurface::OnDraw(SDL_Surface* Surf_Dest, SDL_Surface* Surf_Src, int X, int Y, int X2, int Y2, int W, int H) {
    if(Surf_Dest == NULL || Surf_Src == NULL) {
        return false;
    }
 
    SDL_Rect DestR;
 
    DestR.x = X;
    DestR.y = Y;
 
    SDL_Rect SrcR;
 
    SrcR.x = X2;
    SrcR.y = Y2;
    SrcR.w = W;
    SrcR.h = H;
 
    SDL_BlitSurface(Surf_Src, &SrcR, Surf_Dest, &DestR);
 
    return true;
}

bool CSurface::Transparent(SDL_Surface* Surf_Dest, int R, int G, int B){
    if(Surf_Dest == NULL) {
        return false;
    }
    SDL_SetColorKey(Surf_Dest, SDL_TRUE, SDL_MapRGB(Surf_Dest->format, R, G, B));
    return true;
};



SDL_Texture* CSurface::OnTextureLoad(SDL_Renderer* renderer, SDL_Surface* Surf){
    SDL_Texture* texture = NULL;
    texture = SDL_CreateTextureFromSurface(renderer, Surf);
    if(texture == NULL){
        std::cout<<"Unable to convert texture."<<std::endl<<"SDL_image Error: " << SDL_GetError()<<std::endl;
        return NULL;
    }
    return texture;
};

SDL_Texture* CSurface::OnTextureLoad(SDL_Window* window, SDL_Renderer* renderer, std::string path){
    SDL_Surface* surface = NULL;
    SDL_Texture* texture = NULL;

    surface = CSurface::OnLoad(window, path.c_str());

    texture = SDL_CreateTextureFromSurface(renderer, surface);//IMG_LoadTexture сразу текстуру клепает
    if(texture == NULL){
        std::cout<<"Unable to convert texture "<<path<<std::endl<<"SDL_image Error: " << SDL_GetError()<<std::endl;
        return NULL;
    }
    SDL_FreeSurface(surface);
    return texture;
};

SDL_Texture* CSurface::OnTextureLoad(SDL_Window* window, SDL_Renderer* renderer, std::string path, int R, int G, int B){
    SDL_Surface* surface = NULL;
    SDL_Texture* texture = NULL;

    surface = CSurface::OnLoad(window, path.c_str());
    CSurface::Transparent(surface, R, G, B);

    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if(texture == NULL){
        std::cout<<"Unable to convert texture "<<path<<std::endl<<"SDL_image Error: " << SDL_GetError()<<std::endl;
        return NULL;
    }
    SDL_FreeSurface(surface);
    return texture;
};

bool CSurface::OnTextureDraw(SDL_Renderer* renderer, SDL_Texture* texture, int X, int Y){
    if(renderer == NULL | texture == NULL){
        return false;
    }

    int W, H;
    SDL_QueryTexture(texture, NULL, NULL, &W, &H);

    SDL_Rect DestR;
    DestR.x = X;
    DestR.y = Y;
    DestR.w = W;
    DestR.h = H;

    SDL_RenderCopy(renderer, texture, NULL, &DestR);

    return true;
}

bool CSurface::OnTextureDraw(SDL_Renderer* renderer, SDL_Texture* texture, int X, int Y, int W, int H) {
    if(renderer == NULL | texture == NULL){
        return false;
    }
 
    SDL_Rect DestR;
    DestR.x = X;
    DestR.y = Y;
    DestR.w = W;
    DestR.h = H;
 
    SDL_RenderCopy(renderer, texture, NULL, &DestR);
 
    return true;
}

bool CSurface::OnTextureDraw(SDL_Renderer* renderer, SDL_Texture* texture, int X, int Y, int X2, int Y2, int W2, int H2){
    if(renderer == NULL | texture == NULL){
        return false;
    }

    SDL_Rect DestR;
    DestR.x = X;
    DestR.y = Y;
    DestR.w = W2;
    DestR.h = H2;

    SDL_Rect SrcR;
    SrcR.x = X2;
    SrcR.y = Y2;
    SrcR.w = W2;
    SrcR.h = H2;

    SDL_RenderCopy(renderer, texture, &SrcR, &DestR);

    return true;
}

bool CSurface::OnTextureDraw(SDL_Renderer* renderer, SDL_Texture* texture, int X, int Y, int W, int H, int X2, int Y2, int W2, int H2) {
    if(renderer == NULL | texture == NULL){
        return false;
    }
 
    SDL_Rect DestR;
    DestR.x = X;
    DestR.y = Y;
    DestR.w = W;
    DestR.h = H;
 
    SDL_Rect SrcR;
    SrcR.x = X2;
    SrcR.y = Y2;
    SrcR.w = W2;
    SrcR.h = H2;
 
    SDL_RenderCopy(renderer, texture, &SrcR, &DestR);
 
    return true;
}

bool CSurface::OnTextureDraw(SDL_Renderer* renderer, SDL_Texture* texture, int X, int Y, int W, int H, int X2, int Y2, int W2, int H2, SDL_RendererFlip flip){
    if(renderer == NULL | texture == NULL){
        return false;
    }
 
    SDL_Rect DestR;
    DestR.x = X;
    DestR.y = Y;
    DestR.w = W;
    DestR.h = H;
 
    SDL_Rect SrcR;
    SrcR.x = X2;
    SrcR.y = Y2;
    SrcR.w = W2;
    SrcR.h = H2;

    SDL_Point center;
    center.x = 0;
    center.y = 0;
 
    SDL_RenderCopyEx(renderer, texture, &SrcR, &DestR, 0, NULL, flip);
 
    return true;
}

bool CSurface::OnTextureDraw(SDL_Renderer* renderer, SDL_Texture* texture, int X, int Y, int W, int H, int X2, int Y2, int W2, int H2, SDL_RendererFlip flip, 
 double angle, int aX, int aY){
    if(renderer == NULL | texture == NULL){
        return false;
    }
 
    SDL_Rect DestR;
    DestR.x = X;
    DestR.y = Y;
    DestR.w = W;
    DestR.h = H;
 
    SDL_Rect SrcR;
    SrcR.x = X2;
    SrcR.y = Y2;
    SrcR.w = W2;
    SrcR.h = H2;

    SDL_Point center;
    center.x = aX;
    center.y = aY;
 
    SDL_RenderCopyEx(renderer, texture, &SrcR, &DestR, angle, &center, flip);
 
    return true;
 }