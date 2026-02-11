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
    AMyCart();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    // --- 컴포넌트 ---
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UStaticMeshComponent* MeshComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USpringArmComponent* SpringArm;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class UCameraComponent* Camera;

    // --- 입력 에셋 (BP에서 할당) ---
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    class UInputMappingContext* DefaultMappingContext;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    class UInputAction* ThrottleAction; // W, S

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    class UInputAction* SteerAction;    // A, D

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Input")
    class UInputAction* DriftAction;    // Shift (드리프트)

    // --- 주행 설정 (블루프린트에서 조절 가능) ---
    UPROPERTY(EditAnywhere, Category = "Kart Specs")
    float MaxSpeed = 3000.0f;           // 최고 속도

    UPROPERTY(EditAnywhere, Category = "Kart Specs")
    float Acceleration = 4000.0f;       // 가속력 (질량 무시)

    UPROPERTY(EditAnywhere, Category = "Kart Specs")
    float TurnSpeed = 150.0f;           // 회전 속도

    UPROPERTY(EditAnywhere, Category = "Kart Specs")
    float GravityForce = 2000.0f;       // 바닥에 붙여주는 힘

    // --- 드리프트 설정 ---
    UPROPERTY(EditAnywhere, Category = "Drift Specs")
    float GripFactorNormal = 10.0f;     // 평소 주행 시 미끄러짐 방지 (높을수록 안 미끄러짐)

    UPROPERTY(EditAnywhere, Category = "Drift Specs")
    float GripFactorDrift = 0.5f;       // 드리프트 시 미끄러짐 허용 (낮을수록 잘 미끄러짐)

private:
    // 내부 상태 변수
    float CurrentThrottle;
    float CurrentSteer;
    bool bIsDrifting;

    // 입력 함수
    void InputThrottle(const FInputActionValue& Value);
    void InputSteer(const FInputActionValue& Value);
    void InputDriftStart(const FInputActionValue& Value);
    void InputDriftEnd(const FInputActionValue& Value);

    // 물리 처리 함수 (핵심)
    void ApplyMovementPhysics(float DeltaTime);
    void ApplyLateralFriction(float DeltaTime); // 미끄러짐 방지 로직
};