// Project Revival. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Soldier/SoldierAIController.h"
#include "AICharacter.h"
#include "Components/BoxComponent.h"
#include "ProjectRevival/Public/CoreTypes.h"
#include "GameFramework/Actor.h"
#include "AICoordinator.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPRCoordinator, All, All)



UCLASS()
class PROJECTREVIVAL_API AAICoordinator : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AAICoordinator();

protected:
	UPROPERTY(EditAnywhere)
	UBoxComponent* ArenaComponent;
	UPROPERTY(EditAnywhere)
	UBoxComponent* TriggerComponent;

	UPROPERTY(EditInstanceOnly, Category="Enemies")
	TSubclassOf<AAIController> EnemyControllerClass;
	UPROPERTY(EditInstanceOnly, Category="Enemies")
	TSubclassOf<AAICharacter> EnemyCharacterClass;
	UPROPERTY(EditAnywhere)
	float PlayerPositionUpdateTime=0.5f;

	UPROPERTY(EditInstanceOnly)
	TEnumAsByte<EChangeWorld> CoordinatorWorld;
	
	virtual void PostInitializeComponents() override;
	
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void ProcessBotDeath(ASoldierAIController* BotController);
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
private:
	UPROPERTY()
	TMap<ASoldierAIController*, EWing> BotMap;
	TMap<ASoldierAIController*, bool> BotInfoMap;
	UPROPERTY()
	FPlayerPositionData PlayerPosition;
	FTimerHandle PlayerInfoTimerHandle;
	
	bool InitSpawn();
	UFUNCTION()
	void OnTriggerOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	void SpawnBotsBySide(TArray<UChildActorComponent*>& SideComponents, TArray<UChildActorComponent*>& AllComponents, int32& NumSide, int32& NumAll, EWing WingSide);
	void SpawnBot(AActor* PlayerStartActor, EWing WingSide);
	AAICharacter* SpawnCharacterForBot(AActor* PlayerStartActor, const FTransform& Transform);
	void ConnectController(ASoldierAIController* BotController, EWing);
	void UpdatePlayerInfoFromBot(FPlayerPositionData PlayerPosition);
	UFUNCTION()
	bool MakeDecisionForWingBot() const;
	UFUNCTION()
	void UpdateBotPlayerInfo();
	bool CheckIfPlayerPosHasChanged(FPlayerPositionData NewPlayerPos, bool bState);
};




