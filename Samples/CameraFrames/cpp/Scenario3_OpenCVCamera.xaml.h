//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the Microsoft Public License.
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************
#define OPENCVHELPER_H

#pragma once
#include "Scenario3_OpenCVCamera.g.h"
#include "MainPage.xaml.h"
#include "FrameRenderer.h"
#include "OpenCVHelper.h"

using namespace Windows::UI::Xaml;
using namespace Windows::Media::Capture;
using namespace Windows::Media::Capture::Frames;
using namespace OpenCVBridge;

namespace SDKTemplate
{
	[Windows::Foundation::Metadata::WebHostHidden]
	public ref class Scenario3_OpenCVCamera sealed
	{
	public:
		Scenario3_OpenCVCamera();
		void OperationComboBox_SelectionChanged(Object^ sender, RoutedEventArgs^ e);

	protected:
		virtual void OnNavigatedTo(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e) override;

		virtual void OnNavigatedFrom(Windows::UI::Xaml::Navigation::NavigationEventArgs^ e) override;
	private:
		//concurrency::task<void> InitializeMediaCaptureAsync(MediaFrameSourceGroup^ sourceGroup);
		//concurrency::task<void> CleanupMediaCaptureAsync();

	private:
		MainPage ^ rootPage = MainPage::Current;

		MediaCapture^ m_mediaCapture;
		MediaFrameReader^ m_reader;
		FrameRenderer^ m_previewRenderer;
		FrameRenderer^ m_outputRenderer;

		//int m_frameCount = 0;

		//const int IMAGE_ROWS = 480;
		//const int IMAGE_COLS = 640;

		OpenCVHelper^ m_helper;
		DispatcherTimer^ m_FPSTimer;
		enum class OperationType
		{
			Blur = 0,
			HoughLines,
			Contours,
			Histogram,
			MotionDetector
		};
		OperationType m_currentOperation;
	};
} // SDKTemplate
