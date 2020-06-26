// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <vector>

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ARTrackedFiducial.h"
#include "FiducialRelocalizerComponent.generated.h"


UCLASS(Blueprintable, BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FIDUCIALRELOCALIZER_API UFiducialRelocalizerComponent : public UActorComponent
{
	GENERATED_BODY()
public:
    // Sets default values for this component's properties
    UFiducialRelocalizerComponent();
    
    UPROPERTY(BlueprintReadOnly)
    TMap<FString, UARTrackedFiducial*> ActiveFiducials;
    
    // fiducials, selected for estimation
    UPROPERTY(BlueprintReadOnly)
    TArray<UARTrackedFiducial*> EstimationFiducials;
    
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable)
    UARTrackedFiducial* AddNewFiducial(UARTrackedImage* trackedImage, AFAnchor* fanchor);
    
    UFUNCTION(BlueprintCallable)
    void RemoveFiducial(UARTrackedFiducial* fiducial);
	
    UFUNCTION(BlueprintCallable)
    void PickEstimationFiducials();
    
protected:
    // Called when the game starts
    virtual void BeginPlay() override;
    
private:
    std::vector<UARTrackedFiducial*> fiducialsList_;
    
};
