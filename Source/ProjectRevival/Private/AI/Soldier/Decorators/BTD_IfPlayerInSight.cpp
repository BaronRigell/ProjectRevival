// Project Revival. All Rights Reserved


#include "AI/Soldier/Decorators/BTD_IfPlayerInSight.h"
#include "AIController.h"
#include "PlayerCharacter.h"
#include "Components/PRSoldierAIPerceptionComponent.h"
#include "Soldier/SoldierAIController.h"

UBTD_IfPlayerInSight::UBTD_IfPlayerInSight()
{
	NodeName = "Check If Player in Sight";
}

bool UBTD_IfPlayerInSight::CalcCondition(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	const auto Controller = Cast<ASoldierAIController>(OwnerComp.GetAIOwner());
	if (!Controller) return false;
	const auto PerceptionComp = Cast<UPRSoldierAIPerceptionComponent>(Controller->GetPerceptionComponent());
	if(!PerceptionComp) return false;

	const auto PlayerPos = PerceptionComp->GetClosestEnemy();
	UE_LOG(LogPRAIDecorators, Log, TEXT("PlayerPos.GetActor() %s"), PlayerPos.GetActor()?TEXT("true"):TEXT("false"))
	UE_LOG(LogPRAIDecorators, Log, TEXT("Controller %s !Controller->GetBIsFiring() %s"), *FString(Controller->GetName()),
		!Controller->GetBIsFiring()?TEXT("true"):TEXT("false"))
	return PlayerPos.GetActor() && !Controller->GetBIsFiring();
	
	
	/*UE_LOG(LogPRAIDecorators, Log, TEXT("Is shooting possible %s"), Controller->GetBIsPlayerInSight() && !Controller->GetBIsFiring()?TEXT("true"):TEXT("false"))
	
	return Controller->GetBIsPlayerInSight() && !Controller->GetBIsFiring();*/
}

bool UBTD_IfPlayerInSight::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	return CalcCondition(OwnerComp, NodeMemory);
}

void UBTD_IfPlayerInSight::OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	TNodeInstanceMemory* DecoratorMemory = CastInstanceNodeMemory<TNodeInstanceMemory>(NodeMemory);
	DecoratorMemory->bLastRawResult = CalcCondition(OwnerComp, NodeMemory);
}

void UBTD_IfPlayerInSight::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	TNodeInstanceMemory* DecoratorMemory = CastInstanceNodeMemory<TNodeInstanceMemory>(NodeMemory);
	
	const bool bResult = CalcCondition(OwnerComp, NodeMemory);
	if (bResult != DecoratorMemory->bLastRawResult)
	{
		DecoratorMemory->bLastRawResult = bResult;
		OwnerComp.RequestExecution(this);
	}
}
