#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader_m.h>
#include <learnopengl/model.h>
#include <stb_image.h>
#include <rg/Texture2D.h>
#include <rg/Shader.h>
#include <iostream>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
// settings

const unsigned int SCR_WIDTH = 1980;
const unsigned int SCR_HEIGHT = 1024;

//firefly lightt
glm::vec3 lightColor = glm::vec3(0.7f);
glm::vec3 lightPosition = glm::vec3(1.0f ,0.5f,  -1.0f);

//sunlight
glm::vec3 sunLightDirection = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(75.0f), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(-1.0f, -2.0f, -1.0f, 1.0f));
glm::vec3 sunLightColor = glm::vec3(0.2f);

//sky color
glm::vec3 skyColor = glm::vec3(0.2, 0.5, 0.4);

//cullface - press c to change
bool cullFaceEnabled = true;

//beams - press l to cast
bool beams = false;

//rocks instancing
unsigned int amount = 500;
glm::mat4* modelMatrices = new glm::mat4[amount];
float radius = 80.0;
float offset = 35.0f;
unsigned int buffer;

//camera
glm::vec3 cameraPos = glm::vec3(0.0, 1.0, 4.0);
glm::vec3 cameraFront = glm::vec3(0.0, 0.0, -1.0);
glm::vec3 cameraUp = glm::vec3(0.0, 1.0, 0.0);
float cameraSpeedParameter = 10.0;

//light distance constants
float lightConst = 1.0f;
float linearConst = 0.08;
float quadraticConst = 0.032f;

//spotlight
glm::vec3 spotlightColor = glm::vec3(1.0f);
int spotLightFlag = 1;

//day and night
bool stop = false;

//timing
float delta_time = 0.0f;

float last_frame = 0.0f;
float lastX = SCR_WIDTH / 2.0;

float lastY = SCR_HEIGHT / 2.0;
bool firstMouse = true;
float yaw = -90.0f;
float pitch = 0.0f;
float fov = 45.0f;
// -------------------------------------------------------


void renderBackpack(shader backpackShader, Model backpackModel, glm::mat4 view, glm::mat4 projection);
void renderRocks(shader rockShader, Model rockModel, glm::mat4 view, glm::mat4 projection);
void generateRocks(Model rockModel);
void renderPyramid(Shader pyramidShader, Texture2D pyramidTexture, unsigned VAO, glm::mat4 model, glm::mat4 view, glm::mat4 projection);
void renderGround(Shader groundShader, Texture2D groundTexture, std::string texUniformName, unsigned int VAO, glm::mat4 view,
                  glm::mat4 projection);
void renderFirefly(Shader fireflyShader, unsigned VAO, glm::mat4 view, glm::mat4 projection);
void renderBox(Shader boxShader, unsigned VAO, Texture2D woodTexture, std::string woodTexUniformName,
               Texture2D metalTexture, std::string metalTexUniformName, glm::mat4 model, glm::mat4 view, glm::mat4 projection);
void renderBoxes(Shader boxShader, unsigned VAO, Texture2D woodTexture, Texture2D metalTexture, glm::mat4 view, glm::mat4 projection);
void renderBeams(Shader obeliskShader, unsigned VAO, glm::mat4 view, glm::mat4 projection);

void renderPyramids(Shader pyramidShader, unsigned VAO, Texture2D pyramidTexture, glm::mat4 view, glm::mat4 projection);

void initLoop();

void renderScene(Shader pyramidShader, Texture2D pyramidTexture,
                 Shader groundShader, Texture2D groundTexture, unsigned VAOs[],
                 Shader fireflyShader, unsigned cubeVAO,
                 Shader boxShader, Texture2D woodTexture, Texture2D metalTexture,
                 Shader obeliskShader,
                 shader backpackShader, Model backpackModel,
                 shader rockShader, Model rockModel,
                 glm::mat4 view, glm::mat4 projection);

