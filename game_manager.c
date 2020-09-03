#include <stdlib.h>
#include <stdio.h> // I/O terminal and user
#include <unistd.h> // 
#include <stdbool.h>
#include <time.h>

#include "xwin_sdl.h" // Working game display
//#include "scene_loader.h" // Load images to scenes and other data
//#include "mzapo_canvas.h" // Functions for canvas/display {placing/loading images}
#include "mzapo_controls.h"

// How much each frame should last
#define FRAME_TIME 0.03333 // 30 frames per second

// Should we quit game ?
bool QUIT_GAME = false;
float CURRENT_FPS;

int main(int argc, char** argv){

	/// Create all scenes and load neccessary data like all images ///
	SceneData* GM_ScenesData =  GetInnitialScenesData();
	//////////////////////////////////////////////////////////////////
	
	//////// Initialize game disaplay canvas, height, width /////////
	Canvas* GM_GameCanvas = malloc(sizeof(Canvas)); // Initialize game canvas
	if(GM_GameCanvas == NULL){
		printf("ERROR: Could not malloc 'GM_GameCanvas'\n");
		exit(EXIT_FAILURE);}
	// With this I don't have to malloc myself
	LoadPNG2Canvas("images/test_images/calibrate.png", GM_GameCanvas);
	//////////////////////////////////////////////////////////////////
	
	///// Initialize game display ////
    CanvasInnit(); // Initialize blank canvas
    //////////////////////////////////
    
    ///// Load menu scene
    LoadScene(GM_ScenesData, GM_GameCanvas, SCENE_MENU);
    /////////////////////
    
   
    
    
	
	// Game main loop
	while(!QUIT_GAME){	
		///////// HEAD //////////
		clock_t start = clock(); // START MEASURING TIME
		/////////////////////////
		
		// Refresh images, aim path, map on canvas
		RefreshScene(GM_ScenesData, GM_GameCanvas);
		
		
		
		// Redraw game display with canvas
		CanvasRefresh(GM_GameCanvas); 	
		
		// TEMPORARY CONTROLLS BEFORE BOARD BUILD
		
		// If we are animating ignore the input
		printf("GM_ScenesData[SCENE_GAME_MAP].animating = %d\n", GM_ScenesData[SCENE_GAME_MAP].animating);
		if(GM_ScenesData[SCENE_GAME_MAP].animating == false){
			char c;
			scanf("%c", &c);
			switch(c){
				case('a'):
					ActOnAction(GM_ScenesData, GM_GameCanvas, ACTION_BUTTON_1_MOVE_LEFT);
					break;
				case('s'):
					ActOnAction(GM_ScenesData, GM_GameCanvas, ACTION_BUTTON_1_CLICKED);
					break;
				case('d'):
					ActOnAction(GM_ScenesData, GM_GameCanvas, ACTION_BUTTON_1_MOVE_RIGHT);
					break;
					
				case('j'):
					ActOnAction(GM_ScenesData, GM_GameCanvas, ACTION_BUTTON_2_MOVE_LEFT);
					break;
				case('k'):
					ActOnAction(GM_ScenesData, GM_GameCanvas, ACTION_BUTTON_2_CLICKED);
					break;
				case('l'):
					ActOnAction(GM_ScenesData, GM_GameCanvas, ACTION_BUTTON_2_MOVE_RIGHT);
					break;

				case('q'):
					exit(0);
					break;
					
				case('n'):
					// NEXT TURN, increment turn, change players on turn
					NextTurn(GM_ScenesData);
					RefreshAimPath(GM_ScenesData, false, false); // Refresh aim_path_public data in scenedata
					break;
			}
		}else{
			// Animate
			ProjectileAnimationTick(GM_ScenesData);
			
			// If the explosion is triggered, deal damage to tanks and terrain
			if(GM_ScenesData[SCENE_GAME_MAP].projectile.to_explode){
				ExplodeEndAnimation(GM_ScenesData);	
				
				// Call for next turn
				NextTurn(GM_ScenesData);
				RefreshAimPath(GM_ScenesData, false, false); // Refresh aim_path_public data in scenedata
				
				// Refresh images, aim path, map on canvas
				RefreshScene(GM_ScenesData, GM_GameCanvas);
				RefreshAimPath(GM_ScenesData, false, false); // Refresh aim_path_public data in scenedata
				// Redraw game display with canvas
				CanvasRefresh(GM_GameCanvas); 	
				
			}else if(GM_ScenesData[SCENE_GAME_MAP].animating == false){
				// If the animation ended withouth explosion, call next turn
				NextTurn(GM_ScenesData);
				RefreshAimPath(GM_ScenesData, false, false); // Refresh aim_path_public data in scenedata
			}
			
			
			
			
		}
		
		// If one of the players won, show who won and after some delay, get back to main menu
		if(CURRENT_SCENE == SCENE_GAME_MAP){
			if(GM_ScenesData[SCENE_GAME_MAP].GameOver == true){
				// Refresh images, aim path, map on canvas
				RefreshScene(GM_ScenesData, GM_GameCanvas);
				
				// Show who won
				DrawWinningScreen(GM_ScenesData, GM_GameCanvas);
				
				
				// Redraw game display with canvas
				CanvasRefresh(GM_GameCanvas); 	
				
				// Delay
				msleep(WINNING_DELAY);
				
				// Get to main menu
				LoadScene(GM_ScenesData, GM_GameCanvas, SCENE_MENU);
			}
		}	
		
		
		
		//////////////////////////////////////////
		
		
		
		///////// END /////////
		// STOP MEASURING TIME, printf FPS if requiered
		clock_t end = clock(); 
		float seconds = (float)(end - start) / CLOCKS_PER_SEC; // Convert to seconds
		if(seconds < FRAME_TIME){
			msleep((long int)(FRAME_TIME-seconds));
			
			//set_and_printf_fps(FRAME_TIME); // Print FPS
		}else{
			//set_and_printf_fps(seconds); // Print FPS
		}
		////////////////////////
	}
	return 0;	
}
