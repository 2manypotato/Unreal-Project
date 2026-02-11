// Fill out your copyright notice in the Description page of Project Settings.


#include "MyAnimInstance.h"
#include "MyCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UMyAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	//애니메이션 소유 캐릭터 지정
	MyCharacter = Cast<AMyCharacter>(TryGetPawnOwner());
}

void UMyAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if(MyCharacter)
	{
		MoveSpeed = MyCharacter->GetVelocity().Size2D();

		bIsFalling = MyCharacter->GetCharacterMovement()->IsFalling();

		bIsAttacking = MyCharacter->bIsAttacking;
	}
}