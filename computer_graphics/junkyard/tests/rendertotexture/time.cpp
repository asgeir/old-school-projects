#include "time.h"

#include "SDL.h"

uint32_t Time::oldTime = 0;
uint32_t Time::currentTime = 0;

float Time::deltaTime()
{
	return (Time::currentTime - Time::oldTime) / 1000.0f;
}

void Time::update()
{
	Time::oldTime = Time::currentTime;
    Time::currentTime = SDL_GetTicks();
}
