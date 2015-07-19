#include <Windows.h>
#include <stdio.h>
#include <sstream>
#include <RzChromaSDKDefines.h>
#include <RzChromaSDKTypes.h>
#include <RzErrors.h>

#ifdef _WIN64
#define CHROMASDKDLL        L"RzChromaSDK64.dll"
#else
#define CHROMASDKDLL        L"RzChromaSDK.dll"
#endif

using namespace ChromaSDK;
using namespace ChromaSDK::Keyboard;

HMODULE chromaSDK;
typedef RZRESULT(*INIT)(void);
typedef RZRESULT(*CREATEGRIDEFFECT)(CUSTOM_GRID_EFFECT_TYPE pCustomEffects, RZEFFECTID *pEffectId);
typedef RZRESULT(*CREATEEFFECT)(RZSIZE numKeys, CUSTOM_KEY_EFFECT_TYPE *pCustomEffects, RZEFFECTID *pEffectId);
CREATEGRIDEFFECT createGridEffect;
CREATEEFFECT createEffect;
CUSTOM_GRID_EFFECT_TYPE Grid;
float countDown = 0;
int BombCountdownTimer;
bool isTimerActive = false;

bool loadChromaSDK(){
	chromaSDK = LoadLibrary(CHROMASDKDLL);				//First we load the library into our memory
	if (chromaSDK){												//If 'chromaSDK' does not equals NULL then we've loaded the library succesfull, atleast as far as we know for now.
		INIT init = (INIT)GetProcAddress(chromaSDK, "Init");	//We try to get the address of Init ¿function? from the dll
		if (init){												//If that was succesful
			RZRESULT result = init();							//Then we execute it to initialize the keyboard, By now the keyboard should've gone dark.
			if (result == RZRESULT_SUCCESS){					//If the initialization was succesful then...
				return true;
			}
		}
	}
	return false;
}

bool loadChromaSDKFunctions(){
	createGridEffect = (CREATEGRIDEFFECT)GetProcAddress(chromaSDK, "CreateKeyboardCustomGridEffects"); //Get the ¿function? to create per key effects.
	createEffect = (CREATEEFFECT)GetProcAddress(chromaSDK, "CreateKeyboardCustomEffects"); //Get the ¿function? to create per key effects.
	if (createEffect && createGridEffect) //If it does not equal NULL then we succeeded!
		return true;
	return false;
}

void setStaticColor(COLORREF color){ //Cycle through every key to set one color;
	for (size_t row = 0; row < 6; row++)
		for (size_t col = 0; col < 22; col++){
			Grid.Key[row][col] = color;
		}
}

void setCustomEffect(){
	createGridEffect(Grid, NULL);
}

void resetKeyboardEffect(){
	setStaticColor(RGB(255, 135, 31));
	Grid.Key[2][5] = RGB(100, 255, 0); //R
	Grid.Key[2][3] = RGB(255, 0, 0); //W
	Grid.Key[3][2] = RGB(255, 0, 0); //A
	Grid.Key[3][3] = RGB(255, 0, 0); //S
	Grid.Key[3][4] = RGB(255, 0, 0); //D
	setCustomEffect();
}

void updateCountdownVisuals(){
	if ((int)(countDown*10)%20==2){					//If the keyboard is supposed to be darkened now (for the flickring effect)
		setStaticColor(RGB(0, 0, 0));
	}
	else if (countDown > 10)								//If we have more than 10 seconds then we can defuse it without a kit
		setStaticColor(RGB(50, 255, 50));
	else if (countDown <= 10 && countDown > 5){		//If we have more than 5 seconds but less then 10, we can use a defuse kit
		setStaticColor(RGB(50, 50, 255)); 
	}
	else{
		setStaticColor(RGB(255, 0, 0));				//GET THE FUCK OUT OF HERE ITS GONNNA EXPLOOODE!
	}

	
	for (size_t col = 0; col < 15; col++) //Go through the F keys + three remaining on top.
	{
		if (col > countDown / 3){
			Grid.Key[0][3 + col] = RGB(0, 0, 0);
		}
		else{
			if (col == (int)(countDown / 3)){ //If it is the last key on the bar
				Grid.Key[0][3 + col] = RGB(255, ((int)countDown % 3 * 83), ((int)countDown % 3 * 83));
			}
			else{
				Grid.Key[0][3 + col] = RGB(255, 255, 255);
			}
		}
	}

	
	setCustomEffect();

	//Countdown fixer
	countDown -= (float)0.25;
	if (countDown <= 0){ //Bomb is exploded by now reset ze keyboard.
		resetKeyboardEffect(); //Put back the an fps effect
		printf("But did you win? Bomb countdown ended.\n");
		isTimerActive = false;
		KillTimer(NULL, BombCountdownTimer);
	}
}

int main(){

	CUSTOM_GRID_EFFECT_TYPE Grid = {};

	printf("\t>> Make sure you CLOSE Razer Synapse or the software will NOT work!\n");
	if (!loadChromaSDK()){ //Try and execute the function above, that returns true if succeeded or false if failed.
		printf("#ERROR > Failed to load the chroma SDK make sure you have it installed!\n");
		getchar();
		return 1;
	}
	printf("Succesfully loaded the ChromaSDK!\n");
	if (!loadChromaSDKFunctions()){
		printf("#ERROR > Failed to load the functions for the keyboard!\n");
		getchar();
		return 1;
	}

	if (!RegisterHotKey(NULL, 0x01, MOD_NOREPEAT | MOD_ALT, 0x57)){ //Register the CTRL+ALT+Q keyboard combination for the timer.
		printf("#ERROR > Failed to bind a hotkey... I guess I suck at c++ :(\n");
		getchar();
		return 1;
	}

	resetKeyboardEffect(); //Just a custom effect;

	MSG msg = { 0 };
	while (GetMessage(&msg, NULL, 0, 0) != 0)	//Get system messages
	{
		switch(msg.message)			//Windows lets us know that a hotkey has been pressed
		{
			case WM_HOTKEY:
				if (msg.wParam == 0x01){					//Is the hotkey ID 0x01, if so then its our start timer hotkey
					if (isTimerActive){						//Is the timer currently running? If so then stop it.
						printf("Countdown stopped\n");
						resetKeyboardEffect();
						KillTimer(NULL, BombCountdownTimer);		//Stop the timer.
						isTimerActive = !isTimerActive;			//Switch the boolean.
					}
					else{
						printf("Countdown started\n");
						countDown = 43.75;
						BombCountdownTimer = SetTimer(NULL, 1, 250, NULL);	//Start the timer
						isTimerActive = !isTimerActive;						//Switch the boolean.
					}
				}
				break;
			case WM_TIMER:
				updateCountdownVisuals();
				break;
		}
	}
}
