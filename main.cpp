#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <cmath>

// --- SHADERS ---
const char* vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "uniform float xOffset;\n"
    "uniform float yOffset;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x + xOffset, aPos.y + yOffset, aPos.z, 1.0);\n"
    "}";

const char* fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "uniform vec4 ourColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = ourColor;\n"
    "}";

// --- GAME STATE & PHYSICS ---
float playerY = 0.0f;
float velocityY = 0.0f;
bool isGrounded = true;
int score = 0;

const float gravity = -0.0007f;
const float jumpForce = 0.02f;

// --- OBSTACLE DATA ---
float obstacleX = 1.2f;
float obstacleSpeed = 0.015f;

// Error handling helper
void checkShaderErrors(unsigned int shader, std::string type) {
    int success;
    char infoLog[1024];
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::SHADER_COMPILATION_FAILED: " << type << "\n" << infoLog << std::endl;
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::PROGRAM_LINKING_FAILED\n" << infoLog << std::endl;
        }
    }
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && isGrounded) {
        velocityY = jumpForce;
        isGrounded = false;
    }
}

int main() {
    if (!glfwInit()) return -1;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "The Debugger's Run", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        return -1;
    }

    // Shader Setup
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    checkShaderErrors(vertexShader, "VERTEX");

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    checkShaderErrors(fragmentShader, "FRAGMENT");

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    checkShaderErrors(shaderProgram, "PROGRAM");

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // --- SQUARE DATA (EBO) ---
    // Defined as a Quad using 4 vertices and 6 indices (2 triangles)
    float vertices[] = {
         0.06f,  0.06f, 0.0f,  // Top Right (0)
         0.06f, -0.06f, 0.0f,  // Bottom Right (1)
        -0.06f, -0.06f, 0.0f,  // Bottom Left (2)
        -0.06f,  0.06f, 0.0f   // Top Left (3)
    };
    unsigned int indices[] = {
        0, 1, 3, // First Triangle
        1, 2, 3  // Second Triangle
    };

    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Fixed Player X position
    const float playerX = -0.6f;

    // Game Loop
    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        // --- UPDATE PHYSICS ---
        if (!isGrounded) {
            velocityY += gravity;
            playerY += velocityY;
        }
        if (playerY <= 0.0f) {
            playerY = 0.0f;
            velocityY = 0.0f;
            isGrounded = true;
        }

        // --- UPDATE OBSTACLE ---
        obstacleX -= obstacleSpeed;
        if (obstacleX < -1.2f) {
            obstacleX = 1.2f;
            score += 10;
            std::cout << "Score: " << score << " | Speeding up..." << std::endl;
            obstacleSpeed += 0.001f; // Increase difficulty
        }

        // --- COLLISION DETECTION ---
        // Basic AABB check
        if (std::abs(playerX - obstacleX) < 0.1f && playerY < 0.12f) {
            std::cout << "GAME OVER! Final Score: " << score << std::endl;
            score = 0;
            obstacleX = 1.2f;
            obstacleSpeed = 0.015f; // Reset difficulty
        }

        // --- RENDER ---
        glClearColor(0.02f, 0.02f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);
        int colorLoc = glGetUniformLocation(shaderProgram, "ourColor");
        int xLoc = glGetUniformLocation(shaderProgram, "xOffset");
        int yLoc = glGetUniformLocation(shaderProgram, "yOffset");

        glBindVertexArray(VAO);

        // Draw Player - Blue/Cyan)
        glUniform4f(colorLoc, 0.0f, 0.8f, 1.0f, 1.0f);
        glUniform1f(xLoc, playerX);
        glUniform1f(yLoc, playerY - 0.5f); // Move everything to floor level
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Draw Obstacle (Runtime Error - Red)
        glUniform4f(colorLoc, 1.0f, 0.2f, 0.2f, 1.0f);
        glUniform1f(xLoc, obstacleX);
        glUniform1f(yLoc, -0.5f); // Stay on floor
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    glfwTerminate();
    return 0;
}