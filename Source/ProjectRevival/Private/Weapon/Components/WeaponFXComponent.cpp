// Project Revival. All Rights Reserved


#include "Weapon/Components/WeaponFXComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "PhysicalMaterials/PhysicalMaterial.h"
#include "Components/DecalComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

UWeaponFXComponent::UWeaponFXComponent()
{

	PrimaryComponentTick.bCanEverTick = false;
}

void UWeaponFXComponent::PlayImpactFX(const FHitResult& HitResult)
{
	auto ImpactData = DefaultImpactData;

	if (HitResult.PhysMaterial.IsValid())
	{
		const auto PhysMaterial = HitResult.PhysMaterial.Get();
		if (ImpactDataMap.Contains(PhysMaterial))
		{
			ImpactData = ImpactDataMap[PhysMaterial];
		}
	}

	if (ImpactData.bUseNiagaraImpactEffect == true && ImpactData.NiagaraEffect)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), ImpactData.NiagaraEffect, HitResult.ImpactPoint,
		HitResult.ImpactNormal.Rotation());
	}
	else if (ImpactData.CascadeEffect)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactData.CascadeEffect, HitResult.ImpactPoint,
		HitResult.ImpactNormal.Rotation());
	}
	else return;
	auto DecalComponent = UGameplayStatics::SpawnDecalAtLocation(GetWorld(), ImpactData.DecalData.Material, ImpactData.DecalData.Size,
		HitResult.ImpactPoint, HitResult.ImpactNormal.Rotation());
	if (DecalComponent)
	{
		DecalComponent->SetFadeOut(ImpactData.DecalData.LifeTime, ImpactData.DecalData.FadeOutTime);
	}

	UGameplayStatics::PlaySoundAtLocation(GetWorld(), ImpactData.Sound, HitResult.ImpactPoint);
}

