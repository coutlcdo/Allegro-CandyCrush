#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>

#define INFO_H 64
#define MARGIN 5

#define FPS 60

#define N_LINHAS 9
#define N_COLS 8
#define SPRITE_W 103
#define SPRITE_H 104

#define SIZE_END 4
#define SIZE_ENDD 3
#define SIZE_GTP 7

#define PREMAX 100

typedef struct Candy {
	int type;
	int active;
	int highlight;
	int special;
} Candy;

typedef struct SoundsVec {
	ALLEGRO_SAMPLE *sound;
} SoundsVec;

Candy M[N_LINHAS][N_COLS];
Candy BACKGROUND[N_LINHAS][N_COLS];

SoundsVec gotpoints[SIZE_GTP];
SoundsVec end[SIZE_END];
SoundsVec gotpoints2[SIZE_GTP];
SoundsVec end2[SIZE_ENDD];

const int SCREEN_W = N_COLS*SPRITE_W;
const int SCREEN_H = N_LINHAS*SPRITE_H + INFO_H;

char srcname[PREMAX];
int srcrecord;
int tfrecord;

char buffer[PREMAX];
char *wname = NULL;
int wrecord;

int newRecord(char *name, int score, int *record) {
	FILE *arq = fopen("../records.txt", "r");
	FILE *temp = fopen("../backup.tmp", "w");
	int flag_has = 0, retorno = 0, flag_hastemp = 0, temprecord;

	if(arq == NULL) {
		printf("\nErro ao abrir o arquivo de recordes!");
		return 0;
	}
	if(temp == NULL) {
		printf("\nErro ao criar o arquivo de backup!");
		return 0;
	}

	fgets(buffer, PREMAX, arq);
	while(!feof(arq)) {
		wname = strtok(buffer, "=");
		wrecord = atoi(strtok(NULL, "."));

		if(strcmp(wname, name) == 0) {
			flag_has = 1;
			flag_hastemp = 1;
			retorno = 0;

			temprecord = wrecord;
			if(score > wrecord) {
				retorno = 1;
				temprecord = score;
			}
		}
		else
			fprintf(temp, "%s=%d.\n", wname, wrecord);

		fgets(buffer, PREMAX, arq);
	}

	if(!flag_has) {
		fclose(temp);
		temp = fopen("../backup.tmp", "a");
		fprintf(temp, "%s=%d.\n", name, score);
		*record = score;
		retorno = 1;
	}

	if(flag_hastemp) {
		fclose(temp);
		temp = fopen("../backup.tmp", "a");
		fprintf(temp, "%s=%d.\n", name, temprecord);
		*record = temprecord;
	}

	fclose(arq);
	fclose(temp);
	remove("../records.txt");
	rename("../backup.tmp", "../records.txt");

	printf("\nDB: %s=%d.", wname, wrecord);
	return retorno;
}

float returnCellW() {
	return (float)SCREEN_W/N_COLS;
}

float returnCellH() {
	return (float)(SCREEN_H - INFO_H)/N_LINHAS;
}

// const float CELL_W = (float)SCREEN_W/N_COLS;
// const float CELL_H = (float)(SCREEN_H - INFO_H)/N_LINHAS;

int final_score = 0, plays = 15, min = 5, sec = 0, mlsec = 0, mode = 0, endmode = 1, has_sound = 1, has_special = 1, NUM_TYPES = 5, inf_mode = 0, mov_mode = 0;
char my_score[PREMAX], my_record[PREMAX], my_plays[PREMAX], pressesc[PREMAX], my_time[PREMAX], chartostr[PREMAX];
char bopt1[PREMAX], bopt2[PREMAX], bopt3[PREMAX], bopt4[PREMAX], bopt5[PREMAX], bopt6[PREMAX], bopt7[PREMAX], bopt8[PREMAX];

ALLEGRO_FONT *size_P;
ALLEGRO_FONT *size_P_classic;
ALLEGRO_FONT *size_M;
ALLEGRO_FONT *size_M_classic;
ALLEGRO_FONT *size_G;

ALLEGRO_BITMAP *tile_1;
ALLEGRO_BITMAP *tile_2;
ALLEGRO_BITMAP *sprite;
ALLEGRO_BITMAP *sprite_2;
ALLEGRO_BITMAP *tnt;
ALLEGRO_BITMAP *walking;
ALLEGRO_BITMAP *waving;
ALLEGRO_BITMAP *gamover;
ALLEGRO_BITMAP *fundo_1;
ALLEGRO_BITMAP *fundo_2;
ALLEGRO_BITMAP *fundo_menu;
ALLEGRO_BITMAP *fundo_frame;
ALLEGRO_BITMAP *logo;

ALLEGRO_SAMPLE *swipe;
ALLEGRO_SAMPLE *select_snd;
ALLEGRO_SAMPLE *error_snd;
ALLEGRO_SAMPLE *move_snd;
ALLEGRO_SAMPLE *move_snd_2;
ALLEGRO_SAMPLE *song;
ALLEGRO_SAMPLE *soundtrack_1;
ALLEGRO_SAMPLE *soundtrack_2;
ALLEGRO_SAMPLE *sadsong;
ALLEGRO_SAMPLE *lowsong;
ALLEGRO_SAMPLE_INSTANCE *song_instance;
ALLEGRO_SAMPLE_INSTANCE *sndtrck_stage_1;
ALLEGRO_SAMPLE_INSTANCE *sndtrck_stage_2;
ALLEGRO_SAMPLE_INSTANCE *sadsong_instance;
ALLEGRO_SAMPLE_INSTANCE *lowsong_instance;

void printCandyMType() {
	int i, j;
	for(i=0; i<N_LINHAS; i++) {
		for(j=0; j<N_COLS; j++) {
			printf("%d ", M[i][j].type);
		}
		printf("\n");
	}

	printf("\n");
}

void printCandyMActive() {
	int i, j;
	for(i=0; i<N_LINHAS; i++) {
		for(j=0; j<N_COLS; j++) {
			printf("%d ", M[i][j].active);
		}
		printf("\n");
	}

	printf("\n");
}

int generateRandomCandy() {
	return (rand()%NUM_TYPES + 1);
}

int generateRandomSound(int size) {
	return (rand()%size);
}

void playSound(SoundsVec *vec, int index) {
	al_play_sample(vec[index].sound, 0.6, 0, 1, ALLEGRO_PLAYMODE_ONCE, 0);
}

void play_sound_effect(int stage, int type) {
	if(stage == 1 && type == 1) {
		int r = generateRandomSound(SIZE_GTP);
		// printf("\n%d\n", r);

		playSound(gotpoints, r);
	}
	else if(stage == 2 && type == 1) {
		int r = generateRandomSound(SIZE_GTP);
		printf("\n%d\n", r);

		playSound(gotpoints2, r);
	}
	else if(stage == 1 && type == 2) {
		int r = generateRandomSound(SIZE_END);
		// printf("\n%d\n", r);

		playSound(end, r);
	}
	else if(stage == 2 && type == 2) {
		int r = generateRandomSound(SIZE_ENDD);
		// printf("\n%d\n", r);

		playSound(end2, r);
	}
}

void initGame() {
	int i, j;
	for(i=0; i<N_LINHAS; i++) {
		for(j=0; j<N_COLS; j++) {
			BACKGROUND[i][j].type = 0;
			BACKGROUND[i][j].active = 0;
			BACKGROUND[i][j].highlight = 0;

			M[i][j].type = generateRandomCandy();
			M[i][j].active = 0;
			M[i][j].highlight = 0;
		}
	}
}

int findSequences() {
	int i, j, x, count, aux_count, total_pontos = 0;
	int m = N_LINHAS;
	int n = N_COLS;

	// Verifica sequencias nas linhas
	for(i=0;i<m;i++) {
		count = 0;
		x = NUM_TYPES;
		for(j=0;j<n;j++) {
			if(x == 0)
				break;
			else if(M[i][j].type == x) {
				count++;
				if((M[i][j+1].type != x || j == n-1) && count >= 3) {
					aux_count = count;
					if(aux_count > total_pontos)
						total_pontos = aux_count;
					else
						total_pontos = aux_count;

					int aux;
					for(aux=1;aux<=j;aux++) {
						if(M[i][j-aux].type == M[i][j].type)
							M[i][j-aux].active = 1;
						else
							break;
					}
					M[i][j].active = 1;
					count = 0;
				}

				if(j == n-1) {
					count = 0;
					x--;
					j = -1;
				}
			}
			else if(j == n-1) {
				count = 0;
				x--;
				j = -1;
			}
			else
				count = 0;
		}
	}

	aux_count = 0;

	// Verifica sequencias nas colunas
	for(j=0;j<n;j++) {
		count = 0;
		x = NUM_TYPES;
		for(i=0;i<m;i++) {
			if(x == 0)
				break;
			else if(M[i][j].type == x) {
				count++;
				if((M[i+1][j].type != x || i == m-1) && count >= 3) {
					aux_count = count;
					if(aux_count > count)
						total_pontos += aux_count;
					else
						total_pontos = aux_count;

					int aux;
					for(aux=1;aux<=i;aux++) {
						if(M[i-aux][j].type == M[i][j].type)
							M[i-aux][j].active = 1;
						else
							break;
					}
					M[i][j].active = 1;
					count = 0;
				}

				if(i == m-1) {
					count = 0;
					x--;
					i = -1;
				}
			}
			else if(i == m-1) {
				count = 0;
				x--;
				i = -1;
			}
			else
				count = 0;
		}
	}

	for(i=0;i<m;i++) {
		for(j=0;j<n;j++) {
			if(M[i][j].active)
				BACKGROUND[i][j].highlight = 1;
		}
	}

	return total_pontos;
}

