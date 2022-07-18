#pragma once

#include "cstTypes.h"



class cstEffect : public nsObject
{
	NS_DECLARE_OBJECT(cstEffect)

public:
	cstAttributes Attributes;
	float Duration;
	float Interval;
	int MaxStack;

private:
	cstCharacter* AffectedCharacter;
	float RemainingTime;


public:
	cstEffect();
	cstExecute::EResult CanExecute(float currentTime, const nsObject* sourceObject, const cstExecute::TargetParams& targetParams) const;
	cstExecute::EResult Execute(float currentTime, const nsObject* sourceObject, const cstExecute::TargetParams& targetParams);
	void UpdateExecution(float deltaTime);

};
