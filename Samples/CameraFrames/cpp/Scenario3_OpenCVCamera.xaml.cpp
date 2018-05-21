//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#include "pch.h"
#include "Scenario3_OpenCVCamera.xaml.h"
#include "FrameRenderer.h"

using namespace SDKTemplate;

using namespace concurrency;


using namespace Windows::Graphics::Imaging;
using namespace Windows::Media::Capture;
using namespace Windows::Media::Capture::Frames;
using namespace Windows::Media::MediaProperties;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Navigation;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Media::Capture;
using namespace Windows::Media::Capture::Frames;

using namespace Platform;
using namespace Platform::Collections;

Scenario3_OpenCVCamera::Scenario3_OpenCVCamera()
{
	InitializeComponent();

	m_previewRenderer = ref new FrameRenderer(PreviewImage);
	m_outputRenderer = ref new FrameRenderer(OutputImage);
	m_helper = ref new OpenCVHelper();

	m_FPSTimer = ref new DispatcherTimer();

}

void Scenario3_OpenCVCamera::OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e)
{
	// setting up the combobox, and default operation
	// Get the formats supported by the selected source into the FormatComboBox.
	auto formats = ref new Vector<OperationType>();
	formats->Append(OperationType::Blur);
	formats->Append(OperationType::Contours);
	formats->Append(OperationType::MotionDetector);

	OperationComboBox->ItemsSource = formats;
	OperationComboBox->SelectedIndex = 0;
	m_currentOperation = OperationType::Blur;

	//return CleanupMediaCaptureAsync()
	//	.then([this]()
	//{
	//	return create_task(MediaFrameSourceGroup::FindAllAsync());
	//}, task_continuation_context::get_current_winrt_context())
	//	.then([this](IVectorView<MediaFrameSourceGroup^>^ allGroups)
	//{}
	//IVectorView<MediaFrameSourceGroup^>^ allGroups = MediaFrameSourceGroup::FindAllAsync();

	//if (allGroups->Size == 0)
	//{
	//	return;
	//}

	//// Pick next group in the array after each time the Next button is clicked.
	//int m_selectedSourceGroupIndex = (m_selectedSourceGroupIndex + 1) % allGroups->Size;
	//MediaFrameSourceGroup^ selectedGroup = allGroups->GetAt(m_selectedSourceGroupIndex);

	//InitializeMediaCaptureAsync(selectedGroup);
}

void Scenario3_OpenCVCamera::OnNavigatedFrom(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e)
{
	//CleanupMediaCaptureAsync();
}

task<bool> Scenario3_OpenCVCamera::InitializeMediaCaptureAsync(MediaFrameSourceGroup^ sourceGroup)
{
	if (m_mediaCapture != nullptr)
	{
		return task_from_result(true);
	}

	m_mediaCapture = ref new MediaCapture();
	auto settings = ref new MediaCaptureInitializationSettings();

	// Select the source we will be reading from.
	settings->SourceGroup = sourceGroup;

	// This media capture has exclusive control of the source.
	settings->SharingMode = MediaCaptureSharingMode::ExclusiveControl;

	// Set to CPU to ensure frames always contain CPU SoftwareBitmap images,
	// instead of preferring GPU D3DSurface images.
	settings->MemoryPreference = MediaCaptureMemoryPreference::Cpu;

	// Capture only video. Audio device will not be initialized.
	settings->StreamingCaptureMode = StreamingCaptureMode::Video;

	create_task(m_mediaCapture->InitializeAsync(settings));
}

task<void> Scenario3_OpenCVCamera::CleanupMediaCaptureAsync()
{
	task<void> cleanupTask = task_from_result();

	if (m_mediaCapture != nullptr)
	{
		cleanupTask = cleanupTask && create_task(m_reader->StopAsync());
		m_mediaCapture = nullptr;
	}
	return cleanupTask;
}

void Scenario3_OpenCVCamera::OperationComboBox_SelectionChanged(Object^ sender, RoutedEventArgs^ e)
{
	auto currentOperation = static_cast<OperationType>(OperationComboBox->SelectedItem);

	if (OperationType::Blur == currentOperation)
	{
		CurrentOperationTextBlock->Text = "Current: Blur";
	}
	else if (OperationType::Contours == currentOperation)
	{
		CurrentOperationTextBlock->Text = "Current: Contours";
	}
	else if (OperationType::Histogram == currentOperation)
	{
		CurrentOperationTextBlock->Text = "Current: Histogram of RGB channels";
	}
	else if (OperationType::HoughLines == currentOperation)
	{
		CurrentOperationTextBlock->Text = "Current: Line detection";
	}
	else if (OperationType::MotionDetector == currentOperation)
	{
		CurrentOperationTextBlock->Text = "Current: Motion detection";
	}
	else
	{
		CurrentOperationTextBlock->Text = "";
	}
}
