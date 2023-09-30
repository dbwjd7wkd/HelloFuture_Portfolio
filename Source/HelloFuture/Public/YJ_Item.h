// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "YJ_Item.generated.h"

/**
 * 
 */
UCLASS(Abstract, BlueprintType, Blueprintable, EditInlineNew, DefaultToInstanced)
class HELLOFUTURE_API UYJ_Item : public UObject
{
	GENERATED_BODY()
	
public:
	// 레벨 가져오기
	virtual class UWorld* GetWorld() const { return World; };

	// 속성 초기값 설정
	UYJ_Item();

public:
	// 현재 레벨
	UPROPERTY(Transient)
		class UWorld* World;
	//  인벤토리
	UPROPERTY()
		class UYJ_InventoryComponent* OwningInventory;

	/**아이템 정보*/
	// 사용할 아이템 카테고리 (ex. use, equip, etc)
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
		FText Category;
	// 아이템 썸네일
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
		UTexture2D* Thumbnail;
	// 아이템 이름
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
		FText ItemDisplayName;
	// 아이템 설명
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item", meta = (MultiLine = true))
		FText ItemDescription;
	// 아이템 가격
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
		int32 Price;
	// 아이템 랜덤 가격 최솟값
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
		int32 MinPrice;
	// 아이템 랜덤 가격 최댓값
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
		int32 MaxPrice;
	// 아이템 인덱스
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Item")
		int32 ItemIndex;

	/** 플레이어의 현재 아이템 인벤토리 정보*/
	// 아이템 갯수
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Item")
		int32 Count;
	// 아이템 인벤토리 인덱스
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Item")
		int32 InventoryIndex;

};
