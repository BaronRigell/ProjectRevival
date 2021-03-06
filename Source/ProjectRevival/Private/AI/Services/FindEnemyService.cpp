// Project Revival. All Rights Reserved


#include "AI/Services/FindEnemyService.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "PRUtils.h"
#include "PRAIPerceptionComponent.h"
#include "PRSoldierAIPerceptionComponent.h"

class UPRSoldierAIPerceptionComponent;

UFindEnemyService::UFindEnemyService()
{
	NodeName = "Find Enemy";
}

void UFindEnemyService::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	const auto Blackboard = OwnerComp.GetBlackboardComponent();

	if (Blackboard)
	{
		const auto Controller = OwnerComp.GetAIOwner();
		const auto PerceptionComponent = PRUtils::GetCharacterComponent<UPRSoldierAIPerceptionComponent>(Controller);
		if (PerceptionComponent)
		{
			Blackboard->SetValueAsObject(EnemyActorKey.SelectedKeyName, PerceptionComponent->GetClosestEnemy().GetActor());
		}
	}
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);
}
