// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "YJ_Item.h"
#include "CoreMinimal.h"
#include "YJ_WaitingTicketItem.generated.h"

UCLASS()
class HELLOFUTURE_API UYJ_WaitingTicketItem : public UYJ_Item
{
	GENERATED_BODY()
	
public:
	// 대기번호
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Item", meta = (ClampMin = 0.0))
	int32 ItemWaitingNumber;
};
