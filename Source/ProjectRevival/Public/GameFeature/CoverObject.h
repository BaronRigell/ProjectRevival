// Project Revival. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagAssetInterface.h"
#include "GameFramework/Actor.h"
#include "ProjectRevival/Public/CoreTypes.h"
#include "CoverObject.generated.h"

UCLASS()
class PROJECTREVIVAL_API ACoverObject : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ACoverObject();
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Занято ли укрытие
	bool bIsTaken;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
	FCoverPointsAndPossibility CoverStruct;


public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	bool IsCoverTaken() const { return bIsTaken; }

	virtual bool TryToFindCoverPoint(FVector PlayerPos, FVector& CoverPos);
};
