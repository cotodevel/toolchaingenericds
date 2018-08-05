#include "typedefsTGDS.h"
#include "dsregs.h"
#include "dsregs_asm.h"

sint8*  g_snemulds_str_spa[] = 
{
		"Inicializando FS...\n",			/* 0 */
		"Fallo en FS, contin�a igualmente...\n",				/* 1 */
		"�FS Inicializado!\n",				/* 2 */
		"",								/* 3 */
		"SNEmulDS 0.6 Edici�n de Aniversario, "
		"1997-2007: �SNEmul tiene ya 10 a�os!",	/* 4 */
		"Cr�ditos:",								/* 5 */
		"",								/* 6 */
		"",								/* 7 */
		"",								/* 8 */
		"",								/* 9 */
		"Ok \001",							/* 10 */
		"Cancelar \002",						/* 11 */
		"Aplicar \003",						/* 12 */
		"Guardar \004",						/* 13 */
		"",								/* 14 */
		"",								/* 15 */
		"",								/* 16 */
		"",								/* 17 */
		"",								/* 18 */
		"",								/* 19 */
		"Seleccionar ROM",					/* 20 */
		"Cargar Juego",					/* 21 */
		"Guardar Juego",					/* 22 */
		"Opciones",						/* 23 */
		"SPC Jukebox",						/* 24 */
		"Avanzado",						/* 25 */		
		"",								/* 26 */
		"Reset",						/* 27 */
		"Guardar SRAM",					/* 28 */
		"",								/* 29 */		
		"Sonido Habilitado",				/* 30 */
		"Sonido Deshabilitado",				/* 31 */
		"Velocidad:",						/* 32 */
		"VBlank enabled ",					/* 33 */
		"VBlank disabled ",					/* 34 */
		"",								/* 35 */
		"Opciones de pantalla",		/* 36 */
		"Opciones de Fondos y Sprites",	/* 37 */
		"Hacks de Velocidad:",				/* 38 */		
		"Sin Hacks",					/* 39 */
		"Cycles Hacks",				/* 40 */
		"Interrumpir Hacks",			/* 41 */
		"Todos los Hacks",				/* 42 */
		"",							/* 43 */
		"HUD:",					/* 44 */
		"normal",				/* 45 */
		"ajustar",				/* 46 */
		"ajustar m�s",			/* 47 */
		"",							/* 48 */
		"YScroll:",					/* 49 */
		"medio",			/* 50 */
		"alto",				/* 51 */
		"bajo",			/* 52 */
		"+24",							/* 53 */
		"Escalado:",							/* 54 */
		"sin escalado",		/* 55 */
		"medio",			/* 56 */
		"pantalla completa",		/* 57 */
		"",							/* 58 */
		"",							/* 59 */
		"",		/* 60 */
		"",			/* 61 */
		"Los fondos y los Sprites pueden ser ordenados autom�ticamente "
		"o puedes escoger tu el orden "
		"(el n�mero m�s bajo, significa una prioridad m�s alta).",	/* 62 */
		"",							/* 63 */
		"",							/* 64 */		
		"Orden autom�tico",			/* 65 */
		"Capa #%d",					/* 66 */
		"Sprites",					/* 67 */
		"",							/* 68 */
		"OFF",						/* 69 */
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
		"Guardado autom�tico de SRAM",	/* 82 */
		"",							/* 83 */
		"",							/* 84 */
		"Utiliza la expansi�n de memoria",			/* 85 */
		"",							/* 86 */
		"Partida guardada correctamente", /* 87 */
		"",							/* 88 */
		"",							/* 89 */
		"T�tulo:",					/* 90 */
		"Tama�o:",					/* 91 */
		"Tipo de ROM:",				/* 92 */
		"Territorio:",					/* 93 */
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