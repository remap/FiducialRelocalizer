// Fill out your copyright notice in the Description page of Project Settings.


#include "FiducialRelocalizerComponent.h"

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

	// ...
}


UARTrackedFiducial*
UFiducialRelocalizerComponent::AddNewFiducial(UARTrackedImage* trackedImage, AFAnchor* fanchor)
{
    if (trackedImage)
    {
        UARTrackedFiducial* trackedFiducial = NewObject<UARTrackedFiducial>(this);
        
        trackedFiducial->init(trackedImage, fanchor);
        ActiveFiducialsList.Add(trackedFiducial);
        ActiveFiducials.Add(trackedFiducial->getName(), trackedFiducial);
        
        return trackedFiducial;
    }
    
    return nullptr;
}

UARTrackedFiducial*
UFiducialRelocalizerComponent::getLatestFiducial() const
{
    if (ActiveFiducialsList.Num())
        return ActiveFiducialsList.Last();
    
    return nullptr;
}
