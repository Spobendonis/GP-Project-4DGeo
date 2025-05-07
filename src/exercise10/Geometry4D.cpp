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
#include <time.h>
#include <glm/gtx/transform.hpp>
#include <glm/ext.hpp>

Geometry4DApplication::Geometry4DApplication()
    : Application(1024, 1024, "4D-Geometry demo")
    , m_colorUniform(-1)
    , m_worldMatrixUniform(-1)
    , m_viewProjMatrixUniform(-1)
    , m_rotationVelocities(0)
    , m_cubeCenter(0)
    , m_xyRotation(0)
    , m_xzRotation(0)
    , m_yzRotation(0)
    , m_xwRotation(0)
    , m_ywRotation(0)
    , m_zwRotation(0)
{
}


/*
TODO:
- Make wireframe where you only see intended edges
- Fix inversion bug (rotating in 4th gives a dark phase and light phase)
- Fix edge-frick bug during rotation
- Implement 4D Translate and Scale
*/
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

    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
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
    //3D rotations
    m_xyRotation += m_rotationVelocities[0] / 1000;
    m_xzRotation += m_rotationVelocities[2] / 1000;
    m_yzRotation += m_rotationVelocities[1] / 1000;

    //4D rotations
    m_xwRotation += m_rotationVelocities[3] / 1000;
    m_ywRotation += m_rotationVelocities[4] / 1000;
    m_zwRotation += m_rotationVelocities[5] / 1000;

    glm::mat4 worldMatrix =
        Rotate4D(m_xyRotation, m_xzRotation, m_yzRotation, m_xwRotation, m_ywRotation, m_zwRotation) *
        glm::mat4(1);

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
    struct Vertex
    {
        Vertex() = default;
        Vertex(const glm::vec4& position) : position(position), normal(glm::vec4(0.0f)) {}
        Vertex(const glm::vec4& position, const glm::vec4& normal) : position(position), normal(normal) {}
        glm::vec4 position;
        glm::vec4 normal;
    };

    VertexFormat vertexFormat;
    vertexFormat.AddVertexAttribute<float>(4);
    vertexFormat.AddVertexAttribute<float>(4);

    std::vector<Vertex> vertices;
    //std::vector<Edge> edges;
    std::vector<unsigned short> indices;

    //VBO
    //Top Square
    vertices.push_back(Vertex(glm::vec4(-1, 1, -1, 1)));
    vertices[0].normal = glm::normalize(vertices[0].position);
    vertices.push_back(Vertex(glm::vec4(1, 1, -1, 1)));
    vertices[1].normal = glm::normalize(vertices[1].position);
    vertices.push_back(Vertex(glm::vec4(1, 1, 1, 1)));
    vertices[2].normal = glm::normalize(vertices[2].position);
    vertices.push_back(Vertex(glm::vec4(-1, 1, 1, 1)));
    vertices[3].normal = glm::normalize(vertices[3].position);

    //Bottom Square
    vertices.push_back(Vertex(glm::vec4(-1, -1, -1, 1)));
    vertices[4].normal = glm::normalize(vertices[4].position);
    vertices.push_back(Vertex(glm::vec4(1, -1, -1, 1)));
    vertices[5].normal = glm::normalize(vertices[5].position);
    vertices.push_back(Vertex(glm::vec4(1, -1, 1, 1)));
    vertices[6].normal = glm::normalize(vertices[6].position);
    vertices.push_back(Vertex(glm::vec4(-1, -1, 1, 1)));
    vertices[7].normal = glm::normalize(vertices[7].position);

    //Top Square
    vertices.push_back(Vertex(glm::vec4(-1, 1, -1, 2)));
    vertices[8].normal = glm::normalize(vertices[8].position);
    vertices.push_back(Vertex(glm::vec4(1, 1, -1, 2)));
    vertices[9].normal = glm::normalize(vertices[9].position);
    vertices.push_back(Vertex(glm::vec4(1, 1, 1, 2)));
    vertices[10].normal = glm::normalize(vertices[10].position);
    vertices.push_back(Vertex(glm::vec4(-1, 1, 1, 2)));
    vertices[11].normal = glm::normalize(vertices[11].position);

    //Bottom Square
    vertices.push_back(Vertex(glm::vec4(-1, -1, -1, 2)));
    vertices[12].normal = glm::normalize(vertices[12].position);
    vertices.push_back(Vertex(glm::vec4(1, -1, -1, 2)));
    vertices[13].normal = glm::normalize(vertices[13].position);
    vertices.push_back(Vertex(glm::vec4(1, -1, 1, 2)));
    vertices[14].normal = glm::normalize(vertices[14].position);
    vertices.push_back(Vertex(glm::vec4(-1, -1, 1, 2)));
    vertices[15].normal = glm::normalize(vertices[15].position); 

    //EBO
    //Top Square
    indices.insert(indices.end(), { 0, 2, 1 });
    indices.insert(indices.end(), { 0, 3, 2 });

    indices.insert(indices.end(), { 8, 10, 9 });
    indices.insert(indices.end(), { 8, 11, 10 });

    //Right Square
    indices.insert(indices.end(), { 1, 2, 5 });
    indices.insert(indices.end(), { 2, 6, 5 });

    indices.insert(indices.end(), { 9, 10, 13 });
    indices.insert(indices.end(), { 10, 14, 13 });

    //Left Square
    indices.insert(indices.end(), { 0, 7, 3 });
    indices.insert(indices.end(), { 0, 4, 7 });

    indices.insert(indices.end(), { 8, 15, 11 });
    indices.insert(indices.end(), { 8, 12, 15 });

    //Back Square 
    indices.insert(indices.end(), { 2, 3, 7 });
    indices.insert(indices.end(), { 2, 7, 6 });

    indices.insert(indices.end(), { 10, 11, 15 });
    indices.insert(indices.end(), { 10, 15, 14 });

    //Front Square
    indices.insert(indices.end(), { 0, 1, 5 });
    indices.insert(indices.end(), { 0, 5, 4 });

    indices.insert(indices.end(), { 8, 9, 13 });
    indices.insert(indices.end(), { 8, 13, 12 });

    //Bottom Square
    indices.insert(indices.end(), { 4, 5, 7 });
    indices.insert(indices.end(), { 5, 6, 7 });

    indices.insert(indices.end(), { 12, 13, 15 });
    indices.insert(indices.end(), { 13, 14, 15 });

    //Connect the Top

    indices.insert(indices.end(), { 0, 8, 1 });
    indices.insert(indices.end(), { 8, 9, 1 });

    indices.insert(indices.end(), { 1, 9, 2 });
    indices.insert(indices.end(), { 9, 10, 2 });

    indices.insert(indices.end(), { 2, 10, 3 });
    indices.insert(indices.end(), { 10, 11, 3 });

    indices.insert(indices.end(), { 3, 11, 0 });
    indices.insert(indices.end(), { 11, 8, 0 });

    //Connect the Bottom

    indices.insert(indices.end(), { 4, 5, 12 });
    indices.insert(indices.end(), { 12, 5, 13 });

    indices.insert(indices.end(), { 5, 6, 13 });
    indices.insert(indices.end(), { 13, 6, 14 });

    indices.insert(indices.end(), { 6, 7, 14 });
    indices.insert(indices.end(), { 14, 7, 15 });

    indices.insert(indices.end(), { 7, 4, 15 });
    indices.insert(indices.end(), { 15, 4, 12 });

    //Connect the Left

    indices.insert(indices.end(), { 0, 8, 12 });
    indices.insert(indices.end(), { 0, 12, 4 });

    indices.insert(indices.end(), { 3, 11, 15 });
    indices.insert(indices.end(), { 3, 15, 7 });

    //Connect the Right

    indices.insert(indices.end(), { 1, 9, 13 });
    indices.insert(indices.end(), { 1, 13, 5 });

    indices.insert(indices.end(), { 2, 10, 14 });
    indices.insert(indices.end(), { 2, 14, 6 });

    m_cube.AddSubmesh<Vertex, unsigned short, VertexFormat::LayoutIterator>(Drawcall::Primitive::Triangles, vertices, indices,
        vertexFormat.LayoutBegin(static_cast<int>(vertices.size()), true /* interleaved */), vertexFormat.LayoutEnd());

}

