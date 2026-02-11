// Fill out your copyright notice in the Description page of Project Settings.

#include "MyCart.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Kismet/KismetMathLibrary.h"

AMyCart::AMyCart()
{
    PrimaryActorTick.bCanEverTick = true;

    // 1. 메시 설정
    MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BodyMesh"));
    RootComponent = MeshComponent;
    MeshComponent->SetSimulatePhysics(true);

    // 중요: 물리 엔진이 맘대로 마찰력을 계산하지 못하게 막고, 우리가 코드로 제어합니다.
    MeshComponent->SetLinearDamping(0.5f);  // 공기 저항
    MeshComponent->SetAngularDamping(4.0f); // 회전 저항 (팽이처럼 도는 것 방지)

    // 2. 카메라 설정 (카트라이더 뷰)
    SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
    SpringArm->SetupAttachment(RootComponent);
    SpringArm->TargetArmLength = 400.0f;
    SpringArm->SocketOffset = FVector(0, 0, 150.0f);
    SpringArm->bEnableCameraLag = true; // 카메라가 부드럽게 따라오게 함
    SpringArm->CameraLagSpeed = 10.0f;

    Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
    Camera->SetupAttachment(SpringArm);

    // 변수 초기화
    CurrentThrottle = 0.0f;
    CurrentSteer = 0.0f;
    bIsDrifting = false;

    if (MeshComponent)
    {
        MeshComponent->SetSimulatePhysics(true);
        MeshComponent->SetEnableGravity(true);
        MeshComponent->SetMobility(EComponentMobility::Movable);
    }
}

void AMyCart::BeginPlay()
{
    Super::BeginPlay();

    // 매핑 컨텍스트 추가
    if (APlayerController* PC = Cast<APlayerController>(GetController()))
    {
        if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
        {
            Subsystem->AddMappingContext(DefaultMappingContext, 0);
        }
    }
}

void AMyCart::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 물리 연산 실행
    ApplyMovementPhysics(DeltaTime);
    ApplyLateralFriction(DeltaTime);
}

// ==========================================
// [1] 물리 이동 로직 (가속, 회전, 중력)
// ==========================================
void AMyCart::ApplyMovementPhysics(float DeltaTime)
{
    if (!MeshComponent) return;

    // 1. 가속 (AccelChange = true로 질량 무시)
    // 카트가 바닥에 붙어있을 때만 가속 가능하게 하려면 Raycast가 필요하지만, 
    // 지금은 간단하게 처리합니다.
    if (CurrentThrottle != 0.0f)
    {
        FVector ForwardForce = GetActorForwardVector() * CurrentThrottle * Acceleration;

        // 최고 속도 제한 로직
        float CurrentSpeed = MeshComponent->GetPhysicsLinearVelocity().Size();
        if (CurrentSpeed < MaxSpeed)
        {
            MeshComponent->AddForce(ForwardForce, NAME_None, true);
        }
    }

    // 2. 회전 (제자리 회전 가능 + 속도 비례 조향)
    if (CurrentSteer != 0.0f)
    {
        // 후진 시 핸들 반대 방향 처리
        float SpeedDot = FVector::DotProduct(MeshComponent->GetPhysicsLinearVelocity(), GetActorForwardVector());
        float DirectionMult = (SpeedDot >= -10.0f) ? 1.0f : -1.0f;

        // 드리프트 중이면 회전이 더 잘 먹게 보정
        float DriftBonus = bIsDrifting ? 1.5f : 1.0f;

        FVector Torque = GetActorUpVector() * CurrentSteer * TurnSpeed * DirectionMult * DriftBonus;
        MeshComponent->AddTorqueInDegrees(Torque, NAME_None, true);
    }

    // 3. 인공 중력 (카트라이더처럼 바닥에 쩍 달라붙게 함)
    FVector Gravity = FVector(0, 0, -GravityForce);
    MeshComponent->AddForce(Gravity, NAME_None, true);
}

// ==========================================
// [2] 측면 마찰력 로직 (카트라이더 물리엔진의 핵심)
// ==========================================
void AMyCart::ApplyLateralFriction(float DeltaTime)
{
    if (!MeshComponent) return;

    // 현재 속도 벡터
    FVector Velocity = MeshComponent->GetPhysicsLinearVelocity();

    // 카트의 오른쪽 방향 벡터
    FVector RightVector = GetActorRightVector();

    // 내 속도 중 "옆으로 미끄러지는 속도" 성분만 추출 (내적)
    float LateralSpeed = FVector::DotProduct(Velocity, RightVector);

    // 마찰력 계수 결정 (드리프트 키를 누르면 마찰력을 낮춤)
    float CurrentGrip = bIsDrifting ? GripFactorDrift : GripFactorNormal;

    // 옆으로 가는 속도를 반대 방향으로 힘을 주어 상쇄시킴
    // F = -kv (속도에 비례한 저항력)
    FVector FrictionForce = -RightVector * LateralSpeed * CurrentGrip;

    // 이 힘을 물리 엔진에 적용 (AccelChange = true)
    MeshComponent->AddForce(FrictionForce, NAME_None, true);
}

// ==========================================
// [3] 입력 처리
// ==========================================
void AMyCart::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    UE_LOG(LogTemp, Warning, TEXT("SetupPlayerInputComponent Called!"));

    if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
    {
        EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Triggered, this, &AMyCart::InputThrottle);
        EnhancedInputComponent->BindAction(ThrottleAction, ETriggerEvent::Completed, this, &AMyCart::InputThrottle); // 키 떼면 0으로

        EnhancedInputComponent->BindAction(SteerAction, ETriggerEvent::Triggered, this, &AMyCart::InputSteer);
        EnhancedInputComponent->BindAction(SteerAction, ETriggerEvent::Completed, this, &AMyCart::InputSteer);

        EnhancedInputComponent->BindAction(DriftAction, ETriggerEvent::Started, this, &AMyCart::InputDriftStart);
        EnhancedInputComponent->BindAction(DriftAction, ETriggerEvent::Completed, this, &AMyCart::InputDriftEnd);
    }
}

void AMyCart::InputThrottle(const FInputActionValue& Value)
{
    CurrentThrottle = Value.Get<float>();
    UE_LOG(LogTemp, Warning, TEXT("AMyCart::InputThrottle"));
}

void AMyCart::InputSteer(const FInputActionValue& Value)
{
    CurrentSteer = Value.Get<float>();
    UE_LOG(LogTemp, Warning, TEXT("AMyCart::InputSteer"));
}

void AMyCart::InputDriftStart(const FInputActionValue& Value)
{
    bIsDrifting = true;
    // 드리프트 시작 시 살짝 점프하는 느낌 (옵션)
    // MeshComponent->AddImpulse(FVector(0,0,300), NAME_None, true); 
}

void AMyCart::InputDriftEnd(const FInputActionValue& Value)
{
    bIsDrifting = false;
}