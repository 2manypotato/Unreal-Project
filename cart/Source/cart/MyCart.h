// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "InputActionValue.h"
#include "MyCart.generated.h"

UCLASS()
class CART_API AMyCart : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AMyCart();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
	class UStaticMeshComponent* MeshComponent;

	UPROPERTY(EditAnywhere, Category = "Input")
	class UInputMappingContext* InputMappingContext;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	UPROPERTY(EditAnywhere, Category = "Input")
	class UInputAction* ThrottleAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	class UInputAction* SteerAction;

public:
	void Throttle(const FInputActionValue& Value);
	void Steer(const FInputActionValue& Value);

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Physics")
	float AccelerationForce = 1500000.f; //accel change 사용 수치
};
