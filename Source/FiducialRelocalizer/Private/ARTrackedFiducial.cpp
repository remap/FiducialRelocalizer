// Fill out your copyright notice in the Description page of Project Settings.


#include "ARTrackedFiducial.h"
#include "Kismet/KismetMathLibrary.h"
#include "FiducialRelocalizer.h"

FTrackedImageSnapshot FTrackedImageSnapshot::snap(UARTrackedImage* image)
{
    FTrackedImageSnapshot snapshot;
    
    snapshot.transform_ = image->GetLocalToTrackingTransform_NoAlignment();
    snapshot.frameNo_ = image->GetLastUpdateFrameNumber();
    snapshot.timestamp_ = image->GetLastUpdateTimestamp();
    
    return snapshot;
}


UARTrackedFiducial::UARTrackedFiducial(const FObjectInitializer& objectInitializer)
: UObject(objectInitializer)
{
    initTimestamp_ = FDateTime::MaxValue();
}

UARTrackedFiducial::~UARTrackedFiducial()
{
}

FTimespan
UARTrackedFiducial::getAge() const
{
    return FDateTime::Now() - initTimestamp_;
}

float
UARTrackedFiducial::getFovCoverage() const
{
    return 0;
}

FString
UARTrackedFiducial::getName() const
{
    return name_;
}

void
UARTrackedFiducial::init(UARTrackedImage* trackedImage, AFAnchor* fanchor)
{
    prevTrackingState_ = EARTrackingState::Unknown;
    curTrackingState_ = EARTrackingState::Unknown;
    name_ = trackedImage->GetDetectedImage()->GetFriendlyName();
    fiducialAnchor_ = fanchor;
    isLastUpdateSignificant_ = true;
    initTimestamp_ = FDateTime::Now();
    lastUsed_ = FDateTime::Now();
    
    update(trackedImage);
}

void
UARTrackedFiducial::pin()
{
    // TODO: do something about pinning (do we need it at all)
    // code below doesn't work (fails to pin)

//    pin_ = UARBlueprintLibrary::PinComponent(nullptr,
//                                             trackedImage_->GetLocalToWorldTransform(),
//                                             trackedImage_,
//                                             FName(*getName()));
//    if (pin_)
//        DLOG_MODULE_DEBUG(FiducialRelocalizer, "Pinned anchor {}",
//                      TCHAR_TO_ANSI(*getName()));
//    else
//        DLOG_MODULE_WARN(FiducialRelocalizer, "Failed to pin anchor {}",
//                         TCHAR_TO_ANSI(*getName()));
}

void
UARTrackedFiducial::update(UARTrackedImage* image)
{
    prevTrackingState_ = curTrackingState_;
    curTrackingState_ = image->GetTrackingState();
    lastSnapshot_ = FTrackedImageSnapshot::snap(image);
    name_ = image->GetDetectedImage()->GetFriendlyName();
    
    float locT, rotT, scaT;
    FFiducialRelocalizerModule::GetSharedInstance()->
        getFiducialPoseUpdateThreshold(locT, rotT, scaT);
    
    isLastUpdateSignificant_ =
        !UKismetMathLibrary::NearlyEqual_TransformTransform(lastSignificantUpdate_.transform_,
                                        lastSnapshot_.transform_,
                                        locT, rotT, scaT);
    if (isLastUpdateSignificant_)
    {
        DLOG_MODULE_DEBUG(FiducialRelocalizer, "fiducial {} updated: {} significant: {} previous: {}",
               TCHAR_TO_ANSI(*getName()),
               TCHAR_TO_ANSI(*lastSnapshot_.transform_.ToString()),
               (isLastUpdateSignificant_ ? "YES" : "NO"),
               TCHAR_TO_ANSI(*lastSignificantUpdate_.transform_.ToString()));
        
        lastSignificantUpdate_ = lastSnapshot_;
    }
}

bool
UARTrackedFiducial::getIsPoseUpdateSignificant() const
{
    return isLastUpdateSignificant_;
}

float
UARTrackedFiducial::getTimeSinceLastSignificantUpdate() const
{
    return lastSnapshot_.timestamp_ - lastSignificantUpdate_.timestamp_;
}

int32
UARTrackedFiducial::getFramesSinceLastSignificantUpdate() const
{
    return lastSnapshot_.frameNo_ - lastSignificantUpdate_.frameNo_;
}

void
UARTrackedFiducial::onArAlignmentUpdated()
{
}

void
UARTrackedFiducial::setLastUsedTimestamp(FDateTime ts)
{
    lastUsed_ = ts;
}

float
UARTrackedFiducial::timeSinceLastUsed()
{
    return (FDateTime::Now() - lastUsed_).GetTotalMilliseconds();
}
