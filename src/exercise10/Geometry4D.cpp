#include "Geometry4D.h"

#include <ituGL/shader/Shader.h>
#include <ituGL/shader/ShaderProgram.h>
#include <ituGL/geometry/VertexFormat.h>
#include <ituGL/camera/Camera.h>
#include <imgui.h>
#include <cassert>
#include <array>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <glm/gtx/transform.hpp>

Geometry4DApplication::Geometry4DApplication()
    : Application(1024, 1024, "4D-Geometry demo")
    , m_colorUniform(-1)
    , m_worldMatrixUniform(-1)
    , m_viewProjMatrixUniform(-1)
{
}

void Geometry4DApplication::Initialize()
{
    Application::Initialize();

    InitializeGeometry();
    InitializeShaders();

    m_colorUniform = m_shaderProgram.GetUniformLocation("Color");
    m_worldMatrixUniform = m_shaderProgram.GetUniformLocation("WorldMatrix");
    m_viewProjMatrixUniform = m_shaderProgram.GetUniformLocation("ViewProjMatrix");

}

void Geometry4DApplication::Update()
{
    Application::Update();

    const Window& window = GetMainWindow();

    // (todo) 03.5: Update the camera matrices
    int width, height;
    window.GetDimensions(width, height);
    m_camera.SetOrthographicProjectionMatrix(glm::vec3(-width * 1.0f / height, -1, -3), glm::vec3(width * 1.0f / height, 1, 3));

    glm::vec2 mouseCoords = window.GetMousePosition(true);
    m_camera.SetViewMatrix(glm::vec3(0, 0, 1), glm::vec3(mouseCoords.x, mouseCoords.y, 0));
}

void Geometry4DApplication::Render()
{
    Application::Render();

    GetDevice().Clear(true, Color(0.0f, 0.0f, 0.0f, 1.0f), true, 1.0f);

    //float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    //float g = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
    //float b = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);

    //Color color = Color(r, g, b);

    Color color = Color(1, 1, 1);

    glm::mat4 worldMatrix = glm::scale(glm::vec3(0.1f)) * glm::mat4(1.0f);

    m_shaderProgram.Use();

    m_shaderProgram.SetUniform(m_viewProjMatrixUniform, m_camera.GetViewProjectionMatrix());
    
    m_shaderProgram.SetUniform(m_colorUniform, static_cast<glm::vec3>(color));

    m_shaderProgram.SetUniform(m_worldMatrixUniform, worldMatrix);

    m_cube.DrawSubmesh(0);
}

void Geometry4DApplication::InitializeGeometry()
{
    struct Vertex
    {
        Vertex() = default;
        Vertex(const glm::vec3& position) : position(position) {}
        glm::vec3 position;
    };

    VertexFormat vertexFormat;
    vertexFormat.AddVertexAttribute<float>(3);

    std::vector<Vertex> vertices;
    std::vector<unsigned short> indices;

    //VBO
    //Top Square
    vertices.emplace_back(glm::vec3(-1, 1, -1));
    vertices.emplace_back(glm::vec3(1, 1, -1));
    vertices.emplace_back(glm::vec3(1, 1, 1));
    vertices.emplace_back(glm::vec3(-1, 1, 1));

    //Bottom Square
    vertices.emplace_back(glm::vec3(-1, -1, -1));
    vertices.emplace_back(glm::vec3(1, -1, -1));
    vertices.emplace_back(glm::vec3(1, -1, 1));
    vertices.emplace_back(glm::vec3(-1, -1, 1));


    //EBO
    //Top Square
    indices.insert(indices.end(), { 0, 1, 2 });
    indices.insert(indices.end(), { 1, 2, 3 });

    //Right Square
    indices.insert(indices.end(), { 1, 2, 5 });
    indices.insert(indices.end(), { 2, 5, 6 });

    //Left Square
    indices.insert(indices.end(), { 0, 3, 4 });
    indices.insert(indices.end(), { 3, 5, 7 });

    //Front Square
    indices.insert(indices.end(), { 2, 3, 6 });
    indices.insert(indices.end(), { 2, 6, 7 });

    //Back Square
    indices.insert(indices.end(), { 0, 1, 4 });
    indices.insert(indices.end(), { 0, 4, 5 });

    //Bottom Square
    indices.insert(indices.end(), { 4, 5, 6 });
    indices.insert(indices.end(), { 5, 6, 7 });

    m_cube.AddSubmesh<Vertex, unsigned short, VertexFormat::LayoutIterator>(Drawcall::Primitive::Triangles, vertices, indices,
        vertexFormat.LayoutBegin(static_cast<int>(vertices.size()), true /* interleaved */), vertexFormat.LayoutEnd());

}

void Geometry4DApplication::InitializeShaders()
{
    // Load and compile vertex shader
    Shader vertexShader(Shader::VertexShader);

    LoadAndCompileShader(vertexShader, "shaders/shader.vert");
    //LoadAndCompileShader(vertexShader, "C:\\Users\\spoor\\Desktop\\Uni\\MCS\\Semester2\\GP\\GP - Project - 4DGeo\\src\\exercise10\\shaders\\shader.vert");

    // Load and compile fragment shader
    Shader fragmentShader(Shader::FragmentShader);
    LoadAndCompileShader(fragmentShader, "shaders/shader.frag");
    //LoadAndCompileShader(fragmentShader, "C:\\Users\\spoor\\Desktop\\Uni\\MCS\\Semester2\\GP\\GP - Project - 4DGeo\\src\\exercise10\\shaders\\shader.frag");


    // Attach shaders and link
    if (!m_shaderProgram.Build(vertexShader, fragmentShader))
    {
        std::cout << "Error linking shaders" << std::endl;
        return;
    }
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
