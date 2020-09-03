#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include<time.h> 

#include "scene_loader.h" // Load images to scenes and other data

// x coordinates of player 1 and 2 spawn positions
int PLAYER_1_SPAWN_POSITIONS[4] = {30,60,90,120};
int PLAYER_2_SPAWN_POSITIONS[4] = {440,400,360,320};

/* Define attributes of our projectiles
   Each projectile has its own row and attributes (gravity,wind,velocity multiplier)  
	0- cannon {gravity multiplier, wind multiplier, velocity multiplier, damage, projectile radius, explosion radius}
	1- sniper {gravity multiplier, wind multiplier, velocity multiplier, damage, projectile radius, explosion radius}
*/
double PROJECTILE_ATTRIBUTES[PROJECTILE_TYPE_TYPE_TOTAL][PROJECTILE_ATTRIBUTES_TOTAL] = {
    {1, 4, 1, 20, 4, 70},
    {2, 4, 3, 40, 2, 35}
};

/*  This array encodes wind level speeds -> check WIND_LEVEL enum structure in 'scene_loader.h'
    If you want to change wind effect change attributes of weapon instead, dont change this array  */
double WIND_SPEEDS[WIND_LEVEL_TOTAL_LEVELS] = {-3, -2, -1, 0, 1, 2, 3};


// Color of our array map
#define SAND_COLOR_R 120
#define SAND_COLOR_G 255
#define SAND_COLOR_B 50

// Load all images to SceneData struct and return
SceneData* GetInnitialScenesData(){
	// Mallocate array. Data space for every scene
	SceneData* InnitScenesData = malloc(SCENE_TOTAL_COUNT * sizeof(SceneData)); // WILL HAVE TO BE DESTOYED ON QUIT
	
	if(InnitScenesData == NULL){ // Check malloc
		printf("ERROR: Could not mallocate 'InnitScenesData'\n");
		exit(EXIT_FAILURE);
	}
	
	// Load background images and others for every scene. Look 'scene_loader.h' for more info
	// MAIN MENU
	LoadPNG2Canvas(PATH_TO_IMG_SCENE_MENU, &(InnitScenesData[SCENE_MENU].scene_canvas));
	LoadPNG2Canvas(PATH_TO_IMG_SCOPE, &(InnitScenesData[SCENE_MENU].scope_canvas));
	
	// GAME MAP
	LoadPNG2Canvas(PATH_TO_GAME_MAP, &(InnitScenesData[SCENE_GAME_MAP].scene_canvas));
	LoadPNG2Canvas(PATH_TO_PLAYER_1_TANK, &(InnitScenesData[SCENE_GAME_MAP].player_1_tank));
	LoadPNG2Canvas(PATH_TO_PLAYER_2_TANK, &(InnitScenesData[SCENE_GAME_MAP].player_2_tank));
	// GAME MAP -> explosion	
	//LoadPNG2Canvas(PATH_TO_EXPLOSION_IMG, &(InnitScenesData[SCENE_GAME_MAP].explosion));
	
	InnitScenesData[1].scene_id = SCENE_MENU;
	
	// Malloc map array
	InnitScenesData[SCENE_GAME_MAP].game_map_array = malloc(DISPLAY_WIDTH*sizeof(unsigned int));
	
	// Set initial scene to unknown
	CURRENT_SCENE = -1;
	
	return 	InnitScenesData;
}


/*  This function will refresh entire scene, map, players, shot...  */
void RefreshScene(SceneData* scene_data, Canvas* game_canvas){

	// Load background
	RGB transparent = {.R = 0, .G = 0, .B = 0}; // Wont have effect on scene
	Canvas2CanvasFull(&(scene_data[CURRENT_SCENE].scene_canvas), game_canvas, 0, 0, false, &transparent);
	///////////////////
		

	if(CURRENT_SCENE == SCENE_MENU){ // We are currenty in main menu
		//////////// Draw text
		RGB color_black = {.R = 0, .G = 0, .B = 0};
		DrawText(game_canvas, "PLAY", 350, 70, 5, &color_black);
		DrawText(game_canvas, "WIND", 320, 110, 5, &color_black);
		if(scene_data[SCENE_MENU].wind_on == true){
			DrawText(game_canvas, "ON", 403, 110, 5, &color_black);
		}else{
			DrawText(game_canvas, "OFF", 390, 110, 5, &color_black);
		}
		DrawText(game_canvas, "TANKS", 340, 220, 5, &color_black);
		DrawText(game_canvas, "PLAYER 1:", 305, 250, 5, &color_black);
		DrawText(game_canvas, "PLAYER 2:", 305, 280, 5, &color_black);
		
		// Convert tanks count to string and draw text
		char p_1_tanks[4], p_2_tanks[4];
		sprintf(p_1_tanks, "%d\n", scene_data[SCENE_MENU].player_1_tanks);
		sprintf(p_2_tanks, "%d\n", scene_data[SCENE_MENU].player_2_tanks);
		DrawText(game_canvas, p_1_tanks, 440, 250, 5, &color_black);
		DrawText(game_canvas, p_2_tanks, 440, 280, 5, &color_black);
		/////////////////////////////////////////////////
		
		// Draw scope
		// Scope transparent background
		transparent.R = 254; transparent.G = 254; transparent.B = 254; 
		
		switch(scene_data[CURRENT_SCENE].knob_menu_position){
			case (KNOB_MENU_PLAY):
				Canvas2CanvasFull(&(scene_data[SCENE_MENU].scope_canvas), game_canvas, 300, 58, true, &transparent);
				break;
			case (KNOB_MENU_WIND):
				Canvas2CanvasFull(&(scene_data[SCENE_MENU].scope_canvas), game_canvas, 270, 98, true, &transparent);
				break;
			case (KNOB_MENU_PLAYER_1):
				Canvas2CanvasFull(&(scene_data[SCENE_MENU].scope_canvas), game_canvas, 250, 238, true, &transparent);
				break;
			case (KNOB_MENU_PLAYER_2):
				Canvas2CanvasFull(&(scene_data[SCENE_MENU].scope_canvas), game_canvas, 250, 268, true, &transparent);
				break;
		}
	}else if(CURRENT_SCENE == SCENE_GAME_MAP){
		
		// Fill map with sand
		FillCavnasWithMapArray(scene_data, game_canvas);
		
		// Set position for tanks
		SetAllTanksPosition(scene_data);
		
		// Add tank images to canvas 'game_canvas'
		FillCanvasWithTanks(scene_data, game_canvas);
		
		// Draw the text with current chosen projectile of active tank
		RGB color_black = {.R = 0, .G = 0, .B = 0};
		if(scene_data[SCENE_GAME_MAP].active_tank->chosen_projectile == PROJECTILE_TYPE_TYPE_CANNON){
			DrawText(game_canvas, "Weapon: Cannon", 160, 20, 3, &color_black);
		}else if(scene_data[SCENE_GAME_MAP].active_tank->chosen_projectile == PROJECTILE_TYPE_TYPE_SNIPER){
			DrawText(game_canvas, "Weapon: Sniper", 160, 20, 3, &color_black);
		}
		
		// Draw parabolic aiming path
		DrawAimPath(scene_data, game_canvas, &color_black);
		
		// Draw health bars
		DrawAllHealthBars(scene_data, game_canvas);
		
		// If we are animating, display the flying projectile
		if(scene_data[SCENE_GAME_MAP].animating){
			DrawProjectile(scene_data, game_canvas, &color_black);
		}
		
		// If wind is on, draw wind speed information
		if(scene_data[SCENE_GAME_MAP].wind_on){
			DrawWindSpeeds(scene_data, game_canvas, &color_black);
		}
	}
	return;
}

