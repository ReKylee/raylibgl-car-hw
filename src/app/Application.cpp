#include "app/Application.hpp"

#include "model/Primitives.hpp"
#include "model/Scooby-van.hpp"

#include <raylib.h>
#include <raymath.h>
#include <rlgl.h>

namespace raylibgl::app {

    namespace {

        constexpr int TARGET_FPS = 60;
        constexpr int LIGHT_COUNT = 2;

        constexpr Vector3 LIGHT_POSITIONS[LIGHT_COUNT] = {
            {-3.5f, 4.0f, -3.0f},
            {3.0f, 3.2f, 2.5f},
        };

        constexpr Color LIGHT_COLORS[LIGHT_COUNT] = {
            Color{255, 236, 150, 255},
            Color{150, 205, 255, 255},
        };

        // Headlight spotlights, in the VAN's local frame (matching drawHeadlightLenses): one per
        // lamp, aimed forward (-Z) and slightly down so the cone lands on the ground ahead.
        constexpr int SPOT_COUNT = 2;
        constexpr Vector3 SPOT_LOCAL_POS[SPOT_COUNT] = {
            {-0.66f, -0.46f, -2.00f},
            {0.66f, -0.46f, -2.00f},
        };
        constexpr Vector3 SPOT_LOCAL_DIR = {0.0f, -0.4472f, -0.8944f};  // normalize(0,-0.5,-1)
        constexpr Color SPOT_COLOR = Color{255, 240, 205, 255};         // warm headlight beam

        constexpr const char* LIGHTING_VERTEX_SHADER = R"glsl(
#version 330

in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

uniform mat4 mvp;

out vec3 fragPosition;
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormal;

// We draw in rlgl IMMEDIATE MODE under rlPushMatrix, so rlgl bakes the model (trackball)
// transform straight into vertexPosition/vertexNormal -- they are ALREADY in world space.
// Use them directly: multiplying by matModel would apply the transform a second time and
// put the geometry in a different frame than the world-space lights (incl. the spotlights).
void main()
{
    fragPosition = vertexPosition;
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    fragNormal = normalize(vertexNormal);

    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
)glsl";

