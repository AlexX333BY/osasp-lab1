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

int FillWindowWithColor(HWND hWnd, HBRUSH hBrush)
{
	RECT rect;
	GetClientRect(hWnd, &rect);
	HDC wndDC = GetDC(hWnd);
	int result = FillRect(wndDC, &rect, hBrush);
	ReleaseDC(hWnd, wndDC);
	return result;
}

bool PutSpriteOnWindow(HWND hWnd, HBITMAP sprite, COORD coordinates)
{
	HDC wndDC = GetDC(hWnd);
	HDC spriteDC = CreateCompatibleDC(wndDC);
	HGDIOBJ oldObject = SelectObject(spriteDC, sprite);
	SIZE bitmapSize = GetBitmapSize(sprite);
	bool result = BitBlt(wndDC, coordinates.X, coordinates.Y, bitmapSize.cx, bitmapSize.cy, spriteDC, 0, 0, SRCCOPY);
	SelectObject(spriteDC, oldObject);
	DeleteDC(spriteDC);
	ReleaseDC(hWnd, wndDC);
	return result;
}

bool LoadSprite(HWND hWnd, HBITMAP &sprite)
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
		HANDLE handle = LoadImage(NULL, fileName, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_LOADTRANSPARENT);
		if (handle == NULL)
		{
			return false;
		}
		sprite = (HBITMAP)handle;
		return true;
	}
	return false;
}

bool CanMoveSprite(int spriteDimension, int leftBound, int rightBound, int curCoordinate, int step)
{
	return (((curCoordinate + step) >= leftBound) && ((curCoordinate + step + spriteDimension) <= rightBound));
}

bool MoveSprite(HWND hWnd, HBITMAP sprite, COORD &spritePosition, COORD spriteSteps)
{
	if (sprite == NULL)
	{
		return false;
	}
	FillWindowWithColor(hWnd, (HBRUSH)(COLOR_WINDOW + 1));
	SIZE windowSize = GetClientWindowSize(hWnd), spriteSize = GetBitmapSize(sprite);
	if (CanMoveSprite(spriteSize.cx, 0, windowSize.cx, spritePosition.X, spriteSteps.X))
	{
		spritePosition.X += spriteSteps.X;
	}
	if (CanMoveSprite(spriteSize.cy, 0, windowSize.cy, spritePosition.Y, spriteSteps.Y))
	{
		spritePosition.Y += spriteSteps.Y;
	}
	return PutSpriteOnWindow(hWnd, sprite, spritePosition);
}

bool MoveSpriteUp(HWND hWnd, HBITMAP sprite, COORD &spritePosition)
{
	COORD steps;
	steps.X = 0;
	steps.Y = -SPRITE_STEP;
	return MoveSprite(hWnd, sprite, spritePosition, steps);
}

bool MoveSpriteLeft(HWND hWnd, HBITMAP sprite, COORD &spritePosition)
{
	COORD steps;
	steps.X = -SPRITE_STEP;
	steps.Y = 0;
	return MoveSprite(hWnd, sprite, spritePosition, steps);
}

bool MoveSpriteDown(HWND hWnd, HBITMAP sprite, COORD &spritePosition)
{
	COORD steps;
	steps.X = 0;
	steps.Y = SPRITE_STEP;
	return MoveSprite(hWnd, sprite, spritePosition, steps);
}

bool MoveSpriteRight(HWND hWnd, HBITMAP sprite, COORD &spritePosition)
{
	COORD steps;
	steps.X = SPRITE_STEP;
	steps.Y = 0;
	return MoveSprite(hWnd, sprite, spritePosition, steps);
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static HBITMAP sprite = NULL;
	static COORD spritePosition = { 0 };

	switch (message)
	{
	case WM_LOAD_SPRITE:
		if (LoadSprite(hWnd, sprite))
		{
			FillWindowWithColor(hWnd, (HBRUSH)(COLOR_WINDOW + 1));
			spritePosition = { 0 };
			PutSpriteOnWindow(hWnd, sprite, spritePosition);
		}
		else
		{
			MessageBox(hWnd, "Error while loading image", "Error", MB_OK | MB_ICONERROR);
		}
		break;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_L:
			PostLoadSpriteMessage(hWnd);
			break;
		case VK_UP:
		case VK_W:
			MoveSpriteUp(hWnd, sprite, spritePosition);
			break;
		case VK_LEFT:
		case VK_A:
			MoveSpriteLeft(hWnd, sprite, spritePosition);
			break;
		case VK_DOWN:
		case VK_S:
			MoveSpriteDown(hWnd, sprite, spritePosition);
			break;
		case VK_RIGHT:
		case VK_D:
			MoveSpriteRight(hWnd, sprite, spritePosition);
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
				MoveSpriteLeft(hWnd, sprite, spritePosition);
			}
			else
			{
				MoveSpriteRight(hWnd, sprite, spritePosition);
			}
		}
		else
		{
			if (GET_WHEEL_DELTA_WPARAM(wParam) > 0)
			{
				MoveSpriteUp(hWnd, sprite, spritePosition);
			}
			else
			{
				MoveSpriteDown(hWnd, sprite, spritePosition);
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