/*  Load scene settings  */
void LoadScene(SceneData* scene_data, Canvas* game_canvas, int scene_id){

	// Was previous scene 'SCEME_GAME_MAP' ? Free every mallocked memory from that scene
	if(CURRENT_SCENE == SCENE_GAME_MAP){
		ExitGameMap(scene_data);
	}
	

	CURRENT_SCENE = scene_id;
	if(scene_id == SCENE_MENU){
		scene_data[SCENE_MENU].last_action = ACTION_NO_ACTION; // Refresh actions -> no action
		scene_data[SCENE_MENU].knob_menu_position = KNOB_MENU_PLAY; // Set knob pos to default
		scene_data[SCENE_MENU].wind_on = false; // Wind is off
		scene_data[SCENE_MENU].player_1_tanks = 1;
		scene_data[SCENE_MENU].player_2_tanks = 1;
		scene_data[SCENE_MENU].player_max_tanks = PLAYER_MAX_TANKS;
		scene_data[SCENE_MENU].GameOver = false;
		
		scene_data[SCENE_GAME_MAP].animating = false;
		
		RefreshScene(scene_data, game_canvas);
	}else if(scene_id == SCENE_GAME_MAP){
		// Grab setting from menu
		scene_data[SCENE_GAME_MAP].last_action = scene_data[SCENE_MENU].last_action;
		scene_data[SCENE_GAME_MAP].knob_menu_position = KNOB_MENU_PLAY;
		scene_data[SCENE_GAME_MAP].wind_on = scene_data[SCENE_MENU].wind_on;
		scene_data[SCENE_GAME_MAP].player_1_tanks = scene_data[SCENE_MENU].player_1_tanks;
		scene_data[SCENE_GAME_MAP].player_2_tanks = scene_data[SCENE_MENU].player_2_tanks;
		scene_data[SCENE_GAME_MAP].player_max_tanks = PLAYER_MAX_TANKS;
		scene_data[SCENE_GAME_MAP].player_turn = PLAYER_1_TURN;
		scene_data[SCENE_GAME_MAP].Player_1_tank_ptr = 0;
		scene_data[SCENE_GAME_MAP].Player_2_tank_ptr = 0;
		scene_data[SCENE_GAME_MAP].turn_count = 0;
		scene_data[SCENE_GAME_MAP].GameOver = false;
		scene_data[SCENE_GAME_MAP].gravity = SCENE_GRAVITY;
		scene_data[SCENE_GAME_MAP].animating = false;
		
	
		scene_data[SCENE_GAME_MAP].player_1_tanks_obj = malloc(scene_data[SCENE_GAME_MAP].player_1_tanks*sizeof(Tank));
		scene_data[SCENE_GAME_MAP].player_2_tanks_obj = malloc(scene_data[SCENE_GAME_MAP].player_2_tanks*sizeof(Tank));
		

		
		// Reset 'confirmed movement' boolean
		scene_data[SCENE_GAME_MAP].movement_confirmed = false;
		

		
		if(scene_data[SCENE_GAME_MAP].player_1_tanks_obj == NULL || 
				scene_data[SCENE_GAME_MAP].player_2_tanks_obj == NULL){
			printf("ERROR: Could not mallocate 'player_1_tanks_obj'/'player_2_tanks_obj' returning to main menu");
			LoadScene(scene_data, game_canvas, SCENE_MENU);
		}
		
		// Fill current game map array, with mode 0
		FillGameMapArray(scene_data, 0);
		
		// For each tank, reset 'health', 'angle of fire', 'moves', 'projectile attributes' basciall spawn tanks      
		for(int t = 0; t < scene_data[SCENE_GAME_MAP].player_1_tanks; t++){
			scene_data[SCENE_GAME_MAP].player_1_tanks_obj[t].x = PLAYER_1_SPAWN_POSITIONS[t];
			scene_data[SCENE_GAME_MAP].player_1_tanks_obj[t].health = TANK_HEALTH;
			scene_data[SCENE_GAME_MAP].player_1_tanks_obj[t].angle_of_fire = ANGLE_OF_FIRE_MIN+40; // 40 degrees at the start
			scene_data[SCENE_GAME_MAP].player_1_tanks_obj[t].move_limit = TANK_MOVE_LIMIT;
			scene_data[SCENE_GAME_MAP].player_1_tanks_obj[t].velocity_of_projectile = PROJECTILE_VELOCITY_MAX/2;
			scene_data[SCENE_GAME_MAP].player_1_tanks_obj[t].chosen_projectile = PROJECTILE_TYPE_TYPE_CANNON;
			scene_data[SCENE_GAME_MAP].player_1_tanks_obj[t].is_alive = true;
		}
		for(int t = 0; t < scene_data[SCENE_GAME_MAP].player_2_tanks; t++){
			scene_data[SCENE_GAME_MAP].player_2_tanks_obj[t].x = PLAYER_2_SPAWN_POSITIONS[t];
			scene_data[SCENE_GAME_MAP].player_2_tanks_obj[t].health = TANK_HEALTH;
			scene_data[SCENE_GAME_MAP].player_2_tanks_obj[t].angle_of_fire = ANGLE_OF_FIRE_MAX-40; // 140 degrees at the start
			scene_data[SCENE_GAME_MAP].player_2_tanks_obj[t].move_limit = TANK_MOVE_LIMIT;
			scene_data[SCENE_GAME_MAP].player_2_tanks_obj[t].velocity_of_projectile = PROJECTILE_VELOCITY_MAX/2;
			scene_data[SCENE_GAME_MAP].player_2_tanks_obj[t].chosen_projectile = PROJECTILE_TYPE_TYPE_CANNON;
			scene_data[SCENE_GAME_MAP].player_2_tanks_obj[t].is_alive = true;
		}
		
		// Set active tank this depends on which turn it is 'player_turn' above in this function
		int tank_ptr;
		Tank* temp_tank_ptr;
		if(scene_data[SCENE_GAME_MAP].player_turn == PLAYER_1_TURN){
			tank_ptr = scene_data[SCENE_GAME_MAP].Player_1_tank_ptr;
			temp_tank_ptr = &(scene_data[SCENE_GAME_MAP].player_1_tanks_obj[tank_ptr]);
		}else if(scene_data[SCENE_GAME_MAP].player_turn == PLAYER_2_TURN){
			tank_ptr = scene_data[SCENE_GAME_MAP].Player_2_tank_ptr;
			temp_tank_ptr = &(scene_data[SCENE_GAME_MAP].player_2_tanks_obj[tank_ptr]);
		}
		scene_data[SCENE_GAME_MAP].active_tank = temp_tank_ptr;
		
		// Malloc parabolic aim path_public array as well as private aim path
		RefreshAimPath(scene_data, true, true);
		
		RefreshAimPath(scene_data, true, false);
		
		// Refresh aim path array which is already allocated (shown to player)
		// Draw parabolic aiming path
		RGB color_black = {.R = 0, .G = 0, .B = 0};
		DrawAimPath(scene_data, game_canvas, &color_black);
		
		////////////////////////////////////////////////////////////////////////////////////////
		
		RefreshScene(scene_data, game_canvas);
	}
	return;
}

