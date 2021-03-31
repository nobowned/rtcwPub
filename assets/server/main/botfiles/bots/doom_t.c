//===========================================================================
//
// Name:			Doom_c.c
// Function:		chat lines for Doom
// Programmer:		MrElusive (MrElusive@idsoftware.com)
// Last update:		1999-09-08
// Tab Size:		3 (real tabs)
//===========================================================================

//example initial chats
chat "doom"
{
	//the teamplay.h file is included for all kinds of teamplay chats
	#include "teamplay.h"
	//======================================================
	//======================================================
	type "game_enter" //initiated when the bot enters the game
	{
		"Heh. There's rats in here... big ones.";
		"I keep going 'round in circles ... no escape.";
		"I can run ", 4, " in the dark.";
		"You demons don't stand a chance against me.";
		"You just haven't learned yet, have you?";
		"Dooooooooooommmmmmmmm!";
		HELLO7;
		"Didn't think I'd be fighting cheerleaders.";
		"The ", 4, "? Hey I think I left ", 1 ,"'s spleen in here last time!";
		// 1= random opponent
		// 4= Level's title
		// 0 = bot name
	} //end type

	type "game_exit" //initiated when the bot exits the game
	{
		"Job's done here. Gotta move on... too many demons...";
		GOODBYE1;
		"All roads lead to hell. Every last ~one of 'em.";
		"I'm history! And ", 1, " can kiss my ammo belt.";
		// 1= random opponent
		// 0 = bot name
	} //end type

	type "level_start" //initiated when a new level starts
	{
		"Get the reporters out o' here, this ain't gonna be pretty!";
		"Too many in here! Get out, ", 1 ,"! Run while you can!";
		"Aaaaarrrggghhh!";
		"Destruction and turmoil are my companions.";
		LEVEL_START2;
		// 0 = bot name
	} //end type


	type "level_end" //initiated when a level ends and the bot is not first and not last in the rankings
	{
		"Turn up the heat! Not good enough! Never friggin' good enough!";
		"This isn't the end. I'll be back to finish what I started.";
		"Over? I haven't found the blue key card yet!";
		"Now that we feel all nice and cozy, I'll turn up the heat.";
		"Listen up and listen good, I don't do mediocre.";
		2, " in first? Send me back to boot camp.";
	
		// 2 = opponent in first place
		// 3 = opponent in last place
		// 0 = bot name
	} //end type

	type "level_end_victory" //initiated when a level ends and the bot is first in the rankings
	{	
		"Do I always have to take the lead?";
		"I've been to hell... you toads are nothing!";
		"You hear her, ", 3, "? The fat lady's singing for you.";
		"They all die! Isn't that enough! How many more will be thrown at me?";
		LEVEL_END_VICTORY4;
		"Mission accomplished. Start singing, fat lady.";
		"Cue the fat lady. It's about to be over.";
		"Awwww. Looks like ", 3, " needs a big hug.";
		1, ", go give ", 3, " some tissue to wipe those tears.";
		// 1 = random opponent
		// 3 = opponent in last place
		// 0 = bot name
	} //end type

	type "level_end_lose" //initiated when a level ends and the bot is last in the rankings
	{
		"I was last once before, and a whole planet paid the price.";
		"Well lets see who's in first place after I tear ",2,"'s head off!";
		LEVEL_END_LOSE2;
		"I fear that my lesson is being lost on you, ", 2, ".";
		// 2 = opponent in first place
		// 0 = bot name
	} //end type


	//======================================================
	//======================================================

	type "hit_talking" //bot is hit while chat balloon is visible; lecture attacker on poor sportsmanship
	{
		"A cheap shot, ", 0, ". You don't win a match with cheap shots.";
		"If you so much as point that ", 1, " in my direction again, ", 0, ", you're gonna eat it.";
		"That's a cheap shot, ", 0, ". You don't win matches that way.";
		"Well, so much for being sociable.";
		//1 = weapon used by shooter
		//0 = shooter
	} //end type

	type "hit_nodeath" //bot is hit by an opponent's weapon attack; either praise or insult
	{
		"That's it, ", fighter, ". Try aiming next time and you may actually kill someone.";
		"An attack on me is an attack on humankind. You won't be missed, ", 0, ".";
		0, ", that ", 1, " is going to look great sticking out of your ear.";
		"That's it soldier. Try aiming next time and you may actually kill someone.";
		"Your technique reminds me of a story, ", 0, ". A short dull ~one.";
		"I've taken more damage from flying body parts.";
		"Stimpack! Must have stimpack!";
		//1 = weapon used by shooter		
		//0 = shooter
	} //end type


	type "hit_nokill" //bot hits an opponent but does not kill it
	{
		"Well are you just going to stand there ", 0, "? Do something.";
		"Take it as a warning to quit, ", 0, ". Run away! Do you hear?";		
		"The initial incision seems to create my favorite reactions.";
		"Nothing like a good flesh wound. Stirs the soul doesn't it?";
		"For just a moment, your sweet screams of agony made the voices stop.";
		TAUNT;
		TAUNT7;
		"I'm the surgeon and this is my operating table. Will the bodies ever stop?";
		//0 = opponent	
	} //end type

	//======================================================
	//======================================================

	type "death_telefrag" //initiated when the bot is killed by a telefrag
	{
		"I warned 'em all. I told 'em to split. Guess ",0," listened.";
		"Is this some kind of trick, ", 0, ".";
		"More proof of demonic infiltration!.";
		DEATH_TELEFRAGGED0;
		// 0 = enemy name
	} //end type

	type "death_cratered" //initiated when the bot is killed by taking "normal" falling damage
	{	
		"The ground makes an unfriendly pillow.";
		"It's a dream...Im falling, falling...yeeaaarghhh!";
		"What a waste of a perfectly good pair of legs.";
		// 0 = enemy name
	} //end type

	type "death_lava" //initiated when the bot dies in lava
	{
		"So it's fire and brimstone is it?";
		"I've been to hell once ... kinda felt like this.";
		// 0 = enemy name
	} //end type

	type "death_slime" //initiated when the bot dies in slime
	{	
		"Nothing like croaking in a puddle of demon snot!";
		"shouldn't this stuff be in barrels!?!";
		DEATH_SLIME0;
		"Somebody went outta their way to make this feel like home.";
		// 0 = enemy name
	} //end type

	type "death_drown" //initiated when the bot drowns
	{
		"Peace, but it's only a temporary reprieve.";
		"Mmmmm. Bionic sushi.";
		"Guess I won't make the swim team.";
		"You think falling in this little puddle will stop me?";
		// 0 = enemy name
	} //end type

	type "death_suicide" //initiated when bot blows self up with a weapon or craters
	{
		"The awful irony! To survive demons and go out like this.";
		"Just be glad I didn't take you with me, ", 0, ".";
		"I'm the last... I know I am. It's over now.";
		DEATH_SUICIDE3;
		DEATH_SUICIDE4;
		"The only ~one who can really beat me is... me!";
		"I just felt like it... that's why!";
		// 0 = enemy name
	} //end type

	type "death_gauntlet" //initiated when the bot is killed by a gauntlet attack
	{	
		"They had cold hands, too. So cold.";
		"I didn't think you were strong enough to lay a glove on me.";
		"Hail to the Gloved ~One.";
		DEATH_INSULT4;
		"Your mama slaps harder than that, ", 0, ".";
		//1 = weapon used by shooter	
		// 0 = enemy name
	} //end type

	type "death_rail" //initiated when the bot is killed by a rail gun shot
	{	
		"In a recent study, maggots were ranked higher than campers.";
		DEATH_INSULT0;
		DEATH_RAIL2;
		"Heh, I never heard it. I thought I would hear it...";
		"That rang my chimes!";
		// 0 = enemy name
	} //end type

	type "death_bfg" //initiated when the bot died by a BFG
	{
		"The rain of destruction. It doesn't stop!";
		"Hey, ", 0, ". The ", 1, " is a big kid's toy! Put it down!";
		"Funny what ", 0, " can do when a gun misfires.";
		"That toy ain't no BFG.";
		DEATH_BFG2;
		// 1 = weapon used by shooter
		// 0 = enemy name
	} //end type

	type "death_insult" //insult initiated when the bot died
	{
		"You think this scrape evens the score? Guess again!";
		"The implications of what you have just done are staggering, ", 0, ".";
		"Yeah, take my time... pull out your intestines ~one foot at a tug.";
		DEATH_INSULT3;
		"You just gave me some needed rest!";
		"I'm gonna pull out your bowels, ", fighter, "!";
		"I AM THE DESTROYER!!";
		// 0 = enemy name
	} //end type


	type "death_praise" //praise initiated when the bot died
	{	
		"You and your ", 1, " win this round, fiend face. Enjoy it while it lasts.";
		"Take a moment to reflect on your accomplishment, ", 0, ".";
		D_PRAISE5;
		"D2DM1! I thought you looked familiar, ", 0, "!";
		// 1 = weapon used by shooter
		// 0 = enemy name
	} //end type

	//======================================================
	//======================================================

	type "kill_rail" //initiated when the bot kills someone with rail gun
	{
		"Well if it isn't my old friend Mr. Swiss cheese.";
		KILL_RAIL1;
		"Sssh. Say nothing, just lie down.";
		"Ker-splattttt!";
		"Fore!!!";
		"That's what I call a 'round peg in a square hole.'";
		// 0 = enemy name
	} //end type

	type "kill_gauntlet" //initiated when the bot kills someone with gauntlet
	{
		"It's better this way. No gore. No body parts lying... everywhere.";
		"I think I have some of your face stuck to my glove, ", 0, "!";
		"Need... Berserk pack... must... have... berserk pack!";
		"Bwahahahahahaha!";
		"I can't get enough of this!";
		"Groovy!";
		"This will do just fine 'til I get me a chainsaw.";
		DEATH_GAUNTLET2;
		// 0 = enemy name
	} //end type

	type "kill_telefrag" //initiated when the bot telefragged someone
	{
		"Too many all together... crowded with demons and rotting dead!";
		TELEFRAGGED3;
		"Thats what ", 0, " gets for being such a hot head.";
		"MMMMM... leftovers again!";
		"I love my work!";
		// 0 = enemy name
	} //end type

	type "kill_suicide" //initiated when the player kills self
	{
		KILL_EXTREME_INSULT;
		KILL_INSULT34;
		"Hahahahahaha!";
		"Pretty colors! Do that again!";
		"You think that put me in a better mood? Well, yeah.";
		// 0 = enemy name
	} //end type

	type "kill_insult" //insult initiated when the bot killed someone
	{
		"An endless row of graves...too many to put the names on.";
		"You were there, on Phobos. I saw you running away. I found you now.";
		"Brainless moro... oops... wait I CAN see your brain, ", 0, "!";
		"Hahahahahah ... I just love my job!";
		"I regret that ", 0, " only has ~one life to give for my country.";
		"Don't worry. folks. ", 0, " won't do that again. ", 0, " doesn't have the guts!";
		"That's ~one way to wipe the smile off someone's face.";
		"Kiss my ammo belt.";
		KILL_INSULT19;
		KILL_INSULT24;
		// 0 = enemy name
	} //end type

	type "kill_praise" //praise initiated when the bot killed someone
	{
		PRAISE4;
		"I'll keep your skull as a trophy, ", 0, ". No brains to dirty it up.";
		"You did good ", 0, "... sure... you can believe that if it helps.";
		// 0 = enemy name
	} //end type	

	//======================================================
	//======================================================

	type "random_insult" //insult initiated randomly (just when the bot feels like it)
	{
		"You let them die, ", 0, "! That's why I hate you!";
		"I know why you are all here today... no ~one will miss you.";
		"'Kill more' the voice said and 'hurt them.' The voices don't like you!";
		0," screams louder than a school girl.";
		1, " has an odd fascination with the pavement.";
		"'", 1, " loses in ", 4, ". Perfect for your tombstone, ", 1, "!";	
		// 0 = name of randomly chosen player
		// 1 = bot name
		// 4 = level's title 
	} //end type


	type "random_misc" //miscellanous chats initiated randomly
	{
		"They died in my arms. So many lost friends...but they're with me now.";
		"He cried, you know. Said he didn't want to die. Then he did.";
		"Get out of me! Oh, they won't shut up!";
		"The blood! Too much! ",4,"'s starting to look familiar."
		"Join the ~Marines, see the universe... geez.";
		"You're just jealous because the voices only talk to me, ", 0, ".";
		"So, ", 0, ". Why don't we frag the ", fighter, "s, then duel it out?";
		"Everybody stop persecuting me!?";
		MISC1;
		MISC0;
		one_liners;
		"Where's the freakin' red key card?";
		"So, ", 1, ". How many ", 5, "s do you need anyways?";
		// 0 = name of randomly chosen player
		// 1 = bot name
		// 5 = random weapon from weapons list
	} //end type
} //end chat


