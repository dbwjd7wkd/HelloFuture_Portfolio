// Fill out your copyright notice in the Description page of Project Settings.


#include "YJ_GameModeBase.h"
#include "HelloFutureCharacter.h"
#include "YJ_InventoryComponent.h"
#include "YJ_WaitingTicketItem.h"

void AYJ_GameModeBase::AddWaitingNumber()
{
	WaitingNumber++;
}

void AYJ_GameModeBase::MinusWaitingNumber()
{
	WaitingNumber--;
}

bool AYJ_GameModeBase::CheckMyOrder(AHelloFutureCharacter* Player)
{
	// 1. 들고있는 대기표 아이템 번호가 지금 게임모드의 순서 order와 맞는지 확인
	if (Player)
	{
		for (auto Item : Player->inventory->Items)
		{
			// 2. 만약 대기 아이템이 있다면
			UYJ_WaitingTicketItem* WaitingTicketItem = Cast<UYJ_WaitingTicketItem>(Item);
			if (WaitingTicketItem)
			{
				// 만약 대기 아이템 번호가 번호판 순서와 같다면 true 반환 (대화시작)
				if (WaitingTicketItem->ItemWaitingNumber == WaitingNumber) return true;
				// 순서가 다르다면 false 반환
				else return false;
			}
		}

		// 3. 만약 대기 아이템이 없다면 false 반환
		return false;
	}

	return false;
}