/* Fill game map array into current scene with different modes
   modes - 0 line
         - 1 predefined array n.1
*/
void FillGameMapArray(SceneData* scene_data, int mode){
	
	if(mode == 0){
		int height = 55;
		for(int i = 0; i < DISPLAY_WIDTH; i++){
			scene_data[CURRENT_SCENE].game_map_array[i] = (DISPLAY_HEIGHT - height);
		}
	}else if(mode == 1){
		// Predefined array
	}
	
	return;
}

/*  Set tank y position. Tank will always be on top of our sand.  */
void SetAllTanksPosition(SceneData* scene_data){

	if(scene_data[CURRENT_SCENE].player_1_tanks_obj == NULL || 
	   scene_data[CURRENT_SCENE].player_2_tanks_obj == NULL){
		printf("ERROR: 'player_1_tanks_obj' or 'player_1_tanks_obj' are NULL\n");
		return;   
	}
	
	for(int t = 0; t < scene_data[CURRENT_SCENE].player_1_tanks; t++){
		int x = scene_data[CURRENT_SCENE].player_1_tanks_obj[t].x;
		scene_data[CURRENT_SCENE].player_1_tanks_obj[t].y = scene_data[CURRENT_SCENE].game_map_array[x];
	}
	for(int t = 0; t < scene_data[CURRENT_SCENE].player_2_tanks; t++){
		int x = scene_data[CURRENT_SCENE].player_2_tanks_obj[t].x;
		scene_data[CURRENT_SCENE].player_2_tanks_obj[t].y = scene_data[CURRENT_SCENE].game_map_array[x];
	}
}

/*  Fill our game canvas with sand  */
void FillCavnasWithMapArray(SceneData* scene_data, Canvas* game_canvas){
	
	RGB sand_color = {
		.R = SAND_COLOR_R,
		.G = SAND_COLOR_G,
		.B = SAND_COLOR_B
	};

	for(int x = 0; x < DISPLAY_WIDTH; x++){
		int sand_y = scene_data[CURRENT_SCENE].game_map_array[x];
		for(int y = sand_y; y < DISPLAY_HEIGHT; y++){
			ColorCanvasPixel(x, y, &sand_color, game_canvas);
		}
	}
}

// Add tank images to canvas 'game_canvas'
void FillCanvasWithTanks(SceneData* scene_data, Canvas* game_canvas){
	// Get tank count
	int player_1_tank_count = scene_data[CURRENT_SCENE].player_1_tanks;
	int player_2_tank_count = scene_data[CURRENT_SCENE].player_2_tanks;
	
	// Transparent color for tank is
	RGB transparent_color = {.R = 254, .G = 254, .B = 254};
	// Tank height is n px
	int tank_height = 9;
	
	// Display each tank on canvas, if its alive
	for(int t = 0; t < player_1_tank_count; t++){
		int x = scene_data[CURRENT_SCENE].player_1_tanks_obj[t].x;
		int y = scene_data[CURRENT_SCENE].player_1_tanks_obj[t].y;
		
		if(scene_data[CURRENT_SCENE].player_1_tanks_obj[t].is_alive == true){ // Is the tank alive?
			Canvas2CanvasFull(&(scene_data[CURRENT_SCENE].player_1_tank), game_canvas, x, y-tank_height, true, &transparent_color);
		}
		
	}
	// Display each tank on canvas, if its alive
	for(int t = 0; t < player_2_tank_count; t++){
		int x = scene_data[CURRENT_SCENE].player_2_tanks_obj[t].x;
		int y = scene_data[CURRENT_SCENE].player_2_tanks_obj[t].y;
		
		if(scene_data[CURRENT_SCENE].player_2_tanks_obj[t].is_alive == true){ // Is the tank alive?
			Canvas2CanvasFull(&(scene_data[CURRENT_SCENE].player_2_tank), game_canvas, x, y-tank_height, true, &transparent_color);
		}
		
	}
	return;
}

/*  Move active tank to the left if 'left'==true or right otherwise.  */
void MoveTank(SceneData* scene_data, bool left){
	Tank* tank; // Tank ptr
	
	// Choose tank based on turns at the moment
	if(scene_data[CURRENT_SCENE].player_turn == PLAYER_1_TURN){
		printf("Moving tank PLAYER 1\n");
		// Choose active tank from player 1
		int tank_ptr = scene_data[CURRENT_SCENE].Player_1_tank_ptr;
		tank = &(scene_data[CURRENT_SCENE].player_1_tanks_obj[tank_ptr]);
	}else{
		printf("Moving tank PLAYER 2\n");
		// Choose active tank from player 2
		int tank_ptr = scene_data[CURRENT_SCENE].Player_2_tank_ptr;
		tank = &(scene_data[CURRENT_SCENE].player_2_tanks_obj[tank_ptr]);
	}
	
	if(tank->move_limit > 0){
		if(left){
			if(tank->x > 0){
				tank->x--;
				tank->move_limit--;
			}
		}else{
			if(tank->x <= DISPLAY_WIDTH-20){
				tank->x++;
				tank->move_limit--;
			}
		}
		
		printf("New tank.move_limit: %d\n", tank->move_limit);
	}else{
		printf("You dont have enought moves\n");
	}
	return;
}

