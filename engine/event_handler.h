#pragma once

#include <unordered_map>
#include <SDL.h>
class SDLEventHandler
{
protected:
    std::unordered_map<SDL_Scancode,Uint8> _keyState;
    std::unordered_map<Uint8,Uint8> _buttonState;
    int _xrel;
    int _yrel;

public:
    SDLEventHandler() = default;
    virtual ~SDLEventHandler() = default;
    void handleSDLEvent(const SDL_Event& event);
    virtual void update(float deltaTime) = 0;
    bool isPressed(SDL_Scancode keyCode);
    bool isPressed(Uint8 buttonCode);
    bool isReleased(SDL_Scancode keyCode);
    bool isReleased(Uint8 buttonCode);
};