int main() {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", glfwGetPrimaryMonitor(), NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetCursorPosCallback(window, mouse_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    float pyramid[] = {
        -0.5, 0.0, -0.5, 0.0, 0.0,  -1.25f, 1.25f, 0.0f,//bottom-left 0
        -0.5, 0.0, 0.5, 1.0, 0.0, -1.25f, 1.25f, 0.0f,//bottom-right 1
        0.0, 0.5, 0.0, 0.5, 1.0, -1.25f, 1.25f, 0.0f,//peek 4

        -0.5, 0.0, 0.5, 0.0, 0.0,   0.0f, 1.25f, 1.25f,//bottom-right 1
        0.5, 0.0, 0.5, 1.0, 0.0,   0.0f, 1.25f, 1.25f,//top-right 2
        0.0, 0.5, 0.0, 0.5, 1.0,  0.0f, 1.25f, 1.25f,//peek 4

        0.5, 0.0, 0.5, 0.0, 0.0,  1.25f, 1.25f, 0.0f,//top-right 2
        0.5, 0.0, -0.5, 1.0, 0.0, 1.25f, 1.25f, 0.0f,//top-left 3
        0.0, 0.5, 0.0, 0.5, 1.0,  1.25f, 1.25f, 0.0f,//peek 4

        0.5, 0.0, -0.5, 0.0, 0.0,  0.0f, 1.25f, -1.25f,//top-left 3
        -0.5, 0.0, -0.5, 1.0, 0.0, 0.0f, 1.25f, -1.25f,//bottom-left 0
        0.0, 0.5, 0.0, 0.5, 1.0, 0.0f, 1.25f, -1.25f//peek 4
    };

    float ground[] = {
        150.0f, 0.0f,  150.0f,  150.0f, 0.0f,
        -150.0f, 0.0f,  150.0f,  0.0f, 0.0f,
        -150.0f, 0.0f, -150.0f,  0.0f, 150.0f,

        150.0f, 0.0f,  150.0f,  150.0f, 0.0f,
        -150.0f, 0.0f, -150.0f,  0.0f, 150.0f,
        150.0f, 0.0f, -150.0f,  150.0f, 150.0f
    };

    float cube [] = {
        // positions          // normals           // texture coords
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,
        0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,
        0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f,

        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,
        0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f,
        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f,
        0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,
        0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f,
        0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
        0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f,
        0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
        0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f
    };

    unsigned int cubeVBO, cubeVAO;

    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);

    glBindVertexArray(cubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube), cube, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float ), 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    Shader obeliskShader = Shader(FileSystem::getPath("resources/shaders/obelisk.vert"), FileSystem::getPath("resources/shaders/obelisk.frag"));

    Shader fireflyShader = Shader(FileSystem::getPath("resources/shaders/cube.vert"), FileSystem::getPath("resources/shaders/cube.frag"));
    fireflyShader.use();
    fireflyShader.setVec3("lightColor", lightColor);

    Shader boxShader = Shader(FileSystem::getPath("resources/shaders/sanduk.vert"), FileSystem::getPath("resources/shaders/sanduk.frag"));
    boxShader.use();

    //Vertex Buffer Object & Vertex Array Object
    unsigned VBOs[2], VAOs[2];

    glGenVertexArrays(2, VAOs);
    glGenBuffers(2, VBOs);

    //pyramid
    glBindVertexArray(VAOs[0]);

    glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pyramid), pyramid, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void * )(5* sizeof(float)));
    glEnableVertexAttribArray(2);
    //sand
    glBindVertexArray(VAOs[1]);

    glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ground), ground, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    //Create shaders
    Shader pyramidShader = Shader(FileSystem::getPath("resources/shaders/pyramid.vert"), FileSystem::getPath("/resources/shaders/pyramid.frag"));
    Shader groundShader = Shader (FileSystem::getPath("resources/shaders/ground_shader.vert"),FileSystem::getPath("resources/shaders/ground_shader.frag"));

