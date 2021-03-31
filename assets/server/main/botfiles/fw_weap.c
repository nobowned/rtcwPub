//===========================================================================
//
// Name:			fw_weap.c
// Function:
// Programmer:		Mr Elusive (MrElusive@idsoftware.com)
// Last update:		1999-09-08
// Tab Size:		4 (real tabs)
//===========================================================================

weight "Knife"
{
	switch(INVENTORY_KNIFE)
	{
		case 0: return 0;
		default: return W_KNIFE;
	} //end switch
} //end weight

weight "Luger"
{
	switch (INVENTORY_LUGER)
	{
	case 0: return 0;
	default:
	{
		switch (INVENTORY_9MM_PISTOL)
		{
		case 0: return 0;
		default:
		{
			return W_PISTOL;
		} //end default
		} //end switch
	} //end default
	} //end switch
} //end weight

weight "MP40"
{
	switch(INVENTORY_MP40)
	{
	case 0: return 0;
	default:
	{
		switch(INVENTORY_9MM)
		{
		case 0: return 0;
		default:
		{
			return W_MACHINEGUN;
		} //end default
		} //end switch
	} //end default
	} //end switch
} //end weight

weight "Mauser"
{
	switch (INVENTORY_MAUSER)
	{
	case 0: return 0;
	default:
	{
		switch (INVENTORY_792MM)
		{
		case 0: return 0;
		default:
		{
			return W_BFG10K;
		} //end default
		} //end switch
	} //end default
	} //end switch
} //end weight

weight "Grenade Launcher"
{
	switch (INVENTORY_GRENADE_LAUNCHER)
	{
	case 0: return 0;
	default:
	{
		switch (INVENTORY_GRENADES)
		{
		case 0: return 0;
		default:
		{
			return W_GRENADELAUNCHER;
		} //end default
		} //end switch
	} //end default
	} //end switch
} //end weight

weight "Panzerfaust"
{
	switch (INVENTORY_PANZERFAUST)
	{
	case 0: return 0;
	default:
	{
		return W_GRAPPLE;
	} //end default
	} //end switch
} //end weight

weight "Venom"
{
	switch (INVENTORY_VENOM)
	{
	case 0: return 0;
	default:
	{
		switch (INVENTORY_127MM)
		{
		case 0: return 0;
		default:
		{
			return W_ROCKETLAUNCHER;
		} //end default
		} //end switch
	} //end default
	} //end switch
} //end weight

weight "Flamethrower"
{
	switch (INVENTORY_FLAMETHROWER)
	{
	case 0: return 0;
	default:
	{
		switch (INVENTORY_FUEL)
		{
		case 0: return 0;
		default:
		{
			switch (ENEMY_HORIZONTAL_DIST)
			{
			case 768: return W_LIGHTNING;
			default: return $evalint(W_LIGHTNING*0.1);
			} //end switch
		} //end default
		} //end switch
	} //end default
	} //end switch
} //end weight

weight "Knife2"
{
	switch (INVENTORY_KNIFE2)
	{
	case 0: return 0;
	default: return W_KNIFE;
	} //end switch
} //end weight

weight "Colt"
{
	switch (INVENTORY_COLT)
	{
	case 0: return 0;
	default:
	{
		switch (INVENTORY_45MM_PISTOL)
		{
		case 0: return 0;
		default:
		{
			return W_PISTOL;
		} //end default
		} //end switch
	} //end default
	} //end switch
} //end weight

weight "Thompson"
{
	switch (INVENTORY_THOMPSON)
	{
	case 0: return 0;
	default:
	{
		switch (INVENTORY_45MM)
		{
		case 0: return 0;
		default:
		{
			return W_MACHINEGUN;
		} //end default
		} //end switch
	} //end default
	} //end switch
} //end weight

//weight "Sten"
//{
//	switch (INVENTORY_STEN)
//	{
//	case 1: return 0;
//	default:
//	{
//			   switch (INVENTORY_BULLETS)
//			   {
//			   case 1: return 0;
//			   default:
//			   {
//						  return W_MACHINEGUN;
//			   } //end default
//			   } //end switch
//	} //end default
//	} //end switch
//} //end weight

