#include "nsUnitTest.h"
#include "nsMath.h"

#define GLM_FORCE_DEPTH_TO_ONE
#define GLM_FORCE_LEFT_HANDED
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/random.hpp>
#include <glm/gtx/matrix_decompose.hpp>


NS_INLINE void DecomposeVector(nsVector3& normalCompo, nsVector3& tangentCompo, const nsVector3& outwardDir, const nsVector3& outwardNormal)
{
	normalCompo = nsVector3::Project(outwardDir, outwardNormal);
	tangentCompo = outwardDir - normalCompo;
}


NS_INLINE nsVector3 ReflectVector(const nsVector3& vec, const nsVector3& normal)
{
	return vec - nsVector3::Project(vec, normal) * 2.0f;
}


static void Test_Vector()
{
	{
		const nsVector3 testDecompose(150.0f, -980.0f, 150.0f);
		nsVector3 a, b;
		DecomposeVector(a, b, testDecompose, nsVector3::UP);
		NS_Validate(a.IsEquals(nsVector3(0.0f, -980.0f, 0.0f)));
		NS_Validate(b.IsEquals(nsVector3(150.0f, 0.0f, 150.0f)));
	}

	{
		const nsVector3 testReflect(0.0f, -100.0f, 0.0f);
		const nsVector3 reflected = nsVector3::Reflect(testReflect, nsVector3::UP);
		NS_Validate(reflected.IsEquals(nsVector3(0.0f, 100.0f, 0.0f)));
	}

	{
		const nsVector3 first(0.0f, 0.0f, -1.0f);
		const nsVector3 second(0.0f, 1.0f, 0.0f);
		const float dot = nsVector3::DotProduct(first, second);
		const float angle = nsVector3::AngleBetween(first, second);
		const float radianClamped = nsMath::Clamp(angle, -1.0f, 1.0f);
		const float degree = nsMath::RadToDeg(angle);

	}
}	


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
	Test_Vector();
	Test_Rotation();
}
