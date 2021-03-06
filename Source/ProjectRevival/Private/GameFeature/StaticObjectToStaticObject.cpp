// Project Revival. All Rights Reserved


#include "GameFeature/StaticObjectToStaticObject.h"

#include "DrawDebugHelpers.h"
#include "AbilitySystem/AbilityActors/ChangeWorldSphereActor.h"
#include "Components/BoxComponent.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"

// Sets default values
AStaticObjectToStaticObject::AStaticObjectToStaticObject()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SceneComponent = CreateDefaultSubobject<USceneComponent>("SceneComponent");
	SceneComponent->SetMobility(EComponentMobility::Static);
	RootComponent = SceneComponent;

	SuperMesh1 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SuperMesh1"));
	SuperMesh1->SetupAttachment(RootComponent);
	SuperMesh1->SetMobility(EComponentMobility::Movable);
	SuperMesh1->SetVisibility(true);

	OrWInterpFunction.BindUFunction(this,FName("OrdinaryWTimelineFloatReturn"));
	OtWInterpFunction.BindUFunction(this,FName("OtherWTimelineFloatReturn"));
	
	OrOnTimeLineFinished.BindUFunction(this,FName("OrdinaryWTimelineFinished"));
	OtOnTimeLineFinished.BindUFunction(this,FName("OtherWTimelineFinished"));

	SuperMesh2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("SuperMesh2"));
	SuperMesh2->SetupAttachment(RootComponent);
	SuperMesh2->SetMobility(EComponentMobility::Movable);
	SuperMesh2->SetVisibility(true);
}

// Called when the game starts or when spawned
void AStaticObjectToStaticObject::BeginPlay()
{
	Super::BeginPlay();

	OrdinaryWorldCollisionResponseContainer=SuperMesh1->GetCollisionResponseToChannels();
	for(int i=0;i<SuperMesh1->ComponentTags.Num();i++)
	{
		OrMeshTags.Add(SuperMesh1->ComponentTags[i]);
	}
	OtherWorldCollisionResponseContainer=SuperMesh2->GetCollisionResponseToChannels();
	for(int i=0;i<SuperMesh2->ComponentTags.Num();i++)
	{
		OtMeshTags.Add(SuperMesh2->ComponentTags[i]);
	}
	
	SuperMesh1->SetVisibility(true);
	SuperMesh2->SetVisibility(true);
	
	int32 num=SuperMesh1->GetNumMaterials();
	for (int32 i=0;i<num;i++)
	{
		const auto Material = SuperMesh1->CreateDynamicMaterialInstance(i);
		OrdinaryWMeshesMaterials.Add(Material);
	}
	num=SuperMesh2->GetNumMaterials();
	for (int32 i=0;i<num;i++)
	{
		const auto Material = SuperMesh2->CreateDynamicMaterialInstance(i);
		OtherWMeshesMaterials.Add(Material);
	}
	
	if(OrdinaryWVisualCurve&&OtherWVisualCurve)
	{
		OrdinaryWVisualCurve->GetValueRange(MinOrWValue,MaxOrWValue);
		OtherWVisualCurve->GetValueRange(MinOtWValue,MaxOtWValue);

		OrdinaryWTimeLine.AddInterpFloat(OrdinaryWVisualCurve,OrWInterpFunction);
		OtherWTimeLine.AddInterpFloat(OtherWVisualCurve,OtWInterpFunction);

		
		OrdinaryWTimeLine.SetTimelineFinishedFunc(OrOnTimeLineFinished);
		OtherWTimeLine.SetTimelineFinishedFunc(OtOnTimeLineFinished);
		
		OrdinaryWTimeLine.SetLooping(false);
		OtherWTimeLine.SetLooping(false);
	}
	if (CurrentWorld==OrdinaryWorld)
	{
		if(OrdinaryWVisualCurve&&OtherWVisualCurve)
		{
			for (const auto Material : OrdinaryWMeshesMaterials)
			{
				Material->SetScalarParameterValue("Amount",MinOrWValue);
			}
			for (const auto Material : OtherWMeshesMaterials)
			{
				Material->SetScalarParameterValue("Amount",MaxOtWValue);
			}
		}
		else
		{
			SuperMesh1->SetCollisionProfileName("BlockAll");
			SuperMesh1->SetVisibility(true);
			SuperMesh2->SetCollisionProfileName("OverlapAll");
			SuperMesh2->SetVisibility(false);
		}
		SuperMesh1->SetCollisionProfileName("OverlapAll");
		SuperMesh1->SetCollisionResponseToChannels(OrdinaryWorldCollisionResponseContainer);
		SuperMesh2->SetCollisionProfileName("OverlapAll");
		OrIsAppearing=true;
		OtIsAppearing=false;
		ClearComponentTags(SuperMesh2);
	}
	else
	{
		if(OrdinaryWVisualCurve&&OtherWVisualCurve)
		{
			for (const auto Material : OrdinaryWMeshesMaterials)
			{
				Material->SetScalarParameterValue("Amount",MaxOrWValue);
			}
			for (const auto Material : OtherWMeshesMaterials)
			{
				Material->SetScalarParameterValue("Amount",MinOtWValue);
			}
		}
		else
		{
			SuperMesh1->SetCollisionProfileName("OverlapAll");
			SuperMesh1->SetVisibility(false);
			SuperMesh2->SetCollisionProfileName("BlockAll");
			SuperMesh2->SetVisibility(true);
		}
		SuperMesh2->SetCollisionProfileName("OverlapAll");
		SuperMesh2->SetCollisionResponseToChannels(OrdinaryWorldCollisionResponseContainer);
		SuperMesh1->SetCollisionProfileName("OverlapAll");
		OtIsAppearing=true;
		OrIsAppearing=false;
		ClearComponentTags(SuperMesh1);
	}
	SuperMesh1->OnComponentBeginOverlap.AddDynamic(this,&AStaticObjectToStaticObject::OnOrdinaryMeshCollision);
	SuperMesh2->OnComponentBeginOverlap.AddDynamic(this,&AStaticObjectToStaticObject::OnOtherMeshCollision);
}

