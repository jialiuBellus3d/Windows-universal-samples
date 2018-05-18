// Copyright (c) Microsoft. All rights reserved.

#include "pch.h"
#include "MainPage.xaml.h"
#include "SampleConfiguration.h"

using namespace SDKTemplate;

Platform::Array<Scenario>^ MainPage::scenariosInner = ref new Platform::Array<Scenario>
{
    { "Depth Camera",   "SDKTemplate.Scenario1" },
    { "Device Camera",   "SDKTemplate.Scenario2" },
    { "Head Scanner",         "SDKTemplate.Scenario3" },
    { "Head Processor",           "SDKTemplate.Scenario4" },
};
