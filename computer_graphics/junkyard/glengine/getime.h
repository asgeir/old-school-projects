#pragma once

#include <stdint.h>

class Time
{
public:
	static float deltaTime();

	static void update();

private:
	static uint32_t oldTime;
	static uint32_t currentTime;
};
