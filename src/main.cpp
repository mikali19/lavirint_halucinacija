#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void mouse_callback(GLFWwindow *window, double xpos, double ypos);

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

void processInput(GLFWwindow *window);
void processInput1(GLFWwindow *window);

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

unsigned int loadCubemap(vector<std::string> faces);

unsigned int loadTexture(char const *path);

bool lineColision(glm::vec3 p1,glm::vec3 p2,glm::vec3 px);

bool circleColision(glm::vec3 c,glm::vec3 x,float r);

void renderQuad();
void renderQuad1();


// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
bool freeMove = false;
bool spotlightOn = true;
bool bg_change = false;
bool halutination = false;
bool bloom = true;
bool bloomKeyPressed = false;
// camera

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = false;

glm::vec3 lastCamPosition;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

struct PointLight {
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct ProgramState {
    glm::vec3 clearColor = glm::vec3(0);
    glm::vec3 dirLightDir = glm::vec3(-0.2f, -1.0f, -0.3f);
    glm::vec3 dirLightAmbDiffSpec = glm::vec3(0.01f,0.16f,0.03f);
    bool ImGuiEnabled = false;
    Camera camera;
    bool CameraMouseMovementUpdateEnabled = true;
    glm::vec3 vecCalibrate = glm::vec3(1.0f);
    glm::vec3 vecRotate = glm::vec3(0.0f);
    float fineCalibrate = 0.001f;
    //test
    vector<std::string> faces;
    unsigned int cubemapTexture;
    //-----
    PointLight pointLight;
    ProgramState()
            : camera(glm::vec3(0.0f, 0.0f, 3.0f)) {}

    void SaveToFile(std::string filename);

    void LoadFromFile(std::string filename);
};

void ProgramState::SaveToFile(std::string filename) { //Ovde se cuva pozicija pre iskljucivanja aplikacije
    std::ofstream out(filename);
    out << clearColor.r << '\n'
        << clearColor.g << '\n'
        << clearColor.b << '\n'
        << ImGuiEnabled << '\n'
        << camera.Position.x << '\n'
        << camera.Position.y << '\n'
        << camera.Position.z << '\n'
        << camera.Front.x << '\n'
        << camera.Front.y << '\n'
        << camera.Front.z;
}

void ProgramState::LoadFromFile(std::string filename) { //Ovde se ucitava pozicija iz prethodnog pokretanja aplikacije
    std::ifstream in(filename);
    if (in) {
        in >> clearColor.r
           >> clearColor.g
           >> clearColor.b
           >> ImGuiEnabled
           >> camera.Position.x
           >> camera.Position.y
           >> camera.Position.z
           >> camera.Front.x
           >> camera.Front.y
           >> camera.Front.z;
    }
}

ProgramState *programState;

void DrawImGui(ProgramState *programState);

int main() {
    // glfw: initialize and configure
    // ----------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // -------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // _________________________________________________________
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }


    programState = new ProgramState;
    programState->LoadFromFile("resources/program_state.txt");
    if (programState->ImGuiEnabled) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    // configure global opengl state
    // ----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // ___________________________________________________________________________________________________
    //========================================Milan===========================================================
    Shader ourShader("resources/shaders/directionLight.vs", "resources/shaders/directionLight.fs");
    //=========================================================================================================

    //===========================================Milan==============================================================
    Shader blendingShader("resources/shaders/blending.vs", "resources/shaders/blending.fs");
    //=========================================================================================================

    //==========================================Vladimir===============================================================
    Shader skyboxShader("resources/shaders/skybox.vs", "resources/shaders/skybox.fs");
    //=========================================================================================================

    //===========================================Milan==============================================================
    Shader shaderBlur("resources/shaders/blur.vs", "resources/shaders/blur.fs");
    Shader shaderBloomFinal("resources/shaders/bloom_final.vs", "resources/shaders/bloom_final.fs");
    //=========================================================================================================

    //=============================================Vladimir============================================================
    Shader nmShader("resources/shaders/normalMapping.vs", "resources/shaders/normalMapping.fs");
    //=========================================================================================================



    // loading models
    // __________________________________________________________________________________________

    //========================================Vladimir===========================================================
    //wall
    Model wallModel("resources/objects/wall/concrete_wall.obj",true);
    wallModel.SetShaderTextureNamePrefix("material.");
    //wall sculpture
    Model wallScModel("resources/objects/wallSculpture/marble-frieze-1-edit-2.obj",true);
    wallModel.SetShaderTextureNamePrefix("material.");
    //=========================================================================================================

    //==========================================Milan===============================================================
    //cat
    Model catModel("resources/objects/cat/12221_Cat_v1_l3.obj",true);
    catModel.SetShaderTextureNamePrefix("material.");

    //ramses
    Model ramsesModel("resources/objects/ramses/Colossal_Bust_Rameses_II.obj",true);
    ramsesModel.SetShaderTextureNamePrefix("material.");

    //skull
    Model skullModel("resources/objects/skull/rapid.obj",true);
    skullModel.SetShaderTextureNamePrefix("material.");

    //tree
    Model treeModel("resources/objects/tree/rapid.obj",true);
    treeModel.SetShaderTextureNamePrefix("material.");
    //=========================================================================================================

    //===========================================Vladimir==============================================================
    //mushroom
    Model mushroomModel("resources/objects/mushroom/mushroom-2.obj",true);
    mushroomModel.SetShaderTextureNamePrefix("material.");
    //=========================================================================================================

