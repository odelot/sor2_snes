#include <snes.h>
#include "soundbank.h"


int offset = 0;

extern char __SOUNDBANK__0;
extern char __SOUNDBANK__1;

extern char blaze, blaze_end;
extern char blaze_palette, blaze_palette_end;

extern char galsia_tiles, galsia_tiles_end;
extern char galsia_palette, galsial_palette_end;

extern char signal_tiles, signal_tiles_end;
extern char signal_palette, signal_palette_end;


extern char patterns1,patterns1_end;
extern char palette,palette_end;
extern char map, map_end;



extern char patterns2,patterns2_end;
extern char palette2, palette2_end;
extern char map2, map2_end;

extern char caixa, caixa_end;
extern char caixa_palette, caixa_palette_end;

extern char hud, hud_end;
extern char hud_map, hud_map_end;
extern char hud_pal, hud_pal_end;


extern char socobrr,socobrrend;
brrsamples socosound;

extern char danobrr,danobrrend;
brrsamples danosound;

//numero maximo de personagens na tela
#define  MAXPERSONAGENS 5

//definindo estados
#define IDLE 0
#define SOCO 1
#define ANDANDO 2
#define DANO 3

//define sub estados
#define IDLE_JOGADOR_1 0
#define IDLE_JOGADOR_2 1
#define IDLE_JOGADOR_3 2
#define IDLE_JOGADOR_4 3

#define SOCO_JOGADOR_1 4
#define SOCO_JOGADOR_2 5
#define SOCO_JOGADOR_3 6

#define ANDANDO_JOGADOR_1 7
#define ANDANDO_JOGADOR_2 8
#define ANDANDO_JOGADOR_3 9
#define ANDANDO_JOGADOR_4 10
#define ANDANDO_JOGADOR_5 11
#define ANDANDO_JOGADOR_6 12

#define IDLE_INIMIGO_1 0
#define ANDANDO_INIMIGO_1 1
#define ANDANDO_INIMIGO_2 2
#define SOCO_INIMIGO_1 3
#define DANO_SIMPLES_INIMIGO_1 4


#define HUDSIZE 160

typedef struct _personagem personagem;
typedef struct _sprite sprite;
typedef struct _scroll scroll;

struct _sprite {
	u8* tiles;	
    u8* paleta;
    u16 estado;
    u16 proximoEstado;
    u16 proximoSubEstado;
    u16 proximoFrame;
	u8 xmin;
	u8 xmax;
	u8 ymin;
	u8 ymax;
	bool hitbox;
};

struct _personagem {
	u16 tileSize;
	u16 paletteSize;
	u8 address;
	sprite* sprites;

	u16 _framesDoSprite;
	u16 _vramAddress;
	u16 _vramOffset;
	unsigned char _estado;
    unsigned char _subEstado;

    unsigned char _estadoAntigo;
    unsigned char _subEstadoAntigo;
	int _x;
    int _y;
	int _oldx;
    int _oldy;
	char _vX;
	char _vY;
	u8 speed;
	u8 speedRefresh;
	u8 hflip;
	bool ativo;
	u8 logicState;
	u8 prioridade;
	u8 invisivel;
	u8 _paletteOffset;

	bool inimigo;
	bool dano;
	bool invencivel;
	u8 danoDelay;
	u8 danoCountdown;
	u8 danoAtual;
	bool shakeDano;
	u8 vida;
	u8 vidaTotal;
	u8 inimigo;
	char * nome;


};

struct _scroll {
	u8 id;
	u16 vram;
	u16 scrX;
	u16 scrY;
	u8* bgMap;
	u8 mapPage;
	bool vRamFirstPage;
	u16 scrollX;
	u16 maxScrollX;


};

int danoSFX = 0;

scroll bgMain;
scroll bgSub;

personagem *player1;
personagem personagens[5];

personagem * yOrder[5];
personagem * inimigoApanhando;
personagem * ultimoInimigoApanhando;

bool debug = false;

bool disableLogic = false;

sprite blazeGfx[13];
char blazeName[] = "blaze\0";

sprite galsiaGfx[5];
char galsiaName[] = "galsia\0";

sprite signalGfx[5];
char signalName[] = "signal\0";

unsigned short pad0;


bool hudPlayerChanged = false;
u8 hudPlayerState = 0;
u8 hudFrames = 0;

bool hudOpponentChanged = false;

u16 hudBuffer[160];
u8 paletteBuffer[16*4];

u8 chances = 4;



//dynamic sprites

unsigned char spr_queue, spr_mutex, bg_mutex;

typedef struct {
	u8 *gfxoffset;
	u16 adrgfxvram;
	u16 size;

} spritequeue;

spritequeue sprqueue[16];   // Max 16 entries in queue


typedef struct {
	spritequeue bg1;
	spritequeue bg2;
	u8 refreshBG1;
	u8 refreshBG2;

} background;

background bgInfo;

void addSprite(u8 *pgfx, u16 adrspr, int size) {
	spr_mutex = 1; // to avoid vbl during queue management
	spr_queue++;
	sprqueue[spr_queue].gfxoffset = pgfx;
	sprqueue[spr_queue].adrgfxvram = adrspr;
	sprqueue[spr_queue].size = size;
	spr_mutex = 0;
}

void updateBG1 (u8 *pgfx, u16 adrspr, int size) {
	bg_mutex = 1; // to avoid vbl during queue management
	bgInfo.bg1.adrgfxvram = adrspr;
	bgInfo.bg1.gfxoffset = pgfx;
	bgInfo.bg1.size = size;
	bgInfo.refreshBG1 = true;
	bg_mutex = 0; // to avoid vbl during queue management
}

