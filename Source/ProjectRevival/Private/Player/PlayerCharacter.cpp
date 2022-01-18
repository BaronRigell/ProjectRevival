// Project Revival. All Rights Reserved

#define HIGHLIGHTABLE_TRACE_CHANNEL ECC_GameTraceChannel2
#define HIGHLIGHTABLE_COLLISION_OBJECT ECC_GameTraceChannel1


#include "Player/PlayerCharacter.h"
#include "AICharacter.h"
#include "DrawDebugHelpers.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/InputComponent.h"
#include "Components/WeaponComponent.h"
#include "Components/SphereComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/TimelineComponent.h"
#include "Components/BaseCharacterMovementComponent.h"
#include "ProjectRevival/ProjectRevival.h"
#include "GameFeature/StaticObjectToNothing.h"
#include "Kismet/GameplayStatics.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionConstantForce.h"

APlayerCharacter::APlayerCharacter(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	PrimaryActorTick.bCanEverTick = true;
	
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>("SpringArmComponent");
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->bUsePawnControlRotation = true;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>("CameraComponent");
	CameraComponent->SetupAttachment(SpringArmComponent);

	CameraCollisionComponent = CreateDefaultSubobject<USphereComponent>("CameraCollisionComponent");
	CameraCollisionComponent->SetupAttachment(CameraComponent);
	CameraCollisionComponent->SetSphereRadius(10.0f);
	CameraCollisionComponent->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);

	PlayerMovementComponent = Cast<UBaseCharacterMovementComponent>(GetCharacterMovement());
}


void APlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward",this,&APlayerCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight",PlayerMovementComponent,&UBaseCharacterMovementComponent::MoveRight);
	PlayerInputComponent->BindAxis("LookUp",this,&APlayerCharacter::LookUp);
	PlayerInputComponent->BindAxis("TurnAround",this,&APlayerCharacter::LookAround);
	PlayerInputComponent->BindAction("Jump",EInputEvent::IE_Pressed,PlayerMovementComponent, &UBaseCharacterMovementComponent::Jump);
	PlayerInputComponent->BindAction("Run",EInputEvent::IE_Pressed,this, &APlayerCharacter::StartRun);
	PlayerInputComponent->BindAction("Run",EInputEvent::IE_Released,this, &APlayerCharacter::StopRun);
	//PlayerInputComponent->BindAction("Flip",EInputEvent::IE_Pressed,this, &APlayerCharacter::Flip);
	//PlayerInputComponent->BindAction("Highlight",EInputEvent::IE_Pressed,this, &APlayerCharacter::HighlightAbility);
	PlayerInputComponent->BindAction("Fire",EInputEvent::IE_Pressed,this, &APlayerCharacter::StartFire);
	PlayerInputComponent->BindAction("Fire",EInputEvent::IE_Released,WeaponComponent, &UWeaponComponent::StopFire);
	PlayerInputComponent->BindAction("NextWeapon",EInputEvent::IE_Pressed,WeaponComponent, &UWeaponComponent::NextWeapon);
	PlayerInputComponent->BindAction("Reload",EInputEvent::IE_Pressed,WeaponComponent, &UWeaponComponent::Reload);
	PlayerInputComponent->BindAction("LeftCameraView", EInputEvent::IE_Pressed,this, &APlayerCharacter::OnCameraMove);
	AbilitySystemComponent->BindAbilityActivationToInputComponent(PlayerInputComponent,
	FGameplayAbilityInputBinds(FString("ConfirmTarget"),
	FString("CancelTarget"), FString("EGASInputActions")));
	PlayerInputComponent->BindAction("Zoom", IE_Pressed, this, &APlayerCharacter::CameraZoomIn);
	PlayerInputComponent->BindAction("Zoom", IE_Released, this, &APlayerCharacter::CameraZoomOut);
	PlayerInputComponent->BindAction("ChangeWorld", EInputEvent::IE_Pressed,this, &APlayerCharacter::OnWorldChanged);
	PlayerInputComponent->BindAction("Cover", EInputEvent::IE_Pressed,this, &APlayerCharacter::Cover);
	
}

