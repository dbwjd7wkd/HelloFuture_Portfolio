// Copyright Epic Games, Inc. All Rights Reserved.

#include "HelloFutureCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "YJ_Item.h"
#include "YJ_InventoryComponent.h"
#include "Minsu_Quiz.h"
#include <Components/SceneCaptureComponent2D.h>
#include <Components/SceneComponent.h>
#include "HelloFutureGameMode.h"
#include <Minsu_NameInputUserWidget.h>
#include <Internationalization/Text.h>
#include "HelloFuture.h"
#include <Logging/LogMacros.h>
#include <GameFramework/Actor.h>
#include <Blueprint/UserWidget.h>
#include "Minsu_Activate.h"
#include "Minsu_PlantSeed.h"
#include <Materials/MaterialInterface.h>
#include <Minsu_ChatWidget.h>
#include <YJ_GameInstance.h>
#include "Engine/NetConnection.h"
#include "Components/TextRenderComponent.h"
//File picker includes
#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"
#include "GenericPlatform/GenericPlatformMisc.h"
//#include "Runtime/ImageWrapper/Public/Interfaces/IImageWrapper.h"
//#include "Runtime/ImageWrapper/Public/Interfaces/IImageWrapperModule.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "RHICommandList.h"
#include "Blueprint/UserWidget.h"
#include "MyTalkWidget.h"
#include "Runtime/Engine/Public/TextureResource.h"
#include "Runtime/Engine/Public/HighResScreenshot.h"
#include "Runtime/Engine/Classes/Engine/Texture2D.h"
#include "Components/WidgetComponent.h"
#include "Components/TextBlock.h"

//////////////////////////////////////////////////////////////////////////
// AHelloFutureCharacter
// 기능: saveGame/LoadGame, 커스텀_서버, 닉네임_서버, 채팅, 나무흔들기, 옷구매, 액자이미지 파일 가져오기, input

struct FUpdateTextureData
{
	UTexture2D* Texture2D;
	FUpdateTextureRegion2D Region;
	uint32 Pitch;
	TArray64<uint8> BufferArray;
	TSharedPtr<IImageWrapper> Wrapper;	//to keep the uncompressed data alive
};

AHelloFutureCharacter::AHelloFutureCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// 인벤토리 시스템
	inventory = CreateDefaultSubobject<UYJ_InventoryComponent>(TEXT("Inventory"));

	// 채팅 시스템
	ChatText = CreateDefaultSubobject<UTextRenderComponent>("ChatText");
	ChatText->SetRelativeLocation(FVector(0, 0, 100));
	ChatText->SetHorizontalAlignment(EHTA_Center);
	ChatText->SetupAttachment(RootComponent);
	ChatText->SetVerticalAlignment(EVRTA_TextCenter);
	ChatText->SetTextRenderColor(FColor::Black);
	ChatText->SetText(TEXT(""));
	
	ConstructorHelpers::FObjectFinder<UMaterial> MatFinder(TEXT("Material'/Game/Minsu/Font/Font01/Minsu_TextMat.Minsu_TextMat'"));
	if (MatFinder.Succeeded())
	{
		ChatText->SetTextMaterial(MatFinder.Object);
	}
	
	ConstructorHelpers::FObjectFinder<UFont> FontFinder(TEXT("Font'/Game/Minsu/Font/Font03/Font03.Font03'"));
	if (FontFinder.Succeeded())
	{
		ChatText->SetFont(FontFinder.Object);
	}

	// 옷 구매 TMap
	BoughtClothes.Add("ShopClothes1", false);
	BoughtClothes.Add("ShopClothes2", false);
	BoughtClothes.Add("ShopClothes3", false);
	BoughtClothes.Add("ShopClothes4", false);
	BoughtClothes.Add("ShopClothes5", false);
	BoughtClothes.Add("ShopClothes6", false);
	BoughtClothes.Add("ShopClothes7", false);

	// 닉네임
	C_TextRenderName = CreateDefaultSubobject<UTextRenderComponent>(TEXT("C_TextRenderName"));
	C_TextRenderName->SetRelativeLocation(FVector(17, 0, 52));
	C_TextRenderName->SetHorizontalAlignment(EHTA_Center);
	C_TextRenderName->SetupAttachment(RootComponent);
	C_TextRenderName->SetVerticalAlignment(EVRTA_TextCenter);
	C_TextRenderName->SetTextRenderColor(FColor::Black);
	C_TextRenderName->SetText(TEXT(""));

	if (MatFinder.Succeeded())
	{
		C_TextRenderName->SetTextMaterial(MatFinder.Object);
	}
	if (FontFinder.Succeeded())
	{
		C_TextRenderName->SetFont(FontFinder.Object);
	}

	//닉네임 수정
	// name = CreateDefaultSubobject<UWidgetComponent>(TEXT("name"));
	// name->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepRelativeTransform);
}