        constexpr const char* LIGHTING_FRAGMENT_SHADER = R"glsl(
#version 330

in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;

uniform sampler2D texture0;
uniform vec3 viewPos;
uniform vec4 ambient;

// Material-style emissive term (cf. MATERIAL_MAP_EMISSION): glowing parts ignore the
// scene lights and output their own colour. 0 = normal lit surface.
uniform float emission;

uniform vec3 lightPos0;
uniform vec4 lightColor0;
uniform int lightEnabled0;

uniform vec3 lightPos1;
uniform vec4 lightColor1;
uniform int lightEnabled1;

// Two headlight SPOTLIGHTS attached to the van (cone with soft edge + falloff).
uniform vec3 spotPos[2];
uniform vec3 spotDir[2];     // normalized beam direction (forward + down)
uniform vec3 spotColor[2];
uniform int spotEnabled;

out vec4 finalColor;

vec3 applySpot(int i, vec3 normal)
{
    vec3 toFrag = fragPosition - spotPos[i];
    float dist = length(toFrag);
    vec3 dir = toFrag / max(dist, 0.0001);

    // Cone: full inside ~20deg, fading to 0 by ~38deg off the beam axis.
    float cosA = dot(dir, normalize(spotDir[i]));
    float inner = 0.94;
    float outer = 0.79;
    float cone = clamp((cosA - outer) / (inner - outer), 0.0, 1.0);
    if (cone <= 0.0) return vec3(0.0);

    float attenuation = 1.0 / (1.0 + 0.07 * dist + 0.03 * dist * dist);
    float diffuse = max(dot(normal, -dir), 0.0);
    return spotColor[i] * (cone * cone) * attenuation * diffuse * 2.6;
}

vec3 applyPointLight(vec3 normal, vec3 viewDir, vec3 position, vec4 color, int enabled)
{
    if (enabled == 0) return vec3(0.0);

    vec3 lightDir = normalize(position - fragPosition);
    float distanceToLight = length(position - fragPosition);
    // Gentle attenuation so both lights comfortably reach the model.
    float attenuation = 1.0 / (1.0 + 0.022 * distanceToLight + 0.0019 * distanceToLight * distanceToLight);

    float diffuse = max(dot(normal, lightDir), 0.0);

    // Blinn-Phong specular, only on lit-facing surfaces (no specular leak on the dark side).
    vec3 halfDir = normalize(lightDir + viewDir);
    float specular = pow(max(dot(normal, halfDir), 0.0), 48.0) * step(0.0001, diffuse);

    return color.rgb * attenuation * (diffuse + 0.25 * specular);
}

void main()
{
    vec4 texel = texture(texture0, fragTexCoord) * fragColor;
    if (texel.a < 0.10) discard;

    vec3 normal = normalize(fragNormal);
    vec3 viewDir = normalize(viewPos - fragPosition);

    // Soft hemispheric ambient: cooler light from the sky above, warmer bounce from below.
    float hemi = normal.y * 0.5 + 0.5;
    vec3 ambientCol = mix(ambient.rgb * vec3(1.10, 0.98, 0.86),   // ground bounce (warm)
                          ambient.rgb * vec3(0.86, 0.97, 1.18),   // sky (cool)
                          hemi);

    vec3 lighting = ambientCol;
    lighting += applyPointLight(normal, viewDir, lightPos0, lightColor0, lightEnabled0);
    lighting += applyPointLight(normal, viewDir, lightPos1, lightColor1, lightEnabled1);

    // Subtle cool fresnel rim to lift the silhouette and give the flats some life.
    float fresnel = pow(1.0 - max(dot(normal, viewDir), 0.0), 4.0);
    lighting += fresnel * vec3(0.35, 0.45, 0.60) * 0.45;

    // Headlight spotlights (cast a warm pool on the ground in front of the van).
    if (spotEnabled == 1) {
        lighting += applySpot(0, normal);
        lighting += applySpot(1, normal);
    }

    // Emissive material: add the surface's own colour on top of (independent of) lighting.
    vec3 rgb = texel.rgb * lighting + texel.rgb * emission;
    finalColor = vec4(rgb, texel.a);
}
)glsl";

        void setVector3Uniform(Shader shader, int loc, Vector3 value) {
            const float data[3] = {value.x, value.y, value.z};
            SetShaderValue(shader, loc, data, SHADER_UNIFORM_VEC3);
        }