void updateBG2 (u8 *pgfx, u16 adrspr, int size) {
	bg_mutex = 1; // to avoid vbl during queue management
	bgInfo.bg2.adrgfxvram = adrspr;
	bgInfo.bg2.gfxoffset = pgfx;
	bgInfo.bg2.size = size;
	bgInfo.refreshBG2 = true;
	bg_mutex = 0; // to avoid vbl during queue management
}


void computeBoundaryBox (sprite* s) {
	int i,j,x,y,b,t;
    int minX = 64;
    int maxX = 0;
    int minY = 64;
    int maxY = 0;

    int bitplanes = 4;
	int idx=0;
    int linha =0;
    int coluna = 0;
    for(t = 0; t < 64; t++)
    {
        
        if (t!=0 && t % 8 == 0) {
            idx += 8 * 2 * 8 * 2;         
            linha +=8;
            coluna = 0;
        }
        int tile [8][8];
        memset (tile,0,8*8*sizeof(int));
        for(b=0;b<bitplanes;b+=2) { //loop through bitplane pairs
            for(y=0;y<8;y++) {

                unsigned char data =s->tiles[idx++];
                int mask = 1;
                for(x=0;x<8;x++) {
                    if ( (data & mask) != 0)
						tile[y][(7-x)]+=1;
					mask = mask << 1;
                }


                data =s->tiles[idx++];
                mask = 1;
                for(x=0;x<8;x++) {
                    if ( (data & mask) != 0)
						tile[y][(7-x)]+=1;
					mask = mask << 1;
                }
            }
        }        
        for (y = 0; y <  8; y += 1) {
            for (x = 0; x <  8; x += 1) {
                if (tile[y][x]!= 0 ){
                    int newX = coluna + x;
                    int newY = linha + y;
                    if (newX < minX) {
                        minX = newX;
                    }
                    if (newY < minY) {
                        minY = newY;
                    }

                    if (newX > maxX) {
                        maxX = newX;
                    }
                    if (newY > maxY) {
                        maxY = newY;
                    }
                }                
            }             
        }        
        coluna += 8;
    }
	s->xmin = minX;
	s->xmax = maxX;
	s->ymin = minY;
	s->ymax = maxY;

}


setSprite (
	sprite* s, 
	u8 estado,
	u8 subEstado,
	u8* tiles,
	u8* paleta,
    u8 proximoEstado,
    u8 proximoSubEstado,
    u8 proximoFrame,
	u8 xmin,
	u8 xmax,
	u8 ymin,
	u8 ymax,
	bool hitbox) {
		s[subEstado].estado = estado;
		s[subEstado].paleta = paleta;		
		s[subEstado].proximoEstado = proximoEstado;
		s[subEstado].proximoFrame = proximoFrame;
		s[subEstado].proximoSubEstado = proximoSubEstado;
		s[subEstado].tiles = tiles;
		s[subEstado].xmin = xmin;
		s[subEstado].xmax = xmax;
		s[subEstado].ymax = ymin;
		s[subEstado].ymin = ymax;
		s[subEstado].hitbox = hitbox;

		//computeBoundaryBox (s);


	}

void initPersonagem (personagem *p, u16 tileSize, u16 paletteSize) {
	p->_estado = 0;
	p->_subEstado = 0;
	p->_estadoAntigo = 255;
	p->_subEstadoAntigo = 255;
	p->tileSize = tileSize;
	p->paletteSize = paletteSize;
	p->_framesDoSprite = 0;
	p->_x = 0;
	p->_y = 164;
	p->speed = 1;
	p->speedRefresh = 0;
	p->hflip = 0;
	p->ativo = true;
	p->logicState = 0;
	p->prioridade = 2;
	p->invisivel = false;
	int i;
	for (i=0; i<MAXPERSONAGENS; i+=1) {
		if (yOrder[i]==NULL) {
			yOrder[i] = p;
			break;
		}
	}
	p->dano = false;
	p->danoAtual = 0;
	p->danoCountdown = 0;
	p->danoDelay = 30;

	p->invisivel = false;
	p->shakeDano = false;

	
	

	


}
void initBlaze (personagem *p) {
	setSprite (blazeGfx, IDLE, IDLE_JOGADOR_1, &blaze, &blaze_palette, IDLE, IDLE_JOGADOR_2, 14,0,63,0,63,false);
	setSprite (blazeGfx, IDLE, IDLE_JOGADOR_2, &blaze+0x100, &blaze_palette, IDLE, IDLE_JOGADOR_3, 8,0,63,0,63,false);
	setSprite (blazeGfx, IDLE, IDLE_JOGADOR_3, &blaze+0x1000, &blaze_palette, IDLE, IDLE_JOGADOR_4, 7,0,63,0,63,false);
	setSprite (blazeGfx, IDLE, IDLE_JOGADOR_4, &blaze+0x100, &blaze_palette, IDLE, IDLE_JOGADOR_1, 9,0,63,0,63,false);

	setSprite (blazeGfx, SOCO, SOCO_JOGADOR_1, &blaze+0x1000 + 0x100, &blaze_palette, SOCO, SOCO_JOGADOR_2, 5,0,63,0,63,false);
	setSprite (blazeGfx, SOCO, SOCO_JOGADOR_2, &blaze+0x2000, &blaze_palette, SOCO, SOCO_JOGADOR_3, 5,45,63,44,50,true);
	setSprite (blazeGfx, SOCO, SOCO_JOGADOR_3, &blaze+0x1000 + 0x100, &blaze_palette, IDLE, IDLE_JOGADOR_1, 5,0,63,0,63,false);

	setSprite (blazeGfx, ANDANDO, ANDANDO_JOGADOR_1, &blaze+0x2000+0x100, &blaze_palette, ANDANDO, ANDANDO_JOGADOR_2, 8,0,63,0,63,false);
	setSprite (blazeGfx, ANDANDO, ANDANDO_JOGADOR_2, &blaze+0x3000, &blaze_palette, ANDANDO, ANDANDO_JOGADOR_3, 8,0,63,0,63,false);
	setSprite (blazeGfx, ANDANDO, ANDANDO_JOGADOR_3, &blaze+0x3000+0x100, &blaze_palette, ANDANDO, ANDANDO_JOGADOR_4, 7,0,63,0,63,false);
	setSprite (blazeGfx, ANDANDO, ANDANDO_JOGADOR_4, &blaze+0x4000, &blaze_palette, ANDANDO, ANDANDO_JOGADOR_5, 8,0,63,0,63,false);
	setSprite (blazeGfx, ANDANDO, ANDANDO_JOGADOR_5, &blaze+0x4000+0x100, &blaze_palette, ANDANDO, ANDANDO_JOGADOR_6, 8,0,63,0,63,false);
	setSprite (blazeGfx, ANDANDO, ANDANDO_JOGADOR_6, &blaze+0x5000, &blaze_palette, ANDANDO, ANDANDO_JOGADOR_1, 7,0,63,0,63,false);
	p->vida = 18;
	p->vidaTotal = 18;
	p->inimigo = false;
	p->nome = &blazeName[0];



	p->sprites = blazeGfx;
}