void AHelloFutureCharacter::BeginPlay()
{
	Super::BeginPlay();

	CurrentMessage = "";

	APlayerController* playerController = Cast<APlayerController>(GetController());
	if (playerController) {
		playerController->bShowMouseCursor = true;
		playerController->bEnableClickEvents = true;
		playerController->bEnableMouseOverEvents = true;
	}

	// 닉네임 셋팅
	//name_TextRender->SetText(FText::FromString("Something"));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// 정보 저장 및 가져오기
void AHelloFutureCharacter::SaveGame()
{
	saveGameInstance = Cast<UYJ_SaveGame>(UGameplayStatics::CreateSaveGameObject(UYJ_SaveGame::StaticClass()));
	gameInstance = Cast<UYJ_GameInstance>(GetGameInstance());
	if (!saveGameInstance || !inventory || !gameInstance) return;

	/** 인벤토리**/
	// items 정보 저장
	saveGameInstance->itemCnt = inventory->itemCnt;

	for (int32 i = 0; i < inventory->itemCnt; i++)
	{
		int32 idx = inventory->items[i]->itemIndex;
		saveGameInstance->inventoryIdx[idx] = i;
		saveGameInstance->inventoryCnt[idx] = inventory->items[i]->count;
	}
	// 인벤토리 정보들 저장
	saveGameInstance->accountBalance = inventory->accountBalance;
	saveGameInstance->cash = inventory->cash;
	//SaveGameInstance->columnLength = Inventory->columnLength;
	//SaveGameInstance->rowLength = Inventory->rowLength;
	//SaveGameInstance->Capacity = Inventory->Capacity;

	/** 나머지 정보들 **/
	// 플레이어 이름 저장
	saveGameInstance->PlayerName = Name;
	saveGameInstance->time = time;

	// 구매한 옷
	saveGameInstance->BoughtClothes = BoughtClothes;

	//// 구매한 옷 순서대로 in 옷장
	saveGameInstance->closetBoughts = closetBoughts;

	// 은행
	saveGameInstance->BankBook = inventory->BankBook;
	saveGameInstance->Loan = inventory->Loan;
	saveGameInstance->Tax = inventory->Tax;

	// 시간
	saveGameInstance->worldTime = gameInstance->worldTime;
	saveGameInstance->worldTime_Structure = gameInstance->worldTime_Structure;

	UGameplayStatics::SaveGameToSlot(saveGameInstance, saveGameInstance->SaveSlotName, saveGameInstance->UserIndex);

	OnSaveGame.Broadcast();
}

void AHelloFutureCharacter::LoadGame()
{
	loadGameInstance = Cast<UYJ_SaveGame>(UGameplayStatics::CreateSaveGameObject(UYJ_SaveGame::StaticClass()));
	loadGameInstance = Cast<UYJ_SaveGame>(UGameplayStatics::LoadGameFromSlot(loadGameInstance->SaveSlotName, loadGameInstance->UserIndex));
	gameInstance = Cast<UYJ_GameInstance>(GetGameInstance());

	if (!loadGameInstance || !inventory || !gameInstance) return;

	// 아이템 갯수 정보 로드
	int32 itemCnt = loadGameInstance->itemCnt;
	inventory->itemCnt = itemCnt;
	inventory->items.SetNum(itemCnt);

	// 모든 아이템을 차례대로 확인해, 각 아이템들의 정보 로드
	for (int32 i = 0; i < gameInstance->allItems.Num(); i++)
	{
		gameInstance->allItems[i]->itemIndex = i;
		// 아이템 갯수가 늘어났을 경우 오류 대비
		if (loadGameInstance->inventoryCnt.Num() < i + 1)
		{
			loadGameInstance->inventoryCnt.Add(0);
			loadGameInstance->inventoryIdx.Add(-1);
		}
		// 현재 아이템을 가지고 있지 않으면 건너뛰기
		int32 cnt = loadGameInstance->inventoryCnt[i];
		if (cnt <= 0) 
			continue;

		int32 idx = loadGameInstance->inventoryIdx[i];
		// 인벤토리에 현재 아이템 객체 넣기
		inventory->items[idx] = gameInstance->allItems[i];
		// 아이템 객체에 정보 넣기
		inventory->items[idx]->count = cnt;
		inventory->items[idx]->inventoryIndex = idx;
	}

	// items 로드
	inventory->OnInventoryUpdated.Broadcast();

	// 나머지 인벤토리 정보 로드
	inventory->accountBalance = loadGameInstance->accountBalance;
	inventory->cash = loadGameInstance->cash;

	// 플레이어 이름 로드
	Name = loadGameInstance->PlayerName;
	time = loadGameInstance->time;

	//구매한 옷 로드
	BoughtClothes = loadGameInstance->BoughtClothes;
	closetBoughts = loadGameInstance->closetBoughts;

	// 은행
	inventory->BankBook = loadGameInstance->BankBook;
	inventory->Loan = loadGameInstance->Loan;
	inventory->Tax = loadGameInstance->Tax;

	// 시간
	gameInstance->worldTime = loadGameInstance->worldTime;
	gameInstance->worldTime_Structure = loadGameInstance->worldTime_Structure;

	OnLoadGame.Broadcast();

}


//////////////////////////////////////////////////////////////////////////////////////////////////////
// 커스텀_서버
void AHelloFutureCharacter::GetCustom_OnServer_Implementation(const FString& OldName)
{
	BP_GetCustom(OldName);
	GetCustom_OnClient(OldName);
}

bool AHelloFutureCharacter::GetCustom_OnServer_Validate(const FString& OldName)
{
	return true;
}

void AHelloFutureCharacter::GetCustom_OnClient_Implementation(const FString& OldName)
{
	BP_GetCustom(OldName);
}

bool AHelloFutureCharacter::GetCustom_OnClient_Validate(const FString& OldName)
{
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// 닉네임_서버
void AHelloFutureCharacter::AttemptToSendName(const FString& Message, const FString& OldName)
{
	// 만약 서버가 없다면 ServerSendName를 보내고
	// 아니라면, SendName를 이용
	if (GetLocalRole() < ROLE_Authority)
	{
		// 클라이언트일 때
		ServerSendName(Message);
		GetCustom_OnServer(OldName);
	}
	else
	{
		// 서버일 때
		SendName(Message);
		BP_GetCustom(OldName);
	}
}

void AHelloFutureCharacter::SendName(const FString& Message)
{
	CurrentName = Message;
	UpdateNameText();
	//c->CallFunctionByNameWithArguments(TEXT("Get Custom on Server"), ar, NULL, true);
}

void AHelloFutureCharacter::ServerSendName_Implementation(const FString& Message)
{
	SendName(Message);
	ClientSendName(Message);
}

bool AHelloFutureCharacter::ServerSendName_Validate(const FString& Message)
{
	if (Message.Len() < 255)
	{
		return true;
	}
	else return false;
}

void AHelloFutureCharacter::ClientSendName_Implementation(const FString& Message)
{
	SendName(Message);
}

bool AHelloFutureCharacter::ClientSendName_Validate(const FString& Message)
{
	if (Message.Len() < 255)
	{
		return true;
	}
	else return false;
}

void AHelloFutureCharacter::OnRep_CurrentName()
{
	UpdateNameText();
}

void AHelloFutureCharacter::UpdateNameText()
{
	C_TextRenderName->SetText(CurrentName);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////
// 채팅
void AHelloFutureCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHelloFutureCharacter, CurrentMessage);
	DOREPLIFETIME(AHelloFutureCharacter, CurrentName);
	//DOREPLIFETIME(AHelloFutureCharacter, Name);

}

void AHelloFutureCharacter::AttemptToSendChatMessage(const FString& Message)
{

	// 만약 서버가 없다면 ServersendChat message를 보내고
	// 아니라면, send chat message를 이용
	if (GetLocalRole() < ROLE_Authority)
	{
		ServerSendChatMessage(Message);
	}
	else
	{
		SendChatMessage(Message);
	}
}

void AHelloFutureCharacter::SendChatMessage(const FString& Message)
{
	CurrentMessage = Message;
	UpdateChatText();

	FTimerHandle DummyHandle;
	GetWorldTimerManager().SetTimer(DummyHandle, this, &AHelloFutureCharacter::ClearChatMessage, 5.f);
}

void AHelloFutureCharacter::ClearChatMessage()
{
	CurrentMessage = "";
	UpdateChatText();
}

void AHelloFutureCharacter::ServerSendChatMessage_Implementation(const FString& Message)
{
	SendChatMessage(Message);
}

bool AHelloFutureCharacter::ServerSendChatMessage_Validate(const FString& Message)
{
	if (Message.Len() < 255)
	{
		return true;
	}
	else return false;
}

void AHelloFutureCharacter::OnRep_CurrentMessage()
{
	UpdateChatText();
}

void AHelloFutureCharacter::UpdateChatText()
{
	ChatText->SetText(FText::FromString(CurrentMessage));
}


////////////////////////////////////////////////////////////////////////////////////////////
// 나무 흔들기
void AHelloFutureCharacter::KeyShakeTree()
{
	FHitResult HitResult;
	FVector CamLoc;
	FRotator CamRot;
	
	GetController()->GetPlayerViewPoint(CamLoc, CamRot);
	 
	FVector StartTrace = CamLoc;
	FVector EndTrace = StartTrace + (CamRot.Vector() * TraceDistance);
	
	FCollisionQueryParams TraceParams;
	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartTrace, EndTrace, ECC_Visibility, TraceParams);
	
	DrawDebugLine(GetWorld(), StartTrace, EndTrace, FColor::Green, false, 2.0f);
	if (bHit)
		{
			DrawDebugBox(GetWorld(), HitResult.ImpactPoint, FVector(5, 5, 5), FColor::Green, false, 2.0f);
			if (HitResult.GetActor()->GetClass()->ImplementsInterface(UMinsu_PlantSeed::StaticClass()))
				{
					IMinsu_ShakeTree::Execute_ShakeTree(HitResult.GetActor());
				}
	 
		}
}

