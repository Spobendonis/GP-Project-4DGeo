#include "Geometry4D.h"
#include <ituGL/shader/Shader.h>
#include <ituGL/shader/ShaderProgram.h>
#include <ituGL/geometry/VertexFormat.h>
#include <ituGL/camera/Camera.h>
#include <ituGL/scene/SceneCamera.h>
#include <ituGL/scene/Transform.h>
#include <ituGL/application/Window.h>
#include <imgui.h>
#include <cassert>
#include <array>
#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <vector>
#include <filesystem>
#include <time.h>
#include <glm/gtx/transform.hpp>
#include <glm/ext.hpp>

Geometry4DApplication::Geometry4DApplication()
    : Application(1024, 1024, "4D-Geometry demo")
    , m_colorUniform(-1)
    , m_ambientReflectionUniform(-1)
    , m_diffuseReflectionUniform(-1)
    , m_specularReflectionUniform(-1)
    , m_specularExponentUniform(-1)
    , m_ambientColorUniform(-1)
    , m_lightColorUniform(-1)
    , m_lightPositionUniform(-1)
    , m_cameraPositionUniform(-1)
    , m_worldRotationMatrixUniform(-1)
    , m_worldTranslationVectorUniform(-1)
    , m_worldScaleVectorUniform(-1)
    , m_viewProjMatrixUniform(-1)
    , m_rotationVelocities(0)
    , m_cubeCenter(0)
    , m_scale(1)
    , m_xyRotation(0)
    , m_xzRotation(0)
    , m_yzRotation(0)
    , m_xwRotation(0)
    , m_ywRotation(0)
    , m_zwRotation(0)
{
}


void Geometry4DApplication::Initialize()
{
    Application::Initialize();

    m_imGui.Initialize(GetMainWindow());

    InitializeGeometry();
    InitializeShaders();
    InitializeCamera();
    InitializeUniforms();

    m_cubeCenter[3] = 1;

    //Triangles should wind clock-wise
    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);
    glEnable(GL_DEPTH_TEST);
}

void Geometry4DApplication::Update()
{
    Application::Update();

    m_cameraController.Update(GetMainWindow(), GetDeltaTime());

    ResetState();

    m_camera = *m_cameraController.GetCamera()->GetCamera();
}

void Geometry4DApplication::Render()
{
    Application::Render();

    GetDevice().Clear(true, Color(0.0f, 0.0f, 0.0f, 1.0f), true, 1.0f);

    glm::vec4 white = glm::vec4(1.0f);
    glm::vec4 red = glm::vec4(1.0f, 0, 0, 0);

    float gap = (m_cubeCenter[3] * m_scale * 1.5);

    //rotating by the given amount
    //3D rotations
    m_xyRotation += m_rotationVelocities[0] / 1000;
    m_xzRotation += m_rotationVelocities[2] / 1000;
    m_yzRotation += m_rotationVelocities[1] / 1000;

    //4D rotations
    m_xwRotation += m_rotationVelocities[3] / 1000;
    m_ywRotation += m_rotationVelocities[4] / 1000;
    m_zwRotation += m_rotationVelocities[5] / 1000;

    glm::mat4 worldRotationMatrix =
        Rotate4D(m_xyRotation, m_xzRotation, m_yzRotation, m_xwRotation, m_ywRotation, m_zwRotation);

    glm::vec4 worldTranslationVector = glm::vec4(
        m_cubeCenter[0] - gap,
        m_cubeCenter[1],
        m_cubeCenter[2],
        m_cubeCenter[3]
    );


    glm::vec4 worldScaleVector = glm::vec4(glm::vec3(m_scale), 1);

    m_shaderProgram.Use();
    
    m_shaderProgram.SetUniform(m_ambientReflectionUniform, 1.0f);

    m_shaderProgram.SetUniform(m_diffuseReflectionUniform, 1.0f);
    
    m_shaderProgram.SetUniform(m_specularReflectionUniform, 1.0f);
    
    m_shaderProgram.SetUniform(m_specularExponentUniform, 100.0f);

    m_shaderProgram.SetUniform(m_viewProjMatrixUniform, m_camera.GetViewProjectionMatrix());
    
    m_shaderProgram.SetUniform(m_colorUniform, white);

    m_shaderProgram.SetUniform(m_ambientColorUniform, glm::vec3(0.35f)*glm::vec3(white));

    m_shaderProgram.SetUniform(m_lightColorUniform, glm::vec3(white));

    m_shaderProgram.SetUniform(m_lightPositionUniform, glm::vec3(1, 10, -1));

    m_shaderProgram.SetUniform(m_cameraPositionUniform, m_cameraController.GetCamera()->GetTransform()->GetTranslation());

    m_shaderProgram.SetUniform(m_worldRotationMatrixUniform, worldRotationMatrix);

    m_shaderProgram.SetUniform(m_worldTranslationVectorUniform, worldTranslationVector);

    m_shaderProgram.SetUniform(m_worldScaleVectorUniform, worldScaleVector);

    m_cube.DrawSubmesh(0);

    m_shaderProgram.SetUniform(m_colorUniform, red);

    m_cube.DrawSubmesh(1);

    worldTranslationVector = glm::vec4(
        m_cubeCenter[0] + gap,
        m_cubeCenter[1],
        m_cubeCenter[2],
        m_cubeCenter[3]
    );

    m_shaderProgram.SetUniform(m_colorUniform, white);

    m_shaderProgram.SetUniform(m_worldTranslationVectorUniform, worldTranslationVector);

    m_cube.DrawSubmesh(2);

    RenderGUI();
}

