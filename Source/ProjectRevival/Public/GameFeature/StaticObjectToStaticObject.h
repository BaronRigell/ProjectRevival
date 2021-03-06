// Project Revival. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "ChangeWorld.h"
#include "Interfaces/IChangingWorldActor.h"
#include "ProjectRevival/Public/CoreTypes.h"
#include "StaticObjectToStaticObject.generated.h"

class UBoxComponent;

UCLASS()
class PROJECTREVIVAL_API AStaticObjectToStaticObject : public AChangeWorld, public IIChangingWorldActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AStaticObjectToStaticObject();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	FCollisionResponseContainer OrdinaryWorldCollisionResponseContainer;
	FCollisionResponseContainer OtherWorldCollisionResponseContainer;
	TArray<UMaterialInstanceDynamic*> OrdinaryWMeshesMaterials;
	TArray<UMaterialInstanceDynamic*> OtherWMeshesMaterials;
	UFUNCTION()
	void OrdinaryWTimelineFinished();
	UFUNCTION()
	void OtherWTimelineFinished();
	UFUNCTION()
	void OrdinaryWTimelineFloatReturn(float Value);
	UFUNCTION()
	void OtherWTimelineFloatReturn(float Value);
	FTimeline OrdinaryWTimeLine;
	FTimeline OtherWTimeLine;
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	UCurveFloat* OrdinaryWVisualCurve;
	float MinOrWValue;
	float MaxOrWValue;
	UPROPERTY(EditAnywhere,BlueprintReadWrite)
	UCurveFloat* OtherWVisualCurve;
	float MinOtWValue;
	float MaxOtWValue;
	FOnTimelineFloat OrWInterpFunction;
	FOnTimelineFloat OtWInterpFunction;
	FOnTimelineEvent OrOnTimeLineFinished;
	FOnTimelineEvent OtOnTimeLineFinished;
	bool OrIsAppearing;
	bool OtIsAppearing;
	virtual void Changing() override;
    virtual void LoadComponentTags(UStaticMeshComponent* supermesh) override;
    TArray<FName> OrMeshTags;
    TArray<FName> OtMeshTags;
    
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
	TEnumAsByte<EChangeEditorVisibility> VisibleWorld = BothWorlds;
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
	TEnumAsByte<EChangeAllMapEditorVisibility> AllObjectVisibleWorld=OwnValuesWorld;
	virtual void ClearComponentTags(UStaticMeshComponent* supermesh) override;

	UPROPERTY(EditDefaultsOnly)
	USoundCue* DisappearSound;

	UPROPERTY(EditDefaultsOnly)
	USoundCue* AppearSound;

	virtual void PlaySound(USoundCue* SoundToPlay);
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	
	//UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
	//TEnumAsByte<EChangeWorld> World = OrdinaryWorld;

	TEnumAsByte<EChangeWorld> CurrentWorld = OrdinaryWorld;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
	UStaticMeshComponent* SuperMesh1;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
	UStaticMeshComponent* SuperMesh2;

	UFUNCTION()
	void OnOrdinaryMeshCollision(UPrimitiveComponent* OverlappedComponent, 
					  AActor* OtherActor, 
					  UPrimitiveComponent* OtherComp, 
					  int32 OtherBodyIndex, 
					  bool bFromSweep, 
					  const FHitResult &SweepResult);
	UFUNCTION()
	void OnOtherMeshCollision(UPrimitiveComponent* OverlappedComponent, 
					  AActor* OtherActor, 
					  UPrimitiveComponent* OtherComp, 
					  int32 OtherBodyIndex, 
					  bool bFromSweep, 
					  const FHitResult &SweepResult);
	
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
	FCoverPointsAndPossibility CoverStructForOrdinaryWObject;

	UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
	FCoverPointsAndPossibility CoverStructForOtherWOther;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	USceneComponent* SceneComponent;
	
	virtual void ChangeVisibleWorld(EChangeAllMapEditorVisibility VisibleInEditorWorld) override;
	virtual bool CheckIsChangeAbleObjIsCover() override;
	virtual bool TryToFindCoverPoint(FVector PlayerPos, FVector& CoverPos) override;
	
	//this func might cause problems in the future but we'll see
	virtual void SetLastCoverPointStatus(bool bIsFree) override;

};