void AHelloFutureCharacter::ShakeTree_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("ShakeTree_Activate"));
}


//////////////////////////////////////////////////////////////////////////////////
// 옷 구매 확인
bool AHelloFutureCharacter::GetBoughtClothes(FString key)
{
	bool value = false;
	bool* isKey = BoughtClothes.Find(key);
	if (isKey != nullptr)
	{
		// 유효하면 값을 돌려줌
		value = BoughtClothes[key];
	}
	return value;
}

void AHelloFutureCharacter::SetBoughtClothes(FString key, bool value)
{
	// 자동으로 오버로딩됨
	BoughtClothes.Add(key, value);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////
// 액자 이미지 FilePicker
void AHelloFutureCharacter::OpenFileDialog(int32 frameNumber, const FString& DialogTitle, const FString& DefaultPath, const FString& FileTypes, TArray<FString>& OutFileNames)
{
	if (GEngine)
	{
		if (GEngine->GameViewport)
		{
			void* ParentWindowHandle = GEngine->GameViewport->GetWindow()->GetNativeWindow()->GetOSWindowHandle();
			IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
			if (DesktopPlatform)
			{
				//Opening the file picker!
				uint32 SelectionFlag = 0; //A value of 0 represents single file selection while a value of 1 represents multiple file selection
				DesktopPlatform->OpenFileDialog(ParentWindowHandle, DialogTitle, DefaultPath, FString(""), FileTypes, SelectionFlag, OutFileNames);
			}
		}
	}
}

UTexture2D* AHelloFutureCharacter::GetFile(const FString& File, bool& IsValid, int32& Width, int32& Height)
{

	UE_LOG(LogTemp, Warning, TEXT("okay"));
	IsValid = false;

	// 텍스처를 넣을 자리 지정
	UTexture2D* Texture = NULL;

	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));

	// 이미지 파일 데이터를 받을 TArray
	TArray<uint8> ImageRawData;
	
	//EImageFormkat DetectedFormat = ImageWrapperModule.DetectImageFormat(ImageRawData.GetData(), ImageRawData.Num());

	IImageWrapperPtr ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::JPEG);

	if (!FFileHelper::LoadFileToArray(ImageRawData, *File))
	{
		UE_LOG(LogTemp, Warning, TEXT("NULLNULL"));
		return NULL;
	}

	if (ImageWrapper.IsValid() && ImageWrapper->SetCompressed(ImageRawData.GetData(), ImageRawData.Num()))
	{
		TArray<uint8> UncompressedBGRA;

		UE_LOG(LogTemp, Warning, TEXT("what?"));

		if (ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, UncompressedBGRA))
		{
			UE_LOG(LogTemp, Warning, TEXT("came here?"));
			Texture = UTexture2D::CreateTransient(ImageWrapper->GetWidth(), ImageWrapper->GetHeight(), PF_B8G8R8A8);

			if (!Texture)
			{
				UE_LOG(LogTemp, Warning, TEXT("INValidTexture"));
				return NULL;
			}

			Width = ImageWrapper->GetWidth();
			Height = ImageWrapper->GetHeight();

			void* TextureData = Texture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_WRITE);
			FMemory::Memcpy(TextureData, UncompressedBGRA.GetData(), UncompressedBGRA.Num());
			Texture->PlatformData->Mips[0].BulkData.Unlock();

			Texture->UpdateResource();
			/*IsValid = true;*/
		}
	}
	IsValid = true;
	return Texture;
}

