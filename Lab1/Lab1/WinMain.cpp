#include <windows.h>

#define WND_CLASS_NAME "LaboratoryWork1Class"
#define WM_LOAD_SPRITE WM_USER
#define VK_L 0x4c
#define VK_W 0x57
#define VK_A 0x41
#define VK_S 0x53
#define VK_D 0x44
#define SPRITE_STEP 8

bool PostLoadSpriteMessage(HWND hWnd)
{
	return PostMessage(hWnd, WM_LOAD_SPRITE, NULL, NULL);
}

SIZE GetDeviceContextDimensions(HDC hdc)
{
	BITMAP structBitmapHeader;
	memset(&structBitmapHeader, 0, sizeof(BITMAP));
	GetObject(GetCurrentObject(hdc, OBJ_BITMAP), sizeof(BITMAP), &structBitmapHeader);
	SIZE size;
	size.cx = structBitmapHeader.bmWidth;
	size.cy = structBitmapHeader.bmHeight;
	return size;
}

int FillDeviceContextWithColor(HDC hdc, HBRUSH hBrush)
{
	SIZE size = GetDeviceContextDimensions(hdc);
	RECT rect;
	rect.left = 0;
	rect.top = 0;
	rect.right = size.cx;
	rect.bottom = size.cy;
	return FillRect(hdc, &rect, hBrush);
}

bool PutSpriteOnWindow(HDC wndDC, HDC spriteDC, COORD coordinates)
{
	SIZE bitmapSize = GetDeviceContextDimensions(spriteDC);
	return BitBlt(wndDC, coordinates.X, coordinates.Y, bitmapSize.cx, bitmapSize.cy, spriteDC, 0, 0, SRCCOPY);
}

bool LoadSprite(HWND hWnd, HDC &spriteDC, COORD &spritePosition)
{
	char fileName[MAX_PATH] = { NULL };

	OPENFILENAME openFileName;
	openFileName.lStructSize = sizeof(OPENFILENAME);
	openFileName.hwndOwner = hWnd;
	openFileName.hInstance = NULL;
	openFileName.lpstrFilter = "Bitmap images\0*.bmp\0\0";
	openFileName.lpstrCustomFilter = NULL;
	openFileName.nFilterIndex = 1;
	openFileName.lpstrFile = fileName;
	openFileName.nMaxFile = sizeof(fileName);
	openFileName.lpstrFileTitle = NULL;
	openFileName.lpstrInitialDir = NULL;
	openFileName.lpstrTitle = "Select sprite image";
	openFileName.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
	openFileName.lpstrDefExt = ".bmp";

	if (GetOpenFileName(&openFileName))
	{
		HANDLE sprite = LoadImage(NULL, fileName, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_LOADTRANSPARENT);
		if (sprite == NULL)
		{
			MessageBox(hWnd, "Error while loading image", "Error", MB_OK | MB_ICONERROR);
			return false;
		}

		HDC wndDC = GetDC(hWnd);
		spriteDC = CreateCompatibleDC(wndDC);
		SelectObject(spriteDC, sprite);

		FillDeviceContextWithColor(wndDC, (HBRUSH)(COLOR_WINDOW + 1));
		spritePosition.X = 0;
		spritePosition.Y = 0;
		return PutSpriteOnWindow(wndDC, spriteDC, spritePosition);
	}
	return false;
}

bool CanMoveSprite(int spriteDimension, int leftBound, int rightBound, int curCoordinate, int step)
{
	return (((curCoordinate + step) >= leftBound) && ((curCoordinate + step + spriteDimension) <= rightBound));
}

bool MoveSprite(HDC wndDC, HDC spriteDC, COORD &spritePosition, COORD spriteSteps)
{
	if (spriteDC == NULL)
	{
		return false;
	}
	FillDeviceContextWithColor(wndDC, (HBRUSH)(COLOR_WINDOW + 1));
	SIZE windowSize = GetDeviceContextDimensions(wndDC), spriteSize = GetDeviceContextDimensions(spriteDC);
	if (CanMoveSprite(spriteSize.cx, 0, windowSize.cx, spritePosition.X, spriteSteps.X))
	{
		spritePosition.X += spriteSteps.X;
	}
	if (CanMoveSprite(spriteSize.cy, 0, windowSize.cy, spritePosition.Y, spriteSteps.Y))
	{
		spritePosition.Y += spriteSteps.Y;
	}
	return PutSpriteOnWindow(wndDC, spriteDC, spritePosition);
}

bool MoveSpriteUp(HDC wndDC, HDC spriteDC, COORD &spritePosition)
{
	COORD steps;
	steps.X = 0;
	steps.Y = -SPRITE_STEP;
	return MoveSprite(wndDC, spriteDC, spritePosition, steps);
}

bool MoveSpriteLeft(HDC wndDC, HDC spriteDC, COORD &spritePosition)
{
	COORD steps;
	steps.X = -SPRITE_STEP;
	steps.Y = 0;
	return MoveSprite(wndDC, spriteDC, spritePosition, steps);
}

bool MoveSpriteDown(HDC wndDC, HDC spriteDC, COORD &spritePosition)
{
	COORD steps;
	steps.X = 0;
	steps.Y = SPRITE_STEP;
	return MoveSprite(wndDC, spriteDC, spritePosition, steps);
}

bool MoveSpriteRight(HDC wndDC, HDC spriteDC, COORD &spritePosition)
{
	COORD steps;
	steps.X = SPRITE_STEP;
	steps.Y = 0;
	return MoveSprite(wndDC, spriteDC, spritePosition, steps);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HDC spriteDC = NULL;
	static COORD spritePosition = { 0, 0 };

	switch (message)
	{
	case WM_LOAD_SPRITE:
		LoadSprite(hWnd, spriteDC, spritePosition);
		break;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_L:
			PostLoadSpriteMessage(hWnd);
			break;
		case VK_UP:
		case VK_W:
			MoveSpriteUp(GetDC(hWnd), spriteDC, spritePosition);
			break;
		case VK_LEFT:
		case VK_A:
			MoveSpriteLeft(GetDC(hWnd), spriteDC, spritePosition);
			break;
		case VK_DOWN:
		case VK_S:
			MoveSpriteDown(GetDC(hWnd), spriteDC, spritePosition);
			break;
		case VK_RIGHT:
		case VK_D:
			MoveSpriteRight(GetDC(hWnd), spriteDC, spritePosition);
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
				MoveSpriteLeft(GetDC(hWnd), spriteDC, spritePosition);
			}
			else
			{
				MoveSpriteRight(GetDC(hWnd), spriteDC, spritePosition);
			}
		}
		else
		{
			if (GET_WHEEL_DELTA_WPARAM(wParam) > 0)
			{
				MoveSpriteUp(GetDC(hWnd), spriteDC, spritePosition);
			}
			else
			{
				MoveSpriteDown(GetDC(hWnd), spriteDC, spritePosition);
			}
		}
		break;
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
	wndClassEx.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
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
