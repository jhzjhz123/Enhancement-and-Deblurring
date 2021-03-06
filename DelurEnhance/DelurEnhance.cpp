// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "Windows.h"
#include <iostream>
#include <vector>
#include "Non_blind_deconvolution.h"
#include "PSF_Estimation.h"
#include "Pso_Enhancement.h"


#pragma comment(lib, "opencv_world320d.lib")
#pragma warning(disable : 4996)//_CRT_SECURE_NO_WARNINGS

#define BUTTON_Load      1001
#define BUTTON_Enhance      1002
#define BUTTON_Save      1003
#define BUTTON_Deblur      1004
#define CHECKBOX_Continue 2001

using namespace std;
using namespace cv;


Mat src, value, hsv, dst,dstValue;//src(BGR) hsv(HSV) dst(BGR) 3 channel
vector<Mat> vecHSV;

//========================　ＷinAPI
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow);
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
TCHAR *openfilename(const TCHAR *filter = ((const TCHAR*)L"Mission Files (*.mmf)\0*.mmf"), HWND owner = NULL);


int main()
{
	srand(time(NULL));
	HINSTANCE hInstance = {};
	HINSTANCE hPrevInstance = {};
	PSTR szCmdLine = {};
	int iCmdShow = 1;

	WinMain(hInstance,hPrevInstance, szCmdLine, iCmdShow);
	
	

	//_wsystem(L"pause");
	return 0;
}



