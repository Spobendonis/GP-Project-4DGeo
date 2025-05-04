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
    Vertex(const glm::vec3& position) : position(position), normal(glm::vec3(0.0f)) {}
    Vertex(const glm::vec3& position, const glm::vec3& normal) : position(position), normal(normal) {}
    glm::vec3 position;
    glm::vec3 normal;
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

    void RenderGUI();
    void LoadAndCompileShader(Shader& shader, const char* path);
    void ComputeNormals(Vertex& v1, Vertex& v2, Vertex& v3);

private:
    DearImGui m_imGui;

    Mesh m_cube;

    ShaderProgram m_shaderProgram;

    ShaderProgram::Location m_worldMatrixUniform;

    ShaderProgram::Location m_viewProjMatrixUniform;

    ShaderProgram::Location m_colorUniform;

    Camera m_camera;
    
    CameraController m_cameraController;

    Color m_color;

    float m_xRotation;

    float m_yRotation;

    float m_zRotation;

    float m_rotationVelocities[3];

    float m_cubeCenter[3];
};
