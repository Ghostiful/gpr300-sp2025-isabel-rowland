#include <stdio.h>
#include <math.h>

#include <ew/external/glad.h>
#include <ew/shader.h>
#include <ew/model.h>
#include <ew/camera.h>
#include <ew/transform.h>
#include <ew/cameraController.h>
#include <ew/texture.h>

#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include <ir/animation.h>
#include <ir/animHierarchy.h>

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
GLFWwindow* initWindow(const char* title, int width, int height);

void drawUI();

//Global state
int screenWidth = 1080;
int screenHeight = 720;
float prevFrameTime;
float deltaTime;

ew::Camera camera;
ew::CameraController cameraController;

struct Material {
	float Ka = 1.0;
	float Kd = 0.5;
	float Ks = 0.5;
	float Shininess = 128;
}material;

void resetCamera(ew::Camera* camera, ew::CameraController* controller) {
	camera->position = glm::vec3(0, 0, 5.0f);
	camera->target = glm::vec3(0);
	controller->yaw = controller->pitch = 0;
}

int main() {
	GLFWwindow* window = initWindow("Assignment 0", screenWidth, screenHeight);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glEnable(GL_DEPTH_TEST);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	ew::Shader shader = ew::Shader("assets/lit.vert", "assets/lit.frag");
	ew::Model monkeyModel = ew::Model("assets/Suzanne.obj");
	ew::Transform monkeyTransform;
	
	//Forward kinematics
	ir::Skeleton skeleton;
	skeleton.joints.push_back(new ir::Joint("Hips")); // 0
	skeleton.joints.push_back(new ir::Joint("Torso", skeleton.joints[0])); // 1
	skeleton.joints.push_back(new ir::Joint("Shoulder_R", skeleton.joints[1])); // 2
	skeleton.joints.push_back(new ir::Joint("Shoulder_L", skeleton.joints[1])); // 3
	skeleton.joints.push_back(new ir::Joint("Knee_R", skeleton.joints[0])); // 4
	skeleton.joints.push_back(new ir::Joint("Knee_L", skeleton.joints[0])); // 5
	skeleton.joints.push_back(new ir::Joint("Elbow_R", skeleton.joints[2])); // 6
	skeleton.joints.push_back(new ir::Joint("Elbow_L", skeleton.joints[3])); // 7
	skeleton.joints.push_back(new ir::Joint("Wrist_R", skeleton.joints[6])); // 8
	skeleton.joints.push_back(new ir::Joint("Wrist_L", skeleton.joints[7])); // 9
	skeleton.joints.push_back(new ir::Joint("Ankle_R", skeleton.joints[4])); // 10
	skeleton.joints.push_back(new ir::Joint("Ankle_L", skeleton.joints[5])); // 11
	
	skeleton.joints[0]->localPose.position = glm::vec3(0, 2, 0);
	skeleton.joints[1]->localPose.position = glm::vec3(0, 2, 0);
	skeleton.joints[2]->localPose.position = glm::vec3(1.5, 0, 0);
	skeleton.joints[3]->localPose.position = glm::vec3(-1.5, 0, 0);
	skeleton.joints[4]->localPose.position = glm::vec3(1, -1, 0);
	skeleton.joints[5]->localPose.position = glm::vec3(-1, -1, 0);
	skeleton.joints[6]->localPose.position = glm::vec3(1.3, 0, 0);
	skeleton.joints[7]->localPose.position = glm::vec3(-1.3, 0, 0);
	skeleton.joints[8]->localPose.position = glm::vec3(1.3, 0, 0);
	skeleton.joints[9]->localPose.position = glm::vec3(-1.3, 0, 0);
	skeleton.joints[10]->localPose.position = glm::vec3(0.1, -1.5, 0);
	skeleton.joints[11]->localPose.position = glm::vec3(-0.1, -1.5, 0);

	skeleton.joints[0]->localPose.scale = glm::vec3(1, 1, 1);
	skeleton.joints[1]->localPose.scale = glm::vec3(1.2, 1.2, 1.2);
	skeleton.joints[2]->localPose.scale = glm::vec3(0.7, 0.7, 0.7);
	skeleton.joints[3]->localPose.scale = glm::vec3(0.7, 0.7, 0.7);
	skeleton.joints[4]->localPose.scale = glm::vec3(0.7, 0.7, 0.7);
	skeleton.joints[5]->localPose.scale = glm::vec3(0.7, 0.7, 0.7);
	skeleton.joints[6]->localPose.scale = glm::vec3(0.7, 0.7, 0.7);
	skeleton.joints[7]->localPose.scale = glm::vec3(0.7, 0.7, 0.7);
	skeleton.joints[8]->localPose.scale = glm::vec3(0.7, 0.7, 0.7);
	skeleton.joints[9]->localPose.scale = glm::vec3(0.7, 0.7, 0.7);
	skeleton.joints[10]->localPose.scale = glm::vec3(0.7, 0.7, 0.7);
	skeleton.joints[11]->localPose.scale = glm::vec3(0.7, 0.7, 0.7);



	ir::solveFK(skeleton);

	ir::Joint* selectedJoint = nullptr;


	camera.position = glm::vec3(0.0f, 0.0f, 5.0f);
	camera.target = glm::vec3(0.0f, 0.0f, 0.0f);
	camera.aspectRatio = (float)screenWidth / screenHeight;
	camera.fov = 60.0f; //Vertical field of view, in degrees

	GLuint brickTexture = ew::loadTexture("assets/brick_color.jpg");
	glBindTextureUnit(0, brickTexture);
	//Make "_MainTex" sampler2D sample from the 2D texture bound to unit 0
	shader.use();
	shader.setInt("_MainTex", 0);

	shader.setVec3("_EyePos", camera.position);

	// Animation
	ir::Animator animator;
	animator.clip = new ir::AnimationClip();
	animator.clip->duration = 7;
	// -- Default keys
	animator.clip->positionKeys.push_back(ir::Vec3Key(glm::vec3(0, 0, 0), 0));
	animator.clip->positionKeys.push_back(ir::Vec3Key(glm::vec3(2, 2, 2), 2));
	animator.clip->positionKeys.push_back(ir::Vec3Key(glm::vec3(1, 1, 1), 5));
	animator.clip->positionKeys.push_back(ir::Vec3Key(glm::vec3(3, 3, 3), 7));
	animator.clip->rotationKeys.push_back(ir::Vec3Key(glm::vec3(0, 0, 0), 0));
	animator.clip->rotationKeys.push_back(ir::Vec3Key(glm::vec3(3, 3, 3), 7));
	animator.clip->scaleKeys.push_back(ir::Vec3Key(glm::vec3(1, 1, 1), 0));
	animator.clip->scaleKeys.push_back(ir::Vec3Key(glm::vec3(5, 5, 5), 2));
	animator.clip->scaleKeys.push_back(ir::Vec3Key(glm::vec3(2, 1, 2), 5));
	animator.clip->scaleKeys.push_back(ir::Vec3Key(glm::vec3(3, 3, 3), 7));
	animator.isPlaying = true;

	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		float time = (float)glfwGetTime();
		deltaTime = time - prevFrameTime;
		prevFrameTime = time;

		//RENDER
		glClearColor(0.6f, 0.8f, 0.92f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Animation
		animator.update(deltaTime);
		skeleton.joints[0]->localPose.position = animator.GetNextValue(animator.clip->positionKeys);
		skeleton.joints[0]->localPose.rotation = animator.GetNextValue(animator.clip->rotationKeys);
		skeleton.joints[0]->localPose.scale = animator.GetNextValue(animator.clip->scaleKeys);

		ir::solveFK(skeleton);

		shader.use();
		shader.setMat4("_ViewProjection", camera.projectionMatrix() * camera.viewMatrix());
		shader.setFloat("_Material.Ka", material.Ka);
		shader.setFloat("_Material.Kd", material.Kd);
		shader.setFloat("_Material.Ks", material.Ks);
		shader.setFloat("_Material.Shininess", material.Shininess);
		for each(ir::Joint * j in skeleton.joints) {

			shader.setMat4("_Model", j->globalMat4);
			
			monkeyModel.draw(); //Draws monkey model using current shader
		}

		
		//shader.setMat4("_Model", monkeyTransform.modelMatrix());
		//shader.setMat4("_ViewProjection", camera.projectionMatrix() * camera.viewMatrix());
		//shader.setFloat("_Material.Ka", material.Ka);
		//shader.setFloat("_Material.Kd", material.Kd);
		//shader.setFloat("_Material.Ks", material.Ks);
		//shader.setFloat("_Material.Shininess", material.Shininess);
		//monkeyModel.draw(); //Draws monkey model using current shader

		cameraController.move(window, &camera, deltaTime);


		//UI
		ImGui_ImplGlfw_NewFrame();
		ImGui_ImplOpenGL3_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("Settings");
		if (ImGui::Button("Reset Camera")) {
			resetCamera(&camera, &cameraController);
		}
		//Add more camera settings here!
		if (ImGui::DragFloat("FOV", &camera.fov, 0.1f, 0.0f, 120.0f)) {

		}
		if (ImGui::CollapsingHeader("Material")) {
			ImGui::SliderFloat("AmbientK", &material.Ka, 0.0f, 1.0f);
			ImGui::SliderFloat("DiffuseK", &material.Kd, 0.0f, 1.0f);
			ImGui::SliderFloat("SpecularK", &material.Ks, 0.0f, 1.0f);
			ImGui::SliderFloat("Shininess", &material.Shininess, 2.0f, 1024.0f);
		}

		animator.handleUI();
		if (ImGui::CollapsingHeader("Kinematics")) {
			skeleton.handleUI();
		}
		for each(ir::Joint * j in skeleton.joints) {
			if (j->isClicked) {
				selectedJoint = j;
				j->isClicked = false;
			}
		}
		

		ImGui::End();

		ImGui::SetNextWindowPos({ 600, 50 });
		ImGui::SetNextWindowSize({ 200, 250 });

		ImGui::Begin("Kinematics");

		if (selectedJoint != nullptr) {
			selectedJoint->inspectorUI();
		}

		ImGui::End();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());



		glfwSwapBuffers(window);
	}
	printf("Shutting down...");
}