void Geometry4DApplication::Cleanup()
{
    // Cleanup DearImGUI
    m_imGui.Cleanup();

    Application::Cleanup();
}

void Geometry4DApplication::InitializeGeometry()
{
    Create4DCube(&m_cube, 1.0f);
    Create4DCubeWireframe(&m_cube, 1.0f);
    Create4DCubeWireframe(&m_cube, 1.0f);
}

void Geometry4DApplication::InitializeShaders()
{
    // Load and compile vertex shader
    Shader vertexShader(Shader::VertexShader);

    LoadAndCompileShader(vertexShader, "shaders/shader.vert");
    //LoadAndCompileShader(vertexShader, "C:\\Users\\spoor\\Desktop\\Uni\\MCS\\Semester2\\GP\\GP-Project-4DGeo\\src\\exercise10\\shaders\\shader.vert");

    // Load and compile fragment shader
    Shader fragmentShader(Shader::FragmentShader);
    LoadAndCompileShader(fragmentShader, "shaders/shader.frag");
    //LoadAndCompileShader(fragmentShader, "C:\\Users\\spoor\\Desktop\\Uni\\MCS\\Semester2\\GP\\GP-Project-4DGeo\\src\\exercise10\\shaders\\shader.frag");


    // Attach shaders and link
    if (!m_shaderProgram.Build(vertexShader, fragmentShader))
    {
        std::cout << "Error linking shaders" << std::endl;
        return;
    }
}

void Geometry4DApplication::InitializeCamera()
{
    // Create the main camera
    std::shared_ptr<Camera> camera = std::make_shared<Camera>();
    camera->SetViewMatrix(glm::vec3(0.0f, 1.5f, -5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0));
    float fov = 1.0f;
    camera->SetPerspectiveProjectionMatrix(fov, GetMainWindow().GetAspectRatio(), 0.1f, 100.0f);

    // Create a scene node for the camera
    std::shared_ptr<SceneCamera> sceneCamera = std::make_shared<SceneCamera>("camera", camera);

    // Set the camera scene node to be controlled by the camera controller
    m_cameraController.SetCamera(sceneCamera);
}

