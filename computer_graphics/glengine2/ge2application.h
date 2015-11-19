#pragma once

#include "SDL_events.h"

namespace ge2 {

class Application
{
public:
	virtual ~Application() {}

	virtual void handleEvent(const SDL_Event &event) = 0;
	virtual void update() = 0;
};

} // namespace ge2