/*  This function changes the tank and player in control  */
void NextTurn(SceneData* scene_data){

	// Change turns
	if(scene_data[CURRENT_SCENE].player_turn == PLAYER_1_TURN){
	
		int new_tank_idx = ReturnIndexToLiveTank(scene_data, PLAYER_2_TURN);
		
		if(new_tank_idx == -1){
			// Player 1 won
			scene_data[CURRENT_SCENE].GameOver = true;
			printf("!GAME OVER! - player 1 won\n");
			return;
		}else{
			
			// Check if We didn't destroy ourselves
			int my_tanks_idx = ReturnIndexToLiveTank(scene_data, PLAYER_1_TURN);
			if(my_tanks_idx == -1){
				// We destroy ourselves
				scene_data[CURRENT_SCENE].player_turn = PLAYER_2_TURN;
				scene_data[CURRENT_SCENE].GameOver = true;
				printf("!GAME OVER! - player 2 won\n");
				return;
			}
		
		
			scene_data[CURRENT_SCENE].Player_2_tank_ptr = new_tank_idx;
			scene_data[CURRENT_SCENE].player_turn = PLAYER_2_TURN;
			
			// Set the active tank
			scene_data[CURRENT_SCENE].active_tank = &(scene_data[CURRENT_SCENE].player_2_tanks_obj[new_tank_idx]);
		}
		
	}else{
	
		int new_tank_idx = ReturnIndexToLiveTank(scene_data, PLAYER_1_TURN);
		
		if(new_tank_idx == -1){
			// Player 2 won
			scene_data[CURRENT_SCENE].GameOver = true;
			printf("!GAME OVER! - player 2 won\n");
			return;
		}else{
		
			
			// Check if We didn't destroy ourselves
			int my_tanks_idx = ReturnIndexToLiveTank(scene_data, PLAYER_2_TURN);
			if(my_tanks_idx == -1){
				// We destroy ourselves
				scene_data[CURRENT_SCENE].player_turn = PLAYER_1_TURN;
				scene_data[CURRENT_SCENE].GameOver = true;
				printf("!GAME OVER! - player 1 won\n");
				return;
			}
		
		
			scene_data[CURRENT_SCENE].Player_1_tank_ptr = new_tank_idx;
			scene_data[CURRENT_SCENE].player_turn = PLAYER_1_TURN;
			
			// Set the active tank
			scene_data[CURRENT_SCENE].active_tank = &(scene_data[CURRENT_SCENE].player_1_tanks_obj[new_tank_idx]);
		}
	}
	
	// Set new wind speed if the wind is on
	if(scene_data[CURRENT_SCENE].wind_on){
		SetWindSpeed(scene_data);
	}
	
	scene_data[CURRENT_SCENE].turn_count++;
	ResetTanksMoveLimit(scene_data);
	// Reset our 'confirm movement' bool var
	scene_data[CURRENT_SCENE].movement_confirmed = false;
	
	return;
}

/*  This functions returns the index of the next live tank in array of tanks  
	-1 if there is no live tank.  */
int ReturnLiveTankIdx(Tank* tanks, int tanks_count, int old_index){
	int index = old_index;
	
	// If tanks_count reaches zero, that means we iterated full circle
	while(tanks_count > 0){
		// Go to next valid index
		if((index+1) >= tanks_count){
			index = 0;
		}else{
			index++;
		}
		
		// Check if the tank is dead
		if(tanks[index].health <= 0){
			// It's dead
			tanks_count--;
		}else{
			// It's live, return index
			printf("returning tank index: %d\n", index);
			return index;
		}
	}
	return -1;
}

/*  Reset move limits on all tanks  */
void ResetTanksMoveLimit(SceneData* scene_data){

	for(int t = 0; t < scene_data[CURRENT_SCENE].player_1_tanks; t++){
		scene_data[CURRENT_SCENE].player_1_tanks_obj[t].move_limit = TANK_MOVE_LIMIT;
	}
	for(int t = 0; t < scene_data[CURRENT_SCENE].player_2_tanks; t++){
		scene_data[CURRENT_SCENE].player_2_tanks_obj[t].move_limit = TANK_MOVE_LIMIT;
	}
	return;
}

/*  This function increases current active tank barrel angle  
	if add_deg == true move barrel from 0 def to 180 deg
	else move barrel from 180 deg to 0 deg by ANGLE_OF_FIRE_STEP degree  */
void MoveBarrel(SceneData* scene_data, bool add_deg){
	// Active tank
	Tank* tank = scene_data[SCENE_GAME_MAP].active_tank;
	
	if(add_deg == true){
		if(tank->angle_of_fire >= ANGLE_OF_FIRE_MAX){
			printf("Barrel reached its maximum angle!\n");
		}else{
			// Increase barrel angle by ANGLE_OF_FIRE_STEP deg
			tank->angle_of_fire += ANGLE_OF_FIRE_STEP;
		}
	}else{
		if(tank->angle_of_fire <= ANGLE_OF_FIRE_MIN){
			printf("Barrel reached its minimum angle!\n");
		}else{
			// Decrease barrel angle by ANGLE_OF_FIRE_STEP deg
			tank->angle_of_fire -= ANGLE_OF_FIRE_STEP;
		}
	}
	printf("Barrel angle: %.2f\n", tank->angle_of_fire);
	
	return;
}

/*  This function increases current active tank projectile velocity  
	if add_velocity == true, increase the velocity
	else decreade projectile velocity by PROJECTILE_VELOCITY_STEP degree  */
void ChangeProjectileVelocity(SceneData* scene_data, bool add_velocity){
	// Active tank
	Tank* tank = scene_data[SCENE_GAME_MAP].active_tank;
	
	if(add_velocity == true){
		if(tank->velocity_of_projectile >= PROJECTILE_VELOCITY_MAX){
			printf("Reached projectile velocity maximum!\n");
		}else{
			// Increase projectile velocity by PROJECTILE_VELOCITY_STEP
			tank->velocity_of_projectile += PROJECTILE_VELOCITY_STEP;
		}
	}else{
		if(tank->velocity_of_projectile <= PROJECTILE_VELOCITY_MIN){
			printf("Reached projectile velocity minimum!\n");
		}else{
			// Decrease projectile velocity by PROJECTILE_VELOCITY_STEP
			tank->velocity_of_projectile -= PROJECTILE_VELOCITY_STEP;
		}
	}
	printf("Projectile velocity: %.2f\n", tank->velocity_of_projectile);
	
	return;
}

/*  This function changes current weapon for the active tank  */
void ChangeWapon(SceneData* scene_data){
	if(scene_data[CURRENT_SCENE].active_tank->chosen_projectile < (PROJECTILE_TYPE_TYPE_TOTAL - 1)){
		scene_data[CURRENT_SCENE].active_tank->chosen_projectile++;
	}else{
		scene_data[CURRENT_SCENE].active_tank->chosen_projectile = 0;
	}
	return;
}

/*  This function calculates parabola.
	For each time delta of size 'ARRAY_PATH_TIME_CHANGE' seconds, it will calculate positions x,y.
	We will specify the number of positions (time stamps) we want, which then also corresponds to the flight
	time. If the path gets ouf of left,right, bottom bounds it will end. -> upper boundrary not included
	I wanted to make this function more generic so it could be possibly used for other things.
	-Option 'should_malloc' will allocate our new path to array public or private according to arguments.
	-Option 'private_path' decides which path is calculated public or private one
	 */
