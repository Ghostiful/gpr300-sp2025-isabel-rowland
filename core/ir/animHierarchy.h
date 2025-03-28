#include <glm/glm.hpp>
#include "../ew/transform.h"
#include <vector>
#include <imgui.h>

namespace ir {

	struct JointPose {
		glm::quat rotation;
		glm::vec3 translation;
		glm::vec3 scale;
	};

	struct Joint {
		const char* name;
		Joint* parent;
		std::vector<Joint*> children;
		unsigned int numChildren;
		ew::Transform localPose;
		ew::Transform globalPose;
		glm::mat4 localMat4;
		glm::mat4 globalMat4;
		bool isClicked = false;
		
		Joint() {

		}

		Joint(const char* theName) {
			name = theName;
			parent = nullptr;
		}

		Joint(const char* theName, Joint* theParent) {
			name = theName;
			parent = theParent;
			parent->children.push_back(this);
		}
		
		void handleUI() {
			ImGuiTreeNodeFlags flag = ImGuiTreeNodeFlags_DefaultOpen;
			if (name != nullptr) {
				if (ImGui::TreeNodeEx(name, flag)) {
					if (ImGui::IsItemClicked()) {
						isClicked = true;
					}
					for each(Joint * j in children) {
						j->handleUI();
					}
					ImGui::TreePop();
				}
			}
			else {
				if (ImGui::TreeNodeEx("Nameless Joint", flag)) {
					ImGui::TreePop();
				}
			}

		}

		void inspectorUI() {
			ImGui::Text(name);
			ImGui::DragFloat3("Position", &localPose.position.x, 0.1f);
			ImGui::DragFloat3("Rotation", &localPose.rotEuler.x, 0.1f);
			ImGui::DragFloat3("Scale", &localPose.scale.x, 0.1f);
		}
	};

	struct Skeleton {
		unsigned int jointCount;
		std::vector<Joint*> joints;

		void handleUI() {
			ImGuiTreeNodeFlags flag = ImGuiTreeNodeFlags_DefaultOpen;
			if (ImGui::TreeNodeEx("root", flag)) {
				for each(Joint * j in joints) {
					if (j->parent == nullptr) {
						j->handleUI();
					}
					
				}
				ImGui::TreePop();
			}
		}
	};

	struct SkeletonPose {
		Skeleton* skeleton;
		JointPose* localPoses;
		glm::mat4x4* globalPoses;
	};

	void solveFK(Joint* joint) {
		joint->localMat4 = joint->localPose.modelMatrixEuler();
		if (joint->parent == nullptr) {
			joint->globalMat4 = joint->localMat4;
		}
		else {
			joint->globalMat4 = joint->parent->globalMat4 * joint->localMat4;
		}
		for each(Joint * j in joint->children) {
			solveFK(j);
		}
	}
	
	void solveFK(Skeleton skeleton) {
		for each(Joint* j in skeleton.joints) {
			solveFK(j);
		}
	}

	
}