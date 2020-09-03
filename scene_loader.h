/*  Loading scenes data.
	- Loading images
	- Definition of scene data, tanks, parabola, projectile
	- Initial game settings 
	- Tank actions
	- Map functions
*/

#include <stdbool.h>
#include <math.h>
#include "mzapo_canvas.h"
#include "mzapo_helper_functions.h" // sleep in ms

///////////// WHAT SCENE IS RIGHT NOW ////////////
int CURRENT_SCENE;
//////////////////////////////////////////////////

// Every scene has its number. This has to be abided.
enum {
	SCENE_NO_SCENE,
	SCENE_MENU,
	SCENE_GAME_MAP,
	SCENE_TOTAL_COUNT // Total number of scenes
}SCENES;

/// Our enumeration list for all actions on our board
enum {
	ACTION_NO_ACTION,
	ACTION_BUTTON_1_CLICKED,
	ACTION_BUTTON_1_MOVE_LEFT,
	ACTION_BUTTON_1_MOVE_RIGHT,
	ACTION_BUTTON_2_CLICKED,
	ACTION_BUTTON_2_MOVE_LEFT,
	ACTION_BUTTON_2_MOVE_RIGHT,
	ACTION_TOTAL_ACTIONS // Total number of scenes
}ACTION;

// Whose turn is it
enum{
	PLAYER_1_TURN,
	PLAYER_2_TURN
}PLAYER_TURN;

// What is the wind speed level currently
enum{
	WIND_LEVEL_STRONGEST_LEFT, // From right to left
	WIND_LEVEL_STRONG_LEFT, 
	WIND_LEVEL_WEAKER_LEFT,
	WIND_LEVEL_NONE,
	WIND_LEVEL_WEAKER_RIGHT,
	WIND_LEVEL_STRONG_RIGHT,
	WIND_LEVEL_STRONGEST_RIGHT,
	WIND_LEVEL_TOTAL_LEVELS
}WIND_LEVEL;



// Constants and settings
#define PLAYER_MAX_TANKS 4 // Maximum tanks per player
#define TANK_HEALTH 100
#define TANK_MOVE_LIMIT 90 // max 90px per move

// For how long the winning text is shown
#define WINNING_DELAY 3000

//// Health bar settings
#define HEALTH_BAR_HEIGHT 50 // Width in px
#define HEALTH_BAR_WIDTH 10 // Height in px
#define HEALTH_BAR_BORDER 1 // Border in px
#define HEALTH_BAR_Y_OFFSET 75 // Offset from tank-y
#define HEALTH_BAR_X_OFFSET -5 // Offset from tank-x
#define HEALTH_BAR_INNER_COLOR_R 250
#define HEALTH_BAR_INNER_COLOR_G 0
#define HEALTH_BAR_INNER_COLOR_B 0
#define HEALTH_BAR_BORDER_COLOR_R 0
#define HEALTH_BAR_BORDER_COLOR_G 0
#define HEALTH_BAR_BORDER_COLOR_B 0
////////////////////////


#define ANGLE_OF_FIRE_STEP 1.0
#define ANGLE_OF_FIRE_MAX 180 
#define ANGLE_OF_FIRE_MIN -10

#define PROJECTILE_VELOCITY_STEP 1 // Sensitivity of velocity change by buttons
#define PROJECTILE_VELOCITY_MAX 90.0
#define PROJECTILE_VELOCITY_MIN 1.0
#define PROJECTILE_ANIMATION_DELAY 5 // Speed of projectile animation

// If the distance between projectile is lower than this number, explosion will be triggered
#define PROJECTILE_TANK_DISTANCE_EXPLOSION_TRIGER 10
// If the distance between projectile and the ground will be lower than this number, explosion will be triggered
#define PROJECTILE_GROUND_DISTANCE_EXPLOSION_TRIGER 0

// How much of the terrain should be removed
#define TERRAIN_REMOVER_MULTIPLIER 0.2
#define TERRAIN_BOTTOM_BORDER 10 // how many pixels of map should be there at the bottom which cannot be removed


// Projectile path attributes
#define ARRAY_PATH_TIME_CHANGE 0.05 // 0.01 Seconds between each measurement of position
 
// Scene physics settings
#define SCENE_GRAVITY -10.0
 
// How many pixels will player see in the projectile path
#define AIM_PATH_PUBLIC_PIXELS_SHOW 7
#define AIM_PATH_PRIVATE_ARRAY_MAX_SIZE 2000

#define TANK_BARREL_X_OFFSET 10
#define TANK_BARREL_Y_OFFSET 10

// Every scene has to have its own image. Ordering from SCENES_NUM
#define PATH_TO_IMG_SCENE_MENU "images/backgrounds/menu.png"
#define PATH_TO_GAME_MAP "images/backgrounds/game_map.png"
//////////////////////////////////////////////////////////////////

// Scenes also have additional images
#define PATH_TO_IMG_SCOPE "images/scope/scope.png"
#define PATH_TO_PLAYER_1_TANK "images/players/player_1.png"
#define PATH_TO_PLAYER_2_TANK "images/players/player_2.png"
//#define PATH_TO_EXPLOSION_IMG "images/explosion/explosion.png" 
/////////////////////////////////////

