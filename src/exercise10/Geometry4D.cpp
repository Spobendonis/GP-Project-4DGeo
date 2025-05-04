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

    //Triangles should wind clock-wise
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
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

    Color color = Color(1, 1, 1);

    //rotating by the given amount
    m_xRotation += 1.0f/1000;

    glm::mat4 worldMatrix =
        glm::rotate(m_xRotation, glm::vec3(0, 1, 0)) *
        glm::rotate(m_yRotation, glm::vec3(0, 0, 1)) *
        glm::rotate(m_zRotation, glm::vec3(1, 0, 0)) *
        glm::scale(glm::vec3(0.1f)) * glm::mat4(1.0f);
    
    m_shaderProgram.Use();

    m_shaderProgram.SetUniform(m_viewProjMatrixUniform, m_camera.GetViewProjectionMatrix());
    
    m_shaderProgram.SetUniform(m_colorUniform, static_cast<glm::vec3>(color));

    m_shaderProgram.SetUniform(m_worldMatrixUniform, worldMatrix);

    m_cube.DrawSubmesh(0);
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
    vertices.push_back(Vertex(glm::vec3(1, 1, -1)));
    vertices.push_back(Vertex(glm::vec3(1, 1, 1)));
    vertices.push_back(Vertex(glm::vec3(-1, 1, 1)));

    //Bottom Square
    vertices.push_back(Vertex(glm::vec3(-1, -1, -1)));
    vertices.push_back(Vertex(glm::vec3(1, -1, -1)));
    vertices.push_back(Vertex(glm::vec3(1, -1, 1)));
    vertices.push_back(Vertex(glm::vec3(-1, -1, 1)));

    std::vector<glm::vec3> normals(vertices.size(), glm::vec3(0.0f));

    //EBO
    //Top Square Checked
    indices.insert(indices.end(), { 0, 2, 1 });
    ComputeNormals(vertices[0], vertices[2], vertices[1]);
    indices.insert(indices.end(), { 0, 3, 2 });
    ComputeNormals(vertices[0], vertices[3], vertices[2]);

    //Right Square Checked
    indices.insert(indices.end(), { 1, 2, 5 });
    ComputeNormals(vertices[1], vertices[2], vertices[5]);
    indices.insert(indices.end(), { 2, 6, 5 });
    ComputeNormals(vertices[2], vertices[6], vertices[5]);

    //Left Square Checked
    indices.insert(indices.end(), { 0, 7, 3 });
    ComputeNormals(vertices[0], vertices[7], vertices[3]);
    indices.insert(indices.end(), { 0, 4, 7 });
    ComputeNormals(vertices[0], vertices[4], vertices[7]);

    //Back Square 
    indices.insert(indices.end(), { 2, 3, 7 });
    ComputeNormals(vertices[2], vertices[3], vertices[7]);
    indices.insert(indices.end(), { 2, 7, 6 });
    ComputeNormals(vertices[2], vertices[7], vertices[6]);

    //Front Square
    indices.insert(indices.end(), { 0, 1, 5 });
    ComputeNormals(vertices[0], vertices[1], vertices[5]);
    indices.insert(indices.end(), { 0, 5, 4 });
    ComputeNormals(vertices[0], vertices[5], vertices[4]);

    //Bottom Square Checked
    indices.insert(indices.end(), { 4, 5, 7 });
    ComputeNormals(vertices[4], vertices[5], vertices[7]);
    indices.insert(indices.end(), { 5, 6, 7 });
    ComputeNormals(vertices[5], vertices[6], vertices[7]);

    // After combining all face normals, normalize once
    for (int i = 0; i < vertices.size(); i++) {
        vertices[i].normal = glm::normalize(vertices[i].normal);
    }

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