//void AHelloFutureCharacter::PrintData(const FString& File)
//{
//	//Parse the data into a string array
//	TArray<FString> LoadedText;
//	FFileHelper::LoadFileToStringArray(LoadedText, *File);
//	//Print the contents
//	for (int32 i = 0; i < LoadedText.Num(); i++)
//	{
//		GLog->Log(LoadedText[i]);
//	}
//}


//////////////////////////////////////////////////////////////////////////
// Input
void AHelloFutureCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &AHelloFutureCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AHelloFutureCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AHelloFutureCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AHelloFutureCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &AHelloFutureCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &AHelloFutureCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AHelloFutureCharacter::OnResetVR);

	////////////// 채팅 ////////////////
	/*PlayerInputComponent->BindAction("Chatting", IE_Pressed, this, &AHelloFutureCharacter::Chatting);*/

	// save game
	PlayerInputComponent->BindAction("Save", IE_Pressed, this, &AHelloFutureCharacter::SaveGame);
	PlayerInputComponent->BindAction("Load", IE_Pressed, this, &AHelloFutureCharacter::LoadGame);
}

void AHelloFutureCharacter::CreatePlayerHUD(FText playerName)
{
	if (nameWidgetClass)
	{
		nameWidget = CreateWidget<UMinsu_NameInputUserWidget>(GetWorld(), nameWidgetClass);
		if (nameWidget)
		{
			nameWidget->AddToViewport();
		}
	}
}

