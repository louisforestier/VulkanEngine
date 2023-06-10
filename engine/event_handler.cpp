#include "event_handler.h"

void SDLEventHandler::handleSDLEvent(const SDL_Event &event)
{
    switch (event.type)
	{
	case SDL_KEYDOWN:
	case SDL_KEYUP:
		_keyState[event.key.keysym.scancode] = event.key.state;
		break;
	case SDL_MOUSEBUTTONDOWN:
        _buttonState[event.button.button] = event.button.state;
		break;
	case SDL_MOUSEBUTTONUP:
        _buttonState[event.button.button] = event.button.state;
		break;
	case SDL_MOUSEMOTION:
		_xrel = event.motion.xrel;
		_yrel = event.motion.yrel;
		break;
	default:
		break;
	}
}

bool SDLEventHandler::isPressed(SDL_Scancode keyCode)
{
    return _keyState[keyCode] == SDL_PRESSED;
}

bool SDLEventHandler::isPressed(Uint8 buttonCode)
{
    return _buttonState[buttonCode] == SDL_PRESSED;
}

bool SDLEventHandler::isReleased(SDL_Scancode keyCode)
{
    return _keyState[keyCode] == SDL_RELEASED;
}

bool SDLEventHandler::isReleased(Uint8 buttonCode)
{
    return _buttonState[buttonCode] == SDL_RELEASED;
}
