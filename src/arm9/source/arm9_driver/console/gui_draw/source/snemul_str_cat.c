#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"

sint8*  g_snemulds_str_cat[] = 
{
		"FS Iniciant el sistema...\n",			/* 0 */
		"FS Error, continuar igualment...\n",				/* 1 */
		"FS Succ�s!\n",				/* 2 */
		"",								/* 3 */
		"SNEmulDS 0.6 versi� d'aniversari, "
		"1997-2007: SNEmul fa 10 anys!",	/* 4 */
		"Cr�dits:",								/* 5 */
		"",								/* 6 */
		"",								/* 7 */
		"",								/* 8 */
		"",								/* 9 */
		"Confirmar \001",							/* 10 */
		"Cancel�lar \002",						/* 11 */
		"Aplicar \003",						/* 12 */
		"Guardar \004",						/* 13 */
		"",								/* 14 */
		"",								/* 15 */
		"",								/* 16 */
		"",								/* 17 */
		"",								/* 18 */
		"",								/* 19 */
		"Seleccionar ROM",					/* 20 */
		"Carregar Partida",					/* 21 */
		"Guardar Partida",					/* 22 */
		"Opcions",						/* 23 */
		"SPC Jukebox",						/* 24 */
		"Opcions Avan�ades",						/* 25 */		
		"",								/* 26 */
		"Reset",						/* 27 */
		"Save SRAM",					/* 28 */
		"",								/* 29 */		
		"S� activat",				/* 30 */
		"S� desactivat",				/* 31 */
		"Velocitat:",						/* 32 */
		"VBlank enabled ",					/* 33 */
		"VBlank disabled ",					/* 34 */
		"",								/* 35 */
		"Opcions del Screen Layout",		/* 36 */
		"Opcions de Backgrounds i Sprites",	/* 37 */
		"Optimitzaci�",				/* 38 */		
		"Sense Optimitzaci�",					/* 39 */
		"Optimitzaci� per cicles",				/* 40 */
		"Interrompre Optimitzaci�",			/* 41 */
		"Totes les Optimitzacions",				/* 42 */
		"",							/* 43 */
		"HUD:",					/* 44 */
		"normal",				/* 45 */
		"ajustar",				/* 46 */
		"ajustar m�s",			/* 47 */
		"",							/* 48 */
		"Posici�Y:",					/* 49 */
		"Central",			/* 50 */
		"Dalt",				/* 51 */
		"Baix",			/* 52 */
		"+24",							/* 53 */
		"Escala:",							/* 54 */
		"sense escala",		/* 55 */
		"mitjana",			/* 56 */
		"pantalla completa",		/* 57 */
		"",							/* 58 */
		"",							/* 59 */
		"",		/* 60 */
		"",			/* 61 */
		"Les backgrounds i els sprites poden ser ordenades autom�ticament "
		"o tu pots seleccionar el ordre "
		"(el numero m�s baix, es el que te una prioritat m�s alta).",	/* 62 */
		"",							/* 63 */
		"",							/* 64 */		
		"Ordenar automaticament",			/* 65 */
		"Capes #%d",					/* 66 */
		"Sprites",					/* 67 */
		"",							/* 68 */
		"desactivat",						/* 69 */
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
		"Guardar SRAM autom�ticament",	/* 82 */
		"",							/* 83 */
		"",							/* 84 */
		"Usar memory pak",			/* 85 */
		"",							/* 86 */
		"Partida guardada amb �xit", /* 87 */
		"",							/* 88 */
		"",							/* 89 */
		"T�tol:",					/* 90 */
		"Tamany:",					/* 91 */
		"Tipus de ROM:",				/* 92 */
		"Regi�:",					/* 93 */
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

