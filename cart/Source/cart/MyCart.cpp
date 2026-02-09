// Fill out your copyright notice in the Description page of Project Settings.

#include "MyCart.h"
#include "Components/StaticMeshComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

// Sets default values
AMyCart::AMyCart()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
	RootComponent = MeshComponent;

	MeshComponent->SetSimulatePhysics(true);

	AccelerationForce = 100000.0f;
}

// Called when the game starts or when spawned
void AMyCart::BeginPlay()
{
	Super::BeginPlay();
	
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(InputMappingContext, 0);
		}
	}
}

// Called every frame
void AMyCart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AMyCart::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Triggered, this, &AMyCart::Throttle);
		EnhancedInputComponent->BindAction(SteerAction, ETriggerEvent::Triggered, this, &AMyCart::Steer);
	}

}

void AMyCart::Throttle(const FInputActionValue& Value)
{
	float MoveValue = Value.Get<float>();
	if (MoveValue != 0.0f && MeshComponent)
	{
		MeshComponent->AddForce(GetActorForwardVector() * MoveValue * AccelerationForce, NAME_None, true);
	}
	UE_LOG(LogTemp, Log, TEXT("AMyCart::Throttle"));
}

void AMyCart::Steer(const FInputActionValue& Value)
{
	float SteerValue = Value.Get<float>();

	if (FMath::Abs(SteerValue) > 0.05f && MeshComponent)
	{
		// 현재 전진 속도 벡터와 전진 방향 벡터를 내적하여 '순수 전진 속도'를 구함
		float ForwardSpeed = FVector::DotProduct(MeshComponent->GetPhysicsLinearVelocity(), GetActorForwardVector());

		// 후진 중일 때는 조향이 반대로 되어야 하므로 방향을 체크
		float Direction = (ForwardSpeed >= 0) ? 1.0f : -1.0f;

		// 속도에 따른 회전 보정 (너무 느리면 회전 안함)
		float SpeedAlpha = FMath::Clamp(FMath::Abs(ForwardSpeed) / 100.0f, 0.0f, 1.0f);

		FVector Torque = GetActorUpVector() * SteerValue * 400.0f * SpeedAlpha * Direction;
		MeshComponent->AddTorqueInDegrees(Torque, NAME_None, true);
	}
	UE_LOG(LogTemp, Log, TEXT("AMyCart::Steer"));
}