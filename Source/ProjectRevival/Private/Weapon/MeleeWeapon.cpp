// Project Revival. All Rights Reserved

#include "Weapon/MeleeWeapon.h"
#include "AssassinEnemy.h"
#include "Weapon/Projectile/BaseProjectile.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"

AMeleeWeapon::AMeleeWeapon()
{
	RootComponent = WeaponMesh;
	
	BeamComp = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Beam Component"));
	BeamComp->bAutoActivate = false;
	BeamComp->SetupAttachment(RootComponent);
	
	BladeCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision Component"));
	BladeCollisionBox->SetupAttachment(RootComponent);
	BladeCollisionBox->SetGenerateOverlapEvents(true);
	BladeCollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AMeleeWeapon::OnOverlapBegin);
	BladeCollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AMeleeWeapon::BeginPlay()
{
	Super::BeginPlay();
	check(WeaponMesh);
	check(BladeCollisionBox);
	UE_LOG(LogPRAbilitySystemBase, Error, TEXT("BeginPlay"));
}

void AMeleeWeapon::AddNewBeam(const FVector Point1, const FVector Point2)
{
	BeamComp = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamFX, Point1, FRotator::ZeroRotator, true);
	BeamArray.Add(BeamComp);
 	BeamComp->SetBeamSourcePoint(0, Point1, 0);
	BeamComp->SetBeamTargetPoint(0, Point2, 0);
}

void AMeleeWeapon::ToggleCollisionOn()
{
 	//BladeCollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	const FVector TraceStart = WeaponMesh->GetSocketLocation("TraceStart");
	const FVector TraceEnd = WeaponMesh->GetSocketLocation("TraceEnd");
	if (BeamComp)
	{
		AddNewBeam(TraceStart, TraceEnd);
	}
}

void AMeleeWeapon::ToggleCollisionOff() const
{
	//BladeCollisionBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AMeleeWeapon::MakeDamage(AActor* OtherActor)
{
	//OtherActor->
}

void AMeleeWeapon::OnOverlapBegin(class UPrimitiveComponent* OverlappedComp, class AActor* OtherActor, class UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogPRAbilitySystemBase, Warning, TEXT("this: %s"), *this->GetName()); 
	UE_LOG(LogPRAbilitySystemBase, Warning, TEXT("OtherComp: %s"), *OtherComp->GetOwner()->GetName()); 
	UE_LOG(LogPRAbilitySystemBase, Warning, TEXT("OverlappedComp: %s"), *OverlappedComp->GetOwner()->GetName()); 
	UE_LOG(LogPRAbilitySystemBase, Warning, TEXT("OtherActor: %s"), *OtherActor->GetName());
	if (OtherActor && (OtherActor != this) && OtherComp && OtherActor != this->GetOwner() && !bIsHitDone)
	{
		bIsHitDone = true;
		UE_LOG(LogPRAbilitySystemBase, Error, TEXT("Damage done yaaay"));
	}
}