    //======================================Milan===================================================================
    //fireBall
    Model fireModel("resources/objects/fireBall/sun.obj",true);
    fireModel.SetShaderTextureNamePrefix("material.");
    //=========================================================================================================

    //======================================Milan===================================================================
    //Bloom____________________________________________________________________________________
    // configure framebuffers
    unsigned int hdrFBO;
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    // create 2 color buffers (1 for normal rendering, other for brightness threshold values)
    unsigned int colorBuffers[2];
    glGenTextures(2, colorBuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // attach texture to framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
    }
    // create and attach depth buffer (renderbuffer)
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffers[0], 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);

    // check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ping-pong-framebuffer for blurring
    unsigned int pingpongFBO[2];
    unsigned int pingpongColorbuffers[2];
    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongColorbuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
        // also check if framebuffers are complete (no need for depth buffer)
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;
    }
    //=========================================================================================================

    //==========================================Vladimir===============================================================
    //Seting skybox vertices
    //__________________________________________________________________________________________
    float skyboxVertices[] = {
            // positions
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f,  1.0f
    };
    //=========================================================================================================


    //====================================Milan=====================================================================
    //Grass vertices
    float transparentVertices[] = {
            // positions         // texture Coords (swapped y coordinates because texture is flipped upside down)
            0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
            0.0f, -0.5f,  0.0f,  0.0f,  1.0f,
            1.0f, -0.5f,  0.0f,  1.0f,  1.0f,

            0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
            1.0f, -0.5f,  0.0f,  1.0f,  1.0f,
            1.0f,  0.5f,  0.0f,  1.0f,  0.0f
    };

    //Grass positions
    vector<glm::vec3> grassPositions;
    for(int i =0;i<10;i++){
        grassPositions.push_back(glm::vec3(rand()%35 - 15.0f,-0.5f,rand()%30));
    }
    //=========================================================================================================


    //=======================================Vladimir==================================================================
    // skybox VAO
    //____________________________________________________________________________________________
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // load textures for skybox
    // ___________________________________________________________________________________________
    programState->faces =
            {
                    FileSystem::getPath("resources/textures/skybox/right.jpg"),
                    FileSystem::getPath("resources/textures/skybox/left.jpg"),
                    FileSystem::getPath("resources/textures/skybox/top.jpg"),
                    FileSystem::getPath("resources/textures/skybox/bottom.jpg"),
                    FileSystem::getPath("resources/textures/skybox/front.jpg"),
                    FileSystem::getPath("resources/textures/skybox/back.jpg"),
            };

    programState->cubemapTexture = loadCubemap(programState->faces);

    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    //=========================================================================================================

    //====================================Milan=====================================================================
    // transparent VAO
    unsigned int transparentVAO, transparentVBO;
    glGenVertexArrays(1, &transparentVAO);
    glGenBuffers(1, &transparentVBO);
    glBindVertexArray(transparentVAO);
    glBindBuffer(GL_ARRAY_BUFFER, transparentVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(transparentVertices), transparentVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);
    //Load textures
    unsigned int floorTexture = loadTexture(FileSystem::getPath("resources/textures/brickwall.jpg").c_str());
    unsigned int floorTextureNormal = loadTexture(FileSystem::getPath("resources/textures/brickwall_normal.jpg").c_str());
    unsigned int grassTexture = loadTexture(FileSystem::getPath("resources/textures/grass.png").c_str());
    //=========================================================================================================


    //=====================================Milan====================================================================
    //Point light
    //___________________________________________________________________________________________________
    PointLight& pointLight = programState->pointLight;
    pointLight.position = glm::vec3(-5.6f, 5.0f, 1.7f);
    pointLight.ambient = glm::vec3(0.1, 0.1, 0.1);
    pointLight.diffuse = glm::vec3(0.6, 0.6, 0.6);
    pointLight.specular = glm::vec3(1.0, 1.0, 1.0);

    pointLight.constant = 0.05f;
    pointLight.linear = 0.6f;
    pointLight.quadratic = 0.3f;

    // shader configuration
    // ____________________________________________________________________________________________
    ourShader.use();
    ourShader.setInt("diffuseTexture", 0);
    //=========================================================================================================
    //=====================================Vladimir====================================================================
    ourShader.setVec3("changeColor", glm::vec3(1.0f,1.0f,1.0f));
    //=========================================================================================================

    //=====================================Vladimir====================================================================
    nmShader.use();
    nmShader.setInt("diffuseMap", 0);
    nmShader.setInt("normalMap", 1);
    nmShader.setVec3("changeColor", glm::vec3(1.0f,1.0f,1.0f));
    //=========================================================================================================

    //=========================================Milan================================================================
    shaderBlur.use();
    shaderBlur.setInt("image", 0);
    shaderBloomFinal.use();
    shaderBloomFinal.setInt("scene", 0);
    shaderBloomFinal.setInt("bloomBlur", 1);
    //=========================================================================================================

