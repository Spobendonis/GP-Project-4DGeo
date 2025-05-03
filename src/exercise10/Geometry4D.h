#pragma once

#include <ituGL/application/Application.h>

#include <ituGL/renderer/Renderer.h>
#include <ituGL/camera/Camera.h>
#include <ituGL/camera/CameraController.h>
#include <ituGL/utils/DearImGui.h>

class Material;

class Geometry4DApplication : public Application
{
public:
    Geometry4DApplication();

protected:
    void Initialize() override;
    void Update() override;
    void Render() override;

private:
    void InitializeGeometry();
    void InitializeShaders();
    void LoadAndCompileShader(Shader& shader, const char* path);

private:
    Mesh m_cube;

    ShaderProgram m_shaderProgram;

    ShaderProgram::Location m_worldMatrixUniform;

    ShaderProgram::Location m_viewProjMatrixUniform;

    ShaderProgram::Location m_colorUniform;

    Camera m_camera;
};
