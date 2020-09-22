// Fill out your copyright notice in the Description page of Project Settings.


#include "FiducialRelocalizerComponent.h"
#include <set>
#include <algorithm>
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

using namespace std;

// Sets default values for this component's properties
UFiducialRelocalizerComponent::UFiducialRelocalizerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

    isScanMode = false;
}


// Called when the game starts
void UFiducialRelocalizerComponent::BeginPlay()
{
	Super::BeginPlay();
 
	controller = Cast<AARBasePlayerController>(UGameplayStatics::GetPlayerController(GetWorld(), 0));
    assert(controller);
}

// Called every frame
void UFiducialRelocalizerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    FARSessionStatus sessionStatus = UARBlueprintLibrary::GetARSessionStatus();
    
    
    switch (sessionStatus.Status)
    {
        case EARSessionStatus::Running:
        {
            UpdateActiveFiducials();
            PickEstimationFiducials();
            UpdatePawnEstimate();
        }
            break;
        default:
            // call go back to limbo
            if (lastArSessionStatus_.Status != sessionStatus.Status)
                OnGoBackToLimbo.Broadcast();
            break;
    }
    
    lastArSessionStatus_ = sessionStatus;
}

TMap<FString, UARTrackedFiducial*>
UFiducialRelocalizerComponent::GetActiveFiducials() const
{
    TMap<FString, UARTrackedFiducial*> activeFiducials;
    
    for (auto it:activeFiducials_)
        activeFiducials.Add(it.first, it.second);
    
    return activeFiducials;
}

UARTrackedFiducial*
UFiducialRelocalizerComponent::AddNewFiducial(UARTrackedImage* trackedImage, AFAnchor* fanchor)
{
    if (trackedImage)
    {
        UARTrackedFiducial* trackedFiducial = NewObject<UARTrackedFiducial>(this);
        
        trackedFiducial->init(trackedImage, fanchor);
        trackedFiducial->pin();
        
        activeFiducials_[trackedFiducial->getName()] = trackedFiducial;
        fiducialsList_.push_back(trackedFiducial);
        
        DLOG_MODULE_DEBUG(FiducialRelocalizer, "Added fiducial {}. Has FAnchor: {}",
                          TCHAR_TO_ANSI(*trackedFiducial->getName()),
                          (fanchor ? "YES" : "NO"));
        
        return trackedFiducial;
    }
    
    return nullptr;
}

void
UFiducialRelocalizerComponent::RemoveFiducial(UARTrackedFiducial* fiducial)
{
    if (fiducial)
    {
        DLOG_MODULE_DEBUG(FiducialRelocalizer, "Remove fiducial {} ", TCHAR_TO_ANSI(*fiducial->getName()));
    
        fiducialsList_.erase(find(fiducialsList_.begin(),
                                  fiducialsList_.end(),
                                  fiducial));
        activeFiducials_.erase(fiducial->getName());
    }
}

void
UFiducialRelocalizerComponent::PickEstimationFiducials()
{
    // building max heap
    make_heap(fiducialsList_.begin(), fiducialsList_.end(),
              [](UARTrackedFiducial* f1, UARTrackedFiducial* f2) {
        // f1 < f2 comparison
        return ! (f1->getTimeSinceLastSignificantUpdate() < f2->getTimeSinceLastSignificantUpdate());
    });
    
    EstimationFiducials.Empty();
    
    // pick fiducials for estimation
    for (int i = 0; i < fiducialsList_.size(); ++i)
    {
        auto f = fiducialsList_.front();
        if (f->getTrackedImage()->GetTrackingState() == EARTrackingState::Tracking)
        {
            EstimationFiducials.Add(f);
            f->setLastUsedTimestamp(FDateTime::Now());
        }
        pop_heap(fiducialsList_.begin(), fiducialsList_.end()-i);
    }
}

bool
UFiducialRelocalizerComponent::isValidTrackedImage(UARTrackedImage* trackedImage)
{
    FTransform zeroTransform;
    
    return !zeroTransform.Equals(trackedImage->GetLocalToTrackingTransform_NoAlignment());
}