//====================================Milan=====================================================================
    blendingShader.use();
    blendingShader.setInt("texture1", 0);
    // render loop
    // ________________________________________________________________________________________________
    while (!glfwWindowShouldClose(window)) {

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        float time = currentFrame;

        // input
        // _________________________________________________________________________________
        if(freeMove) {
            processInput(window);
        }else{
            processInput1(window);
            programState->camera.Position.y = 0.7f;
        }

        // render
        // ________________________________________________________________________________
        glClearColor(programState->clearColor.r, programState->clearColor.g, programState->clearColor.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

//=========================================================================================================

//=====================================Milan====================================================================
        //render scene into floating point framebuffer
        // ____________________________________________________________________________________
        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//=========================================================================================================

//===================================Milan======================================================================
        // enable shader before setting uniforms
        ourShader.use();
        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(programState->camera.Zoom),
                                                (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = programState->camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        ourShader.setVec3("viewPosition", programState->camera.Position);
        ourShader.setFloat("material.shininess", 32.0f);
        // directional light
        //________________________________________________________________________________________________

        ourShader.setVec3("dirLight.direction", programState->dirLightDir);
        ourShader.setVec3("dirLight.ambient", glm::vec3(programState->dirLightAmbDiffSpec.x));
        ourShader.setVec3("dirLight.diffuse", glm::vec3(programState->dirLightAmbDiffSpec.y));
        ourShader.setVec3("dirLight.specular", glm::vec3(programState->dirLightAmbDiffSpec.z));

//=========================================================================================================

//========================================Vladimir=================================================================
        //halutination
        if(!halutination)
            ourShader.setVec3("changeColor", programState->vecCalibrate);
        else {
            ourShader.setVec3("changeColor",
                              programState->vecCalibrate + glm::vec3(-0.5f + sin(time) * 5.0f, -0.6f, -0.9f));
            programState->camera.Position+=glm::vec3(cos(time*4.0)*0.006,cos(time*4.0)*0.008,sin(time*4.0)*0.006f);
        }
//=========================================================================================================

//=========================================Vladimir================================================================
        // point light
        //_______________________________________________________________________________________________

        ourShader.setVec3("pointLights[0].position", glm::vec3(15.45f+sin(time*2.0f)*0.5f, 0.5f+cos(time*5.0f)*0.2f, 28.35f+cos(time*3.0f)*0.5f));
        ourShader.setVec3("pointLights[0].ambient", pointLight.ambient);
        ourShader.setVec3("pointLights[0].diffuse", pointLight.diffuse);
        ourShader.setVec3("pointLights[0].specular", pointLight.specular);
        ourShader.setFloat("pointLights[0].constant", pointLight.constant);
        ourShader.setFloat("pointLights[0].linear", pointLight.linear);
        ourShader.setFloat("pointLights[0].quadratic", pointLight.quadratic);

        ourShader.setVec3("pointLights[1].position", glm::vec3(2.85f +sin(time*2.0f), 1.25f+sin(time*3.0f), 24.0f+cos(time*3.0f)));
        ourShader.setVec3("pointLights[1].ambient", pointLight.ambient);
        ourShader.setVec3("pointLights[1].diffuse", pointLight.diffuse);
        ourShader.setVec3("pointLights[1].specular", pointLight.specular);
        ourShader.setFloat("pointLights[1].constant", pointLight.constant);
        ourShader.setFloat("pointLights[1].linear", pointLight.linear);
        ourShader.setFloat("pointLights[1].quadratic", pointLight.quadratic);

        ourShader.setVec3("pointLights[2].position", glm::vec3(-7.2f+sin(time*3.0f)*2.0f, 2.7f+cos(time*3.0f), 13.85f+cos(time*2.0f)*3.0f));
        ourShader.setVec3("pointLights[2].ambient", pointLight.ambient);
        ourShader.setVec3("pointLights[2].diffuse", pointLight.diffuse);
        ourShader.setVec3("pointLights[2].specular", pointLight.specular);
        ourShader.setFloat("pointLights[2].constant", pointLight.constant);
        ourShader.setFloat("pointLights[2].linear", pointLight.linear);
        ourShader.setFloat("pointLights[2].quadratic", pointLight.quadratic);

        ourShader.setVec3("pointLights[3].position", glm::vec3(14.75f, 1.4f, 18.65f));
        ourShader.setVec3("pointLights[3].ambient", pointLight.ambient);
        ourShader.setVec3("pointLights[3].diffuse", pointLight.diffuse);
        ourShader.setVec3("pointLights[3].specular", pointLight.specular);
        ourShader.setFloat("pointLights[3].constant", pointLight.constant);
        ourShader.setFloat("pointLights[3].linear", pointLight.linear);
        ourShader.setFloat("pointLights[3].quadratic", pointLight.quadratic);

        ourShader.setVec3("pointLights[4].ambient", pointLight.ambient);
        ourShader.setVec3("pointLights[4].diffuse", pointLight.diffuse);
        ourShader.setVec3("pointLights[4].specular", pointLight.specular);
        ourShader.setFloat("pointLights[4].constant", pointLight.constant);
        ourShader.setFloat("pointLights[4].linear", pointLight.linear);
        ourShader.setFloat("pointLights[4].quadratic", pointLight.quadratic);


        ourShader.setVec3("pointLights[6].position", glm::vec3(14.0f, 0.55f, -1.65f));
        ourShader.setVec3("pointLights[6].ambient", pointLight.ambient);
        ourShader.setVec3("pointLights[6].diffuse", pointLight.diffuse);
        ourShader.setVec3("pointLights[6].specular", pointLight.specular);
        ourShader.setFloat("pointLights[6].constant", pointLight.constant);
        ourShader.setFloat("pointLights[6].linear", pointLight.linear);
        ourShader.setFloat("pointLights[6].quadratic", pointLight.quadratic);

//=========================================================================================================

//======================================Milan===================================================================
        // spotLight
        //________________________________________________________________________________________________
        if (spotlightOn) {
            ourShader.setVec3("pointLights[5].position", programState->camera.Position+ glm::vec3(0.0f,0.2f,0.0f) + glm::vec3(programState->camera.Front.x*0.9f+sin(time*2.0f)*0.1f,programState->camera.Front.y,programState->camera.Front.z*0.9f+cos(time*2.0f)*0.1f)*3.0f);
            ourShader.setVec3("pointLights[5].ambient", pointLight.ambient);
            ourShader.setVec3("pointLights[5].diffuse", pointLight.diffuse);
            ourShader.setVec3("pointLights[5].specular", pointLight.specular);
            ourShader.setFloat("pointLights[5].constant", pointLight.constant);
            ourShader.setFloat("pointLights[5].linear", pointLight.linear);
            ourShader.setFloat("pointLights[5].quadratic", pointLight.quadratic);

        }else{
            ourShader.setVec3("pointLights[5].position", programState->camera.Position );
            ourShader.setVec3("pointLights[5].ambient", glm::vec3(0.0f));
            ourShader.setVec3("pointLights[5].diffuse", glm::vec3(0.0f));
            ourShader.setVec3("pointLights[5].specular", glm::vec3(0.0f));
            ourShader.setFloat("pointLights[5].constant", pointLight.constant);
            ourShader.setFloat("pointLights[5].linear", pointLight.linear);
            ourShader.setFloat("pointLights[5].quadratic", pointLight.quadratic);

        }
//=========================================================================================================

//============================================Vladimir=============================================================
        //draw walls
        {//wals left
            glm::mat4 modelWall = glm::mat4(1.0f);
            modelWall = glm::translate(modelWall, glm::vec3(17.0f, -0.5f, 0.0f));
            modelWall = glm::scale(modelWall, glm::vec3(0.2f));
            modelWall = glm::rotate(modelWall, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            ourShader.setMat4("model", modelWall);
            wallModel.Draw(ourShader);
            for (int i = 0; i < 7; i++) {
                modelWall = glm::translate(modelWall, glm::vec3(-20.0f, 0.0f, 0.0f));
                ourShader.setMat4("model", modelWall);
                wallModel.Draw(ourShader);
            }
            //wals right
            modelWall = glm::mat4(1.0f);
            modelWall = glm::translate(modelWall, glm::vec3(-15.0f, -0.5f, 0.0f));
            modelWall = glm::scale(modelWall, glm::vec3(0.2f));
            modelWall = glm::rotate(modelWall, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            ourShader.setMat4("model", modelWall);
            wallModel.Draw(ourShader);
            for (int i = 0; i < 7; i++) {
                modelWall = glm::translate(modelWall, glm::vec3(-20.0f, 0.0f, 0.0f));
                ourShader.setMat4("model", modelWall);
                wallModel.Draw(ourShader);
            }

            //wals front
            modelWall = glm::mat4(1.0f);
            modelWall = glm::translate(modelWall, glm::vec3(16.0f, -0.5f, 29.0f));
            modelWall = glm::scale(modelWall, glm::vec3(0.2f));
            ourShader.setMat4("model", modelWall);
            wallModel.Draw(ourShader);
            for (int i = 0; i < 7; i++) {
                modelWall = glm::translate(modelWall, glm::vec3(-20.0f, 0.0f, 0.0f));
                ourShader.setMat4("model", modelWall);
                wallModel.Draw(ourShader);
            }

            //wals front1
            modelWall = glm::mat4(1.0f);
            modelWall = glm::translate(modelWall, glm::vec3(16.0f, -0.5f, 1.0f));
            modelWall = glm::scale(modelWall, glm::vec3(0.2f));
            ourShader.setMat4("model", modelWall);
            wallModel.Draw(ourShader);
            for (int i = 0; i < 7; i++) {
                modelWall = glm::translate(modelWall, glm::vec3(-20.0f, 0.0f, 0.0f));
                ourShader.setMat4("model", modelWall);
                if (i == 3)
                    continue;
                wallModel.Draw(ourShader);
            }
            //wals front2
            modelWall = glm::mat4(1.0f);
            modelWall = glm::translate(modelWall, glm::vec3(16.0f, -0.5f, 6.0f));
            modelWall = glm::scale(modelWall, glm::vec3(0.2f));
            ourShader.setMat4("model", modelWall);
            wallModel.Draw(ourShader);
            for (int i = 0; i < 7; i++) {
                modelWall = glm::translate(modelWall, glm::vec3(-20.0f, 0.0f, 0.0f));
                ourShader.setMat4("model", modelWall);
                if (i == 1)
                    continue;
                wallModel.Draw(ourShader);
            }
            //wals front3
            modelWall = glm::mat4(1.0f);
            modelWall = glm::translate(modelWall, glm::vec3(16.0f, -0.5f, 11.0f));
            modelWall = glm::scale(modelWall, glm::vec3(0.2f));
            ourShader.setMat4("model", modelWall);
            wallModel.Draw(ourShader);
            for (int i = 0; i < 7; i++) {
                modelWall = glm::translate(modelWall, glm::vec3(-20.0f, 0.0f, 0.0f));
                ourShader.setMat4("model", modelWall);
                if (i == 4)
                    continue;
                wallModel.Draw(ourShader);
            }
            //wals front4
            modelWall = glm::mat4(1.0f);
            modelWall = glm::translate(modelWall, glm::vec3(16.0f, -0.5f, 16.0f));
            modelWall = glm::scale(modelWall, glm::vec3(0.2f));
            ourShader.setMat4("model", modelWall);
            wallModel.Draw(ourShader);
            for (int i = 0; i < 7; i++) {
                modelWall = glm::translate(modelWall, glm::vec3(-20.0f, 0.0f, 0.0f));
                ourShader.setMat4("model", modelWall);
                if (i == 0)
                    continue;
                wallModel.Draw(ourShader);
            }
            //wals front5
            modelWall = glm::mat4(1.0f);
            modelWall = glm::translate(modelWall, glm::vec3(16.0f, -0.5f, 21.0f));
            modelWall = glm::scale(modelWall, glm::vec3(0.2f));
            ourShader.setMat4("model", modelWall);
            wallModel.Draw(ourShader);
            for (int i = 0; i < 7; i++) {
                modelWall = glm::translate(modelWall, glm::vec3(-20.0f, 0.0f, 0.0f));
                ourShader.setMat4("model", modelWall);
                if (i == 5)
                    continue;
                wallModel.Draw(ourShader);
            }
            //wals front6
            modelWall = glm::mat4(1.0f);
            modelWall = glm::translate(modelWall, glm::vec3(16.0f, -0.5f, 26.0f));
            modelWall = glm::scale(modelWall, glm::vec3(0.2f));
            ourShader.setMat4("model", modelWall);
            wallModel.Draw(ourShader);
            for (int i = 0; i < 7; i++) {
                modelWall = glm::translate(modelWall, glm::vec3(-20.0f, 0.0f, 0.0f));
                ourShader.setMat4("model", modelWall);
                if (i == 3)
                    continue;
                wallModel.Draw(ourShader);
            }
        }
        //draw Wall sculpture
        glm::mat4 modelWallSc = glm::mat4(1.0f);
        modelWallSc = glm::translate(modelWallSc,glm::vec3(2.55f, -0.5f, -2.0f));
        modelWallSc = glm::scale(modelWallSc, glm::vec3(1.001f));
        ourShader.setMat4("model", modelWallSc);
        wallScModel.Draw(ourShader);
//=========================================================================================================

//==========================================Milan===============================================================
        //draw cat
        glm::mat4 modelCat = glm::mat4(1.0f);
        modelCat = glm::translate(modelCat,glm::vec3(-8.55f, -0.5f, 4.9f));
        modelCat = glm::scale(modelCat, glm::vec3(0.035f));
        modelCat = glm::rotate(modelCat,glm::radians(-90.0f), glm::vec3(1.0f ,0.0f, 0.0f));
        modelCat = glm::rotate(modelCat,glm::radians(110.0f), glm::vec3(0.0f ,0.0f, 1.0f));
        ourShader.setMat4("model", modelCat);
        catModel.Draw(ourShader);
        //draw cat3
        modelCat = glm::mat4(1.0f);
        modelCat = glm::translate(modelCat,glm::vec3(5.0f, -0.5f, 7.6f));
        modelCat = glm::scale(modelCat, glm::vec3(0.04f));
        modelCat = glm::rotate(modelCat,glm::radians(-90.0f), glm::vec3(1.0f ,0.0f, 0.0f));
        modelCat = glm::rotate(modelCat,glm::radians(102.6f), glm::vec3(0.0f ,0.0f, 1.0f));
        ourShader.setMat4("model", modelCat);
        catModel.Draw(ourShader);
        //draw cat2
        modelCat = glm::mat4(1.0f);
        modelCat = glm::translate(modelCat,glm::vec3(-5.75f, -4.55f, 28.85f));
        modelCat = glm::scale(modelCat, glm::vec3(0.15f));
        modelCat = glm::rotate(modelCat,glm::radians(-90.0f), glm::vec3(1.0f ,0.0f, 0.0f));
        modelCat = glm::rotate(modelCat,glm::radians(102.6f), glm::vec3(0.0f ,0.0f, 1.0f));
        ourShader.setMat4("model", modelCat);
        catModel.Draw(ourShader);

        //draw ramses
        glm::mat4 modelRamses = glm::mat4(1.0f);
        modelRamses = glm::translate(modelRamses,glm::vec3(-7.2f, -0.7f, 13.85f));
        modelRamses = glm::scale(modelRamses, glm::vec3(3.55f));
        modelRamses = glm::rotate(modelRamses,glm::radians(-90.0f), glm::vec3(1.0f ,0.0f, 0.0f));
        modelRamses = glm::rotate(modelRamses,glm::radians(105.5f), glm::vec3(0.0f ,0.0f, 1.0f));
        ourShader.setMat4("model", modelRamses);
        ramsesModel.Draw(ourShader);

        //draw skull
        glm::mat4 modelSkull = glm::mat4(1.0f);
        modelSkull = glm::translate(modelSkull,glm::vec3(15.75f, -9.4f, 19.65f));
        modelSkull = glm::scale(modelSkull, glm::vec3(0.02));
        modelSkull = glm::rotate(modelSkull,glm::radians(-80.0f), glm::vec3(0.0f ,1.0f, 0.0f));
        ourShader.setMat4("model", modelSkull);
        skullModel.Draw(ourShader);

        //draw tree
        glm::mat4 modelTree = glm::mat4(1.0f);
        modelTree = glm::translate(modelTree,glm::vec3(2.85f, 0.25f, 24.0f));
        modelTree = glm::scale(modelTree, glm::vec3(0.01f));
        modelTree = glm::rotate(modelTree,glm::radians(-2.45f), glm::vec3(1.0f ,0.0f, 0.0f));
        ourShader.setMat4("model", modelTree);
        treeModel.Draw(ourShader);
//=========================================================================================================

//=====================================Vladimir====================================================================
        //draw mushroom
        glm::mat4 modelMushroom = glm::mat4(1.0f);
        modelMushroom = glm::translate(modelMushroom,glm::vec3(15.45f, -0.5f, 28.35f));
        modelMushroom = glm::scale(modelMushroom, glm::vec3(0.15f));
        ourShader.setMat4("model", modelMushroom);
        mushroomModel.Draw(ourShader);
//=========================================================================================================

//===================================Milan======================================================================
        //draw fireBall
        if (spotlightOn) {
            glm::mat4 modelFire = glm::mat4(1.0f);
            modelFire = glm::translate(modelFire, programState->camera.Position +
                                                  glm::vec3(programState->camera.Front.x + sin(time * 2.0f) * 0.1f,
                                                            programState->camera.Front.y,
                                                            programState->camera.Front.z + cos(time * 2.0f) * 0.1f) *3.0f);
            modelFire = glm::scale(modelFire, glm::vec3(0.1f));
            ourShader.setMat4("model", modelFire);
            fireModel.Draw(ourShader);
        }
//=========================================================================================================

//============================================Vladimir=============================================================
        // floor
        {
            // enable shader before setting uniforms
            nmShader.use();
            // view/projection transformations
            glm::mat4 projection = glm::perspective(glm::radians(programState->camera.Zoom),
                                                    (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
            glm::mat4 view = programState->camera.GetViewMatrix();
            nmShader.setMat4("projection", projection);
            nmShader.setMat4("view", view);

            nmShader.setVec3("viewPos", programState->camera.Position);
            nmShader.setFloat("material.shininess", 32.0f);

            //halutination
            if(!halutination)
                nmShader.setVec3("changeColor", programState->vecCalibrate);
            else
                nmShader.setVec3("changeColor", programState->vecCalibrate +glm::vec3(-0.5f+sin(time)*5.0f,-0.6f,-0.9f));
            // point light
            //_____________________________________________________________________________________________

            if (spotlightOn) {
                nmShader.setVec3("lightPos", programState->camera.Position - glm::vec3(0.0f, 2.5f, 0.0f) +
                                             glm::vec3(programState->camera.Front.x + sin(time * 2.0f) * 0.1f,
                                                       -programState->camera.Front.y,
                                                       programState->camera.Front.z + cos(time * 2.0f) * 0.1f) * 3.0f);
            }else{
                nmShader.setVec3("lightPos", programState->camera.Position + glm::vec3(0.0f,-2.0f,0.0f));
            }

        }
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floorTexture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, floorTextureNormal);
//=========================================================================================================

//========================================Milan=================================================================
        glm::mat4 modelFloor = glm::mat4(1.0f);

        modelFloor = glm::translate(modelFloor, glm::vec3(0.0f, -0.5f, 0.0));
        modelFloor = glm::scale(modelFloor, glm::vec3(2.0f, 0.5f, 2.0f));
        modelFloor = glm::translate(modelFloor, glm::vec3(-8.0f, 0.0f, 0.0f));

        for(int i = 0;i<8;i++){
            for(int j = 0;j<8;j++){
                modelFloor = glm::translate(modelFloor, glm::vec3(2.0f, 0.0f, 0.0f));
                modelFloor = glm::rotate(modelFloor,glm::radians(-90.0f), glm::vec3(1.0f ,0.0f, 0.0f));//ggg
                nmShader.setMat4("model", modelFloor);
                renderQuad1();
                modelFloor = glm::rotate(modelFloor,glm::radians(90.0f), glm::vec3(1.0f ,0.0f, 0.0f));//ggg

            }
            modelFloor = glm::translate(modelFloor, glm::vec3(0.0f, 0.0f, 2.0f));
            modelFloor = glm::translate(modelFloor, glm::vec3(-16.0f, 0.0f, 0.0f));
        }

        // vegetation
        blendingShader.use();
        glm::mat4 modelGrass = glm::mat4(1.0f);
        blendingShader.setMat4("projection", projection);
        blendingShader.setMat4("view", view);
        glBindVertexArray(transparentVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, grassTexture);
        for (unsigned int i = 0; i < 10; i++)
        {
            modelGrass = glm::mat4(1.0f);
            modelGrass = glm::scale(modelGrass, glm::vec3(0.5f));
            modelGrass = glm::translate(modelGrass, grassPositions[i]);
            blendingShader.setMat4("model", modelGrass);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
//=========================================================================================================

//======================================Vladimir===================================================================
        //________________________________________________________________________________________
        // draw skybox
        //__________________________________________________________________________________________
        glDepthFunc(GL_LEQUAL);
        skyboxShader.use();
        view = glm::mat4(glm::mat3(programState->camera.GetViewMatrix()));
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);
        // skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, programState->cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default

        //Changing bg
        if(!circleColision(glm::vec3(15.45f, -0.5f, 28.35f),programState->camera.Position,0.1f)){
            bg_change = true;
            halutination = true;
        }
        if(bg_change){
            bg_change = !bg_change;
            programState->dirLightAmbDiffSpec= glm::vec3(0.01f,0.16f,0.03f);
            programState->faces =
                    {
                            FileSystem::getPath("resources/textures/skybox1/right.jpg"),
                            FileSystem::getPath("resources/textures/skybox1/left.jpg"),
                            FileSystem::getPath("resources/textures/skybox1/top.jpg"),
                            FileSystem::getPath("resources/textures/skybox1/bottom.jpg"),
                            FileSystem::getPath("resources/textures/skybox1/front.jpg"),
                            FileSystem::getPath("resources/textures/skybox1/back.jpg"),
                    };
            programState->cubemapTexture = loadCubemap(programState->faces);
        }
//=========================================================================================================

//======================================Milan===================================================================
        if (programState->ImGuiEnabled)
            DrawImGui(programState);
//=========================================================================================================

//==================================Milan=======================================================================
        // blur bright fragments with two-pass Gaussian Blur
        // _____________________________________________________________________________________
        bool horizontal = true, first_iteration = true;
        unsigned int amount = 5;
        shaderBlur.use();
        for (unsigned int i = 0; i < amount; i++)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
            shaderBlur.setInt("horizontal", horizontal);
            glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongColorbuffers[!horizontal]);  // bind texture of other framebuffer (or scene if first iteration)
            renderQuad();
            horizontal = !horizontal;
            if (first_iteration)
                first_iteration = false;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // now render floating point color buffer to 2D quad and tonemap HDR colors to default framebuffer's (clamped) color range
        //___________________________________________________________________________________________________
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shaderBloomFinal.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);
        shaderBloomFinal.setInt("bloom", bloom);
        shaderBloomFinal.setFloat("exposure", 1.0f);
        renderQuad();
//=========================================================================================================

//====================================Milan=====================================================================
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // ______________________________________________________________________________________
        glfwSwapBuffers(window);
        glfwPollEvents();
//=========================================================================================================
    }
//======================================Vladimir===================================================================
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVAO);
//=========================================================================================================

//=============================Milan============================================================================
    programState->SaveToFile("resources/program_state.txt");
    delete programState;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ______________________________________________________________________________________
    glfwTerminate();
    return 0;
//=========================================================================================================
}
//================================Milan=========================================================================
// renderQuad() renders a 1x1 XY quad in NDC
// ________________________________________________________________________________________
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
                // positions        // texture Coords
                -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
                1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}