void AHelloFutureCharacter::OnResetVR()
{
	// If HelloFuture is added to a project via 'Add Feature' in the Unreal Editor the dependency on HeadMountedDisplay in HelloFuture.Build.cs is not automatically propagated
	// and a linker error will result.
	// You will need to either:
	//		Add "HeadMountedDisplay" to [YourProject].Build.cs PublicDependencyModuleNames in order to build successfully (appropriate if supporting VR).
	// or:
	//		Comment or delete the call to ResetOrientationAndPosition below (appropriate if not supporting VR)
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void AHelloFutureCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
}

void AHelloFutureCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}

void AHelloFutureCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AHelloFutureCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AHelloFutureCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AHelloFutureCharacter::MoveRight(float Value)
{
	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}

void AHelloFutureCharacter::SetInteractiveInRange(class AOH_InteractiveBase* Interactive)
{
	if (Interactive != nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("WWWw"));
		currentInteractive = Interactive;
		
		
	}
}

void AHelloFutureCharacter::ClearInteractiveInRange(class AOH_InteractiveBase* Interactive)
{
	UE_LOG(LogTemp, Warning, TEXT("WWWww"));
	currentInteractive = nullptr;
	
}

////////////////////////////////////////////////
void AHelloFutureCharacter::InitializeGame()
{

}

