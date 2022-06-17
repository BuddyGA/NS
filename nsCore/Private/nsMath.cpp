#include "nsMath.h"


const nsColor nsColor::BLACK(0, 0, 0, 255);
const nsColor nsColor::BLACK_TRANSPARENT(0, 0, 0, 0);
const nsColor nsColor::BLUE(0, 0, 255, 255);
const nsColor nsColor::CYAN(0, 255, 255, 255);
const nsColor nsColor::GRAY(125, 125, 125, 255);
const nsColor nsColor::GREEN(0, 255, 0, 255);
const nsColor nsColor::RED(255, 0, 0, 255);
const nsColor nsColor::YELLOW(255, 255, 0, 255);
const nsColor nsColor::WHITE(255, 255, 255, 255);
const nsColor nsColor::WHITE_TRANSPARENT(255, 255, 255, 0);


const nsLinearColor nsLinearColor::BLACK(0.0f, 0.0f, 0.0f, 1.0f);
const nsLinearColor nsLinearColor::BLACK_TRANSPARENT(0.0f, 0.0f, 0.0f, 0.0f);
const nsLinearColor nsLinearColor::BLUE(0.0f, 0.0f, 1.0f, 1.0f);
const nsLinearColor nsLinearColor::CYAN(0.0f, 1.0f, 1.0f, 1.0f);
const nsLinearColor nsLinearColor::GREEN(0.0f, 1.0f, 0.0f, 1.0f);
const nsLinearColor nsLinearColor::RED(1.0f, 0.0f, 0.0f, 1.0f);
const nsLinearColor nsLinearColor::YELLOW(1.0f, 1.0f, 0.0f, 1.0f);
const nsLinearColor nsLinearColor::WHITE(1.0f, 1.0f, 1.0f, 1.0f);
const nsLinearColor nsLinearColor::WHITE_TRANSPARENT(1.0f, 1.0f, 1.0f, 0.0f);


const nsVector2 nsVector2::ZERO(0.0f, 0.0f);


const nsVector3 nsVector3::ZERO(0.0f, 0.0f, 0.0f);
const nsVector3 nsVector3::RIGHT(1.0f, 0.0f, 0.0f);
const nsVector3 nsVector3::UP(0.0f, 1.0f, 0.0f);
const nsVector3 nsVector3::FORWARD(0.0f, 0.0f, 1.0f);


const nsQuaternion nsQuaternion::IDENTITY(0.0f, 0.0f, 0.0f, 1.0f);


const nsMatrix3 nsMatrix3::IDENTITY(1.0f, 0.0f, 0.0f,
									0.0f, 1.0f, 0.0f,
									0.0f, 0.0f, 1.0f);


const nsMatrix4 nsMatrix4::IDENTITY(1.0f, 0.0f, 0.0f, 0.0f,
									0.0f, 1.0f, 0.0f, 0.0f,
									0.0f, 0.0f, 1.0f, 0.0f,
									0.0f, 0.0f, 0.0f, 1.0f);