void initGalsia (personagem *p) {
	setSprite (galsiaGfx, IDLE, IDLE_INIMIGO_1, &galsia_tiles, &galsia_palette, ANDANDO, IDLE_INIMIGO_1, 1000,0,63,0,63,false);
	setSprite (galsiaGfx, ANDANDO, ANDANDO_INIMIGO_1, &galsia_tiles+0x100, &galsia_palette, ANDANDO, ANDANDO_INIMIGO_2, 10,0,63,0,63,false);
	setSprite (galsiaGfx, ANDANDO, ANDANDO_INIMIGO_2, &galsia_tiles+0x1000, &galsia_palette, ANDANDO, ANDANDO_INIMIGO_1, 10,0,63,0,63,false);
	setSprite (galsiaGfx, SOCO, SOCO_INIMIGO_1, &galsia_tiles+0x1000+0x100, &galsia_palette, IDLE, IDLE_INIMIGO_1, 9,0,63,0,63,false);
	setSprite (galsiaGfx, DANO, DANO_SIMPLES_INIMIGO_1, &galsia_tiles+0x2000+0x100, &galsia_palette, IDLE, IDLE_INIMIGO_1, 1000,0,63,0,63,false);
	p->vida = 6;
	p->vidaTotal = 6;
	p->inimigo = true;
	p->nome = &galsiaName[0];


	p->sprites = galsiaGfx;
}

void initSignal (personagem *p) {
	setSprite (signalGfx, IDLE, IDLE_INIMIGO_1, &signal_tiles, &signal_palette, ANDANDO, IDLE_INIMIGO_1, 1000,26,38,4,63,false);
	setSprite (signalGfx, ANDANDO, ANDANDO_INIMIGO_1, &signal_tiles+0x100, &signal_palette, ANDANDO, ANDANDO_INIMIGO_2, 10,0,63,0,63,false);
	setSprite (signalGfx, ANDANDO, ANDANDO_INIMIGO_2, &signal_tiles+0x1000, &signal_palette, ANDANDO, ANDANDO_INIMIGO_1, 10,0,63,0,63,false);
	setSprite (signalGfx, SOCO, SOCO_INIMIGO_1, &signal_tiles+0x1000+0x100, &signal_palette, IDLE, IDLE_INIMIGO_1, 9,0,63,0,63,false);
	setSprite (signalGfx, DANO, DANO_SIMPLES_INIMIGO_1, &signal_tiles+0x2000+0x100, &signal_palette, IDLE, IDLE_INIMIGO_1, 1000,0,63,0,63,false);
	p->vida = 10;
	p->vidaTotal = 10;
	p->inimigo = true;
	p->nome = &signalName[0];


	p->sprites = signalGfx;
}

void mudaEstado (personagem *p, int estado, int subEstado) {
	if (p->inimigo == false) {
		if (p->_subEstado == SOCO_JOGADOR_2 ) {
			//spcSetSoundEntryspcSetSoundEntry(15, 15, 6, &socobrrend-&socobrr, &socobrr, &socosound);
			if (danoSFX == 0) {
				spcSetSoundTableEntry (&socosound);
				spcPlaySound(0);
			}
		}
	}
	
	p->_estadoAntigo = p->_estado;
	p->_subEstadoAntigo = p->_subEstado;

	p->_subEstado = subEstado;
	p->_estado = estado;
	p->_framesDoSprite = 0;
}

