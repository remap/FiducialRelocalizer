// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ARTrackable.h"
#include "FAnchor.h"

#include "ARTrackedFiducial.generated.h"

USTRUCT(Blueprintable)
struct FTrackedImageSnapshot {
    GENERATED_BODY()
    
    UPROPERTY(BlueprintReadWrite)
    FTransform transform_;
    
    UPROPERTY(BlueprintReadWrite)
    int32 frameNo_;
    
    UPROPERTY(BlueprintReadWrite)
    float timestamp_;
    
    static FTrackedImageSnapshot snap(UARTrackedImage* image);
};

UCLASS(Blueprintable)
class UARTrackedFiducial : public UObject
{
    GENERATED_BODY()

public:
    UARTrackedFiducial(const FObjectInitializer& objectInitializer);
	~UARTrackedFiducial();
    
    UFUNCTION(BlueprintCallable)
    UARTrackedImage* getTrackedImage() const { return trackedImage_; }
    
    UFUNCTION(BlueprintCallable)
    AFAnchor* getFiducialAnchor() const { return fiducialAnchor_; }
    
    UFUNCTION(BlueprintCallable)
    FTrackedImageSnapshot getSnapshot() const { return lastSnapshot_; }
    
    UFUNCTION(BlueprintCallable)
    FString getName() const;
    
    UFUNCTION(BlueprintCallable)
    FTimespan getAge() const;
    
    UFUNCTION(BlueprintCallable)
    float getFovCoverage() const;
    
    UFUNCTION(BlueprintCallable)
    void init(UARTrackedImage* trackedImage, AFAnchor* fanchor);
    
    UFUNCTION(BlueprintCallable)
    void update(UARTrackedImage* image);
    
    UFUNCTION(BlueprintCallable)
    bool getIsPoseUpdateSignificant() const;
    
    UFUNCTION(BlueprintCallable)
    float getTimeSinceLastSignificantUpdate() const;
    
    UFUNCTION(BlueprintCallable)
    int32 getFramesSinceLastSignificantUpdate() const;
    
    UFUNCTION(BlueprintCallable)
    void onArAlignmentUpdated();
    
    UFUNCTION(BlueprintCallable)
    float timeSinceLastUsed();
    
    void setLastUsedTimestamp(FDateTime ts);
    
private:
    FDateTime initTimestamp_, lastUsed_;
    bool isLastUpdateSignificant_;
    
    UARTrackedImage* trackedImage_;
    AFAnchor* fiducialAnchor_;
    FTrackedImageSnapshot lastSnapshot_, lastSignificantUpdate_;
};
