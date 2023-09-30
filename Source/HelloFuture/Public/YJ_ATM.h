// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Blueprint/UserWidget.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "YJ_ATM.generated.h"

UCLASS()
class HELLOFUTURE_API AYJ_ATM : public AActor
{
	GENERATED_BODY()
	
public:
	// 초기값 셋팅
	AYJ_ATM();
	virtual void BeginPlay() override;

	/** 오버랩 */
	// 캐릭터가 ATM기에 오버랩하면 ATM UI 띄우기
	UFUNCTION() 
		void OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	// 캐릭터의 ATM기 오버랩이 끝나면 ATM UI 숨기기
	UFUNCTION()
		void OnOverlapEnd(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	/** ATM UI */
	// ATM UI 띄우기
	UFUNCTION(BlueprintCallable)
		void ShowATMWidget();
	// ATM UI 숨기기
	UFUNCTION(BlueprintCallable)
		void HideATMWidget();

public:
	// box콜리전
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ATM")
		class UBoxComponent* BoxCollision;
	// 메시
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "ATM")
		class UStaticMeshComponent* StaticMesh;

	// ATM UI 클래스
	// @Warning 블루프린트에서 ATM UI 클래스레퍼런스를 넣어줘야 함.
	// "WidgetBlueprint'/Game/YJ/UI/ATM/WBP_ATM'"
	UPROPERTY(EditDefaultsOnly, Category = "ATM")
		TSubclassOf<UUserWidget> ATMWidgetClass;

private:
	// ATM UI 인스턴스
	UPROPERTY(VisibleAnywhere)
		UUserWidget* ATMWidget;

	// 오버랩 된 플레이어
	UPROPERTY(VisibleAnywhere)
		class AHelloFutureCharacter* OverlapPlayer;

};
