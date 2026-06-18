#pragma once

#include <raylib.h>

namespace raylibgl::model {

	void LoadCarResources();
	void UnloadCarResources();

	void drawChassis(bool wire);
	void drawWheel(bool wire);
	void drawCar(bool wire);

	// Split draw for the lit/emissive/transparent passes (see Application::drawCarWithOptionalLighting).
	void drawCarBody(bool wire);   // opaque, lit
	void drawCarLights(bool wire); // light cores (draw with the shader's emissive term)
	void drawCarGlass(bool wire);  // semi-transparent windshield (draw last)
	void drawCarGlow(bool wire);   // additive bloom halos (draw outside the shader)

} // namespace raylibgl::model
