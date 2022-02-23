// Project Revival. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ChangeWorld.h"
#include "StaticObjectToNothing.generated.h"

class UBoxComponent;

UCLASS()

class PROJECTREVIVAL_API AStaticObjectToNothing : public AChangeWorld
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AStaticObjectToNothing();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	FCollisionResponseContainer CollisionResponseContainer;
	TArray<UMaterialInstanceDynamic*> MeshesMaterials;
	
    FTimeline TimeLine;
    FOnTimelineEvent OnTimeLineFinished;
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	UCurveFloat* VisualCurve;
    float MinCurveValue;
    float MaxCurveValue;
    bool isApearing=false;
    UFUNCTION()
    virtual void TimeLineFinished();
    UFUNCTION()
    virtual void TimeLineFloatReturn(float Value);
    FOnTimelineFloat InterpFunction;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
	UStaticMeshComponent* SuperMesh;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
	TEnumAsByte<EChangeWorld> World = OrdinaryWorld;
	

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USceneComponent* SceneComponent;

	UFUNCTION()
	void OnMeshComponentCollision(UPrimitiveComponent* OverlappedComponent, 
					  AActor* OtherActor, 
					  UPrimitiveComponent* OtherComp, 
					  int32 OtherBodyIndex, 
					  bool bFromSweep, 
					  const FHitResult &SweepResult);
	
	void Changing() override;
};