void APlayerCharacter::MoveForward(float Amount)
{
	IsMovingForward = Amount>0;
	PlayerMovementComponent->MoveForward(Amount);
}

void APlayerCharacter::StartRun()
{
	if (PlayerMovementComponent->GetPlayerMovementLogic().IsInJump() || PlayerMovementComponent->GetPlayerMovementLogic().IsPivotTargeted) return;
	bWantsToRun=true;
}

void APlayerCharacter::StopRun()
{
	bWantsToRun=false;
}

void APlayerCharacter::StartFire()
{
	if (!PlayerMovementComponent->GetPlayerMovementLogic().IsPivotTargeted ||
		PlayerMovementComponent->GetPlayerMovementLogic().IsInJump()) return;
	WeaponComponent->StartFire();
}

void APlayerCharacter::LookUp(float Amount)
{
	AddControllerPitchInput(Amount);
}

void APlayerCharacter::LookAround(float Amount)
{
	AddControllerYawInput(Amount);
}

void APlayerCharacter::Cover()
{
	
	if (IsInCover)
	{
		StopCover_Internal();
		return;
	}
	FHitResult CoverHit;
	const ECoverType CoverType = CoverTrace(CoverHit);
	if (CoverType==ECoverType::None) return;
	StartCover_Internal(CoverHit);
}

FRotator APlayerCharacter::GetAimDelta() const
{
	if (!GetController()) return FRotator();
	const auto CameraRotation = GetController()->K2_GetActorRotation();
	const auto PawnRotation = GetActorRotation();

	auto Delta = CameraRotation.Yaw-PawnRotation.Yaw;
	if (FMath::Abs<float>(Delta)>180)
	{
		Delta = Delta + 360.0*FMath::Sign<float>(Delta)*(-1.0);
	}

	return FRotator(CameraRotation.Pitch, Delta, 0.0f);
}

void APlayerCharacter::OnCameraCollisionBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                                     UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	CheckCameraOverlap();
}

void APlayerCharacter::OnCameraCollisionEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
CheckCameraOverlap();
}

void APlayerCharacter::CheckCameraOverlap()
{
	const auto HideMesh = CameraCollisionComponent->IsOverlappingComponent(GetCapsuleComponent());
	GetMesh()->SetOwnerNoSee(HideMesh);

	TArray<USceneComponent*> MeshChildren;
	GetMesh()->GetChildrenComponents(true, MeshChildren);

	for (auto MeshChild : MeshChildren)
	{
		const auto
		MeshChildGeometry = Cast<UPrimitiveComponent>(MeshChild);
		if (MeshChildGeometry)
		{
			MeshChildGeometry->SetOwnerNoSee(HideMesh);
		}
	}
}

bool APlayerCharacter::IsRunning() const
{
	return bWantsToRun && IsMovingForward && !GetVelocity().IsZero();
}

void APlayerCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (OnEnergyValueChangedHandle.IsBound()) OnEnergyValueChangedHandle.Clear();
	CameraCollisionComponent->OnComponentBeginOverlap.Clear();
	CameraCollisionComponent->OnComponentEndOverlap.Clear();
	Super::EndPlay(EndPlayReason);
}

void APlayerCharacter::OnEnergyAttributeChanged(const FOnAttributeChangeData& Data)
{
	Super::OnEnergyAttributeChanged(Data);
	//Для подключения делегата необходимо получить объект типа APlayerCharacter и сделать OnEnergyValueChangedHandle.BindUObject(this, &тип_класса::название_метода);
	//Далее указанный метод будет вызываться автоматически при помощи следующей команды
	float MaxEnergy = AttributeSet->GetMaxEnergy();
	if (OnEnergyValueChangedHandle.IsBound()) OnEnergyValueChangedHandle.Execute(Data.NewValue/MaxEnergy);
	
}

void APlayerCharacter::OnCooldownExpired(const FActiveGameplayEffect& ExpiredEffect)
{
	Super::OnCooldownExpired(ExpiredEffect);
	
}

