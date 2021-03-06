// Project Revival. All Rights Reserved


#include "AbilitySystem/Abilities/DimensionShotAbility.h"

#include "BaseCharacter.h"
#include "DimensionRevolver.h"
#include "PlayerCharacter.h"
#include "WeaponComponent.h"
#include "AbilitySystem/Abilities/GhostAbility.h"
#include "Misc/OutputDeviceConsole.h"

UDimensionShotAbility::UDimensionShotAbility()
{
	AbilityAction=EGASInputActions::DimensionShot;
}
bool UDimensionShotAbility::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                               const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
                                               const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	auto Player=Cast<APlayerCharacter>(ActorInfo->OwnerActor);
	
	bool IsRunning=Player->IsRunning();
	bool IsInJump=Player->GetMovementComponent()->GetPlayerMovementLogic().IsInJump();
	bool IsInCover=Player->GetCoverData().IsInCover();
	bool IsReloading=Player->GetWeaponComponent()->IsWeaponReloading();
	bool CanMakeAbility=!IsRunning&&!IsInJump&&!IsInCover&&!IsReloading;
	
	if(CanMakeAbility&&Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags))
	{
		Player->CameraZoomOut();
	}
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags)&&CanMakeAbility;
}

void UDimensionShotAbility::ShotWasMade()
{
	FinishAbility();
	EndAbility(GetCurrentAbilitySpecHandle(),GetCurrentActorInfo(),GetCurrentActivationInfo(),true,false);
}

void UDimensionShotAbility::InputPressed(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::InputPressed(Handle, ActorInfo, ActivationInfo);
	if(ShotBeingMade)
	{
		FinishAbility();
		EndAbility(Handle,ActorInfo,ActivationInfo,true,true);
	}
}

void UDimensionShotAbility::InterruptAbility()
{
	FinishAbility();
	EndAbility(GetCurrentAbilitySpecHandle(),GetCurrentActorInfo(),GetCurrentActivationInfo(),true,true);
}

void UDimensionShotAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                            const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                            const FGameplayEventData* TriggerEventData)
{
	//Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	ShotBeingMade=true;
	GiveRevolverTask=UDimensionShotTask_GiveRevolver::DimensionShotTaskInit(this,DimensionRevolverData,HealPercent);
	auto Weapon=GiveRevolverTask->StartTask();
	Weapon->OnWeaponShotDelegate.AddUObject(this,&UDimensionShotAbility::ShotWasMade);
	auto Revolver=Cast<ADimensionRevolver>(Weapon);
	Revolver->SetHealPercentToBullet(HealPercent);

}



void UDimensionShotAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateEndAbility, bool bWasCancelled)
{
	if(!bWasCancelled)
		CommitAbilityCooldown(Handle,ActorInfo,ActivationInfo,false);
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UDimensionShotAbility::CancelAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
	bool bReplicateCancelAbility)
{
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
}

void UDimensionShotAbility::FinishAbility()
{
	auto player=Cast<APlayerCharacter>(GetActorInfo().OwnerActor.Get());
	player->DimensionShotAbStruct.IsInRevolverAim=false;
	player->CameraZoomOut();
	auto weaponcomponent=player->GetWeaponComponent();
	weaponcomponent->GetCurrentWeapon()->IsAppearing=false;
	weaponcomponent->GetCurrentWeapon()->Changing();
	weaponcomponent->DeleteWeapon();
	weaponcomponent->GetCurrentWeapon()->IsAppearing=true;
	weaponcomponent->GetCurrentWeapon()->Changing();
	ShotBeingMade=false;
	GiveRevolverTask->EndTask();
}
