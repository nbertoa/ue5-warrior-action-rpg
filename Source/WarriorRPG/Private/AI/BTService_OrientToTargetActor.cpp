#include "AI/BTService_OrientToTargetActor.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Kismet/KismetMathLibrary.h"

UBTService_OrientToTargetActor::UBTService_OrientToTargetActor()
{
	NodeName = TEXT("Native Orient Rotation To Target Actor");

	INIT_SERVICE_NODE_NOTIFY_FLAGS();

	// RotationInterpSpeed is initialized in the header via default member initializer.
	// Interval and RandomDeviation are set to 0 so the service ticks every frame —
	// rotation toward the target must be continuous for smooth locomotion.
	Interval = 0.0f;
	RandomDeviation = 0.0f;

	InTargetActorKey.AddObjectFilter(this,
	                                 GET_MEMBER_NAME_CHECKED(ThisClass,
	                                                         InTargetActorKey),
	                                 AActor::StaticClass());
}

void UBTService_OrientToTargetActor::InitializeFromAsset(UBehaviorTree& Asset)
{
	Super::InitializeFromAsset(Asset);

	if (UBlackboardData* BBAsset = GetBlackboardAsset())
	{
		InTargetActorKey.ResolveSelectedKey(*BBAsset);
	}
}

FString UBTService_OrientToTargetActor::GetStaticDescription() const
{
	const FString KeyDescription = InTargetActorKey.SelectedKeyName.ToString();

	return FString::Printf(TEXT("Orient rotation to %s Key %s"),
	                       *KeyDescription,
	                       *GetStaticServiceDescription());
}

void UBTService_OrientToTargetActor::TickNode(UBehaviorTreeComponent& OwnerComp,
                                              uint8* NodeMemory,
                                              float DeltaSeconds)
{
	Super::TickNode(OwnerComp,
	                NodeMemory,
	                DeltaSeconds);

	AAIController* AIOwner = OwnerComp.GetAIOwner();
	check(AIOwner);
	APawn* OwningPawn = AIOwner->GetPawn();
	check(OwningPawn);

	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	check(BlackboardComponent);

	UObject* ActorObject = BlackboardComponent->GetValueAsObject(InTargetActorKey.SelectedKeyName);

	if (AActor* TargetActor = Cast<AActor>(ActorObject))
	{
		const FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(OwningPawn->GetActorLocation(),
		                                                                  TargetActor->GetActorLocation());
		const FRotator TargetRot = FMath::RInterpTo(OwningPawn->GetActorRotation(),
		                                            LookAtRot,
		                                            DeltaSeconds,
		                                            RotationInterpSpeed);

		OwningPawn->SetActorRotation(TargetRot);
	}
}
