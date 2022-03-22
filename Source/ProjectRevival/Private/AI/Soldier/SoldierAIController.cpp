// Project Revival. All Rights Reserved


#include "AI/Soldier/SoldierAIController.h"
#include "AI/AICharacter.h"
#include "BasePickup.h"
#include "Kismet/GameplayStatics.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "RespawnComponent.h"
#include "SoldierEnemy.h"
#include "ProjectRevival/Public/CoreTypes.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Sight.h"

DEFINE_LOG_CATEGORY(LogPRAIController);
DEFINE_LOG_CATEGORY(LogPRAIDecorators);

ASoldierAIController::ASoldierAIController()
{
	PRPerceptionComponent = CreateDefaultSubobject<UPRSoldierAIPerceptionComponent>("PRPerceptionComponent");
	const auto SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>("Sight Config");
	SightConfig->SightRadius = 2000.0f;
	SightConfig->LoseSightRadius = 2500.0f;
	SightConfig->PeripheralVisionAngleDegrees = 90.0f;
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->SetMaxAge(15.0f);
	PRPerceptionComponent->ConfigureSense(*SightConfig);

	const auto HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>("Hearing Config");
	HearingConfig->HearingRange = 3000.0f;
	HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
	HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
	HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
	HearingConfig->SetMaxAge(15.0f);
	PRPerceptionComponent->ConfigureSense(*HearingConfig);
	
	PRPerceptionComponent->SetDominantSense(UAISense_Sight::StaticClass());

	SetPerceptionComponent(*PRPerceptionComponent);

	RespawnComponent = CreateDefaultSubobject<URespawnComponent>("RespawnController");
	
	SideMovementAmount = 0;
	bWantsPlayerState = true;
	bIsFiring = false;
	bIsInCover = false;
	bIsSideTurning = false;
	BotWing = EWing::Center;
}

void ASoldierAIController::SetPlayerPos(const FPlayerPositionData &NewPlayerPos)
{
	if (NewPlayerPos.GetActor())
	{
		PlayerPos.SetActor(NewPlayerPos.GetActor());
	}
	if (NewPlayerPos.GetCover())
	{
		PlayerPos.SetCover(NewPlayerPos.GetCover());
	}
	OnPlayerSpotted.Broadcast(PlayerPos);
}

void ASoldierAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	const auto AIChar = Cast<AAICharacter>(InPawn);
	//UE_LOG(LogPRAIController, Log, TEXT("Character Possessed"));
	if (AIChar)
	{
		//UE_LOG(LogPRAIController, Log, TEXT("BehaviorTree started"));
		RunBehaviorTree(AIChar->BehaviorTreeAsset);
		Cast<ASoldierEnemy>(GetPawn())->StopEnteringCoverDelegate.AddDynamic(this, &ASoldierAIController::StopEnteringCover);
		Cast<ASoldierEnemy>(GetPawn())->StopExitingCoverDelegate.AddDynamic(this, &ASoldierAIController::StopExitingCover);
		Cast<ASoldierEnemy>(GetPawn())->StopCoverSideMovingDelegate.AddDynamic(this, &ASoldierAIController::StopCoverSideMoving);
		Cast<ASoldierEnemy>(GetPawn())->StartFireDelegate.AddDynamic(this, &ASoldierAIController::StartFiring);
		Cast<ASoldierEnemy>(GetPawn())->StopFireDelegate.AddDynamic(this, &ASoldierAIController::StopFiring);
	}
}

void ASoldierAIController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	const auto AimActor = GetFocusOnActor();
	SetFocus(AimActor);
}

void ASoldierAIController::BeginPlay()
{
	Super::BeginPlay();
}

void ASoldierAIController::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	OnBotDied.Clear();
	OnPlayerSpotted.Clear();
	if (OnBotWingDecision.IsBound()) OnBotWingDecision.Unbind();
}

void ASoldierAIController::StartFiring()
{
	const auto PlayerCoordinates = PlayerPos.GetActorPosition();
	UE_LOG(LogPRAIController, Log, TEXT("Shoot at Player pos X: %0.2f, Y: %0.2f, Z: %0.2f"), PlayerCoordinates.X, PlayerCoordinates.Y);
	PlayerPosDelegate.Broadcast(PlayerPos);
}

void ASoldierAIController::StopFiring()
{
	SetBIsFiring(false);
}

void ASoldierAIController::StartEnteringCover()
{
	UE_LOG(LogPRAIController, Log, TEXT("%i Cover pos X: %0.2f, Y: %0.2f"), BotWing, CoverPos.X, CoverPos.Y);
	StartEnteringCoverDelegate.Broadcast(CoverPos);
}

void ASoldierAIController::StopEnteringCover()
{
	SetBIsInCover(true);
}

void ASoldierAIController::StartExitingCover()
{
	StartExitingCoverDelegate.Broadcast();
}

void ASoldierAIController::StopExitingCover()
{
	SetBIsInCover(false);
}

void ASoldierAIController::StartCoverSideMoving()
{
	SetBIsSideTurning(true);
	//SideMovementAmount defines the desired movement distance while in cover
	StartCoverSideMovingDelegate.Broadcast(SideMovementAmount);
}

void ASoldierAIController::StopCoverSideMoving()
{
	SetBIsSideTurning(false);
}

void ASoldierAIController::FindNewCover()
{
	bool const bFlag = PRPerceptionComponent->GetBestCoverWing(BotWing, CoverPos);
	const auto BlackboardComp = GetBlackboardComponent();
	if (bFlag && BlackboardComp)
	{
		const auto PlayerCoordinates = PlayerPos.GetActorPosition();
		UE_LOG(LogPRAIController, Log, TEXT("Player pos X: %0.2f, Y: %0.2f"), PlayerCoordinates.X, PlayerCoordinates.Y);
		UE_LOG(LogPRAIController, Log, TEXT("Cover pos was set X: %0.2f, Y: %0.2f"), CoverPos.X, CoverPos.Y);
		BlackboardComp->SetValueAsVector(CoverKeyName, CoverPos);
	}
	//MoveToLocation(CoverPos);
}

AActor* ASoldierAIController::GetFocusOnActor()
{
	if (!GetBlackboardComponent()) return nullptr;
	return Cast<AActor>(GetBlackboardComponent()->GetValueAsObject(FocusOnKeyName));
}
