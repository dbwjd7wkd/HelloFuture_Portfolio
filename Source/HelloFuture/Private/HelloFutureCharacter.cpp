// Copyright Epic Games, Inc. All Rights Reserved.

/** AHelloFutureCharacter
* 기능: saveGame/LoadGame, 커스텀_서버, 닉네임_서버, 채팅, 나무흔들기, 옷구매, 액자이미지 파일 가져오기, input
*/
#include "HelloFutureCharacter.h"
#include "Blueprint/UserWidget.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/SceneComponent.h"
#include "Components/TextBlock.h"
#include "Components/TextRenderComponent.h"
#include "Components/WidgetComponent.h"
#include "Developer/DesktopPlatform/Public/DesktopPlatformModule.h"
#include "Developer/DesktopPlatform/Public/IDesktopPlatform.h"
#include "Engine/NetConnection.h"
#include "GameFramework/Actor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "HelloFuture.h"
#include "HelloFutureGameMode.h"
#include "IImageWrapper.h"
#include "IImageWrapperModule.h"
#include "Internationalization/Text.h"
#include "Logging/LogMacros.h"
#include "Materials/MaterialInterface.h"
#include "Minsu_Activate.h"
#include "Minsu_ChatWidget.h"
#include "Minsu_NameInputUserWidget.h"
#include "Minsu_PlantSeed.h"
#include "Minsu_Quiz.h"
#include "MyTalkWidget.h"
#include "RHICommandList.h"
#include "Runtime/Engine/Classes/Engine/Texture2D.h"
#include "Runtime/Engine/Public/HighResScreenshot.h"
#include "Runtime/Engine/Public/TextureResource.h"
#include "YJ_GameInstance.h"
#include "YJ_InventoryComponent.h"
#include "YJ_Item.h"
#include "YJ_SaveGame.h" // save game

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
	// 캡슐콜리전 컴포넌트 생성 및 셋팅
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("YJ_HelloFutureCharacter_Capsule"));

	// 메시 컴포넌트 생성 및 셋팅
	USkeletalMeshComponent* MeshComponenet = GetMesh();
	MeshComponenet->SetCollisionProfileName(TEXT("YJ_HelloFutureCharacter_Mesh"));

	static auto SkeletalMeshName = TEXT("SkeletalMesh'/Game/GH/Assets/RealMeshs/Body/T-Poser.T-Poser'");
	static auto SkeletalMeshFinder = ConstructorHelpers::FObjectFinder<USkeletalMesh>(SkeletalMeshName);
	if (SkeletalMeshFinder.Succeeded())
	{
		MeshComponenet->SetSkeletalMesh(SkeletalMeshFinder.Object);
	}

	static auto AnimBlueprintName = TEXT("AnimBlueprint'/Game/Map/Animation/REALMain_ABP.REALMain_ABP_C'");
	static auto AnimBlueprintFinder = ConstructorHelpers::FClassFinder<UAnimInstance>(AnimBlueprintName);
	if (AnimBlueprintFinder.Succeeded())
	{
		GetMesh()->SetAnimInstanceClass(AnimBlueprintFinder.Class);
	}
	
	// camera boom 생성 및 셋팅 (콜리전에 부딪히면 플레이어쪽으로 카메라 당겨짐)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 667.921143f; // 카메라가 이 거리만큼 뒤에서 캐릭터를 따라다님.	
	CameraBoom->bUsePawnControlRotation = true; // 컨트롤러 기준으로 암을 회전시킴.
	CameraBoom->ProbeSize = 1.0f;

	// follow camera 생성 및 셋팅
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // 붐 끝에 카메라를 부착하고 붐이 컨트롤러 방향에 맞게 조정되도록 함.
	FollowCamera->bUsePawnControlRotation = false; // 카메라가 암에 대해 회전하지 않음.

	// turn rates for input 셋팅
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// 컨트롤로 회전 셋팅. 컨트롤러 회전할 때 회전하지 않기
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// character movement 셋팅
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// 인벤토리 시스템 생성
	Inventory = CreateDefaultSubobject<UYJ_InventoryComponent>(TEXT("Inventory"));

	// 채팅 시스템 생성 및 셋팅
	ChatText = CreateDefaultSubobject<UTextRenderComponent>("ChatText");
	ChatText->SetRelativeLocation(FVector(0, 0, 100));
	ChatText->SetHorizontalAlignment(EHTA_Center);
	ChatText->SetupAttachment(RootComponent);
	ChatText->SetVerticalAlignment(EVRTA_TextCenter);
	ChatText->SetTextRenderColor(FColor::Black);
	ChatText->SetText(FText::FromString(TEXT("")));
	
	ConstructorHelpers::FObjectFinder<UMaterial> MatFinder(TEXT("Material'/Game/Minsu/Font/Font03/Minsu_TextMat03.Minsu_TextMat03'"));
	if (MatFinder.Succeeded())
	{
		ChatText->SetTextMaterial(MatFinder.Object);
	}
	
	ConstructorHelpers::FObjectFinder<UFont> FontFinder(TEXT("Font'/Game/Minsu/Font/Font03/Font03.Font03'"));
	if (FontFinder.Succeeded())
	{
		ChatText->SetFont(FontFinder.Object);
	}

	// 옷 구매 TMap 셋팅
	BoughtClothes.Add("ShopClothes1", false);
	BoughtClothes.Add("ShopClothes2", false);
	BoughtClothes.Add("ShopClothes3", false);
	BoughtClothes.Add("ShopClothes4", false);
	BoughtClothes.Add("ShopClothes5", false);
	BoughtClothes.Add("ShopClothes6", false);
	BoughtClothes.Add("ShopClothes7", false);

	// 닉네임 생성 및 셋팅
	NameTextRender = CreateDefaultSubobject<UTextRenderComponent>(TEXT("NameTextRender"));
	NameTextRender->SetupAttachment(RootComponent);
	NameTextRender->SetRelativeLocation(FVector(0.0f, 0.0f, 73.834061f));
	NameTextRender->SetText(FText::FromString(TEXT("헬로퓨쳐")));
	NameTextRender->SetHorizontalAlignment(EHTA_Center);
	NameTextRender->SetVerticalAlignment(EVRTA_TextCenter);
	NameTextRender->SetTextRenderColor(FColor::White);
	NameTextRender->SetWorldSize(36.0f);

	if (MatFinder.Succeeded())
	{
		NameTextRender->SetTextMaterial(MatFinder.Object);
	}
	if (FontFinder.Succeeded())
	{
		NameTextRender->SetFont(FontFinder.Object);
	}

	bReplicates = true;

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

