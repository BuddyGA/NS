#include "nsUnitTest.h"
#include "nsMath.h"



static void Test_Rotation()
{
	nsVector3 rotation(30.0f, 30.0f, 0.0f);
	nsQuaternion quat = nsQuaternion::FromRotation(rotation);
	nsVector3 fromQuat = quat.GetPitchYawRoll();
	NS_Validate(nsMath::FloatEquals(fromQuat.X, rotation.X, NS_MATH_EPS_LOW_P));
	NS_Validate(nsMath::FloatEquals(fromQuat.Y, rotation.Y, NS_MATH_EPS_LOW_P));
	NS_Validate(nsMath::FloatEquals(fromQuat.Z, rotation.Z, NS_MATH_EPS_LOW_P));


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
