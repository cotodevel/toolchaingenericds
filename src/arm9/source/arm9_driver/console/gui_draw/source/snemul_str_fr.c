#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"

sint8*  g_snemulds_str_fr[] = 
{
		"Initialisation du syst�me...\n",			/* 0 */
		"Echec, continue malgr� tout...\n",				/* 1 */
		"Le syst�me a �t� initialis� correctement!\n",				/* 2 */
		"",								/* 3 */
		"SNEmulDS 0.6 sp�cial anniversaire, "
		"1997-2007: SNEmul a 10 ans!",	/* 4 */
		"Cr�dits:",								/* 5 */		
		"",								/* 6 */
		"",								/* 7 */
		"",								/* 8 */
		"",							/* 9 */
		"Confirmer \001",							/* 10 */
		"Annuler \002",						/* 11 */
		"Appliquer \003",								/* 12 */
		"Sauver \004",							/* 13 */
		"",								/* 14 */
		"",								/* 15 */
		"",								/* 16 */
		"",								/* 17 */
		"",								/* 18 */
		"",								/* 19 */
		"Choix du jeu",					/* 20 */
		"Charger partie",					/* 21 */
		"Sauver partie",					/* 22 */
		"Options",						/* 23 */
		"Jukebox SPC",						/* 24 */
		"Avanc�",						/* 25 */
		"",								/* 26 */
		"Reset",						/* 27 */
		"Sauver SRAM",					/* 28 */
		"",								/* 29 */		
		"Son activ�",						/* 30 */
		"Son d�sactiv�",					/* 31 */
		"Vitesse:",						/* 32 */
		"VBlank enabled ",					/* 33 */
		"VBlank disabled ",					/* 34 */
		"",								/* 35 */
		"Options �cran",					/* 36 */
		"Options d�cors et sprites",					/* 37 */
		"Optimisations:",				/* 38 */
		"Aucune optimisation",					/* 39 */
		"Optimisations cycles",				/* 40 */
		"Optimisations interrupt",			/* 41 */
		"Toutes les optimisations",				/* 42 */
		"",							/* 43 */
		"HUD:",							/* 44 */
		"normale",				/* 45 */
		"�cras�e",				/* 46 */
		"tr�s �cras�e",			/* 47 */
		"",							/* 48 */
		"Vertical:",							/* 49 */
		"milieu",			/* 50 */
		"haut",				/* 51 */
		"bas",			/* 52 */
		"+24",							/* 53 */
		"D�formation:",							/* 54 */
		"aucune",		/* 55 */
		"l�g�re",			/* 56 */
		"plein �cran",		/* 57 */
		"",							/* 58 */
		"",							/* 59 */
		"",		/* 60 */
		"",			/* 61 */
		"L'ordre des d�cors et des sprites peut �tre ajust� automatiquement, "
		"ou vous pouvez r�gler l'ordre (plus la valeur est faible, plus la "
		"priorit� est grande).",		/* 62 */
		"",							/* 63 */
		"",							/* 64 */		
		"Ordre des d�cors et sprites automatique",	/* 65 */
		"Plan %d",					/* 66 */
		"Sprites",						/* 67 */
		"",							/* 68 */
		"cach�",						/* 69 */
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
		" ",							/* 80 */
		"X",						/* 81 */
		"Sauvegarder automatiquement la SRAM",	/* 82 */
		"",							/* 83 */
		"",							/* 84 */
		"Utilise une extension m�moire",			/* 85 */
		"",							/* 86 */
		"Partie sauvegard�e", /* 87 */
		"",							/* 88 */
		"",							/* 89 */
		"Titre:",					/* 90 */
		"Taille:",					/* 91 */
		"Type ROM:",				/* 92 */
		"Pays:",					/* 93 */
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
};