void CalculateParabolicPath(SceneData* scene_data, int max_positions, 
							int start_x, int start_y, 
							double angle, double gravity, double wind_speed, double velocity, 
							bool wind,
							bool should_malloc,
							bool private_path
							){
	/* We will calculate this parabola in conventional cartesian coordinate system
	   we will have to map those coordinates to our display grid with different values.  */
	int valid_size = max_positions; //  <= max_positions. Size depends on duration of flight 
	
	Parabola* parabola;
	if(should_malloc){
		// Malloc our Parabola datatype
		parabola = malloc(sizeof(Parabola));
		if(parabola == NULL){
			printf("ERROR: Could not malloc 'Parabola' in CalculateParabolicPath!\n");
			exit(EXIT_FAILURE);
		}
		
		// Malloc our array in which we will store our pixels
		parabola->parabolic_path = malloc(max_positions*2*sizeof(short int)); // Each pixel has 2 coordinates
		
		
		if(parabola->parabolic_path == NULL){
			printf("ERROR: Could not malloc 'parabolic_path' in CalculateParabolicPath!\n");
			exit(EXIT_FAILURE);
	}
	}else{
		if(private_path){
			parabola = scene_data[SCENE_GAME_MAP].aim_path_private;
		}else{
			parabola = scene_data[SCENE_GAME_MAP].aim_path_public;
			// If we have public parabola, player doesnt see effect of the wind 
			wind = false;
		}
	}
	
	
	////////// START CALCULATION ///////////
	double converter = PI / 180; // Convert degrees to radians

	double v_x = velocity*cos(angle*converter); // horizontal velocity, along x-axis
	double v_y = velocity*sin(angle*converter); // vertical velocity, along y-axis
	
	for(int i = 0; i < max_positions; i++){
		short int new_x, new_y; // Variables for new positions
		double time_delta = (double)i*ARRAY_PATH_TIME_CHANGE;
		
		// Calculate new x coordinate  // s = start_x + v*t + wind_v*t   // wind has negative speed when it moves to the left 
		if(wind){ // WIND == ON
			new_x = (short int)(start_x + (v_x*time_delta) + (wind_speed*time_delta));
		}else{ // WIND == OFF
			new_x = (short int)(start_x + (v_x*time_delta));
			printf("new_x: %d\n", new_x);
			printf("v_x*time_delta: %f\n", v_x*time_delta);
			printf("v_x: %f\n", v_x);
			printf("time_delta: %f\n", time_delta);
		}
		
		// Calculate new y coordinate // s =  start_y + v0*t + (1/2)*g*t^2 
		new_y = (short int)(start_y + (v_y*time_delta) + (0.5)*gravity*time_delta*time_delta);

		// Check bounds, break if needed
		if((new_y <= 0) || (new_x <= 0) || (new_x >= DISPLAY_WIDTH)){
			valid_size = i-1; // Our size of valid pixels will be smaller,
			break;
		}else{
			parabola->parabolic_path[(i*2)] = new_x;
			parabola->parabolic_path[(i*2)+1] = new_y;
		}
	}
	////////////////////////////////////////
	
	// Write final valid size to parabola
	parabola->number_of_pairs = valid_size;
	
	if(should_malloc){
		if(private_path){
			scene_data[SCENE_GAME_MAP].aim_path_private = parabola;
		}else{
			scene_data[SCENE_GAME_MAP].aim_path_public = parabola;
		}
	}
	return;
}

/*  This function calls function 'CalculateParabolicPath'
	If 'should_malloc' is true the new path will be mallock'd to new array and saved
	accordingly to 'aim_path_private' if 'private_path' == true, or save it to 
	'aim_path_public' if 'private_path' == false  */
void RefreshAimPath(SceneData* scene_data, bool should_malloc, bool private_path){
	// Calculate aim path for active tank
	Tank* tank = scene_data[SCENE_GAME_MAP].active_tank;

	// How many time steps to calculate
	int max_positions; 
	if(private_path){
		max_positions = AIM_PATH_PRIVATE_ARRAY_MAX_SIZE;
	}else{
		max_positions = AIM_PATH_PUBLIC_PIXELS_SHOW;
	}
	
	// Load physics multipliers based on current chose weapon
	int gravity_m = PROJECTILE_ATTRIBUTES[tank->chosen_projectile][PROJECTILE_G_MULTIPLIER];
	int wind_m = PROJECTILE_ATTRIBUTES[tank->chosen_projectile][PROJECTILE_W_MULTIPLIER];
	int velocity_m = PROJECTILE_ATTRIBUTES[tank->chosen_projectile][PROJECTILE_V_MULTIPLIER];
	
	CalculateParabolicPath(scene_data,
		max_positions,
		(tank->x) + TANK_BARREL_X_OFFSET, 
		(DISPLAY_HEIGHT-(tank->y)) + TANK_BARREL_Y_OFFSET, 
		tank->angle_of_fire, 
		scene_data[SCENE_GAME_MAP].gravity*gravity_m, 
		WIND_SPEEDS[scene_data[SCENE_GAME_MAP].wind_speed_index]*wind_m, 
		(tank->velocity_of_projectile)*velocity_m, 
		scene_data[SCENE_GAME_MAP].wind_on,
		should_malloc, // if should_malloc == false -> don't malloc, save it to scene_data aim_path_public/private
		private_path // if true, save the data to private path
		);

	return;
}

/*  This function draws 'aim_path_public' from scene_data onto canvas from the
	onto active tank  */
void DrawAimPath(SceneData* scene_data, Canvas* game_canvas, RGB* color){
	Parabola* parabola = scene_data[SCENE_GAME_MAP].aim_path_public;
	for(int pair = 0; pair < parabola->number_of_pairs; pair++){
		if(pair >= AIM_PATH_PUBLIC_PIXELS_SHOW){
			break; // Show only few pixels
		}
	    short int xx = (short int)parabola->parabolic_path[(pair*2)];
	    short int yy = DISPLAY_HEIGHT-(short int)parabola->parabolic_path[(pair*2)+1];

		CanvasCircle(xx, yy, 2, color, game_canvas);
	}
}

/*  This function randomly sets wind speed. This game will have 'WIND_LEVEL_TOTAL_LEVELS' 
	levels of wind speeds. For now: <<<, <<, <, (.) , >, >>, >>> */
void SetWindSpeed(SceneData* scene_data){
	// Pick random number bewteen 0 and WIND_LEVEL_TOTAL_LEVELS
	srand(time(0)); 
  	int random_index = rand()%WIND_LEVEL_TOTAL_LEVELS;
 	
	/* Then set wind speed index 'scene_data' to chosen index 
	   of the 'WIND_SPEEDS' array element  */
	scene_data[CURRENT_SCENE].wind_speed_index = random_index;
	
	return;
}

/*  This function encodes wind speed information to display and informs user of 
    the wind situation  */
void DrawWindSpeeds(SceneData* scene_data, Canvas* game_canvas, RGB* color){

	// Draw wind speed to display
	switch(scene_data[CURRENT_SCENE].wind_speed_index){
		case WIND_LEVEL_STRONGEST_LEFT:
			DrawText(game_canvas, "Wind: <<<", 30, 20, 5, color);
			break;
		case WIND_LEVEL_STRONG_LEFT:
			DrawText(game_canvas, "Wind:  <<", 30, 20, 5, color);
			break;
		case WIND_LEVEL_WEAKER_LEFT:
			DrawText(game_canvas, "Wind:   <", 30, 20, 5, color);
			break;
		case WIND_LEVEL_NONE:
			DrawText(game_canvas, "Wind:  | ", 30, 20, 5, color);
			break;
		case WIND_LEVEL_WEAKER_RIGHT:
			DrawText(game_canvas, "Wind:   >", 30, 20, 5, color);
			break;
		case WIND_LEVEL_STRONG_RIGHT:
			DrawText(game_canvas, "Wind:  >>", 30, 20, 5, color);
			break;
		case WIND_LEVEL_STRONGEST_RIGHT:
			DrawText(game_canvas, "Wind: >>>", 30, 20, 5, color);
			break;
	}
	return;
}

