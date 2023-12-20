// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Item.generated.h"

UCLASS()
class HELLOFUTURE_API AItem : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AItem();
	virtual void OnConstruction(const FTransform& Transform);
	// 부모 블루프린트 Construction Script에서 호출 필수.
	UFUNCTION(BlueprintCallable)
		void Construct();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
protected:
	UFUNCTION()
		void OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bfromSweep, const FHitResult& SweepResult);
	UFUNCTION()
		void OnComponentEndOverlap(UPrimitiveComponent* overlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	// 아이템 기능 사용: 순수 가상함수
	UFUNCTION()
		virtual void UseItem() PURE_VIRTUAL(AItem::UseItem, );

private:
	UFUNCTION()
		void PickUpItem();

protected:
	/**아이템 몸체*/
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components")
		class USceneComponent* SceneRoot;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components")
		class UCapsuleComponent* Capsule;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Components")
		class UStaticMeshComponent* StaticMesh;

public:
	/**아이템 정보*/
	// 아이템 메시
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
		class UStaticMesh* NewMesh;
	// 아이템 썸네일
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
		UTexture2D* Thumbnail;
	// 아이템 이름
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
		FText ItemDisplayName;
	// 아이템 설명
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item", meta = (MultiLine = true))
		FText ItemDescription;
	// 아이템 기능 배열
	//UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	//	TArray<TSubclassOf<class UItemAffect>> AffectArray;
	// 아이템 가격
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
		int32 Price;
	// 아이템 랜덤 가격 최솟값
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
		int32 MinPrice;
	// 아이템 랜덤 가격 최댓값
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
		int32 MaxPrice;

	// 아이템 줍는 동작 애니메이션
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
		class UAnimMontage* PickingUpMontage;
	// 아이템 주울 때 사운드
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item")
		class USoundCue* PickingUpSound;

private:
	/** 오버랩 정보*/
	UPROPERTY()
		class AHelloFutureCharacter* Player;

};
