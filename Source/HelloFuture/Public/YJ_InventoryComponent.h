// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

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
	NONE = 0,
	Apple,
	ClothingHelmet,
	ClothingVest,
	CornItem,
	CornSeed,
	Radish,
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
	E_MAX,
};

// 통장
USTRUCT(Atomic, BlueprintType)
struct FBankBookStruct
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
	/** 속성 초기값 설정 */
	UYJ_InventoryComponent();
	void BeginPlay();

	/** 아이템 관련 함수 */
	// 인벤토리에 아이템 추가1
	UFUNCTION(BlueprintCallable)
		bool AddItem(class UYJ_Item* Item);
	// 인벤토리에 아이템 추가2
	UFUNCTION(BlueprintCallable)
		bool AddItem2(EItemEnum Item);
	// 인벤토리에 아이템 추가3
	//UFUNCTION(BlueprintCallable)
	//	bool AddItem3(TSubclassOf<class UYJ_Item> ItemClass, int32 Num = 1);
	// 갯수만큼 인벤토리에 아이템 추가
	UFUNCTION(BlueprintCallable)
		bool AddItemByNumber(EItemEnum Item, int32 Num);
	// 인벤토리에 아이템 제거1
	UFUNCTION(BlueprintCallable)
		bool RemoveItem(class UYJ_Item* Item);
	// 인벤토리에 아이템 제거2
	UFUNCTION(BlueprintCallable)
		bool RemoveItem2(EItemEnum Item);
	//// 인벤토리에 아이템 제거3
	//UFUNCTION(BlueprintCallable)
	//	bool RemoveItem3(TSubclassOf<class UYJ_Item> ItemClass, int32 Num = 1);
	// 갯수만큼 인벤토리에 아이템 제거
	UFUNCTION(BlueprintCallable)
		bool RemoveItemByNumber(EItemEnum Item, int32 Num);
	// 아이템 enum -> 아이템 객체로 변환
	UFUNCTION(BlueprintCallable)
		class UYJ_Item* EnumIndexToItem(EItemEnum Item);
	// 해당 아이템을 가지고 있는지 확인
	UFUNCTION(BlueprintCallable)
		bool CheckItemAsYJ_Item(class UYJ_Item* Item);
	// 아이템 enum으로 해당 아이템을 가지고 있는지 확인
	UFUNCTION(BlueprintCallable)
		bool CheckItemAsEnum(EItemEnum Item);

	/**돈 관련 함수*/
	// 현금 더하거나 빼기
	UFUNCTION(BlueprintCallable)
		bool AddCash(int32 AddPrice);
	// 계좌잔금 더하거나 빼기
	UFUNCTION(BlueprintCallable)
		bool AddAccountBalance(int32 AddPrice);

	/**통장이자, 대출금, 세금 관련 함수*/
	// 통장이자 업데이트
	UFUNCTION(BlueprintCallable)
		bool UpdateBankBookInterest();
	// 대출금 업데이트
	UFUNCTION(BlueprintCallable)
		bool UpdateLoan();
	// 갚아야 할 대출금 반환
	UFUNCTION(BlueprintCallable)
		int32 GetTotalLoanAmount();
	// 세금 업데이트
	UFUNCTION(BlueprintCallable)
		bool UpdateTax();

public:
	/** 인벤토리 칸 정보 */
	// 인벤토리 칸 용량
	UPROPERTY(EditDefaultsOnly, Category = "Inventory")
		int32 Capacity;
	// 인벤토리 가로칸 갯수
	UPROPERTY(EditDefaultsOnly, blueprintReadWrite, Category = "Inventory")
		int32 ColumnLength;
	// 인벤토리 세로칸 갯수
	UPROPERTY(EditDefaultsOnly, blueprintReadWrite, Category = "Inventory")
		int32 RowLength;

	/** 가지고 있는 것들 정보 */
	// 가지고 있는 인벤토리 아이템
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Items")
		TArray<class UYJ_Item*> Items;
	// 가지고 있는 인벤토리 아이템 갯수
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Items")
		int32 ItemCnt;
	// 계좌잔고
	UPROPERTY(EditDefaultsOnly, blueprintReadWrite, Category = "Inventory")
		int32 AccountBalance;
	// 현금잔고
	UPROPERTY(EditDefaultsOnly, blueprintReadWrite, Category = "Inventory")
		int32 Cash;

	/** 가지고 있는 것들 정보 */
	/*
	USTRUCT(Atomic, BlueprintType)
	struct ItemStruct
	{
		// 아이템 클래스
		UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BankBook")
			TSubclassOf<class UYJ_Item> ItemClass;
		// 아이템 갯수
		UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Items")
			int32 ItemNumber;
	};
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Items")
		TArray<ItemStruct> ItemStructArray;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Items")
		TMap<TSubclassOf<class UYJ_Item>, int32> ItemIndexMap;
	*/

	/** 통장, 대출, 세금 이자에 관한 정보 */
	// 통장
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "BankBook")
		FBankBookStruct BankBook;
	// 대출
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Loan")
		FLoanStruct Loan;
	// 세금
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tax")
		FTaxStruct Tax;

	/** 델리게이트 */
	// 인벤토리 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Inventory")
		FOnInventoryUpdated	OnInventoryUpdated;
	// 세금 델리게이트
	UPROPERTY(BlueprintAssignable, Category = "Tax")
		FOnTaxUpdated OnTaxUpdated;
};
