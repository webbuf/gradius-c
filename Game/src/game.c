#include "baseTypes.h"
#include "input.h"
#include "application.h"
#include "framework.h"

#include "levelmgr.h"
#include "objmgr.h"

#include <time.h>
#include <Windows.h>

static void _gameInit();
static void _gameShutdown();
static void _gameDraw();
static void _gameUpdate(uint32_t milliseconds);

static bool killDaGame = false;

static LevelDef _levelDefs[] = {
	{
		{{0, 0}, {1024, 700}},		// fieldBounds
		0x00ff0000,					// fieldColor
		20,							// numBalls
		10,							// numFaces
		24							// numEnemies
	}
};
static Level* _curLevel = NULL;

/// @brief Program Entry Point (WinMain)
/// @param hInstance 
/// @param hPrevInstance 
/// @param lpCmdLine 
/// @param nCmdShow 
/// @return 
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	const char GAME_NAME[] = "Gradius";

	Application* app = appNew(hInstance, GAME_NAME, _gameDraw, _gameUpdate);

	if (app != NULL)
	{
		GLWindow* window = fwInitWindow(app);

		if (window != NULL)
		{
			_gameInit();

			bool running = true;

			while (running && !killDaGame)
			{
				running = fwUpdateWindow(window);
			}

			_gameShutdown();
			fwShutdownWindow(window);
		}

		appDelete(app);
	}
}

/// @brief Initialize code to run at application startup
static void _gameInit()
{
	srand((unsigned int) time(NULL));
	const uint32_t MAX_OBJECTS = 200;
	objMgrInit(MAX_OBJECTS);
	levelMgrInit();

	_curLevel = levelMgrLoad(&_levelDefs[0]);
}

/// @brief Cleanup the game and free up any allocated resources
static void _gameShutdown()
{
	levelMgrUnload(_curLevel);

	levelMgrShutdown();
	objMgrShutdown();
}

/// @brief Draw everything to the screen for current frame
static void _gameDraw() 
{
	objMgrDraw();
}

/// @brief Perform updates for all game objects, for the elapsed duration
/// @param milliseconds 
static void _gameUpdate(uint32_t milliseconds)
{
	// ESC exits the program
	if (inputKeyPressed(VK_ESCAPE))
	{
		killDaGame = true;
	}

	// F1 toggles fullscreen
	if (inputKeyPressed(VK_F1))
	{
		// TODO 
		//ToggleFullscreen(window);
	}

	objMgrUpdate(milliseconds);
}