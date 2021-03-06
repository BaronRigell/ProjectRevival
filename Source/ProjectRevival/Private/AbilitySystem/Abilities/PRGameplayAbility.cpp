// Project Revival. All Rights Reserved


#include "AbilitySystem/Abilities/PRGameplayAbility.h"
#include "BasePlayerController.h"
#include "GameHUD.h"
#include "PlayerHUDWidget.h"
#include "Kismet/GameplayStatics.h"

UPRGameplayAbility::UPRGameplayAbility()
{
	
}

void UPRGameplayAbility::PlaySound(USoundCue* SoundToPlay)
{
	if (SoundToPlay)
	{
		UGameplayStatics::SpawnSoundAttached(SoundToPlay, GetOwningActorFromActorInfo()->GetRootComponent());
	}
}

void UPRGameplayAbility::CommitExecute(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo)
{
	//Super::CommitExecute(Handle, ActorInfo, ActivationInfo);
	ApplyCost(Handle, ActorInfo, ActivationInfo);
	UE_LOG(LogPRAbilitySystemBase, Display, TEXT("%s has started"), *GetName());
	
	//Вызов старта кулдауна способности
	if (ActorInfo)
	{
		if (ActorInfo->PlayerController.IsValid())
		{
			auto Controller = ActorInfo->PlayerController.Get();
			if (Controller)
			{
				auto PlayerController = Cast<ABasePlayerController>(Controller);
				if (PlayerController)
				{
					auto PlayerHUD = PlayerController->GetHUD<AGameHUD>();
					if (PlayerHUD)
					{
						auto HUDWidget = PlayerHUD->GetPlayerHUDWidget();
						if (HUDWidget)
						{
							auto PlayerHUDWidget = Cast<UPlayerHUDWidget>(HUDWidget);
							if (PlayerHUDWidget)
							{
								AbilityWidget = PlayerHUDWidget->GetWidgetByAction(AbilityAction);
	
								if (!AbilityWidget) UE_LOG(LogPRAbilitySystemBase, Error,
                                    TEXT("Widget have not found. Check Blueprint version on AbilityAction parameter or widget method directly"));
								if (AbilityWidget)
								{
									AbilityWidget->StartAbility();	
								}
							}
						}
					}
				}
			}
		}
			
	}
	
	//K2_EndAbility();
}

void UPRGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	float CooldownMagnitude=0.f;
	UGameplayEffect* CooldownEffect = GetCooldownGameplayEffect(); 
	if (CooldownEffect)
	{
		CooldownEffect->DurationMagnitude.GetStaticMagnitudeIfPossible(1.f, CooldownMagnitude);
		UE_LOG(LogPRAbilitySystemBase, Display, TEXT("Cooldown is %f seconds"), CooldownMagnitude);
	}
	if(!bWasCancelled)
	{
		ApplyCooldown(Handle, ActorInfo, ActivationInfo);
		if (ActorInfo)
		{
			if (ActorInfo->PlayerController.IsValid())
			{
				auto Controller = ActorInfo->PlayerController.Get();
				if (Controller)
				{
					auto PlayerController = Cast<ABasePlayerController>(Controller);
					if (PlayerController)
					{
						auto PlayerHUD = PlayerController->GetHUD<AGameHUD>();
						if (PlayerHUD)
						{
							auto HUDWidget = PlayerHUD->GetPlayerHUDWidget();
							if (HUDWidget)
							{
								auto PlayerHUDWidget = Cast<UPlayerHUDWidget>(HUDWidget);
								if (PlayerHUDWidget)
								{
									AbilityWidget = PlayerHUDWidget->GetWidgetByAction(AbilityAction);
	
									if (!AbilityWidget) UE_LOG(LogPRAbilitySystemBase, Error,
										TEXT("Widget have not found. Check Blueprint version on AbilityAction parameter or widget method directly"));
									if (AbilityWidget)
									{
										AbilityWidget->StartCooldown(CooldownMagnitude);	
									}
								}
							}
						}
					}
				}
			}
			
		}
	}
	
	UE_LOG(LogPRAbilitySystemBase, Display, TEXT("%s has ended"), *GetName());
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UPRGameplayAbility::OnGiveAbility(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	Super::OnGiveAbility(ActorInfo, Spec);

	//Получение соответствующего инстанса виджета
	// auto PlayerHUDWidget = Cast<UPlayerHUDWidget>(Cast<ABasePlayerController>(ActorInfo->PlayerController.Get())->GetHUD<AGameHUD>()->GetPlayerHUDWidget());
	// if (PlayerHUDWidget)
	// {
	// 	// AbilityWidget = PlayerHUDWidget->GetWidgetByAction(AbilityAction);
	// }
	// AbilityWidget = PlayerHUDWidget->GetWidgetByAction(AbilityAction);
	// // AbilityWidget = Cast<UPlayerHUDWidget>(Cast<ABasePlayerController>(ActorInfo->PlayerController.Get())->GetHUD<AGameHUD>()->GetCurrentWidget())->GetWidgetByAction(AbilityAction);
	// if (!AbilityWidget) UE_LOG(LogPRAbilitySystemBase, Error, TEXT("Widget have not found. Check Blueprint version on AbilityAction parameter or widget method directly"));

}
