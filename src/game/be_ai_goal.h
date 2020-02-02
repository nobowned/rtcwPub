
/*****************************************************************************
 * name:		be_ai_goal.h
 *
 * desc:		goal AI
 *
 * $Author: Sherman $ 
 * $Revision: 2 $
 * $Modtime: 7/01/01 4:04p $
 * $Date: 7/03/01 1:35a $
 *
 *****************************************************************************/

#define MAX_AVOIDGOALS			64
#define MAX_GOALSTACK			8
#define MAX_PATROL_PATH_LENGTH		8

#define GFL_NONE				0
#define GFL_ITEM				1
#define GFL_ROAM				2
#define GFL_NOSLOWAPPROACH		4
#define GFL_ACTIVATE			8
#define GFL_REVIVE				16
#define	GFL_ROUTE				32
#define GFL_ROUTE_REVERSE		64
#define GFL_ROUTE_CAMP			128
#define GFL_WALK				256
#define GFL_SPRINT				512

//a bot goal
typedef struct bot_goal_s
{
	vec3_t origin;				//origin of the goal
	int areanum;				//area number of the goal
	vec3_t mins, maxs;			//mins and maxs of the goal
	int entitynum;				//number of the goal entity
	int number;					//goal number
	int flags;					//goal flags
	int iteminfo;				//item information
} bot_goal_t;

typedef enum {
	ROUTE_TEAM_BOTH,	// both teams can use this route.
	ROUTE_TEAM_RED,		// route is intended for the red team.
	ROUTE_TEAM_BLUE		// route is intended for the blue team.
} bot_route_preferred_team_t;

typedef enum {
	ROUTE_TYPE_STARTING,	// exit spawn routes.
	ROUTE_TYPE_PRIMARY,		// key area (cycle for period of time).
	ROUTE_TYPE_ENDING		// around enemy spawn (camp these routes after killing enemy).
} bot_route_type_t;

//map patrol route from .aoi file
typedef struct bot_route_s {
	vec3_t path[MAX_PATROL_PATH_LENGTH]; // ideally only 2-3 per path.
	int path_length;
	bot_route_preferred_team_t team;
	bot_route_type_t type;
	int index;
} bot_route_t;

//reset the whole goal state, but keep the item weights
void BotResetGoalState(int goalstate);
//reset avoid goals
void BotResetAvoidGoals(int goalstate);
//remove the goal with the given number from the avoid goals
void BotRemoveFromAvoidGoals(int goalstate, int number);
//push a goal
void BotPushGoal(int goalstate, bot_goal_t *goal);
//pop a goal
void BotPopGoal(int goalstate);
//makes the bot's goal stack empty
void BotEmptyGoalStack(int goalstate);
//dump the avoid goals
void BotDumpAvoidGoals(int goalstate);
//dump the goal stack
void BotDumpGoalStack(int goalstate);
//name of the goal
void BotGoalName(int number, char *name, int size);
//get goal from top of stack
int BotGetTopGoal(int goalstate, bot_goal_t *goal);
int BotGetSecondGoal(int goalstate, bot_goal_t *goal);
//choose the best long term goal item for the bot
int BotChooseLTGItem(int goalstate, vec3_t origin, int *inventory, int travelflags);
//try finding and pushing a route goal
int BotChooseRouteGoal(int goalstate, int team, vec3_t origin, bot_goal_t *current_goal, int seconds_doing_route);
//get route at index
int GetRouteAtIndex(int index, bot_route_t *route);
//choose the best nearby goal item for the bot
int BotChooseNBGItem(int goalstate, vec3_t origin, int *inventory, int travelflags,
							bot_goal_t *ltg, float maxtime);
//returns true if the bot touches the goal
int BotTouchingGoal(vec3_t origin, bot_goal_t *goal);
//returns true if the goal should be visible but isn't
int BotItemGoalInVisButNotVisible(int viewer, vec3_t eye, vec3_t viewangles, bot_goal_t *goal);
//get some info about a level item
int BotGetLevelItemGoal(int index, char *classname, bot_goal_t *goal);
//get the next camp spot in the map
int BotGetNextCampSpotGoal(int num, bot_goal_t *goal);
//get the map location with the given name
int BotGetMapLocationGoal(char *name, bot_goal_t *goal);
//returns the avoid goal time
float BotAvoidGoalTime(int goalstate, int number);
//initializes the items in the level
void BotInitLevelItems(void);
//initializes the routes are in the .rts file
void BotInitRoutes(const char *mapname);
//regularly update dynamic entity items (dropped weapons, flags etc.)
void BotUpdateEntityItems(void);
//interbreed the goal fuzzy logic
void BotInterbreedGoalFuzzyLogic(int parent1, int parent2, int child);
//save the goal fuzzy logic to disk
void BotSaveGoalFuzzyLogic(int goalstate, char *filename);
//mutate the goal fuzzy logic
void BotMutateGoalFuzzyLogic(int goalstate, float range);
//loads item weights for the bot
int BotLoadItemWeights(int goalstate, char *filename);
//frees the item weights of the bot
void BotFreeItemWeights(int goalstate);
//returns the handle of a newly allocated goal state
int BotAllocGoalState(int client);
//free the given goal state
void BotFreeGoalState(int handle);
//setup the goal AI
int BotSetupGoalAI(void);
//shut down the goal AI
void BotShutdownGoalAI(void);
