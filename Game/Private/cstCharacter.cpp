#include "cstCharacter.h"



NS_DEFINE_OBJECT(cstCharacter, nsActor);

cstCharacter::cstCharacter()
{
	CharMovementComp = AddComponent<nsCharacterMovementComponent>("character_movement");
	CharMovementComp->CapsuleHeight = 120.0f;
	CharMovementComp->CapsuleRadius = 36.0f;
	SetRootComponent(CharMovementComp);


}
