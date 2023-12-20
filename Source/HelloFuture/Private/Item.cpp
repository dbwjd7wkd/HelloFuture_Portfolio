// Fill out your copyright notice in the Description page of Project Settings.


#include "Item.h"
#include "Misc/OutputDeviceNull.h"
#include "UObject/ConstructorHelpers.h"
#include "HelloFutureCharacter.h"
#include "Components/CapsuleComponent.h"
#include "Engine/Classes/Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Sound/SoundCue.h"


// Sets default values
AItem::AItem()
{
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	// 캡슐콜리전 컴포넌트 생성 및 셋팅
	Capsule = CreateDefaultSubobject<UCapsuleComponent>(TEXT("Capsule"));
	Capsule->SetupAttachment(GetRootComponent());
	Capsule->SetCollisionProfileName(TEXT("Item"));
	//CapsuleCollision->InitCapsuleSize(42.f, 96.0f);
	//CapsuleCollision->SetCollisionProfileName(TEXT("YJ_HelloFutureCharacter_Capsule"));
	Capsule->OnComponentBeginOverlap.AddDynamic(this, &AItem::OnComponentBeginOverlap);
	Capsule->OnComponentEndOverlap.AddDynamic(this, &AItem::OnComponentEndOverlap);

	// 메시 컴포넌트 생성 및 셋팅
	StaticMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	StaticMesh->SetupAttachment(GetRootComponent());
	StaticMesh->SetCollisionProfileName(TEXT("NoCollision"));



	//static ConstructorHelpers::FObjectFinder<UStaticMesh> StaticMeshAsset(TEXT("/Game/StarterContent/Shapes/Shape_Sphere.Shape_Sphere"));
	//if (StaticMeshAsset.Succeeded())
	//{
	//	StaticMesh->SetStaticMesh(StaticMeshAsset.Object);
	//	StaticMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -40.0f));
	//	StaticMesh->SetWorldScale3D(FVector(0.8f));
	//}
	//StaticMesh->SetCollisionProfileName(TEXT("YJ_HelloFutureCharacter_Mesh"));

	// 애니메이션 몽타주 가져오기
	//static auto AnimBlueprintName = TEXT("AnimBlueprint'/Game/Map/Animation/REALMain_ABP.REALMain_ABP_C'");
	//static auto AnimBlueprintFinder = ConstructorHelpers::FClassFinder<UAnimInstance>(AnimBlueprintName);
	//if (AnimBlueprintFinder.Succeeded())
	//{
	//	StaticMesh->SetAnimInstanceClass(AnimBlueprintFinder.Class);
	//}
}

void AItem::OnConstruction(const FTransform& Transform)
{
	StaticMesh->SetStaticMesh(NewMesh);
}

void AItem::Construct()
{
	//StaticMesh->SetStaticMesh(NewMesh);
}

// Called when the game starts or when spawned
void AItem::BeginPlay()
{
	Super::BeginPlay();
	
}

void AItem::OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bfromSweep, const FHitResult& SweepResult)
{
	// 부딪힌 게 플레이어이면, 
	// 0. 플레이어 저장하기
	Player = Cast<AHelloFutureCharacter>(OtherActor);
	if (!Player)
	{
		Player = nullptr;
		return;
	}

	Player->OnPickUpItemDelegate.BindUObject(this, &AItem::PickUpItem);

}

void AItem::OnComponentEndOverlap(UPrimitiveComponent* overlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	// 나가는 게 플레이어라면, 
	// 플레이어 nullptr로 만들기.
	Player = Cast<AHelloFutureCharacter>(OtherActor);
	if (Player)
	{
		Player->OnPickUpItemDelegate.Unbind();
		Player = nullptr;
	}

}

void AItem::PickUpItem()
{
	if (!Player) return;

	// 1. 아이템 줍는 애니메이션 + 사운드 재생. => 캐릭터의 블루프린트 함수 부르기
	if (PickingUpMontage)
	{
		FOutputDeviceNull AR;
		const FString command = FString::Printf(TEXT("GrabItemOnServer %s %s"), *PickingUpMontage->GetName(), *this->GetName());
		Player->CallFunctionByNameWithArguments(*command, AR, NULL, true);
	}

	// 2. 인벤토리에 주운 아이템 넣기.
	Player->Inventory->AddItem2(EItemEnum::Apple);

	// 3. 현재 아이템 객체 없애기.
	this->Destroy();
}

