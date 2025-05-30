// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

namespace UE::NavigationGlobals
{
	// COLLISIONS ------
	// Objects with this collision type should implement NavigationEffector interface
	inline constexpr ECollisionChannel NavigationAffectorChannel = ECollisionChannel::ECC_GameTraceChannel1;
	
	inline const FName NavigationAffectorProfileName = "NavigationAffector";

	// GRIDS ------
	
	inline constexpr uint8 MaxCost      = MAX_uint8;	// Max cost of a cell in Costs Grid
}