initGFX (personagem *p, u16 address,u16 vramAddress, u16 vramOffset, u8 tilePaletteNumber) {
	p->address = address;
	p->_vramAddress = vramAddress;
	p->_vramOffset = vramOffset;
	unsigned char palEntry;	
	WaitForVBlank(); 	
	// Init tiles
	dmaCopyVram(p->sprites[0].tiles, (vramAddress), p->tileSize);
	
	// Init palette
	u8 palEntry = (128+tilePaletteNumber*16);
  	dmaCopyCGram(p->sprites[0].paleta, palEntry, p->paletteSize);	
	p->_paletteOffset = tilePaletteNumber;
	// Define sprites parameters
	oamSet(address,  0, 0, p->prioridade, 0, 0, vramOffset, tilePaletteNumber); // Put sprite in 100,100, with maximum priority 3 from tile entry 0, palette 0
	oamSetEx(address, OBJ_LARGE, OBJ_SHOW);

}

void updateGfx (personagem* p) {
	int estadoAntigo = p->_estado;
	int subestadoAntigo = p->_subEstado;
	if (p->_subEstado != p->_subEstadoAntigo) {
		//tem que trocar o sprite
		p->_subEstadoAntigo = p->_subEstado;
		p->_estadoAntigo = p->_estado;
		addSprite (p->sprites[p->_subEstado].tiles, p->_vramAddress, p->tileSize);
		return;
	}
		
	if (p->_framesDoSprite == p->sprites[p->_subEstado].proximoFrame) {
			mudaEstado (p, p->sprites[p->_subEstado].proximoEstado, p->sprites[p->_subEstado].proximoSubEstado);					
	}
    if (p->_subEstado != subestadoAntigo) {
		//tem que trocar o sprite
		addSprite (p->sprites[p->_subEstado].tiles, p->_vramAddress, p->tileSize);		
	}
	p->_framesDoSprite += 1;
	
}

void move (personagem *p, int x, int y) {
	p->_vX = x;
	p->_vY = y;
}

unsigned int backgroundAnimControl = 0;
bool backgroundAnim = false;

// VERY VERY INEFFICIENT WAY, BUT WORKS
void animBackground () {			
			u8 paletteEntry = 2;
			u16 paletteSize = 16*4;
			u16  palEntry = paletteEntry*BG_16COLORS;
			if (backgroundAnim == 0) {
				backgroundAnim = !backgroundAnim;

				WaitForVBlank();
				setPaletteColor (21+32,RGB15(31,4,0)); //24
				setPaletteColor (24+32,RGB15(31,31,0)); //21				
				setPaletteColor (18+32,RGB15(31,31,0)); //21
				setPaletteColor (20+32,RGB15(0,4,13)); //17
				setPaletteColor (26+32,RGB15(9,31,31)); //20
				setPaletteColor (23+32,RGB15(0,9,0)); //20
				setPaletteColor (25+32,RGB15(4,31,0)); //20
				
			  //WaitForVBlank();
				//memcpy (paletteBuffer, &palette,paletteSize);
				//change palette here just one time
				//dmaCopyCGram(paletteBuffer, palEntry, paletteSize);
				

			} else {
				backgroundAnim = !backgroundAnim;
				WaitForVBlank();
				dmaCopyCGram(&palette, palEntry, paletteSize);
			}
}

void animBackgroundControl () {
	backgroundAnimControl += 1;
	if (backgroundAnimControl == 12) {
		animBackground ();
		backgroundAnimControl = 0;
	}
}


void enableDano (personagem *p) {
	if (p->inimigo == true) {
		inimigoApanhando = p;
	}
	if (p->invencivel == false) {
		danoSFX = 1;		
		spcSetSoundTableEntry(&danosound);
		spcPlaySound(0);

		if (p->vida > 0) //TODO: QUANDO FOR ZERO FAZER O CARA MORRER
			p->vida -= 1; //TODO: tirar vida de forma diferente?
		p->dano = true;
		p->danoCountdown = p->danoDelay;
		p->danoAtual = 0;
		p->invencivel = true;
		if (p->inimigo == true) {
			hudOpponentChanged = true;
		}
	}
}

void updateLogicNPC (personagem *p) {

	if (p->dano == true) {
		if (p->_estado != DANO) {
			mudaEstado (p, DANO, DANO_SIMPLES_INIMIGO_1);				
		}

		if (p->danoCountdown != 0) {
			p->danoCountdown -= 1;
			p->danoAtual += 1;
		} else {
			p->dano = false;
			danoSFX = 0;
			mudaEstado (p, IDLE, IDLE_INIMIGO_1);
		}
		if (p->danoAtual >= 7) {
			p->invencivel = false;
		}

		if ( (p->danoCountdown & 4) == 0 ){
			if (p->shakeDano == true) {
				p->_x -= 2;
				p->shakeDano = false;
			} else {
				p->_x += 2;
				p->shakeDano = true;
			}
		}
	}
}


void updateLogic (personagem *p, unsigned short pad) {
	if (p->_estado != SOCO) {

		if ((pad & KEY_UP) || (pad & KEY_DOWN) || (pad & KEY_LEFT) || (pad & KEY_RIGHT) ) {
			if (p->_estado != ANDANDO) {
				mudaEstado (p, ANDANDO, ANDANDO_JOGADOR_1);
			}    
		}

		if (pad & KEY_UP ) {
			move (p,0,-1);

		}
		if (pad & KEY_DOWN ) {
			move (p,0,+1);
		}
		if (pad & KEY_LEFT ) {
			move (p,-3,0);
			//move (p,-2,0);
		}
		if (pad & KEY_RIGHT ) {
			//move (p,2,0);
			move (p,3,0);
		}
		if (p->_estado == ANDANDO) {
			if ((pad & KEY_UP) == 0 && (pad & KEY_DOWN) == 0 && (pad & KEY_LEFT) == 0 && (pad & KEY_RIGHT) == 0 ) {
				move (p,0,0);
				mudaEstado (p, IDLE, IDLE_JOGADOR_1);
			}
		 }
	}
	if (p->_estado != SOCO) {
		if (pad & KEY_A ) {
			move (p,0,0);
			mudaEstado (p,SOCO, SOCO_JOGADOR_1);
		}
		if (pad & KEY_B ) {			
		}
		if (pad & KEY_Y) {
		}
	}
}

