#pragma once

#include <raylib.h>

/**
 * @file Primitives.hpp
 * @brief Small custom drawing helpers for the car model.
 *
 * Boxes wrap raylib's cube primitive directly; cylinders are emitted locally along +Y
 * with explicit normals so they shade correctly under the lighting shader. Callers use
 * rlgl matrices to place and rotate each part within the model hierarchy.
 */
namespace raylibgl::model {

    /**
     * @brief Draw the XYZ axes as three unlit coloured lines from the origin (+X red, +Y
     *        green, +Z blue).
     * @param length Length of each axis line, in model units.
     */
    void drawAxes(float length);

    /**
     * @brief Draw an axis-aligned box, filled or as wireframe edges.
     * @param center Box centre.
     * @param size   Full extents (x = width, y = height, z = depth).
     * @param color  Surface colour.
     * @param wire   Draw wireframe edges instead of filled faces.
     */
    void drawBox(Vector3 center, Vector3 size, Color color, bool wire);

    /**
     * @brief Draw a low-poly cylinder centred at @p center with its axis along local +Y.
     *
     * The filled path emits one normal per side for a deliberately faceted look. Callers
     * rotate the current matrix beforehand to aim the axis along X or Z.
     * @param center Cylinder centre.
     * @param radius Cross-section radius.
     * @param height Extent along the +Y axis.
     * @param color  Surface colour.
     * @param wire   Draw wireframe edges instead of filled faces.
     * @param sides  Number of radial facets (clamped to a minimum of 3).
     */
    void drawCylinder(Vector3 center, float radius, float height, Color color, bool wire, int sides = 12);

} // namespace raylibgl::model
