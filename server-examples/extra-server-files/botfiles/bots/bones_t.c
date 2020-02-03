//===========================================================================
//
// Name:		Bones_c.c
// Function:    chat lines for Bones
// Programmer:	MrElusive (MrElusive@idsoftware.com)
// Last update:	November 14, 1999
//Author:		Paul Jaquays
//Editor:		Paul Jaquays
// Tab Size:		3 (real tabs)
//===========================================================================


chat "bones"
{
	#include "teamplay.h"

	//======================================================
	//======================================================
	type "game_enter" //initiated when the bot enters the game
	{
		"Yo, ", 1, ", I got a ~bone to pick with you.";
		"This place is dead.";
		"I'm NOT gonna need to work my fingers to the ~bone to beat this crew.";
		"I'm here to cut out the dead wood.";
		"What's this? Only a ~skeleton crew here today?";
		// 0 = bot name
		//1 = random player
		//4 = level's title
	} //end type

	type "game_exit" //initiated when the bot exits the game
	{
		"I'm rattling on out of here.";
		"Dead man walking ... right on outta here.";
		
		// 0 = bot name
		//4 = level's title
	} //end type

	type "level_start" //initiated when a new level starts
	{
		"What better time to fight than the dead of the night?";
		"Bring out your dead!";
		"'Into the valley of death rode the ~six ~hundred!'";
		// 0 = bot name
	} //end type

	type "level_end" //initiated when a level ends and the bot is not first and not last in the rankings
	{
		"And here I was hoping to finish in a dead heat.";
		2, ", you are now wanted 'dead or alive' -- and you know my preference.";
	
		// 0 = bot name
		// 1 = random opponent
		// 2 = opponent in first place
		// 3 = opponent in last place
		// 4 = level's title
	} //end type

	type "level_end_victory" //initiated when a level ends and the bot is first in the rankings
	{
		"Yeah! This is LIVING... uh, so to speak.";
		"Score ~one for the dead guy.";
		// 0 = bot name
	} //end type

	type "level_end_lose" //initiated when a level ends and the bot is last in the rankings
	{
		"I'm dead to the world here.";
		"Dead last again! Arrrrrrrr.";
		"I need a stiff drink.";
		"Looks like I've been given up for dead again.";
		// 0 = bot name
	} //end type

	//======================================================
	//======================================================

	type "hit_talking" //bot is hit while chat balloon is visible; lecture attacker on poor sportsmanship
	{
		"So it is true-- 'Dead men really tell no tales.";
		"This ", fighter, "'s bad to the ~bone?";
		//0 = shooter
	} //end type

	type "hit_nodeath" //bot is hit by an opponent's weapon attack; either praise or insult
	{
		"That ~one cut close to the ~bone!";
		"Well, you'll be the death of me...";
		//0 = shooter
	} //end type

	type "hit_nokill" //bot hits an opponent but does not kill it
	{
		"I need a bigger gun.";
		"A marrow Escape. Sorry. Make that A NARROW escape.";
		//0 = opponent
	} //end type

	//======================================================
	//======================================================

	type "death_telefrag" //initiated when the bot is killed by a telefrag
	{
		"Like being cremated and having your ashes scattered.";
		" Anyone for pick up sticks?";
		// 0 = enemy name
	} //end type

	type "death_cratered" //initiated when the bot is killed by taking "normal" falling damage
	{
		"Brittle as an old ~bone.";
		"That's going ~six feet under the hard way... no shovel.";
		"I'm drop dead ~sexy!";
		// 0 = enemy name
	} //end type

	type "death_lava" //initiated when the bot dies in lava
	{
		"Well, I'm no longer chilled to the ~bone.";
		"Well that's definately not the dead of winter.";
		// 0 = enemy name
	} //end type

	type "death_slime" //initiated when the bot dies in slime
	{
		"Slime. Better non-living through chemistry.";
		"Hey, how do you think I got this glow-in-the-dark look in the first place?";
		// 0 = enemy name
	} //end type

	type "death_drown" //initiated when the bot drowns
	{
		ponder, " Drowning? Who would have figured?";
		"Got me a condo here in Davy Jone's locker.";
		"Worse things happen at sea.";
		// 0 = enemy name
	} //end type

	type "death_suicide" //initiated when bot blows self up with a weapon or craters
	{
		"Nothing like digging your own grave, eh?";
		"Hey, beats cremation.";
		"'I still can say... I did it my way!'";
		// 0 = enemy name
	} //end type

	type "death_gauntlet" //initiated when the bot is killed by a gauntlet attack
	{
		"That was like being savaged by a dead sheep.";
		"I'm all broken up about this.";
		"You touching me... now THAT's a fate wose than death.";
		// 0 = enemy name
	} //end type

	type "death_rail" //initiated when the bot is killed by a rail gun shot
	{
		"Wooo! Snapped me like a dry ~bone.";
		// 0 = enemy name
	} //end type

	type "death_bfg" //initiated when the bot died by a BFG
	{
		"Just call me ~bone meal.";
		"Ow. When the pony dies, the ride is over.";
		// 0 = enemy name
	} //end type

	type "death_insult" //insult initiated when the bot died
	{
		"Oooooo. I am just scared ~stiff.";
		"That just tickled me to death.";
		"Somebody explain all these meaty gibs to me.";
		"You'll be dead and gone soon enough, ", 0, ".";
		"You're gonna be dead as dodo, ", 0, "!";

		// 0 = enemy name
	} //end type

	type "death_praise" //praise initiated when the bot died
	{
		"I'm dead Jim.";
		"Not bad for a meat sack, ", 0, ". Make no ~bones about it.";
		"And here I'd already given you up for dead, ", 0, ".";
		"You had me dead to rights there, ", 0, ".";
		"'Because I could not stop for death, he kindly stopped for me--'";
		// 0 = enemy name
	} //end type

	//======================================================
	//======================================================
	type "kill_rail" //initiated when the bot kills someone with rail gun
	{
		"How's that for a 'kiss of death?'";
		"That had to be good for a broken ~bone or ~two.";
		
		// 0 = enemy name
	} //end type

	type "kill_gauntlet" //initiated when the bot kills someone with gauntlet
	{
		"More knuckles than you're used too, eh, ", 0, "?";
		"This gauntlet thingy gnaws right to the ~bone.";
		// 0 = enemy name
	} //end type

	type "kill_telefrag" //initiated when the bot telefragged someone
	{
		"That pretty much scatters your ashes, ", 0, ".";
		"Grind you up for bonemeal!";
		// 0 = enemy name
	} //end type

	type "kill_suicide" //initiated when the players kills self
	{
		"If I was you, ", 0, ", I'd be turning in my grave!";
		"You normally got ~one foot in the grave like that, ", 0, "?";
		// 0 = enemy name
	} //end type

	type "kill_insult" //insult initiated when the bot killed someone
	{
		"Didn't your ", random_counselor, " tell you never to hunt rabbit with a dead dog?";
		0, ", that last move of yours was a dead giveaway.";
		"You've slammed into yet another dead end, ", 0, ".";
		"Gotcha! You are dead as a doornail.";
		"That ought to put the fear of death into you, ", 0, "!";
		"You're better off dead, ", 0, ".";
		"You have just reached a dead end, ", 0, ".";
		// 0 = enemy name
	} //end type

	type "kill_praise" //praise initiated when the bot killed someone
	{
		"Listen, ", 0, ", being dead and gone ain't as bad as it seems.";
		"I had you dead to rights there, ", 0, ".";
		"Wrong. Dead Wrong. ";
		// 0 = enemy name
	} //end type

	//======================================================
	//======================================================

	type "random_insult" //insult initiated randomly (just when the bot feels like it)
	{
		0, ", you are already dead from the neck up. So a little more won't hurt.";
		0, " you are a dead man walking.";
		"Oh drop dead already, ", 1, ".";
		"Good thing I don't lose frags for being bored to death.";
		"Don't roll the ~bones with death and expect to win. His dice are loaded.";
		
		// 0 = name of randomly chosen player
		// 1 = last victim name
	} //end type

	type "random_misc" //miscellanous chats initiated randomly
	{
		"'Better dead than red?'... sounds like blue-centric bigotry to me.";
		"Well, yes, I do have a ~skeleton in my closet.";
		"You meat sacks just leave those sticks and stones at home, OK?";
		"Throw me a frickin' ~bone here.";
		"'Fifteen men on a dead man's chest...' better get their sorry butts off of me.";
		femalebot, " is drop dead gorgeous!";
		"'The wages of sin are death'... and I am handing out paychecks here.";
		"Death is just a part of life... the last part.";
		"Hey life goes on. Death just goes on longer.";
		"Yeah, I still miss my old girlfriend, ", femalebot, ". But my aim is getting better.";
		one_liners;
		"Anyone got a bottle of ", liquid, "? I'm dry as a ~bone here.";
		"Hey! You seen those cuties in Deva Station ~quad room?! WoooHooo!";
		"Is my brother Skully still hangin' around in the training level?";
		"I'm outta the closet now. Say it now and say it loud. 'I'm dead and I'm proud.'";
		// 0 = name of randomly chosen player
		// 1 = last victim name
	} //end type
} //end bones chat	
	
