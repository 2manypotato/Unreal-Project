// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/Character.h"
#include "GameFramework/Controller.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameEngine.h"
#include "DrawDebugHelpers.h"
#include "AIController.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "TimerManager.h"

// Sets default values
AMyCharacter::AMyCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	//점프 높이
	GetCharacterMovement()->JumpZVelocity = 400.f;
	//공중 컨트롤, 값이 클수록 쉽다
	GetCharacterMovement()->AirControl = 0.35f;

}

// Called when the game starts or when spawned
void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();

	CurrentHealth = MaxHealth;

	GetMesh()->SetSimulatePhysics(false);

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (PC->PlayerCameraManager)
		{
			PC->PlayerCameraManager->ViewPitchMin = -60.0f; // 아래로 내려다보는 각도 제한
			PC->PlayerCameraManager->ViewPitchMax = 30.0f;  // 위로 올려다보는 각도 제한
		}
	}

}

// Called every frame
void AMyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

	if (EnhancedInputComponent)
	{
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMyCharacter::Move);
		}

		if (JumpAction)
		{
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &AMyCharacter::Jump);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		}

		if (LookAction)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMyCharacter::Look);
		}

		if (AttackAction)
		{
			EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &AMyCharacter::Attack);
		}

		if (RollAction)
		{
			EnhancedInputComponent->BindAction(RollAction, ETriggerEvent::Triggered, this, &AMyCharacter::Roll);
		}

		if (FireAction)
		{
			EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &AMyCharacter::Fire);
		}
	}

}

void AMyCharacter::PossessedBy(AController* NewController)
{
	UE_LOG(LogTemp, Log, TEXT("AMyCharacter::PossessedBy"));
	Super::PossessedBy(NewController);

	APlayerController* pc = Cast<APlayerController>(NewController);

	if (pc)
	{
		ULocalPlayer* LocalPlayer = pc->GetLocalPlayer();
		if (LocalPlayer)
		{
			UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
			if (InputSubsystem)
			{
				UE_LOG(LogTemp, Log, TEXT("AMyCharacter::PossessedBy - InputSubsystem is valid"));
				InputSubsystem->AddMappingContext(InputMappingContext, 0);
			}
			else
			{
				UE_LOG(LogTemp, Log, TEXT("AMyCharacter::PossessedBy - InputSubsystem is invalid"));
			}
		}
	}
}

void AMyCharacter::Move(const FInputActionValue& Value)
{
	const FVector2D MovementVector = Value.Get<FVector2D>();

	const FRotator Rotation = Controller->GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	AddMovementInput(ForwardDirection, MovementVector.Y);

	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
	AddMovementInput(RightDirection, MovementVector.X);
}

void AMyCharacter::Jump()
{
	Super::Jump();

	UE_LOG(LogTemp, Log, TEXT("AMyCharacter::Jump"));
}

void AMyCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	AddControllerYawInput(LookAxisVector.X);

	AddControllerPitchInput(-LookAxisVector.Y);
}

void AMyCharacter::Attack(const FInputActionValue& Value)
{
	ProcessAttack();
}

void AMyCharacter::Roll(const FInputActionValue& Value)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance&&!AnimInstance->Montage_IsPlaying(RollMontage))
	{
		AnimInstance->Montage_Play(RollMontage);
	}
}

void AMyCharacter::Fire(const FInputActionValue& Value)
{
	if (!bCanFire)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("AMyCharacter::Fire"));
	if (ProjectileClass)
	{
		UE_LOG(LogTemp, Log, TEXT("AMyCharacter::Fire - if(projectilclass)"));


		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

		if (AnimInstance && !AnimInstance->Montage_IsPlaying(FireMontage))
		{
			AnimInstance->Montage_Play(FireMontage);
		}

		HandleFire();
		
	}

	GetWorldTimerManager().SetTimer(TimerHandle_FireRate, this, &AMyCharacter::ResetFire, FireRate, false);
}

