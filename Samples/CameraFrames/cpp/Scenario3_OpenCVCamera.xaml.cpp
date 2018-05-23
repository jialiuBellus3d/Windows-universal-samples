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
using namespace Windows::Media::MediaProperties;

using namespace Platform;
using namespace Platform::Collections;

Scenario3_OpenCVCamera::Scenario3_OpenCVCamera()
{
	InitializeComponent();

	m_previewRenderer = ref new FrameRenderer(PreviewImage);
	m_outputRenderer = ref new FrameRenderer(OutputImage);
	m_helper = ref new OpenCVHelper();

	m_FPSTimer = ref new DispatcherTimer();
	m_logger = ref new SimpleLogger(outputTextBlock);
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
	CleanupMediaCaptureAsync().then([this]()
	{
		return create_task(MediaFrameSourceGroup::FindAllAsync());
	}, task_continuation_context::get_current_winrt_context())
		.then([this](IVectorView<MediaFrameSourceGroup^>^ allGroups)
	{
		if (allGroups->Size == 0)
		{
			m_logger->Log("No source groups found.");
			return;
		}

		// Pick next group in the array after each time the Next button is clicked.
		int m_selectedSourceGroupIndex = 0;
		m_selectedSourceGroupIndex = (m_selectedSourceGroupIndex + 1) % allGroups->Size;
		MediaFrameSourceGroup^ selectedGroup = allGroups->GetAt(m_selectedSourceGroupIndex);

		m_logger->Log("Found " + allGroups->Size.ToString() + " groups and " +
			"selecting index [" + m_selectedSourceGroupIndex.ToString() + "] : " +
			selectedGroup->DisplayName + "   " + selectedGroup->Id);
		
		InitializeMediaCaptureAsync(selectedGroup).then([this]() {
			m_logger->Log("InitializeMediaCaptureAsync done");
			// Create the frame reader
			MediaFrameSource^ frameSource;
			IMapView<String^, MediaFrameSource^>^ frameSources = nullptr;
			frameSources = m_mediaCapture->FrameSources;
			if (frameSources == nullptr) {
				m_logger->Log("kvp is null");
			}
			else {
				m_logger->Log(frameSources->Size.ToString());
				for (auto kvp : frameSources)
				{
					if (kvp != nullptr) {
						m_logger->Log(kvp->Key);
						frameSource = kvp->Value;
					}
				}
			}

			BitmapSize bitmapSize = BitmapSize();
			bitmapSize.Height = IMAGE_ROWS;
			bitmapSize.Width = IMAGE_COLS;

			m_logger->Log("before CreateFrameReaderAsync");

			create_task(m_mediaCapture->CreateFrameReaderAsync(frameSource, MediaEncodingSubtypes::Bgra8, bitmapSize))
				.then([this](MediaFrameReader^ mediaFrameReader)
			{
				m_reader = mediaFrameReader;
				m_logger->Log("CreateFrameReaderAsync done");
				//m_reader->FrameArrived += ColorFrameReader_FrameArrivedAsync;
				/*create_task(ColorFrameReader_FrameArrivedAsync(m_reader))
					.then([this]() {
					m_reader->StartAsync();
				}, task_continuation_context::get_current_winrt_context());*/
			}, task_continuation_context::get_current_winrt_context());
		}, task_continuation_context::get_current_winrt_context());
	}, task_continuation_context::get_current_winrt_context());
}

void Scenario3_OpenCVCamera::OnNavigatedFrom(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e)
{
	CleanupMediaCaptureAsync();
}

task<void> Scenario3_OpenCVCamera::InitializeMediaCaptureAsync(MediaFrameSourceGroup^ sourceGroup)
{
	if (m_mediaCapture != nullptr)
	{
		m_logger->Log("m_mediaCapture not null, last capture not finish yet");
		return CleanupMediaCaptureAsync();
	}
	else {
		m_logger->Log("m_mediaCapture is null, start new capture");
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

	m_logger->Log("before InitializeAsync");
	return create_task(m_mediaCapture->InitializeAsync(settings));
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

task<void> Scenario3_OpenCVCamera::ColorFrameReader_FrameArrivedAsync(MediaFrameReader^ sender)
{
	auto frame = sender->TryAcquireLatestFrame();
	if (frame != nullptr)
	{
		SoftwareBitmap^ originalBitmap = nullptr;
		auto inputBitmap = frame->VideoMediaFrame;
		if (inputBitmap != nullptr)
		{
			// The XAML Image control can only display images in BRGA8 format with premultiplied or no alpha
			// The frame reader as configured in this sample gives BGRA8 with straight alpha, so need to convert it
			//originalBitmap = SoftwareBitmap::Convert(inputBitmap, BitmapPixelFormat::Bgra8, BitmapAlphaMode::Premultiplied);

			SoftwareBitmap^ outputBitmap = ref new SoftwareBitmap(BitmapPixelFormat::Bgra8, originalBitmap->PixelWidth, 
				originalBitmap->PixelHeight, BitmapAlphaMode::Premultiplied);

	//		// Operate on the image in the manner chosen by the user.
			if (m_currentOperation == OperationType::Blur)
			{
				m_helper->Blur(originalBitmap, outputBitmap);
			}
			else if (m_currentOperation == OperationType::HoughLines)
			{
				m_helper->HoughLines(originalBitmap, outputBitmap);
			}
			else if (m_currentOperation == OperationType::Contours)
			{
				m_helper->Contours(originalBitmap, outputBitmap);
			}
			else if (m_currentOperation == OperationType::Histogram)
			{
				m_helper->Histogram(originalBitmap, outputBitmap);
			}
			else if (m_currentOperation == OperationType::MotionDetector)
			{
				m_helper->MotionDetector(originalBitmap, outputBitmap);
			}

			// Display both the original bitmap and the processed bitmap.
			m_previewRenderer->BufferBitmapForRendering(originalBitmap);
			m_outputRenderer->BufferBitmapForRendering(outputBitmap);
		}

	//	Interlocked.Increment(ref _frameCount);
	}
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
