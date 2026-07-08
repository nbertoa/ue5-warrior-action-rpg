// PawnExtensionComponentBase.h
// Generic base for any ActorComponent that lives on a Pawn and needs
// type-safe access to its owning Pawn or Controller.
// Includes strict defensive programming to prevent CDO and lifecycle crashes.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PawnExtensionComponentBase.generated.h"

/**
 * Base class for components that extend a Pawn's functionality.
 * Provides templated accessors for the owning Pawn and its Controller
 * with compile-time type checking and runtime defensive validation.
 */
UCLASS(ClassGroup=(Custom),
    meta=(BlueprintSpawnableComponent))
class WARRIORRPG_API UPawnExtensionComponentBase : public UActorComponent
{
    GENERATED_BODY()

protected:
    /**
     * Returns the owning Pawn cast to the requested type.
     * * @tparam T  The expected Pawn subclass. Must derive from APawn.
     * @return    The owning actor cast to T, or nullptr if no owner exists.
     */
    template <class T>
    T* GetOwningPawn() const
    {
        static_assert(TPointerIsConvertibleFromTo<T, APawn>::Value,
                      "'T' Template Parameter to GetPawn must be derived from APawn");

        // Defensive: Components can temporarily lack an owner during CDO initialization 
        // or destruction phases. We must never pass a null pointer to CastChecked.
        AActor* OwnerActor = GetOwner();
        if (!ensureMsgf(OwnerActor,
                        TEXT("%s: GetOwningPawn called but GetOwner() returned null. " "Check lifecycle phase (CDO/Destruction)."),
                        *GetName()))
        {
            return nullptr;
        }

        // Safe to proceed. CastChecked will crash intentionally only if the owner 
        // exists but is NOT of type T (which implies a fatal configuration error).
        return CastChecked<T>(OwnerActor);
    }

    /**
     * Convenience overload that returns the owning Pawn as the base APawn type.
     */
    APawn* GetOwningPawn() const
    {
        return GetOwningPawn<APawn>();
    }

    /**
     * Returns the owning Pawn's Controller cast to the requested type.
     *
     * @tparam T  The expected Controller subclass. Must derive from AController.
     * @return    The owning Pawn's Controller cast to T, or nullptr.
     */
    template <class T>
    T* GetOwningController() const
    {
        static_assert(TPointerIsConvertibleFromTo<T, AController>::Value,
                      "'T' Template Parameter to GetController must be derived from AController");

        // Defensive: GetOwningPawn handles its own null checks and logging.
        APawn* OwningPawn = GetOwningPawn<APawn>();
        if (!OwningPawn)
        {
            return nullptr;
        }

        // It is valid for a Pawn to not have a controller (e.g., unpossessed AI).
        // We use Cast instead of CastChecked here.
        return Cast<T>(OwningPawn->GetController());
    }
};
