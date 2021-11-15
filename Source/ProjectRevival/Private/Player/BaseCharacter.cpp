// Project Revival. All Rights Reserved


#include "Player/BaseCharacter.h"
#include "Components/BaseCharacterMovementComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/HealthComponent.h"
#include "Components/TextRenderComponent.h"
#include "Components/WeaponComponent.h"
#include "Components/CapsuleComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"


DEFINE_LOG_CATEGORY(LogPRAbilitySystemBase);

// Sets default values
ABaseCharacter::ABaseCharacter(const FObjectInitializer& ObjectInitializer) :Super(
	ObjectInitializer.SetDefaultSubobjectClass<UBaseCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	

	HealthComponent = CreateDefaultSubobject<UHealthComponent>("HealthComponent");

	WeaponComponent = CreateDefaultSubobject<UWeaponComponent>("WeaponComponent");
	
	AbilitySystemComponent = CreateDefaultSubobject<UPRAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AttributeSet = CreateDefaultSubobject<UPRAttributeSet>(TEXT("AttributeSet"));
}

void ABaseCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
		AddStartupGameplayAbilities();
	}
}

void ABaseCharacter::UnPossessed()
{
	
	Super::UnPossessed();
}

UAbilitySystemComponent* ABaseCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

// Called when the game starts or when spawned
void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	check(HealthComponent);
	
	HealthComponent->OnDeath.AddUObject(this, &ABaseCharacter::OnDeath);
	HealthComponent->OnHealthChanged.AddUObject(this, &ABaseCharacter::OnHealthChanged);
	OnHealthChanged(HealthComponent->GetHealth(), 1.0f);
	LandedDelegate.AddDynamic(this,&ABaseCharacter::OnGroundLanded);
}

// Called every frame
void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}

bool ABaseCharacter::IsRunning() const
{
	return false;
}

float ABaseCharacter::GetMovementDirection() const
{
	if (GetVelocity().IsZero()) return 0.0f;
	const FVector VelocityNormal = GetVelocity().GetSafeNormal();
	const auto AngleBetween =FMath::Acos(FVector::DotProduct(GetActorForwardVector(), VelocityNormal));
	const auto CrossProduct = FVector::CrossProduct(GetActorForwardVector(), VelocityNormal);
	const auto Degrees = FMath::RadiansToDegrees(AngleBetween);
	return Degrees*FMath::Sign(CrossProduct.Z);
}

// Called to bind functionality to input


void ABaseCharacter::SetPlayerColor(const FLinearColor& Color)
{
	const auto MateralInst = GetMesh()->CreateAndSetMaterialInstanceDynamic(0);
	if (!MateralInst) return;

	MateralInst->SetVectorParameterValue(MaterialColorName, Color);
}


void ABaseCharacter::AddStartupGameplayAbilities()
{
	for (const auto& Ability : GameplayAbilities)
	{
		AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(Ability.Value, 1, static_cast<int32>(Ability.Key), this));
		UE_LOG(LogPRAbilitySystemBase, Display, TEXT("%s has granted"), *Ability.Value.GetDefaultObject()->GetName());
	}
}

void ABaseCharacter::OnEnergyAttributeChanged(const FOnAttributeChangeData& Data)
{
	UE_LOG(LogPRAbilitySystemBase, Display, TEXT("New value of %s is %f"), *Data.Attribute.GetName(), Data.NewValue);
}

void ABaseCharacter::OnCooldownExpired(const FActiveGameplayEffect& ExpiredEffect)
{
	UE_LOG(LogPRAbilitySystemBase, Display, TEXT("Cooldown effect %s is expired"), *ExpiredEffect.Handle.ToString());
}

void ABaseCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(AttributeSet->GetEnergyAttribute())
		.AddUObject(this, &ABaseCharacter::OnEnergyAttributeChanged);
	AbilitySystemComponent->OnAnyGameplayEffectRemovedDelegate().AddUObject(this, &ABaseCharacter::OnCooldownExpired);
}

void ABaseCharacter::OnDeath()
{
	//PlayAnimMontage(DeathAnimMontage);
	GetCharacterMovement()->DisableMovement();
	SetLifeSpan(5.0f);
	GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	WeaponComponent->StopFire();

	GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	GetMesh()->SetSimulatePhysics(true);

	UGameplayStatics::PlaySoundAtLocation(GetWorld(), DeathSound, GetActorLocation());
}

void ABaseCharacter::OnHealthChanged(float CurrentHealth, float HealthDelta)
{
	//const auto Health = HealthComponent->GetHealth();
	
}

void ABaseCharacter::OnGroundLanded(const FHitResult& HitResult)
{
	const auto FallVelocity = -GetCharacterMovement()->Velocity.Z;
	if (FallVelocity<LandedDamageVelocity.X) return;
	const auto FinalDamage = FMath::GetMappedRangeValueClamped(LandedDamageVelocity,LandedDamage, FallVelocity);
	TakeDamage(FinalDamage,FDamageEvent{}, nullptr, nullptr);
}