void oamSetXYEx2(unsigned int id, unsigned int x, unsigned char y) {
	#define OAM_HI_TABLE_START 128*4

    // Set low byte of x position and y position:
    unsigned char x_lo = (unsigned char)x;
    oamSetXY(id,x,y);

    // Note that id isn't the OAM id, but a direct index into OAM shadow memory;
    // the result is that id/4 is the actual oam index, which is required for determining
    // the OAM high table position.
    unsigned char oam_id = (unsigned char)(id >> 2);

    // Set highest bit of x position: Either at bit 0,2,4,6 depending on oam id,
    // as 4 oam entries are merged into one byte of the OAM high table.
    //int bit_shift = 2*(oam_id % 4);
		int bit_shift = oam_id & 3; // (oam_id % 4);
		bit_shift = bit_shift << 1; // (oam_id % 4) * 2

    int in_byte_position = 1 << bit_shift;

    //int oam_high_table_pos = OAM_HI_TABLE_START + oam_id / 4;
		int oam_high_table_pos = (oam_id >> 4) + OAM_HI_TABLE_START;

    oamMemory[oam_high_table_pos] &= ~in_byte_position; // Clear current high bit of x
    oamMemory[oam_high_table_pos] |= (x>>8)<<bit_shift; // Fill in new high bit of x
} 

void updatePos (personagem* p) {
	p->_oldx = p->_x;
	p->_oldy = p->_y;
	if (p->_vX != 0) {
		if (p->speedRefresh == 0) {
			p->_x += p->_vX;			
		}
		if (p->_vX > 0) {
			p->hflip = 0;
		} else {
			p->hflip = 1;
		}
	}
	if (p->_vY != 0) {
		if (p->speedRefresh == 0) {
			p->_y += p->_vY;
		}
	}

	if (p->speedRefresh == 0) {
		p->speedRefresh = p->speed;
	} else {
		p->speedRefresh -= 1;
	}
	if (p->_x >= 218)
		p->_x = 218;
	if (p->_x <= -30)
		p->_x = -30;
		

	oamFlip(p->address, p->hflip, 0);
	oamSetXY (p->address,  p->_x, p->_y -64); //GAMBIARRA	

}


void updatePosNPC (personagem* p) {
	p->_oldx = p->_x;
	p->_oldy = p->_y;
	if (p->_vX != 0) {
		if (p->speedRefresh == 0) {
			p->_x += p->_vX;			
		}
		if (p->_vX > 0) {
			p->hflip = 0;
		} else {
			p->hflip = 1;
		}
	}
	if (p->_vY != 0) {
		if (p->speedRefresh == 0) {
			p->_y += p->_vY;
		}
	}

	if (p->speedRefresh == 0) {
		p->speedRefresh = p->speed;
	} else {
		p->speedRefresh -= 1;
	}

	oamFlip(p->address, p->hflip, 0);
	oamSetXY (p->address,  p->_x, p->_y -64); //GAMBIARRA
	if (p->_x < -64)
		oamSetVisible(p->address, OBJ_HIDE);

}

void updatePersonagemJogavel (personagem* p, unsigned short pad) {	
	updateLogic (p,pad);
	updatePos (p);	
	updateGfx (p);
}

void updatePersonagemNPC (personagem* p) {	
	updateLogicNPC (p);
	updatePosNPC (p);
	updateGfx (p);
}

void handleScroll (personagem* p, scroll* s) {
	u8 i;

	if ( p->_x > 94) {
		if (s->scrX < s->maxScrollX) {
			s->scrX += 1;

			for (i = 0; i < MAXPERSONAGENS; i += 1) {
				if (personagens[i].ativo == true) {
					personagens[i]._x -= 1;
				}

			}
			
		}
		if ( (s->scrX >= (s->mapPage+1)*256) )  {
			s->mapPage += 1;
			s->vRamFirstPage = !s->vRamFirstPage;
			u16 vram = s->vram;
			if (s->vRamFirstPage == false) {
				vram += 1024;
			}			
			updateBG1 (s->bgMap + 2048 * s->mapPage,vram,2048);
		}
		bgSetScroll(s->id,s->scrX-255,s->scrY-223);
	}
	
	
	
}

void handleScrollSub (personagem* p, scroll* s) {

	if ( p->_x > 94) {
		if (s->scrX < s->maxScrollX) {
			s->scrX += 2;			
		}

		if ( (s->scrX >= (s->mapPage+1)*256) )  {			
			s->mapPage += 1;
			s->vRamFirstPage = !s->vRamFirstPage;
			u16 vram = s->vram;
			if (s->vRamFirstPage == false) {
				vram += 1024;
			}			
			updateBG2 (s->bgMap + 2048 * s->mapPage,vram,2048);
		}
		bgSetScroll(s->id,s->scrX-255,s->scrY-223);
	}
	
	
	
}

