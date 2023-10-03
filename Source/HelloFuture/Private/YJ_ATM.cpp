// Fill out your copyright notice in the Description page of Project Settings.

#include "YJ_ATM.h"
#include "Blueprint/UserWidget.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "HelloFutureCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"
#include "YJ_InventoryComponent.h"

AYJ_ATM::AYJ_ATM()
{
	// 콜리전 셋팅
	BoxCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("BoxCollision"));
	SetRootComponent(BoxCollision);
	BoxCollision->InitBoxExtent(FVector(120.f, 120.f, 70.f));
	// 메시 셋팅
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMesh->SetupAttachment(BoxCollision);
	// ATM UI 클래스 가져오기
	if (ATMWidgetClass == nullptr)
	{
		static ConstructorHelpers::FClassFinder<UUserWidget> ATMWidgetClassFinder(TEXT("WidgetBlueprint'/Game/YJ/UI/ATM/WBP_ATM.WBP_ATM_C'"));
		if (ATMWidgetClassFinder.Succeeded())
		{
			ATMWidgetClass = ATMWidgetClassFinder.Class;
		}
	}

}

void AYJ_ATM::BeginPlay()
{
	Super::BeginPlay();

	// overlap 바인딩
	BoxCollision->OnComponentBeginOverlap.AddDynamic(this, &AYJ_ATM::OnOverlapBegin);
	BoxCollision->OnComponentEndOverlap.AddDynamic(this, &AYJ_ATM::OnOverlapEnd);

}

void AYJ_ATM::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	OverlapPlayer = Cast<AHelloFutureCharacter>(OtherActor);
	if (OverlapPlayer)
	{ 
		// ATM UI 띄우기
		ShowATMWidget();
	}

}

void AYJ_ATM::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	OverlapPlayer = Cast<AHelloFutureCharacter>(OtherActor);
	if (OverlapPlayer)
	{
		// ATM UI 숨기기
		HideATMWidget();
	}

}

void AYJ_ATM::ShowATMWidget()
{
	if (ATMWidgetClass == nullptr) return;

	// ATM UI 인스턴스가 없으면, 생성 및 뷰포트에 띄우기
	if (ATMWidget == nullptr)
	{
		ATMWidget = CreateWidget<UUserWidget>(GetWorld(), ATMWidgetClass);
	}
	ATMWidget->AddToViewport();

}

void AYJ_ATM::HideATMWidget()
{
	if (ATMWidget == nullptr) return;

	ATMWidget->RemoveFromViewport();

}
