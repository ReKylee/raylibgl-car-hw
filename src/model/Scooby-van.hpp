#pragma once

#include <raylib.h>

/**
 * @file Scooby-van.hpp
 * @brief Public draw API for the Scooby-Doo "Mystery Machine" van model.
 *
 * The implementation is based on a glTF model made in Blockbench. All draw
 * calls take @c wire to switch between filled and wireframe rendering.
 * 
 * Link: https://sketchfab.com/3d-models/scooby-do-the-mystery-machine-256aaa1b6e44484c85894a0bcd654333
 */
namespace raylibgl::model {

	/// @brief Load the van's textures (side decal, glow sprite). Safe to call repeatedly.
	void LoadCarResources();
	/// @brief Release the van's textures.
	void UnloadCarResources();

	/// @brief Draw the chassis: the extruded-hull body (with bumpers) plus the roof bars.
	void drawChassis(bool wire);
	/// @brief Draw one road wheel at the local origin, axle along X.
	void drawWheel(bool wire);
	/// @brief Draw the whole van in a single pass (used for the wireframe path).
	void drawCar(bool wire);

	// Split draw for the lit/emissive/transparent passes (see Application::drawCarWithOptionalLighting).

	/// @brief Opaque, scene-lit parts (hull, wheels, emblem, housings, seats, decals).
	void drawCarBody(bool wire);
	/// @brief Emissive light cores (head/tail-light lenses); draw with the shader's emission term.
	void drawCarLights(bool wire);
	/// @brief Semi-transparent parts (aquarium, windshield, door glass); draw last.
	void drawCarGlass(bool wire);
	/// @brief Additive bloom halos around the lights; draw outside the lighting shader.
	void drawCarGlow(bool wire);

} // namespace raylibgl::model
