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

        constexpr const char* LIGHTING_VERTEX_SHADER = R"glsl(
#version 330

in vec3 vertexPosition;
in vec2 vertexTexCoord;
in vec3 vertexNormal;
in vec4 vertexColor;

uniform mat4 mvp;
uniform mat4 matModel;

out vec3 fragPosition;
out vec2 fragTexCoord;
out vec4 fragColor;
out vec3 fragNormal;

void main()
{
    vec4 worldPosition = matModel * vec4(vertexPosition, 1.0);
    mat3 normalMatrix = transpose(inverse(mat3(matModel)));

    fragPosition = worldPosition.xyz;
    fragTexCoord = vertexTexCoord;
    fragColor = vertexColor;
    fragNormal = normalize(normalMatrix * vertexNormal);

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

uniform vec3 lightPos0;
uniform vec4 lightColor0;
uniform int lightEnabled0;

uniform vec3 lightPos1;
uniform vec4 lightColor1;
uniform int lightEnabled1;

out vec4 finalColor;

vec3 applyPointLight(vec3 normal, vec3 viewDir, vec3 position, vec4 color, int enabled)
{
    if (enabled == 0) return vec3(0.0);

    vec3 lightDir = normalize(position - fragPosition);
    float distanceToLight = length(position - fragPosition);
    float attenuation = 1.0 / (1.0 + 0.045 * distanceToLight + 0.010 * distanceToLight * distanceToLight);

    float diffuse = max(dot(normal, lightDir), 0.0);

    vec3 reflectDir = reflect(-lightDir, normal);
    float specular = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);

    return color.rgb * attenuation * (0.85 * diffuse + 0.35 * specular);
}

void main()
{
    vec4 texel = texture(texture0, fragTexCoord) * fragColor;
    if (texel.a < 0.10) discard;

    vec3 normal = normalize(fragNormal);
    vec3 viewDir = normalize(viewPos - fragPosition);

    vec3 lighting = ambient.rgb;
    lighting += applyPointLight(normal, viewDir, lightPos0, lightColor0, lightEnabled0);
    lighting += applyPointLight(normal, viewDir, lightPos1, lightColor1, lightEnabled1);

    finalColor = vec4(texel.rgb * lighting, texel.a);
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
        while (!WindowShouldClose()) {
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
        BeginShaderMode(m_lightingShader);
        model::drawCar(false);
        EndShaderMode();
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

        m_lightPosLoc[0] = GetShaderLocation(m_lightingShader, "lightPos0");
        m_lightColorLoc[0] = GetShaderLocation(m_lightingShader, "lightColor0");
        m_lightEnabledLoc[0] = GetShaderLocation(m_lightingShader, "lightEnabled0");

        m_lightPosLoc[1] = GetShaderLocation(m_lightingShader, "lightPos1");
        m_lightColorLoc[1] = GetShaderLocation(m_lightingShader, "lightColor1");
        m_lightEnabledLoc[1] = GetShaderLocation(m_lightingShader, "lightEnabled1");

        const float ambient[4] = {0.18f, 0.18f, 0.20f, 1.0f};
        SetShaderValue(m_lightingShader, m_ambientLoc, ambient, SHADER_UNIFORM_VEC4);

        for (int i = 0; i < LIGHT_COUNT; ++i) {
            setVector3Uniform(m_lightingShader, m_lightPosLoc[i], LIGHT_POSITIONS[i]);
            setColorUniform(m_lightingShader, m_lightColorLoc[i], LIGHT_COLORS[i]);
        }
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
    }

} // namespace raylibgl::app