void Geometry4DApplication::InitializeUniforms()
{
    //Should be in material
    m_ambientReflectionUniform = m_shaderProgram.GetUniformLocation("AmbientReflection");
    m_diffuseReflectionUniform = m_shaderProgram.GetUniformLocation("DiffuseReflection");
    m_specularReflectionUniform = m_shaderProgram.GetUniformLocation("SpecularReflection");
    m_specularExponentUniform = m_shaderProgram.GetUniformLocation("SpecularExponent");

    m_colorUniform = m_shaderProgram.GetUniformLocation("Color");
    m_ambientColorUniform = m_shaderProgram.GetUniformLocation("AmbientColor");
    m_lightColorUniform = m_shaderProgram.GetUniformLocation("LightColor");
    m_lightPositionUniform = m_shaderProgram.GetUniformLocation("LightPosition");
    m_cameraPositionUniform = m_shaderProgram.GetUniformLocation("CameraPosition");
    m_worldRotationMatrixUniform = m_shaderProgram.GetUniformLocation("WorldRotationMatrix");
    m_worldTranslationVectorUniform = m_shaderProgram.GetUniformLocation("WorldTranslationVector");
    m_worldScaleVectorUniform = m_shaderProgram.GetUniformLocation("WorldScaleVector");
    m_viewProjMatrixUniform = m_shaderProgram.GetUniformLocation("ViewProjMatrix");
}

void Geometry4DApplication::LoadAndCompileShader(Shader& shader, const char* path)
{
    // Open the file for reading
    std::ifstream file(path);
    if (!file.is_open())
    {
        std::cout << "Can't find file: " << path << std::endl;
        std::cout << "Is your working directory properly set?" << std::endl;
        return;
    }

    // Dump the contents into a string
    std::stringstream stringStream;
    stringStream << file.rdbuf();

    // Set the source code from the string
    shader.SetSource(stringStream.str().c_str());

    // Try to compile
    if (!shader.Compile())
    {
        // Get errors in case of failure
        std::array<char, 256> errors;
        shader.GetCompilationErrors(errors);
        std::cout << "Error compiling shader: " << path << std::endl;
        std::cout << errors.data() << std::endl;
    }
}

void Geometry4DApplication::ResetState()
{
    bool resetState = GetMainWindow().IsKeyPressed(GLFW_KEY_R);
    if (resetState)
    {
        m_scale = m_cubeCenter[3] = 1.0f;
        m_cubeCenter[0] = m_cubeCenter[1] = m_cubeCenter[2] = 0;
        m_rotationVelocities[0] = m_rotationVelocities[1] = m_rotationVelocities[2]
            = m_rotationVelocities[3] = m_rotationVelocities[4] = m_rotationVelocities[5] = 0;
        m_xyRotation = m_yzRotation = m_xzRotation = m_xwRotation = m_ywRotation = m_zwRotation = 0;
    }
}

void Geometry4DApplication::RenderGUI()
{
    m_imGui.BeginFrame();

    if (auto window = m_imGui.UseWindow("Scene parameters"))
    {
        glm::mat4 viewMatrix = m_cameraController.GetCamera()->GetCamera()->GetViewMatrix();

        if (ImGui::TreeNodeEx("Cube", ImGuiTreeNodeFlags_DefaultOpen))
        {
            // Add controls for cube parameters
            ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
            ImGui::SliderFloat("Scale", &m_scale, 0.5f, 1.5f);
            ImGui::SliderFloat3("Center", &m_cubeCenter[0], -5.0f, 5.0f);
            ImGui::SliderFloat3("3D Rotation Velocity", &m_rotationVelocities[0], -5.0f, 5.0f);
            ImGui::SliderFloat("Center W", &m_cubeCenter[3], 1.0f, 5.0f);
            ImGui::SliderFloat3("4D Rotation Velocity", &m_rotationVelocities[3], -5.0f, 5.0f);
            ImGui::TreePop();
        }
    }

    m_imGui.EndFrame();
}