//    Pyramid texture
    Texture2D pyramidTexture = Texture2D(GL_REPEAT, GL_REPEAT, GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_NEAREST);
    pyramidTexture.load(FileSystem::getPath("resources/textures/pyramid_2.jpg"), true);
    pyramidTexture.reflect_vertically();
    pyramidTexture.free_data();

//    Sand texture
    Texture2D groundTexture = Texture2D(GL_REPEAT, GL_REPEAT, GL_NEAREST_MIPMAP_NEAREST, GL_NEAREST_MIPMAP_NEAREST);
    groundTexture.load(FileSystem::getPath("/resources/textures/sand.jpg"), true);
    groundTexture.reflect_vertically();
    groundTexture.free_data();

//    wood texture
    Texture2D woodTexture = Texture2D(GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR);
    woodTexture.load(FileSystem::getPath("resources/textures/container2.png"), true);
    woodTexture.reflect_vertically();
    woodTexture.free_data();

    // metal texture
    Texture2D metalTexture = Texture2D(GL_REPEAT, GL_REPEAT, GL_LINEAR, GL_LINEAR);
    metalTexture.load(FileSystem::getPath("resources/textures/container2_specular.png"), false);
    metalTexture.reflect_vertically();
    metalTexture.free_data();

    //Initial color of background

    //Enabling depth testing
    stbi_set_flip_vertically_on_load(true);
    glEnable(GL_DEPTH_TEST);

    shader backpackShader("resources/shaders/model_loading.vs",
                       "resources/shaders/model_loading.fs");

//    backpackShader.use();

    Model backpackModel(FileSystem::getPath("resources/objects/backpack/backpack.obj"));

    //rock loading

    shader rockShader("resources/shaders/rock.vs",
                      "resources/shaders/rock.fs");

    Model rockModel(FileSystem::getPath("resources/objects/rock/Rock1/Rock1.obj"));

    //Rendering loop