//=========================================================================================================

//========================================Vladimir=================================================================
// __________________________________________________________________________________________
unsigned int quadVAO1 = 0;
unsigned int quadVBO1;
void renderQuad1()
{
    if (quadVAO1 == 0)
    {
        // positions
        glm::vec3 pos1(-1.0f,  1.0f, 0.0f);
        glm::vec3 pos2(-1.0f, -1.0f, 0.0f);
        glm::vec3 pos3( 1.0f, -1.0f, 0.0f);
        glm::vec3 pos4( 1.0f,  1.0f, 0.0f);
        // texture coordinates
        glm::vec2 uv1(0.0f, 1.0f);
        glm::vec2 uv2(0.0f, 0.0f);
        glm::vec2 uv3(1.0f, 0.0f);
        glm::vec2 uv4(1.0f, 1.0f);
        // normal vector
        glm::vec3 nm(0.0f, 0.0f, 1.0f);

        // calculate tangent/bitangent vectors of both triangles
        glm::vec3 tangent1, bitangent1;
        glm::vec3 tangent2, bitangent2;
        // triangle 1
        // ----------
        glm::vec3 edge1 = pos2 - pos1;
        glm::vec3 edge2 = pos3 - pos1;
        glm::vec2 deltaUV1 = uv2 - uv1;
        glm::vec2 deltaUV2 = uv3 - uv1;

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

        bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);

        // triangle 2
        // ----------
        edge1 = pos3 - pos1;
        edge2 = pos4 - pos1;
        deltaUV1 = uv3 - uv1;
        deltaUV2 = uv4 - uv1;

        f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);


        bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);


        float quadVertices[] = {
                // positions            // normal         // texcoords  // tangent                          // bitangent
                pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
                pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
                pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

                pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
                pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
                pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
        };
        // configure plane VAO
        glGenVertexArrays(1, &quadVAO1);
        glGenBuffers(1, &quadVBO1);
        glBindVertexArray(quadVAO1);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO1);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
    }
    glBindVertexArray(quadVAO1);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}
