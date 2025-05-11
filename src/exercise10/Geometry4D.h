#pragma once

#include <ituGL/application/Application.h>
#include <ituGL/renderer/Renderer.h>
#include <ituGL/camera/Camera.h>
#include <ituGL/camera/CameraController.h>
#include <ituGL/utils/DearImGui.h>
#include <glm/glm.hpp>

struct Vertex
{
    Vertex() = default;
    Vertex(const glm::vec4& position) : position(position), normal(glm::vec4(0.0f)) {}
    Vertex(const glm::vec4& position, const glm::vec4& normal) : position(position), normal(normal) {}
    glm::vec4 position;
    glm::vec4 normal;
};

class Geometry4DApplication : public Application
{
public:
    Geometry4DApplication();

protected:
    void Initialize() override;
    void Update() override;
    void Render() override;
    void Cleanup() override;

private:
    void InitializeGeometry();
    void InitializeShaders();
    void InitializeCamera();
    void InitializeUniforms();

    void RenderGUI();
    void LoadAndCompileShader(Shader& shader, const char* path);
    void ResetState();

    // 4D Meshes
    void Create4DCube(Mesh* mesh, float size);
    void Create4DCubeWireframe(Mesh* mesh, float size);

    // 4D transformations
    glm::mat4 Rotate4D(float xy, float yz, float xz, float xw, float yw, float zw);

private:
    DearImGui m_imGui;

    Mesh m_cube;

    ShaderProgram m_shaderProgram;

    //Material Uniforms
    ShaderProgram::Location m_ambientReflectionUniform;

    ShaderProgram::Location m_diffuseReflectionUniform;

    ShaderProgram::Location m_specularReflectionUniform;

    ShaderProgram::Location m_specularExponentUniform;

    //Uniforms 
    ShaderProgram::Location m_colorUniform;

    ShaderProgram::Location m_ambientColorUniform;

    ShaderProgram::Location m_lightColorUniform;

    ShaderProgram::Location m_lightPositionUniform;

    ShaderProgram::Location m_cameraPositionUniform;

    ShaderProgram::Location m_worldRotationMatrixUniform;

    ShaderProgram::Location m_worldTranslationVectorUniform;

    ShaderProgram::Location m_worldScaleVectorUniform;

    ShaderProgram::Location m_viewProjMatrixUniform;

    Camera m_camera;
    
    CameraController m_cameraController;

    Color m_color;

    //Cube Parameters
    float m_scale;

    float m_xyRotation;

    float m_yzRotation;

    float m_xzRotation;

    float m_xwRotation;

    float m_ywRotation;

    float m_zwRotation;

    float m_rotationVelocities[6];

    float m_cubeCenter[4];
};
