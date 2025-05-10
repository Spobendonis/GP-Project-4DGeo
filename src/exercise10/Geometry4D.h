#pragma once

#include <ituGL/application/Application.h>
#include <ituGL/renderer/Renderer.h>
#include <ituGL/camera/Camera.h>
#include <ituGL/camera/CameraController.h>
#include <ituGL/utils/DearImGui.h>
#include <glm/glm.hpp>

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

    ShaderProgram::Location m_worldRotationMatrixUniform;

    ShaderProgram::Location m_worldTranslationVectorUniform;

    ShaderProgram::Location m_worldScaleVectorUniform;

    ShaderProgram::Location m_viewProjMatrixUniform;

    ShaderProgram::Location m_colorUniform;

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