void AStaticObjectToStaticObject::ChangeVisibleWorld(EChangeAllMapEditorVisibility VisibleInEditorWorld)
{
	if(VisibleInEditorWorld!=OwnValuesWorld)
	{
		switch (VisibleInEditorWorld)
		{
		case DefaultVisibleWorld:
			VisibleWorld=DefaultWorld;
			break;
		case OtherVisibleWorld:
			VisibleWorld=AltirnativeWorld;
			break;
		case BothVisibleWorlds:
			VisibleWorld=BothWorlds;
			break;
		default:
			break;
		}
		if(VisibleWorld==DefaultWorld)
		{
			SuperMesh1->SetVisibility(true);
			SuperMesh2->SetVisibility(false);

		}
		else if(VisibleWorld==AltirnativeWorld)
		{
			SuperMesh1->SetVisibility(false);
			SuperMesh2->SetVisibility(true);
		}
		else
		{
			SuperMesh1->SetVisibility(true);
			SuperMesh2->SetVisibility(true);
		}
	}
	else
	{
		SuperMesh1->SetVisibility(true);
		SuperMesh2->SetVisibility(false);
	}
}

bool AStaticObjectToStaticObject::CheckIsChangeAbleObjIsCover()
{
	return SuperMesh1->ComponentTags.Contains("Cover")||SuperMesh2->ComponentTags.Contains("Cover");
}

void AStaticObjectToStaticObject::ClearComponentTags(UStaticMeshComponent* supermesh)
{
	IIChangingWorldActor::ClearComponentTags(supermesh);
}