void drawUI() {
	ImGui_ImplGlfw_NewFrame();
	ImGui_ImplOpenGL3_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Settings");
	if (ImGui::Button("Reset Camera")) {
		resetCamera(&camera, &cameraController);
	}
	//Add more camera settings here!
	if (ImGui::DragFloat("FOV", &camera.fov, 0.1f, 0.0f, 120.0f)) {

	}
	if (ImGui::CollapsingHeader("Material")) {
		ImGui::SliderFloat("AmbientK", &material.Ka, 0.0f, 1.0f);
		ImGui::SliderFloat("DiffuseK", &material.Kd, 0.0f, 1.0f);
		ImGui::SliderFloat("SpecularK", &material.Ks, 0.0f, 1.0f);
		ImGui::SliderFloat("Shininess", &material.Shininess, 2.0f, 1024.0f);
	}
	// Animation
	//if (ImGui::CollapsingHeader("Position Keyframes")) {
	//	for (int i = 0; i <)
	//}


	ImGui::End();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
	screenWidth = width;
	screenHeight = height;
	camera.aspectRatio = (float)screenWidth / screenHeight;

}

/// <summary>
/// Initializes GLFW, GLAD, and IMGUI
/// </summary>
/// <param name="title">Window title</param>
/// <param name="width">Window width</param>
/// <param name="height">Window height</param>
/// <returns>Returns window handle on success or null on fail</returns>
GLFWwindow* initWindow(const char* title, int width, int height) {
	printf("Initializing...");
	if (!glfwInit()) {
		printf("GLFW failed to init!");
		return nullptr;
	}

	GLFWwindow* window = glfwCreateWindow(width, height, title, NULL, NULL);
	if (window == NULL) {
		printf("GLFW failed to create window");
		return nullptr;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGL(glfwGetProcAddress)) {
		printf("GLAD Failed to load GL headers");
		return nullptr;
	}

	//Initialize ImGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	return window;
}