//    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    generateRocks(rockModel);

    while(!glfwWindowShouldClose(window)){
        initLoop();
        processInput(window);

        //view and projection matrices
        glm::mat4 view = glm::lookAt(cameraPos , cameraFront + cameraPos, cameraUp);
        glm::mat4 projection = glm::perspective(glm::radians(fov), (float)SCR_WIDTH/SCR_HEIGHT, 0.1f, 1000.0f);

        //render scene
        renderScene(pyramidShader, pyramidTexture,
                    groundShader, groundTexture, VAOs,
                    fireflyShader, cubeVAO,
                    boxShader, woodTexture, metalTexture,
                    obeliskShader, backpackShader, backpackModel,
                    rockShader, rockModel,
                    view, projection);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void renderScene(Shader pyramidShader, Texture2D pyramidTexture,
                 Shader groundShader, Texture2D groundTexture, unsigned VAOs[],
                 Shader fireflyShader, unsigned cubeVAO,
                 Shader boxShader, Texture2D woodTexture, Texture2D metalTexture,
                 Shader obeliskShader,
                 shader backpackShader, Model backpackModel,
                 shader rockShader, Model rockModel,
                 glm::mat4 view, glm::mat4 projection) {
    //render pyramids
    renderPyramids(pyramidShader, VAOs[0], pyramidTexture, view, projection);

    //render ground
    renderGround(groundShader, groundTexture, "sand_texture", VAOs[1], view, projection);

    //render firefly
    renderFirefly(fireflyShader, cubeVAO, view, projection);

    //render boxes
    renderBoxes(boxShader, cubeVAO, woodTexture, metalTexture, view, projection);

    //render laser beams
    renderBeams(obeliskShader, cubeVAO, view, projection);

    //render model backpack
    renderBackpack(backpackShader, backpackModel, view, projection);

    //render model rock
    renderRocks(rockShader, rockModel, view, projection);
}

void initLoop() {
    glClearColor(skyColor.x, skyColor.y, skyColor.z, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float radius = 3.0f;
    lightPosition = glm::vec3(cos(glfwGetTime())*radius  ,0.5,  sin(glfwGetTime())*radius);


    //frame-time logic
    float current_frame = glfwGetTime();
    delta_time = current_frame - last_frame;
    last_frame = current_frame;
}

void renderPyramids(Shader pyramidShader, unsigned VAO, Texture2D pyramidTexture, glm::mat4 view, glm::mat4 projection) {

    // Create model matrix for super pyramid
    glm::mat4 modelSuperPyramid = glm::mat4(1.0f);
    modelSuperPyramid = glm::scale(modelSuperPyramid, glm::vec3(300.0f));

    //CULL FACE enabled for small pyrmid
    bool flag = false;
    if(cullFaceEnabled){
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
        flag = true;
    }

    //render small pyramid
    renderPyramid(pyramidShader, pyramidTexture, VAO, modelSuperPyramid, view, projection);

    // Create model matrix for small pyramid
    glm::mat4 modelSmallPyramid = glm::mat4(1.0f);
    modelSmallPyramid = glm::translate(modelSmallPyramid, glm::vec3(2.0f, 0.0f, 0.0f));
    modelSmallPyramid = glm::scale(modelSmallPyramid, glm::vec3(2.0f, 2.0f, 2.0f));

    renderPyramid(pyramidShader, pyramidTexture, VAO, modelSmallPyramid, view, projection);

    //DISABLING CULL FACE for small pyramid and super pyramid
    if(flag){
        glDisable(GL_CULL_FACE);
    }

    //render big pyramid
    glm::mat4 modelBigPyramid = glm::mat4(1.0f);
    modelBigPyramid = glm::translate(modelBigPyramid, glm::vec3(5.0f, 0.0f, -5.0f));
    modelBigPyramid = glm::rotate(modelBigPyramid, glm::radians(7.0f) ,glm::vec3(0.0f, 1.0f, 0.0f));
    modelBigPyramid = glm::scale(modelBigPyramid, glm::vec3(4.0f, 4.0f, 4.0f));

    renderPyramid(pyramidShader, pyramidTexture, VAO, modelBigPyramid, view, projection);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    const float cameraSpeed = cameraSpeedParameter * delta_time;

    //da ne ide kamera ispod y ose
    if (cameraPos.y  < 0.3f)
    {
        cameraPos.y = 0.3f;
    }

    if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
        cameraPos += cameraFront * cameraSpeed;
    }

    if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
        cameraPos -= cameraFront * cameraSpeed;
    }

    if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }

    if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {

    if(key == GLFW_KEY_F && action == GLFW_PRESS){
        spotLightFlag = 1 - spotLightFlag;
    }

    if(key == GLFW_KEY_L && action == GLFW_PRESS){
        beams = !beams;
    }

    if(key == GLFW_KEY_UP && action == GLFW_PRESS){
        if(cameraSpeedParameter >= 100.0){
            cameraSpeedParameter = 100.0;
        }
        else{
            cameraSpeedParameter += 10.0;
        }
    }

    if(key == GLFW_KEY_DOWN && action == GLFW_PRESS){
        if(cameraSpeedParameter <= 5.0){
            cameraSpeedParameter = 5.0;
        }
        else{
            cameraSpeedParameter -= 10.0;
        }
    }

    if(key == GLFW_KEY_C && action == GLFW_PRESS){
        cullFaceEnabled = !cullFaceEnabled;
    }

}
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f; // change this value to your liking
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
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

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    fov -= (float)yoffset;
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 45.0f)
        fov = 45.0f;
}

