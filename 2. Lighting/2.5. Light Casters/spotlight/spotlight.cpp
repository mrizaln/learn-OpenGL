// glad
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h>

// GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// shader
#include <shader_header/shader.h>

// camera
#include <camera_header/camera.h>

// texture
#include <texture_header/texture.h>

// shapes
#include <shapes/sphere/sphere.h>
#include <shapes/cube/cube.h>

// materials
#include <material_header/material.h>    // include this for Material struct


// STL
#include <iostream>
#include <typeinfo>     // for typeid()

//===========================================================================================================


void framebuffer_size_callback(GLFWwindow*, int, int);
void cursor_position_callback(GLFWwindow*, double, double);
void scroll_callback(GLFWwindow*, double, double);
void key_callback(GLFWwindow*, int, int, int, int);

void processInput(GLFWwindow*);
void updateDeltaTime();
void changeAlpha(const Shader&, const std::string&);

//===========================================================================================================


namespace configuration
{
    constexpr int screenWidth{ 800 };
    constexpr int screenHeight{ 600 };
    float aspectRatio{ static_cast<float>(screenWidth)/screenHeight };
}

namespace timing
{
    float lastFrame{};
    float deltaTime{};
}

namespace mouse
{
    float lastX{};
    float lastY{};
    bool firstMouse { true };
    bool captureMouse{ true };
}


// create camera object
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));


// textured material
class TexturedMaterial
{
    // ambient should be the same as diffuse

    Texture diffuseMap{};
    Texture specularMap{};
    float shininess{};

public:
    auto& getDiffuseMap() { return diffuseMap; }
    auto& getSpecularMap() { return specularMap; }
    auto& getShininess() { return shininess; }
};


// object 
template <class object_type, class material_type = MaterialBasic>
class Object
{
    object_type object{};
    glm::vec3 position{};
    glm::vec3 scale{ 1.0f };
    Shader shader{};
    Material<material_type> material{};
    glm::mat4 modelMatrix{ glm::mat4(1.0f) };

public:
    Object(object_type obj, glm::vec3 objPos, Shader objShader, Material<material_type> material)
        : object{ obj }
        , position { objPos }
        , scale{ 1.0f }
        , shader{ objShader }
        , material{ material }
        , modelMatrix{ glm::mat4(1.0f) }
    {
    }

    void setPosition(glm::vec3& pos) { position = pos; }
    void setPosition(float x, float y, float z) { setPosition(glm::vec3{ x, y, z }); }
    void setScale(float scaling) { scale = scaling; }
    void setShader(Shader& shdr) { shader = shdr; }
    void setMaterial(Material<material_type>& mat) { material = mat; }

    auto& getObject() { return object; }
    auto& getPosition() { return position; }
    auto& getShader() { return shader; }
    auto& getMaterial() { return material; }
    auto& getModelMatrix() { updateModelMatrix(); return modelMatrix; }

    // apply material through shader
    void applyMaterial()
    {
        void* mat_void{ &material };

        if (typeid(material) == typeid(Material<MaterialBasic>))
        {
            Material<MaterialBasic> mat{ *((Material<MaterialBasic>*)mat_void) };

            shader.use();
            shader.setVec3("material.ambient",    mat.getAmbient());
            shader.setVec3("material.diffuse",    mat.getDiffuse());
            shader.setVec3("material.specular",   mat.getSpecular());
            shader.setFloat("material.shininess", mat.getShininess());
        }
        else if (typeid(material) == typeid(Material<MaterialTextured>))
        {
            Material<MaterialTextured> mat{ *((Material<MaterialTextured>*)mat_void) };

            shader.use();
            shader.setInt("material.diffuse",     mat.getDiffuse().textureUnitNum);
            shader.setInt("material.specular",    mat.getSpecular().textureUnitNum);
            shader.setFloat("material.shininess", mat.getShininess());
            
            // repurpose ambient as emission
            shader.setInt("material.emission",    mat.getAmbient().textureUnitNum);
        }
    }

    void applyTexture()
    {
        if (typeid(material) != typeid(Material<MaterialTextured>))
            return;

        void* mat_void{ &material };
        auto mat{ *((Material<MaterialTextured>*)mat_void) };

        // diffuse map
        glActiveTexture(GL_TEXTURE0 + mat.getDiffuse().textureUnitNum);
        glBindTexture(GL_TEXTURE_2D, mat.getDiffuse().textureID);

        // specular map
        glActiveTexture(GL_TEXTURE0 + mat.getSpecular().textureUnitNum);
        glBindTexture(GL_TEXTURE_2D, mat.getSpecular().textureID);

        // emissive map (from repurposed ambient map)
        glActiveTexture(GL_TEXTURE0 + mat.getAmbient().textureUnitNum);
        glBindTexture(GL_TEXTURE_2D, mat.getAmbient().textureID);
    }

private:
    void updateModelMatrix()
    {
        modelMatrix = glm::translate(glm::mat4(1.0f), position);
        modelMatrix = glm::scale(modelMatrix, scale);
    }
};


//===========================================================================================================