void
UFiducialRelocalizerComponent::UpdateActiveFiducials()
{
    TArray<UARTrackedImage*> trackedImages = UARBlueprintLibrary::GetAllTrackedImages();
    set<FString> trackedImageNames;
    for (UARTrackedImage* img : trackedImages)
        trackedImageNames.insert(img->GetDetectedImage()->GetFriendlyName());
    
    set<FString> activeImageNames;
    transform(fiducialsList_.begin(), fiducialsList_.end(),
              inserter(activeImageNames, activeImageNames.begin()),
              [](const UARTrackedFiducial* f){ return f->getName(); });
    
    set<FString> newFiducials;
    set_difference(trackedImageNames.begin(), trackedImageNames.end(),
                   activeImageNames.begin(), activeImageNames.end(),
                   inserter(newFiducials, newFiducials.begin()));
    set<FString> oldFiducials;
    set_difference(activeImageNames.begin(), activeImageNames.end(),
                   trackedImageNames.begin(), trackedImageNames.end(),
                   inserter(oldFiducials, oldFiducials.begin()));
    
    // remove old fiducials:
    //  -- add fiducials that won't be tracked again
    for (auto &f : fiducialsList_)
        if (f->getTrackedImage()->GetTrackingState() == EARTrackingState::StoppedTracking ||
            f->getTrackedImage()->GetTrackingState() == EARTrackingState::Unknown)
        {
            DLOG_MODULE_TRACE(FiducialRelocalizer,
                              "fiducial {} has invalid tracking state - {}",
                              TCHAR_TO_ANSI(*f->getName()),
                              f->getTrackedImage()->GetTrackingState());

            oldFiducials.insert(f->getName());
        }
    
    if (oldFiducials.size())
    {
        DLOG_MODULE_DEBUG(FiducialRelocalizer, "{} old fiducials will be deleted",
                          oldFiducials.size());
    
        // remove fiducials that are not tracked anymore
        for (const FString& imgName : oldFiducials)
            RemoveFiducial(activeFiducials_.at(imgName));
    }
    
    // update active fiducials and add new ones
    set_difference(activeImageNames.begin(), activeImageNames.end(),
                   oldFiducials.begin(), oldFiducials.end(),
                   inserter(activeImageNames, activeImageNames.begin()));
    for (auto& trackedImage : trackedImages)
    {
        if (!isValidTrackedImage(trackedImage))
            continue;
        
        // update existing
        FString imgName = trackedImage->GetDetectedImage()->GetFriendlyName();
        if (activeImageNames.find(imgName) != activeImageNames.end())
        {
            assert(activeFiducials_.find(imgName) != activeFiducials_.end());
            
            UARTrackedFiducial *trackedFiducial = activeFiducials_[imgName];
            
            assert(trackedFiducial->getTrackedImage()->GetTrackingState() != EARTrackingState::EARTrackingState::StoppedTracking);
            assert(trackedFiducial->getTrackedImage()->GetTrackingState() != EARTrackingState::EARTrackingState::Unknown);
            
            trackedFiducial->update(trackedImage);
        }
        
        // add new
        if (newFiducials.find(imgName) != newFiducials.end())
        {
            AddNewFiducial(trackedImage, getFanchorWithName(imgName));
        }
    }
}

void
UFiducialRelocalizerComponent::UpdatePawnEstimate()
{
    if (EstimationFiducials.Num() && !isScanMode)
    {
        // TODO: estimate based on N fiducials
        UARTrackedFiducial *fiducial = EstimationFiducials[0];
        if (fiducial->getIsPoseUpdateSignificant())
        {
            FanchorAlignment alignment = FanchorAlignment::Unaligned;
            if (fiducial->getFiducialAnchor())
                alignment = fiducial->getFiducialAnchor()->Alignment;
            
            FTransform t = AFAnchor::getTransformAligned(fiducial->getSnapshot().transform_, alignment).Inverse();
            
//            DLOG_MODULE_DEBUG(FiducialRelocalizer, "Set AR alignment. Fanchor {} (transform {})",
//                              TCHAR_TO_ANSI(*fiducial->getName()),
//                              TCHAR_TO_ANSI(*t.ToHumanReadableString()));
//
//            controller->setArAlignment(t);
            
            if (fiducial->getFiducialAnchor())
            {
                EstimationCounter += 1;
                OnNewPawnEstimation.Broadcast(fiducial->getName(),
                                              t,
                                              fiducial->getFiducialAnchor()->getAlignedTransform(),
                                              fiducial->getFiducialAnchor());
            }
        }
    }
}

AFAnchor* UFiducialRelocalizerComponent::getFanchorWithName(FString fanchorName)
{
    FName tag(*fanchorName);
    TArray<AActor*> outActors;
    
    UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName(*fanchorName), outActors);
    
    if (outActors.Num())
    {
        if (outActors.Num() > 1)
            DLOG_MODULE_WARN(FiducialRelocalizer, "found more than one fanchor matching {}",
                             TCHAR_TO_ANSI(*fanchorName));
        return Cast<AFAnchor>(outActors[0]);
    }
    
    DLOG_MODULE_WARN(FiducialRelocalizer, "fanchor {} was not found",
                     TCHAR_TO_ANSI(*fanchorName));
    
    return nullptr;
}