void oamSwap (personagem* p1, personagem* p2) {
	u8 p1A = p1->address;
	u8 p2A = p2->address;
	u8 oamBuffer[4];
	memcpy(oamBuffer, &oamMemory[p1A], 4);
	memcpy( &oamMemory[p1A],&oamMemory[p2A], 4);
	memcpy(&oamMemory[p2A], oamBuffer, 4);
	p2->address = p1->address;
	p1->address = p2A;
	
	
}

void handleYorder () {
	int i;
	for (i = 0; i < MAXPERSONAGENS-1; i+=1) {
		if (yOrder[i]!= NULL) {
			if (yOrder[i+1] != NULL) {
				if (yOrder[i+1]->_y > yOrder[i]->_y){
					personagem *aux = yOrder[i];
					yOrder[i] = yOrder[i+1];
					yOrder[i+1] = aux;
					oamSwap (yOrder[i],yOrder[i+1]);
				}
			}
		} else {
			if (yOrder[i+1] != NULL) {
				yOrder[i] = yOrder[i+1];
			}
		}
	}
}

void writeNum (u16 pos, u8 num) {	
	int offset = 160;
	u16 n=0x28df+1;
	n+= num;
	hudBuffer[pos] = n;
	n=0x28e9+1;
	n+= num;
	hudBuffer[pos+32] = n;
}