void AMyCharacter::HandleFire()
{
	FVector CameraLocation;
	FRotator CameraRotation;
	GetActorEyesViewPoint(CameraLocation, CameraRotation);

	FVector MuzzleLocation = CameraLocation + FTransform(CameraRotation).TransformVector(MuzzleOffset);
	FRotator MuzzleRotation = CameraRotation;
	//MuzzleRotation.Pitch += 10.f;

	UWorld* World = GetWorld();
	if (World)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.Instigator = GetInstigator();

		AFire* Projectile = World->SpawnActor<AFire>(ProjectileClass, MuzzleLocation, MuzzleRotation, SpawnParams);
		if (Projectile)
		{
			FVector LaunchDirection = MuzzleRotation.Vector();
			Projectile->FireInDirection(LaunchDirection);
		}
	}
}

void AMyCharacter::ResetFire()
{
	bCanFire = true; // 다시 공격 가능하도록 변경
	UE_LOG(LogTemp, Log, TEXT("Fire Ready"));
}

float AMyCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	CurrentHealth = FMath::Clamp(CurrentHealth - ActualDamage, 0.0f, MaxHealth);

	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("CurrentHp : %.2f"), CurrentHealth));

	// 3. 사망 판정
	if (CurrentHealth <= 0)
	{
		Die();
	}

	return ActualDamage;
}

void AMyCharacter::ProcessAttack()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance)
	{
		AnimInstance->Montage_Play(AttackMontage);
		UE_LOG(LogTemp, Log, TEXT("AMyCharacter::ProcessAttack"));
		IsAttack();
	}
}

void AMyCharacter::IsAttack()
{
	FHitResult HitResult;
	FCollisionQueryParams Params(SCENE_QUERY_STAT(Attack), false, this);

	// 1. 캡슐 설정 (가로로 누울 것이므로 HalfHeight가 가로 길이가 됨)
	float CapsuleRadius = 30.f;
	float CapsuleHalfHeight = 90.f; // 양옆으로 뻗는 길이

	// 2. 위치 설정: 플레이어 발 밑이 아니라 가슴 높이(+50) 정도로 올림
	FVector CenterLocation = GetActorLocation() + (GetActorForwardVector() * 100.f) + (GetActorUpVector() * 30.f);

	// 3. 캡슐 눕히기 (Pitch를 90도 회전시켜 가로로 만듦)
	// 캐릭터의 현재 회전값에 Pitch 90도를 더함
	FRotator CapsuleRotator = GetActorRotation();
	CapsuleRotator.Pitch += 90.f;
	FQuat CapsuleQuat = FQuat(CapsuleRotator);

	// 4. 캡슐 판정 수행 (가로로 누운 상태이므로 위치 이동 없이 중심점에서 즉시 검사)
	bool bHasHit = GetWorld()->SweepSingleByChannel(
		HitResult,
		CenterLocation,
		CenterLocation, // 시작과 끝을 같게 하면 그 자리에서 즉시 충돌 검사
		CapsuleQuat,
		ECollisionChannel::ECC_Visibility,
		FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight),
		Params
	);

	// 5. 디버그 캡슐 그리기
	FColor DrawColor = bHasHit ? FColor::Green : FColor::Red;
	DrawDebugCapsule(GetWorld(), CenterLocation, CapsuleHalfHeight, CapsuleRadius, CapsuleQuat, DrawColor, false, 2.0f);

	if (bHasHit)
	{
		if (AActor* HitActor = HitResult.GetActor())
		{
			UGameplayStatics::ApplyDamage(HitActor, AttackDamage, GetController(), this, UDamageType::StaticClass());
			UE_LOG(LogTemp, Log, TEXT("Hit Actor: %s"), *HitActor->GetName());
		}
	}
}

void AMyCharacter::Die()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

	if (AnimInstance)
	{
		AnimInstance->Montage_Play(DieMontage);
		UE_LOG(LogTemp, Log, TEXT("AMyCharacter::Die"));
	}

	GetWorldTimerManager().SetTimer(MemberTimerHandle, [this]()
		{
			this->Destroy();
		}, 3.0f, false);

	
}
