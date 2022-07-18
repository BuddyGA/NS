#include "cstEffect.h"



NS_CLASS_BEGIN(cstEffect, nsObject)
NS_CLASS_END(cstEffect)

cstEffect::cstEffect()
{
	Duration = 0.0f;
	Interval = 0.0f;
	MaxStack = 1;
	AffectedCharacter = nullptr;
	RemainingTime = 0.0f;
}


cstExecute::EResult cstEffect::CanExecute(float currentTime, const nsObject* sourceObject, const cstExecute::TargetParams& targetParams) const
{
	return cstExecute::RESULT_SUCCESS;
}


cstExecute::EResult cstEffect::Execute(float currentTime, const nsObject* sourceObject, const cstExecute::TargetParams& targetParams)
{
	const cstExecute::EResult result = CanExecute(currentTime, sourceObject, targetParams);

	return result;
}


void cstEffect::UpdateExecution(float deltaTime)
{

}
