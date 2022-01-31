// Project Revival. All Rights Reserved

#pragma once

#include "CoreMinimal.h"
#include "ProjectRevival/Public/CoreTypes.h"
#include "BehaviorTree/BTDecorator.h"
#include "BTD_IfBotInCover.generated.h"

/**
 * 
 */
UCLASS()
class PROJECTREVIVAL_API UBTD_IfBotInCover : public UBTDecorator
{
	GENERATED_BODY()

	typedef FBTPlayerCheckDecoratorMemory TNodeInstanceMemory;

	UBTD_IfBotInCover();

protected:
	virtual bool CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const override;
	virtual void OnBecomeRelevant(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	bool CalcCondition(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const;
};