void Geometry4DApplication::Create4DCube(Mesh* mesh, float size)
{
    VertexFormat vertexFormat;
    vertexFormat.AddVertexAttribute<float>(4);
    vertexFormat.AddVertexAttribute<float>(4);

    std::vector<Vertex> vertices;
    std::vector<unsigned short> indices;

    //VBO
//Top Square
    vertices.push_back(Vertex(glm::vec4(-size / 2, size / 2, -size / 2, -size / 2)));
    vertices.push_back(Vertex(glm::vec4(size / 2, size / 2, -size / 2, -size / 2)));
    vertices.push_back(Vertex(glm::vec4(size / 2, size / 2, size / 2, -size / 2)));
    vertices.push_back(Vertex(glm::vec4(-size / 2, size / 2, size / 2, -size / 2)));

    //Bottom Square
    vertices.push_back(Vertex(glm::vec4(-size / 2, -size / 2, -size / 2, -size / 2)));
    vertices.push_back(Vertex(glm::vec4(size / 2, -size / 2, -size / 2, -size / 2)));
    vertices.push_back(Vertex(glm::vec4(size / 2, -size / 2, size / 2, -size / 2)));
    vertices.push_back(Vertex(glm::vec4(-size / 2, -size / 2, size / 2, -size / 2)));

    //Top Square
    vertices.push_back(Vertex(glm::vec4(-size / 2, size / 2, -size / 2, size / 2)));
    vertices.push_back(Vertex(glm::vec4(size / 2, size / 2, -size / 2, size / 2)));
    vertices.push_back(Vertex(glm::vec4(size / 2, size / 2, size / 2, size / 2)));
    vertices.push_back(Vertex(glm::vec4(-size / 2, size / 2, size / 2, size / 2)));

    //Bottom Square
    vertices.push_back(Vertex(glm::vec4(-size / 2, -size / 2, -size / 2, size / 2)));
    vertices.push_back(Vertex(glm::vec4(size / 2, -size / 2, -size / 2, size / 2)));
    vertices.push_back(Vertex(glm::vec4(size / 2, -size / 2, size / 2, size / 2)));
    vertices.push_back(Vertex(glm::vec4(-size / 2, -size / 2, size / 2, size / 2)));

    for (int i = 0; i < vertices.size(); i++) {
        vertices[i].normal = glm::normalize(vertices[i].position);
    }

    //EBO
    indices = {
        //Top Square
        2, 1, 0,
        3, 2, 0,
        10, 9, 8,
        11, 10, 8,
        //Right Square
        5, 1, 2,
        6, 5, 2,
        13, 9, 10,
        14, 13, 10,
        //Left Square
        3, 0, 7,
        0, 4, 7,
        11, 8, 15,
        8, 12, 15,
        //Back Square
        7, 2, 3,
        2, 7, 6,
        15, 10, 11,
        10, 15, 14,
        //Front Square
        5, 0, 1,
        4, 0, 5,
        13, 8, 9,
        8, 13, 12,
        //Bottom Square
        7, 4, 5,
        6, 7, 5,
        15, 12, 13,
        14, 15, 13,
        //Connect the Top
        0, 8, 1,
        8, 9, 1,
        1, 9, 2,
        9, 10, 2,
        2, 10, 3,
        10, 11, 3,
        3, 11, 0,
        11, 8, 0,
        //Connect the Bottom
        4, 5, 12,
        5, 13, 12,
        5, 6, 13,
        6, 14, 13,
        6, 7, 14,
        7, 15, 14,
        7, 4, 15,
        4, 12, 15,
        //Connect the Left
        0, 8, 12,
        4, 0, 12,
        3, 11, 15,
        7, 3, 15,
        //Connect the Right
        1, 9, 13,
        5, 1, 13,
        2, 10, 14,
        6, 2, 14,
    };

    mesh->AddSubmesh<Vertex, unsigned short, VertexFormat::LayoutIterator>(Drawcall::Primitive::Triangles, vertices, indices,
        vertexFormat.LayoutBegin(static_cast<int>(vertices.size()), true /* interleaved */), vertexFormat.LayoutEnd());
}