void APlayerCharacter::OnDeath()
{
	Super::OnDeath();
	if (Controller)
	{
		Controller->ChangeState(NAME_Spectating);
	}
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	check(CameraCollisionComponent);
	check(GetCharacterMovement());
	CameraCollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &APlayerCharacter::OnCameraCollisionBeginOverlap);
	CameraCollisionComponent->OnComponentEndOverlap.AddDynamic(this, &APlayerCharacter::OnCameraCollisionEndOverlap);
}

void APlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CurveTimeline.TickTimeline(DeltaTime);
	LeftSideViewCurveTimeline.TickTimeline(DeltaTime);
}


void APlayerCharacter::TimelineProgress(float Value)
{
	FVector NewLocation = FMath::Lerp(PlayerAimZoom.StartLoc, PlayerAimZoom.EndLoc, Value);
	SpringArmComponent->SocketOffset = NewLocation;
}


void APlayerCharacter::TimelineFieldOfView(float Value)
{
	float NewFieldOfView = FMath::Lerp(CameraComponent->FieldOfView, PlayerAimZoom.CurrentFieldOfView, Value);
	CameraComponent->FieldOfView = NewFieldOfView;
	if (CameraComponent->FieldOfView >= PlayerAimZoom.CurrentFieldOfView && PlayerAimZoom.CurrentFieldOfView == 90.0) PlayerAimZoom.IsZooming = false;
}

void APlayerCharacter::TimelineLeftSideView(float Value)
{
	float NewView = FMath::Lerp(LeftSideView.StartPos, LeftSideView.EndPos, Value);
	SpringArmComponent->SocketOffset.Y = NewView;
	if ((SpringArmComponent->SocketOffset.Y >= LeftSideView.EndPos && LeftSideView.CamPos == true || SpringArmComponent->SocketOffset.Y <= LeftSideView.EndPos && LeftSideView.CamPos == false) && LeftSideView.Repeat == false) { CameraStop(); LeftSideView.Repeat = true; }
	
}

void APlayerCharacter::CameraZoomIn()
{
	if (LeftSideView.IsMoving == false || PlayerAimZoom.IsZooming==false)
	{
		if (PlayerMovementComponent->GetPlayerMovementLogic().IsInJump() || PlayerMovementComponent->GetPlayerMovementLogic().IsPivotTargeted) return;
		bWantsToRun=false;
		PlayerMovementComponent->bOrientRotationToMovement = 0;
		bUseControllerRotationYaw=true;
		PlayerMovementComponent->AimStart();

		
		if (PlayerAimZoom.StartStartPos == FVector(0.0, 0.0, 0.0)) PlayerAimZoom.StartStartPos = SpringArmComponent->SocketOffset;
		SpringArmComponent->SocketOffset = PlayerAimZoom.StartStartPos;
		FOnTimelineVector TimelineProgress;
		FOnTimelineFloat TimelineFieldOfView;
		TimelineProgress.BindUFunction(this, FName("TimelineProgress"));
		TimelineFieldOfView.BindUFunction(this, FName("TimelineFieldOfView"));
		CurveTimeline.AddInterpVector(PlayerAimZoom.CurveVector, TimelineProgress);
		CurveTimeline.AddInterpFloat(PlayerAimZoom.CurveFloat, TimelineFieldOfView);

		PlayerAimZoom.StartLoc = SpringArmComponent->SocketOffset;
		PlayerAimZoom.EndLoc = FVector(SpringArmComponent->SocketOffset.X + PlayerAimZoom.Offset.X, SpringArmComponent->SocketOffset.Y, SpringArmComponent->SocketOffset.Z + PlayerAimZoom.Offset.Z);
		if (LeftSideView.CamPos == false) PlayerAimZoom.EndLoc.Y -= PlayerAimZoom.Offset.Y; else PlayerAimZoom.EndLoc.Y += PlayerAimZoom.Offset.Y / 2.0;
		PlayerAimZoom.CurrentFieldOfView = PlayerAimZoom.FieldOfView;

		PlayerAimZoom.IsZooming = true;
		CurveTimeline.PlayFromStart();
		
	}
}

