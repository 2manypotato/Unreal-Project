// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "InputMappingContext.h"
#include "Fire.h"
#include "MyCharacter.generated.h"

UCLASS()
class PROJECT_API AMyCharacter : public ACharacter
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputMappingContext* InputMappingContext;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* MoveAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* JumpAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* LookAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* AttackAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* RollAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	UInputAction* FireAction;

public:
	// Sets default values for this character's properties
	AMyCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void PossessedBy(AController* NewController) override;

	FTimerHandle MemberTimerHandle;
	FTimerHandle TimerHandle_FireRate;
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool IsJumping;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsAttacking;

protected:
	UFUNCTION()
	void Move(const FInputActionValue& Value);

	UFUNCTION()
	void Look(const FInputActionValue& Value);

	virtual void Jump() override;

	UFUNCTION()
	void Attack(const FInputActionValue& Value);

	UFUNCTION()
	void Roll(const FInputActionValue& Value);

	UFUNCTION()
	void Fire(const FInputActionValue& Value);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* AttackMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* RollMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* DieMontage;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UAnimMontage* FireMontage;

public:
	UFUNCTION()
	void IsAttack();

	UFUNCTION()
	void Die();

	UFUNCTION(BlueprintCallable)
	void HandleFire();

	UFUNCTION(BlueprintCallable)
	void ProcessAttack();

	UFUNCTION()
	void ResetFire();

	float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float CurrentHealth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float MaxHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float AttackDamage = 20.0f;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float FireRate = 0.5f;

	bool bCanFire = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GamePlay")
	FVector MuzzleOffset;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Projectile")
	TSubclassOf<class AFire> ProjectileClass;

	UPROPERTY(EditAnywhere, Category = "Projectile")
	USphereComponent* CollisionComponent;

	UPROPERTY(EditAnywhere, Category = "Movement")
	UProjectileMovementComponent* ProjectileMovementComponent;

	UPROPERTY(EditAnywhere, Category = "Movement")
	UStaticMeshComponent* ProjectileMeshComponent;
};