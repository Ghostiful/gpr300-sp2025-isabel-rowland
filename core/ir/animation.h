#include <glm/glm.hpp>
#include <vector>
#include <math.h>
#include <imgui.h>

// All easing functions are from https://easings.net/

namespace ir {
#pragma region Easing
	// Easing functions
	const enum EasingType {
		NONE,
		IN_OUT_ELASTIC,
		IN_SINE,
		OUT_BACK
	}; 

	const char* EasingNames[] = {
		"None",
		"In Out Elastic",
		"In Sine",
		"Out Back"
	};

	const float PI = 3.141592653;

	float noEasing(float x) {
		return x;
	}

	float easeInOutElastic(float x) {
		const float c5 = (2 * PI) / 4.5;

		return
			(x == 0) ? 0 :
			(x == 1) ? 1 :
			(x < 0.5) ? -(pow(2, 20 * x - 10) * sin((20 * x - 11.125) * c5)) / 2 : 
			(pow(2, -20 * x + 10) * sin((20 * x - 11.125) * c5)) / 2 + 1;
	}

	float easeInSine(float x) {
		return 1 - cos((x * PI) / 2);
	}

	float easeOutBack(float x) {
		const float c1 = 1.70158;
		const float c3 = c1 + 1;

		return 1 + (c3 * pow(x - 1, 3)) + (c1 * pow(x - 1, 2));
	}

	typedef float EasingFunc(float);

	const std::vector<EasingFunc*> EASING_FUNCTIONS = { noEasing, easeInOutElastic, easeInSine, easeOutBack};

	float ease(float val, int type) {
		return EASING_FUNCTIONS[type](val);
	}
#pragma endregion

	// Lerping
	template<class T>
	T lerp(T a, T b, float v) {
		return a + (b - a) * v;
	}

	template<class T>
	T inverseLerp(T a, T b, float v) {
		return (v - a) / (b - a);
	}

	struct Vec3Key {
		glm::vec3 value;
		float time;
		int easeType;

		Vec3Key() {
			time = -1; // ignore frames with -1 time
			easeType = NONE;
		}

		Vec3Key(glm::vec3 _value, float _time) {
			value = _value;
			time = _time;
			easeType = NONE;
		}

		bool operator==(const Vec3Key& other) {
			return (time == other.time) && (value == other.value);
		}
	};

	struct AnimationClip {
		float duration;
		std::vector<Vec3Key> positionKeys;
		std::vector<Vec3Key> rotationKeys;
		std::vector<Vec3Key> scaleKeys;

		AnimationClip() {
			duration = -1;
		}

	};

	struct Animator {
		AnimationClip* clip;
		bool isPlaying;
		float playbackSpeed; // negatives play backwards
		bool isLooping; // if false, stop once clip.duration is reached
		float playbackTime; // current time, between 0 and clip.duration

		Animator() {
			clip = nullptr;
			isPlaying = false;
			playbackSpeed = 1;
			isLooping = false;
			playbackTime = 0;
		}

		~Animator() {
			delete clip;
			clip = nullptr;
		}

		bool update(float dt) {
			if (!isPlaying) {
				return false;
			}
			
			playbackTime += playbackSpeed * dt;

			if (playbackTime > clip->duration) {
				if (isLooping) {
					playbackTime = (playbackTime + dt) - clip->duration;
				}
				else {
					playbackTime = clip->duration;
				}
			}
			if (playbackSpeed < 0 && playbackTime < 0) {
				if (isLooping) {
					playbackTime = clip->duration;
				}
				else {
					playbackTime = 0;
				}
			}
			return true;
		}