void AHelloFutureCharacter::PickUpItem()
{
	OnPickUpItemDelegate.ExecuteIfBound();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
// 정보 저장 및 가져오기
void AHelloFutureCharacter::SaveGame()
{
	SaveGameInstance = Cast<UYJ_SaveGame>(UGameplayStatics::CreateSaveGameObject(UYJ_SaveGame::StaticClass()));
	GameInstance = Cast<UYJ_GameInstance>(GetGameInstance());
	if (!SaveGameInstance || !Inventory || !GameInstance) return;

	/** 인벤토리**/
	// items 정보 저장
	SaveGameInstance->ItemCnt = Inventory->ItemCnt;

	for (int32 i = 0; i < Inventory->ItemCnt; i++)
	{
		int32 idx = Inventory->Items[i]->ItemIndex;
		SaveGameInstance->InventoryIdxArray[idx] = i;
		SaveGameInstance->InventoryCntArray[idx] = Inventory->Items[i]->Count;
	}
	// 인벤토리 정보들 저장
	SaveGameInstance->AccountBalance = Inventory->AccountBalance;
	SaveGameInstance->Cash = Inventory->Cash;
	//SaveGameInstance->columnLength = Inventory->columnLength;
	//SaveGameInstance->rowLength = Inventory->rowLength;
	//SaveGameInstance->Capacity = Inventory->Capacity;

	/** 나머지 정보들 **/
	// 플레이어 이름 저장
	SaveGameInstance->PlayerName = Name;
	SaveGameInstance->Time = Time;

	// 구매한 옷
	SaveGameInstance->BoughtClothes = BoughtClothes;

	//// 구매한 옷 순서대로 in 옷장
	SaveGameInstance->ClosetBoughts = ClosetBoughts;

	// 은행
	SaveGameInstance->BankBook = Inventory->BankBook;
	SaveGameInstance->Loan = Inventory->Loan;
	SaveGameInstance->Tax = Inventory->Tax;

	// 시간
	SaveGameInstance->WorldTime = GameInstance->WorldTime;
	SaveGameInstance->WorldTimeStructure = GameInstance->WorldTimeStructure;

	UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveGameInstance->SaveSlotName, SaveGameInstance->UserIndex);

	OnSaveGame.Broadcast();
}

