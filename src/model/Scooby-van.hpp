#pragma once

#include <raylib.h>

namespace raylibgl::model {

	void LoadCarResources();
	void UnloadCarResources();

	void drawChassis(bool wire);
	void drawWheel(bool wire);
	void drawCar(bool wire);

} // namespace raylibgl::model
