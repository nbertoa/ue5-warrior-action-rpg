#include "AI/BTTask_RotateToFaceTarget.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/KismetMathLibrary.h"

UBTTask_RotateToFaceTarget::UBTTask_RotateToFaceTarget()
{
	NodeName = TEXT("Native Rotate to Face Target Actor");

	// AnglePrecision and RotationInterpSpeed are initialized via default member
	// initializers in the header — no need to repeat them here.
	bNotifyTick = true;
	bNotifyTaskFinished = true;
	bCreateNodeInstance = false;

	INIT_TASK_NODE_NOTIFY_FLAGS();

	InTargetToFaceKey.AddObjectFilter(this,
	                                  GET_MEMBER_NAME_CHECKED(ThisClass,
	                                                          InTargetToFaceKey),
	                                  AActor::StaticClass());
}

void UBTTask_RotateToFaceTarget::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	UBlackboardData* BBAsset = GetBlackboardAsset();
	check(BBAsset);

	InTargetToFaceKey.ResolveSelectedKey(*BBAsset);
}

uint16 UBTTask_RotateToFaceTarget::GetInstanceMemorySize() const
{
	return sizeof(FRotateToFaceTargetTaskMemory);
}

FString UBTTask_RotateToFaceTarget::GetStaticDescription() const
{
	const FString KeyDescription = InTargetToFaceKey.SelectedKeyName.ToString();

	return FString::Printf(TEXT("Smoothly rotates to face %s Key until the angle precision: %s is reached"),
	                       *KeyDescription,
	                       *FString::SanitizeFloat(AnglePrecision));
}

EBTNodeResult::Type UBTTask_RotateToFaceTarget::ExecuteTask(UBehaviorTreeComponent& OwnerComp,
                                                            uint8* NodeMemory)
{
	UBlackboardComponent* BBComponent = OwnerComp.GetBlackboardComponent();
	check(BBComponent);

	// Cast instead of CastChecked — the Blackboard key may be unset at runtime
	// (designer forgot to wire it in the BT asset), in which case GetValueAsObject
	// returns null. Failing gracefully here is preferable to a hard crash.
	UObject* ActorObject = BBComponent->GetValueAsObject(InTargetToFaceKey.SelectedKeyName);
	AActor* TargetActor = Cast<AActor>(ActorObject);

	APawn* OwningPawn = OwnerComp.GetAIOwner()->GetPawn();
	check(OwningPawn);

	FRotateToFaceTargetTaskMemory* Memory = CastInstanceNodeMemory<FRotateToFaceTargetTaskMemory>(NodeMemory);
	check(Memory);

	Memory->OwningPawn = OwningPawn;
	Memory->TargetActor = TargetActor;

	if (!Memory->IsValid())
	{
		return EBTNodeResult::Failed;
	}

	// Early-out: if already within angle precision, succeed immediately
	// without entering the tick loop — avoids one unnecessary frame of interpolation.
	if (HasReachedAnglePrecision(OwningPawn,
	                             TargetActor))
	{
		Memory->Reset();
		return EBTNodeResult::Succeeded;
	}

	return EBTNodeResult::InProgress;
}

void UBTTask_RotateToFaceTarget::TickTask(UBehaviorTreeComponent& OwnerComp,
                                          uint8* NodeMemory,
                                          float DeltaSeconds)
{
	FRotateToFaceTargetTaskMemory* Memory = CastInstanceNodeMemory<FRotateToFaceTargetTaskMemory>(NodeMemory);

	if (!Memory->IsValid())
	{
		// Pawn or target was destroyed mid-task — fail cleanly rather than dereferencing a stale pointer.
		FinishLatentTask(OwnerComp,
		                 EBTNodeResult::Failed);
		return;
	}

	if (HasReachedAnglePrecision(Memory->OwningPawn.Get(),
	                             Memory->TargetActor.Get()))
	{
		Memory->Reset();
		FinishLatentTask(OwnerComp,
		                 EBTNodeResult::Succeeded);
		return;
	}

	// Still rotating — interpolate toward the target this frame.
	APawn* OwningPawn = Memory->OwningPawn.Get();
	AActor* TargetActor = Memory->TargetActor.Get();

	const FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(OwningPawn->GetActorLocation(),
	                                                                  TargetActor->GetActorLocation());
	const FRotator TargetRot = FMath::RInterpTo(OwningPawn->GetActorRotation(),
	                                            LookAtRot,
	                                            DeltaSeconds,
	                                            RotationInterpSpeed);

	OwningPawn->SetActorRotation(TargetRot);
}

bool UBTTask_RotateToFaceTarget::HasReachedAnglePrecision(APawn* QueryPawn,
                                                          AActor* TargetActor) const
{
	check(QueryPawn);
	check(TargetActor);

	const FVector OwnerForward = QueryPawn->GetActorForwardVector();
	const FVector OwnerToTargetNormalized = (TargetActor->GetActorLocation() - QueryPawn->GetActorLocation()).
			GetSafeNormal();

	// DotProduct gives cos(angle) — DegAcos converts to degrees.
	// When the result is <= AnglePrecision the pawn is considered to be facing the target.
	const float DotResult = FVector::DotProduct(OwnerForward,
	                                            OwnerToTargetNormalized);
	const float AngleDiff = UKismetMathLibrary::DegAcos(DotResult);

	return AngleDiff <= AnglePrecision;
}
