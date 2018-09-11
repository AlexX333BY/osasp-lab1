#include <windows.h>

#define WND_CLASS_NAME "LaboratoryWork1Class"
#define WM_LOAD_SPRITE WM_USER
#define VK_L 0x4c

bool PostLoadSpriteMessage(HWND hWnd)
{
	return PostMessage(hWnd, WM_LOAD_SPRITE, NULL, NULL);
}

void LoadSprite(HWND hWnd)
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
		// to be implemented
	}
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_LOAD_SPRITE:
		LoadSprite(hWnd);
		break;
	case WM_KEYDOWN:
		if (wParam == VK_L)
		{
			PostLoadSpriteMessage(hWnd);
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