handleHitBox () {
	int i;
	//writeNum (100,0);
	for (i=0; i< MAXPERSONAGENS; i+=1) {
		if (personagens[i].ativo == true) {
			sprite * spriteAtual = &personagens[i].sprites[personagens[i]._subEstado];
			if (spriteAtual->hitbox == true) {
				//writeNum (100,1);
				int j;
				for (j=0; j < MAXPERSONAGENS; j += 1) {
					if (j != i) {
						if (personagens[j].ativo == true) {
							int y1 = personagens[i]._y;
							int y2 = personagens[j]._y;
							if ( y2 < (y1 + 4) && y2 > (y1 - 4) ) {	 
								//estao na mesma faixa
								//writeNum (100,2);
								sprite * spriteOponente = &personagens[j].sprites[personagens[j]._subEstado];
								
								 int A = personagens[i]._x + spriteAtual->xmin;
								 int B = personagens[i]._x + spriteAtual->xmax;
								 int C = personagens[j]._x + spriteOponente->xmin;
								 int D = personagens[j]._x + spriteOponente->xmax;
								if (A <= D && B >= C) {
									//colidiu em X
									//writeNum (100,3);
									 int X = personagens[i]._y - spriteAtual->ymin;
									 int Y = personagens[i]._y - spriteAtual->ymax;
									 int W = personagens[j]._y - spriteOponente->ymin;
									 int Z = personagens[j]._y - spriteOponente->ymax;
									if (X <= Z && Y >= W) {
										//colidiu em Y
										//writeNum (100,4);
										enableDano (&personagens[j]);
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

void handleCollisions () {
	int i, j;
	
	for (i=0; i< MAXPERSONAGENS; i+=1) {
		if (personagens[i].ativo == true) {
			for (j = i+1; j < MAXPERSONAGENS; j+=1) {
				if (personagens[j].ativo == true) {
					
					personagem *p1 = &personagens[i];
					personagem *p2 = &personagens[j];					
					sprite* sp1 = &p1->sprites[p1->_subEstado];
					sprite* sp2 = &p2->sprites[p2->_subEstado];
					if ( ((p1->_x + sp1->xmin) > (p2->_x + sp2->xmin) && (p1->_x + sp1->xmin) < (p2->_x + sp2->xmax)) || 
						((p1->_x + sp1->xmax) > (p2->_x + sp2->xmin) && (p1->_x + sp1->xmax) < (p2->_x + sp2->xmax)) )
					{
						if ( ((p1->_y + sp1->ymin) > (p2->_y + sp2->ymin) && (p1->_y + sp1->ymin) < (p2->_y + sp2->ymax)) || 
						((p1->_y + sp1->ymax) > (p2->_y + sp2->ymin) && (p1->_y + sp1->ymax) < (p2->_y + sp2->ymax)) )
						{							
							if (p1->_y < p2->_y) {
								if (p1->address < p2->address) {
									oamSwap (p1,p2);
								}
							} else {
								if (p2->address < p1->address) {
									oamSwap (p1,p2);
								}
							}
						}
					}
				}
			}
		}	
	}
}

void myconsoleVblank () {
	int dma = 0;
	dma += 0x220;
	dmaCopyOAram((unsigned char *) &oamMemory,0,0x220);	
	// if tile sprite queued
	if (bg_mutex == 0) {
		if (bgInfo.refreshBG1 == true) {
			dmaCopyVram(bgInfo.bg1.gfxoffset, bgInfo.bg1.adrgfxvram, bgInfo.bg1.size); 
			dma += bgInfo.bg1.size;
			bgInfo.refreshBG1 = false;
		}
	}
	if (bg_mutex == 0) {
		if (bgInfo.refreshBG2 == true) {
			dmaCopyVram(bgInfo.bg2.gfxoffset, bgInfo.bg2.adrgfxvram, bgInfo.bg2.size); 
			dma += bgInfo.bg2.size;
			bgInfo.refreshBG2 = false;
		}
	}
	//copy hud
	dmaCopyVram((u8*)hudBuffer, 0x1000, 320);
	// Refresh pad values
	dma += 320 ;

	if (spr_queue != 0xff) {
		if (spr_mutex == 0) { // if we have finished adding things 
			// copy memory to vram (2 tiles of the 16x16 sprites)
			while (spr_queue != 0xff) {
				if (dma + sprqueue[spr_queue].size > 6144)
					break;				
				dmaCopyVram(sprqueue[spr_queue].gfxoffset, sprqueue[spr_queue].adrgfxvram, sprqueue[spr_queue].size); 
				spr_queue--;
			}
		}
	}
}

void writeChar (u16 pos, char letra) {
	u16 a = 0x24c0;
	a += letra - 97;
	hudBuffer[pos] = a;
}


void writeText (u16 pos, char* text) {
	int i = 0;
	while (text[i]!='\0') {
		writeChar (pos + i, text[i]);
		i+=1;
	}
}

void initTimer () {
	hudBuffer[46]=0x20a8;
	hudBuffer[47]=0x20a8+1;
	hudBuffer[48]=0x20a8+2;
	hudBuffer[49]=0x20a8+3;

	hudBuffer[46+32]=0x20a8+4;
	hudBuffer[47+32]=0x20a8+5;
	hudBuffer[48+32]=0x20a8+6;
	hudBuffer[49+32]=0x20a8+7;

	hudBuffer[46+64]=0x20a8+8;
	hudBuffer[47+64]=0x20a8+9;
	hudBuffer[48+64]=0x20a8+10;
	hudBuffer[49+64]=0x20a8+11;

	hudBuffer[46+96]=0x20a8+12;
	hudBuffer[47+96]=0x20a8+13;
	hudBuffer[48+96]=0x20a8+14;
	hudBuffer[49+96]=0x20a8+15;
}

void initHudPersonagemJogavel () {
	u8 i;
	for (i=0; i < 9; i+=1) {
		hudBuffer[67+i] = 0x28f4;
	}
	
	hudBuffer[76] = 0x20fa+1;

	hudPlayerState = 0;
	hudFrames = 0;
}

void atualizaBarraVida (u8 vida) {
	u8 total = 18;
	u8 vidaS2 = vida >> 1;
	u8 naoVida = total - vida;
	u8 naoVidaS2 = naoVida >> 1;
	u8 i;
	for (i=0; i < vidaS2; i+=1) {
		hudBuffer[67+i] = 0x20f6;
	}
	if (vida & 1 != 0) {
		hudBuffer[67+vidaS2] =0x2cf5 ;
	} 
	for (i=vidaS2+1; i < 9; i+=1) {
		hudBuffer[67+i] = 0x28f4;
	}
}

void atualizaBarraVidaInimigo (u8 vida, u8 total) {
	
	u8 vidaS2 = vida >> 1;	
	u8 totalS2 = total >> 1;
	u8 i;
	for (i=0; i < vidaS2; i+=1) {
		hudBuffer[131+i] = 0x20f6;
	}
	if (vida & 1 != 0) {
		hudBuffer[131+vidaS2] =0x2cf5 ;
	} else {
		if (vidaS2 != totalS2)
			hudBuffer[131+vidaS2] = 0x28f4;
	}
	for (i=vidaS2+1; i < totalS2; i+=1) {
		hudBuffer[131+i] = 0x28f4;
	}
	hudBuffer[131+totalS2] =  0x20f6 + 1;
}

void handleHudPersonagem () {
	if (hudPlayerState == 0) {
		player1->vida = 0;
		player1->invencivel = true;
		hudFrames = 0;
		hudPlayerState = 1;
		return;
	}
	if (hudPlayerState == 1) {
		hudFrames += 1;
		if (hudFrames == 2) {
			hudFrames = 0;
			player1->vida += 1;
			atualizaBarraVida (player1->vida);
		}
		if (player1->vida == 18) {
			hudPlayerState = 2;
		}
	}
}

void handleHudInimigo () {
	if (inimigoApanhando != ultimoInimigoApanhando) {
		//limpa hud
		u8 i;
		for (i=0; i< 8; i+=1) {
			hudBuffer[100+i] = 0x2a00;
		}
		for (i=0; i< 9; i+=1) {
			hudBuffer[131+i] = 0x2a00;
		}
	}
	if (inimigoApanhando != NULL) {
		ultimoInimigoApanhando = inimigoApanhando; 
		if (hudOpponentChanged == true) {
			writeText (100,inimigoApanhando->nome);
			atualizaBarraVidaInimigo (inimigoApanhando->vida,inimigoApanhando->vidaTotal);
		}
	}
}

int timer1 = 9;
int timer2 = 9;
int frame = 0;

//---------------------------------------------------------------------------------
int main(void) {

	// using this to disable sound (when i cant take any longer lol)
	bool soundEnabled = true;
	bool enableBGsound = true;

	// Initialize sound engine (take some time)
	if (soundEnabled) {
		spcBoot();
	}

    // Initialize SNES 
	consoleInit();
	

	// Set give soundbank
	if (soundEnabled) {
		spcSetModuleVolume (128);
		spcSetBank(&__SOUNDBANK__1);
		spcSetBank(&__SOUNDBANK__0);

		// allocate around 10K of sound ram (39 256-byte blocks)
		spcAllocateSoundRegion(39);

		// Load music
		spcLoad(MOD_MUSIC);

		spcSetSoundEntry(15, 1, 6, &socobrrend-&socobrr, &socobrr, &socosound);
		spcSetSoundEntry(15, 1, 6, &danobrrend-&danobrr, &danobrr, &danosound);
	
	}

	
	u8 i;
	for  (i=0; i< MAXPERSONAGENS; i+=1) {
		personagens[i].ativo = false;
		yOrder[i] = NULL;
	}

	//iniciando oamtable no endereço 4000
	REG_OBSEL = OBJ_SIZE32 | (0x4000 >> 13);

	inimigoApanhando = NULL;
	ultimoInimigoApanhando = NULL;

	//inicnado a blaze
	initPersonagem (&personagens[0],0x1000,16*2);
	initBlaze (&personagens[0]);	
	initGFX (&personagens[0], 0, 0x4000,0,0);

	player1 = &personagens[0];
	
	//iniciando inimigo 1
	initPersonagem (&personagens[1],0x1000,16*2);
	initGalsia (&personagens[1]);
	initGFX (&personagens[1], 4, 0x4000+2048,128,2);
	
	personagens[1]._x = 200;
	personagens[1]._y = 164;
	personagens[1].hflip = 1;


	//iniciando inimigo 2
	initPersonagem (&personagens[2],0x1000,16*2);
	initSignal (&personagens[2]);
	initGFX (&personagens[2], 8, 0x4000+4096,256,3);
	

	personagens[2]._x = 150;
	personagens[2]._y = 189;
	personagens[2].hflip = 1;


	oamSetVisible(0,OBJ_SHOW);
	oamSetVisible(4,OBJ_SHOW);
	oamSetVisible(8,OBJ_SHOW);
	
	//bgInitTileSet(0, &patterns1, &palette, 0, (&patterns1_end-&patterns1), 16*2, BG_16COLORS, 0x5000);
	WaitForVBlank();
	bgSetMapPtr(0, 0x0000, SC_64x32);
	bgSetMapPtr(1, 0x0000+2048, SC_64x32);
	bgSetMapPtr(2, 0x1000, SC_32x32);
	bgInitTileSet(0, &patterns1, &palette, 2, (&patterns1_end - &patterns1), 16*4, BG_16COLORS, 0x2000);
	bgInitTileSet(1, &patterns2, &palette2, 4, (&patterns2_end - &patterns2), 16*4, BG_16COLORS, 0x3000);
	bgInitTileSet(2, &hud, &hud_pal, 0, 1536, 16*4, BG_4COLORS, 0x1000 + 0x500);
	
	dmaCopyOAram((unsigned char *) &oamMemory,0,0x220);	
	int j=0;
	//WaitForVBlank();
	memcpy (paletteBuffer, &palette, 16*4);

	

	//copiando hud para buffer
	
	memcpy ((u8*)hudBuffer, &hud_map,HUDSIZE*2);
	//fazendo um shift para a metade do endereço de memoria (reuso de map e sprite no mesmo banco)
	for (j =0; j< HUDSIZE*2; j+=1) {
		hudBuffer[j]+=(10)*0x10;

	}
	//zerando o resto do mapa
	for ( i=0; i< 32; i+=1) {
		for (j=0; j<32; j+=1) {
			hudBuffer[i*32+j]=0x2a00;
		}
		WaitForVBlank();
		dmaCopyVram((u8*)hudBuffer, 0x1000+i*32, 64);
	}
	

	dmaCopyVram((u8*)hudBuffer, 0x1000, 320);
	initTimer ();
	writeText (36,blazeName);

	initHudPersonagemJogavel ();

	writeNum (45,4);
	writeNum (79,9);
	writeNum (80,9);
	
	u8 i;
	
	setMode(BG_MODE1,BG3_MODE1_PRORITY_HIGH); 
	
	
	bgSetDisable(3); 

	setScreenOn();
	
	// Play file from the beginning
	if (enableBGsound)
		spcPlay(0);

	
	bgMain.id = 0;
	bgMain.scrX = 255;
	bgMain.scrY = 255;
	bgMain.vram = 0x0000;
	bgMain.vRamFirstPage = true;
	bgMain.mapPage = 0;
	bgMain.bgMap = &map;
	bgMain.maxScrollX = 768;

	bgSub.id = 1;
	bgSub.scrX = 255;
	bgSub.scrY = 255;
	bgSub.vram = 0x0000 + 2048;
	bgSub.vRamFirstPage = true;
	bgSub.mapPage = 0;
	bgSub.bgMap = &map2;
	bgSub.maxScrollX = 1024+128;

	bgSetScroll(0,0,32);
	bgSetScroll(1,0,32);


	dmaCopyOAram((unsigned char *) &oamMemory,0,0x220);


	spr_queue = 0xff; spr_mutex = 0; bg_mutex = 0;
	updateBG1 (&map,0x0000,2048);
	updateBG2 (&map2,0x0000+2048,2048);
	__nmi_handler=myconsoleVblank;
	u8 i;
	// Wait for nothing :P
	while(1) {
		
		scanPads();
		pad0 = padsCurrent(0);
		
		frame +=1;
		if (frame == 60) {
			frame = 0;
			timer2 -=1;
			if (timer2 < 0) {
				if (timer1 > 0) {
					timer1 -= 1;
					timer2 = 9;
				} else {
					timer2 = 0;
				}
			}
			writeNum (79, timer1);
			writeNum (80, timer2);			

		}
		animBackgroundControl ();
		handleScroll (player1,&bgMain);
		handleScrollSub (player1,&bgSub);
		updatePersonagemJogavel(player1, pad0);
		
		for (i = 1; i < MAXPERSONAGENS; i+=1) {
			if (personagens[i].ativo == true)
				updatePersonagemNPC(&personagens[i]);
		}
		if (disableLogic == false ){
			
			//handleCollisions ();
			handleYorder ();
			handleHitBox ();
			handleHudPersonagem ();
			handleHudInimigo ();
			if (soundEnabled)
				spcProcess();
		}
		WaitForVBlank();
	}
	return 0;
}
