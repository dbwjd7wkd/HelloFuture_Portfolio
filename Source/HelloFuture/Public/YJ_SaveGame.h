// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Engine/SkeletalMesh.h"
#include "Materials/MaterialInterface.h"
#include "YJ_InventoryComponent.h"
#include "YJ_SaveGame.generated.h"

// 옷 카테고리 enum
UENUM(BlueprintType)
enum class EClosetBoughtCategory : uint8
{
    Category1 UMETA(DisplayName = "None"),
    Category2 UMETA(DisplayName = "Head"),
    Category3 UMETA(DisplayName = "HairStyle"),
    Category4 UMETA(DisplayName = "Nose"),
    Category5 UMETA(DisplayName = "UpperClothes"),
    Category6 UMETA(DisplayName = "UnderClothes"),
    Category7 UMETA(DisplayName = "WholeClothes"),
    Category8 UMETA(DisplayName = "Glasses"),
    Category9 UMETA(DisplayName = "Hat"),
    Category10 UMETA(DisplayName = "Bag"),
    Category11 UMETA(DisplayName = "Watch"),
    Category12 UMETA(DisplayName = "HatHair"),
    Category13 UMETA(DisplayName = "Hair_Acc_R"),
    Category14 UMETA(DisplayName = "Hair_Acc_BB"),
};

// 구매한 옷 구조체
USTRUCT(Atomic, BlueprintType)
struct FcloseBoughtMStruct
{
    GENERATED_BODY()

public:
    // 옷 스켈레탈 메시
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ClosetBought")
        USkeletalMesh* ClosetBoughtMesh;
    // 옷 머티리얼
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ClosetBought")
        TArray<UMaterialInterface*> ClosetBoughtMaterial;
    // 옷 카테고리
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ClosetBought")
       EClosetBoughtCategory ClosetBoughtCategory;
};

UCLASS()
class HELLOFUTURE_API UYJ_SaveGame : public USaveGame
{
	GENERATED_BODY()

public:
    UYJ_SaveGame();

    /** save slot 정보*/
    // save slot 이름
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Basic")
        FString SaveSlotName;
    // save slot 인덱스
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Basic")
        int32 UserIndex;

    /** 플레이어 정보 */
    // 플레이어 이름
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Basic")
        FText PlayerName;
    // 플레이어가 게임 내에서 플레이 한 시간(단위: 일)
    UPROPERTY(VisibleAnywhere, Category = "Basic")
        float Time;

    /** 옷 정보 */
    // 구매한 옷
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "BoughtClothes")
        TMap<FString, bool> BoughtClothes;
    // 구매한 옷 순서대로 in 옷장(Material array)-외부 배열
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ClosetBought")
        TArray<FcloseBoughtMStruct> ClosetBoughts;

    /** 인벤토리 정보 */
    // 통장잔고
    UPROPERTY(VisibleAnywhere, Category = "Inventory")
        int32 AccountBalance;
    // 현금
    UPROPERTY(VisibleAnywhere, Category = "Inventory")
        int32 Cash;
    //UPROPERTY(VisibleAnywhere, Category = "Inventory")
    //    int32 columnLength;
    //UPROPERTY(VisibleAnywhere, Category = "Inventory")
    //    int32 rowLength;
    //UPROPERTY(VisibleAnywhere, Category = "Inventory")
    //    int32 Capacity;

    /** 인벤토리 아이템 정보 */
    // 인벤토리 내 가진 아이템 갯수
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Inventory")
        int32 ItemCnt;
    // 인벤토리의 각 아이템 인덱스
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Inventory")
        TArray<int32> InventoryIdxArray;
    // 인벤토리의 각 아이템 갯수
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Inventory")
        TArray<int32> InventoryCntArray;

    //UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Inventory")
    //    TArray<TSubclassOf<class UYJ_Item>> Items;
    // 삭제 예정
    //UPROPERTY(VisibleAnywhere, Category = "Inventory")
    //    TArray<class UYJ_Item*> Items;

    /** 신용 관련 정보 */
    // 통장이자
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BankBook")
        FBankBookStruct BankBook;
    // 대출
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Loan")
        FLoanStruct Loan;
    // 세금
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tax")
        FTaxStruct Tax;

    /** 퀘스트 정보 */
    // 퀘스트
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Quest")
    UActorComponent* SaveQuest;

    /** 흐른 시간 정보 */
	// 시간
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
        int32 WorldTime;
    // 구체적인 시간 구조체
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
		FDateTime WorldTimeStructure;
};
