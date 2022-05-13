//*********************************************************************
//*                                                                   *                                                                **
//**************              ZSInput.cpp           *********************
//**                                                                  *                                                               **
//**                                                                  *                                                               **
//*********************************************************************
//*                                                                   *                                                                  *
//*Revision:    Oct 10, 2000			                                  *  
//*Author:      Mat Williams            
//*Purpose:                                                           *
//*		This file provides implementation of ZSWindow class methods
//*
//*********************************************************************
//*Outstanding issues:                                                                                                      *
//*		Need to switch from directx surface as background to zssurface	
//*		No copy contructor or assignment operators defined
//*********************************************************************
//*********************************************************************
//revision 3: found source of memory leak
//revision 4: switched to event notification instead of immediate processing

#include "zsinput.h"
#include "zswindow.h"
#include "zsutilities.h"
#include "zssound.h"
#include "zsengine.h"

#define MOUSE_CURSOR_WIDTH	32
#define MOUSE_CURSOR_HEIGHT 32

#ifdef USE_SDL
int ZSInputSystem::Init()
{
	memset(KeyState, 0, 256);
	memset(NewKeys, 0, 256);
	MouseResolution = 2;

	return TRUE;
}
#else
int ZSInputSystem::Init(HWND hWindow, HINSTANCE hInstance, BOOL Windowed)
{
	DIPROPDWORD DIProp;

	DIProp.diph.dwSize = sizeof(DIPROPDWORD);
	DIProp.diph.dwHeaderSize = sizeof(DIPROPHEADER);

	HRESULT hr;
	hr = DirectInputCreateEx(hInstance,
								DIRECTINPUT_VERSION,
								IID_IDirectInput7,
								(void **)&DirectInput,
								NULL);

	if(hr != DI_OK)
	{
		SafeExit("unable to create direct input obejct");
	}

	hr = DirectInput->CreateDeviceEx(GUID_SysKeyboard, IID_IDirectInputDevice7,
        (void**)&KeyBoard, NULL);

	if(hr != DI_OK)
	{
		SafeExit("unable to create direct Keyboard");
	}

	hr = DirectInput->CreateDeviceEx(GUID_SysMouse, IID_IDirectInputDevice7,
        (void**)&Mouse, NULL);

	if(hr != DI_OK)
	{
		SafeExit("unable to create direct Mouse");
	}

//**********************************************************************************
//		Setting the mouse and keyboard to nonexclusive mode eats the hell out of system
//		resources, but is fairly necessary for debugging
//		all tests show that setting to exclusive mode eliminates resource gobbling
//		Resources are returned on safe exit
//
//************************************************************************************

	//set the devices' cooperation levels
	if(Windowed)
	{
		hr = Mouse->SetCooperativeLevel(hWindow, 
			  DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
	}
	else
	{
		hr = Mouse->SetCooperativeLevel(hWindow, 
			  DISCL_EXCLUSIVE | DISCL_FOREGROUND);
	}
	if(hr != DI_OK)
	{
		SafeExit("unable to set Mouse co-op");
	}

	if(Windowed)
	{
		hr = KeyBoard->SetCooperativeLevel(hWindow, 
        DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
	}
	else
	{
		hr = KeyBoard->SetCooperativeLevel(hWindow, 
			  DISCL_EXCLUSIVE | DISCL_FOREGROUND);
	}

	if(hr != DI_OK)
	{
		SafeExit("unable to set keyboard co-op");
	}

	hr = Mouse->SetDataFormat(&c_dfDIMouse);

	if(hr != DI_OK)
	{
		SafeExit("unable to set mouse format");
	}

	//set the starting cursor position 
	POINT mp;

	//get the windows cursor position
	GetCursorPos(&mp);

	rMouseScreen.left = mp.x;
	rMouseScreen.top = mp.y;

	//set resolution to 2
	//grainularity should be finer than this....
	MouseResolution = 2;

	hr = KeyBoard->SetDataFormat(&c_dfDIKeyboard);

	if(hr != DI_OK)
	{
		SafeExit("unable to set keyboard format");
	}

	//set event handling
	InputHandles[0] =	CreateEvent( NULL, FALSE, FALSE, NULL );
	hr = Mouse->SetEventNotification(InputHandles[0]);
	if(hr != DI_OK)
	{
		SafeExit("unable to set Mouse to Event Handling");
	}

	//acquire the mouse
	hr = Mouse->Acquire();
	if(hr != DI_OK)
	{
		SafeExit("unable to set acquire mouse");
	}

	//set event handling
	InputHandles[1] =	CreateEvent( NULL, FALSE, FALSE, NULL );
	hr = KeyBoard->SetEventNotification(InputHandles[1]);
	if(hr != DI_OK)
	{
		SafeExit("unable to set keyboard to event Handling");
	}

	//acquire the keyboard
	hr = KeyBoard->Acquire();
	if(hr != DI_OK)
	{
		SafeExit("unable to acquire keyboard");
	}


	return TRUE;
}
#endif

int ZSInputSystem::ShutDown()
{
#ifndef USE_SDL
	if(Mouse) {
		Mouse->SetEventNotification(NULL);
		CloseHandle(InputHandles[0]);
		Mouse->Unacquire();
		Mouse->Release();
		Mouse = NULL;
	}

	if(KeyBoard) {
		KeyBoard->SetEventNotification(NULL);
		CloseHandle(InputHandles[1]);
		KeyBoard->Unacquire();
		KeyBoard->Release();
		KeyBoard = NULL;
	}

	if(DirectInput) {
		DirectInput->Release();
		DirectInput = NULL;
	}
#endif
	return FALSE;
}

static int sdl_keycode_to_directinput(SDL_Keycode kc)
{
	switch (kc)
	{
	case SDLK_ESCAPE:
		return DIK_ESCAPE;
	case SDLK_0:
		return DIK_0;
	case SDLK_1:
		return DIK_1;
	case SDLK_2:
		return DIK_2;
	case SDLK_3:
		return DIK_3;
	case SDLK_4:
		return DIK_4;
	case SDLK_5:
		return DIK_5;
	case SDLK_6:
		return DIK_6;
	case SDLK_7:
		return DIK_7;
	case SDLK_8:
		return DIK_8;
	case SDLK_9:
		return DIK_9;
	case SDLK_F1:
		return DIK_F1;
	case SDLK_F2:
		return DIK_F2;
	case SDLK_F3:
		return DIK_F3;
	case SDLK_F4:
		return DIK_F4;
	case SDLK_F5:
		return DIK_F5;
	case SDLK_F6:
		return DIK_F6;
	case SDLK_F7:
		return DIK_F7;
	case SDLK_F8:
		return DIK_F8;
	case SDLK_F9:
		return DIK_F9;
	case SDLK_F10:
		return DIK_F10;
	case SDLK_F11:
		return DIK_F11;
	case SDLK_F12:
		return DIK_F12;
	}
	return -1;
}

static void handle_key_state(BYTE* keys, SDL_Keycode kc, int down)
{
	int index = sdl_keycode_to_directinput(kc);
	if (index < 0 || index >= 256)
	{
		return;
	}

	if (down)
	{
		keys[index] |= 0x80;
	}
	else
	{
		keys[index] &= ~(0x80);
	}
}

#ifdef USE_SDL
void ZSInputSystem::Update(ZSWindow* pWin)
{
	SDL_Event event;
	long zero = 0;
	while (SDL_PollEvent(&event))
	{
		if (event.type == SDL_QUIT)
		{
			//TODO: Handle this
		}
		else if (event.type == SDL_KEYDOWN)
		{
			handle_key_state(NewKeys, event.key.keysym.sym, 1);
		}
		else if (event.type == SDL_KEYUP)
		{
			handle_key_state(NewKeys, event.key.keysym.sym, 0);
		}
		else if (event.type == SDL_MOUSEMOTION)
		{
			//update the mouse position from the new mouse state
			rMouseScreen.left += (event.motion.xrel * MouseResolution);
			rMouseScreen.top += (event.motion.yrel * MouseResolution);

			//confirm the position from the input focus
			pWin->MoveMouse(&rMouseScreen.left, &rMouseScreen.top, &zero);
			rMouseScreen.right = rMouseScreen.left + MOUSE_CURSOR_WIDTH;
			rMouseScreen.bottom = rMouseScreen.top + MOUSE_CURSOR_HEIGHT;

		}
		else if (event.type == SDL_MOUSEBUTTONDOWN)
		{
			if (event.button.button == SDL_BUTTON_LEFT)
			{
				//see if the left button up has already been handled
				if (!LeftButtonDownHandled)
				{
					LeftButtonDownHandled = TRUE;
					LeftButtonUpHandled = FALSE;

					//if not send message
					pWin->LeftButtonDown(rMouseScreen.left, rMouseScreen.top);
				}
			}
			if (event.button.button == SDL_BUTTON_RIGHT)
			{
				//see if right button down has been handled
				if (!RightButtonDownHandled)
				{
					RightButtonDownHandled = TRUE;
					RightButtonUpHandled = FALSE;

					//if not send message
					pWin->RightButtonDown(rMouseScreen.left, rMouseScreen.top);
				}
			}
		}
		else if (event.type == SDL_MOUSEBUTTONUP)
		{
			if (event.button.button == SDL_BUTTON_LEFT)
			{
				//see if the left button up has already been handled
				if (!LeftButtonUpHandled)
				{
					LeftButtonUpHandled = TRUE;
					LeftButtonDownHandled = FALSE;

					//if not send a message to the input focus
					pWin->LeftButtonUp(rMouseScreen.left, rMouseScreen.top);
				}
			}
			if (event.button.button == SDL_BUTTON_RIGHT)
			{
				if (!RightButtonUpHandled)
				{
					RightButtonUpHandled = TRUE;
					RightButtonDownHandled = FALSE;

					//if not send message
					pWin->RightButtonUp(rMouseScreen.left, rMouseScreen.top);
				}
			}
		}
	}

	//send the new and last keystates to the imput focus
	pWin->HandleKeys(NewKeys, KeyState);
	memcpy(KeyState, NewKeys, 256);
	if (NewKeys[DIK_ESCAPE] & 0x80 &&
		(NewKeys[DIK_LCONTROL] & 0x80 || NewKeys[DIK_RCONTROL] & 0x80) &&
		(NewKeys[DIK_LSHIFT] & 0x80 || NewKeys[DIK_RSHIFT] & 0x80))
	{
		pWin->ReleaseFocus();
		DEBUG_INFO("forcing a focus release from input module\n");
	}
	else
		if (NewKeys[DIK_ESCAPE] & 0x80 &&
			(NewKeys[DIK_LCONTROL] & 0x80 || NewKeys[DIK_RCONTROL] & 0x80))
		{
			SafeExit("hard exit from input module");
		}
}
#else
void ZSInputSystem::Update(ZSWindow *pWin)
{
	DIMOUSESTATE ms;
	HRESULT hr;
	DWORD dwResult;

//	Engine->Sound()->Update();
	
	//dwResult = MsgWaitForMultipleObjects(2, InputHandles, FALSE, 0, QS_ALLINPUT); 
	dwResult = WaitForSingleObject(InputHandles[0], 0); 
	
	if(dwResult == WAIT_OBJECT_0)
	{
  // Event 1 has been set. If the event was 
        // created as autoreset, it has also 
        // been reset. 
		//get the mouse state
		hr = Mouse->GetDeviceState(sizeof(DIMOUSESTATE),&ms);
		if(hr != DI_OK)
		{
			//if the mouse was lost attempt to re-acquire it
			if(hr == DIERR_INPUTLOST)
				Mouse->Acquire();
		}
		else
		{
			//update the mouse position from the new mouse state
			rMouseScreen.left += (MouseState.lX * MouseResolution);
			rMouseScreen.top += (MouseState.lY * MouseResolution);
			MouseState.lZ *= MouseResolution;

			//confirm the position from the input focus
			pWin->MoveMouse(&rMouseScreen.left,&rMouseScreen.top, &MouseState.lZ);
			rMouseScreen.right = rMouseScreen.left + MOUSE_CURSOR_WIDTH;
			rMouseScreen.bottom = rMouseScreen.top + MOUSE_CURSOR_HEIGHT;

			//check button 1
			if(MouseState.rgbButtons[0] &0x80)
			{
				if(ms.rgbButtons[0] & 0x80)
				{
				}
				else
				{
					//see if the left button up has already been handled
					if(!LeftButtonUpHandled)
					{
						LeftButtonUpHandled = TRUE;
						LeftButtonDownHandled = FALSE;

						//if not send a message to the input focus
						pWin->LeftButtonUp(rMouseScreen.left,rMouseScreen.top);
					}
				}
			}
			else
			{
				if(ms.rgbButtons[0] & 0x80)
				{
					//see if the left button down has been handled
					if(!LeftButtonDownHandled)
					{
						LeftButtonDownHandled = TRUE;
						LeftButtonUpHandled = FALSE;
						
						//if not send message
						pWin->LeftButtonDown(rMouseScreen.left,rMouseScreen.top);
					}
				}
				else
				{
				}
			}

			if(MouseState.rgbButtons[1] & 0x80)
			{
				if(ms.rgbButtons[1] & 0x80)
				{
				}
				else
				{
					//see if right button up has been handled
					if(!RightButtonUpHandled)
					{
						RightButtonUpHandled = TRUE;
						RightButtonDownHandled = FALSE;

						//if not send message
						pWin->RightButtonUp(rMouseScreen.left,rMouseScreen.top);
					}
				}
			}
			else
			{
				if(ms.rgbButtons[1] & 0x80)
				{
					//see if right button down has been handled
					if(!RightButtonDownHandled)
					{
						RightButtonDownHandled = TRUE;
						RightButtonUpHandled = FALSE;

						//if not send message
						pWin->RightButtonDown(rMouseScreen.left,rMouseScreen.top);
					}
				}
				else
				{
				}
			}
			MouseState = ms;

		}

		//done with mouse
	}

	//dwResult = MsgWaitForMultipleObjects(2, InputHandles, FALSE, 0, QS_ALLINPUT); 
	dwResult = WaitForSingleObject(InputHandles[1], 0); 
	
	if(dwResult == WAIT_OBJECT_0)
	{
		// Event 2 has been set. If the event was 
        // created as autoreset, it has also 
        // been reset. 
			//check keyboard
			hr = KeyBoard->GetDeviceState(256, NewKeys);
			if(hr != DI_OK)
			{
				//reacquire if necessary
				if(hr == DIERR_INPUTLOST)
					KeyBoard->Acquire();
				
			}
			else
			{
				//send the new and last keystates to the imput focus
				pWin->HandleKeys(NewKeys, KeyState);
				memcpy(KeyState,NewKeys,256);
				if(NewKeys[DIK_ESCAPE] & 0x80 &&
					(NewKeys[DIK_LCONTROL] & 0x80 || NewKeys[DIK_RCONTROL] & 0x80) &&
					(NewKeys[DIK_LSHIFT] & 0x80 || NewKeys[DIK_RSHIFT] & 0x80))
					{
						pWin->ReleaseFocus();
						DEBUG_INFO("forcing a focus release from input module\n");
					}
				else
				if(NewKeys[DIK_ESCAPE] & 0x80 &&
					(NewKeys[DIK_LCONTROL] & 0x80 || NewKeys[DIK_RCONTROL] & 0x80))
					{
					  SafeExit("hard exit from input module");
					}
				
		
			}

	}

/*   switch (dwResult) { 
    case WAIT_OBJECT_0: 
        // Event 1 has been set. If the event was 
        // created as autoreset, it has also 
        // been reset. 
		//get the mouse state
		hr = Mouse->GetDeviceState(sizeof(DIMOUSESTATE),&ms);
		if(hr != DI_OK)
		{
			//if the mouse was lost attempt to re-acquire it
			if(hr == DIERR_INPUTLOST)
				Mouse->Acquire();
		}
		else
		{
			//update the mouse position from the new mouse state
			rMouseScreen.left += (MouseState.lX * MouseResolution);
			rMouseScreen.top += (MouseState.lY * MouseResolution);

			//confirm the position from the input focus
			pWin->MoveMouse(&rMouseScreen.left,&rMouseScreen.top);
			rMouseScreen.right = rMouseScreen.left + MOUSE_CURSOR_WIDTH;
			rMouseScreen.bottom = rMouseScreen.top + MOUSE_CURSOR_HEIGHT;

			//check button 1
			if(MouseState.rgbButtons[0] &0x80)
			{
				if(ms.rgbButtons[0] & 0x80)
				{
				}
				else
				{
					//see if the left button up has already been handled
					if(!LeftButtonUpHandled)
					{
						LeftButtonUpHandled = TRUE;
						LeftButtonDownHandled = FALSE;

						//if not send a message to the input focus
						pWin->LeftButtonUp(rMouseScreen.left,rMouseScreen.top);
					}
				}
			}
			else
			{
				if(ms.rgbButtons[0] & 0x80)
				{
					//see if the left button down has been handled
					if(!LeftButtonDownHandled)
					{
						LeftButtonDownHandled = TRUE;
						LeftButtonUpHandled = FALSE;
						
						//if not send message
						pWin->LeftButtonDown(rMouseScreen.left,rMouseScreen.top);
					}
				}
				else
				{
				}
			}

			if(MouseState.rgbButtons[1] & 0x80)
			{
				if(ms.rgbButtons[1] & 0x80)
				{
				}
				else
				{
					//see if right button up has been handled
					if(!RightButtonUpHandled)
					{
						RightButtonUpHandled = TRUE;
						RightButtonDownHandled = FALSE;

						//if not send message
						pWin->RightButtonUp(rMouseScreen.left,rMouseScreen.top);
					}
				}
			}
			else
			{
				if(ms.rgbButtons[1] & 0x80)
				{
					//see if right button down has been handled
					if(!RightButtonDownHandled)
					{
						RightButtonDownHandled = TRUE;
						RightButtonUpHandled = FALSE;

						//if not send message
						pWin->RightButtonDown(rMouseScreen.left,rMouseScreen.top);
					}
				}
				else
				{
				}
			}
			MouseState = ms;

		}

		//done with mouse
	    break; 
 
    case WAIT_OBJECT_0 + 1: 
        // Event 2 has been set. If the event was 
        // created as autoreset, it has also 
        // been reset. 
			//check keyboard
			hr = KeyBoard->GetDeviceState(256, NewKeys);
			if(hr != DI_OK)
			{
				//reacquire if necessary
				if(hr == DIERR_INPUTLOST)
					KeyBoard->Acquire();
				
			}
			else
			{
				//send the new and last keystates to the imput focus
				pWin->HandleKeys(NewKeys, KeyState);
				memcpy(KeyState,NewKeys,256);
			}
	    break; 
		case WAIT_OBJECT_0 + 2:
        while ( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) ) 
        {
            if (msg.message == WM_QUIT) 
            {
                // stop loop if it's a quit message
					exit(1);
				} 
            else 
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        break;
	}
*/
  
}

#endif

void ZSInputSystem::ClearStates()
{
#ifndef USE_SDL
	memset(&MouseState,0,sizeof(MouseState));
#endif
	memset(KeyState,0,256);
	RightButtonDownHandled = FALSE;
	RightButtonUpHandled = FALSE;
	LeftButtonUpHandled = FALSE;
	LeftButtonDownHandled = FALSE;
}


ZSInputSystem::ZSInputSystem()
{
#ifndef USE_SDL
	memset(&MouseState,0,sizeof(MouseState));
#endif
	memset(KeyState,0,256);
	RightButtonDownHandled = FALSE;
	RightButtonUpHandled = FALSE;
	LeftButtonUpHandled = FALSE;
	LeftButtonDownHandled = FALSE;
#ifndef USE_SDL
	Mouse = NULL;
	DirectInput = NULL;
	KeyBoard = NULL;
#endif
}

ZSInputSystem::~ZSInputSystem()
{
	//confirm shutdown
	
	//if mouse or keyboard or input exists we have not shut down
#ifndef USE_SDL
	if(Mouse || KeyBoard || DirectInput)
#else
	if (1)
#endif
	{
		ShutDown();
	}

}