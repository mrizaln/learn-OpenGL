// glad
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

// stb_image
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image/stb_image.h>

// GLM
#include <glm/glm.hpp>                      // ⎫
#include <glm/gtc/matrix_transform.hpp>     // ⎬ for transformations
#include <glm/gtc/type_ptr.hpp>             // ⎭

// shader
#include <shader_header/shader.h>

// STL
#include <string>
#include <array>        // for std::size


//===========================================================================================================
//===========================================================================================================

// [ forward declarations ]

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset);
void mouse_callback(GLFWwindow* window, double xPos, double yPos);
void processInput(GLFWwindow *window);
void updateDeltaTime();


/**
 * @brief 
 * 
 */

//===========================================================================================================
//===========================================================================================================

// [ configurations ]

namespace globals
{
    const int SCR_WIDTH { 800 };
    const int SCR_HEIGHT { 600 };
    float ASPECT_RATIO { SCR_WIDTH / static_cast<float>(SCR_HEIGHT) };

    /**
     * @brief 
     * deltaTime is used to keep track of the time it took to render the last frame.
     * we can use this to take care of unstable movement speed (because of hardware issue for example)
     * by multiplying the movement speed with this deltaTime.
     */
    float deltaTime { 0.0f };
    float lastFrame { 0.0f };
}

namespace camera
{
    float fov { 45.0f };

    // look around
    float pitch {};             // altitude (angular) (if in physics: pitch = 2pi - theta)
    float yaw { -90.0f };       // azimuth (if in physics: yaw = phi)
    glm::vec3 direction { 0.0f, 0.0f, -1.0f };
    
    /**
     * @brief 
     * NDC is left handed:
     *  cross(x,y) == -z,
     *  cross(y,z) == -x,
     *  cross(z,x) == -y
     */
    glm::vec3 cameraPos { 0.0f, 0.0f, 3.0f };
    glm::vec3 cameraUp { 0.0f, 1.0f, 0.0f };                                        // y
    glm::vec3 cameraFront { glm::normalize(direction) };                            // z
    glm::vec3 cameraRight { glm::normalize(glm::cross(cameraFront, cameraUp)) };    // x

    const float cameraSpeed { 2.5f };  // adjust accordingly

    void updateCamera()
    {
        direction.y = sin(glm::radians(pitch));
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFront = glm::normalize(direction);
        cameraRight = glm::normalize(glm::cross(cameraFront, cameraUp));
    }
}

namespace mouse
{
    float lastX {};        // center of screen
    float lastY {};        // center of screen
    const float sensitivity { 0.1f };

    // handle the issue of sudden movement of camera when mouse first captured
    bool firstMouse { true };
}


//===========================================================================================================
//===========================================================================================================

// [ main program ]

int main()
{
    // initialize glfw
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw window creation
    GLFWwindow *window = glfwCreateWindow(globals::SCR_WIDTH, globals::SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cerr << "Failed to Create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // set framebuffer size callback
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // set input mode to capture mouse and callback
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);

    // set scroll callback
    glfwSetScrollCallback(window, scroll_callback);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))        // bool == 0 if success
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    // enable depth testing
    glEnable(GL_DEPTH_TEST);

    // build and compile shader
    Shader theShader("shader.vs", "shader.fs");

    // vertex data
    float vertices[] {
        // positions                // texture coords
        -0.5f, -0.5f, -0.5f,        0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,        1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,        1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,        1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,        0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,        0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,        0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,        1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,        1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,        1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,        0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,        0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,        1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,        1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,        0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,        0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,        0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,        1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,        1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,        1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,        0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,        0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,        0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,        1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,        0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,        1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,        1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,        1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,        0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,        0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,        0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,        1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,        1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,        1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,        0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,        0.0f, 1.0f
    };

    glm::vec3 cubePositions[] {
        glm::vec3( 0.0f,  0.0f,  0.0f), 
        glm::vec3( 2.0f,  5.0f, -15.0f), 
        glm::vec3(-1.5f, -2.2f, -2.5f),  
        glm::vec3(-3.8f, -2.0f, -12.3f),  
        glm::vec3( 2.4f, -0.4f, -3.5f),  
        glm::vec3(-1.7f,  3.0f, -7.5f),  
        glm::vec3( 1.3f, -2.0f, -2.5f),  
        glm::vec3( 1.5f,  2.0f, -2.5f), 
        glm::vec3( 1.5f,  0.2f, -1.5f), 
        glm::vec3(-1.3f,  1.0f, -1.5f)  
    };

    // setting up buffer(s)
    unsigned int VAO;
    unsigned int VBO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // texture attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2);