void AHelloFutureCharacter::UseItem(class UYJ_Item* Item)
{
	if (Item)
	{
		Item->Use(this);
		Item->OnUse(this); // bp event
	}
}

void AHelloFutureCharacter::Chatting()
{
// 	if (ChatWidgetClass)
// 	{
// 		ChatWidget = CreateWidget<UMinsu_ChatWidget>(UGameplayStatics::GetPlayerController(GetWorld(), 0), ChatWidgetClass);
// 		if (ChatWidget)
// 		{
// 			ChatWidget->AddToViewport();
// 		}
// 	}
}

// void AHelloFutureCharacter::Activate_Implementation()
// {
// 	/*UE_LOG(LogTemp, Warning, TEXT("Activate"));*/
// }
// 
// void AHelloFutureCharacter::KeySeed()
// {
// // 	FHitResult HitResult;
// // 	FVector CamLoc;
// // 	FRotator CamRot;
// // 
// // 	GetController()->GetPlayerViewPoint(CamLoc, CamRot);
// // 
// // 	FVector StartTrace = CamLoc;
// // 	FVector EndTrace = StartTrace + (CamRot.Vector() * TraceDistance);
// // 
// // 	FCollisionQueryParams TraceParams;
// // 	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartTrace, EndTrace, ECC_Visibility, TraceParams);
// // 
// // 	DrawDebugLine(GetWorld(), StartTrace, EndTrace, FColor::Green, false, 2.0f);
// // 
// // 	if (bHit)
// // 	{
// // 		DrawDebugBox(GetWorld(), HitResult.ImpactPoint, FVector(5, 5, 5), FColor::Green, false, 2.0f);
// // 		
// // 		if (HitResult.GetActor()->GetClass()->ImplementsInterface(UMinsu_PlantSeed::StaticClass()))
// // 		{
// // 			IMinsu_PlantSeed::Execute_PlantSeed(HitResult.GetActor());
// // 		}
// // 
// // 	}
// }
// 
// void AHelloFutureCharacter::PlantSeed_Implementation()
// {
// 	/*UE_LOG(LogTemp, Warning, TEXT("PlantSeed"));*/
// }
// 
// void AHelloFutureCharacter::KeyActivate()
// {
// // 	FHitResult HitResult;
// // 	FVector CamLoc;
// // 	FRotator CamRot;
// // 
// // 	GetController()->GetPlayerViewPoint(CamLoc, CamRot);
// // 
// // 	FVector StartTrace = CamLoc;
// // 	FVector EndTrace = StartTrace + (CamRot.Vector() * TraceDistance);
// // 
// // 	FCollisionQueryParams TraceParams;
// // 	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, StartTrace, EndTrace, ECC_Visibility, TraceParams);
// // 
// // 	DrawDebugLine(GetWorld(), StartTrace, EndTrace, FColor::Red, false, 2.0f);
// // 
// // 	if (bHit)
// // 	{
// // 		DrawDebugBox(GetWorld(), HitResult.ImpactPoint, FVector(5, 5, 5), FColor::Red, false, 2.0f);
// // 		
// // 		if (HitResult.GetActor()->GetClass()->ImplementsInterface(UMinsu_Activate::StaticClass()))
// // 		{
// // 			IMinsu_Activate::Execute_Activate(HitResult.GetActor());
// // 		}
// // 	}
// 	
// }
// 
// void AHelloFutureCharacter::UpSeed()
// {
// 	/*Seed = FMath::Clamp(Seed + 1, 0, 2);*/
// }
// 
// void AHelloFutureCharacter::DownSeed()
// {
// 	/*Seed = FMath::Clamp(Seed - 1, 0, 2);*/
// }

