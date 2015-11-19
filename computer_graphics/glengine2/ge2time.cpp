#include "ge2time.h"

#include "SDL.h"

using namespace ge2;

uint32_t Time::oldTime = 0;
uint32_t Time::currentTime = 0;

float Time::deltaTime()
{
	return (Time::currentTime - Time::oldTime) / 1000.0f;
}

float Time::totalSeconds()
{
	return (float)((double)Time::currentTime / 1000.0);
}

void Time::update()
{
	Time::oldTime = Time::currentTime;
	Time::currentTime = SDL_GetTicks();
}
