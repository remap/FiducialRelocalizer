//
// FiducialRelocalizer.cpp
//
//  Generated on March 10 2020
//  Template created by Peter Gusev on 27 January 2020.
//  Copyright 2013-2019 Regents of the University of California
//

#include "FiducialRelocalizer.h"
#include "logging.hpp"
#include "git-describe.h"

#define STRINGIZE_VERSION(v) STRINGIZE_TOKEN(v)
#define STRINGIZE_TOKEN(t) #t
#define PLUGIN_VERSION STRINGIZE_VERSION(GIT_DESCRIBE)

#define MODULE_NAME "FiducialRelocalizer"
#define LOCTEXT_NAMESPACE "FFiducialRelocalizerModule"

FFiducialRelocalizerModule* FFiducialRelocalizerModule::sharedInstance_ = nullptr;

void FFiducialRelocalizerModule::StartupModule()
{
    initModule(MODULE_NAME, PLUGIN_VERSION);
    FFiducialRelocalizerModule::sharedInstance_ = this;
 
    locThreshold_ = 3.; // location tolerance -- centimeters
    rotThreshold_ = 5.; // quaternion tolerance -- ???
    scaleThreshold_ = 0.1; // scale tolerance, i.e. 10%
    
    // To log using ReLog plugin, use these macro definitions:
    // DLOG_PLUGIN_ERROR("Error message");
    // DLOG_PLUGIN_WARN("Warning message");
    // DLOG_PLUGIN_INFO("Info message");
    // DLOG_PLUGIN_DEBUG("Debug message");
    // DLOG_PLUGIN_TRACE("Trace message");

    // This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
}

void FFiducialRelocalizerModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

void
FFiducialRelocalizerModule::setFiducialPoseUpdateThreshold(float locTh, float rotTh,
                                                           float scaleTh)
{
    locThreshold_ = locTh;
    rotThreshold_ = rotTh;
    scaleThreshold_ = scaleTh;
}

void
FFiducialRelocalizerModule::getFiducialPoseUpdateThreshold(float& locTh, float& rotTh,
                                                           float& scaleTh) const
{
    locTh = locThreshold_;
    rotTh = rotThreshold_;
    scaleTh = scaleThreshold_;
}

FFiducialRelocalizerModule* FFiducialRelocalizerModule::GetSharedInstance()
{
    return FFiducialRelocalizerModule::sharedInstance_;
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FFiducialRelocalizerModule, FiducialRelocalizer)
