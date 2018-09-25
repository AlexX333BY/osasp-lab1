#include <windows.h>
#include <gdiplus.h>

#define _USE_MATH_DEFINES 
#include <cmath>  

#define WND_CLASS_NAME "LaboratoryWork1Class"

#define WM_LOAD_SPRITE WM_USER
#define WM_UPDATE_SPRITE (WM_USER+1)

#define VK_L 0x4c
#define VK_W 0x57
#define VK_A 0x41
#define VK_S 0x53
#define VK_D 0x44
#define VK_Q 0x51
#define VK_E 0x45
#define VK_H 0x48
#define VK_V 0x56

#define SPRITE_STEP 16
#define SPRITE_DEGREE_ROTATE_STEP 15

#define BACKGROUND_COLOR GetSysColor(COLOR_WINDOW)

typedef struct InvertionStruct
{
	bool isHorizontallyInverted;
	bool isVerticallyInverted;
} _InvertionStruct;

typedef enum LoadResult
{
	NoError = 0,
	CancelledByUser = 1,
	UnknownError = -1,
	TooBigSizeError = -2
} _LoadResult;

bool PostLoadSpriteMessage(HWND hWnd)
{
	return PostMessage(hWnd, WM_LOAD_SPRITE, NULL, NULL);
}

bool PostUpdateSpriteMessage(HWND hWnd)
{
	return PostMessage(hWnd, WM_UPDATE_SPRITE, NULL, NULL);
}

SIZE GetClientWindowSize(HWND hWnd)
{
	RECT rect;
	GetClientRect(hWnd, &rect);
	SIZE result;
	result.cx = rect.right - rect.left;
	result.cy = rect.bottom - rect.top;
	return result;
}

SIZE GetBitmapSize(HBITMAP hBitmap)
{
	BITMAP bitmap;
	GetObject(hBitmap, sizeof(BITMAP), &bitmap);
	SIZE result;
	result.cx = bitmap.bmWidth;
	result.cy = bitmap.bmHeight;
	return result;
}

int FillWindowWithColor(HWND hWnd, COLORREF color)
{
	RECT rect;
	GetClientRect(hWnd, &rect);
	HDC wndDC = GetDC(hWnd);
	HBRUSH hBrush = CreateSolidBrush(color);
	int result = FillRect(wndDC, &rect, hBrush);
	DeleteObject(hBrush);
	ReleaseDC(hWnd, wndDC);
	return result;
}

XFORM GetRotationXform(short degreeAngle)
{
	XFORM xForm;
	FLOAT radAngle = (FLOAT)(M_PI * degreeAngle / 180);
	FLOAT angleSin = sin(radAngle);
	FLOAT angleCos = cos(radAngle);
	xForm.eM11 = angleCos;
	xForm.eM12 = angleSin;
	xForm.eM21 = -angleSin;
	xForm.eM22 = angleCos;
	xForm.eDx = 0;
	xForm.eDy = 0;
	return xForm;
}

XFORM GetInvertionXform(InvertionStruct invertionStruct)
{
	XFORM xForm;
	xForm.eM11 = (invertionStruct.isVerticallyInverted ? -1 : 1);
	xForm.eM12 = 0;
	xForm.eM21 = 0;
	xForm.eM22 = (invertionStruct.isHorizontallyInverted ? -1 : 1);
	xForm.eDx = 0;
	xForm.eDy = 0;
	return xForm;
}

XFORM GetMovementXform(COORD coordinates)
{
	XFORM xForm;
	xForm.eM11 = 1;
	xForm.eM12 = 0;
	xForm.eM21 = 0;
	xForm.eM22 = 1;
	xForm.eDx = coordinates.X;
	xForm.eDy = coordinates.Y;
	return xForm;
}

bool PutSpriteOnWindow(HWND hWnd, HBITMAP sprite, COORD coordinates, short angle, InvertionStruct invertionStruct)
{
	HDC wndDC = GetDC(hWnd);
	HDC spriteDC = CreateCompatibleDC(wndDC);
	HGDIOBJ oldObject = SelectObject(spriteDC, sprite);
	SIZE bitmapSize = GetBitmapSize(sprite);

	XFORM xForm;
	int prevGraphicsMode = SetGraphicsMode(wndDC, GM_ADVANCED);

	xForm = GetMovementXform(coordinates);
	SetWorldTransform(wndDC, &xForm);

	COORD transformationCenter;
	transformationCenter.X = - (coordinates.X + bitmapSize.cx / 2);
	transformationCenter.Y = - (coordinates.Y + bitmapSize.cy / 2);
	xForm = GetMovementXform(transformationCenter);
	ModifyWorldTransform(wndDC, &xForm, MWT_RIGHTMULTIPLY);

	xForm = GetRotationXform(angle);
	ModifyWorldTransform(wndDC, &xForm, MWT_RIGHTMULTIPLY);

	xForm = GetInvertionXform(invertionStruct);
	ModifyWorldTransform(wndDC, &xForm, MWT_RIGHTMULTIPLY);

	transformationCenter.X = -transformationCenter.X;
	transformationCenter.Y = -transformationCenter.Y;
	xForm = GetMovementXform(transformationCenter);
	ModifyWorldTransform(wndDC, &xForm, MWT_RIGHTMULTIPLY);

	bool result = BitBlt(wndDC, 0, 0, bitmapSize.cx, bitmapSize.cy, spriteDC, 0, 0, SRCCOPY);
	ModifyWorldTransform(wndDC, NULL, MWT_IDENTITY);
	SetGraphicsMode(wndDC, prevGraphicsMode);

	SelectObject(spriteDC, oldObject);
	DeleteDC(spriteDC);
	ReleaseDC(hWnd, wndDC);
	return result;
}

