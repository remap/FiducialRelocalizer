// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ARTrackable.h"
#include "FAnchor.h"
#include "ARPin.h"

#if PLATFORM_WINDOWS

#include "AllowWindowsPlatformTypes.h"
#include "ARBlueprintLibrary.h"
#include "HideWindowsPlatformTypes.h"

#else

#include "ARBlueprintLibrary.h"

#endif

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
    void pin();
    
    UFUNCTION(BlueprintCallable)
    bool isPinned() { return pin_ != nullptr; }
    
    UFUNCTION(BlueprintCallable)
    UARPin* getPin() const { return pin_; }
    
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
    
    /**
     * Returns true if, after calling update(), there was a change in tracking state
     * of underlying augmented image marker, otherwise returns false.
     */
    UFUNCTION(BlueprintCallable)
    bool hasTrackingStateUpdated() const { return prevTrackingState_ != curTrackingState_; }
    
    /**
     * Returns tracking state as it was before calling update().
     */
    UFUNCTION(BlueprintCallable)
    EARTrackingState getPreviousTrackingState() const { return prevTrackingState_; }
    
    /***
     * Returns tracking state as it was set when calling update().
     */
    UFUNCTION(BlueprintCallable)
    EARTrackingState getCurrentTrackingState() const { return curTrackingState_; }
    
    void setLastUsedTimestamp(FDateTime ts);
    
private:
    FDateTime initTimestamp_, lastUsed_;
    bool isLastUpdateSignificant_;
    UARPin *pin_;
    
    EARTrackingState prevTrackingState_, curTrackingState_;
    UARTrackedImage* trackedImage_;
    AFAnchor* fiducialAnchor_;
    FTrackedImageSnapshot lastSnapshot_, lastSignificantUpdate_;
};