void AStaticObjectToStaticObject::PlaySound(USoundCue* SoundToPlay)
{
	if (SoundToPlay)
	{
		UGameplayStatics::SpawnSoundAttached(SoundToPlay,GetRootComponent());
	}
}

// Called every frame
void AStaticObjectToStaticObject::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if(OrdinaryWTimeLine.IsPlaying())
	{
		OrdinaryWTimeLine.TickTimeline(DeltaTime);
	}
	if(OtherWTimeLine.IsPlaying())
	{
		OtherWTimeLine.TickTimeline(DeltaTime);
	}

}
#if WITH_EDITOR
void AStaticObjectToStaticObject::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if(PropertyChangedEvent.Property->GetName()=="VisibleWorld")
	{
		if(VisibleWorld==DefaultWorld)
		{
			SuperMesh1->SetVisibility(true);
			SuperMesh2->SetVisibility(false);

		}
		else if(VisibleWorld==AltirnativeWorld)
		{
			SuperMesh1->SetVisibility(false);
			SuperMesh2->SetVisibility(true);
		}
		else
		{
			SuperMesh1->SetVisibility(true);
			SuperMesh2->SetVisibility(true);
		}
	}
	if(PropertyChangedEvent.Property->GetName()=="AllObjectVisibleWorld")
	{
		TArray<AActor*> ChangeAbleObjs;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(),AChangeWorld::StaticClass(),ChangeAbleObjs);
		for(auto obj:ChangeAbleObjs)
		{
			ChangeVisibleWorld(AllObjectVisibleWorld);
		}
	}
	if(PropertyChangedEvent.Property->GetName()=="CoverPointsAmount")
	{
		CoverStructForOrdinaryWObject.PositionsOfCoverPoints.Empty();
		for(auto covpos:CoverStructForOrdinaryWObject.CoverPositions)
		{
			if(covpos)
				CoverStructForOrdinaryWObject.PositionsOfCoverPoints.Push(covpos->GetRelativeLocation());
			covpos->DestroyComponent();
		}
		if(CoverStructForOrdinaryWObject.CoverPositions.Num()>=0) CoverStructForOrdinaryWObject.CoverPositions.Empty();
		CoverStructForOrdinaryWObject.CoverPositions.SetNum(CoverStructForOrdinaryWObject.CoverPointsAmount);
		for(int i=0;i<CoverStructForOrdinaryWObject.CoverPositions.Num();++i)
		{
			auto newCoverPoint=NewObject<UBoxComponent>(this,UBoxComponent::StaticClass(),*FString("CoverOrdinaryPos").Append(FString::FromInt(i+1)));
			newCoverPoint->CreationMethod=EComponentCreationMethod::Native;
			newCoverPoint->OnComponentCreated();
			newCoverPoint->SetupAttachment(RootComponent);
			newCoverPoint->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			newCoverPoint->SetMobility(EComponentMobility::Movable);
			if(i<CoverStructForOrdinaryWObject.PositionsOfCoverPoints.Num())
			{
				newCoverPoint->SetWorldLocation(CoverStructForOrdinaryWObject.PositionsOfCoverPoints[i]);
				newCoverPoint->SetRelativeLocation(CoverStructForOrdinaryWObject.PositionsOfCoverPoints[i]);
			}
			newCoverPoint->RegisterComponent();
			CoverStructForOrdinaryWObject.CoverPositions[i]=newCoverPoint;
		}

		CoverStructForOtherWOther.PositionsOfCoverPoints.Empty();
		for(auto covpos:CoverStructForOtherWOther.CoverPositions)
		{
			if(covpos)
				CoverStructForOtherWOther.PositionsOfCoverPoints.Push(covpos->GetRelativeLocation());
			covpos->DestroyComponent();
		}
		if(CoverStructForOtherWOther.CoverPositions.Num()>=0) CoverStructForOtherWOther.CoverPositions.Empty();
		CoverStructForOtherWOther.CoverPositions.SetNum(CoverStructForOtherWOther.CoverPointsAmount);
		for(int i=0;i<CoverStructForOtherWOther.CoverPositions.Num();++i)
		{
			auto newCoverPoint=NewObject<UBoxComponent>(this,UBoxComponent::StaticClass(),*FString("CoverOtherPos").Append(FString::FromInt(i+1)));
			newCoverPoint->CreationMethod=EComponentCreationMethod::Native;
			newCoverPoint->OnComponentCreated();
			newCoverPoint->SetupAttachment(RootComponent);
			newCoverPoint->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			newCoverPoint->SetMobility(EComponentMobility::Movable);
			if(i<CoverStructForOtherWOther.PositionsOfCoverPoints.Num())
			{
				newCoverPoint->SetWorldLocation(CoverStructForOtherWOther.PositionsOfCoverPoints[i]);
				newCoverPoint->SetRelativeLocation(CoverStructForOtherWOther.PositionsOfCoverPoints[i]);
			}
			newCoverPoint->RegisterComponent();
			CoverStructForOtherWOther.CoverPositions[i]=newCoverPoint;
		}
		
		}
		else if(VisibleWorld==AltirnativeWorld)
		{
			SuperMesh1->SetVisibility(false);
			SuperMesh2->SetVisibility(true);
		}
		else
		{
			SuperMesh1->SetVisibility(true);
			SuperMesh2->SetVisibility(true);
		}
}

