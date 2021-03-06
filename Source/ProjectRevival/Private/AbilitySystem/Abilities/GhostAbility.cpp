// Project Revival. All Rights Reserved


#include "AbilitySystem/Abilities/GhostAbility.h"
#include "PRGameModeBase.h"
#include "AbilitySystem/Abilities/Miscellaneuos/IDynMaterialsFromMesh.h"
#include "Kismet/GameplayStatics.h"

UGhostAbility::UGhostAbility()
{
	AbilityAction = EGASInputActions::Ghost; 
}

void UGhostAbility::CommitExecute(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo)
{
	Super::CommitExecute(Handle, ActorInfo, ActivationInfo);
	//далее идет место с твоей логикой, туда ты вставляешь свой код.
	//--------------------------------------------------------------------
	const auto Owner = ActorInfo->OwnerActor.Get();
	if (!Owner->Implements<UIDynMaterialsFromMesh>())
	{
		UE_LOG(LogPRAbilitySystemBase, Error, TEXT("Owner actor MUST implement IIDynMaterialsFromMesh interface!!!"))
		K2_EndAbility();
	}
	const auto GameMode = ActorInfo->OwnerActor->GetWorld()->GetAuthGameMode<APRGameModeBase>();
	const auto CurrentWorld = (GameMode) ? GameMode->GetCurrentWorld() : OrdinaryWorld; 
	GhostTask = UGhostTask_InvisibilityToggle::InvisibilityToggle(this, VisualCurve, Cast<IIDynMaterialsFromMesh>(Owner)->GetDynMaterials(), CurrentWorld);
	GhostTask->OnDisappearFinished.BindUFunction(this, "OnDisappearEnded");
	GhostTask->OnAppearFinished.BindUFunction(this, "OnAppearEnded");
	DelayTask = UAbilityTask_WaitDelay::WaitDelay(this, Duration);
	DelayTask->OnFinish.AddDynamic(this, &UGhostAbility::BeginAppear);

	PlaySound(StartSound);
	GhostTask->Activate();
	//--------------------------------------------------------------------
	
}

void UGhostAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
                               const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility,
                               bool bWasCancelled)
{
	
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGhostAbility::OnAppearEnded()
{
	GhostTask->OnAppearFinished.Unbind();
	ChangeBlendMode(false);
	GhostTask->EndTask();
	K2_EndAbility();
}

void UGhostAbility::OnDisappearEnded()
{
	GhostTask->OnDisappearFinished.Unbind();
	ChangeBlendMode(true);
	DelayTask->Activate();	
}


void UGhostAbility::BeginAppear()
{
	DelayTask->OnFinish.RemoveDynamic(this, &UGhostAbility::BeginAppear);
	PlaySound(EndSound);
	GhostTask->AppearMeshes();
}

void UGhostAbility::ChangeBlendMode(bool IsDisappearing)
{
	auto Materials = Cast<IIDynMaterialsFromMesh>((GetCurrentActorInfo()->OwnerActor.Get()))->GetDynMaterials();
	for (const auto Material : Materials)
	{
		if (Material)
			Material->BlendMode = IsDisappearing ? TEnumAsByte<EBlendMode>(EBlendMode::BLEND_Translucent) : TEnumAsByte<EBlendMode>(EBlendMode::BLEND_Opaque);
	}
}
