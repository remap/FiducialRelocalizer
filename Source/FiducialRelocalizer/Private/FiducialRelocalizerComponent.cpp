// Fill out your copyright notice in the Description page of Project Settings.


#include "FiducialRelocalizerComponent.h"

using namespace std;

// Sets default values for this component's properties
UFiducialRelocalizerComponent::UFiducialRelocalizerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UFiducialRelocalizerComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

// Called every frame
void UFiducialRelocalizerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}


UARTrackedFiducial*
UFiducialRelocalizerComponent::AddNewFiducial(UARTrackedImage* trackedImage, AFAnchor* fanchor)
{
    if (trackedImage)
    {
        UARTrackedFiducial* trackedFiducial = NewObject<UARTrackedFiducial>(this);
        
        trackedFiducial->init(trackedImage, fanchor);
        ActiveFiducials.Add(trackedFiducial->getName(), trackedFiducial);
        fiducialsList_.push_back(trackedFiducial);
        
        DLOG_MODULE_DEBUG(FiducialRelocalizer, "Added fiducial {} ", TCHAR_TO_ANSI(*trackedFiducial->getName()));
        
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
    
        ActiveFiducials.Remove(fiducial->getName());
        fiducialsList_.erase(find(fiducialsList_.begin(),
                                  fiducialsList_.end(),
                                  fiducial));
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
