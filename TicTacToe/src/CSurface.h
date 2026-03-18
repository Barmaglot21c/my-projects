#ifndef _CSURFACE_H_
    #define _CSURFACE_H_
 
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
//#include "log/sendlog.h"
 
class CSurface {
    public:
        CSurface();
 
        static SDL_Surface* OnLoad(SDL_Window* window, const char* File);

        static bool OnDraw(SDL_Surface* Surf_Dest, SDL_Surface* Surf_src, int X, int Y);
        static bool OnDraw(SDL_Surface* Surf_Dest, SDL_Surface* Surf_Src, int X, int Y, int X2, int Y2, int W, int H);
        static bool Transparent(SDL_Surface* Surf_Dest, int R, int G, int B);

        static SDL_Texture* OnTextureLoad(SDL_Renderer* renderer, SDL_Surface* Surf);
        static SDL_Texture* OnTextureLoad(SDL_Window* window, SDL_Renderer* renderer, std::string path);
        static SDL_Texture* OnTextureLoad(SDL_Window* window, SDL_Renderer* renderer, std::string path, int R, int G, int B);

        static bool OnTextureDraw(SDL_Renderer* renderer, SDL_Texture* texture, int X, int Y);
        static bool OnTextureDraw(SDL_Renderer* renderer, SDL_Texture* texture, int X, int Y, int W, int H);
        static bool OnTextureDraw(SDL_Renderer* renderer, SDL_Texture* texture, int X, int Y, int X2, int Y2, int W2, int H2);
        static bool OnTextureDraw(SDL_Renderer* renderer, SDL_Texture* texture, int X, int Y, int W, int H, int X2, int Y2, int W2, int H2);
        static bool OnTextureDraw(SDL_Renderer* renderer, SDL_Texture* texture, int X, int Y, int W, int H, int X2, int Y2, int W2, int H2, SDL_RendererFlip flip);
        static bool OnTextureDraw(SDL_Renderer* renderer, SDL_Texture* texture, int X, int Y, int W, int H, int X2, int Y2, int W2, int H2, SDL_RendererFlip flip, 
         double angle, int aX, int aY);
};
 
#endif