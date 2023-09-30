// Fill out your copyright notice in the Description page of Project Settings.


#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "YJ_InventoryComponent.h"
#include "YJ_GameInstance.generated.h"

UCLASS()
class HELLOFUTURE_API UYJ_GameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	// 초기값 셋팅
	UYJ_GameInstance();

	// 원하는 아이템 enum으로 가져오기
	UFUNCTION(BlueprintCallable)
	class UYJ_Item* GetItemAsEnum(EItemEnum ItemEnum);

	// 내 플레이어의 컴퓨터 주소 가져오기
	UFUNCTION(BlueprintCallable)
	FString GetMyIpAddress();
	
public:
	// 모든 아이템 인스턴스
	// @note 플레이 시 모든 아이템들 객체 들어가있음
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Instanced)
		TArray<class UYJ_Item*> allItems;

	// ClothingShopMap에서 구매한 옷을 배치할 위치 백터 어레이 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ClosetSpawnLocation")
		TArray<FVector> closetSpawnLocation;

	// 시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
		int32 worldTime = 1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
		FDateTime worldTime_Structure;
};