#define PI 3.14159265

/// Our enumeration list for all types of projectiles we will be using
enum {
	PROJECTILE_TYPE_TYPE_CANNON,
	PROJECTILE_TYPE_TYPE_SNIPER,
	PROJECTILE_TYPE_TYPE_TOTAL // Total number of scenes
}PROJECTILE_TYPE;

// 'Gravity', 'wind' and 'velocity on launch' multiplier will characterize each projectile
enum {
	PROJECTILE_G_MULTIPLIER,
	PROJECTILE_W_MULTIPLIER,
	PROJECTILE_V_MULTIPLIER,
	PROJECTILE_DAMAGE,
	PROJECTILE_RADIUS,
	PROJECTILE_EXPLOSION_RADIUS,
	PROJECTILE_ATTRIBUTES_TOTAL
}PROJECTILE_ATTRIBUTE;

typedef struct Parabola{
	short int* parabolic_path;
	int number_of_pairs; // Number of pixel pairs in 'parabolic_path' array
}Parabola;

typedef struct Projectile{	
	// Projectile type
	int type; // PROJECTILE_TYPE_TYPE_CANNON// SNIPER// CLUSTER BOMB
	// type will correspond then to damage, and gravity,wind,velocity multiplier
	
	// Current position in the private aim_path array 
	Parabola* parabola;
	int projectile_path_pairs_index;
	
	// Game info
	bool to_explode; // Inform the game to explode this projectile, already hit
	bool exploded;
	
	/* Frame counter, to keep track for how long to show explostion picture. 
	   Use 'EXPLOSION_MAX_FRAMES' as frames cap.  */
	int explosion_current_frames; 
	
	int distance_from_ground; // Distance from ground in pixels
	
}Projectile;

// Struct for tanks
typedef struct Tank{ // Friendly fire on
	int x, y; // Coordinates
	double angle_of_fire;
	double velocity_of_projectile; // > 0
	
	int health; // Tank health
	bool is_alive;
	int move_limit; // max pixels per move, will be counted down after each move
	
	/* This will be used to print the name of the projectile on the screen 
	   and initialize projectile on shooting  */
	int chosen_projectile; 
	double distance_from_projectile;
}Tank;

/* Here we have the data we will need for every scene.
   We will not discriminate and each scene will have this struct.
   All scenes will be in array. Numberings must be ordered by SCENES_NUM  */
typedef struct ScenesData{
	
	int scene_id;
	Canvas scene_canvas; // Scene has own full-screen image
	
	// Controls
	int last_action; // The last action user made
	int knob_menu_position; // 0-PLAY, 1-WIND, 2-PLAYER 1 TANKS, 3-PLAYER 2 TANKS
	///////////
	
	/// Scene data
	double gravity; // Mapping 1px = 1m. [px/s]. direction(up -> grav>0)(down -> grav<0)
	bool wind_on;
	// Mapping 1px = 1m. [px/s]. direction(right -> wind>0)(left -> wind<0)
	int wind_speed_index; // -> this value is an index to 'WIND_SPEEDS' array in 'scene_loader.c'
	bool animating; // If this is true tanks cannot move and animation must end
		
	/// Menu
	Canvas scope_canvas; // Variable for menu scope image
	int player_1_tanks;
	int player_2_tanks;
	int player_max_tanks;
	////////
	
	/// Gamemplay
	unsigned short* game_map_array; // Our sand map
	/* Create first 'SHOW_PATH_PIXELS' pixels of our parabolic path.
	   This path will use current weapon settings but not wind even if the wind is on.
	   This path will be refreshed each barrel,tank movement, velocity or weapon change*/
	Parabola* aim_path_private; 
	Parabola* aim_path_public; 
	
	Canvas player_1_tank; // Player 1 tank img
	Canvas player_2_tank; // Player 2 tank img
	Canvas explosion; // Explosion image
	Projectile projectile; // Projectile data - 1 projectile for turn
	// Player 1 and 2 tanks
	Tank* player_1_tanks_obj;
	Tank* player_2_tanks_obj;
	//////////////
	
	// Did we confirm our movement and are ready to shoot ? -> reset after round
	bool movement_confirmed; 
	
	// Takes care of turns
	int player_turn;
	int Player_1_tank_ptr; 
	int Player_2_tank_ptr; 
	int turn_count;
	
	Tank* active_tank; // Active tank this turn
	
	// Game over
	bool GameOver;
	
	/////////////

	
	
}SceneData;

/// Our enumeration list for all knob positions within main menu
enum{
	KNOB_MENU_PLAY,
	KNOB_MENU_WIND,
	KNOB_MENU_PLAYER_1,
	KNOB_MENU_PLAYER_2,
	KNOB_MENU_TOTAL_POSITIONS
}KNOB_MENU_POSITION;

// Load all images to SceneData struct and return
SceneData* GetInnitialScenesData();

// Load specific scene. Initialize scene settings
void LoadScene(SceneData* scene_data, Canvas* game_canvas, int scene_id);