int countZeros() {
	int i, j, n_zeros = 0;
	int m = N_LINHAS;
	int n = N_COLS;

	for(i=0;i<m;i++) {
		for(j=0;j<n;j++) {
			if(!M[i][j].type)
				n_zeros++;
		}
	}

	return n_zeros;
}

void clearSequences() {
	int i, j;
	for(i=0;i<N_LINHAS;i++) {
		for(j=0;j<N_COLS;j++) {
			if(M[i][j].active) {
				M[i][j].active = 0;
				M[i][j].type = 0;

				BACKGROUND[i][j].highlight = 0;
			}
		}
	}

	// if(sound)
	// 	play_sound_effect();
}

void swapZeros() {
	int i, j, t, t2, c;
	int m = N_LINHAS;
	int n = N_COLS;

	for(j=0;j<n;j++) {
		for(i=(m-1);(i<m && i!=0);i--) {
			if(!M[i][j].type) {
				c = M[i][j].type;
				for(t=0;t<i;t++) {
					if(M[t][j].type) {
						M[i][j].type = M[t][j].type;
						M[t][j].type = c;

						break;
					}
				}
				// printf("\nM[%d][%d] = %d\n", i, j, M[i][j]);
			}
		}
	}
}

void changeZerosForCandies() {
	int i, j;
	for(i=0;i<N_LINHAS;i++) {
		for(j=0;j<N_COLS;j++) {
			if(!M[i][j].type)
				M[i][j].type = generateRandomCandy();
		}
	}
}

void draw_background(int lin, int col, int stage) {
	float CELL_W = returnCellW();
	float CELL_H = returnCellH();

	int cell_x = CELL_W*col;
	int cell_y = INFO_H + CELL_H*lin;

	ALLEGRO_BITMAP *tileByStg;
	switch(stage) {
		case 2:
			tileByStg = tile_2;
			break;
		default:
			tileByStg = tile_1;
	}

	// Desenha os ladrilhos
	if(!BACKGROUND[lin][col].highlight) {
		al_draw_tinted_bitmap(tileByStg,
						al_map_rgba_f(0.3, 0.3, 0.3, 0.4),
						cell_x + MARGIN, cell_y + MARGIN,
						0);
	}
	else if(BACKGROUND[lin][col].highlight) {
		al_draw_tinted_bitmap(tileByStg,
						al_map_rgba_f(0.9, 1, 0.65, 0.9),
						cell_x + MARGIN, cell_y + MARGIN,
						0);
	}
}

void draw_candy(int lin, int col, int stage) {
	float CELL_W = returnCellW();
	float CELL_H = returnCellH();

	int cell_x = CELL_W*col;
	int cell_y = INFO_H + CELL_H*lin;

	int type = M[lin][col].type - 1;

	ALLEGRO_BITMAP *spriteByStg;
	switch(stage) {
		case 2:
			spriteByStg = sprite_2;
			break;
		default:
			spriteByStg = sprite;
	}

	al_draw_bitmap_region(spriteByStg,
					(SPRITE_W*type), 0,
					SPRITE_W, SPRITE_H,
					cell_x, cell_y,
					0);
}

// Fixa e percorre a altura H para um doce especifico
void draw_spec_candyFixedH(int lin, int col, int type, int frame, int stage) {
	float CELL_W = returnCellW();
	float CELL_H = returnCellH();

	int cell_x = CELL_W*col;
	int cell_y = INFO_H + CELL_H*lin;

	int type_minus = type - 1;

	ALLEGRO_BITMAP *spriteByStg;
	switch(stage) {
		case 2:
			spriteByStg = sprite_2;
			break;
		default:
			spriteByStg = sprite;
	}

	al_draw_bitmap_region(spriteByStg,
						(SPRITE_W*type_minus), 0,
						SPRITE_W, SPRITE_H,
						cell_x, (cell_y + frame),
						0);
}

// Fixa e percorre a largura W para um doce especifico
void draw_spec_candyFixedW(int lin, int col, int type, int frame, int stage) {
	float CELL_W = returnCellW();
	float CELL_H = returnCellH();

	int cell_x = CELL_W*col;
	int cell_y = INFO_H + CELL_H*lin;

	int type_minus = type - 1;

	ALLEGRO_BITMAP *spriteByStg;
	switch(stage) {
		case 2:
			spriteByStg = sprite_2;
			break;
		default:
			spriteByStg = sprite;
	}

	al_draw_bitmap_region(spriteByStg,
						(SPRITE_W*type_minus), 0,
						SPRITE_W, SPRITE_H,
						(cell_x + frame), cell_y,
						0);
}

void getCell(int x, int y, int *lin, int *col) {
	float CELL_W = returnCellW();
	float CELL_H = returnCellH();

	*lin = (y - INFO_H)/CELL_H;
	*col = x/CELL_W;
}

int dist(int lin1, int col1, int lin2, int col2) {
	// Distancia entre dois pontos
	float hip = sqrt(pow(lin1-lin2, 2) + pow(col1-col2, 2));
	if(hip != 1)
		return 0;

	int catet = lin1-lin2;
	if(catet < 0)
		catet *= -1;

	float senTeta = catet/hip;

	// Verifica se a celula esta ou nao na diagonal
	if(senTeta == 0 || senTeta == 1 || senTeta == -1)
		return 1;
	return 0;
}

double to_degrees(double radians) {
    return radians * (180.0 / M_PI);
}

int direction(int lin1, int col1, int lin2, int col2, float *sen, float *cos) {
	float hip = sqrt(pow(lin1-lin2, 2) + pow(col1-col2, 2));
	if(hip != 1)
		return 0;

	int catet1 = lin1-lin2;
	int catet2 = col1-col2;

	float senTeta = catet1/hip;
	float cosTeta = catet2/hip;
	// printf("\nsen %f cos %f\n", senTeta, cosTeta);

	*sen = senTeta;
	*cos = cosTeta;
}

int getCandyType(int lin, int col) {
	int i, j;
	for(i=0;i<N_LINHAS;i++) {
		for(j=0;j<N_COLS;j++) {
			if(M[i][j].type == M[lin][col].type)
				return M[i][j].type;
		}
	}
}

void swapCells(int lin1, int col1, int lin2, int col2) {
	Candy aux = M[lin1][col1];
	M[lin1][col1] = M[lin2][col2];
	M[lin2][col2] = aux;
	if(!inf_mode)
		plays--;

	al_play_sample(swipe, 1.5, 0, 1, ALLEGRO_PLAYMODE_ONCE, 0);
}

void draw_menu(ALLEGRO_DISPLAY *display) {
	ALLEGRO_COLOR BKG_COLOR = al_map_rgb(0,0,0);
	al_set_target_bitmap(al_get_backbuffer(display));
	al_clear_to_color(BKG_COLOR);

	// Desenha o fundo
	al_draw_bitmap(fundo_menu, 0, 0, 0);
}

void draw_scenario(ALLEGRO_DISPLAY *display, int stage) {
	ALLEGRO_COLOR BKG_COLOR = al_map_rgb(0,0,0);
	al_set_target_bitmap(al_get_backbuffer(display));
	al_clear_to_color(BKG_COLOR);

	ALLEGRO_BITMAP *fundo;
	int stg;
	switch(stage) {
		case 2:
			fundo = fundo_2;
			stg = 2;
			break;
		default:
			fundo = fundo_1;
			stg = 1;
	}

	// Desenha o fundo
	al_draw_bitmap(fundo, 0, 0, 0);

	// Desenha os pontos feitos
	sprintf(my_score, "Score: %d", final_score);
	al_draw_text(size_P, al_map_rgb(255, 255, 255), SCREEN_W/4 + 350, INFO_H/4, 0, my_score);

	if(endmode) {
		// Desenha tempo restante se modo é 1
		if(min < 10) {
			sprintf(my_time, "Time left: 0%d:%d", min, sec);
			if(sec < 10) {
				sprintf(my_time, "Time left: 0%d:0%d", min, sec);
			}
		}
		else if(sec < 10)
			sprintf(my_time, "Time left: %d:0%d", min, sec);
		else
			sprintf(my_time, "Time left: %d:%d", min, sec);
		al_draw_text(size_P, al_map_rgb(255, 255, 255), SCREEN_W/2 - 330, INFO_H/4, 0, my_time);
	}
	else {
		// Desenha jogadas restantes
		sprintf(my_plays, "Plays left: %d", plays);
		al_draw_text(size_P, al_map_rgb(255, 255, 255), SCREEN_W/2 - 330, INFO_H/4, 0, my_plays);
	}

	int i, j;
	for(i=0; i<N_LINHAS; i++) {
		for(j=0; j<N_COLS; j++) {
			draw_background(i, j, stg);
			draw_candy(i, j, stg);
		}
	}
}

