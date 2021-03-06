// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FAnchor.generated.h"

UENUM(BlueprintType)
enum class FanchorAlignment : uint8  {
    Unaligned UMETA(DisplayNAme = "Unaligned"),
    Vertical UMETA(DisplayNAme = "Vertical"),
    Horizontal UMETA(DisplayNAme = "Horizontal"),
};

UCLASS()
class FIDUCIALRELOCALIZER_API AFAnchor : public AActor
{
	GENERATED_BODY()
	
public:
    static FTransform getTransformAligned(FTransform t, FanchorAlignment alignment);
    
	// Sets default values for this actor's properties
	AFAnchor();

    UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
    FString FiducialName;
    
    // specified which levels to stream in when this fiducial is detected
    UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
    TArray<FName> StreamingLevels;
    
    // specifies whether all other currently loaded levels must be unloaded first
    UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
    bool IsExclusive;
    
    UPROPERTY(EditInstanceOnly, BlueprintReadWrite)
    bool IsActive;
    
    UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
    FanchorAlignment Alignment;
    
    // returns Actor's transform aligned by the anchor's alignment
    UFUNCTION(BlueprintCallable)
    FTransform getAlignedTransform() const;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
