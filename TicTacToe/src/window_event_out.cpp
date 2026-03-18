#include "log/window_event_out.h"

using namespace std;

string window_event_name[19] = {
    "SDL_WINDOWEVENT_NONE",           //< Never used 
    "SDL_WINDOWEVENT_SHOWN",          //< Window has been shown 
    "SDL_WINDOWEVENT_HIDDEN",         //< Window has been hidden 
    "SDL_WINDOWEVENT_EXPOSED",        //< Window has been exposed and should be redrawn                                    
    "SDL_WINDOWEVENT_MOVED",          //< Window has been moved to data1, data2                                 
    "SDL_WINDOWEVENT_RESIZED",        //< Window has been resized to data1xdata2 *
    "SDL_WINDOWEVENT_SIZE_CHANGED",   //< The window size has changed, either as  a result of an API call or through the system or user changing the window size.                                                                            
    "SDL_WINDOWEVENT_MINIMIZED",      //< Window has been minimized                                //окно свернуто
    "SDL_WINDOWEVENT_MAXIMIZED",      //< Window has been maximized                                //окно на весь экран
    "SDL_WINDOWEVENT_RESTORED",       //< Window has been restored to normal size and position     // Окно развернуто                                
    "SDL_WINDOWEVENT_ENTER",          //< Window has gained mouse focus                            //
    "SDL_WINDOWEVENT_LEAVE",          //< Window has lost mouse focus                              //
    "SDL_WINDOWEVENT_FOCUS_GAINED",   //< Window has gained keyboard focus                         //
    "SDL_WINDOWEVENT_FOCUS_LOST",     //< Window has lost keyboard focus                           //
    "SDL_WINDOWEVENT_CLOSE",          //< The window manager requests that the window be closed 
    "SDL_WINDOWEVENT_TAKE_FOCUS",     //< Window is being offered a focus (should SetWindowInputFocus() on itself or a subwindow, or ignore) 
    "SDL_WINDOWEVENT_HIT_TEST",       //< Window had a hit test that wasn't SDL_HITTEST_NORMAL. 
    "SDL_WINDOWEVENT_ICCPROF_CHANGED",//< The ICC profile of the window's display has changed. 
    "SDL_WINDOWEVENT_DISPLAY_CHANGED"
    };

void window_event_out (SDL_Event* Event){
    if (Event->type == SDL_WINDOWEVENT){
        cout<<"::"<<window_event_name[(int)Event->window.event];
        if (!(Event->window.event == SDL_WINDOWEVENT_RESIZED)) cout<<endl;
        else if(Event->window.event == SDL_WINDOWEVENT_RESIZED) cout<<" "<<Event->window.data1<<"x"<<Event->window.data2<<endl;
    }
}