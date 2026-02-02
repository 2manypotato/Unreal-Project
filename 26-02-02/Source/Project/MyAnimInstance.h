// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "MyAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class PROJECT_API UMyAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	


public:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	virtual void NativeInitializeAnimation() override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MoveSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool BIsFalling;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	class AMyCharacter* MyCharacter;
};
