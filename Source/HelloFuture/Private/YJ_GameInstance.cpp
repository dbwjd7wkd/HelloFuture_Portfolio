// Fill out your copyright notice in the Description page of Project Settings.


#include "YJ_GameInstance.h"
#include "YJ_Item.h"
#include "SocketSubsystem.h"

UYJ_GameInstance::UYJ_GameInstance()
{
    // ClothingShopMap에서 구매한 옷을 배치할 위치 백터 배열에 추가
    int32 Length = 13;
    for (int32 i = 0; i < Length; i++)
    {
        int32 X = 100;
        int32 Y = -750 + 100 * i;
        int32 Z = 100;
        closetSpawnLocation.Add(FVector(X, Y, Z));
    }
    
}

UYJ_Item* UYJ_GameInstance::GetItemAsEnum(EItemEnum ItemEnum)
{
    // 게임인스턴스 가져오기
	UWorld* World = GetWorld();
	if (!World) return false;
	UYJ_GameInstance* GameInstance = Cast<UYJ_GameInstance>(World->GetGameInstance());
	if (!GameInstance) return false;

    // 아이템 배열에서 enum을 인덱스로 선택해서 가져오기
	int32 ItemIdx = (int32)ItemEnum;
	UYJ_Item* Item = GameInstance->allItems[ItemIdx];

    return Item;
}

FString UYJ_GameInstance::GetMyIpAddress()
{
    // 로컬주소 string으로 반환
    FString IpAddr("NONE");
    bool bCanBind = false;
    TSharedRef<FInternetAddr> LocalIp = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->GetLocalHostAddr(*GLog, bCanBind);
    if (LocalIp->IsValid())
    {
        IpAddr = LocalIp->ToString(false);
    }
    return IpAddr;
}