/*  This function will initialize default state for the projectile and starts animation.  */
void StartProjectileAnimation(SceneData* scene_data){
	// Load projectile and tank then fill data to projectile in scene_data
	Projectile* projectile = &(scene_data[SCENE_GAME_MAP].projectile);
	Tank* tank = scene_data[SCENE_GAME_MAP].active_tank;
	
	projectile->type = tank->chosen_projectile; // Copy current weapon data
	projectile->to_explode = false; // Dont explode yet
	projectile->exploded = false;
	projectile->explosion_current_frames = 0; // Animation frame -> 0
	
	// Calculate copy and set animation path data
	RefreshAimPath(scene_data, false, true); // Calculate priva path data
	projectile->parabola = scene_data[SCENE_GAME_MAP].aim_path_private;
	projectile->projectile_path_pairs_index = 0; // Where are we in the animation path
	
	// Start animation
	scene_data[SCENE_GAME_MAP].animating = true;
	
	printf("Projectile animation activated!\n");
	
	return;
}

/*  This function moves the projectile in animation frame by frame and checks if it
    should explode and end animation.  */
void ProjectileAnimationTick(SceneData* scene_data){

	printf("Projectile animation tick!\n");
	
	// Load projectile
	Projectile* projectile = &(scene_data[SCENE_GAME_MAP].projectile);
	
	// Give the projectile new index pair if there is a valid one left
	int total_pairs = (projectile->parabola)->number_of_pairs;
	int current_index = projectile->projectile_path_pairs_index;
	
	printf("current_index: %d  total_pairs: %d\n", current_index, total_pairs);
	
	if(total_pairs > current_index){
		projectile->projectile_path_pairs_index++;
	}else{
		// End of animation
		printf("Last animation point reached!\n");
		
		// For now close animation
		scene_data[SCENE_GAME_MAP].animating = false;
	}
	
	// Refresh the distances between projectile and players
	RefreshProjectilePlayersDistance(scene_data);
	// Refresh the distance between projectile and the ground
	RefreshProjectileGroundDistance(scene_data);
	// Check if the projectile should explode
	ShouldProjectileExplode(scene_data);
	if(projectile->to_explode){
		printf("Starting the explosion\n");
	}
	
	// Delay the animation a bit
	msleep(PROJECTILE_ANIMATION_DELAY);

	return;
}

/*  This function draws current position of projectile onto canvas.
	Draws a circle and arguments are color and radius of the circle  */
void DrawProjectile(SceneData* scene_data, Canvas* game_canvas, RGB* color){
	
	// Load projectile
	Projectile* projectile = &(scene_data[SCENE_GAME_MAP].projectile);
	
	// Get the data about position and type of projectile
	short int x = (projectile->parabola)->parabolic_path[(projectile->projectile_path_pairs_index)*2];
	short int y = DISPLAY_HEIGHT-(projectile->parabola)->parabolic_path[(projectile->projectile_path_pairs_index)*2 + 1];
	int radius = PROJECTILE_ATTRIBUTES[projectile->type][PROJECTILE_RADIUS];
	CanvasCircle(x, y, radius, color, game_canvas);
	
	printf("Drawing projectile!\n");

	return;
}

/*  This function calculates distance of projectile from players and saves it to	
	each tank  */
void RefreshProjectilePlayersDistance(SceneData* scene_data){
	// Load projectile and get the x and y coordinates
	Projectile* projectile = &(scene_data[SCENE_GAME_MAP].projectile);
	int aim_path_index = projectile->projectile_path_pairs_index;
	Parabola* projectile_parabola = projectile->parabola;
	
	int projectile_x = projectile_parabola->parabolic_path[aim_path_index*2];
	int projectile_y = projectile_parabola->parabolic_path[aim_path_index*2 + 1];
	//////////////////////////////////////////////////		
	
	// Check only those tanks which are alive, others will have distance -1
	int p1_tanks_total = scene_data[SCENE_GAME_MAP].player_1_tanks;
	int p2_tanks_total = scene_data[SCENE_GAME_MAP].player_2_tanks;

	for(int t = 0; t < p1_tanks_total; t++){
		Tank* temp_tank = &(scene_data[SCENE_GAME_MAP].player_1_tanks_obj[t]);
		
		// If the tank is dead, set distance to -1;
		if(temp_tank->is_alive == false){
			temp_tank->distance_from_projectile = -1;
		}else{
			// Calculate distance
			int tank_x = temp_tank->x;
			int tank_y = temp_tank->y;

			temp_tank->distance_from_projectile = GetDistance(tank_x, DISPLAY_HEIGHT-tank_y, projectile_x, projectile_y);

		}
		printf("Player 1 distances from projectile: %.2f\n",temp_tank->distance_from_projectile);
		
	}
	for(int t = 0; t < p2_tanks_total; t++){
		Tank* temp_tank = &(scene_data[SCENE_GAME_MAP].player_2_tanks_obj[t]);
		
		// If the tank is dead, set distance to -1;
		if(temp_tank->is_alive == false){
			temp_tank->distance_from_projectile = -1;
		}else{
			// Calculate distance
			int tank_x = temp_tank->x;
			int tank_y = temp_tank->y;

			temp_tank->distance_from_projectile = GetDistance(tank_x, DISPLAY_HEIGHT-tank_y, projectile_x, projectile_y);

		}
		printf("Player 2 distances from projectile: %.2f\n",temp_tank->distance_from_projectile);
	}
	
	return;
}

/*  This function calculates immediate distance of projectile from the ground  */
void RefreshProjectileGroundDistance(SceneData* scene_data){
	// Load projectile and get the x and y coordinates
	Projectile* projectile = &(scene_data[SCENE_GAME_MAP].projectile);
	int aim_path_index = projectile->projectile_path_pairs_index;
	Parabola* projectile_parabola = projectile->parabola;
	
	int projectile_x = projectile_parabola->parabolic_path[aim_path_index*2];
	int projectile_y = projectile_parabola->parabolic_path[aim_path_index*2 + 1];
	//////////////////////////////////////////////////		
	
	/* We will use the x index of parabola to get the y information about the
	   height from array map  */
	int ground_distance = projectile_y - (DISPLAY_HEIGHT - scene_data[SCENE_GAME_MAP].game_map_array[projectile_x]);
	
	printf("Distance from the ground is: %d\n", ground_distance);
	// Save the height info to projectile
	projectile->distance_from_ground = ground_distance;
	
	return;
}