// Desenha o cenario sem os doces especificos
void draw_scenario_removeSpecCandy(ALLEGRO_DISPLAY *display, int lin1, int col1, int lin2, int col2, int stage) {
	ALLEGRO_COLOR BKG_COLOR = al_map_rgb(0,0,0);
	al_set_target_bitmap(al_get_backbuffer(display));
	al_clear_to_color(BKG_COLOR);

	ALLEGRO_BITMAP *fundo;
	int stg;
	switch(stage) {
		case 2:
			fundo = fundo_2;
			stg = 2;
			break;
		default:
			fundo = fundo_1;
			stg = 1;
	}

	// Desenha o fundo
	al_draw_bitmap(fundo, 0, 0, 0);

	// Desenha os pontos feitos
	sprintf(my_score, "Score: %d", final_score);
	al_draw_text(size_P, al_map_rgb(255, 255, 255), SCREEN_W/4 + 350, INFO_H/4, 0, my_score);

	if(endmode) {
		// Desenha tempo restante se modo é 1
		if(min < 10) {
			sprintf(my_time, "Time left: 0%d:%d", min, sec);
			if(sec < 10) {
				sprintf(my_time, "Time left: 0%d:0%d", min, sec);
			}
		}
		else if(sec < 10)
			sprintf(my_time, "Time left: %d:0%d", min, sec);
		else
			sprintf(my_time, "Time left: %d:%d", min, sec);
		al_draw_text(size_P, al_map_rgb(255, 255, 255), SCREEN_W/2 - 330, INFO_H/4, 0, my_time);
	}
	else {
		// Desenha jogadas restantes
		sprintf(my_plays, "Plays left: %d", plays);
		al_draw_text(size_P, al_map_rgb(255, 255, 255), SCREEN_W/2 - 330, INFO_H/4, 0, my_plays);
	}

	int i, j;
	for(i=0; i<N_LINHAS; i++) {
		for(j=0; j<N_COLS; j++) {
			draw_background(i, j, stg);
			if(i == lin1 && j == col1)
				continue;
			else if(i == lin2 && j == col2)
				continue;
			else
				draw_candy(i, j, stg);
		}
	}
}

void pausa(ALLEGRO_TIMER *timer) {
	al_stop_timer(timer);
	al_rest(1);
	al_start_timer(timer);
}

// void slowStopInstance(ALLEGRO_SAMPLE_INSTANCE *inst) {
// 	int i = 1;
// 	float takeoff = 0.0;
// 	while((i-takeoff)>0) {
// 		takeoff += 0.1;
// 		al_set_sample_instance_gain(inst, (i-takeoff));
// 		al_rest(0.1);
// 	}

// 	al_stop_sample_instance(inst);
// }

