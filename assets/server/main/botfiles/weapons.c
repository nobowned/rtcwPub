//===========================================================================
//
// Name:			weapons.c
// Function:		weapon configuration
// Programmer:		Mr Elusive (MrElusive@idsoftware.com)
// Last update:		1999-09-08
// Tab Size:		4 (real tabs)
//===========================================================================

#include "inv.h"
#include "game.h"

#define VEC_ORIGIN						{0, 0, 0}
//projectile flags
#define PFL_WINDOWDAMAGE			1		//projectile damages through window
#define PFL_RETURN					2		//set when projectile returns to owner
//weapon flags
#define WFL_FIRERELEASED			1		//set when projectile is fired with key-up event
//damage types
#define DAMAGETYPE_IMPACT			1		//damage on impact
#define DAMAGETYPE_RADIAL			2		//radial damage
#define DAMAGETYPE_VISIBLE			4		//damage to all entities visible to the projectile
#define DAMAGETYPE_IGNOREARMOR		8		//projectile goes right through armor

#define WEAPONINDEX_KNIFE			1
#define WEAPONINDEX_LUGER			2
#define WEAPONINDEX_MP40		    3
#define WEAPONINDEX_MAUSER	        4
#define WEAPONINDEX_FG42			5
#define WEAPONINDEX_GRENADE_LAUNCHER	6
#define WEAPONINDEX_PANZERFAUST		7
#define WEAPONINDEX_VENOM			8
#define WEAPONINDEX_FLAMETHROWER	9
#define WEAPONINDEX_TESLA		    10
#define WEAPONINDEX_SPEARGUN	    11
#define WEAPONINDEX_KNIFE2	        12
#define WEAPONINDEX_COLT		    13
#define WEAPONINDEX_THOMPSON	    14
//#define WEAPONINDEX_STEN			25

//===========================================================================
// Knife
//===========================================================================

projectileinfo //for Knife
{
	name				"knifedamage"
	damage				25
	damagetype			DAMAGETYPE_IMPACT
}

weaponinfo //Knife
{
	name				"Knife"
	number				WEAPONINDEX_KNIFE
	projectile			"knifedamage"
	numprojectiles		1
	speed				0
} //end weaponinfo

//===========================================================================
// Luger
//===========================================================================

projectileinfo //for Luger
{
	name				"lugerbullet"
	damage				10
	damagetype			DAMAGETYPE_IMPACT
}

weaponinfo //Luger
{
	name				"Luger"
	number				WEAPONINDEX_LUGER
	projectile			"lugerbullet"
	numprojectiles		1
	speed				0
} //end weaponinfo

//===========================================================================
// MP40
//===========================================================================

projectileinfo //for MP40
{
	name				"mp40bullet"
	damage				15
	damagetype			DAMAGETYPE_IMPACT
}

weaponinfo //MP40
{
	name				"MP40"
	number				WEAPONINDEX_MP40
	projectile			"mp40bullet"
	numprojectiles		1
	speed				0
} //end weaponinfo

//===========================================================================
// Mauser
//===========================================================================

projectileinfo //for Mauser
{
	name				"mauserbullet"
	damage				50
	radius				0
	damagetype			DAMAGETYPE_IMPACT
}

weaponinfo //Mauser
{
	name				"Mauser"
	number				WEAPONINDEX_MAUSER
	projectile			"mauserbullet"
	numprojectiles		1
	speed				0
} //end weaponinfo

//===========================================================================
// FG42
//===========================================================================

projectileinfo //for FG42
{
	name				"fg42bullet"
	damage				50
	radius				0
	damagetype			DAMAGETYPE_IMPACT
}

weaponinfo //FG42
{
	name				"FG42"
	number				WEAPONINDEX_FG42
	projectile			"fg42bullet"
	numprojectiles		1
	speed				0
} //end weaponinfo

//===========================================================================
// Grenade Launcher
//===========================================================================

projectileinfo //for Grenade Launcher
{
	name				"grenade"
	damage				120
	radius				160
	damagetype			$evalint(DAMAGETYPE_IMPACT | DAMAGETYPE_RADIAL)
}