		glm::vec3 GetNextValue(std::vector<Vec3Key> keys) {
			// default cases
			if (keys.empty()) {
				return glm::vec3(0, 0, 0);
			}
			if (keys.size() == 1) {
				return keys.front().value;
			}

			int index = 0;
			Vec3Key* currentKey = nullptr;
			Vec3Key* nextKey = nullptr;

			for (index = 0; index < keys.size(); index++) {
				if (playbackSpeed > 0) {
					if (playbackTime <= keys[index].time) {
						nextKey = &keys[index];
						currentKey = &keys[index - 1];
						break;
					}
					
				}
				else if (playbackSpeed < 0) {
					if (playbackTime >= keys[index].time) {
						nextKey = &keys[index];
						currentKey = &keys[index + 1];
						break;
					}
				}
				
			}

			if (nextKey == nullptr) {
				return keys.back().value;
			}

			

			float val = inverseLerp(currentKey->time, nextKey->time, playbackTime);
			val = ease(val, currentKey->easeType);
			return lerp(currentKey->value, nextKey->value, val);
		}

		void handleUI() {

			if (ImGui::CollapsingHeader("Animation Settings")) {
				ImGui::Checkbox("Playing", &isPlaying);
				ImGui::DragFloat("Playback Speed", &playbackSpeed);
				ImGui::Checkbox("Looping", &isLooping);
				ImGui::SliderFloat("Playback Time", &playbackTime, 0, clip->duration);

				if (ImGui::CollapsingHeader("Position Keyframes")) {
					for (int i = 0; i < clip->positionKeys.size(); i++) {
						ImGui::PushID(i);
						ImGui::DragFloat("Time", &clip->positionKeys[i].time, 0.1f);
						ImGui::DragFloat3("Value", &clip->positionKeys[i].value.x, 0.1f);
						ImGui::Combo("Easing Type", &clip->positionKeys[i].easeType, EasingNames, EASING_FUNCTIONS.size());
						ImGui::PopID();
					}

					if (ImGui::Button("Add Keyframe")) {
						clip->positionKeys.push_back(Vec3Key(glm::vec3(0, 0, 0), -1));
					}
					if (ImGui::Button("Remove Keyframe")) {
						if (!clip->positionKeys.empty()) {
							clip->positionKeys.erase(std::find(clip->positionKeys.begin(), clip->positionKeys.end(), clip->positionKeys.back()));
						}
					}
				}

				if (ImGui::CollapsingHeader("Rotation Keyframes")) {
					for (int i = 0; i < clip->rotationKeys.size(); i++) {
						ImGui::PushID(i);
						ImGui::DragFloat("Time", &clip->rotationKeys[i].time, 0.1f);
						ImGui::DragFloat3("Value", &clip->rotationKeys[i].value.x, 0.1f);
						ImGui::Combo("Easing Type", &clip->rotationKeys[i].easeType, EasingNames, EASING_FUNCTIONS.size());
						ImGui::PopID();
					}

					if (ImGui::Button("Add Keyframe")) {
						clip->rotationKeys.push_back(Vec3Key(glm::vec3(0, 0, 0), -1));
					}
					if (ImGui::Button("Remove Keyframe")) {
						if (!clip->rotationKeys.empty()) {
							clip->rotationKeys.erase(std::find(clip->rotationKeys.begin(), clip->rotationKeys.end(), clip->rotationKeys.back()));
						}
					}
				}

				if (ImGui::CollapsingHeader("Scale Keyframes")) {
					for (int i = 0; i < clip->scaleKeys.size(); i++) {
						ImGui::PushID(i);
						ImGui::DragFloat("Time", &clip->scaleKeys[i].time, 0.1f);
						ImGui::DragFloat3("Value", &clip->scaleKeys[i].value.x, 0.1f);
						ImGui::Combo("Easing Type", &clip->scaleKeys[i].easeType, EasingNames, EASING_FUNCTIONS.size());
						ImGui::PopID();
					}

					if (ImGui::Button("Add Keyframe")) {
						clip->scaleKeys.push_back(Vec3Key(glm::vec3(0, 0, 0), -1));
					}
					if (ImGui::Button("Remove Keyframe")) {
						if (!clip->scaleKeys.empty()) {
							clip->scaleKeys.erase(std::find(clip->scaleKeys.begin(), clip->scaleKeys.end(), clip->scaleKeys.back()));
						}
					}
				}

			}
		}
	};



}