/*  This function calculates distance between two points {x1,y1,x2,y2}  */
double GetDistance(int x1, int y1, int x2, int y2){
	int y_dist = abs(y2-y1);
	int x_dist = abs(x2-x1);
	printf("y_dist = %d  x_dist = %d\n", y_dist, x_dist);
	double distance =  ((double)sqrt(y_dist*y_dist + x_dist*x_dist));
	printf("Distance calculated: %f\n", distance);
	return distance;
}


/*  This function will decide if the projectile should explode.
	If it should, 'to_explode' variable in projectile will be set to true*/
void ShouldProjectileExplode(SceneData* scene_data){

	// Load projectile
	Projectile* projectile = &(scene_data[SCENE_GAME_MAP].projectile);
	
	// Iterate for each living tank and decide if the projectile should explode
	int p1_tanks_total = scene_data[SCENE_GAME_MAP].player_1_tanks;
	int p2_tanks_total = scene_data[SCENE_GAME_MAP].player_2_tanks;

	for(int t = 0; t < p1_tanks_total; t++){
		Tank* temp_tank = &(scene_data[SCENE_GAME_MAP].player_1_tanks_obj[t]);
		
		if(temp_tank->is_alive == true){ // Is the tank alive?
			if((temp_tank->distance_from_projectile) < PROJECTILE_TANK_DISTANCE_EXPLOSION_TRIGER){
				// EXPLODE
				printf("Projectile should explode!!!\n");
				projectile->to_explode = true; 
				return;
			}
		}
	}
	for(int t = 0; t < p2_tanks_total; t++){
		Tank* temp_tank = &(scene_data[SCENE_GAME_MAP].player_2_tanks_obj[t]);
		
		if(temp_tank->is_alive == true){ // Is the tank alive?
			if((temp_tank->distance_from_projectile) < PROJECTILE_TANK_DISTANCE_EXPLOSION_TRIGER){
				// EXPLODE
				printf("Projectile should explode!!!\n");
				projectile->to_explode = true; 
				return;
			}
		}
	}
	///////////////////////////////////////////////////////////////////////////
	
	// Check the ground distance and decide if the projectile should explode
	if((projectile->distance_from_ground) < PROJECTILE_GROUND_DISTANCE_EXPLOSION_TRIGER){
		// EXPLODE
		printf("Projectile should explode!!!\n");
		projectile->to_explode = true;
		return; 
	}
	return;
}

/*  This function dealth damage to its surroundings, player tanks and terrain  */
void ProjectileDoDamage(SceneData* scene_data){
	
	// Load projectile
	Projectile* projectile = &(scene_data[SCENE_GAME_MAP].projectile);
	
	// Distance up to which players and terrain is damaged
	int explosion_radius = PROJECTILE_ATTRIBUTES[projectile->type][PROJECTILE_EXPLOSION_RADIUS];
	
	// Damage the projectile will make to players around (distance < explosion_radius)
	int projectile_damage = PROJECTILE_ATTRIBUTES[projectile->type][PROJECTILE_DAMAGE];
	
	/* Iterate through all the tanks and if the tank is alive
	   subtract their health with 'projectile_damage'*/
	int p1_tanks_total = scene_data[SCENE_GAME_MAP].player_1_tanks;
	int p2_tanks_total = scene_data[SCENE_GAME_MAP].player_2_tanks;

	for(int t = 0; t < p1_tanks_total; t++){
		Tank* temp_tank = &(scene_data[SCENE_GAME_MAP].player_1_tanks_obj[t]);
		
		if(temp_tank->is_alive == true){ // Is the tank alive?
			printf("temp_tank->distance_from_projectile <= explosion_radius :::  %f  <=   %d\n", temp_tank->distance_from_projectile, explosion_radius);
			if((temp_tank->distance_from_projectile) <= explosion_radius){
				// Subtract health
				printf("Subtracting health!  old health: %d  ", temp_tank->health);
				
				// Do damage linearly
				double percentage_to_deal = (explosion_radius-temp_tank->distance_from_projectile)/(explosion_radius);
				int dmg = percentage_to_deal*projectile_damage;
				
				temp_tank->health -= dmg;
				printf("new health: %d\n", temp_tank->health);
				
				// Kill the tank if the new health is <= 0
				if(temp_tank->health <= 0){
					temp_tank->is_alive = false;
				}
			}
		}
	}
	for(int t = 0; t < p2_tanks_total; t++){
		Tank* temp_tank = &(scene_data[SCENE_GAME_MAP].player_2_tanks_obj[t]);
		
		if(temp_tank->is_alive == true){ // Is the tank alive?
			printf("temp_tank->distance_from_projectile <= explosion_radius :::  %f  <=   %d\n", temp_tank->distance_from_projectile, explosion_radius);
			if((temp_tank->distance_from_projectile) <= explosion_radius){
				// Subtract health
				printf("Subtracting health!  old health: %d  ", temp_tank->health);
				
				// Do damage linearly
				double percentage_to_deal = (explosion_radius-temp_tank->distance_from_projectile)/(explosion_radius);
				int dmg = percentage_to_deal*projectile_damage;
				
				temp_tank->health -= dmg;
				printf("new health: %d\n", temp_tank->health);
				
				// Kill the tank if the new health is <= 0
				if(temp_tank->health <= 0){
					temp_tank->is_alive = false;
				}
				
			}
		}
	}
	
	// ADD TERRAIN DAMAGE
	
	/* Iterate through the whole map array and get the distance from projectile to
	   the top of the map in each point  */
	   
	// Load projectile and get the x and y coordinates
	int aim_path_index = projectile->projectile_path_pairs_index;
	Parabola* projectile_parabola = projectile->parabola;
	
	int projectile_x = projectile_parabola->parabolic_path[aim_path_index*2];
	int projectile_y = projectile_parabola->parabolic_path[aim_path_index*2 + 1];
	
	printf("PROJECTILE x: %d  y:  %d\n", projectile_x, projectile_y);
	//////////////////////////////////////////////////	
	
	for(int x = 0; x < DISPLAY_WIDTH; x++){
		// Get y position of terraint
		int y = DISPLAY_HEIGHT-scene_data[SCENE_GAME_MAP].game_map_array[x];
		
		printf("TERRAIN x: %d  y:  %d\n", x, y);
		
		// Get the distance from parabola to terrain point
		int distance = (int)GetDistance(projectile_x, projectile_y, x, y);
		printf("GOT DISTANCE: %d\n", distance);
		/* If the distance is withing the explosion radius or the explosion occured
		   under the terrain -> remove chunks of terrain.  */
		if(distance < explosion_radius){
			// The closer to the center, remove more chunks of terrain
			int to_remove = abs((int)((explosion_radius - distance)*TERRAIN_REMOVER_MULTIPLIER));
			printf("REMOVE: %d px\n", to_remove);
			if((scene_data[SCENE_GAME_MAP].game_map_array[x] + to_remove)  >= (DISPLAY_HEIGHT-TERRAIN_BOTTOM_BORDER)){
				scene_data[SCENE_GAME_MAP].game_map_array[x] = DISPLAY_HEIGHT-TERRAIN_BOTTOM_BORDER;
			}else{
				scene_data[SCENE_GAME_MAP].game_map_array[x] += to_remove;
			}
		}else if((projectile_y < (y-explosion_radius)) && (abs(projectile_x - x) <= explosion_radius)){
			int to_remove = abs((int)((explosion_radius - projectile_x)*TERRAIN_REMOVER_MULTIPLIER));
			printf("REMOVE: %d px\n", to_remove);
			if((scene_data[SCENE_GAME_MAP].game_map_array[x] + to_remove)  >= (DISPLAY_HEIGHT-TERRAIN_BOTTOM_BORDER)){
				scene_data[SCENE_GAME_MAP].game_map_array[x] = DISPLAY_HEIGHT-TERRAIN_BOTTOM_BORDER;
			}else{
				scene_data[SCENE_GAME_MAP].game_map_array[x] += to_remove;
			}
		}
		
	}
	
	
	
	return;
}

