// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "YJ_GameModeBase.generated.h"

UCLASS()
class HELLOFUTURE_API AYJ_GameModeBase : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	// 은행창구 대기번호
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Numbering System")
		int32 WaitingNumber = 1;

public:
	// 은행창구 대기번호 더하기
	UFUNCTION(BlueprintCallable)
		void AddWaitingNumber();
	// 은행창구 대기번호 빼기
	UFUNCTION(BlueprintCallable)
		void MinusWaitingNumber();
	// 은행창구 내 차례 맞는지 확인
	UFUNCTION(BlueprintCallable)
		bool CheckMyOrder(class AHelloFutureCharacter* Player);
};