#endif


void AStaticObjectToStaticObject::OnOrdinaryMeshCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if(OrdinaryWVisualCurve)
	{
		if(Cast<AChangeWorldSphereActor>(OtherActor))
		{
			if(!OrIsAppearing)
			{
				OrIsAppearing=true;
				PlaySound(AppearSound);
				LoadComponentTags(SuperMesh1);
				OrdinaryWTimeLine.PlayFromStart();
				SuperMesh1->SetCollisionResponseToChannels(OrdinaryWorldCollisionResponseContainer);
			}
			else
			{
				PlaySound(DisappearSound);
				OrIsAppearing=false;
				SuperMesh1->SetCollisionProfileName("OverlapAll");
				OrdinaryWTimeLine.PlayFromStart();
			}
		}
	}
	else
	{
		Changing();
	}
}

void AStaticObjectToStaticObject::OnOtherMeshCollision(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{

	if(Cast<AChangeWorldSphereActor>(OtherActor))
	{
		if(OtherWVisualCurve)
		{
			if(!OtIsAppearing)
			{
				OtIsAppearing=true;
				PlaySound(AppearSound);
				LoadComponentTags(SuperMesh2);
				OtherWTimeLine.PlayFromStart();
				SuperMesh2->SetCollisionResponseToChannels(OtherWorldCollisionResponseContainer);
			}
			else
			{
				PlaySound(DisappearSound);
				OtIsAppearing=false;
				SuperMesh2->SetCollisionProfileName("OverlapAll");
				OtherWTimeLine.PlayFromStart();
			
			}
		}
		else
		{
			Changing();
		}
	}
}

void AStaticObjectToStaticObject::SetLastCoverPointStatus(bool bIsFree)
{
	FCoverPointsAndPossibility coverstruct;
	if(SuperMesh1->GetCollisionResponseToChannels()==OrdinaryWorldCollisionResponseContainer)
	{
		coverstruct=CoverStructForOrdinaryWObject;
	}
	else
	{
		coverstruct=CoverStructForOtherWOther;
	}
	coverstruct.PointIsNotTaken[Cast<UBoxComponent>(coverstruct.LastCoverPosition)] = bIsFree;
}

bool AStaticObjectToStaticObject::TryToFindCoverPoint(FVector PlayerPos, FVector& CoverPos)
{
	FCoverPointsAndPossibility coverstruct;
	if(SuperMesh1->GetCollisionResponseToChannels()==OrdinaryWorldCollisionResponseContainer)
	{
		coverstruct=CoverStructForOrdinaryWObject;
	}
	else
	{
		coverstruct=CoverStructForOtherWOther;
	}
	if(coverstruct.CoverPositions.Num()==0) return false;
	UE_LOG(LogTemp,Warning,TEXT("StaticToStatic: Found coverpoints in ord world 3"))
	for(USceneComponent* covpos:coverstruct.CoverPositions)
	{
		FVector TraceStart=covpos->GetComponentLocation();
		FVector TraceEnd=PlayerPos;
		FHitResult HitResult;
		FCollisionQueryParams CollisionParams;
		GetWorld()->LineTraceSingleByChannel(HitResult,TraceStart,TraceEnd,ECollisionChannel::ECC_Visibility,CollisionParams);
		UBoxComponent* box = Cast<UBoxComponent>(covpos);
		if(HitResult.bBlockingHit)
		{
			DrawDebugLine(GetWorld(),TraceStart,HitResult.ImpactPoint,FColor::Blue,false,3.0f,0,3.0f);
			if(HitResult.Actor==this && coverstruct.PointIsNotTaken.Contains(box) && coverstruct.PointIsNotTaken[box])
			{
				UE_LOG(LogPRAISoldier, Log, TEXT("StaticToStatic: covpos set to %s"), *covpos->GetComponentLocation().ToString())
				CoverPos = TraceStart;
				coverstruct.LastCoverPosition = covpos;
				return true;
			}
		}
	}
	return false;
}

void AStaticObjectToStaticObject::OrdinaryWTimelineFinished()
{
	if(!OrIsAppearing)
	{
		ClearComponentTags(SuperMesh1);
	}
}

void AStaticObjectToStaticObject::OtherWTimelineFinished()
{
	if(!OtIsAppearing)
	{
		ClearComponentTags(SuperMesh2);
	}
	
}

void AStaticObjectToStaticObject::OrdinaryWTimelineFloatReturn(float Value)
{
	for(const auto material:OrdinaryWMeshesMaterials)
	{
		if(OrIsAppearing)
		{
			material->SetScalarParameterValue("Amount",Value);
		}
		else
		{
			float val=MinOrWValue-Value;
			val=MaxOrWValue+val;
			material->SetScalarParameterValue("Amount",val);
		}
	}
}

void AStaticObjectToStaticObject::OtherWTimelineFloatReturn(float Value)
{
	for(const auto material:OtherWMeshesMaterials)
	{
		if(OtIsAppearing)
		{
			material->SetScalarParameterValue("Amount",Value);
		}
		else
		{
			float val=MinOtWValue-Value;
			val=MaxOtWValue+val;
			material->SetScalarParameterValue("Amount",val);
		}
	}
}





void AStaticObjectToStaticObject::Changing()
{
	

	if(CurrentWorld==OrdinaryWorld)
	{
		CurrentWorld=OtherWorld;
		SuperMesh1->SetCollisionProfileName("OverlapAll");
		SuperMesh1->SetVisibility(false);
		SuperMesh2->SetCollisionProfileName("BlockAll");
		SuperMesh2->SetVisibility(true);
	}
	else
	{
		CurrentWorld=OrdinaryWorld;
		SuperMesh1->SetCollisionProfileName("BlockAll");
		SuperMesh1->SetVisibility(true);
		SuperMesh2->SetCollisionProfileName("OverlapAll");
		SuperMesh2->SetVisibility(false);
	}
	
	
}

void AStaticObjectToStaticObject::LoadComponentTags(UStaticMeshComponent* supermesh)
{
	if(supermesh==SuperMesh1)
	{
		for(int i=0;i<OrMeshTags.Num();i++)
		{
			supermesh->ComponentTags.AddUnique(OrMeshTags[i]);
		}
	}
	else
	{
		for(int i=0;i<OtMeshTags.Num();i++)
		{
			supermesh->ComponentTags.AddUnique(OtMeshTags[i]);
		}
	}
}
