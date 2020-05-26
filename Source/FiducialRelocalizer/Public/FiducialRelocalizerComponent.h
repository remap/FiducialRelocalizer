// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

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

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
    UPROPERTY(BlueprintReadOnly)
    TMap<FString, UARTrackedFiducial*> ActiveFiducials;
    
    // active fiducials, added in chronological order
    UPROPERTY(BlueprintReadOnly)
    TArray<UARTrackedFiducial*> ActiveFiducialsList;
    
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable)
    UARTrackedFiducial* AddNewFiducial(UARTrackedImage* trackedImage, AFAnchor* fanchor);
    
    UFUNCTION(BlueprintCallable)
    UARTrackedFiducial* getLatestFiducial() const;
		
};
