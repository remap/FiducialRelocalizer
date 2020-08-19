// Fill out your copyright notice in the Description page of Project Settings.


#include "FAnchor.h"
#include "DDLog.h"
#include "FiducialRelocalizer.h"

// Sets default values
AFAnchor::AFAnchor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
    Alignment = FanchorAlignment::Unaligned;
}

// Called when the game starts or when spawned
void AFAnchor::BeginPlay()
{
	Super::BeginPlay();
    
    Tags.Add(FName(*FiducialName));
}

// Called every frame
void AFAnchor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

FTransform AFAnchor::getAlignedTransform() const
{
    return getTransformAligned(GetTransform(), Alignment);
}

FTransform AFAnchor::getTransformAligned(FTransform t, FanchorAlignment alignment)
{
    FTransform aligned = t;
    FRotator rotator = t.Rotator();
    
    if (alignment == FanchorAlignment::Horizontal)
    {
        rotator.Pitch = 0;
        rotator.Roll = 0;
    }
    
    if (alignment == FanchorAlignment::Vertical)
    {
        rotator.Pitch = 90;
    }
    
    aligned.SetRotation(FQuat(rotator));
    
    return aligned;
}
