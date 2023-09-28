// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "YJ_Item.h"
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "YJ_InventoryComponent.generated.h"

// UI를 업데이트 하기위해, 블루프린트에서 여기에 바인딩 할 것임.
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInventoryUpdated);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnTaxUpdated);

// 아이템 객체를 가져오기 위한 인덱스로 사용될 enum
UENUM(BlueprintType)
enum class EItemEnum : uint8
{
	Apple = 0,
	ClothingHelmet,
	ClothingVest,
	CornItem,
	CornSeed,
	Radish,
	Bread,
	Lemon,
	Pumpkin,
	PumpkinSeed,
	WaterMelon,
	Wheat,
	WheatSeed,
	RadishSeed,
	WaitingTicket,
	Fertilizer,
	Fertilizer2,
	MagicPowder,
	Tonic,
};

// 통장이자
USTRUCT(Atomic, BlueprintType)
struct FBankBookInterestStruct
{
	GENERATED_BODY()

	// 통장을 가지고 있는지
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BankBook")
		bool bHaveBankBook = false;
	// 통장 이자
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BankBook")
		int32 BankBookInterest = 0;
	// 통장 이자 준 후 지난 날짜
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BankBook")
		int32 BankBookPastDate = 0;
	// 통장 이자율
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BankBook")
		float BankBookPercent = 0.01;
};

// 대출
USTRUCT(Atomic, BlueprintType)
struct FLoanStruct
{
	GENERATED_BODY()

	// 대출을 했었는지
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Loan")
		bool bHaveBorrowedLoan = false;
	// 아직 안 갚은 대출금이 있는지
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Loan")
		bool bUnpaidLoan = false;
	// 대출 이자
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Loan")
		int32 LoanInterest = 0;
	// 대출 후 지난 날짜
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Loan")
		int32 LoanPastDate = 0;
	// 대출 이자율
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Loan")
		float LoanPercent = 0.05;
	// 원래 대출금
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Loan")
		int32 LoanValue = 100000;
};

// 세금
USTRUCT(Atomic, BlueprintType)
struct FTaxStruct
{
	GENERATED_BODY()

	// 아직 안 갚은 세금이 있는지
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tax")
		bool bUnpaidTax = false;
	// 세금 이자
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tax")
		int32 TaxInterest;
	// 세금 고지 후 지난 날짜
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tax")
		int32 TaxPastDate = 0;
	// 세금 비율
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tax")
		float TaxPercent;
	// 세금고지서 번호
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tax")
		int32 TaxContent = 0;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class HELLOFUTURE_API UYJ_InventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// 속성들 초기값 성정
	UYJ_InventoryComponent();

	virtual void BeginPlay() override;
	// 아이템 관련 함수
	UFUNCTION(BlueprintCallable)
		bool AddItem(UYJ_Item* Item);
	UFUNCTION(BlueprintCallable)
		bool AddItem2(EItemEnum Item);
	UFUNCTION(BlueprintCallable)
		bool AddItem3(TSubclassOf<UYJ_Item> ItemClass);
	UFUNCTION(BlueprintCallable)
		bool AddItemByNumber(EItemEnum Item, int32 Num);

	UFUNCTION(BlueprintCallable)
		bool RemoveItem(UYJ_Item* Item);
	UFUNCTION(BlueprintCallable)
		bool RemoveItem2(EItemEnum Item);
	UFUNCTION(BlueprintCallable)
		bool RemoveItem3(TSubclassOf<UYJ_Item> ItemClass);
	UFUNCTION(BlueprintCallable)
		bool RemoveItemByNumber(EItemEnum Item, int32 Num);

	UFUNCTION(BlueprintCallable)
		UYJ_Item* EnumIndexToItem(EItemEnum Item);
	UFUNCTION(BlueprintCallable)
		bool CheckItemAsYJ_Item(UYJ_Item* Item);
	UFUNCTION(BlueprintCallable)
		bool CheckItemAsEnum(EItemEnum Item);

	// 돈 관련 함수
	UFUNCTION(BlueprintCallable)
		bool AddCash(int32 AddPrice);
	UFUNCTION(BlueprintCallable)
		bool AddAccountBalance(int32 AddPrice);

	// 통장이자 관련 함수
	UFUNCTION(BlueprintCallable)
		bool UpdateBankBookInterest();
	// 대출 관련 함수
	UFUNCTION(BlueprintCallable)
		bool UpdateLoan();
	UFUNCTION(BlueprintCallable)
		int32 GetTotalLoanAmount();
	// 세금 관련 함수
	UFUNCTION(BlueprintCallable)
		bool UpdateTax();

public:
	UPROPERTY(EditDefaultsOnly, Instanced)
		TArray<UYJ_Item*> DefaultItems;
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
		int32 Capacity;
	UPROPERTY(EditDefaultsOnly, blueprintReadWrite, Category = "Inventory")
		int32 ColumnLength;
	UPROPERTY(EditDefaultsOnly, blueprintReadWrite, Category = "Inventory")
		int32 RowLength;
	UPROPERTY(EditDefaultsOnly, blueprintReadWrite, Category = "Inventory")
		int32 AccountBalance;
	UPROPERTY(EditDefaultsOnly, blueprintReadWrite, Category = "Inventory")
		int32 Cash;

	UPROPERTY(BlueprintAssignable, Category = "Inventory")
		FOnInventoryUpdated	OnInventoryUpdated;
	UPROPERTY(BlueprintAssignable, Category = "Tax")
		FOnTaxUpdated OnTaxUpdated;

	UPROPERTY(EditDefaultsOnly, blueprintReadWrite, Category = "Inventory")
		FName State; // add, remove

	// 인벤토리 아이템
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Items")
		TArray<UYJ_Item*> Items;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Items")
		int32 ItemCnt;

	// 통장이자
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BankBook")
		FBankBookInterestStruct BankBook;
	// 대출
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Loan")
		FLoanStruct Loan;
	// 세금
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tax")
		FTaxStruct Tax;

	//// SaveGame에 넣을 인벤토리 아이템 정보
	//UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Inventory")
	//	TArray<int32> inventoryCnt; // 인벤토리의 각 아이템 갯수

	//UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Inventory")
	//	TArray<int32> inventoryIdx; // 인벤토리의 각 아이템 인덱스
};