void APlayerCharacter::CameraZoomOut()
{
	if (LeftSideView.IsMoving == false && PlayerAimZoom.IsZooming == true)
	{
		PlayerMovementComponent->bOrientRotationToMovement = 1;
		bUseControllerRotationYaw=false;;
		PlayerMovementComponent->AimEnd();
		
		FOnTimelineVector TimelineProgress;
		FOnTimelineFloat TimelineFieldOfView;
		TimelineProgress.BindUFunction(this, FName("TimelineProgress"));
		TimelineFieldOfView.BindUFunction(this, FName("TimelineFieldOfView"));
		CurveTimeline.AddInterpVector(PlayerAimZoom.CurveVector, TimelineProgress);
		CurveTimeline.AddInterpFloat(PlayerAimZoom.CurveFloat, TimelineFieldOfView);
	
		PlayerAimZoom.EndLoc = PlayerAimZoom.StartLoc;
		PlayerAimZoom.StartLoc = SpringArmComponent->SocketOffset;
		PlayerAimZoom.CurrentFieldOfView = 90.0;

		PlayerAimZoom.IsZooming = false;
		CurveTimeline.PlayFromStart();
	}
}


void APlayerCharacter::OnCameraMove()
{
	if (LeftSideView.Block == false && PlayerAimZoom.IsZooming == false && LeftSideView.IsMoving == false)
	{
		if (LeftSideView.CamPos == false) LeftSideView.Proverka = SpringArmComponent->SocketOffset.Y;
		FOnTimelineFloat TimelineLeftSideView;
		TimelineLeftSideView.BindUFunction(this, FName("TimelineLeftSideView"));
		LeftSideViewCurveTimeline.AddInterpFloat(PlayerAimZoom.CurveFloat, TimelineLeftSideView);

		LeftSideView.StartPos = SpringArmComponent->SocketOffset.Y;
		LeftSideView.EndPos = LeftSideView.StartPos - (SpringArmComponent->SocketOffset.Y + tan(CameraComponent->GetRelativeRotation().Yaw * PI / 180) * SpringArmComponent->TargetArmLength) * 2.f;
		LeftSideView.Block = true;
		LeftSideView.IsMoving = true;
		LeftSideView.Repeat = false;
		LeftSideViewCurveTimeline.PlayFromStart();
	}
}


void APlayerCharacter::CameraStop()
{
	FTimerHandle TimerCameraBlock;
	LeftSideView.IsMoving = false;
	GetWorld()->GetTimerManager().SetTimer(TimerCameraBlock, this, &APlayerCharacter::CameraBlock, 0.5, false);
	if (LeftSideView.CamPos == true)
	{
		LeftSideView.CamPos = false;
		SpringArmComponent->SocketOffset.Y = LeftSideView.Proverka;
	}
	else LeftSideView.CamPos = true;
	PlayerAimZoom.StartStartPos = SpringArmComponent->SocketOffset;
}



void APlayerCharacter::CameraBlock()
{
	LeftSideView.Block = false;
}


void APlayerCharacter::OnWorldChanged()
{
	TSubclassOf<AStaticObjectToNothing> ClassToFind;
	ClassToFind = AStaticObjectToNothing::StaticClass();
	TArray<AActor*> OutActors;
	UGameplayStatics::GetAllActorsOfClass(this, ClassToFind, OutActors);
	for (int EveryActor = 0; EveryActor < OutActors.Num(); EveryActor++)
	{
		Cast<AStaticObjectToNothing>(OutActors[EveryActor])->Changing();
	}
}

bool APlayerCharacter::StartCover_Internal(FHitResult& CoverHit)
{
	const bool Sup = Super::StartCover_Internal(CoverHit);
	if (!Sup)return false;
	IsInCover=true;
	return true;
}

bool APlayerCharacter::StopCover_Internal()
{
	const bool Sup = Super::StopCover_Internal();
	if (!Sup)return false;
	IsInCover=false;;
	return true;
}
