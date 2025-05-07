#include "Geometry4D.h"

#include <ituGL/shader/Shader.h>
#include <ituGL/shader/ShaderProgram.h>
#include <ituGL/geometry/VertexFormat.h>
#include <ituGL/camera/Camera.h>
#include <ituGL/scene/SceneCamera.h>
#include <imgui.h>
#include <cassert>
#include <array>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <filesystem>
#include <glm/gtx/transform.hpp>
#include <glm/ext.hpp>

Geometry4DApplication::Geometry4DApplication()
    : Application(1024, 1024, "4D-Geometry demo")
    , m_colorUniform(-1)
    , m_worldMatrixUniform(-1)
    , m_viewProjMatrixUniform(-1)
    , m_xRotation(0)
    , m_yRotation(0)
    , m_zRotation(0)
    , m_rotationVelocities(0)
    , m_cubeCenter(0)
{
}

void Geometry4DApplication::Initialize()
{
    Application::Initialize();

    m_imGui.Initialize(GetMainWindow());

    InitializeGeometry();
    InitializeShaders();
    InitializeCamera();

    m_colorUniform = m_shaderProgram.GetUniformLocation("Color");
    m_worldMatrixUniform = m_shaderProgram.GetUniformLocation("WorldMatrix");
    m_viewProjMatrixUniform = m_shaderProgram.GetUniformLocation("ViewProjMatrix");

    //Triangles should wind clock-wise
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
}

void Geometry4DApplication::Update()
{
    Application::Update();

    const Window& window = GetMainWindow();

    m_cameraController.Update(GetMainWindow(), GetDeltaTime());

    m_camera = *m_cameraController.GetCamera()->GetCamera();
}

void Geometry4DApplication::Render()
{
    Application::Render();

    GetDevice().Clear(true, Color(0.0f, 0.0f, 0.0f, 1.0f), true, 1.0f);

    Color color = Color(1, 1, 1);

    //rotating by the given amount
    m_xRotation += m_rotationVelocities[0] / 1000;
    m_yRotation += m_rotationVelocities[1] / 1000;
    m_zRotation += m_rotationVelocities[2] / 1000;

    glm::mat4 worldMatrix =
        glm::translate(glm::vec3(m_cubeCenter[0], m_cubeCenter[1], m_cubeCenter[2])) *
        glm::rotate(m_xRotation, glm::vec3(0, 1, 0)) *
        glm::rotate(m_yRotation, glm::vec3(0, 0, 1)) *
        glm::rotate(m_zRotation, glm::vec3(1, 0, 0)) *
        glm::mat4(1.0f);
    
    m_shaderProgram.Use();

    m_shaderProgram.SetUniform(m_viewProjMatrixUniform, m_camera.GetViewProjectionMatrix());
    
    m_shaderProgram.SetUniform(m_colorUniform, static_cast<glm::vec3>(color));

    m_shaderProgram.SetUniform(m_worldMatrixUniform, worldMatrix);

    m_cube.DrawSubmesh(0);

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
    VertexFormat vertexFormat;
    vertexFormat.AddVertexAttribute<float>(3);
    vertexFormat.AddVertexAttribute<float>(3);

    std::vector<Vertex> vertices;
    std::vector<unsigned short> indices;

    //VBO
    //Top Square
    vertices.push_back(Vertex(glm::vec3(-1, 1, -1)));
    vertices[0].normal = glm::normalize(vertices[0].position);
    vertices.push_back(Vertex(glm::vec3(1, 1, -1)));
    vertices[1].normal = glm::normalize(vertices[1].position);
    vertices.push_back(Vertex(glm::vec3(1, 1, 1)));
    vertices[2].normal = glm::normalize(vertices[2].position);
    vertices.push_back(Vertex(glm::vec3(-1, 1, 1)));
    vertices[3].normal = glm::normalize(vertices[3].position);

    //Bottom Square
    vertices.push_back(Vertex(glm::vec3(-1, -1, -1)));
    vertices[4].normal = glm::normalize(vertices[4].position);
    vertices.push_back(Vertex(glm::vec3(1, -1, -1)));
    vertices[5].normal = glm::normalize(vertices[5].position);
    vertices.push_back(Vertex(glm::vec3(1, -1, 1)));
    vertices[6].normal = glm::normalize(vertices[6].position);
    vertices.push_back(Vertex(glm::vec3(-1, -1, 1)));
    vertices[7].normal = glm::normalize(vertices[7].position);

    std::vector<glm::vec3> normals(vertices.size(), glm::vec3(0.0f));

    //EBO
    //Top Square Checked
    indices.insert(indices.end(), { 0, 2, 1 });
    indices.insert(indices.end(), { 0, 3, 2 });

    //Right Square Checked
    indices.insert(indices.end(), { 1, 2, 5 });
    indices.insert(indices.end(), { 2, 6, 5 });

    //Left Square Checked
    indices.insert(indices.end(), { 0, 7, 3 });
    indices.insert(indices.end(), { 0, 4, 7 });

    //Back Square 
    indices.insert(indices.end(), { 2, 3, 7 });
    indices.insert(indices.end(), { 2, 7, 6 });

    //Front Square
    indices.insert(indices.end(), { 0, 1, 5 });
    indices.insert(indices.end(), { 0, 5, 4 });

    //Bottom Square Checked
    indices.insert(indices.end(), { 4, 5, 7 });
    indices.insert(indices.end(), { 5, 6, 7 });

    m_cube.AddSubmesh<Vertex, unsigned short, VertexFormat::LayoutIterator>(Drawcall::Primitive::Triangles, vertices, indices,
        vertexFormat.LayoutBegin(static_cast<int>(vertices.size()), true /* interleaved */), vertexFormat.LayoutEnd());

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
    camera->SetViewMatrix(glm::vec3(0.0f, 0.0f, -4.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0));
    float fov = 1.0f;
    camera->SetPerspectiveProjectionMatrix(fov, GetMainWindow().GetAspectRatio(), 0.1f, 100.0f);

    // Create a scene node for the camera
    std::shared_ptr<SceneCamera> sceneCamera = std::make_shared<SceneCamera>("camera", camera);

    // Set the camera scene node to be controlled by the camera controller
    m_cameraController.SetCamera(sceneCamera);
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

void Geometry4DApplication::RenderGUI()
{
    m_imGui.BeginFrame();

    if (auto window = m_imGui.UseWindow("Scene parameters"))
    {
        glm::mat4 viewMatrix = m_cameraController.GetCamera()->GetCamera()->GetViewMatrix();

        if (ImGui::TreeNodeEx("Cube", ImGuiTreeNodeFlags_DefaultOpen))
        {
            // Add controls for cube parameters
            ImGui::SliderFloat3("Center", &m_cubeCenter[0], -5.0f, 5.0f);
            ImGui::SliderFloat3("Rotation Velocity", &m_rotationVelocities[0], -5.0f, 5.0f);
            ImGui::TreePop();
        }
    }

    m_imGui.EndFrame();
}

void Geometry4DApplication::ComputeNormals(Vertex& v1, Vertex& v2, Vertex& v3)
{
    glm::vec3 pos1 = v1.position;
    glm::vec3 pos2 = v2.position;
    glm::vec3 pos3 = v3.position;

    glm::vec3 edge1 = pos2 - pos1;
    glm::vec3 edge2 = pos3 - pos1;

    glm::vec3 faceNormal = glm::normalize(glm::cross(edge1, edge2));

    v1.normal += faceNormal;
    v2.normal += faceNormal;
    v3.normal += faceNormal;

}