LoadResult LoadSprite(HWND hWnd, HBITMAP &sprite)
{
	char fileName[MAX_PATH] = { NULL };

	OPENFILENAME openFileName;
	openFileName.lStructSize = sizeof(OPENFILENAME);
	openFileName.hwndOwner = hWnd;
	openFileName.hInstance = NULL;
	openFileName.lpstrFilter = "Images\0*.bmp;*.gif;*.jpeg;*.png;*.tiff;*.exif;*.wmf;*.emf\0\0";
	openFileName.lpstrCustomFilter = NULL;
	openFileName.nFilterIndex = 1;
	openFileName.lpstrFile = fileName;
	openFileName.nMaxFile = sizeof(fileName);
	openFileName.lpstrFileTitle = NULL;
	openFileName.lpstrInitialDir = NULL;
	openFileName.lpstrTitle = "Select sprite image";
	openFileName.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
	openFileName.lpstrDefExt = NULL;

	if (GetOpenFileName(&openFileName))
	{
		int fileNameLength = MultiByteToWideChar(CP_ACP, 0, fileName, -1, NULL, 0);
		WCHAR *wideCharFileName = new WCHAR[MultiByteToWideChar(CP_ACP, 0, fileName, -1, NULL, 0)];
		MultiByteToWideChar(CP_ACP, 0, fileName, -1, wideCharFileName, fileNameLength);

		Gdiplus::GdiplusStartupInput gdiplusStartupInput;
		ULONG_PTR gdiplusToken;
		GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

		Gdiplus::Bitmap *sourceImage = Gdiplus::Bitmap::FromFile(wideCharFileName);
		HBITMAP hBitmap;
		Gdiplus::Color imageBackgroundColor;
		imageBackgroundColor.SetFromCOLORREF(BACKGROUND_COLOR);
		Gdiplus::Status bitmapStatus = sourceImage->GetHBITMAP(imageBackgroundColor, &hBitmap);

		Gdiplus::GdiplusShutdown(gdiplusToken);

		if (bitmapStatus != Gdiplus::Ok)
		{
			return UnknownError;
		}

		SIZE windowSize = GetClientWindowSize(hWnd), spriteSize = GetBitmapSize(hBitmap);
		if ((windowSize.cx < spriteSize.cx) || (windowSize.cy < spriteSize.cy))
		{
			return TooBigSizeError;
		}

		sprite = hBitmap;
		return NoError;
	}
	return CancelledByUser;
}

COORD CreateNewSpritePosition(COORD spritePosition, COORD spriteSteps, HWND hWnd, HBITMAP sprite)
{
	SIZE windowSize = GetClientWindowSize(hWnd), spriteSize = GetBitmapSize(sprite);
	COORD newSpritePosition;
	newSpritePosition.X = spritePosition.X + spriteSteps.X;
	if (newSpritePosition.X < 0)
	{
		newSpritePosition.X = 0;
	}
	else if (newSpritePosition.X + spriteSize.cx > windowSize.cx)
	{
		newSpritePosition.X = windowSize.cx - spriteSize.cx;
	}
	newSpritePosition.Y = spritePosition.Y + spriteSteps.Y;
	if (newSpritePosition.Y < 0)
	{
		newSpritePosition.Y = 0;
	}
	else if (newSpritePosition.Y + spriteSize.cy > windowSize.cy)
	{
		newSpritePosition.Y = windowSize.cy - spriteSize.cy;
	}
	return newSpritePosition;
}

COORD CreateUpSteps()
{
	COORD steps;
	steps.X = 0;
	steps.Y = -SPRITE_STEP;
	return steps;
}

COORD CreateLeftSteps()
{
	COORD steps;
	steps.X = -SPRITE_STEP;
	steps.Y = 0;
	return steps;
}

COORD CreateDownSteps()
{
	COORD steps;
	steps.X = 0;
	steps.Y = SPRITE_STEP;
	return steps;
}

COORD CreateRightSteps()
{
	COORD steps;
	steps.X = SPRITE_STEP;
	steps.Y = 0;
	return steps;
}