void renderBackpack(shader backpackShader, Model backpackModel, glm::mat4 view, glm::mat4 projection){
    //Model
    glm::mat4 model_model = glm::mat4(1.0f);
    model_model = glm::translate(model_model, glm::vec3(1.4, 0.1, -1.95));
    model_model = glm::rotate(model_model, (float)glm::radians(-25.0f), glm::vec3(1.0, 0.0, 1.0));
    model_model = glm::rotate(model_model, (float)glm::radians(-55.0f), glm::vec3(0.0, 1.0, 0.0));
    model_model = glm::scale(model_model, glm::vec3(0.05));

    backpackShader.use();
    backpackShader.setMat4("model", model_model);
    backpackShader.setMat4("view", view);
    backpackShader.setMat4("projection", projection);

    //spotLight for model
    backpackShader.setFloat("spotLight.lightConst", lightConst);
    backpackShader.setFloat("spotLight.linearConst", linearConst);
    backpackShader.setFloat("spotLight.quadraticConst", quadraticConst);
    backpackShader.setInt("spotLight.spotLightFlag", spotLightFlag);
    backpackShader.setVec3("spotLight.position", cameraPos);
    backpackShader.setVec3("spotLight.direction", cameraFront);
    backpackShader.setVec3("spotLight.color", glm::vec3 (1.0f));
    backpackShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(10.0f)));
    backpackShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(12.5f)));

    //firefly
    backpackShader.setFloat("pointLight.lightConst", lightConst);
    backpackShader.setFloat("pointLight.linearConst", linearConst);
    backpackShader.setFloat("pointLight.quadraticConst", quadraticConst);
    backpackShader.setVec3("pointLight.position", lightPosition);
    backpackShader.setVec3("pointLight.color", lightColor);

    backpackShader.setVec3("viewPos", cameraPos);

    //sun light
    backpackShader.setVec3("dirLight.direction", sunLightDirection);
    backpackShader.setVec3("dirLight.color", sunLightColor);
    backpackModel.Draw(backpackShader);
}

void generateRocks(Model rockModel){

    srand(glfwGetTime()); // initialize random seed

    for (unsigned int i = 0; i < amount; i++)
    {
        glm::mat4 model = glm::mat4(1.0f);
        // 1. translation: displace along circle with 'radius' in range [-offset, offset]
        float angle = (float)i / (float)amount * 360.0f;
        float displacement = (rand() % (int)(2 * offset * 100)) / 100.0f - offset;
        float x = sin(angle) * radius + displacement;
        float z = cos(angle) * radius + displacement;
        model = glm::translate(model, glm::vec3(x, -0.01, z));

        // 2. scale: Scale between 0.01 and 0.10f
        float scale = (rand() % 20) / 250.0f + 0.01;
        model = glm::scale(model, glm::vec3(scale));

        // 3. rotation: add random rotation around a (semi)randomly picked rotation axis vector
        float rotAngle = (rand() % 360);
        model = glm::rotate(model, rotAngle, glm::vec3(0.0f, 1.0f, 0.0f));

        // 4. now add to list of matrices
        modelMatrices[i] = model;
    }

    // configure instanced array
    // -------------------------
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, amount * sizeof(glm::mat4), &modelMatrices[0], GL_STATIC_DRAW);

    // set transformation matrices as an instance vertex attribute (with divisor 1)
    // note: we're cheating a little by taking the, now publicly declared, VAO of the model's mesh(es) and adding new vertexAttribPointers
    // normally you'd want to do this in a more organized fashion, but for learning purposes this will do.
    // -----------------------------------------------------------------------------------------------------------------------------------
    for (unsigned int i = 0; i < rockModel.meshes.size(); i++)
    {
        unsigned int VAO = rockModel.meshes[i].VAO;
        glBindVertexArray(VAO);
        // set attribute pointers for matrix (4 times vec4)
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(sizeof(glm::vec4)));
        glEnableVertexAttribArray(5);
        glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(2 * sizeof(glm::vec4)));
        glEnableVertexAttribArray(6);
        glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(3 * sizeof(glm::vec4)));

        glVertexAttribDivisor(3, 1);
        glVertexAttribDivisor(4, 1);
        glVertexAttribDivisor(5, 1);
        glVertexAttribDivisor(6, 1);

        glBindVertexArray(0);
    }
}