//=========================================================================================================

//======================================Milan===================================================================
bool lineColision(glm::vec3 p1,glm::vec3 p2,glm::vec3 px){
    float res;
    res = ((p2.z-p1.z)/(p2.x-p1.x))*(px.x - p1.x) + p1.z - px.z;
    if(res <= 0)
        return true;
    else return false;
}
//=========================================================================================================

//====================================Vladimir=====================================================================
bool circleColision(glm::vec3 c,glm::vec3 x,float r){
    float res;
    res = pow(x.x-c.x,2.0f)+pow(x.z-c.z,2.0f) - pow(r,2.0f);
    if(res >= 0)
        return true;
    else return false;
}
//=========================================================================================================

//=======================================Milan==================================================================
// process all input
// ____________________________________________________________________________________________
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(RIGHT, deltaTime);
}
void processInput1(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if(glfwGetKey(window, GLFW_KEY_N)==GLFW_PRESS){
        bg_change = !bg_change;
        if(bg_change){
            programState->dirLightAmbDiffSpec= glm::vec3(0.15f,0.35f,0.2f);

            programState->faces =
                    {
                            FileSystem::getPath("resources/textures/skybox1/right.jpg"),
                            FileSystem::getPath("resources/textures/skybox1/left.jpg"),
                            FileSystem::getPath("resources/textures/skybox1/top.jpg"),
                            FileSystem::getPath("resources/textures/skybox1/bottom.jpg"),
                            FileSystem::getPath("resources/textures/skybox1/front.jpg"),
                            FileSystem::getPath("resources/textures/skybox1/back.jpg"),
                    };
            programState->cubemapTexture = loadCubemap(programState->faces);
        }else {
            programState->dirLightAmbDiffSpec= glm::vec3(0.25f,0.45f,0.6f);
            programState->faces = {
                    FileSystem::getPath("resources/textures/skybox/right.jpg"),
                    FileSystem::getPath("resources/textures/skybox/left.jpg"),
                    FileSystem::getPath("resources/textures/skybox/top.jpg"),
                    FileSystem::getPath("resources/textures/skybox/bottom.jpg"),
                    FileSystem::getPath("resources/textures/skybox/front.jpg"),
                    FileSystem::getPath("resources/textures/skybox/back.jpg"),
            };
            programState->cubemapTexture = loadCubemap(programState->faces);
        }
    }

    //Colisions -borders
    if ((lineColision(glm::vec3(18.0f,0.0f,-1.0f), glm::vec3(-14.0f,0.0f,-1.0f),programState->camera.Position)) &&
        !(lineColision(glm::vec3(18.0f,0.0f,29.0f), glm::vec3(-14.0f,0.0f,29.0f),programState->camera.Position)) &&
        (lineColision(glm::vec3(17.0f,0.0f,-2.0f), glm::vec3(17.0f,0.0f,30.0f),programState->camera.Position)) &&
        (lineColision(glm::vec3(-13.0f,0.0f,30.0f), glm::vec3(-13.0f,0.0f,-2.0f),programState->camera.Position))
            ) {
        lastCamPosition = programState->camera.Position;
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            programState->camera.ProcessKeyboard(FORWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            programState->camera.ProcessKeyboard(BACKWARD, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            programState->camera.ProcessKeyboard(LEFT, deltaTime);
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            programState->camera.ProcessKeyboard(RIGHT, deltaTime);
    }else {
        programState->camera.Position = lastCamPosition;
    }
//=====================================Vladimir====================================================================
    if(glfwGetKey(window, GLFW_KEY_C)==GLFW_PRESS) {
        halutination = false;
    }
//=========================================================================================================
//============================================Milan=============================================================
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ___________________________________________________________________________________________
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// ________________________________________________________________________________________
void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    if (programState->CameraMouseMovementUpdateEnabled)
        programState->camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    programState->camera.ProcessMouseScroll(yoffset);
}

void DrawImGui(ProgramState *programState) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    {
        static float f = 0.0f;
        ImGui::Begin("Options");

        ImGui::Text("Direction light calibration");
        ImGui::DragFloat3("DL direction", (float*)&programState->dirLightDir, 0.05, -20.0, 20.0);

        ImGui::Text("amb       diff        spec");
        ImGui::DragFloat3("DL settings", (float*)&programState->dirLightAmbDiffSpec, 0.05, 0.001, 1.0);

        ImGui::Text("Point lights calibration:");
        ImGui::DragFloat("PL constant", &programState->pointLight.constant, 0.005, 0.0001, 1.0);
        ImGui::DragFloat("PL linear", &programState->pointLight.linear, 0.005, 0.0001, 1.0);
        ImGui::DragFloat("PL quadratic", &programState->pointLight.quadratic, 0.005, 0.0001, 1.0);

        ImGui::Text("Color calibration:");
        ImGui::Text("Red       Green       Blue");
        ImGui::DragFloat3("", (float*)&programState->vecCalibrate, 0.05, -300.0, 300.0);

        ImGui::End();
    }

    {
        ImGui::Begin("Camera info");
        const Camera& c = programState->camera;
        ImGui::Text("Camera position: (%f, %f, %f)", c.Position.x, c.Position.y, c.Position.z);
        ImGui::Text("(Yaw, Pitch): (%f, %f)", c.Yaw, c.Pitch);
        ImGui::Text("Camera front: (%f, %f, %f)", c.Front.x, c.Front.y, c.Front.z);
        ImGui::Checkbox("Camera mouse update", &programState->CameraMouseMovementUpdateEnabled);
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
//=========================================================================================================

//=============================================Milan============================================================
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        programState->ImGuiEnabled = !programState->ImGuiEnabled;
        if (programState->ImGuiEnabled) {
            programState->CameraMouseMovementUpdateEnabled = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            programState->CameraMouseMovementUpdateEnabled = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }

    if (key == GLFW_KEY_F && action == GLFW_PRESS){
        if(spotlightOn){
            spotlightOn = false;
        }else{
            spotlightOn = true;
        }
    }
    if (key == GLFW_KEY_F2 && action == GLFW_PRESS){
        if(freeMove){
            freeMove = false;
        }else{
            freeMove = true;
        }
    }
//=========================================================================================================
    //=======================================Milan==================================================================
    if (glfwGetKey(window, GLFW_KEY_F3) == GLFW_PRESS && !bloomKeyPressed)
    {
        bloom = !bloom;
        bloomKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_F3) == GLFW_RELEASE)
    {
        bloomKeyPressed = false;
    }
//=========================================================================================================
}
//=========================================Milan================================================================
unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrComponents;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrComponents, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

unsigned int loadTexture(char const *path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

//=========================================================================================================