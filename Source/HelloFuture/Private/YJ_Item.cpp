// Fill out your copyright notice in the Description page of Project Settings.


#include "YJ_Item.h"

UYJ_Item::UYJ_Item()
{
	ItemDisplayName = FText::FromString("Item");
	Category = FText::FromString("Use");

	count = 0;
	inventoryIndex = -1;
	itemIndex = -1;
}

void UYJ_Item::Use(class AHelloFutureCharacter* Character)
{

}
