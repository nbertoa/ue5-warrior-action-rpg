#include "Controllers/WarriorAIController.h"

#include "BehaviorTree/BlackboardComponent.h"
#include "Navigation/CrowdFollowingComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Utils/WarriorRPGDebugHelper.h"

AWarriorAIController::AWarriorAIController(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer.SetDefaultSubobjectClass<UCrowdFollowingComponent>(TEXT("PathFollowingComponent")))
{
    AISenseConfig_Sight = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("EnemySenseConfig_Sight"));
    AISenseConfig_Sight->DetectionByAffiliation.bDetectEnemies = true;
    AISenseConfig_Sight->DetectionByAffiliation.bDetectFriendlies = false;
    AISenseConfig_Sight->DetectionByAffiliation.bDetectNeutrals = false;
    AISenseConfig_Sight->SightRadius = 5000.0f;
    AISenseConfig_Sight->LoseSightRadius = 0.0f;
    AISenseConfig_Sight->PeripheralVisionAngleDegrees = 360.0f;

    EnemyPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("EnemyPerceptionComponent"));
    EnemyPerceptionComponent->ConfigureSense(*AISenseConfig_Sight);
    EnemyPerceptionComponent->SetDominantSense(UAISenseConfig_Sight::StaticClass());
    EnemyPerceptionComponent->OnTargetPerceptionUpdated.AddUniqueDynamic(this,
                                                                         &ThisClass::OnEnemyPerceptionUpdated);

    SetGenericTeamId(FGenericTeamId(1));
}

ETeamAttitude::Type AWarriorAIController::GetTeamAttitudeTowards(const AActor& Other) const
{
    const APawn* PawnToCheck = Cast<const APawn>(&Other);
    if (!PawnToCheck)
    {
        return ETeamAttitude::Neutral;
    }

    const AController* OtherController = PawnToCheck->GetController();
    if (!OtherController)
    {
        return ETeamAttitude::Neutral;
    }

    const IGenericTeamAgentInterface* OtherTeamAgent = Cast<const IGenericTeamAgentInterface>(OtherController);
    if (!OtherTeamAgent)
    {
        return ETeamAttitude::Neutral;
    }

    return OtherTeamAgent->GetGenericTeamId() < GetGenericTeamId() ? ETeamAttitude::Hostile : ETeamAttitude::Friendly;
}

void AWarriorAIController::BeginPlay()
{
    Super::BeginPlay();

    UCrowdFollowingComponent* CrowdComp = Cast<UCrowdFollowingComponent>(GetPathFollowingComponent());
    if (CrowdComp == nullptr)
    {
        return;
    }

    CrowdComp->SetCrowdSimulationState(bEnableDetourCrowdAvoidance ? ECrowdSimulationState::Enabled : ECrowdSimulationState::Disabled);

    ECrowdAvoidanceQuality::Type QualityType;
    switch (DetourCrowdAvoidanceQuality)
    {
        case 1:
            QualityType = ECrowdAvoidanceQuality::Low;
            break;
        case 2:
            QualityType = ECrowdAvoidanceQuality::Medium;
            break;
        case 3:
            QualityType = ECrowdAvoidanceQuality::Good;
            break;
        case 4:
            QualityType = ECrowdAvoidanceQuality::High;
            break;
        default:
            QualityType = ECrowdAvoidanceQuality::Low;
            break;
    }

    CrowdComp->SetCrowdAvoidanceQuality(QualityType);
    CrowdComp->SetAvoidanceGroup(1);
    CrowdComp->SetGroupsToAvoid(1);
    CrowdComp->SetCrowdCollisionQueryRange(CollisionQueryRange);
}

void AWarriorAIController::OnEnemyPerceptionUpdated(AActor* Actor,
                                                    FAIStimulus Stimulus)
{
    check(Actor);

    if (Stimulus.WasSuccessfullySensed())
    {
        if (UBlackboardComponent* BlackboardComponent = GetBlackboardComponent())
        {
            BlackboardComponent->SetValueAsObject(FName("TargetActor"),
                                                  Actor);
        }
    }
}
