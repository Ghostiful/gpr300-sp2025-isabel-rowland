#include <glm/glm.hpp>

namespace ir {
	struct Joint {
		
	};
	
	struct Skeleton {
		unsigned int jointCount;
		Joint* joints;
	};

	struct JointPose {
		// Quaternion rotation;
		glm::vec3 translation;
		glm::vec3 scale;
	};

	struct SkeletonPose {
		Skeleton* skeleton;
		JointPose* localPose;
	};
}