void renderRocks(shader rockShader, Model rockModel, glm::mat4 view, glm::mat4 projection) {

    rockShader.use();

    rockShader.setFloat("spotLight.lightConst", lightConst);
    rockShader.setFloat("spotLight.linearConst", linearConst);
    rockShader.setFloat("spotLight.quadraticConst", quadraticConst);
    rockShader.setInt("spotLight.spotLightFlag", spotLightFlag);
    rockShader.setVec3("spotLight.position", cameraPos);
    rockShader.setVec3("spotLight.direction", cameraFront);
    rockShader.setVec3("spotLight.color", glm::vec3 (1.0f));
    rockShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(10.0f)));
    rockShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(12.5f)));
    rockShader.setVec3("lightColor", lightColor);
    rockShader.setVec3("viewPos", cameraPos);

    rockShader.setMat4("projection", projection);
    rockShader.setMat4("view", view);
    rockShader.setVec3("dirLight.direction", sunLightDirection);
    rockShader.setVec3("dirLight.color", sunLightColor);
    rockShader.setVec3("viewPos", cameraPos);
    rockShader.setInt("texture_diffuse1", 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, rockModel.textures_loaded[0].id); // note: we also made the textures_loaded vector public (instead of private) from the model class.
    for (unsigned int i = 0; i < rockModel.meshes.size(); i++)
    {
        glBindVertexArray(rockModel.meshes[i].VAO);
        glDrawElementsInstanced(GL_TRIANGLES, rockModel.meshes[i].indices.size(), GL_UNSIGNED_INT, 0, amount);
        glBindVertexArray(0);
    }
}

void renderPyramid(Shader pyramidShader, Texture2D pyramidTexture, unsigned VAO, glm::mat4 model, glm::mat4 view, glm::mat4 projection) {
    //Set matrices for pyramid
    pyramidShader.use();
    pyramidShader.setMat4("model", model);
    pyramidShader.setMat4("view", view);
    pyramidShader.setMat4("projection", projection);

    //viewPos
    pyramidShader.setVec3("viewPos", cameraPos);

    //spotLight specification
    pyramidShader.setFloat("spotLight.lightConst", lightConst);
    pyramidShader.setFloat("spotLight.linearConst", linearConst);
    pyramidShader.setFloat("spotLight.quadraticConst", quadraticConst);
    pyramidShader.setInt("spotLight.spotLightFlag", spotLightFlag);
    pyramidShader.setVec3("spotLight.position", cameraPos);
    pyramidShader.setVec3("spotLight.direction", cameraFront);
    pyramidShader.setVec3("spotLight.color", glm::vec3 (1.0f));
    pyramidShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(10.0f)));
    pyramidShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(12.5f)));

//        //sunLight specification
    pyramidShader.setVec3("dirLight.direction", sunLightDirection);
    pyramidShader.setVec3("dirLight.color", sunLightColor);
//
//        //bug1 specification
    pyramidShader.setFloat("pointLight.lightConst", lightConst);
    pyramidShader.setFloat("pointLight.linearConst", linearConst);
    pyramidShader.setFloat("pointLight.quadraticConst", quadraticConst);
    pyramidShader.setVec3("pointLight.position", lightPosition);
    pyramidShader.setVec3("pointLight.color", lightColor);

    //pyramid texture
    pyramidShader.setInt("texture_pyramid", 0);
    pyramidTexture.activate(GL_TEXTURE0);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 12);

    glBindVertexArray(0);
}