/*===========================================================================================================
                                    ============[ Texture ]============    
-----------------------------------------------------------------------------------------------------------*/
    
    // generate texture (ID)
    unsigned int textureID[2];
    glGenTextures(2, textureID);

    // texture 0
    //----------

    // bind texture
    glBindTexture(GL_TEXTURE_2D, textureID[0]);

    // set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // load image (texture 0)
    int imageWidth, imageHeight, nrChannels;
    stbi_set_flip_vertically_on_load(true);     // fix flipped image when loaded
    unsigned char* imageData { stbi_load("img/container.jpg", &imageWidth, &imageHeight, &nrChannels, 0) };
    if (imageData)
    {
        // now generate texture from image
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageWidth, imageHeight, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        // fail
        std::cerr << "Failed to load texture 0\n" ;
    }
    stbi_image_free(imageData);

    // texture 1
    //----------
    
    // bind texture
    glBindTexture(GL_TEXTURE_2D, textureID[1]);

    // set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // load image (texture 0)
    imageData = stbi_load("img/awesomeface.png", &imageWidth, &imageHeight, &nrChannels, 0);
    if (imageData)
    {
        // now generate texture from image
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, imageWidth, imageHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        // fail
        std::cerr << "Failed to load texture 1\n" ;
    }
    stbi_image_free(imageData);


    // tell opengl for each sampler to which texture unit it belongs to
    theShader.use();
    theShader.setInt("texture0", 0);
    theShader.setInt("texture1", 1);
    theShader.setFloat("alpha", 0.2f);     // third uniform

//===========================================================================================================

/*===========================================================================================================
                ============[ Transformations (Coordinate Systems and Camera) ]============
-----------------------------------------------------------------------------------------------------------*/

    // model matrix
    glm::mat4 model(1.0f);

    // view matrix
    glm::mat4 view(1.0f);

    // projection matrix
    glm::mat4 projection { glm::perspective(glm::radians(45.0f), globals::ASPECT_RATIO , 0.1f, 100.0f) };

//===========================================================================================================

    // render loop
    while (!glfwWindowShouldClose(window))
    {
        // input
        processInput(window);

        // render
        glClearColor(0.2f, 0.3f, 0.4f, 1.0f);

        // clear color buffer and depth buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // bind texture to corresponding texture units
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureID[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textureID[1]);

        glBindVertexArray(VAO);

        // ------------------[ transformations ]------------------
        // camera is handled in namespace camera
        view = glm::lookAt(camera::cameraPos, camera::cameraFront + camera::cameraPos, camera::cameraUp);
        theShader.setMat4("view", view);
        
        // projection matrix changes a lot because of the aspect ratio, so we'll update it
        projection = glm::perspective(glm::radians(camera::fov), globals::ASPECT_RATIO, 0.1f, 100.0f);
        theShader.setMat4("projection", projection);
        
        // model matrix is applied 10 times to create 10 cubes
        for (int i { 1 }; const auto& vec : cubePositions)
        {
            model = glm::translate(glm::mat4(1.0f), vec);
            float angle { glm::radians(i*20.0f*static_cast<float>(glfwGetTime())) };
            // glm::vec3 axis { glm::vec3( sin(glfwGetTime()+1.0f), sin(glfwGetTime()+0.3f), sin(glfwGetTime()+0.5f) ) };
            glm::vec3 axis { glm::vec3(1.0f, 0.3f, 0.5f) };
            model = glm::rotate(model, angle, axis);
            theShader.setMat4("model", model);
            ++i;
            glDrawArrays(GL_TRIANGLES, 0, std::size(vertices));
        }
        //-------------------------------------------------------

        glfwSwapBuffers(window);
        glfwPollEvents();
        updateDeltaTime();
    }

    // de-allocate all resources
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

    // clearing all previously allocated GLFW resources.
    glfwTerminate();
    return 0;
}




//===========================================================================================================
//===========================================================================================================

// [ forwarded functions definition ]

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
    globals::ASPECT_RATIO = width / static_cast<float>(height);
    // std::cout << "aspect ratio: " << globals::ASPECT_RATIO << '\n';
}

void mouse_callback(GLFWwindow* window, double xPos, double yPos)
{
    if (mouse::firstMouse)
    {
        mouse::lastX = xPos;
        mouse::lastY = yPos;
        mouse::firstMouse = false;
    }

    // 1. calculate offset
    float xOffset { static_cast<float>(xPos) - mouse::lastX };
    float yOffset { mouse::lastY - static_cast<float>(yPos) };  // reversed since y-coordinates range from borrom to top (?)

    mouse::lastX = xPos;
    mouse::lastY = yPos;

    xOffset *= mouse::sensitivity;
    yOffset *= mouse::sensitivity;

    // 2. add last offset
    camera::yaw   += xOffset;
    camera::pitch += yOffset;

    // 3. constraints
    if (camera::pitch >  89.0f) camera::pitch =  89.0f;
    if (camera::pitch < -89.0f) camera::pitch = -89.0f;

    // 4. calculate the direction vector
    camera::updateCamera();
}

void scroll_callback(GLFWwindow* window, double xOffset, double yOffset)
{
    camera::fov -= static_cast<float>(yOffset);
    if (camera::fov <  1.0f) camera::fov =  1.0f;
    if (camera::fov > 45.0f) camera::fov = 45.0f;
}

void updateDeltaTime()
{
    float currentFrame { static_cast<float>(glfwGetTime()) };
    globals::deltaTime = currentFrame - globals::lastFrame;
    globals::lastFrame = currentFrame;
}

void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float speed { camera::cameraSpeed * globals::deltaTime };
        // update cameraSpeed by multipying it with globals::deltaTime
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
        speed *= 2;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera::cameraPos += speed * camera::cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera::cameraPos -= speed * camera::cameraFront;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera::cameraPos += speed * camera::cameraRight;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera::cameraPos -= speed * camera::cameraRight;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera::cameraPos += speed * camera::cameraUp;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera::cameraPos -= speed * camera::cameraUp;    
}