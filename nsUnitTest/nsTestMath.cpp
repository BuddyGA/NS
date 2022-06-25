#include "nsUnitTest.h"
#include "nsMath.h"

#define GLM_FORCE_DEPTH_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/matrix_decompose.hpp>


static void Test_Rotation()
{
	nsVector3 __rotation(90.0f, -40.0f, 0.0f);

	glm::quat glmQuat(glm::vec3(glm::radians(__rotation.X), glm::radians(__rotation.Y), glm::radians(__rotation.Z)));

	nsQuaternion quat = nsQuaternion::FromRotation(__rotation.X, __rotation.Y, __rotation.Z);

	glm::vec3 glmFromQuat = glm::eulerAngles(glmQuat);
	glmFromQuat.x = glm::degrees(glmFromQuat.x);
	glmFromQuat.y = glm::degrees(glmFromQuat.y);
	glmFromQuat.z = glm::degrees(glmFromQuat.z);

	nsVector3 fromQuat = quat.GetPitchYawRoll();

	NS_Validate(nsMath::FloatEquals(fromQuat.X, __rotation.X, NS_MATH_EPS_LOW_P));
	NS_Validate(nsMath::FloatEquals(fromQuat.Y, __rotation.Y, NS_MATH_EPS_LOW_P));
	NS_Validate(nsMath::FloatEquals(fromQuat.Z, __rotation.Z, NS_MATH_EPS_LOW_P));


	// Axis angle
	{
		nsVector3 RESULT = nsVector3(0.707f, 0.0f, 0.707f);

		nsVector3 v = nsVector3::FORWARD;
		nsVector3 a = nsVector3::UP;
		const float radian = nsMath::DegToRad(45.0f);
		nsVector3 rotV = v * nsMath::Cos(radian) + nsVector3::CrossProduct(a, v) * nsMath::Sin(radian) + v * nsVector3::DotProduct(a, v) * (1.0f - nsMath::Cos(radian));
		bool bValid = rotV.IsEquals(RESULT, 0.001f);
		NS_Validate(bValid);
	}
}


void nsUnitTest::TestMath()
{
	Test_Rotation();
}