/*  This function takes care of finalizing the explosion. Calls function for
    dealing area damage  'ProjectileDoDamage' and sets explosion picture for a few
    frames.  */
void ExplodeEndAnimation(SceneData* scene_data){
	
	// Deal damage
	printf("Dealing damage!\n");
	ProjectileDoDamage(scene_data);
	
	// Turn off animation sequence
	scene_data[SCENE_GAME_MAP].animating = false;
	
	return;
}

/*  This function iterates through all the live tanks and call function 
	'DrawSingleHealthBar' to draw a health bar over them  */
void DrawAllHealthBars(SceneData* scene_data, Canvas* game_canvas){
	// Iterate through each tank and if the tank is alive, draw a health bar over it
	
	int p1_tanks_total = scene_data[SCENE_GAME_MAP].player_1_tanks;
	int p2_tanks_total = scene_data[SCENE_GAME_MAP].player_2_tanks;

	for(int t = 0; t < p1_tanks_total; t++){
		Tank* temp_tank = &(scene_data[SCENE_GAME_MAP].player_1_tanks_obj[t]);
		
		if(temp_tank->is_alive == true){ // Is the tank alive?
			DrawSingleHealthBar(temp_tank, game_canvas);
		}
	}
	for(int t = 0; t < p2_tanks_total; t++){
		Tank* temp_tank = &(scene_data[SCENE_GAME_MAP].player_2_tanks_obj[t]);
		
		if(temp_tank->is_alive == true){ // Is the tank alive?
			// Draw health bar over that tank
			DrawSingleHealthBar(temp_tank, game_canvas);
		}
	}

	return;
}

/*  This function takes tank as an argument and draws healthbar over it in game_canvas  */
void DrawSingleHealthBar(Tank* tank, Canvas* game_canvas){
	
	// Starting position for drawing
	int start_x = tank->x - HEALTH_BAR_X_OFFSET;
	int start_y = tank->y - HEALTH_BAR_Y_OFFSET;
	
	// Health and border color
	RGB inner_color = {.R = HEALTH_BAR_INNER_COLOR_R, .G = HEALTH_BAR_INNER_COLOR_G, .B = HEALTH_BAR_INNER_COLOR_B}; 
	RGB border_color = {.R = HEALTH_BAR_BORDER_COLOR_R, .G = HEALTH_BAR_BORDER_COLOR_G, .B = HEALTH_BAR_BORDER_COLOR_B}; 
	
	// How much health does the tank have ? How high should the inner part be
	double health_height = (((((double)tank->health)/TANK_HEALTH))*HEALTH_BAR_HEIGHT);

	for(int y = 0; y < HEALTH_BAR_HEIGHT; y++){
		for(int x = 0; x < HEALTH_BAR_WIDTH; x++){
			// Try to color only if we are withing boundraries
			if((start_x+x >= 0) && (start_x+x < DISPLAY_WIDTH) && (start_y+y >= 0) && (start_y+y < DISPLAY_HEIGHT)){
				if((y < HEALTH_BAR_BORDER) || (y > (HEALTH_BAR_HEIGHT-HEALTH_BAR_BORDER-1))){
					// Color border UP and DOWN
					ColorCanvasPixel(start_x+x, start_y+y, &border_color, game_canvas);
				}else{
					if((x < HEALTH_BAR_BORDER) || (x > (HEALTH_BAR_WIDTH-HEALTH_BAR_BORDER-1))){
						// Color border LEFT and RIGHT
						ColorCanvasPixel(start_x+x, start_y+y, &border_color, game_canvas);
					}else{
						// Color inner part if we satisfy requierement
						if(y >= (HEALTH_BAR_HEIGHT-health_height)){
							// Color innter part
							ColorCanvasPixel(start_x+x, start_y+y, &inner_color, game_canvas);
						}
					}
				}
			}
		}
	}
	
	return;
}

/*  This function draws name of the player that won on the canvas  */
void DrawWinningScreen(SceneData* scene_data, Canvas* game_canvas){

	RGB color_black = {.R = 0, .G = 0, .B = 0};
	
	if(scene_data[CURRENT_SCENE].player_turn == PLAYER_1_TURN){
		DrawText(game_canvas, "PLAYER 1 WON", 155, 150, 5, &color_black);
	}else if(scene_data[CURRENT_SCENE].player_turn == PLAYER_2_TURN){
		DrawText(game_canvas, "PLAYER 2 WON", 155, 150, 5, &color_black);
	}
	
	return;
}

/*  This function server as an exit function from the game scene. Free's every
    mallocked memory within game map.  */
void ExitGameMap(SceneData* scene_data){
	free(scene_data[SCENE_GAME_MAP].player_1_tanks_obj);
	free(scene_data[SCENE_GAME_MAP].player_2_tanks_obj);
	free(scene_data[SCENE_GAME_MAP].aim_path_private->parabolic_path); 
	free(scene_data[SCENE_GAME_MAP].aim_path_public->parabolic_path); 
	return;
}

/*  This function check live tanks of 'player' {PLAYER_1_TURN-player_1/PLAYER_2_TURN-player_2}  
	if there is a live tank it returns index to it for 'player_1_tanks_obj' or 'player_2_tanks_obj'
	based on whose tanks we wanted to check  */
int ReturnIndexToLiveTank(SceneData* scene_data, int player){

	int new_tank_idx;
	
	if(player == PLAYER_1_TURN){
		new_tank_idx = ReturnLiveTankIdx(scene_data[CURRENT_SCENE].player_1_tanks_obj, 
											 scene_data[CURRENT_SCENE].player_1_tanks, 
											 scene_data[CURRENT_SCENE].Player_1_tank_ptr);
	}else if(player == PLAYER_2_TURN){
		new_tank_idx = ReturnLiveTankIdx(scene_data[CURRENT_SCENE].player_2_tanks_obj, 
											 scene_data[CURRENT_SCENE].player_2_tanks, 
											 scene_data[CURRENT_SCENE].Player_2_tank_ptr);
	}
	
	return new_tank_idx;
}
