
/*****************************************************************************
 * name:		ai_chat.h
 *
 * desc:		Quake3 bot AI
 *
 * $Author: Sherman $ 
 * $Revision: 2 $
 * $Modtime: 7/01/01 2:36p $
 * $Date: 7/03/01 1:35a $
 *
 *****************************************************************************/

//
int BotChat_EnterGame(bot_state_t *bs);
//
int BotChat_ExitGame(bot_state_t *bs);
//
int BotChat_StartLevel(bot_state_t *bs);
//
int BotChat_EndLevel(bot_state_t *bs);
//
int BotChat_HitTalking(bot_state_t *bs);
//
int BotChat_HitNoDeath(bot_state_t *bs);
//
int BotChat_HitNoKill(bot_state_t *bs);
//
int BotChat_Death(bot_state_t *bs);
//
int BotChat_Kill(bot_state_t *bs);
//
int BotChat_EnemySuicide(bot_state_t *bs);
//
int BotChat_Random(bot_state_t *bs);
// time the selected chat takes to type in
float BotChatTime(bot_state_t *bs);
// returns true if the bot can chat at the current position
int BotValidChatPosition(bot_state_t *bs);
// test the initial bot chats
void BotChatTest(bot_state_t *bs);