weaponinfo //Grenade Launcher
{
	name				"Grenade Launcher"
	number				WEAPONINDEX_GRENADE_LAUNCHER
	projectile			"grenade"
	numprojectiles		1
	speed				700
} //end weaponinfo

//===========================================================================
// Panzerfaust
//===========================================================================

projectileinfo //for Panzerfaust
{
	name				"panzerexplosion"
	damage				100
	radius				100
	damagetype			$evalint(DAMAGETYPE_IMPACT | DAMAGETYPE_RADIAL)
}

weaponinfo //Panzerfaust
{
	name				"Panzerfaust"
	number				WEAPONINDEX_PANZERFAUST
	projectile			"panzerexplosion"
	numprojectiles		1
	speed				1000
} //end weaponinfo

//===========================================================================
// Venom
//===========================================================================

projectileinfo //for Venom
{
	name				"venombullet"
	damage				100
	radius				50
	damagetype			DAMAGETYPE_IMPACT
}

weaponinfo //Venom
{
	name				"Venom"
	number				WEAPONINDEX_VENOM
	projectile			"venombullet"
	numprojectiles		10
	speed				0
} //end weaponinfo

//===========================================================================
// Flamethrower
//===========================================================================

projectileinfo //for Flamethrower
{
	name				"flame"
	damage				10
	damagetype			DAMAGETYPE_IMPACT
}

weaponinfo //Flamethrower
{
	name				"Flamethrower"
	number				WEAPONINDEX_FLAMETHROWER
	projectile			"flame"
	numprojectiles		1
	speed				1000
} //end weaponinfo

//===========================================================================
// Tesla
//===========================================================================

projectileinfo //for Flamethrower
{
	name				"lightning"
	damage				10
	damagetype			DAMAGETYPE_IMPACT
}

weaponinfo //Flamethrower
{
	name				"Tesla"
	number				WEAPONINDEX_TESLA
	projectile			"lightning"
	numprojectiles		1
	speed				1000
} //end weaponinfo

//===========================================================================
// Speargun
//===========================================================================

projectileinfo //for Speargun
{
	name				"spear"
	damage				15
	damagetype			DAMAGETYPE_IMPACT
}

weaponinfo //Speargun
{
	name				"Speargun"
	number				WEAPONINDEX_SPEARGUN
	projectile			"spear"
	numprojectiles		1
	speed				0
} //end weaponinfo

//===========================================================================
// Knife2
//===========================================================================

projectileinfo //for Knife2
{
	name				"knifedamage2"
	damage				25
	damagetype			DAMAGETYPE_IMPACT
}

weaponinfo //Knife2
{
	name				"Knife2"
	number				WEAPONINDEX_KNIFE2
	projectile			"knifedamage2"
	numprojectiles		1
	speed				0
} //end weaponinfo

//===========================================================================
// Colt
//===========================================================================

projectileinfo //for Luger
{
	name				"coltbullet"
	damage				10
	damagetype			DAMAGETYPE_IMPACT
}

weaponinfo //Luger
{
	name				"Colt"
	number				WEAPONINDEX_COLT
	projectile			"coltbullet"
	numprojectiles		1
	speed				0
} //end weaponinfo

//===========================================================================
// Thompson
//===========================================================================

projectileinfo //for Thompson
{
	name				"thompsonbullet"
	damage				15
	damagetype			DAMAGETYPE_IMPACT
}

weaponinfo //Thompson
{
	name				"Thompson"
	number				WEAPONINDEX_THOMPSON
	projectile			"thompsonbullet"
	numprojectiles		1
	speed				0
} //end weaponinfo

//===========================================================================
// Sten
//===========================================================================

//projectileinfo //for Sten
//{
//	name				"stenbullet"
//	damage				15
//	damagetype			DAMAGETYPE_IMPACT
//}
//
//weaponinfo //Sten
//{
//	name				"Sten"
//	number				WEAPONINDEX_STEN
//	projectile			"stenbullet"
//	numprojectiles		1
//	speed				0
//} //end weaponinfo

