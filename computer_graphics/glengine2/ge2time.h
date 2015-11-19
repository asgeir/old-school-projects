#pragma once

#include <stdint.h>

namespace ge2 {

class Time
{
public:
	static float deltaTime();
	static float totalSeconds();

	static void update();

private:
	static uint32_t oldTime;
	static uint32_t currentTime;
};

} // namespace ge2
