#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"

sint8*  g_snemulds_str_dan[] = 
{
		"FS installerer...\n",			/* 0 */
		"FS fejl, Forts�tter aligevel...\n",				/* 1 */
		"FS success!\n",				/* 2 */
		"",								/* 3 */
		"SNEmulDS 0.6 F�dselsdags version, "
		"1997-2007: SNEmul er 10 �r gammel!",	/* 4 */
		"Credits:",								/* 5 */
		"",								/* 6 */
		"",								/* 7 */
		"",								/* 8 */
		"",								/* 9 */
		"Ok \001",							/* 10 */
		"Annuller \002",						/* 11 */
		"Anvend \003",						/* 12 */
		"Gem \004",						/* 13 */
		"",								/* 14 */
		"",								/* 15 */
		"",								/* 16 */
		"",								/* 17 */
		"",								/* 18 */
		"",								/* 19 */
		"V�lg ROM",					/* 20 */
		"Hent spilstadie",					/* 21 */
		"Gem spilstadie",					/* 22 */
		"Indstillinger",						/* 23 */
		"SPC Jukebox",						/* 24 */
		"Advanceret",						/* 25 */		
		"",								/* 26 */
		"Genstart",						/* 27 */
		"Gem SRAM",					/* 28 */
		"",								/* 29 */		
		"Lyd p�",				/* 30 */
		"Lyd fra",				/* 31 */
		"Hastighed:",						/* 32 */
		"VBlank enabled ",					/* 33 */
		"VBlank disabled ",					/* 34 */
		"",								/* 35 */
		"Sk�rm indstillinger",		/* 36 */
		"Baggrunde & Sprites instillinger",	/* 37 */
		"Speed Hacks:",				/* 38 */		
		"Ingen Hacks",					/* 39 */
		"Cycles Hacks",				/* 40 */
		"Interrupt Hacks",			/* 41 */
		"Full Hacks",				/* 42 */
		"",							/* 43 */
		"HUD:",					/* 44 */
		"normal",				/* 45 */
		"Pres sammen",				/* 46 */
		"Pres mere sammen",			/* 47 */
		"",							/* 48 */
		"YScroll:",					/* 49 */
		"midten",			/* 50 */
		"toppen",				/* 51 */
		"bunden",			/* 52 */
		"+24",							/* 53 */
		"Sk�rmopl�sning:",							/* 54 */
		"Ingen �ndring",		/* 55 */
		"middel",			/* 56 */
		"full sk�rm",		/* 57 */
		"",							/* 58 */
		"",							/* 59 */
		"",		/* 60 */
		"",			/* 61 */
		"Baggrundene og spritesene kan blive sorteret automatisk "
		"Eller du kan selv v�lge ordenen "
		"(Jo mindre nummeret er, jo st�rre prioritet har det).",	/* 62 */
		"",							/* 63 */
		"",							/* 64 */		
		"Automatic sortering",			/* 65 */
		"Lag #%d",					/* 66 */
		"Sprites",					/* 67 */
		"",							/* 68 */
		"FRA",						/* 69 */
		"0",						/* 70 */
		"1",						/* 71 */
		"2",						/* 72 */
		"3",						/* 73 */
		"4",						/* 74 */
		"5",						/* 75 */
		"6",						/* 76 */
		"7",						/* 77 */		
		"8",						/* 78 */
		"9",						/* 79 */
		" ",						/* 80 */
		"X",						/* 81 */
		"Automatisk SRAM gemning",	/* 82 */
		"",							/* 83 */
		"",							/* 84 */
		"Brug hukommelses modul",			/* 85 */
		"",							/* 86 */
		"Gemning af spilstadie vellykket", /* 87 */
		"",							/* 88 */
		"",							/* 89 */
		"Titel:",					/* 90 */
		"St�rrelse:",					/* 91 */
		"ROM Type:",				/* 92 */
		"Land:",					/* 93 */
		"",							/* 94 */
		"",							/* 95 */
		"GFX config",				/* 96 */
		"Priority per tile for:",	/* 97 */
		"None",						/* 98 */
		"BG1",						/* 99 */
		"BG2",						/* 100 */
		"Block priority:",			/* 101 */
		"on",						/* 102 */
		"off",						/* 103 */	
		"Blank tile:",				/* 104 */
		"Fix graphics",				/* 105 */
		"BG%d",						/* 106 */
		"BG%d(low)",				/* 107 */
		"Sprites %d",				/* 108 */			
		"Multi: ",				/* 109 */	
		"Single ",				/* 110 */	
		"Local: Host ",				/* 111 */	
		"Local: Guest ",				/* 112 */	
};