void Geometry4DApplication::InitializeShaders()
{
    // Load and compile vertex shader
    Shader vertexShader(Shader::VertexShader);

    //LoadAndCompileShader(vertexShader, "shaders/shader.vert");
    LoadAndCompileShader(vertexShader, "C:\\Users\\spoor\\Desktop\\Uni\\MCS\\Semester2\\GP\\GP-Project-4DGeo\\src\\exercise10\\shaders\\shader.vert");

    // Load and compile fragment shader
    Shader fragmentShader(Shader::FragmentShader);
    //LoadAndCompileShader(fragmentShader, "shaders/shader.frag");
    LoadAndCompileShader(fragmentShader, "C:\\Users\\spoor\\Desktop\\Uni\\MCS\\Semester2\\GP\\GP-Project-4DGeo\\src\\exercise10\\shaders\\shader.frag");


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
    camera->SetViewMatrix(glm::vec3(0.0f, 0.0f, -8.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0));
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
            ImGui::SliderFloat3("3D Rotation Velocity", &m_rotationVelocities[0], -5.0f, 5.0f);
            ImGui::SliderFloat3("4D Rotation Velocity", &m_rotationVelocities[3], -5.0f, 5.0f);
            ImGui::TreePop();
        }
    }

    m_imGui.EndFrame();
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

//TODO: Fix
glm::mat4 Geometry4DApplication::Translate4D(glm::vec4 v)
{
    float m[16] =
    {
        v.x, 0, 0, 0,
        0, v.y, 0, 0,
        0, 0, v.z, 0,
        0, 0, 0, v.z

    };
    return glm::make_mat4(m);
}

//TODO: Fix
glm::mat4 Geometry4DApplication::Scale4D(float s)
{
    return glm::mat4(s);
}