int main()
{
    // initialize GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // window creation
    GLFWwindow* window { glfwCreateWindow(configuration::screenWidth, configuration::screenHeight, "LearnOpenGL", NULL, NULL) };
    if (!window)
    {
        std::cerr << "Failed to create GLFW window";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // set callbacks
    //--------------
    // set framebuffer callback
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    // set glfw to capture cursor and set the callback
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    // set scroll callback
    glfwSetScrollCallback(window, scroll_callback);
    // set key callback
    glfwSetKeyCallback(window, key_callback);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))        // bool == 0 if success
    {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    // enable depth testing
    glEnable(GL_DEPTH_TEST);


    // create objects
    //---------------
    // a cube container object (will be cloned 10 times)
    Object<Cube, MaterialTextured> cube(
        Cube(0.5f),
        glm::vec3{ 0.0f,  0.0f,  0.0f},
        Shader("shader.vs", "shader.fs"),
        Material<MaterialTextured>{
            // Texture{"../../../resources/img/matrix.jpg"},      // ambient is repurposed as emission map
            Texture{},
            Texture{"../../../resources/img/container2.png"},
            Texture{"../../../resources/img/container2_specular_new.png"},
            32.0f
        }
    );

    // 10 positions of the cloned cubes
    glm::vec3 cubePositions[] = {
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


    // set material uniform in shader.fs;
    cube.applyMaterial();

    // set light uniforms
    cube.getShader().setVec3("light.ambient",  glm::vec3{  0.2f,  0.2f,  0.2f });
    cube.getShader().setVec3("light.diffuse",  glm::vec3{  0.5f,  0.5f,  0.5f });
    cube.getShader().setVec3("light.specular", glm::vec3{  1.0f,  1.0f,  1.0f });

    // set light attenuation constants
    cube.getShader().setFloat("light.constant",  1.0f);
    cube.getShader().setFloat("light.linear",    0.007f);
    cube.getShader().setFloat("light.quadratic", 0.0002f);


    //=======================================================================================================

    // render loop
    while (!glfwWindowShouldClose(window))
    {
        // input
        processInput(window);

        // render
        glClearColor(0.2f, 0.3f, 0.4f, 1.0f);

        // clear color buffer and depth buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        // container
        //----------
        // use shader
        cube.getShader().use();

        // apply texture
        cube.applyTexture();

        // set uniform
        cube.getShader().setVec3("viewPos", camera.position);
        
        // set spotlight
        cube.getShader().setVec3("light.position", camera.position);
        cube.getShader().setVec3("light.direction", camera.front);
        cube.getShader().setFloat("light.cutOff", glm::cos(glm::radians(12.5f)));       // 12.5 degrees of cutOff angle
        cube.getShader().setFloat("light.outerCutOff", glm::cos(glm::radians(17.5f)));       // 12.5 degrees of cutOff angle

        // view is handled by camera class
        auto view { camera.getViewMatrix() };
        cube.getShader().setMat4("view", view);
        
        // projection matrix changes a lot because of the aspect ratio, so we'll update it
        auto projection { glm::perspective(glm::radians(camera.fov), configuration::aspectRatio, 0.1f, 100.0f) };
        cube.getShader().setMat4("projection", projection);
        
        // applied 10 times
        for(unsigned int i = 0; i < 10; i++)
        {
            // model matrix
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, cubePositions[i]);
            float angle = 20.0f * i;
            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            cube.getShader().setMat4("model", model);

            // draw
            cube.getObject().draw();
        }
        //----------


        glfwSwapBuffers(window);
        glfwPollEvents();
        updateDeltaTime();
    }

    // clearing all previously allocated GLFW resources.
    // sphere.getObject().~Cube();
    glfwTerminate();
    return 0;
}

//===========================================================================================================


// window resize callback
void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
    configuration::aspectRatio = width / static_cast<float>(height);
    // std::cout << "aspect ratio: " << configuration::aspectRatio << '\n';
}

// cursor position callback
void cursor_position_callback(GLFWwindow* window, double xPos, double yPos)
{
    if (!mouse::captureMouse)
        return;

    if (mouse::firstMouse)
    {
        mouse::lastX = xPos;
        mouse::lastY = yPos;
        mouse::firstMouse = false;
    }

    float xOffset { static_cast<float>(xPos) - mouse::lastX };
    float yOffset { mouse::lastY - static_cast<float>(yPos) };

    camera.processMouseMovement(xOffset, yOffset);

    mouse::lastX = xPos;
    mouse::lastY = yPos;
}

// scroll callback
void scroll_callback(GLFWwindow* window, double xOffset, double yOffset)
{
    camera.processMouseScroll(static_cast<float>(yOffset));
}

// key press callback (for 1 press)
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // close window
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // toggle capture mouse
    if (key == GLFW_KEY_C && action == GLFW_PRESS)
    {
        // toggle
        mouse::captureMouse = !mouse::captureMouse;

        if (mouse::captureMouse)
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        else
        {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            mouse::firstMouse = true;
        }
    }

    // set camera target to (0,0,0)
    if (key == GLFW_KEY_BACKSPACE && action == GLFW_PRESS)
    {
        // std::cout << "Before: " << camera.front.x << ' ' << camera.front.y << ' ' << camera.front.z << " | " << camera.yaw << ' ' << camera.pitch << '\n';
        camera.lookAtOrigin();      // look at (0,0,0)
        mouse::firstMouse = true;
        // std::cout << "After: " << camera.front.x << ' ' << camera.front.y << ' ' << camera.front.z << " | " << camera.yaw << ' ' << camera.pitch << "\n\n";
    }
}

// for continuous input
void processInput(GLFWwindow* window)
{
    // camera movement
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.moveCamera(CameraMovement::FORWARD, timing::deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.moveCamera(CameraMovement::BACKWARD, timing::deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.moveCamera(CameraMovement::RIGHT, timing::deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.moveCamera(CameraMovement::LEFT, timing::deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.moveCamera(CameraMovement::UPWARD, timing::deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.moveCamera(CameraMovement::DOWNWARD, timing::deltaTime);

    // print fps
    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
        std::cout << "fps: " << static_cast<int>(1/timing::deltaTime) << '\n';
}

// record frame draw time
void updateDeltaTime()
{
    float currentFrame{ static_cast<float>(glfwGetTime()) };
    timing::deltaTime = currentFrame - timing::lastFrame;
    timing::lastFrame = currentFrame;
}
