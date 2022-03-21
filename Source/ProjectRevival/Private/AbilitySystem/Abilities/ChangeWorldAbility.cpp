// Project Revival. All Rights Reserved


#include "AbilitySystem/Abilities/ChangeWorldAbility.h"

#include "PRGameModeBase.h"
#include "AbilitySystem/AbilityActors/ChangeWorldSphereActor.h"

void UChangeWorldAbility::CommitExecute(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::CommitExecute(Handle, ActorInfo, ActivationInfo);
	const auto Owner = ActorInfo->OwnerActor.Get();
	if (!Owner)
	{
		UE_LOG(LogPRAbilitySystemBase, Error, TEXT("Unable to get Owner Actor"))
		K2_EndAbility();
	}
	auto GameMode = Cast<APRGameModeBase>(Owner->GetWorld()->GetAuthGameMode());
	if (!GameMode)
	{
		UE_LOG(LogPRAbilitySystemBase, Error, TEXT("Incorrect gamemode!!!"))
		K2_EndAbility();
	}
	GameMode->SetCurrentWorld(GameMode->GetCurrentWorld()==OrdinaryWorld ? OtherWorld : OrdinaryWorld);
	ChangeWorldTask=UChangeWorldTask_SpawnSphere::ChangeWorldInit(this,ChangeWorldShere,TraceSpawnDistance);
	auto SpawnedSphereActor = ChangeWorldTask->StartTask(*ActorInfo->OwnerActor);
	if(SpawnedSphereActor)
	{
		SpawnedSphereActor->AbilityEnded.AddUObject(this,&UChangeWorldAbility::FinishAbility);
	}
	
}

void UChangeWorldAbility::FinishAbility()
{
	ChangeWorldTask->EndTask();
	K2_EndAbility();
}
void UChangeWorldAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