void renderGround(Shader groundShader, Texture2D groundTexture, std::string texUniformName ,unsigned int VAO, glm::mat4 view,
                  glm::mat4 projection) {

    glm::mat4 model = glm::mat4(1.0f);

    groundShader.use();
    groundShader.setMat4("model", model);
    groundShader.setMat4("view", view);
    groundShader.setMat4("projection", projection);

    //viewPos
    groundShader.setVec3("viewPos", cameraPos);

    //sun light (directional light)
    groundShader.setVec3("dirLight.direction", sunLightDirection);
    groundShader.setVec3("dirLight.color", sunLightColor);

    //bug light (point light)
    groundShader.setFloat("pointLight.lightConst", lightConst);
    groundShader.setFloat("pointLight.linearConst", linearConst);
    groundShader.setFloat("pointLight.quadraticConst", quadraticConst);
    groundShader.setVec3("pointLight.position", lightPosition);
    groundShader.setVec3("pointLight.color", lightColor);

    //spotlight
    groundShader.setFloat("spotLight.lightConst", lightConst);
    groundShader.setFloat("spotLight.linearConst", linearConst);
    groundShader.setFloat("spotLight.quadraticConst", quadraticConst);
    groundShader.setInt("spotLight.spotLightFlag", spotLightFlag);
    groundShader.setVec3("spotLight.position", cameraPos);
    groundShader.setVec3("spotLight.direction", cameraFront);
    groundShader.setVec3("spotLight.color", glm::vec3 (1.0f));
    groundShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(10.0f)));
    groundShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(12.5f)));

    // texture activation
    groundShader.setInt(texUniformName, 0);
    groundTexture.activate(GL_TEXTURE0);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

void renderFirefly(Shader fireflyShader, unsigned VAO, glm::mat4 view, glm::mat4 projection) {
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, lightPosition);
    model = glm::scale(model, glm::vec3(0.04f));

    fireflyShader.use();

    fireflyShader.setMat4("model", model);
    fireflyShader.setMat4("view", view);
    fireflyShader.setMat4("projection", projection);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}

void renderBeams(Shader obeliskShader, unsigned VAO, glm::mat4 view, glm::mat4 projection) {
    obeliskShader.use();

    //sun light (directional light)
    obeliskShader.setVec3("dirLight.direction", sunLightDirection);
    obeliskShader.setVec3("dirLight.color", sunLightColor);

    //bug light (point light)
    obeliskShader.setFloat("pointLight.lightConst", lightConst);
    obeliskShader.setFloat("pointLight.linearConst", linearConst);
    obeliskShader.setFloat("pointLight.quadraticConst", quadraticConst);
    obeliskShader.setVec3("pointLight.position", lightPosition);
    obeliskShader.setVec3("pointLight.color", lightColor);

    //spotlight
    obeliskShader.setFloat("spotLight.lightConst", lightConst);
    obeliskShader.setFloat("spotLight.linearConst", linearConst);
    obeliskShader.setFloat("spotLight.quadraticConst", quadraticConst);
    obeliskShader.setInt("spotLight.spotLightFlag", spotLightFlag);
    obeliskShader.setVec3("spotLight.position", cameraPos);
    obeliskShader.setVec3("spotLight.direction", cameraFront);
    obeliskShader.setVec3("spotLight.color", glm::vec3 (1.0f));
    obeliskShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(10.0f)));
    obeliskShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(12.5f)));

    obeliskShader.setVec3("material.ambient", glm::vec3(0.0215,	0.1745, 0.0215));
    obeliskShader.setVec3("material.diffuse", glm::vec3(0.07568, 0.61424, 0.07568));
    obeliskShader.setVec3("material.specular", glm::vec3(0.633, 0.727811, 0.633));
    obeliskShader.setFloat("material.shininess", 0.6);

    obeliskShader.setVec3("viewPos", lightPosition);

    glBindVertexArray(VAO);

    for (int i = 0; i < 12 && beams; i++) {

        float radius = 7.0f;
        glm::mat4 model_obelisk = glm::mat4(1.0f);
        float angle = 30.0f;

        model_obelisk = glm::translate(model_obelisk, glm::vec3(radius * glm::cos(glm::radians(i*angle)) + 5.0f, 0.0, radius * glm::sin(glm::radians(i*angle))-5.0f));
        model_obelisk = glm::scale(model_obelisk, glm::vec3(0.02f, 5000.0f,  0.02f));

        obeliskShader.setMat4("model", model_obelisk);
        obeliskShader.setMat4("view", view);
        obeliskShader.setMat4("projection", projection);

        glDrawArrays(GL_TRIANGLES, 0, 36);
    }

    glBindVertexArray(0);
}

