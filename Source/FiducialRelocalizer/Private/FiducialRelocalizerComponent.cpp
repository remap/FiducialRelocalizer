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
    
    for (auto it:activeFiducialsDict_)
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
        
        activeFiducialsDict_[trackedFiducial->getName()] = trackedFiducial;
        activeFiducialsList_.push_back(trackedFiducial);
        
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
    
        activeFiducialsList_.erase(find(activeFiducialsList_.begin(),
                                  activeFiducialsList_.end(),
                                  fiducial));
        activeFiducialsDict_.erase(fiducial->getName());
    }
}

void
UFiducialRelocalizerComponent::PickEstimationFiducials()
{
    // building max heap
    make_heap(activeFiducialsList_.begin(), activeFiducialsList_.end(),
              [](UARTrackedFiducial* f1, UARTrackedFiducial* f2) {
        // f1 < f2 comparison
        return ! (f1->getTimeSinceLastSignificantUpdate() < f2->getTimeSinceLastSignificantUpdate());
    });
    
    EstimationFiducials.Empty();
    
    // pick fiducials for estimation
    for (int i = 0; i < activeFiducialsList_.size(); ++i)
    {
        auto f = activeFiducialsList_.front();
        if (f->getCurrentTrackingState() == EARTrackingState::Tracking)
        {
            EstimationFiducials.Add(f);
            f->setLastUsedTimestamp(FDateTime::Now());
        }
        pop_heap(activeFiducialsList_.begin(), activeFiducialsList_.end()-i);
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
    set<FString> oldFiducials;
    vector<UARTrackedFiducial*> updatedFiducials;
    
    {
        set<FString> trackedImageNames;
        for (UARTrackedImage* img : trackedImages)
        {
            if (!img->GetDetectedImage())
                DLOG_MODULE_ERROR(FiducialRelocalizer, "img->GetDetectedImage() returned NULL");
            else
            {
                trackedImageNames.insert(img->GetDetectedImage()->GetFriendlyName());
                DLOG_MODULE_TRACE(FiducialRelocalizer, "tracked image name {} state {}",
                                  TCHAR_TO_ANSI(*img->GetDetectedImage()->GetFriendlyName()),
                                  img->GetTrackingState());
            }
        }
        
        set<FString> activeFiducialsNames;
        transform(activeFiducialsList_.begin(), activeFiducialsList_.end(),
                  inserter(activeFiducialsNames, activeFiducialsNames.begin()),
                  [](const UARTrackedFiducial* f){ return f->getName(); });
        
        for (auto &n : activeFiducialsNames)
            DLOG_MODULE_TRACE(FiducialRelocalizer, "active fiducial name {}",
                              TCHAR_TO_ANSI(*n));
        
        // set old fiducials -- ones that are not present in currently tracked images
        set_difference(activeFiducialsNames.begin(), activeFiducialsNames.end(),
                       trackedImageNames.begin(), trackedImageNames.end(),
                       inserter(oldFiducials, oldFiducials.begin()));
    }
    
    for (auto& trackedImage : trackedImages)
    {
        if (!isValidTrackedImage(trackedImage))
            continue;
        
        // update existing
        if (!trackedImage->GetDetectedImage())
        {
            DLOG_MODULE_ERROR(FiducialRelocalizer, "img->GetDetectedImage() returned NULL. continue loop");
            continue;
        }
        assert(trackedImage->GetDetectedImage());

        FString imgName = trackedImage->GetDetectedImage()->GetFriendlyName();
        bool isTracking = !(trackedImage->GetTrackingState() == EARTrackingState::StoppedTracking ||
                           trackedImage->GetTrackingState() == EARTrackingState::Unknown);
        bool isActive = (activeFiducialsDict_.find(imgName) != activeFiducialsDict_.end());
        
        // 0. add unsuable fiducials to oldFiducials
        if (!isTracking)
        {
            if (isActive)
            {
                DLOG_MODULE_TRACE(FiducialRelocalizer, "fiducial {} got non-tracking state", TCHAR_TO_ANSI(*imgName));
                
                oldFiducials.insert(imgName);
            }
        }
        else
        {
            // 1. update existing fiducial
            if (isActive)
            {
                UARTrackedFiducial *trackedFiducial = activeFiducialsDict_[imgName];
                
                trackedFiducial->update(trackedImage);
                
                if (trackedFiducial->hasTrackingStateUpdated())
                    updatedFiducials.push_back(trackedFiducial);
            }
            else // 2. add new fiducial
            {
                UARTrackedFiducial *f = AddNewFiducial(trackedImage, getFanchorWithName(imgName));
                
                if (f)
                    updatedFiducials.push_back(f);
            }
        }
    }
    
    // remove old fiducials
    // TODO: notify about removed fiducials
    if (oldFiducials.size())
    {
        DLOG_MODULE_WARN(FiducialRelocalizer, "{} old fiducials will be deleted",
                          oldFiducials.size());
        
        // remove fiducials that are not tracked anymore
        for (const FString& imgName : oldFiducials)
        {
            if (activeFiducialsDict_.find(imgName) != activeFiducialsDict_.end())
                RemoveFiducial(activeFiducialsDict_.at(imgName));
            else
            {
                DLOG_MODULE_ERROR(FiducialRelocalizer, "DATA VIOLATION: old fiducial {} not found in active fiducials map",
                                  TCHAR_TO_ANSI(*imgName));
            }
        }
    }
    
    // notify about fiducials with updated tracking status
    for (auto &f : updatedFiducials)
        OnFiducialTrackingStateChanged.Broadcast(f);
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

void
UFiducialRelocalizerComponent::NewFiducialMeasurement(UARTrackedFiducial* fiducial)
{
    if (measurements_.find(fiducial->getName()) == measurements_.end())
        measurements_[fiducial->getName()] = vector<FTrackedImageSnapshot>();
    
    measurements_[fiducial->getName()].push_back(fiducial->getSnapshot());
}

FTransform
UFiducialRelocalizerComponent::GetAverageMeasurement(FString fiducialName) const
{
    FTransform t;
    
    if (measurements_.find(fiducialName) != measurements_.end())
    {
        size_t len = measurements_.at(fiducialName).size();
        if (len > 0)
        {
            FVector avgLocation = FVector::ZeroVector;
            FQuat avgQuat = FQuat::Identity;
            
            for (auto &s : measurements_.at(fiducialName))
            {
                avgLocation += s.transform_.GetLocation();
                avgQuat += s.transform_.GetRotation();
            }
            
            avgLocation *= 1/float(len);
            avgQuat *= 1/float(len);
            
            t.SetLocation(avgLocation);
            t.SetRotation(avgQuat);
        }
    }
    
    return t;
}

TArray<FString>
UFiducialRelocalizerComponent::GetMeasuredFiducials() const
{
    TArray<FString> arr;
    for (auto const& it:measurements_)
        arr.Add(it.first);

    return arr;
}