InvertionStruct GetUninvertedStruct()
{
	InvertionStruct invertion;
	invertion.isHorizontallyInverted = false;
	invertion.isVerticallyInverted = false;
	return invertion;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HBITMAP sprite = NULL;
	static COORD spritePosition = { 0 };
	static short angle = 0;
	static InvertionStruct invertion = GetUninvertedStruct();

	switch (message)
	{
	case WM_LOAD_SPRITE:
		switch (LoadSprite(hWnd, sprite))
		{
		case NoError:
			spritePosition = { 0 };
			angle = 0;
			invertion = GetUninvertedStruct();
			PostMessage(hWnd, WM_UPDATE_SPRITE, NULL, NULL);
			break;
		case TooBigSizeError:
			MessageBox(hWnd, "Image size is too big to fit in windows", "Too big", MB_OK | MB_ICONERROR);
			break;
		case UnknownError:
			MessageBox(hWnd, "Error while loading image", "Error", MB_OK | MB_ICONERROR);
			break;
		}
		break;
	case WM_UPDATE_SPRITE:
		FillWindowWithColor(hWnd, BACKGROUND_COLOR);
		PutSpriteOnWindow(hWnd, sprite, spritePosition, angle, invertion);
		break;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_L:
			PostLoadSpriteMessage(hWnd);
			break;
		case VK_UP:
		case VK_W:
			spritePosition = CreateNewSpritePosition(spritePosition, CreateUpSteps(), hWnd, sprite);
			PostUpdateSpriteMessage(hWnd);
			break;
		case VK_LEFT:
		case VK_A:
			spritePosition = CreateNewSpritePosition(spritePosition, CreateLeftSteps(), hWnd, sprite);
			PostUpdateSpriteMessage(hWnd);
			break;
		case VK_DOWN:
		case VK_S:
			spritePosition = CreateNewSpritePosition(spritePosition, CreateDownSteps(), hWnd, sprite);
			PostUpdateSpriteMessage(hWnd);
			break;
		case VK_RIGHT:
		case VK_D:
			spritePosition = CreateNewSpritePosition(spritePosition, CreateRightSteps(), hWnd, sprite);
			PostUpdateSpriteMessage(hWnd);
			break;
		case VK_Q:
			angle -= SPRITE_DEGREE_ROTATE_STEP;
			PostUpdateSpriteMessage(hWnd);
			break;
		case VK_E:
			angle += SPRITE_DEGREE_ROTATE_STEP;
			PostUpdateSpriteMessage(hWnd);
			break;
		case VK_H:
			invertion.isHorizontallyInverted = !invertion.isHorizontallyInverted;
			PostUpdateSpriteMessage(hWnd);
			break;
		case VK_V:
			invertion.isVerticallyInverted = !invertion.isVerticallyInverted;
			PostUpdateSpriteMessage(hWnd);
			break;
		}
		if (wParam != VK_ESCAPE)
		{
			break;
		}
	case WM_CLOSE:
		if (MessageBox(hWnd, "Are you sure you want to exit?", "Exit question", MB_YESNO | MB_ICONQUESTION) == IDYES)
		{
			PostQuitMessage(0);
		}
		break;
	case WM_MOUSEWHEEL:
		if (GET_KEYSTATE_WPARAM(wParam) == MK_SHIFT)
		{
			if (GET_WHEEL_DELTA_WPARAM(wParam) > 0)
			{
				spritePosition = CreateNewSpritePosition(spritePosition, CreateLeftSteps(), hWnd, sprite);
				PostUpdateSpriteMessage(hWnd);
			}
			else
			{
				spritePosition = CreateNewSpritePosition(spritePosition, CreateRightSteps(), hWnd, sprite);
				PostUpdateSpriteMessage(hWnd);
			}
		}
		else
		{
			if (GET_WHEEL_DELTA_WPARAM(wParam) > 0)
			{
				spritePosition = CreateNewSpritePosition(spritePosition, CreateUpSteps(), hWnd, sprite);
				PostUpdateSpriteMessage(hWnd);
			}
			else
			{
				spritePosition = CreateNewSpritePosition(spritePosition, CreateDownSteps(), hWnd, sprite);
				PostUpdateSpriteMessage(hWnd);
			}
		}
		break;
	case WM_SIZE:
		PostUpdateSpriteMessage(hWnd);
		return DefWindowProc(hWnd, message, wParam, lParam);
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	WNDCLASSEX wndClassEx;
	wndClassEx.cbSize = sizeof(WNDCLASSEX);
	wndClassEx.style = CS_DBLCLKS;
	wndClassEx.lpfnWndProc = WndProc;
	wndClassEx.cbClsExtra = 0;
	wndClassEx.cbWndExtra = 0;
	wndClassEx.hInstance = hInstance;
	wndClassEx.hIcon = NULL;
	wndClassEx.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndClassEx.hbrBackground = CreateSolidBrush(BACKGROUND_COLOR);
	wndClassEx.lpszMenuName = NULL;
	wndClassEx.lpszClassName = WND_CLASS_NAME;
	wndClassEx.hIconSm = NULL;
	RegisterClassEx(&wndClassEx);

	HWND hWnd = CreateWindow(WND_CLASS_NAME, "Lab 1", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, hInstance, NULL);
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	MSG msg;
	PostLoadSpriteMessage(hWnd);
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}
