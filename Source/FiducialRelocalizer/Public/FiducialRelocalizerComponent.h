// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <string>
#include <vector>
#include <map>

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ARTrackedFiducial.h"
#include "Fanchor.h"
#include "ARBasePlayerController.h"
#include "FiducialRelocalizerComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FGoBackToLimboDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnNewPawnEstimation,
                                               FString, fiducialName,
                                               FTransform, arAlignment,
                                               FTransform, fanchorTransform,
                                               AFAnchor*, fanchor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFiducialTrackingStateChanged,
                                            UARTrackedFiducial*, fiducial);


UCLASS(Blueprintable, BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FIDUCIALRELOCALIZER_API UFiducialRelocalizerComponent : public UActorComponent
{
	GENERATED_BODY()
public:
    // Sets default values for this component's properties
    UFiducialRelocalizerComponent();
    
    UFUNCTION(BlueprintCallable)
    TMap<FString, UARTrackedFiducial*> GetActiveFiducials() const;
    
    // fiducials, selected for estimation
    UPROPERTY(BlueprintReadOnly)
    TArray<UARTrackedFiducial*> EstimationFiducials;
    
    UPROPERTY(BlueprintReadWrite)
    bool isScanMode;
    
    UPROPERTY(BlueprintReadOnly)
    int32 EstimationCounter;
    
    UPROPERTY(BlueprintReadOnly)
    AARBasePlayerController *controller;
    
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable)
    UARTrackedFiducial* AddNewFiducial(UARTrackedImage* trackedImage, AFAnchor* fanchor);
    
    UFUNCTION(BlueprintCallable)
    void RemoveFiducial(UARTrackedFiducial* fiducial);
	
    UFUNCTION(BlueprintCallable)
    void PickEstimationFiducials();
    
    UPROPERTY(BlueprintCallable, BlueprintAssignable)
    FGoBackToLimboDelegate OnGoBackToLimbo;
    
    UPROPERTY(BlueprintCallable, BlueprintAssignable)
    FOnNewPawnEstimation OnNewPawnEstimation;
    
    UPROPERTY(BlueprintCallable, BlueprintAssignable)
    FOnFiducialTrackingStateChanged OnFiducialTrackingStateChanged;
    
    UFUNCTION(BlueprintCallable)
    bool isValidTrackedImage(UARTrackedImage* trackedImage);
    
    /**
     * Scan mode: adds new measurement for a fiducial
     */
    UFUNCTION(BlueprintCallable)
    void NewFiducialMeasurement(UARTrackedFiducial* fiducial);
    
    /**
     * Scan mode: returns average transform for a fiducial based on
     * previous measurements
     */
    UFUNCTION(BlueprintCallable)
    FTransform GetAverageMeasurement(FString fiducialName) const;
    
    /**
     * Scan mode: returns array of measured fiducials names
     */
    UFUNCTION(BlueprintCallable)
    TArray<FString> GetMeasuredFiducials() const;
    
protected:
    // Called when the game starts
    virtual void BeginPlay() override;
    
private:
    FARSessionStatus lastArSessionStatus_;
    
    std::vector<UARTrackedFiducial*> activeFiducialsList_;
    
    UPROPERTY()
    TArray<UARTrackedFiducial*> activeFiducialsArray_; // for keeping UE4 garbage collector happy
    UPROPERTY()
    TMap<FString, UARTrackedFiducial*> activeFiducialsDict_;
    
    std::map<std::string, std::vector<FTrackedImageSnapshot>> measurements_;
    
    void UpdateActiveFiducials();
    void UpdatePawnEstimate();
    
    AFAnchor* getFanchorWithName(std::string fanchorName);
};
