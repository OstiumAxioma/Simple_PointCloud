#include <GL/glew.h>  
#include <GLFW/glfw3.h>  
//GLM  
#include <glm/glm.hpp>  
#include <glm/gtc/matrix_transform.hpp>  
#include <glm/gtc/type_ptr.hpp>  

#include <iostream>  
#include <fstream>  
#include <vector>  

#pragma region 变量定义  
//窗口参数  
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
//相机参数  
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 0.5f, 0.0f);
//时间参数  
float deltaTime = 0.0f;
float lastTime = 0.0f;

// 鼠标参数  
bool firstMouse = true;
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
float fov = 45.0f;

// 点大小  
float pointSize = 1.0f; // 初始化点大小  

#pragma endregion  

#pragma region 函数定义  
// 处理输入  
void processInput(GLFWwindow* window)
{
    float cameraSpeed = 2.5f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraUp;
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraUp;
    // 退出  
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    // 调整点大小  
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        pointSize += 0.1f;
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        pointSize = std::max(0.1f, pointSize - 0.1f);
}

// 鼠标移动回调  
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

// 鼠标滚轮回调  
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    fov -= (float)yoffset;
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 45.0f)
        fov = 45.0f;
}

//窗口大小回调  
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

static unsigned int CompileShader(unsigned int type, const std::string& source)
{
    const char* src = source.c_str();
    unsigned int id = glCreateShader(type);
    glShaderSource(id, 1, &src, NULL);
    glCompileShader(id);

    int success;
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);

    if (success == GL_FALSE)
    {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)alloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);
        std::cout
            << (type == GL_VERTEX_SHADER ? "Vertex" : "Fragment")
            << " shader failed to compile! \n"
            << message
            << std::endl;
        glDeleteShader(id);
        return 0;
    }
    return id;
}
#pragma endregion  

int main(void)
{
    // 初始化GLFW  
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Point Cloud Visualization", NULL, NULL);
    if (!window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // 捕获鼠标  
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (glewInit() != GLEW_OK)
        std::cout << "GLEW failed to initialize!" << std::endl;

    std::cout << "OpenGL version: " << glGetString(GL_VERSION) << std::endl;

    // 加载数据  
    struct Points
    {
        float x, y, z;       // 位置  
        float r, g, b;       // 颜色  
        float other1, other2; // 其他数据  
    };

    std::vector<Points> vertices;
    std::ifstream ifs;
    ifs.open("bun180 - Cloud.txt", std::ios::in);
    if (!ifs.is_open())
    {
        std::cout << "File not found!" << std::endl;
        return -1;
    }

    Points tempPoint;
    while (ifs >> tempPoint.x >> tempPoint.y >> tempPoint.z
        >> tempPoint.r >> tempPoint.g >> tempPoint.b
        >> tempPoint.other1 >> tempPoint.other2)
    {
        tempPoint.r /= 255.0f;
        tempPoint.g /= 255.0f;
        tempPoint.b /= 255.0f;
        vertices.push_back(tempPoint);
    }

    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    // 生成VBO  
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Points), &vertices[0], GL_STATIC_DRAW);

    // 指定顶点属性  
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Points), (void*)offsetof(Points, x));
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Points), (void*)offsetof(Points, r));
    glEnableVertexAttribArray(1);

    // 着色器程序  
    const char* vertexShaderSource =
        "#version 330 core\n"
        "layout (location = 0) in vec3 aPos;\n"
        "layout (location = 1) in vec3 aColor;\n"
        "uniform mat4 model;\n"
        "uniform mat4 view;\n"
        "uniform mat4 projection;\n"
        "out vec3 VertexColor;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
        "   VertexColor = aColor;\n"
        "}\0";

    unsigned int vertexShader = CompileShader(GL_VERTEX_SHADER, vertexShaderSource);

    const char* fragmentShaderSource =
        "#version 330 core\n"
        "in vec3 VertexColor;\n"
        "out vec4 FragColor;\n"
        "void main()\n"
        "{\n"
        "   FragColor = vec4(VertexColor, 1.0f);\n"
        "}\n\0";

    unsigned int fragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    // 着色器程序链接  
    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // 深度测试  
    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastTime;
        lastTime = currentFrame;

        // 输入处理  
        processInput(window);

        // 清屏  
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glfwSwapInterval(1);

        // 使用程序  
        glUseProgram(shaderProgram);

        // 创建变换矩阵  
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glm::mat4 projection = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

        // 传递矩阵到着色器  
        unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
        unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
        unsigned int projectionLoc = glGetUniformLocation(shaderProgram, "projection");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(VAO);
        glPointSize(pointSize); // 使用当前点大小  
        glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(vertices.size()));

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // 清理OpenGL资源  
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}