void AHelloFutureCharacter::LoadGame()
{
	LoadGameInstance = Cast<UYJ_SaveGame>(UGameplayStatics::CreateSaveGameObject(UYJ_SaveGame::StaticClass()));
	LoadGameInstance = Cast<UYJ_SaveGame>(UGameplayStatics::LoadGameFromSlot(LoadGameInstance->SaveSlotName, LoadGameInstance->UserIndex));
	GameInstance = Cast<UYJ_GameInstance>(GetGameInstance());

	if (!LoadGameInstance || !Inventory || !GameInstance) return;

	// 아이템 갯수 정보 로드
	Inventory->ItemCnt = LoadGameInstance->ItemCnt;
	Inventory->Items.SetNum(LoadGameInstance->ItemCnt);

	// 모든 아이템을 차례대로 확인해, 각 아이템들의 정보 로드
	for (int32 i = 0; i < GameInstance->AllItems.Num(); i++)
	{
		GameInstance->AllItems[i]->ItemIndex = i;
		// 아이템 갯수가 늘어났을 경우 오류 대비
		if (LoadGameInstance->InventoryCntArray.Num() < i + 1)
		{
			LoadGameInstance->InventoryCntArray.Add(0);
			LoadGameInstance->InventoryIdxArray.Add(-1);
		}
		// 현재 아이템을 가지고 있지 않으면 건너뛰기
		int32 cnt = LoadGameInstance->InventoryCntArray[i];
		if (cnt <= 0) 
			continue;

		int32 idx = LoadGameInstance->InventoryIdxArray[i];
		// 인벤토리에 현재 아이템 객체 넣기
		Inventory->Items[idx] = GameInstance->AllItems[i];
		// 아이템 객체에 정보 넣기
		Inventory->Items[idx]->Count = cnt;
		Inventory->Items[idx]->InventoryIndex = idx;
	}

	// items 로드
	Inventory->OnInventoryUpdated.Broadcast();

	// 나머지 인벤토리 정보 로드
	Inventory->AccountBalance = LoadGameInstance->AccountBalance;
	Inventory->Cash = LoadGameInstance->Cash;

	// 플레이어 이름 로드
	AttempToSetName(LoadGameInstance->PlayerName);
	Time = LoadGameInstance->Time;

	//구매한 옷 로드
	BoughtClothes = LoadGameInstance->BoughtClothes;
	ClosetBoughts = LoadGameInstance->ClosetBoughts;

	// 은행
	Inventory->BankBook = LoadGameInstance->BankBook;
	Inventory->Loan = LoadGameInstance->Loan;
	Inventory->Tax = LoadGameInstance->Tax;

	// 시간
	GameInstance->WorldTime = LoadGameInstance->WorldTime;
	GameInstance->WorldTimeStructure = LoadGameInstance->WorldTimeStructure;

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
void AHelloFutureCharacter::AttempToSetName(const FText& PlayerName)
{
	// 캐릭터가 로컬에서 컨트롤 되고 있는지
	if (IsLocallyControlled())
	{
		// 서버인지
		if(HasAuthority())
		{
			SetName(PlayerName);
		}
		else
		{
			ServerSetName(PlayerName);
		}
	}
}

bool AHelloFutureCharacter::ServerSetName_Validate(const FText& PlayerName)
{
	if (PlayerName.ToString().Len() < 255)
	{
		return true;
	}
	else return false;
}

void AHelloFutureCharacter::ServerSetName_Implementation
(const FText& PlayerName)
{
	SetName(PlayerName);
}

void AHelloFutureCharacter::SetName(const FText& PlayerName)
{
	Name = PlayerName;
	OnRep_Name();
}

void AHelloFutureCharacter::OnRep_Name()
{
	UpdateNameTextRender();
}

void AHelloFutureCharacter::UpdateNameTextRender()
{
	NameTextRender->SetText(Name);
}

void AHelloFutureCharacter::PrintDebug(const FString& str)
{
	//UE_LOG(LogTemp, Warning, TEXT("Name: %s"), *Name.ToString());
	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, FString::Printf(TEXT("%s: CurName %s"), *str, *Name.ToString()));
	//if(GetNetOwningPlayer())
	//	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, FString::Printf(TEXT("%s"), *GetNetOwningPlayer()->GetFName().ToString()));
	//else
	//	GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, FString::Printf(TEXT("Non-Owning Player!")));
	if (IsLocallyControlled())
		GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Blue, FString::Printf(TEXT("be locally controlled!")));
}

void AHelloFutureCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AHelloFutureCharacter, CurrentMessage);
	DOREPLIFETIME(AHelloFutureCharacter, Name);
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
	//UE_LOG(LogTemp, Warning, TEXT("ShakeTree_Activate"));
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
	IsValid = false;

	// 텍스처를 넣을 자리 지정
	UTexture2D* Texture = NULL;

	IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));

	// 이미지 파일 데이터를 받을 TArray
	TArray<uint8> ImageRawData;
	
	//EImageFormkat DetectedFormat = ImageWrapperModule.DetectImageFormat(ImageRawData.GetData(), ImageRawData.Num());

	TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(EImageFormat::JPEG);

	if (!FFileHelper::LoadFileToArray(ImageRawData, *File))
	{
		return NULL;
	}

	if (ImageWrapper.IsValid() && ImageWrapper->SetCompressed(ImageRawData.GetData(), ImageRawData.Num()))
	{
		TArray<uint8> UncompressedBGRA;

		if (ImageWrapper->GetRaw(ERGBFormat::BGRA, 8, UncompressedBGRA))
		{
			Texture = UTexture2D::CreateTransient(ImageWrapper->GetWidth(), ImageWrapper->GetHeight(), PF_B8G8R8A8);

			if (!Texture)
			{
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

	// 아이템 줍기 델리게이트 바인딩 됨.
	PlayerInputComponent->BindAction("PickUpItem", IE_Pressed, this, &AHelloFutureCharacter::PickUpItem);

	////////////// 채팅 ////////////////
	/*PlayerInputComponent->BindAction("Chatting", IE_Pressed, this, &AHelloFutureCharacter::Chatting);*/
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
		CurrentInteractive = Interactive;
	}
}

void AHelloFutureCharacter::ClearInteractiveInRange(class AOH_InteractiveBase* Interactive)
{
	CurrentInteractive = nullptr;
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