//========================　ＷinAPI
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HWND btnLoad, btnEnhance, btnSave, btnDeblur;
	static HWND CheckBox;
	HINSTANCE hInstance = {};
	UINT t = NULL;//check ceckbox's status
	switch (uMsg)
	{
	case WM_CREATE://Create button

		btnLoad = CreateWindow((LPCSTR)"button",(LPCSTR)"Load",WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
			100, 100, 80, 20,hwnd, (HMENU)BUTTON_Load,	hInstance, NULL);
		btnEnhance = CreateWindow((LPCSTR)"button", (LPCSTR)"Enhance", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
			200, 100, 80, 20, hwnd, (HMENU)BUTTON_Enhance, hInstance, NULL);
		btnSave = CreateWindow((LPCSTR)"button", (LPCSTR)"Save", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
			100, 150, 80, 20, hwnd, (HMENU)BUTTON_Save, hInstance, NULL);
		CheckBox = CreateWindowEx(NULL, (LPCSTR)"button", (LPCSTR)"Continuing Operation",
			WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
			100, 50, 200, 20, hwnd, (HMENU)CHECKBOX_Continue, GetModuleHandle(NULL), NULL);
		btnDeblur = CreateWindow((LPCSTR)"button", (LPCSTR)"Deblur", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON,
			200, 150, 80, 20, hwnd, (HMENU)BUTTON_Deblur, hInstance, NULL);
		break;
	case WM_COMMAND://message
		t = SendMessage(CheckBox, BM_GETCHECK, 0, 0);//check ceckbox's status
		if (LOWORD(wParam)== BUTTON_Load)//when click Load button
		{
			TCHAR   *FilePath = new TCHAR[128];//pointer should be new or given an adress(new/ TCHAR filepath[128]). Or data maybe lose	

			FilePath = openfilename(((const TCHAR*)"Source\0*.*;*.CXX\0All\0*.*\0"), hwnd);
			//_stprintf_s(FilePath, sizeof(TCHAR) * 128, _T("%s"), openfilename(_T("Source\0*.bmp;*.CXX\0All\0*.*\0"), h));
			char cPath[100] = { '/0' };
			wcstombs(cPath, (const wchar_t*)FilePath, wcslen((const wchar_t*)FilePath));//tchar 2 char
		
		
			src = imread(FilePath, IMREAD_COLOR);
			if (src.rows == 0)//exception
				return 0;

			
			cvtColor(src, hsv, COLOR_BGR2HSV);
			split(hsv, vecHSV);
			value = p.Img8U2Img32F(vecHSV[2]);
			dstValue = value.clone();
			p.ShowImg64F((char *)"src", src);
			p.ShowImg32F((char *)"value", value);
		
		
			waitKey(10);
		}
		else if (LOWORD(wParam) == BUTTON_Enhance)//when click button
		{
			if (src.rows == 0)//exception
				return 0;
			if(t)
				value = dstValue.clone();//continue mode
			value = p.Img64F2Img32F(value);

			value /= 255;
			dstValue = Run(value, -1, -1);
			//output = img.clone();
			

			
			vecHSV[2] = p.Img64F2Img8U(dstValue);//gray value result 2 hsv
			cv::merge(vecHSV, hsv);
			cvtColor(hsv, dst, COLOR_HSV2BGR);
			imshow("value", value);
			p.ShowImg32F((char *)"dstValue", dstValue);
			p.ShowImg64F((char *)"dst", dst);
			
		}
		else if (LOWORD(wParam) == BUTTON_Save)
		{
			cout<<imwrite("dst.bmp", dst);
		}
		else if (LOWORD(wParam) == BUTTON_Deblur)
		{
			if (src.rows == 0)//exception
				return 0;
			if (t)
				value = dstValue.clone();//continue mode

			Mat enhance, img, est, Kernel;

			value = p.Img32F2Img64F(value);

			est = Edge_Estimating(value);
			cout << "Estimating..." << endl;
			Kernel = Kernel_Estimating(est, value, Size(11,11));//estimate kernel
			
			Kernel_Refine(Kernel);//kernel refine
			p.ShowImg64F((char *)"Kernel", 2550 * Kernel);
			cout << "Deconvouting..." << endl;
			dstValue = Algorithm1(value, Kernel, 0.001);//deconvolution

			vecHSV[2] = p.Img64F2Img8U(dstValue);//gray value result 2 hsv
			cv::merge(vecHSV, hsv);
			cvtColor(hsv, dst, COLOR_HSV2BGR);
			p.ShowImg64F((char *)"value", value);
			p.ShowImg32F((char *)"dstValue", dstValue);
			p.ShowImg64F((char *)"dst", dst);
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hwnd, &ps);

		FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

		EndPaint(hwnd, &ps);
	}
	return 0;

	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
	//DefWindowProc Calls the default window procedure to provide default processing for any window messages that an application does not process. This function ensures that every message is processed. 
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
	static TCHAR szAppName[] = TEXT("Enhancement");

	HWND		hwnd;
	MSG			msg;
	WNDCLASS		wndclass;//Contains the window class attributes that are registered by the RegisterClass function.
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WindowProc;//handle message
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = szAppName;//connect wndclass and hwnd


	if (!RegisterClass(&wndclass))
	{
		MessageBox(NULL, TEXT("This program requires Windows NT!"), szAppName, MB_ICONERROR);
		return 0;
	}

	hwnd = CreateWindow(szAppName,	// window class name = wndclass.lpszClassName
		TEXT("The Hello Program"),	// window caption
		WS_OVERLAPPEDWINDOW,	// window style
		CW_USEDEFAULT,		// initial x position
		CW_USEDEFAULT,		// initial y position
		CW_USEDEFAULT,		// initial x size
		CW_USEDEFAULT,		// initial y size
		NULL,				// parent window handle
		NULL,				// window menu handle
		hInstance,			// program instance handle
		NULL); 			// creation parameters
						//CreateWindow call WindowProc to create window


	ShowWindow(hwnd, iCmdShow); //call WindowProc to show
	HDC hdc = GetDC(hwnd);

	UpdateWindow(hwnd); //call WindowProc to update dc
	

	

	while (GetMessage(&msg, NULL, 0, 0))//call WindowProc to get message(msg)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);//call WindowProc to process (ex: close win)

							  //printf("Win Running\n");
	}
	ReleaseDC(hwnd, hdc);

	return 1;

}

TCHAR *openfilename(const TCHAR *filter , HWND owner ) {
	OPENFILENAME ofn;
	//TCHAR fileName[MAX_PATH] = _T("");
	TCHAR *fileName = new TCHAR[128]{ 0 };
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = owner;
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = fileName;
	ofn.nMaxFile = MAX_PATH;
	ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
	ofn.lpstrDefExt = ((const TCHAR*)"");
	ofn.lpstrInitialDir = ((const TCHAR*)"Missions\\");

	//TCHAR fileNameStr[MAX_PATH] = { 0 };
	TCHAR *fileNameStr = new TCHAR[128]{ 0 };
	if (GetOpenFileName(&ofn))//open Dialog
		return ofn.lpstrFile;

	//fileNameStr = fileName;
	//_stprintf_s(fileNameStr,  sizeof(TCHAR) * 128, _T("%s"), fileName);

	return fileNameStr;
}
//========================　ＷinAPI