// Refresh background, dynamically draw text if needed, etc...
void RefreshScene(SceneData* scene_data, Canvas* game_canvas);

/* Fill game map array into current scene with different modes
   modes - 0 line
         - 1 predefined array n.1
*/
void FillGameMapArray(SceneData* scene_data, int mode);

/*  Set tank y position. Tank will always be on top of our sand.  */
void SetAllTanksPosition(SceneData* scene_data);

/*  Fill our game canvas with sand  */
void FillCavnasWithMapArray(SceneData* scene_data, Canvas* game_canvas);

// Add tank images to canvas 'game_canvas'
void FillCanvasWithTanks(SceneData* scene_data, Canvas* game_canvas);

/*  Move tank to the left if 'left'==true or right otherwise.  */
void MoveTank(SceneData* scene_data, bool left);

/*  This function changes the tank and player in control  */
void NextTurn(SceneData* scene_data);

/*  This functions returns the index of the next live tank in array of tanks  
	-1 if there is no live tank.  */
int ReturnLiveTankIdx(Tank* tanks, int tanks_count, int old);

/*  Reset move limits on all tanks  */
void ResetTanksMoveLimit(SceneData* scene_data);

/*  This function increases current active tank barrel angle  
	if add_deg == true move barrel from 0 def to 180 deg
	else move barrel from 180 deg to 0 deg by 1 degree  */
void MoveBarrel(SceneData* scene_data, bool add_deg);

/*  This function increases current active tank projectile velocity  
	if add_velocity == true, increase the velocity
	else decreade projectile velocity by PROJECTILE_VELOCITY_STEP degree  */
void ChangeProjectileVelocity(SceneData* scene_data, bool add_velocity);

/*  This function changes current weapon for the active tank  */
void ChangeWapon(SceneData* scene_data);


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
							);
							
/*  This function calls function 'CalculateParabolicPath'
	If 'should_malloc' is true the new path will be mallock'd to new array and saved
	accordingly to 'aim_path_private' if 'private_path' == true, or save it to 
	'aim_path_public' if 'private_path' == false  */
void RefreshAimPath(SceneData* scene_data, bool should_malloc, bool private_path);

/*  This function draws 'aim_path_public' from scene_data onto canvas from the
	onto active tank  */
void DrawAimPath(SceneData* scene_data, Canvas* game_canvas, RGB* color);

/*  This function randomly sets wind speed. This game will have 'WIND_LEVEL_TOTAL_LEVELS' 
	levels of wind speeds. For now: <<<, <<, <, (.) , >, >>, >>> */
void SetWindSpeed(SceneData* scene_data);

/*  This function encodes wind speed information to display and informs user of 
    the wind situation  */
void DrawWindSpeeds(SceneData* scene_data, Canvas* game_canvas, RGB* color);

/*  This function will initialize default state for the projectile and starts animation.  */
void StartProjectileAnimation(SceneData* scene_data);

/*  This function moves the projectile in animation frame by frame and checks if it
    should explode and end animation.  */
void ProjectileAnimationTick(SceneData* scene_data);

/*  This function draws current position of projectile onto canvas.
	Draws a circle and arguments are color and radius of the circle  */
void DrawProjectile(SceneData* scene_data, Canvas* game_canvas, RGB* color);

/*  This function calculates distance between two points {x1,y1,x2,y2}  */
double GetDistance(int x1, int y1, int x2, int y2);

/*  This function calculates distance of projectile from players and saves it to	
	each tank  */
void RefreshProjectilePlayersDistance(SceneData* scene_data);

/*  This function calculates immediate distance of projectile from the ground  */
void RefreshProjectileGroundDistance(SceneData* scene_data);

/*  This function will decide if the projectile should explode.
	If it should, 'to_explode' variable in projectile will be set to true*/
void ShouldProjectileExplode(SceneData* scene_data);

/*  This function dealth damage to its surroundings, player tanks and terrain  */
void ProjectileDoDamage(SceneData* scene_data);

/*  This function takes care of finalizing the explosion. Calls function for
    dealing area damage  'ProjectileDoDamage' and sets explosion picture for a few
    frames.  */
void ExplodeEndAnimation(SceneData* scene_data);

/*  This function iterates through all the live tanks and call function 
	'DrawSingleHealthBar' to draw a health bar over them  */
void DrawAllHealthBars(SceneData* scene_data, Canvas* game_canvas);

/*  This function takes tank as an argument and draws healthbar over it in game_canvas  */
void DrawSingleHealthBar(Tank* tank, Canvas* game_canvas);

/*  This function draws name of the player that won on the canvas  */
void DrawWinningScreen(SceneData* scene_data, Canvas* game_canvas);

/*  This function server as an exit function from the game scene. Free's every
    mallocked memory within game map.  */
void ExitGameMap(SceneData* scene_data);

/*  This function check live tanks of 'player' {PLAYER_1_TURN-player_1/PLAYER_2_TURN-player_2}  
	if there is a live tank it returns index to it for 'player_1_tanks_obj' or 'player_2_tanks_obj'
	based on whose tanks we wanted to check  */
int ReturnIndexToLiveTank(SceneData* scene_data, int player);