        void setColorUniform(Shader shader, int loc, Color color) {
            const float data[4] = {
                static_cast<float>(color.r) / 255.0f,
                static_cast<float>(color.g) / 255.0f,
                static_cast<float>(color.b) / 255.0f,
                static_cast<float>(color.a) / 255.0f,
            };
            SetShaderValue(shader, loc, data, SHADER_UNIFORM_VEC4);
        }

    } // namespace

    Application::Application(int width, int height, const char* title) :
            m_width(width),
            m_height(height),
            m_title(title) {
        SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_MSAA_4X_HINT);

        InitWindow(m_width, m_height, m_title);
        SetExitKey(KEY_F10);
        SetTargetFPS(TARGET_FPS);

        // Exercise requirement: back-face culling remains enabled.
        rlEnableBackfaceCulling();
        rlEnableDepthTest();

        model::LoadCarResources();
        loadLightingShader();
    }

    Application::~Application() {
        unloadLightingShader();
        model::UnloadCarResources();
        CloseWindow();
    }

    void Application::run() {
        while (!WindowShouldClose() && !m_debugState.exitRequested) {
            update();
            render();
        }
    }

    void Application::update() {
        m_width = GetScreenWidth();
        m_height = GetScreenHeight();

        debug::Update(m_debugState, m_camera);
        m_camera.Update(m_width, m_height, !debug::WantsMouseCapture(m_debugState));
    }

    void Application::render() {
        BeginDrawing();
        ClearBackground(Color{10, 10, 12, 255});

        BeginMode3D(m_camera.GetCamera());
        drawSceneRlgl();
        EndMode3D();

        debug::DrawOverlay(m_debugState, m_camera);

        EndDrawing();
    }

    void Application::drawSceneRlgl() {
        if (m_debugState.showGrid) {
            DrawGrid(20, 1.0f);
        }

        if (m_debugState.showAxes && !m_debugState.axesFollowModel) {
            model::drawAxes(2.5f);
        }

        if (m_debugState.showLightMarkers) {
            drawLightMarkers();
        }

        rlPushMatrix();
        {
            rlMultMatrixf(MatrixToFloat(m_camera.GetRotation()));

            if (m_debugState.showAxes && m_debugState.axesFollowModel) {
                model::drawAxes(2.5f);
            }

            drawCarWithOptionalLighting();
        }
        rlPopMatrix();
    }

    void Application::drawCarWithOptionalLighting() {
        if (m_debugState.wireframe || m_lightingShader.id == 0) {
            model::drawCar(m_debugState.wireframe);
            return;
        }

        updateLightingShader();

        const float emissiveOn = 1.4f;   // head/tail light cores: bright emissive material
        const float emissiveOff = 0.0f;

        BeginShaderMode(m_lightingShader);
        {
            // 1) Opaque body, lit by the scene lights (emission off).
            SetShaderValue(m_lightingShader, m_emissionLoc, &emissiveOff, SHADER_UNIFORM_FLOAT);
            model::drawCarBody(false);
            rlDrawRenderBatchActive();  // flush so the emission change below doesn't affect the body

            // 2) Head/tail light cores as an EMISSIVE material (MATERIAL_MAP_EMISSION-style):
            //    they output their own colour on top of the lighting, so they read as lit lamps.
            SetShaderValue(m_lightingShader, m_emissionLoc, &emissiveOn, SHADER_UNIFORM_FLOAT);
            model::drawCarLights(false);
            rlDrawRenderBatchActive();

            // 3) Semi-transparent windshield, drawn last (emission off) so alpha blends over the body.
            SetShaderValue(m_lightingShader, m_emissionLoc, &emissiveOff, SHADER_UNIFORM_FLOAT);
            model::drawCarGlass(false);
        }
        EndShaderMode();

        // 4) Additive bloom halos around the lights (not a surface material -- a glow effect).
        model::drawCarGlow(false);
    }

    void Application::drawLightMarkers() {
        for (int i = 0; i < LIGHT_COUNT; ++i) {
            DrawSphereEx(LIGHT_POSITIONS[i], 0.13f, 12, 12, LIGHT_COLORS[i]);
            DrawSphereWires(LIGHT_POSITIONS[i], 0.20f, 8, 8, ColorAlpha(LIGHT_COLORS[i], 0.45f));
        }
    }

    void Application::loadLightingShader() {
        m_lightingShader = LoadShaderFromMemory(LIGHTING_VERTEX_SHADER, LIGHTING_FRAGMENT_SHADER);

        m_viewPosLoc = GetShaderLocation(m_lightingShader, "viewPos");
        m_ambientLoc = GetShaderLocation(m_lightingShader, "ambient");
        m_emissionLoc = GetShaderLocation(m_lightingShader, "emission");

        m_lightPosLoc[0] = GetShaderLocation(m_lightingShader, "lightPos0");
        m_lightColorLoc[0] = GetShaderLocation(m_lightingShader, "lightColor0");
        m_lightEnabledLoc[0] = GetShaderLocation(m_lightingShader, "lightEnabled0");

        m_lightPosLoc[1] = GetShaderLocation(m_lightingShader, "lightPos1");
        m_lightColorLoc[1] = GetShaderLocation(m_lightingShader, "lightColor1");
        m_lightEnabledLoc[1] = GetShaderLocation(m_lightingShader, "lightEnabled1");

        m_spotPosLoc = GetShaderLocation(m_lightingShader, "spotPos");
        m_spotDirLoc = GetShaderLocation(m_lightingShader, "spotDir");
        m_spotColorLoc = GetShaderLocation(m_lightingShader, "spotColor");
        m_spotEnabledLoc = GetShaderLocation(m_lightingShader, "spotEnabled");

        const float ambient[4] = {0.30f, 0.31f, 0.34f, 1.0f};
        SetShaderValue(m_lightingShader, m_ambientLoc, ambient, SHADER_UNIFORM_VEC4);

        const float emission = 0.0f;  // default: surfaces are lit, not emissive
        SetShaderValue(m_lightingShader, m_emissionLoc, &emission, SHADER_UNIFORM_FLOAT);

        for (int i = 0; i < LIGHT_COUNT; ++i) {
            setVector3Uniform(m_lightingShader, m_lightPosLoc[i], LIGHT_POSITIONS[i]);
            setColorUniform(m_lightingShader, m_lightColorLoc[i], LIGHT_COLORS[i]);
        }

        // Spotlight colours + enable are constant; positions/directions are updated per frame.
        const float spotColors[SPOT_COUNT * 3] = {
            SPOT_COLOR.r / 255.0f, SPOT_COLOR.g / 255.0f, SPOT_COLOR.b / 255.0f,
            SPOT_COLOR.r / 255.0f, SPOT_COLOR.g / 255.0f, SPOT_COLOR.b / 255.0f,
        };
        SetShaderValueV(m_lightingShader, m_spotColorLoc, spotColors, SHADER_UNIFORM_VEC3, SPOT_COUNT);
        const int spotOn = 1;
        SetShaderValue(m_lightingShader, m_spotEnabledLoc, &spotOn, SHADER_UNIFORM_INT);
    }

    void Application::unloadLightingShader() {
        if (m_lightingShader.id != 0) {
            UnloadShader(m_lightingShader);
            m_lightingShader = {};
        }
    }

    void Application::updateLightingShader() {
        const Camera3D camera = m_camera.GetCamera();
        setVector3Uniform(m_lightingShader, m_viewPosLoc, camera.position);

        // The exercise requires two positional lights. The debug [L] toggle hides
        // their marker spheres, not the lighting itself, so the shaded model stays
        // useful while the UI is decluttered.
        for (int i = 0; i < LIGHT_COUNT; ++i) {
            const int enabled = 1;
            SetShaderValue(m_lightingShader, m_lightEnabledLoc[i], &enabled, SHADER_UNIFORM_INT);
            setVector3Uniform(m_lightingShader, m_lightPosLoc[i], LIGHT_POSITIONS[i]);
            setColorUniform(m_lightingShader, m_lightColorLoc[i], LIGHT_COLORS[i]);
        }

        // The headlight spotlights are attached to the van, so transform their local pos/dir by
        // the same trackball rotation the model is drawn with (see drawSceneRlgl).
        const Matrix rot = m_camera.GetRotation();
        float spotPos[SPOT_COUNT * 3];
        float spotDir[SPOT_COUNT * 3];
        for (int i = 0; i < SPOT_COUNT; ++i) {
            const Vector3 p = Vector3Transform(SPOT_LOCAL_POS[i], rot);
            const Vector3 d = Vector3Normalize(Vector3Transform(SPOT_LOCAL_DIR, rot));
            spotPos[i * 3 + 0] = p.x;
            spotPos[i * 3 + 1] = p.y;
            spotPos[i * 3 + 2] = p.z;
            spotDir[i * 3 + 0] = d.x;
            spotDir[i * 3 + 1] = d.y;
            spotDir[i * 3 + 2] = d.z;
        }
        SetShaderValueV(m_lightingShader, m_spotPosLoc, spotPos, SHADER_UNIFORM_VEC3, SPOT_COUNT);
        SetShaderValueV(m_lightingShader, m_spotDirLoc, spotDir, SHADER_UNIFORM_VEC3, SPOT_COUNT);
    }

} // namespace raylibgl::app
