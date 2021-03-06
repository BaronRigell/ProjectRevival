// Project Revival. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ProjectRevival/Public/CoreTypes.h"
#include "WeaponComponent.generated.h"

class ABaseWeapon;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PROJECTREVIVAL_API UWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UWeaponComponent();
	virtual void StartFire();
	void StopFire();
	virtual void NextWeapon();
	void Reload();
	bool GetCurrentWeaponUIData(FWeaponUIData& UIData) const;
	bool GetCurrentWeaponAmmoData(FAmmoData& AmmoData) const;
	int32 GetCurrentWeaponClips() const;
	int32 GetCurrentWeaponBullets() const;
	int32 GetMaxWeaponClips() const;
	int32 GetMaxWeaponBullets() const;
	int32 GetWeaponNum()const{return WeaponDatas.Num();}
	bool TryToAddAmmo(TSubclassOf<ABaseWeapon> WeaponType, int32 ClipsAmount);
	bool CanFire();
	bool CanEquip();
	bool CanReload();
	UFUNCTION(BlueprintCallable)
	bool IsShooting();
	UFUNCTION(BlueprintCallable)
	bool IsReloading();
	bool IsWeaponBlocked() const { return bIsWeaponBlocked; }
	void SetWeaponBlocked(const bool bIsBlocked) { bIsWeaponBlocked = bIsBlocked; }
	bool IsWeaponReloading() const {return ReloadAnimInProgress;}
	TArray<UMaterialInstanceDynamic*> GetCurrentWeaponMaterials();
	UFUNCTION(BlueprintCallable)
	ABaseWeapon* GetCurrentWeapon();
	TArray<FAmmoData> GetAllWeapons();
	void SetWeponData(FAmmoData NewAmmoData);
	void AddWeapon(TSubclassOf<ABaseWeapon> NewWeaponData);
	void DeleteWeapon();
	void EquipWeapon(int32 WeaponIndex);
protected:
	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	TArray<FWeaponData> WeaponDatas;

	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	FName WeaponEquipSocketName="WeaponPoint";

	UPROPERTY(EditDefaultsOnly, Category="Weapon")
	FName WeaponArmorySocketName="ArmorySocket";

	UPROPERTY(EditDefaultsOnly, Category="Animation")
	UAnimMontage* EquipAnimMontage;

	UPROPERTY(EditDefaultsOnly, Category="Animations")
	UAnimMontage* FireMontage;
	
	UPROPERTY()
	ABaseWeapon* CurrentWeapon=nullptr;

	UPROPERTY()
	TArray<ABaseWeapon*> Weapons;

	

	int32 CurrentWeaponIndex = 0;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
private:
	UPROPERTY()
	UAnimMontage* CurrentReloadAnimMontage;
	

	bool EquipAnimInProgress;
	bool ReloadAnimInProgress;
	bool ShootingInProgress;
	bool bIsWeaponBlocked;
	void SpawnWeapons();
	void AttachWeaponToSocket(ABaseWeapon* Weapon, USceneComponent* CharacterMesh, const FName& SocketName);
	
	void PlayAnimMontage(UAnimMontage* MontageToPlay) const;
	void InitAnimations();
	void OnEquipFinished(USkeletalMeshComponent* MeshComponent);
	void OnReloadFinished(USkeletalMeshComponent* MeshComponent);
	void OnShotMade();

	void OnEmptyClip(ABaseWeapon* AmmoEmptyWeapon);
	void ChangeClip();
};