void Geometry4DApplication::Create4DCubeWireframe(Mesh* mesh, float size)
{
    VertexFormat vertexFormat;
    vertexFormat.AddVertexAttribute<float>(4);
    vertexFormat.AddVertexAttribute<float>(4);

    std::vector<Vertex> vertices;
    std::vector<unsigned short> indices;

    //VBO
    //Top Square
    vertices.push_back(Vertex(glm::vec4(-size / 2, size / 2, -size / 2, -size / 2)));
    vertices.push_back(Vertex(glm::vec4(size / 2, size / 2, -size / 2, -size / 2)));
    vertices.push_back(Vertex(glm::vec4(size / 2, size / 2, size / 2, -size / 2)));
    vertices.push_back(Vertex(glm::vec4(-size / 2, size / 2, size / 2, -size / 2)));

    //Bottom Square
    vertices.push_back(Vertex(glm::vec4(-size / 2, -size / 2, -size / 2, -size / 2)));
    vertices.push_back(Vertex(glm::vec4(size / 2, -size / 2, -size / 2, -size / 2)));
    vertices.push_back(Vertex(glm::vec4(size / 2, -size / 2, size / 2, -size / 2)));
    vertices.push_back(Vertex(glm::vec4(-size / 2, -size / 2, size / 2, -size / 2)));

    //Top Square
    vertices.push_back(Vertex(glm::vec4(-size / 2, size / 2, -size / 2, size / 2)));
    vertices.push_back(Vertex(glm::vec4(size / 2, size / 2, -size / 2, size / 2)));
    vertices.push_back(Vertex(glm::vec4(size / 2, size / 2, size / 2, size / 2)));
    vertices.push_back(Vertex(glm::vec4(-size / 2, size / 2, size / 2, size / 2)));

    //Bottom Square
    vertices.push_back(Vertex(glm::vec4(-size / 2, -size / 2, -size / 2, size / 2)));
    vertices.push_back(Vertex(glm::vec4(size / 2, -size / 2, -size / 2, size / 2)));
    vertices.push_back(Vertex(glm::vec4(size / 2, -size / 2, size / 2, size / 2)));
    vertices.push_back(Vertex(glm::vec4(-size / 2, -size / 2, size / 2, size / 2)));

    for (int i = 0; i < vertices.size(); i++) {
        vertices[i].normal = glm::normalize(vertices[i].position);
    }
    //EBO
    indices = {
        0, 1,
        0, 3,
        0, 4,
        0, 8,
        1, 2,
        1, 5,
        1, 9,
        2, 3,
        2, 6,
        2, 10,
        3, 7,
        3, 11,
        4, 5,
        4, 7,
        4, 12,
        5, 6,
        5, 13,
        6, 7,
        6, 14,
        7, 15,
        8, 9,
        8, 11,
        8, 12,
        9, 10,
        9, 13,
        10, 11,
        10, 14,
        11, 15,
        12, 13,
        12, 15,
        13, 14,
        14, 15
    };

    mesh->AddSubmesh<Vertex, unsigned short, VertexFormat::LayoutIterator>(Drawcall::Primitive::Lines, vertices, indices,
        vertexFormat.LayoutBegin(static_cast<int>(vertices.size()), true /* interleaved */), vertexFormat.LayoutEnd());
}


glm::mat4 Geometry4DApplication::Rotate4D(float xy, float yz, float xz, float xw, float yw, float zw)
{
    float mXY[16] = {
            cos(xy), -sin(xy), 0, 0,
            sin(xy), cos(xy), 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
    };
    glm::mat4 rXY = glm::make_mat4(mXY);

    float mYZ[16] = {
    1, 0, 0, 0,
    0, cos(yz), -sin(yz), 0,
    0, sin(yz), cos(yz), 0,
    0, 0, 0, 1
    };
    glm::mat4 rYZ = glm::make_mat4(mYZ);

    float mXZ[16] = {
            cos(xz), 0, -sin(xz), 0,
            0, 1, 0, 0,
            sin(xz), 0, cos(xz), 0,
            0, 0, 0, 1
    };
    glm::mat4 rXZ = glm::make_mat4(mXZ);

    float mXW[16] = {
        cos(xw), 0, 0, -sin(xw),
        0, 1, 0, 0,
        0, 0, 1, 0,
        sin(xw), 0, 0, cos(xw)
    };
    glm::mat4 rXW = glm::make_mat4(mXW);

    float mYW[16] = {
        1, 0, 0, 0,
        0, cos(yw), 0, -sin(yw),
        0, 0, 1, 0,
        0, sin(yw), 0, cos(yw)
    };
    glm::mat4 rYW = glm::make_mat4(mYW);

    float mZW[16] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, cos(zw), -sin(zw),
        0, 0, sin(zw), cos(zw)
    };
    glm::mat4 rZW = glm::make_mat4(mZW);

    return rXY * rXZ * rYZ * rXW * rYW * rZW;
}