void renderBoxes(Shader boxShader, unsigned VAO, Texture2D woodTexture, Texture2D metalTexture, glm::mat4 view, glm::mat4 projection) {
    //model
    glm::mat4 model_cube = glm::mat4(1.0f);
    model_cube = glm::translate(model_cube, glm::vec3(1.3, 0.12, -2.3));
    model_cube = glm::scale(model_cube, glm::vec3(0.2f));

    renderBox(boxShader, VAO, woodTexture, "material.diffuse", metalTexture, "material.specular", model_cube, view, projection);

    model_cube = glm::translate(model_cube, glm::vec3(1.1 , 0.0, 1.2));
    model_cube = glm::rotate(model_cube, glm::radians(29.0f), glm::vec3(0.0, 1.0, 0.0));

    renderBox(boxShader, VAO, woodTexture, "material.diffuse", metalTexture, "material.specular", model_cube, view, projection);

    model_cube = glm::translate(model_cube, glm::vec3(0.1 , 1.0, -0.15));
    model_cube = glm::rotate(model_cube, glm::radians(18.0f), glm::vec3(0.0, 1.0, 0.0));
    boxShader.setMat4("model", model_cube);
    boxShader.setMat4("view", view);
    boxShader.setMat4("projection", projection);

    renderBox(boxShader, VAO, woodTexture, "material.diffuse", metalTexture, "material.specular", model_cube, view, projection);
}

void renderBox(Shader boxShader, unsigned VAO, Texture2D woodTexture, std::string woodTexUniformName,
               Texture2D metalTexture, std::string metalTexUniformName, glm::mat4 model, glm::mat4 view, glm::mat4 projection) {
    boxShader.use();
    boxShader.setMat4("model", model);
    boxShader.setMat4("view", view);
    boxShader.setMat4("projection", projection);
    //viewPos
    boxShader.setVec3("viewPos", cameraPos);

    //sun light (directional light)
    boxShader.setVec3("dirLight.direction", sunLightDirection);
    boxShader.setVec3("dirLight.color", sunLightColor);

    //bug light (point light)
    boxShader.setFloat("pointLight.lightConst", lightConst);
    boxShader.setFloat("pointLight.linearConst", linearConst);
    boxShader.setFloat("pointLight.quadraticConst", quadraticConst);
    boxShader.setVec3("pointLight.position", lightPosition);
    boxShader.setVec3("pointLight.color", lightColor);

    //spotlight
    boxShader.setFloat("spotLight.lightConst", lightConst);
    boxShader.setFloat("spotLight.linearConst", linearConst);
    boxShader.setFloat("spotLight.quadraticConst", quadraticConst);
    boxShader.setInt("spotLight.spotLightFlag", spotLightFlag);
    boxShader.setVec3("spotLight.position", cameraPos);
    boxShader.setVec3("spotLight.direction", cameraFront);
    boxShader.setVec3("spotLight.color", glm::vec3 (1.0f));
    boxShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(10.0f)));
    boxShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(12.5f)));

    boxShader.setFloat("material.shininess", 16.0f);

    boxShader.setInt(woodTexUniformName, 0);
    woodTexture.activate(GL_TEXTURE0);

    boxShader.setInt(metalTexUniformName, 1);
    metalTexture.activate(GL_TEXTURE1);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
}