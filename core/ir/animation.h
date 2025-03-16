#include <glm/glm.hpp>
#include <vector>
#include <math.h>

// All easing functions are from https://easings.net/

namespace ir {
	// Easing functions
	enum EasingType {
		NONE,
		IN_OUT_ELASTIC,
		IN_SINE,
		OUT_BACK
	};

	const float PI = 3.141592653;

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

	const std::vector<EasingFunc*> EASING_FUNCTIONS = { easeInOutElastic, easeInSine, easeOutBack};

	float ease(float x, EasingType type) {
		if (type == NONE) {
			return x;
		}
		return EASING_FUNCTIONS[type](x);
	}
	
	struct Vec3Key {
		glm::vec3 value;
		float time;
		EasingType easeType;

		Keyframe() {
			time = -1; // ignore frames with -1 time
			easeType = NONE;
		}

		Keyframe(glm::vec3 _value, float _time) {
			value = _values;
			time = _time;
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

	class Animator {
		AnimationClip* clip;
		bool isPlaying;
		float playbackSpeed; // negatives play backwards
		bool isLooping; // if false, stop once clip.duration is reached
		float playbackTime; // current time, between 0 and clip.duration

		Animator() {
			clip = nullptr;
			isPlaying = false;
			playbackSpeed = 0;
			isLooping = false;
			playbackSpeed = 0;
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

			if (playbackSpeed > clip->duration) {
				if (isLooping) {
					playbackTime = playbackTime + dt - clip->duration;
				}
				else {
					isPlaying = false;
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

			for (; index < keys.size(); index++) {
				if (playbackTime <= keys[index].time) {
					nextKey = &keys[index];
					break;
				}
			}

			if (nextKey == nullptr) {
				return keys.back().value;
			}
		}
	};

}