int main(int argc, char **argv){
	// srand(time(NULL));

	ALLEGRO_DISPLAY *display = NULL;
	ALLEGRO_EVENT_QUEUE *event_queue = NULL;
	ALLEGRO_TIMER *timer = NULL;


	//------------------------Inicio das rotinas de inicializacao----------------------------
	if(!al_init()) {
		fprintf(stderr, "Failed to initialize allegro!\n");
		return -1;
	}

	if(!al_init_primitives_addon()){
		fprintf(stderr, "Failed to initialize primitives!\n");
		return -1;
	}

	timer = al_create_timer(1.0 / FPS);
	if(!timer) {
		fprintf(stderr, "Failed to create timer!\n");
		return -1;
	}

	display = al_create_display(SCREEN_W, SCREEN_H);
	if(!display) {
		fprintf(stderr, "Failed to create display!\n");
		al_destroy_timer(timer);
		return -1;
	}

	event_queue = al_create_event_queue();
	if(!event_queue) {
		fprintf(stderr, "Failed to create event_queue!\n");
		al_destroy_display(display);
		al_destroy_timer(timer);
		return -1;
	}

	al_install_keyboard();
	al_install_mouse();

	// Inicializa os modulos para fontes
	al_init_font_addon();
	al_init_ttf_addon();

	// Carrega a fonte para uma variavel ALLEGRO_FONT
	size_P = al_load_font("../assets/crash-a-like.ttf", 42, 1);
	size_M = al_load_font("../assets/crash-a-like.ttf", 55, 1);
	size_G = al_load_font("../assets/crash-a-like.ttf", 95, 1);
	size_P_classic = al_load_font("../assets/crash-wumpa.ttf", 30, 1);
	size_M_classic = al_load_font("../assets/crash-wumpa.ttf", 42, 1);

	// Inicializa o modulo de imagens
	al_init_image_addon();

	// Carrega a imagem fonte do sprite
	sprite = al_load_bitmap("../assets/crash-bandicoot-2.bmp");
	sprite_2 = al_load_bitmap("../assets/coco-bandicoot.bmp");
	tile_1 = al_load_bitmap("../assets/tile_1.png");
	tile_2 = al_load_bitmap("../assets/tile_2.png");
	tnt = al_load_bitmap("../assets/tnt.png");
	gamover = al_load_bitmap("../assets/gameoversprite.bmp");
	fundo_1 = al_load_bitmap("../assets/background_1.png");
	fundo_2 = al_load_bitmap("../assets/background_2.png");
	fundo_menu = al_load_bitmap("../assets/background_menu.png");
	fundo_frame = al_load_bitmap("../assets/background_frame.png");
	walking = al_load_bitmap("../assets/crash_walking.bmp");
	waving = al_load_bitmap("../assets/crash_waving.bmp");
	logo = al_load_bitmap("../assets/candycrash-logo.png");

	// Converte magenta em transparente
    al_convert_mask_to_alpha(sprite, al_map_rgb(255,0,255));
    al_convert_mask_to_alpha(sprite_2, al_map_rgb(255,0,255));

	// Rotinas de audio
	al_install_audio();
	al_init_acodec_addon();

	al_reserve_samples(10);

	gotpoints[0].sound = al_load_sample("../assets/sounds/sound_1.wav");
	gotpoints[1].sound = al_load_sample("../assets/sounds/sound_2.wav");
	gotpoints[2].sound = al_load_sample("../assets/sounds/sound_3.wav");
	gotpoints[3].sound = al_load_sample("../assets/sounds/sound_4.wav");
	gotpoints[4].sound = al_load_sample("../assets/sounds/sound_5.wav");
	gotpoints[5].sound = al_load_sample("../assets/sounds/sound_6.wav");
	gotpoints[6].sound = al_load_sample("../assets/sounds/sound_7.wav");

	end[0].sound = al_load_sample("../assets/sounds/sound_end_1.wav");
	end[1].sound = al_load_sample("../assets/sounds/sound_end_2.wav");
	end[2].sound = al_load_sample("../assets/sounds/sound_end_3.wav");
	end[3].sound = al_load_sample("../assets/sounds/sound_end_4.wav");

	gotpoints2[0].sound = al_load_sample("../assets/sounds/sound_cc_1.wav");
	gotpoints2[1].sound = al_load_sample("../assets/sounds/sound_cc_2.wav");
	gotpoints2[2].sound = al_load_sample("../assets/sounds/sound_cc_3.wav");
	gotpoints2[3].sound = al_load_sample("../assets/sounds/sound_cc_4.wav");
	gotpoints2[4].sound = al_load_sample("../assets/sounds/sound_cc_5.wav");
	gotpoints2[5].sound = al_load_sample("../assets/sounds/sound_cc_6.wav");
	gotpoints2[6].sound = al_load_sample("../assets/sounds/sound_cc_7.wav");

	end2[0].sound = al_load_sample("../assets/sounds/sound_cc_end_1.wav");
	end2[1].sound = al_load_sample("../assets/sounds/sound_cc_end_2.wav");
	end2[2].sound = al_load_sample("../assets/sounds/sound_cc_end_3.wav");

	swipe = al_load_sample("../assets/sounds/jump.wav");
	select_snd = al_load_sample("../assets/sounds/select.wav");
	error_snd = al_load_sample("../assets/sounds/error.wav");
	move_snd = al_load_sample("../assets/sounds/move.wav");
	move_snd_2 = al_load_sample("../assets/sounds/move_2.wav");
	song = al_load_sample("../assets/sounds/theme.wav");
	soundtrack_1 = al_load_sample("../assets/sounds/soundtrack_1.wav");
	soundtrack_2 = al_load_sample("../assets/sounds/soundtrack_2.wav");
	sadsong = al_load_sample("../assets/sounds/crashsadsong.wav");
	lowsong = al_load_sample("../assets/sounds/lowsong_background.wav");

	song_instance = al_create_sample_instance(song);
	al_set_sample_instance_playmode(song_instance, ALLEGRO_PLAYMODE_LOOP);
	al_attach_sample_instance_to_mixer(song_instance, al_get_default_mixer());

	sndtrck_stage_1 = al_create_sample_instance(soundtrack_1);
	al_set_sample_instance_playmode(sndtrck_stage_1, ALLEGRO_PLAYMODE_LOOP);
	al_attach_sample_instance_to_mixer(sndtrck_stage_1, al_get_default_mixer());

	sndtrck_stage_2 = al_create_sample_instance(soundtrack_2);
	al_set_sample_instance_playmode(sndtrck_stage_2, ALLEGRO_PLAYMODE_LOOP);
	al_attach_sample_instance_to_mixer(sndtrck_stage_2, al_get_default_mixer());

	sadsong_instance = al_create_sample_instance(sadsong);
	al_set_sample_instance_playmode(sadsong_instance, ALLEGRO_PLAYMODE_LOOP);
	al_attach_sample_instance_to_mixer(sadsong_instance, al_get_default_mixer());

	lowsong_instance = al_create_sample_instance(lowsong);
	al_set_sample_instance_playmode(lowsong_instance, ALLEGRO_PLAYMODE_LOOP);
	al_attach_sample_instance_to_mixer(lowsong_instance, al_get_default_mixer());

   // Registros na fila de eventos...
   // ...quando a tela é alterada
	al_register_event_source(event_queue, al_get_display_event_source(display));
   // ...o tempo de jogo (timer) é alterado
	al_register_event_source(event_queue, al_get_timer_event_source(timer));
	// ...os eventos de teclado
	al_register_event_source(event_queue, al_get_keyboard_event_source());
	// ...os eventos de mouse
	al_register_event_source(event_queue, al_get_mouse_event_source());

   // Inicia o temporizador
	al_start_timer(timer);

	//--------------------------Fim das rotinas de inicializacao-----------------------------------

	// Variaveis para a animacao do game over
	int spriteH = SCREEN_H, spriteW = SCREEN_W;
	int maxLin = 3, currLin = 0;
	int src_x = 0, src_y = 0;
	int frame = 0, frameDelay = 15;
	int dest_x = 0, dest_y = 0;

	// Variaveis para animacao dos sprites do menu
	int spriteH1 = 44, spriteW1 = 41;
	int spriteH2 = 49, spriteW2 = 48;
	int maxCol1 = 19, currCol1 = 0;
	int maxCol2 = 9, currCol2 = 0;
	int src1_x = 0, src1_y = 0;
	int src2_x = 0, src2_y = 0;
	int dest1_x = SCREEN_W/2 - 150, dest1_y = SCREEN_H/2 + 30;
	int dest2_x = SCREEN_W/2 - 150, dest2_y = SCREEN_H/2 + 30;
	int frame1 = 0, frameDelay1 = 3;
	int frame2 = 0, frameDelay2 = 8;
	int walkSprite = 1, waveSprite = 0, spriteCod = 0, spriteDelay = 0;
	ALLEGRO_COLOR baseColor1 = al_map_rgb(0,0,0);
	ALLEGRO_COLOR baseColor2 = al_map_rgb(0,0,0);
	ALLEGRO_COLOR baseColor3 = al_map_rgb(0,0,0);
	ALLEGRO_COLOR baseColor4 = al_map_rgb(0,0,0);
	void resetMenu() {
		spriteH1 = 44; spriteW1 = 41; spriteH2 = 49; spriteW2 = 48; maxCol1 = 19; currCol1 = 0; maxCol2 = 9; currCol2 = 0;
		src1_x = 0; src1_y = 0; src2_x = 0; src2_y = 0; dest1_x = SCREEN_W/2 - 150; dest1_y = SCREEN_H/2 + 30;
		dest2_x = SCREEN_W/2 - 150; dest2_y = SCREEN_H/2 + 30; frame1 = 0; frameDelay1 = 3; frame2 = 0; frameDelay2 = 8;
		walkSprite = 1; waveSprite = 0; spriteCod = 0; spriteDelay = 0; baseColor1 = al_map_rgb(0,0,0); baseColor2 = al_map_rgb(0,0,0);
		baseColor3 = al_map_rgb(0,0,0); baseColor4 = al_map_rgb(0,0,0);
	}

	// Variaveis para o menu 2
	int firstPosX = SCREEN_W/2 + SCREEN_W/4, firstPosY = 160;
	int bfirstPosY = 160;
	char opt1[PREMAX][PREMAX], opt2[PREMAX][PREMAX], opt3[PREMAX][PREMAX], opt4[PREMAX][PREMAX], opt5[PREMAX][PREMAX], opt6[PREMAX][PREMAX], opt7[PREMAX][PREMAX], opt8[PREMAX][PREMAX];
	// Opcoes maximas ja subtraidas de 1 por causa do index, menos a opcao 4
	int maxOpt1 = 1, maxOpt2 = 1, maxOpt3 = 5, maxOpt4 = 26, maxOpt5 = 5, maxOpt6 = 2, maxOpt7 = 2, maxOpt8 = 2;
	// Essa variavel e quem muda as opcoes, permitindo deixar as opcoes default
	int count_opt1 = 0, count_opt2 = 1, count_opt3 = 2, count_opt4 = 15, count_opt5 = 3, count_opt6 = 0, count_opt7 = 0, count_opt8 = 0;
	ALLEGRO_COLOR bbaseColor1 = al_map_rgb(255, 255, 255), bbaseColor2 = al_map_rgb(255, 255, 255);
	ALLEGRO_COLOR bbaseColor3 = al_map_rgb(255, 255, 255), bbaseColor4 = al_map_rgb(255, 255, 255);
	ALLEGRO_COLOR bbaseColor5 = al_map_rgb(255, 255, 255), bbaseColor6 = al_map_rgb(255, 255, 255);
	ALLEGRO_COLOR bbaseColor7 = al_map_rgb(255, 255, 255), bbaseColor8 = al_map_rgb(255, 255, 255), bbaseColor9 = al_map_rgb(255, 255, 255);
	strcpy(opt1[0], "Normal");
	strcpy(opt1[1], "No stages");
	strcpy(opt2[0], "By Plays");
	strcpy(opt2[1], "By Time");
	strcpy(opt3[0], "3");
	strcpy(opt3[1], "4");
	strcpy(opt3[2], "5");
	strcpy(opt3[3], "6");
	strcpy(opt3[4], "7");
	strcpy(opt3[5], "8");
	strcpy(opt5[0], "00:30");
	strcpy(opt5[1], "01:00");
	strcpy(opt5[2], "03:00");
	strcpy(opt5[3], "05:00");
	strcpy(opt5[4], "10:00");
	strcpy(opt5[5], "Infinity");
	strcpy(opt6[0], "On");
	strcpy(opt6[1], "Off");
	strcpy(opt7[0], "On");
	strcpy(opt7[1], "Off");
	strcpy(opt8[0], "Locked");
	strcpy(opt8[1], "Free");

	// Geral
	float sen, cos;
	int menu = 0, menu2 = 0, putRecord = 1, playing = 0, mustEnd = 0, swapping = 0, score, col_src, lin_src, col_dst, lin_dst, flag_animation = 0, flag_dontrepeat = 0, flag_callonce = 1;
	int stage1 = 1, stage2 = 0, stage3 = 0;

	// Recorde
	char letters[36];
	int i;
	for(i=0;i<26;i++) {
		letters[i] = i+65;
	}
	for(i=26;i<36;i++) {
		letters[i] = i+22;
	}
	// for(i=0;i<36;i++) {
	// 	printf("%c", letters[i]);
	// }

	int basePosX = SCREEN_W/4 + 30, basePosY = SCREEN_H/2 + 30;
	int indx1 = 0, indx2 = 0, indx3 = 0, indx4 = 0;

	while(putRecord) {
		al_set_sample_instance_gain(lowsong_instance, 0.3);
		al_play_sample_instance(lowsong_instance);

		ALLEGRO_EVENT ev;
	  	// Espera por um evento e o armazena na variavel de evento ev
		al_wait_for_event(event_queue, &ev);

		if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
			if(ev.keyboard.keycode == ALLEGRO_KEY_LEFT) {
				basePosX -= 100;
				if(basePosX < SCREEN_W/4 + 30)
					basePosX = SCREEN_W/4 + 30;

				al_play_sample(move_snd, 1.7, 0, 1, ALLEGRO_PLAYMODE_ONCE, 0);
			}
			else if(ev.keyboard.keycode == ALLEGRO_KEY_RIGHT) {
				basePosX += 100;
				if(basePosX > SCREEN_W/4 + 30 + 300)
					basePosX = SCREEN_W/4 + 30 + 300;

				al_play_sample(move_snd, 1.7, 0, 1, ALLEGRO_PLAYMODE_ONCE, 0);
			}
			else if(ev.keyboard.keycode == ALLEGRO_KEY_UP) {
				if(basePosX == SCREEN_W/4 + 30) {
					indx1++;
					if(indx1 > 35)
						indx1 = 0;
				}
				else if(basePosX == SCREEN_W/4 + 30 + 100) {
					indx2++;
					if(indx2 > 35)
						indx2 = 0;
				}
				else if(basePosX == SCREEN_W/4 + 30 + 200) {
					indx3++;
					if(indx3 > 35)
						indx3 = 0;
				}
				else if(basePosX == SCREEN_W/4 + 30 + 300) {
					indx4++;
					if(indx4 > 35)
						indx4 = 0;
				}

				al_play_sample(move_snd_2, 1.7, 0, 1, ALLEGRO_PLAYMODE_ONCE, 0);
			}
			else if(ev.keyboard.keycode == ALLEGRO_KEY_DOWN) {
				if(basePosX == SCREEN_W/4 + 30) {
					indx1--;
					if(indx1 < 0)
						indx1 = 35;
				}
				else if(basePosX == SCREEN_W/4 + 30 + 100) {
					indx2--;
					if(indx2 < 0)
						indx2 = 35;
				}
				else if(basePosX == SCREEN_W/4 + 30 + 200) {
					indx3--;
					if(indx3 < 0)
						indx3 = 35;
				}
				else if(basePosX == SCREEN_W/4 + 30 + 300) {
					indx4--;
					if(indx4 < 0)
						indx4 = 35;
				}

				al_play_sample(move_snd_2, 1.7, 0, 1, ALLEGRO_PLAYMODE_ONCE, 0);
			}
			else if(ev.keyboard.keycode == ALLEGRO_KEY_ENTER) {
				srcname[0] = letters[indx1];
				srcname[1] = letters[indx2];
				srcname[2] = letters[indx3];
				srcname[3] = letters[indx4];
				srcname[4] = '\0';

				menu = 1;
				putRecord = 0;
				playing = 0;

				al_stop_sample_instance(lowsong_instance);
				al_play_sample(select_snd, 1, 0, 1, ALLEGRO_PLAYMODE_ONCE, 0);
			}
			else if(ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
				menu = 0;
				putRecord = 0;
				playing = 0;
			}
		}

		else if(ev.type == ALLEGRO_EVENT_TIMER) {
			ALLEGRO_COLOR BKG_COLOR = al_map_rgb(0,0,0);
			al_set_target_bitmap(al_get_backbuffer(display));
			al_clear_to_color(BKG_COLOR);

			al_draw_text(size_M_classic, al_map_rgb(255, 255, 255), SCREEN_W/4 - 60, 120, 0, "Write your name, use the arrows");

			al_draw_text(size_M_classic, al_map_rgb(255, 255, 255), SCREEN_W/4 - 25, SCREEN_H/2 + 240, 0, "Press ENTER to confirm name");

			al_draw_filled_rectangle(basePosX, basePosY, basePosX + 60, basePosY + 6, al_map_rgb(255, 255, 255));
			al_flip_display();

			sprintf(chartostr, "%c", letters[indx1]);
			al_draw_text(size_G, al_map_rgb(255, 255, 255), SCREEN_W/4 + 30 + 10, basePosY - 80, 0, chartostr);
			sprintf(chartostr, "%c", letters[indx2]);
			al_draw_text(size_G, al_map_rgb(255, 255, 255), SCREEN_W/4 + 30 + 110, basePosY - 80, 0, chartostr);
			sprintf(chartostr, "%c", letters[indx3]);
			al_draw_text(size_G, al_map_rgb(255, 255, 255), SCREEN_W/4 + 30 + 210, basePosY - 80, 0, chartostr);
			sprintf(chartostr, "%c", letters[indx4]);
			al_draw_text(size_G, al_map_rgb(255, 255, 255), SCREEN_W/4 + 30 + 310, basePosY - 80, 0, chartostr);
			al_flip_display();

			// printf("%s", name);
		}

		else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
			menu = 0;
			putRecord = 0;
			playing = 0;
		}
	}

	while(menu) {
		if(!has_sound) {
			al_uninstall_audio();
		}

		al_stop_sample_instance(lowsong_instance);

		// Enquanto estiver no menu toca o tema
		al_set_sample_instance_gain(song_instance, 0.5);
		al_play_sample_instance(song_instance);

		ALLEGRO_EVENT ev;
	  	// Espera por um evento e o armazena na variavel de evento ev
		al_wait_for_event(event_queue, &ev);

		if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
			if(ev.keyboard.keycode == ALLEGRO_KEY_UP) {
				if(!menu2) {
					dest1_y -= 60;
					dest2_y -= 60;
					if(dest1_y < SCREEN_H/2 + 30) {
						dest1_y += 60;
						dest2_y += 60;
					}

					al_play_sample(move_snd, 1.7, 0, 1, ALLEGRO_PLAYMODE_ONCE, 0);
				}
				else {
					if(firstPosY == 160 + 600)
						firstPosY = 160 + 480;

					firstPosY -= 60;
					if(firstPosY < 160)
						firstPosY = 160;

					al_play_sample(move_snd, 1.7, 0, 1, ALLEGRO_PLAYMODE_ONCE, 0);
				}
			}
			else if(ev.keyboard.keycode == ALLEGRO_KEY_DOWN) {
				if(!menu2) {
					dest1_y += 60;
					dest2_y += 60;
					if(dest1_y > SCREEN_H/2 + 210) {
						dest1_y -= 60;
						dest2_y -= 60;
					}

					al_play_sample(move_snd, 1.7, 0, 1, ALLEGRO_PLAYMODE_ONCE, 0);
				}
				else {
					firstPosY += 60;
					if(firstPosY > 160 + 420)
						firstPosY = 160 + 600;

					al_play_sample(move_snd, 1.7, 0, 1, ALLEGRO_PLAYMODE_ONCE, 0);
				}
			}
			else if(ev.keyboard.keycode == ALLEGRO_KEY_LEFT) {
				if(menu2) {
					if(firstPosY == 160) {
						count_opt1--;
						if(count_opt1 < 0)
							count_opt1 = 0;
					}
					else if(firstPosY == 160 + 60) {
						count_opt2--;
						if(count_opt2 < 0)
							count_opt2 = 0;
					}
					else if(firstPosY == 160 + 60*2) {
						count_opt3--;
						if(count_opt3 < 0)
							count_opt3 = 0;
					}
					else if(firstPosY == 160 + 60*3) {
						count_opt4--;
						if(count_opt4 < 1)
							count_opt4 = 1;
					}
					else if(firstPosY == 160 + 60*4) {
						count_opt5--;
						if(count_opt5 < 0)
							count_opt5 = 0;
					}
					else if(firstPosY == 160 + 60*5) {
						count_opt6--;
						if(count_opt6 < 0)
							count_opt6 = 0;
					}
					else if(firstPosY == 160 + 60*6) {
						count_opt7--;
						if(count_opt7 < 0)
							count_opt7 = 0;
					}
					else if(firstPosY == 160 + 60*7) {
						count_opt8--;
						if(count_opt8 < 0)
							count_opt8 = 0;
					}

					al_play_sample(move_snd_2, 1.7, 0, 1, ALLEGRO_PLAYMODE_ONCE, 0);
				}
			}
			else if(ev.keyboard.keycode == ALLEGRO_KEY_RIGHT) {
				if(menu2) {
					if(firstPosY == 160) {
						count_opt1++;
						if(count_opt1 > maxOpt1)
							count_opt1 = maxOpt1;
					}
					else if(firstPosY == 160 + 60) {
						count_opt2++;
						if(count_opt2 > maxOpt2)
							count_opt2 = maxOpt2;
					}
					else if(firstPosY == 160 + 60*2) {
						count_opt3++;
						if(count_opt3 > maxOpt3)
							count_opt3 = maxOpt3;
					}
					else if(firstPosY == 160 + 60*3) {
						count_opt4++;
						if(count_opt4 > maxOpt4)
							count_opt4 = maxOpt4;
					}
					else if(firstPosY == 160 + 60*4) {
						count_opt5++;
						if(count_opt5 > maxOpt5)
							count_opt5 = maxOpt5;
					}
					else if(firstPosY == 160 + 60*5) {
						count_opt6++;
						if(count_opt6 > maxOpt6)
							count_opt6 = maxOpt6;
					}
					else if(firstPosY == 160 + 60*6) {
						count_opt7++;
						if(count_opt7 > maxOpt7)
							count_opt7 = maxOpt7;
					}
					else if(firstPosY == 160 + 60*7) {
						count_opt8++;
						if(count_opt8 > maxOpt8)
							count_opt8 = maxOpt8;
					}

					al_play_sample(move_snd_2, 1.7, 0, 1, ALLEGRO_PLAYMODE_ONCE, 0);
				}
			}
			else if(ev.keyboard.keycode == ALLEGRO_KEY_ENTER) {
				if(!menu2) {
					walkSprite = 0;
					waveSprite = 1;

					if(dest1_y == SCREEN_H/2 + 30) {
						spriteCod = 1;
					}
					else if(dest1_y == SCREEN_H/2 + 90) {
						spriteCod = 2;
					}
					else if(dest1_y == SCREEN_H/2 + 150) {
						spriteCod = 3;
					}
					else if(dest1_y == SCREEN_H/2 + 210) {
						spriteCod = 4;
					}

					al_play_sample(select_snd, 1, 0, 1, ALLEGRO_PLAYMODE_ONCE, 0);
				}
				else {
					if(firstPosY == 160 + 600) {
						if(strcmp(opt1[count_opt1], "Normal") == 0)
							mode = 0;
						else
							mode = 1;

						if(strcmp(opt2[count_opt2], "By Time") == 0)
							endmode = 1;
						else
							endmode = 0;

						NUM_TYPES = atoi(opt3[count_opt3]);
						if(count_opt4 == 26) {
							inf_mode = 1;
						}
						else
							plays = count_opt4;

						if(strcmp(opt5[count_opt5], "00:30") == 0) {
							min = 0;
							sec = 30;
							mlsec = 0;
						}
						else if(strcmp(opt5[count_opt5], "01:00") == 0) {
							min = 1;
							sec = 0;
							mlsec = 0;
						}
						else if(strcmp(opt5[count_opt5], "03:00") == 0) {
							min = 3;
							sec = 0;
							mlsec = 0;
						}
						else if(strcmp(opt5[count_opt5], "05:00") == 0) {
							min = 5;
							sec = 0;
							mlsec = 0;
						}
						else if(strcmp(opt5[count_opt5], "10:00") == 0) {
							min = 10;
							sec = 0;
							mlsec = 0;
						}
						else if(strcmp(opt5[count_opt5], "Infinity") == 0) {
							inf_mode = 1;
						}

						if(strcmp(opt6[count_opt6], "On") == 0)
							has_sound = 1;
						else
							has_sound = 0;

						if(strcmp(opt7[count_opt7], "On") == 0)
							has_special = 1;
						else
							has_special = 0;

						if(strcmp(opt8[count_opt8], "Locked") == 0)
							mov_mode = 0;
						else
							mov_mode = 1;

						menu2 = 0;
						resetMenu();

						al_play_sample(select_snd, 1, 0, 1, ALLEGRO_PLAYMODE_ONCE, 0);
					}
				}
			}
		}
		else if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
			if(ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
				if(menu2) {
					playing = 0;
					putRecord = 0;
					menu = 0;
				}
			}
		}

		else if(ev.type == ALLEGRO_EVENT_TIMER) {
			if(!menu2) {
				draw_menu(display);

				al_draw_bitmap(logo,
							SCREEN_H/4 - 60, 0,
							0);

				al_draw_bitmap(fundo_frame, 0, 0, 0);

				if(walkSprite) {
					frame1++;
					if(frame1 >= frameDelay1) {
						frame1 = 0;
						currCol1++;
						if(currCol1 >= maxCol1)
							currCol1 = 0;

						src1_x = currCol1 * spriteW1;
					}

					al_draw_bitmap_region(walking,
									src1_x, src1_y,
									spriteW1, spriteH1,
									dest1_x, dest1_y - 3,
									0);
				}
				else if(waveSprite) {
					frame2++;
					if(frame2 >= frameDelay2) {
						frame2 = 0;
						currCol2++;
						if(currCol2 >= maxCol2)
							currCol2 = 0;

						src2_x = currCol2 * spriteW2;
					}

					al_draw_bitmap_region(waving,
									src2_x, src2_y,
									spriteW2, spriteH2,
									dest2_x, dest2_y - 3,
									0);
				}

				// Converte azul em transparente
				al_convert_mask_to_alpha(walking, al_map_rgb(77,109,243));
				al_convert_mask_to_alpha(waving, al_map_rgb(77,109,243));

				if(spriteCod == 1) {
					al_stop_sample_instance(song_instance);
					spriteDelay++;
					if(spriteDelay == 30) {
						playing = 1;
						putRecord = 0;
						menu = 0;
					}
				}
				else if(spriteCod == 3) {
					spriteDelay++;
					if(spriteDelay == 30)
						menu2 = 1;
				}
				else if(spriteCod == 4) {
					al_stop_sample_instance(song_instance);
					spriteDelay++;
					if(spriteDelay == 30) {
						menu = 0;
						putRecord = 0;
						playing = 0;
					}
				}

				if(dest1_y == SCREEN_H/2 + 30) {
					baseColor1 = al_map_rgb(255,0,0);
					baseColor2 = al_map_rgb(0,0,0);
					baseColor3 = al_map_rgb(0,0,0);
					baseColor4 = al_map_rgb(0,0,0);
				}
				else if(dest1_y ==  SCREEN_H/2 + 90) {
					baseColor1 = al_map_rgb(0,0,0);
					baseColor2 = al_map_rgb(255,0,0);
					baseColor3 = al_map_rgb(0,0,0);
					baseColor4 = al_map_rgb(0,0,0);
				}
				else if(dest1_y ==  SCREEN_H/2 + 150) {
					baseColor1 = al_map_rgb(0,0,0);
					baseColor2 = al_map_rgb(0,0,0);
					baseColor3 = al_map_rgb(255,0,0);
					baseColor4 = al_map_rgb(0,0,0);
				}
				else if(dest1_y ==  SCREEN_H/2 + 210) {
					baseColor1 = al_map_rgb(0,0,0);
					baseColor2 = al_map_rgb(0,0,0);
					baseColor3 = al_map_rgb(0,0,0);
					baseColor4 = al_map_rgb(255,0,0);
				}

				al_draw_text(size_M_classic, baseColor1, SCREEN_W/2 - 90, SCREEN_H/2 + 30, 0, "Play Single");

				al_draw_text(size_M_classic, baseColor2, SCREEN_W/2 - 90, SCREEN_H/2 + 90, 0, "Play Versus");

				al_draw_text(size_M_classic, baseColor3, SCREEN_W/2 - 90, SCREEN_H/2 + 150, 0, "Options");

				al_draw_text(size_M_classic, baseColor4, SCREEN_W/2 - 90, SCREEN_H/2 + 210, 0, "Exit");

				al_flip_display();
			}
			else {
				ALLEGRO_COLOR BKG_COLOR = al_map_rgb(0,0,0);
				al_set_target_bitmap(al_get_backbuffer(display));
				al_clear_to_color(BKG_COLOR);

				if(firstPosY == 160) {
					bbaseColor1 = al_map_rgb(255,0,0);
					bbaseColor2 = al_map_rgb(255, 255, 255);
					bbaseColor3 = al_map_rgb(255, 255, 255);
					bbaseColor4 = al_map_rgb(255, 255, 255);
					bbaseColor5 = al_map_rgb(255, 255, 255);
					bbaseColor6 = al_map_rgb(255, 255, 255);
					bbaseColor7 = al_map_rgb(255, 255, 255);
					bbaseColor8 = al_map_rgb(255, 255, 255);
					bbaseColor9 = al_map_rgb(255, 255, 255);
				}
				else if(firstPosY == 160 + 60) {
					bbaseColor1 = al_map_rgb(255, 255, 255);
					bbaseColor2 = al_map_rgb(255,0,0);
					bbaseColor3 = al_map_rgb(255, 255, 255);
					bbaseColor4 = al_map_rgb(255, 255, 255);
					bbaseColor5 = al_map_rgb(255, 255, 255);
					bbaseColor6 = al_map_rgb(255, 255, 255);
					bbaseColor7 = al_map_rgb(255, 255, 255);
					bbaseColor8 = al_map_rgb(255, 255, 255);
					bbaseColor9 = al_map_rgb(255, 255, 255);
				}
				else if(firstPosY == 160 + 60*2) {
					bbaseColor1 = al_map_rgb(255, 255, 255);
					bbaseColor2 = al_map_rgb(255, 255, 255);
					bbaseColor3 = al_map_rgb(255,0,0);
					bbaseColor4 = al_map_rgb(255, 255, 255);
					bbaseColor5 = al_map_rgb(255, 255, 255);
					bbaseColor6 = al_map_rgb(255, 255, 255);
					bbaseColor7 = al_map_rgb(255, 255, 255);
					bbaseColor8 = al_map_rgb(255, 255, 255);
					bbaseColor9 = al_map_rgb(255, 255, 255);
				}
				else if(firstPosY == 160 + 60*3) {
					bbaseColor1 = al_map_rgb(255, 255, 255);
					bbaseColor2 = al_map_rgb(255, 255, 255);
					bbaseColor3 = al_map_rgb(255, 255, 255);
					bbaseColor4 = al_map_rgb(255,0,0);
					bbaseColor5 = al_map_rgb(255, 255, 255);
					bbaseColor6 = al_map_rgb(255, 255, 255);
					bbaseColor7 = al_map_rgb(255, 255, 255);
					bbaseColor8 = al_map_rgb(255, 255, 255);
					bbaseColor9 = al_map_rgb(255, 255, 255);
				}
				else if(firstPosY == 160 + 60*4) {
					bbaseColor1 = al_map_rgb(255, 255, 255);
					bbaseColor2 = al_map_rgb(255, 255, 255);
					bbaseColor3 = al_map_rgb(255, 255, 255);
					bbaseColor4 = al_map_rgb(255, 255, 255);
					bbaseColor5 = al_map_rgb(255,0,0);
					bbaseColor6 = al_map_rgb(255, 255, 255);
					bbaseColor7 = al_map_rgb(255, 255, 255);
					bbaseColor8 = al_map_rgb(255, 255, 255);
					bbaseColor9 = al_map_rgb(255, 255, 255);
				}
				else if(firstPosY == 160 + 60*5) {
					bbaseColor1 = al_map_rgb(255, 255, 255);
					bbaseColor2 = al_map_rgb(255, 255, 255);
					bbaseColor3 = al_map_rgb(255, 255, 255);
					bbaseColor4 = al_map_rgb(255, 255, 255);
					bbaseColor5 = al_map_rgb(255, 255, 255);
					bbaseColor6 = al_map_rgb(255,0,0);
					bbaseColor7 = al_map_rgb(255, 255, 255);
					bbaseColor8 = al_map_rgb(255, 255, 255);
					bbaseColor9 = al_map_rgb(255, 255, 255);
				}
				else if(firstPosY == 160 + 60*6) {
					bbaseColor1 = al_map_rgb(255, 255, 255);
					bbaseColor2 = al_map_rgb(255, 255, 255);
					bbaseColor3 = al_map_rgb(255, 255, 255);
					bbaseColor4 = al_map_rgb(255, 255, 255);
					bbaseColor5 = al_map_rgb(255, 255, 255);
					bbaseColor6 = al_map_rgb(255, 255, 255);
					bbaseColor7 = al_map_rgb(255,0,0);
					bbaseColor8 = al_map_rgb(255, 255, 255);
					bbaseColor9 = al_map_rgb(255, 255, 255);
				}
				else if(firstPosY == 160 + 60*7) {
					bbaseColor1 = al_map_rgb(255, 255, 255);
					bbaseColor2 = al_map_rgb(255, 255, 255);
					bbaseColor3 = al_map_rgb(255, 255, 255);
					bbaseColor4 = al_map_rgb(255, 255, 255);
					bbaseColor5 = al_map_rgb(255, 255, 255);
					bbaseColor6 = al_map_rgb(255, 255, 255);
					bbaseColor7 = al_map_rgb(255, 255, 255);
					bbaseColor8 = al_map_rgb(255,0,0);
					bbaseColor9 = al_map_rgb(255, 255, 255);
				}
				else if(firstPosY == 160 + 600) {
					bbaseColor1 = al_map_rgb(255, 255, 255);
					bbaseColor2 = al_map_rgb(255, 255, 255);
					bbaseColor3 = al_map_rgb(255, 255, 255);
					bbaseColor4 = al_map_rgb(255, 255, 255);
					bbaseColor5 = al_map_rgb(255, 255, 255);
					bbaseColor6 = al_map_rgb(255, 255, 255);
					bbaseColor7 = al_map_rgb(255, 255, 255);
					bbaseColor8 = al_map_rgb(255, 255, 255);
					bbaseColor9 = al_map_rgb(255,0,0);
				}

				al_draw_text(size_M, al_map_rgb(255, 255, 255), firstPosX - SCREEN_W/2 - 150, bfirstPosY, 0, "Play Mode:");
				al_draw_text(size_M, al_map_rgb(255, 255, 255), firstPosX - SCREEN_W/2 - 150, bfirstPosY + 60, 0, "Type of Endgame:");
				al_draw_text(size_M, al_map_rgb(255, 255, 255), firstPosX - SCREEN_W/2 - 150, bfirstPosY + 120, 0, "Max Nº of Candies:");
				al_draw_text(size_M, al_map_rgb(255, 255, 255), firstPosX - SCREEN_W/2 - 150, bfirstPosY + 180, 0, "Max Nº of Plays:");
				al_draw_text(size_M, al_map_rgb(255, 255, 255), firstPosX - SCREEN_W/2 - 150, bfirstPosY + 240, 0, "Max Time:");
				al_draw_text(size_M, al_map_rgb(255, 255, 255), firstPosX - SCREEN_W/2 - 150, bfirstPosY + 300, 0, "Sound:");
				al_draw_text(size_M, al_map_rgb(255, 255, 255), firstPosX - SCREEN_W/2 - 150, bfirstPosY + 360, 0, "Special Candies:");
				al_draw_text(size_M, al_map_rgb(255, 255, 255), firstPosX - SCREEN_W/2 - 150, bfirstPosY + 420, 0, "Movement Mode:");

				sprintf(bopt1, "< %s >", opt1[count_opt1]);
				al_draw_text(size_M, bbaseColor1, firstPosX - 120, bfirstPosY, 0, bopt1);
				sprintf(bopt2, "< %s >", opt2[count_opt2]);
				al_draw_text(size_M, bbaseColor2, firstPosX - 120, bfirstPosY + 60, 0, bopt2);
				sprintf(bopt3, "< %s >", opt3[count_opt3]);
				al_draw_text(size_M, bbaseColor3, firstPosX - 120, bfirstPosY + 120, 0, bopt3);
				if(count_opt4 == maxOpt4)
					sprintf(bopt4, "< Infinity >");
				else
					sprintf(bopt4, "< %d >", count_opt4);
				al_draw_text(size_M, bbaseColor4, firstPosX - 120, bfirstPosY + 180, 0, bopt4);
				sprintf(bopt5, "< %s >", opt5[count_opt5]);
				al_draw_text(size_M, bbaseColor5, firstPosX - 120, bfirstPosY + 240, 0, bopt5);
				sprintf(bopt6, "< %s >", opt6[count_opt6]);
				al_draw_text(size_M, bbaseColor6, firstPosX - 120, bfirstPosY + 300, 0, bopt6);
				sprintf(bopt7, "< %s >", opt7[count_opt7]);
				al_draw_text(size_M, bbaseColor7, firstPosX - 120, bfirstPosY + 360, 0, bopt7);
				sprintf(bopt8, "< %s >", opt8[count_opt8]);
				al_draw_text(size_M, bbaseColor8, firstPosX - 120, bfirstPosY + 420, 0, bopt8);

				al_draw_text(size_M, bbaseColor9, SCREEN_W/2 - 150, bfirstPosY + 600, 0, "Save and Back");

				al_flip_display();
			}
		}

		else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
			menu = 0;
			putRecord = 0;
			playing = 0;
		}
	}

	initGame();
	printCandyMType();

	// Inicia o jogo sem sequencias
	const int hasSequence = 0;
	while(1) {
		findSequences();
		printCandyMType();
		clearSequences();

		if(countZeros() == hasSequence)
			break;
		else {
			swapZeros();
			changeZerosForCandies();
		}
	}

	while(playing) {
		al_stop_sample_instance(song_instance);

		if(stage3) {
			ALLEGRO_EVENT ev;
			al_wait_for_event(event_queue, &ev);

			if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
				if(ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
					menu = 0;
					putRecord = 0;
					playing = 0;
				}
			}
			else if(ev.type == ALLEGRO_EVENT_TIMER) {}
			else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
				menu = 0;
				playing = 0;
			}
		}
		else {
			if(stage1) {
				// Enquanto 'playing' e dependendo do estagio toca uma musica
				al_set_sample_instance_gain(sndtrck_stage_1, 0.7);
				al_play_sample_instance(sndtrck_stage_1);
			}
			if(stage2) {
				al_stop_sample_instance(sndtrck_stage_1);
				al_set_sample_instance_gain(sndtrck_stage_2, 0.7);
				al_play_sample_instance(sndtrck_stage_2);
			}

			ALLEGRO_EVENT ev;
			// Espera por um evento e o armazena na variavel de evento ev
			al_wait_for_event(event_queue, &ev);

			if(ev.type == ALLEGRO_EVENT_KEY_DOWN) {
				if(ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
					menu = 0;
					putRecord = 0;
					playing = 0;
				}
				else if(ev.keyboard.keycode == ALLEGRO_KEY_Q) {
					if(stage1)
						play_sound_effect(1, 2);
					if(stage2)
						play_sound_effect(2, 2);

					mustEnd = 1;
				}
			}

			else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN && !flag_animation) {
				// printf("\nclicou em (%d, %d)", ev.mouse.x, ev.mouse.y);
				getCell(ev.mouse.x, ev.mouse.y, &lin_src, &col_src);
				printf("\nCelula(%d, %d)", lin_src+1, col_src+1);
				printf("\nType = %d", M[lin_src][col_src]);
			}

			else if(ev.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP && !flag_animation) {
				// printf("\nsoltou em (%d, %d)", ev.mouse.x, ev.mouse.y);
				getCell(ev.mouse.x, ev.mouse.y, &lin_dst, &col_dst);
				printf("\nCelula(%d, %d)", lin_dst+1, col_dst+1);
				printf("\nType = %d", M[lin_src][col_src]);

				if(dist(lin_src, col_src, lin_dst, col_dst) == 1 && M[lin_src][col_src].type && M[lin_dst][col_dst].type) {
					flag_animation = 1;
					swapping = 1;

					int candytypeSrc = getCandyType(lin_src, col_src);
					int candytypeDst = getCandyType(lin_dst, col_dst);

					direction(lin_src, col_src, lin_dst, col_dst, &sen, &cos);
					int frames, inverted_frames = 0;

					if(sen == -1 && cos == 0) {
						// Animacao para baixo
						if(stage1) {
							for(frames=0;frames<SPRITE_W;frames++) {
								draw_scenario_removeSpecCandy(display, lin_src, col_src, lin_dst, col_dst, 1);
								draw_spec_candyFixedH(lin_dst, col_dst, candytypeDst, inverted_frames, 1);
								draw_spec_candyFixedH(lin_src, col_src, candytypeSrc, frames, 1);
								al_rest(0.002);
								al_flip_display();
								inverted_frames--;
							}
						}
						if(stage2) {
							for(frames=0;frames<SPRITE_W;frames++) {
								draw_scenario_removeSpecCandy(display, lin_src, col_src, lin_dst, col_dst, 2);
								draw_spec_candyFixedH(lin_dst, col_dst, candytypeDst, inverted_frames, 2);
								draw_spec_candyFixedH(lin_src, col_src, candytypeSrc, frames, 2);
								al_rest(0.002);
								al_flip_display();
								inverted_frames--;
							}
						}
					}
					else if(sen == 1 && cos == 0) {
						if(stage1) {
							// Animacao para cima
							for(frames=0;frames<SPRITE_H;frames++) {
								draw_scenario_removeSpecCandy(display, lin_src, col_src, lin_dst, col_dst, 1);
								draw_spec_candyFixedH(lin_dst, col_dst, candytypeDst, frames, 1);
								draw_spec_candyFixedH(lin_src, col_src, candytypeSrc, inverted_frames, 1);
								al_rest(0.002);
								al_flip_display();
								inverted_frames--;
							}
						}
						if(stage2) {
							for(frames=0;frames<SPRITE_H;frames++) {
								draw_scenario_removeSpecCandy(display, lin_src, col_src, lin_dst, col_dst, 2);
								draw_spec_candyFixedH(lin_dst, col_dst, candytypeDst, frames, 2);
								draw_spec_candyFixedH(lin_src, col_src, candytypeSrc, inverted_frames, 2);
								al_rest(0.002);
								al_flip_display();
								inverted_frames--;
							}
						}
					}
					else if(sen == 0 && cos == -1) {
						if(stage1) {
							// Animacao para direita
							for(frames=0;frames<SPRITE_H;frames++) {
								draw_scenario_removeSpecCandy(display, lin_src, col_src, lin_dst, col_dst, 1);
								draw_spec_candyFixedW(lin_dst, col_dst, candytypeDst, inverted_frames, 1);
								draw_spec_candyFixedW(lin_src, col_src, candytypeSrc, frames, 1);
								al_rest(0.002);
								al_flip_display();
								inverted_frames--;
							}
						}
						if(stage2) {
							for(frames=0;frames<SPRITE_H;frames++) {
								draw_scenario_removeSpecCandy(display, lin_src, col_src, lin_dst, col_dst, 2);
								draw_spec_candyFixedW(lin_dst, col_dst, candytypeDst, inverted_frames, 2);
								draw_spec_candyFixedW(lin_src, col_src, candytypeSrc, frames, 2);
								al_rest(0.002);
								al_flip_display();
								inverted_frames--;
							}
						}
					}
					else if(sen == 0 && cos == 1) {
						if(stage1) {
							// Animacao para esquerda
							for(frames=0;frames<SPRITE_H;frames++) {
								draw_scenario_removeSpecCandy(display, lin_src, col_src, lin_dst, col_dst, 1);
								draw_spec_candyFixedW(lin_dst, col_dst, candytypeDst, frames, 1);
								draw_spec_candyFixedW(lin_src, col_src, candytypeSrc, inverted_frames, 1);
								al_rest(0.002);
								al_flip_display();
								inverted_frames--;
							}
						}
						if(stage2) {
							for(frames=0;frames<SPRITE_H;frames++) {
								draw_scenario_removeSpecCandy(display, lin_src, col_src, lin_dst, col_dst, 2);
								draw_spec_candyFixedW(lin_dst, col_dst, candytypeDst, frames, 2);
								draw_spec_candyFixedW(lin_src, col_src, candytypeSrc, inverted_frames, 2);
								al_rest(0.002);
								al_flip_display();
								inverted_frames--;
							}
						}
					}

					swapCells(lin_src, col_src, lin_dst, col_dst);
					if(!mov_mode) {
						if(findSequences() == 0) {
							al_play_sample(select_snd, 1, 0, 1, ALLEGRO_PLAYMODE_ONCE, 0);
							swapCells(lin_dst, col_dst, lin_src, col_src);
							if(!inf_mode)
								plays++;
						}
					}
				}

				swapping = 0;
			}

			// Se o tipo de evento for um evento de timer, redesenha a tela
			else if(ev.type == ALLEGRO_EVENT_TIMER) {
				// Contagem do relogio
				if(!inf_mode) {
					if(mlsec < 60)
						mlsec--;
					if(mlsec < 0) {
						mlsec = 59;
						sec--;
						if(sec < 0) {
							sec = 59;
							min--;
							if(min <= 0) {
								min = 0;
								sec = 0;
								mlsec = 0;
							}
						}
					}
				}

				if(!mustEnd && !swapping) {
					if(stage1) {
						draw_scenario(display, 1);
						al_flip_display();
					}
					if(stage2) {
						draw_scenario(display, 2);
						al_flip_display();
					}
				}

				score = findSequences();
				while(score > 0) {
					if(stage1) {
						draw_scenario(display, 1);
						al_flip_display();
						play_sound_effect(1, 1);
					}
					if(stage2) {
						draw_scenario(display, 2);
						al_flip_display();
						play_sound_effect(2, 1);
					}
					pausa(timer);
					clearSequences();
					swapZeros();
					final_score += score*12;
					score = findSequences();
				}
				changeZerosForCandies();

				if(final_score > 70) {
					stage1 = 0;
					stage2 = 1;
				}

				if(inf_mode) {
					min = 59;
					sec = 59;
					plays = 999;
				}
				else {
					if(endmode && !flag_dontrepeat) {
						if(min == 0 && sec == 0) {
							al_stop_sample_instance(sndtrck_stage_1);
							al_stop_sample_instance(sndtrck_stage_2);

							if(stage1)
								play_sound_effect(1, 2);
							if(stage2)
								play_sound_effect(2, 2);

							al_rest(1.3);

							mustEnd = 1;
						}
					}
					else if(!endmode && !flag_dontrepeat) {
						if(plays == 0) {
							al_stop_sample_instance(sndtrck_stage_1);
							al_stop_sample_instance(sndtrck_stage_2);

							if(stage1)
								play_sound_effect(1, 2);
							if(stage2)
								play_sound_effect(2, 2);

							al_rest(1.3);

							mustEnd = 1;
						}
					}
				}

				// Reseta animacao depois de um evento de timer
				flag_animation = 0;

				if(mustEnd) {
					ALLEGRO_COLOR BKG_COLOR = al_map_rgb(0,0,0);
					al_set_target_bitmap(al_get_backbuffer(display));
					al_clear_to_color(BKG_COLOR);

					al_stop_sample_instance(sndtrck_stage_1);
					al_stop_sample_instance(sndtrck_stage_2);
					al_set_sample_instance_gain(sadsong_instance, 0.7);
					al_play_sample_instance(sadsong_instance);

					flag_animation = 1;
					flag_dontrepeat = 1;

					frame++;
					if(frame >= frameDelay) {
						frame = 0;
						currLin++;
						if(currLin >= maxLin)
							currLin = 0;

						src_y = currLin * spriteH;
					}

					al_draw_bitmap_region(gamover,
										src_x, src_y,
										spriteW, spriteH,
										dest_x, dest_y,
										0);

					sprintf(my_score, "Your final score: %d", final_score);
					al_draw_text(size_M_classic, al_map_rgb(240,240,250), SCREEN_W/4 + 30, SCREEN_H/4 + SCREEN_H/2 + 30, 0, my_score);
					if(flag_callonce) {
						tfrecord = newRecord(srcname, final_score, &srcrecord);
						flag_callonce = 0;
					}
					if(tfrecord) {
						al_draw_text(size_M_classic, al_map_rgb(240,240,250), SCREEN_W/4 + 105, SCREEN_H/4 + SCREEN_H/2 - 40, 0, "NEW RECORD!");
					}
					else {
						sprintf(my_record, "Record: %d", srcrecord);
						al_draw_text(size_M_classic, al_map_rgb(240,240,250), SCREEN_W/4 + 120, SCREEN_H/4 + SCREEN_H/2 - 40, 0, my_record);
					}

					sprintf(pressesc, "Press ESC to exit");
					al_draw_text(size_P_classic, al_map_rgb(255, 255, 255),	SCREEN_W/4 + 90, SCREEN_H/4 + SCREEN_H/2 + 120, 0, pressesc);

					al_flip_display();
				}
			}

			else if(ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
				menu = 0;
				playing = 0;
			}
		}
	}

	al_stop_sample_instance(lowsong_instance);
	al_stop_sample_instance(song_instance);
	al_stop_sample_instance(sndtrck_stage_1);
	al_stop_sample_instance(sndtrck_stage_2);
	al_stop_sample_instance(sadsong_instance);

	// Termino do programa
	al_clear_to_color(al_map_rgb(0,0,0));

	sprintf(my_record, "Player: %s", srcname);
	al_draw_text(size_G, al_map_rgb(240,240,250), SCREEN_W/4, SCREEN_H/2 - 100, 0, my_record);
	sprintf(my_record, "Record: %d", srcrecord);
	al_draw_text(size_G, al_map_rgb(240,240,250), SCREEN_W/4, SCREEN_H/2 + 10, 0, my_record);

	// Reinicia a tela para mostrar os pontos e o recorde
	al_flip_display();

	al_rest(2);

	// Rotinas de limpeza de memória
	al_destroy_timer(timer);
	al_destroy_display(display);
	al_destroy_event_queue(event_queue);
	al_destroy_bitmap(sprite);
	al_destroy_bitmap(sprite_2);
	al_destroy_bitmap(tnt);
	al_destroy_bitmap(gamover);
	al_destroy_bitmap(tile_1);
	al_destroy_bitmap(tile_2);
	al_destroy_bitmap(fundo_1);
	al_destroy_bitmap(fundo_2);
	al_destroy_bitmap(fundo_menu);
	al_destroy_bitmap(fundo_frame);
	al_destroy_bitmap(walking);
	al_destroy_bitmap(waving);
	al_destroy_bitmap(logo);
	al_destroy_sample(gotpoints[0].sound);
	al_destroy_sample(gotpoints[1].sound);
	al_destroy_sample(gotpoints[2].sound);
	al_destroy_sample(gotpoints[3].sound);
	al_destroy_sample(gotpoints[4].sound);
	al_destroy_sample(gotpoints[5].sound);
	al_destroy_sample(gotpoints[6].sound);
	al_destroy_sample(gotpoints[7].sound);
	al_destroy_sample(end[0].sound);
	al_destroy_sample(end[1].sound);
	al_destroy_sample(end[2].sound);
	al_destroy_sample(end[3].sound);
	al_destroy_sample(gotpoints2[0].sound);
	al_destroy_sample(gotpoints2[1].sound);
	al_destroy_sample(gotpoints2[2].sound);
	al_destroy_sample(gotpoints2[3].sound);
	al_destroy_sample(gotpoints2[4].sound);
	al_destroy_sample(gotpoints2[5].sound);
	al_destroy_sample(gotpoints2[6].sound);
	al_destroy_sample(gotpoints2[7].sound);
	al_destroy_sample(end2[0].sound);
	al_destroy_sample(end2[1].sound);
	al_destroy_sample(end2[2].sound);
	al_destroy_sample(swipe);
	al_destroy_sample(select_snd);
	al_destroy_sample(error_snd);
	al_destroy_sample(move_snd);
	al_destroy_sample(move_snd_2);
	al_destroy_sample(song);
	al_destroy_sample_instance(song_instance);
	al_destroy_sample(soundtrack_1);
	al_destroy_sample(soundtrack_2);
	al_destroy_sample_instance(sndtrck_stage_1);
	al_destroy_sample_instance(sndtrck_stage_2);
	al_destroy_sample(sadsong);
	al_destroy_sample_instance(sadsong_instance);
	al_destroy_sample(lowsong);
	al_destroy_sample_instance(lowsong_instance);

	return 0;
}