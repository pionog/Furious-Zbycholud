#include"defining.h"

int startTheGame(int argc, char** argv);
// narysowanie napisu txt na powierzchni screen, zaczynaj¹c od punktu (x, y)
// charset to bitmapa 128x128 zawieraj¹ca znaki
// draw a text txt on surface screen, starting from the point (x, y)
// charset is a 128x128 bitmap containing character images
void DrawString(SDL_Surface *screen, int x, int y, const char *text,
                SDL_Surface *charset) {
	int px, py, c;
	SDL_Rect size, dimension;
	size.w = 8;
	size.h = 8;
	dimension.w = 8;
	dimension.h = 8;
	while(*text) {
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		size.x = px;
		size.y = py;
		dimension.x = x;
		dimension.y = y;
		SDL_BlitSurface(charset, &size, screen, &dimension);
		x += 8;
		text++;
		};
	};
typedef struct {
	double xHelPos; // pozycja pigulki na ekranie w osi X
	double yHelPos; // pozycja pigulki na ekranie w osi Y
	double xHelRel; // relatywna pozycja pigulki w osi X z uwzglednieniem poprawki
	double yHelRel; // relatywna pozycja pigulki w osi Y z uwzlgednieniem poprawki
	int widHel; // szerokosc pigulki
	int heiHel; // wysokosc pigulki
	int timeHel; // czas, po ktorym pojawi sie pigulka na planszy
	int taken; // sprawdzanie, czy pigulka zostala zazyta. 0 - nie, 1 - tak
	SDL_Surface* sprite; // plik tekstury pigulki
	SDL_Rect areaHel; // obszar pigulki
}healthPills;

typedef struct {
	double xPlaVel; // predkosc w osi X
	double yPlaVel; // predkosc w osi Y
	double xPlaPos; // pozycja postaci na ekranie w osi X
	double yPlaPos; // pozycja postaci na ekranie w osi Y
	double xPlaRel; // relatywna pozycja postaci w osi X z uwzglednieniem poprawki
	double yPlaRel; // relatywna pozycja postaci w osi Y z uwzlgednieniem poprawki
	double xPlaCor; // relatywna pozycja postaci w osi X bez korekty pozycji o pozycje na ekranie
	double yPlaCor; // relatywna pozycja postaci w osi Y bez korekty pozycji o pozycje na ekranie
	int widPla; // szerokosc postaci
	int heiPla; // wysokosc postaci
	int health; // liczba zdrowia
	int whichType; // rozroznienie typu postaci. 0 - gracz, 1 - przeciwnik
	double timeReload; // czas przeladowania kolejnego pocisku
	SDL_Surface* texture; // grafika postaci
	SDL_Rect areaPla; // obszar grafiki postaci
}character;

// ustawienie sterowania
typedef struct {
	int up;
	int down;
	int left;
	int right;
	int fire; // strzelanie kryje sie pod spacja
} keyboardUsage;
static keyboardUsage movement;

typedef struct {
	double xBulVel; // predkosc w osi X
	double yBulVel; // predkosc w osi Y
	double xBulPos; // pozycja pocisku na ekranie w osi X
	double yBulPos; // pozycja pocisku na ekranie w osi Y
	double xBulRel; // relatywna pozycja pocisku w osi X z uwzglednieniem poprawki
	double yBulRel; // relatywna pozycja pocisku w osi Y z uwzlgednieniem poprawki
	int widBul; // szerokosc pocusku
	int heiBul; // wysokosc pocisku
	int healBul; // liczba zdrowia pocisku ile zada graczowi
	int dirBul; // kierunek lotu pocisku
	int owner; // zmienna umozliwiajaca rozpoznanie, kto wystrzelil dany pocisk. 0 - gracz, 1 - przeciwnik
	SDL_Surface* sprite; // tekstura pocisku
	SDL_Rect areaBul; // obszar pocisku
}bullet;

typedef struct {
	bullet* bb; // wskaznik na strukture pocisku
	int count; // liczba pociskow
	int max_count; // maksymalna liczba pociskow
}bullet_array;
static bullet_array* bullet_array_ptr;
bullet_array* bulletArrayInit(bullet_array* bullet_array_ptr, int size) {
	if (bullet_array_ptr == NULL) {
		bullet_array_ptr = (bullet_array*)malloc(sizeof(bullet_array));
		memset(bullet_array_ptr, 0, sizeof(bullet_array));
	}
	if (bullet_array_ptr->bb) {
		free(bullet_array_ptr->bb);
	}
	if (size > 0) {
		bullet_array_ptr->bb = (bullet*)malloc(sizeof(bullet) * size);
	}
	bullet_array_ptr->count = 0;
	bullet_array_ptr->max_count = size;
	bullet_array* bul_arr;
	bul_arr = bullet_array_ptr;
	return bul_arr;
}


void addBullet(bullet_array* bullet_ptr, SDL_Surface* sprite, SDL_Rect areaBul, int x, int y, int xVel, int yVel, int owner, float direction) {
	if (bullet_ptr->count == bullet_ptr->max_count) {
		bullet* new_bb = (bullet*)malloc(sizeof(bullet) * bullet_ptr->max_count * 2);
		memcpy(new_bb, bullet_ptr->bb, sizeof(bullet) * bullet_ptr->max_count);
		free(bullet_ptr->bb);
		bullet_ptr->bb = new_bb;
		bullet_ptr->max_count = bullet_ptr->max_count * 2;
	}
	bullet_ptr->bb[bullet_ptr->count].xBulRel = x;
	bullet_ptr->bb[bullet_ptr->count].yBulRel = y;
	bullet_ptr->bb[bullet_ptr->count].healBul = 1;
	bullet_ptr->bb[bullet_ptr->count].xBulVel = xVel;
	bullet_ptr->bb[bullet_ptr->count].yBulVel = yVel;
	bullet_ptr->bb[bullet_ptr->count].sprite = sprite;
	bullet_ptr->bb[bullet_ptr->count].areaBul = areaBul;
	bullet_ptr->bb[bullet_ptr->count].owner = owner;
	bullet_ptr->count += 1;

}
// tworzenie recta wykorzysujac informacje z surface'a (wysokosc, szerokosc) oraz koordynaty
SDL_Rect SaveRectInfo(SDL_Surface* sprite, int x, int y) {
	SDL_Rect dest;
	dest.x = x;
	dest.y = y;
	dest.w = sprite->w;
	dest.h = sprite->h;
	return dest;
}
// narysowanie na ekranie screen powierzchni sprite w punkcie (x, y)
// (x, y) to punkt œrodka obrazka sprite na ekranie
// draw a surface sprite on a surface screen in point (x, y)
// (x, y) is the center of sprite on screen
void DrawSurface(SDL_Surface* screen, SDL_Surface* sprite, int x, int y) {
	SDL_Rect dest = SaveRectInfo(sprite, x, y);
	SDL_BlitSurface(sprite, NULL, screen, &dest);
};
//funkcja jak wyzej, tyle ze wspiera rysowanie animacji
void DrawAnimation(SDL_Surface* screen, SDL_Surface* sprite, int howManyFrames, int whichFrame, int x, int y) {
	SDL_Rect dest = SaveRectInfo(sprite, x, y);
	SDL_Rect IMG;
	IMG.w = sprite->w;
	IMG.h = sprite->h;
	IMG.y = 0;
	IMG.x = sprite->w / howManyFrames * whichFrame;
	SDL_BlitSurface(sprite, &IMG, screen, &dest);
};

// rysowanie pojedynczego pixela
// draw a single pixel
void DrawPixel(SDL_Surface *surface, int x, int y, Uint32 color) {
	int bpp = surface->format->BytesPerPixel;
	Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32 *)p = color;
	};


// rysowanie linii o d³ugoœci l w pionie (gdy dx = 0, dy = 1) 
// b¹dŸ poziomie (gdy dx = 1, dy = 0)
// draw a vertical (when dx = 0, dy = 1) or horizontal (when dx = 1, dy = 0) line
extern void DrawLine(SDL_Surface *screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for(int i = 0; i < l; i++) {
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
		};
	};


// rysowanie prostok¹ta o d³ugoœci boków l i k
// draw a rectangle of size l by k
extern void DrawRectangle(SDL_Surface *screen, int x, int y, int l, int k,
                   Uint32 outlineColor, Uint32 fillColor) {
	int i;
	DrawLine(screen, x, y, k, 0, 1, outlineColor);
	DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(screen, x, y, l, 1, 0, outlineColor);
	DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for(i = y + 1; i < y + k - 1; i++)
		DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
	};

// wyrzucanie bledu, ktory wystapi w trakcie wczytywania obrazka
int returningEr(SDL_Surface* screen, SDL_Texture* scrtex, SDL_Window* window, SDL_Renderer* renderer)
{
	SDL_FreeSurface(screen);
	SDL_DestroyTexture(scrtex);
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_Quit();
	return 1;
}
// sprawdzanie kolizji
int collisionChecker(SDL_Rect* rect1, SDL_Rect* rect2) {
	if (rect1->y > rect2->y + rect2->h) return 0;
	else if (rect1->y + rect1->h < rect2->y) return 0;
	else if (rect1->x > rect2->x + rect2->w) return 0;
	else if (rect1->x + rect1->w < rect2->x) return 0;
	else return 1;
}
// sprawdzanie kolizji i blokowanie ruchu, gdy postac1 probuje przeniknac przez postac2
int dontMoveWhileColides(character character, SDL_Rect rect, keyboardUsage keyboard) {
	if (collisionChecker(&SaveRectInfo(character.texture, character.xPlaRel + character.xPlaVel, character.yPlaRel + character.yPlaVel), &rect) == 1) {
		if (keyboard.left || keyboard.right) {return 2;}
		if (keyboard.up || keyboard.down) {return 3;}
		if ((keyboard.left || keyboard.right) && (keyboard.up || keyboard.down)) { return 4; }
		else return 0;
	}
	else return 0;
}

static SDL_Rect camera = { AREA_WIDTH/2, AREA_HEIGHT/2, SCREEN_WIDTH, SCREEN_HEIGHT };
// rysowanie obiektow uwzgledniajac ruch kamery
void DrawRelativeSurface(SDL_Surface* screen, SDL_Surface* sprite, int x, int y) {
	SDL_Rect relDest = { SaveRectInfo(sprite, x, y).x, SaveRectInfo(sprite, x, y).y, SaveRectInfo(sprite, x, y).w, SaveRectInfo(sprite, x, y).h };
	if (collisionChecker(&camera, &relDest)) {
		SDL_Rect relDest = { SaveRectInfo(sprite, x, y).x - camera.x, SaveRectInfo(sprite, x, y).y - camera.y, SaveRectInfo(sprite, x, y).w, SaveRectInfo(sprite, x, y).h };
		SDL_BlitSurface(sprite, NULL, screen, &relDest);
	}
};
// widok pociskow
void viewBullets(SDL_Surface* screen, bullet_array* bullet_ptr) {
	if (bullet_ptr != nullptr)
	for (int i = 0; i < bullet_ptr->count; i++) {
		bullet_ptr->bb[i].yBulRel += bullet_ptr->bb[i].yBulVel;
		bullet_ptr->bb[i].areaBul.y += bullet_ptr->bb[i].yBulVel;
		bullet_ptr->bb[i].xBulRel += bullet_ptr->bb[i].xBulVel;
		bullet_ptr->bb[i].areaBul.x += bullet_ptr->bb[i].xBulVel;
		if (bullet_ptr->bb[i].healBul == 1) {DrawRelativeSurface(screen, bullet_ptr->bb[i].sprite, (int)bullet_ptr->bb[i].xBulRel, (int)bullet_ptr->bb[i].yBulRel);}
	}
}
// zadawanie obrazen przez pociski
int dealDamage(character character, bullet_array* bullet_ptr) {
	if (bullet_ptr != nullptr)
		for (int i = 0; i < bullet_ptr->count; i++) {
			if (collisionChecker(&SaveRectInfo(character.texture, character.xPlaRel - character.widPla/2, character.yPlaRel - character.heiPla / 2), &bullet_ptr->bb[i].areaBul) == 1 && bullet_ptr->bb[i].healBul != 0 && bullet_ptr->bb[i].owner != character.whichType) {
				bullet_ptr->bb[i].healBul = 0;
				return 1;
			}
		}
	return 0;
}


// main
#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char** argv) {
	startTheGame(argc, argv);
	return 0;
}
int startTheGame(int argc, char** argv){
	int t1, t2, quit, frames, animationFrames, descendingFrames, shiftingImages, menuStart = 1, rc, score = 0, playerHits, playerHitsAtTheMoment = 0, enemyHits = 0, menuState = 1, gameState = 1, immunity = 0, xPos = SCREEN_WIDTH / 2, yPos = 400;
	double delta, hitsInArow = 0, worldTime, menuTime, elapsedTime, multiplier, fpsTimer, fps, distance, etiSpeed, xVel = 0, yVel = 0;
	int playerSpeed = PLAYER_SPEED_UNIT * PLAYER_SPEED_MULTIPLIER;
	SDL_Event event;
	SDL_Surface *background;
	SDL_Surface *eti, *title, *shot, *shotplayer, *heal, *heart, *heartHalf, *heartEmpty;
	SDL_Surface *playership, *zbycholud, *playershipForward, *playershipLeft, *playershipRight, *playershipBackward;
	SDL_Surface *gradeA, *gradeAPlus, *gradeB, *gradeBPlus, *gradeC, *gradeCPlus, *gradeD, *gradeDPlus, *gradeF;
	SDL_Surface *screen, *charset; // screen - cale okienko w tle, charset tablica znakow
	SDL_Texture *scrtex; // tekstura tla
	SDL_Window *window; // okienko z gra
	SDL_Renderer *renderer; // obslugiwanie renderowania
	
	
	if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
		}

	// tryb pe³noekranowy / fullscreen mode
//	rc = SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP,
//	                                 &window, &renderer);
	rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0,
	                                 &window, &renderer);
	if(rc != 0) {
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		return 1;
		};
	
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

	SDL_SetWindowTitle(window, "projekt2");


	screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
	                              0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

	scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
	                           SDL_TEXTUREACCESS_STREAMING,
	                           SCREEN_WIDTH, SCREEN_HEIGHT);


	// wy³¹czenie widocznoœci kursora myszy
	//SDL_ShowCursor(SDL_DISABLE);

	// wczytanie obrazka cs8x8.bmp
	charset = SDL_LoadBMP("./assets/cs8x8.bmp");
	if (charset == NULL) {
		printf("SDL_LoadBMP(cs8x8.bmp) error: %s\n", SDL_GetError());
		returningEr(screen, scrtex, window, renderer);
	};
	SDL_SetColorKey(charset, true, 0x000000);
	eti = SDL_LoadBMP("./assets/eti.bmp");
	if (eti == NULL) {
		printf("SDL_LoadBMP(eti.bmp) error: %s\n", SDL_GetError());
		returningEr(screen, scrtex, window, renderer);
	};
	title = SDL_LoadBMP("./assets/title.bmp");
	if (title == NULL) {
		printf("SDL_LoadBMP(title.bmp) error: %s\n", SDL_GetError());
		returningEr(screen, scrtex, window, renderer);
	};
	zbycholud = SDL_LoadBMP("./assets/zbycholud.bmp");
	if (zbycholud == NULL) {
		printf("SDL_LoadBMP(zbycholud.bmp) error: %s\n", SDL_GetError());
		returningEr(screen, scrtex, window, renderer);
	};
	shot = SDL_LoadBMP("./assets/shot.bmp");
	if (shot == NULL) {
		printf("SDL_LoadBMP(shot.bmp) error: %s\n", SDL_GetError());
		returningEr(screen, scrtex, window, renderer);
	};
	shotplayer = SDL_LoadBMP("./assets/shotplayer.bmp");
	if (shotplayer == NULL) {
		printf("SDL_LoadBMP(shotplayer.bmp) error: %s\n", SDL_GetError());
		returningEr(screen, scrtex, window, renderer);
	};
	playership = SDL_LoadBMP("./assets/spaceship.bmp");
	if (playership == NULL) {
		printf("SDL_LoadBMP(spaceship.bmp) error: %s\n", SDL_GetError());
		returningEr(screen, scrtex, window, renderer);
	};
	playershipLeft = SDL_LoadBMP("./assets/spaceshipLeft.bmp");
	if (playership == NULL) {
		printf("SDL_LoadBMP(spaceshipLeft.bmp) error: %s\n", SDL_GetError());
		returningEr(screen, scrtex, window, renderer);
	};
	playershipRight = SDL_LoadBMP("./assets/spaceshipRight.bmp");
	if (playership == NULL) {
		printf("SDL_LoadBMP(spaceshipRight.bmp) error: %s\n", SDL_GetError());
		returningEr(screen, scrtex, window, renderer);
	};
	playershipForward = SDL_LoadBMP("./assets/spaceshipForward.bmp");
	if (playership == NULL) {
		printf("SDL_LoadBMP(spaceshipForward.bmp) error: %s\n", SDL_GetError());
		returningEr(screen, scrtex, window, renderer);
	};
	playershipBackward = SDL_LoadBMP("./assets/spaceshipBackward.bmp");
	if (playership == NULL) {
		printf("SDL_LoadBMP(spaceshipBackward.bmp) error: %s\n", SDL_GetError());
		returningEr(screen, scrtex, window, renderer);
	};
	heal = SDL_LoadBMP("./assets/heal.bmp");
	if (heal == NULL) {
		printf("SDL_LoadBMP(heal.bmp) error: %s\n", SDL_GetError());
		returningEr(screen, scrtex, window, renderer);
	};
	heart = SDL_LoadBMP("./assets/serduszko.bmp");
	if (heart == NULL) {
		printf("SDL_LoadBMP(serduszko.bmp) error: %s\n", SDL_GetError());
		returningEr(screen, scrtex, window, renderer);
	};
	heartHalf = SDL_LoadBMP("./assets/serduszkopol.bmp");
	if (heartHalf == NULL) {
		printf("SDL_LoadBMP(serduszkopol.bmp) error: %s\n", SDL_GetError());
		returningEr(screen, scrtex, window, renderer);
	};
	heartEmpty = SDL_LoadBMP("./assets/serduszkonic.bmp");
	if (heartEmpty == NULL) {
		printf("SDL_LoadBMP(serduszkonic.bmp) error: %s\n", SDL_GetError());
		returningEr(screen, scrtex, window, renderer);
	};
	background = SDL_LoadBMP("./assets/background.bmp");
	if (background == NULL) {
		printf("SDL_LoadBMP(background.bmp) error: %s\n", SDL_GetError());
		returningEr(screen, scrtex, window, renderer);
	};
	// wczytywanie grafik ocen
	gradeA = SDL_LoadBMP("./assets/animatedA.bmp");
	if (gradeA == NULL) {
		printf("SDL_LoadBMP(animatedA.bmp) error: %s\n", SDL_GetError());
		returningEr(screen, scrtex, window, renderer);
	};
	gradeAPlus = SDL_LoadBMP("./assets/animatedAPlus.bmp");
	if (gradeAPlus == NULL) {
		printf("SDL_LoadBMP(animatedAPlus.bmp) error: %s\n", SDL_GetError());
		returningEr(screen, scrtex, window, renderer);
	};
	gradeB = SDL_LoadBMP("./assets/animatedB.bmp");
	if (gradeB == NULL) {
		printf("SDL_LoadBMP(animatedB.bmp) error: %s\n", SDL_GetError());
		returningEr(screen, scrtex, window, renderer);
	};
	gradeBPlus = SDL_LoadBMP("./assets/animatedBPlus.bmp");
	if (gradeBPlus == NULL) {
		printf("SDL_LoadBMP(animatedBPlus.bmp) error: %s\n", SDL_GetError());
		returningEr(screen, scrtex, window, renderer);
	};
	gradeC = SDL_LoadBMP("./assets/animatedC.bmp");
	if (gradeC == NULL) {
		printf("SDL_LoadBMP(animatedC.bmp) error: %s\n", SDL_GetError());
		returningEr(screen, scrtex, window, renderer);
	};
	gradeCPlus = SDL_LoadBMP("./assets/animatedCPlus.bmp");
	if (gradeCPlus == NULL) {
		printf("SDL_LoadBMP(animatedCPlus.bmp) error: %s\n", SDL_GetError());
		returningEr(screen, scrtex, window, renderer);
	};
	gradeD = SDL_LoadBMP("./assets/animatedD.bmp");
	if (gradeD == NULL) {
		printf("SDL_LoadBMP(animatedD.bmp) error: %s\n", SDL_GetError());
		returningEr(screen, scrtex, window, renderer);
	};
	gradeDPlus = SDL_LoadBMP("./assets/animatedDPlus.bmp");
	if (gradeDPlus == NULL) {
		printf("SDL_LoadBMP(animatedDPlus.bmp) error: %s\n", SDL_GetError());
		returningEr(screen, scrtex, window, renderer);
	};
	gradeF = SDL_LoadBMP("./assets/animatedf.bmp");
	if (gradeF == NULL) {
		printf("SDL_LoadBMP(animatedf.bmp) error: %s\n", SDL_GetError());
		returningEr(screen, scrtex, window, renderer);
	};

	// inicjowanie structów gracza oraz pocisków
	bullet_array* bul_arr = bulletArrayInit(bullet_array_ptr, 100);
	character spaceship = {
		0, // szybkosc gracza w osi X
		0, // szybkosc gracza w osi Y
		HUDX / 2, // pozycja gracza w osi X
		SCREEN_HEIGHT / 2, // pozycja gracza w osi Y
		camera.x - SCREEN_WIDTH / 2, // relatywna pozycja gracza w osi X
		camera.y, // relatywna pozycja gracza w osi Y
		camera.x - SCREEN_WIDTH / 2,
		camera.y,
		SaveRectInfo(playership, spaceship.xPlaPos, spaceship.yPlaPos).w, // szerekosc gracza(statku)
		SaveRectInfo(playership, spaceship.xPlaPos, spaceship.yPlaPos).h, // wysokosc gracza(statku)
		100, // zdrowie gracza
		0, // typ postaci - gracz
		0, // poczatkowy czas trwania przeladowania liczona w liczbie klatek(zmienia sie w trakcie gry)
		playership, // grafika gracza
		SaveRectInfo(playership, spaceship.xPlaPos, spaceship.yPlaPos), // obszar gracza
	};
	character enemy1 = {
		0, // szybkosc gracza w osi X
		0, // szybkosc gracza w osi Y
		AREA_WIDTH / 2, // pozycja gracza w osi X
		AREA_HEIGHT / 4, // pozycja gracza w osi Y
		AREA_WIDTH/2, // relatywna pozycja gracza w osi X
		AREA_HEIGHT/4, // relatywna pozycja gracza w osi Y
		AREA_WIDTH/2,
		AREA_HEIGHT/4,
		SaveRectInfo(zbycholud, enemy1.xPlaPos, enemy1.yPlaPos).w, // szerekosc gracza(statku)
		SaveRectInfo(zbycholud, enemy1.xPlaPos, enemy1.yPlaPos).h, // wysokosc gracza(statku)
		100, // zdrowie przeciwnika
		1, // typ postaci - przeciwnik
		0, // poczatkowy czas trwania przeladowania liczona w liczbie klatek(zmienia sie w trakcie gry)
		zbycholud, // grafika gracza
		SaveRectInfo(zbycholud, enemy1.xPlaPos, enemy1.yPlaPos), // obszar gracza
	};
	healthPills health[PILLS_AMOUNT];
	for (int i = 0; i < PILLS_AMOUNT; i++) { 
		health[i].xHelRel = rand() % AREA_WIDTH - SCREEN_WIDTH + HUDX;
		health[i].yHelRel = rand() % AREA_HEIGHT;
		health[i].sprite = heal;
		health[i].timeHel = 2000 * i;
		health[i].taken = 0;
		health[i].widHel = health[i].sprite->w;
		health[i].heiHel = health[i].sprite->h;
		health[i].areaHel.x = health[i].xHelRel;
		health[i].areaHel.y = health[i].yHelRel;
		health[i].areaHel.w = health[i].widHel;
		health[i].areaHel.h = health[i].heiHel;
	}
	

	char text[128];
	int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	int zielony = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
	int czerwony = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
	int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);

	t1 = SDL_GetTicks();

	frames = 0;
	fpsTimer = 0;
	fps = 0;
	quit = 0;
	worldTime = 0;
	menuTime = 0;
	elapsedTime = 0;
	distance = 0;
	etiSpeed = 1;

	while (!quit) {
		
		if (gameState == 1) { 
			if (bul_arr != NULL ){ 
				free(bul_arr->bb);
			}
			bul_arr = bulletArrayInit(bullet_array_ptr, 100);
			t1 = SDL_GetTicks();
			score = 0;
			enemyHits = 0;
			playerHits = 0;
			playerHitsAtTheMoment = 0;
			immunity = 0;
			frames = 0;
			animationFrames = 0;
			descendingFrames = 0;
			shiftingImages = 0;
			fpsTimer = 0;
			fps = 0;
			quit = 0;
			worldTime = 0;
			menuTime = 0;
			elapsedTime = 0;
			hitsInArow = 0;
			multiplier = 1;
			distance = 0;
			etiSpeed = 1;
			camera = { AREA_WIDTH / 2 - SCREEN_WIDTH / 2, AREA_HEIGHT / 2, SCREEN_WIDTH, SCREEN_HEIGHT };
			for (int i = 0; i < 5; i++) { health[i].xHelRel = rand() % AREA_WIDTH - SCREEN_WIDTH + HUDX; health[i].yHelRel = rand() % AREA_HEIGHT; }
			// inicjowanie structów gracza oraz pocisków
			spaceship.xPlaVel = 0;
			spaceship.yPlaVel = 0;
			spaceship.xPlaPos = HUDX / 2;
			spaceship.yPlaPos = SCREEN_HEIGHT / 2;
			spaceship.xPlaRel = camera.x - SCREEN_WIDTH / 2;
			spaceship.yPlaRel = camera.y;
			spaceship.xPlaCor = camera.x - SCREEN_WIDTH / 2;
			spaceship.yPlaCor = camera.y;
			spaceship.health = 100;
			spaceship.whichType = 0;
			spaceship.timeReload = 0;
			spaceship.texture = playership;
			spaceship.areaPla = SaveRectInfo(playership, spaceship.xPlaPos, spaceship.yPlaPos);
			enemy1 = {
				0, // szybkosc gracza w osi X
				0, // szybkosc gracza w osi Y
				AREA_WIDTH / 2, // pozycja gracza w osi X
				AREA_HEIGHT / 4, // pozycja gracza w osi Y
				AREA_WIDTH / 2, // relatywna pozycja gracza w osi X
				AREA_HEIGHT / 4, // relatywna pozycja gracza w osi Y
				AREA_WIDTH / 2,
				AREA_HEIGHT / 4,
				SaveRectInfo(zbycholud, enemy1.xPlaPos, enemy1.yPlaPos).w, // szerekosc gracza(statku)
				SaveRectInfo(zbycholud, enemy1.xPlaPos, enemy1.yPlaPos).h, // wysokosc gracza(statku)
				100, // zdrowie przeciwnika
				1, // typ postaci - przeciwnik
				0, // poczatkowy czas trwania przeladowania liczona w liczbie klatek(zmienia sie w trakcie gry)
				zbycholud, // grafika gracza
				SaveRectInfo(zbycholud, enemy1.xPlaPos, enemy1.yPlaPos), // obszar gracza
			};
			for (int i = 0; i < PILLS_AMOUNT; i++) {
				health[i].xHelRel = rand() % AREA_WIDTH - SCREEN_WIDTH + HUDX;
				health[i].yHelRel = rand() % AREA_HEIGHT;
				health[i].sprite = heal;
				health[i].widHel = health[i].sprite->w;
				health[i].heiHel = health[i].sprite->h;
				health[i].timeHel = 2000 * i;
				health[i].taken = 0;
				health[i].areaHel.x = health[i].xHelRel;
				health[i].areaHel.y = health[i].yHelRel;
				health[i].areaHel.w = health[i].widHel;
				health[i].areaHel.h = health[i].heiHel;
			}
			if (menuStart == 1) {
				gameState = 3;
				menuState = 4;
				menuStart = 0;
			}
			else gameState = 2;
		}
		else if (gameState == 2) {

			// licznik czasu
			t2 = SDL_GetTicks();

			if (elapsedTime == 0) {
				delta = (t2 - t1) * 0.001;
			}
			else if (menuTime > 0) {
				delta = (menuTime - t1) * 0.001;
				menuTime = 0;
			}
			t1 = t2;

			worldTime += delta;
			distance += etiSpeed * delta;
			srand(worldTime); // wykorzysatnie czasu gry do wygenerowania losowego ziarna RNG
			SDL_FillRect(screen, NULL, czarny);


			//DrawSurface(screen, background, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
			SDL_BlitSurface(background, &camera, screen, NULL);

			//system ruchu
			if (movement.up && camera.y > 0) { spaceship.yPlaVel = -playerSpeed; } // poruszanie w gore	
			else if (movement.down && camera.y < AREA_HEIGHT - SCREEN_HEIGHT) { spaceship.yPlaVel = playerSpeed; } // poruszanie w dol
			else { spaceship.yPlaVel = 0; } // brak ruchu w osi Y
			if (movement.left && camera.x > 0) { spaceship.xPlaVel = -playerSpeed; } // poruszanie w lewo
			else if (movement.right && camera.x < AREA_WIDTH - HUDX) { spaceship.xPlaVel = playerSpeed; } // poruszanie w prawo
			else { spaceship.xPlaVel = 0; } // brak ruchu w osi X
			if (movement.fire) {
				if (spaceship.timeReload == 0) {
					addBullet(bul_arr, shotplayer, SaveRectInfo(shotplayer, spaceship.xPlaRel, spaceship.yPlaRel), spaceship.xPlaRel + spaceship.widPla / 2, spaceship.yPlaRel - SaveRectInfo(shotplayer, spaceship.xPlaRel, spaceship.yPlaRel).h, 0, -PLAYER_BULLET_SPEED, 0, 1);
					spaceship.timeReload = 20; // ustawianie czasu przeladowania
				}
			}
			if (dontMoveWhileColides(spaceship, enemy1.areaPla, movement) == 2) { spaceship.xPlaVel = 0; spaceship.health = 0; }
			if (dontMoveWhileColides(spaceship, enemy1.areaPla, movement) == 3) { spaceship.yPlaVel = 0; spaceship.health = 0; }
			if (dontMoveWhileColides(spaceship, enemy1.areaPla, movement) == 4) { spaceship.xPlaVel = 0; spaceship.yPlaVel = 0; spaceship.health = 0; }
			
			// warunki poruszania sie na ekranie
			// xPlaPos
			if (spaceship.xPlaPos + spaceship.xPlaVel > 0 && spaceship.xPlaPos + spaceship.xPlaVel < HUDX - spaceship.widPla) {
				spaceship.xPlaPos += spaceship.xPlaVel; // zmienianie pozycji w osi X
			}
			// yPlaPos
			if (spaceship.yPlaPos + spaceship.yPlaVel > 0 && spaceship.yPlaPos + spaceship.yPlaVel < SCREEN_HEIGHT - spaceship.heiPla) {
				spaceship.yPlaPos += spaceship.yPlaVel; // zmienianie pozycji w osi Y
			}
			// poprawienie relatywnej pozycji o pozycje na ekranie
			spaceship.xPlaRel = camera.x + spaceship.xPlaPos;
			spaceship.yPlaRel = camera.y + spaceship.yPlaPos;

			// granice kamery
			//if(spaceship.xPlaRel>=HUDX/2 && spaceship.xPlaRel<=AREA_WIDTH - SCREEN_WIDTH+HUDX)
			camera.x += spaceship.xPlaVel;
			if (camera.x < 0) { camera.x = 0; }
			else if (camera.x > AREA_WIDTH - HUDX) { camera.x = AREA_WIDTH - HUDX - spaceship.xPlaVel * 2; }
			camera.y += spaceship.yPlaVel;
			if (camera.y < 0) { camera.y = 0; }
			else if (camera.y > AREA_HEIGHT - SCREEN_HEIGHT) { camera.y = AREA_HEIGHT - SCREEN_HEIGHT - spaceship.yPlaVel * 2; }

			for (int i = 0; i < PILLS_AMOUNT; i++) { 
				if (health[i].timeHel <= 0 && health[i].taken == 0) DrawRelativeSurface(screen, heal, health[i].xHelRel, health[i].yHelRel); 
				if (health[i].taken == 0 && collisionChecker(&SaveRectInfo(spaceship.texture, spaceship.xPlaRel, spaceship.yPlaRel), &health[i].areaHel) == 1) {
					health[i].taken = 1;
					if (spaceship.health <= 80) {spaceship.health += 20;}
					else { spaceship.health = 100; }
				}
			}
			// przesuwanie kamery + statku wraz z efektem "trafienia" poprzez migotanie statku
			if (immunity % 10 == 0 || immunity < 10) { 
				if (spaceship.yPlaVel < 0) { DrawSurface(screen, playershipForward, spaceship.xPlaPos, spaceship.yPlaPos); }
				else if (spaceship.yPlaVel > 0) { DrawSurface(screen, playershipBackward, spaceship.xPlaPos, spaceship.yPlaPos); }
				else if (spaceship.xPlaVel > 0) { DrawSurface(screen, playershipLeft, spaceship.xPlaPos, spaceship.yPlaPos); }
				else if (spaceship.xPlaVel < 0) { DrawSurface(screen, playershipRight, spaceship.xPlaPos, spaceship.yPlaPos); }
				else { DrawSurface(screen, playership, spaceship.xPlaPos, spaceship.yPlaPos); }
			}

			if (enemy1.timeReload == 0) {
				addBullet(bul_arr, shot, SaveRectInfo(shot, enemy1.xPlaRel, enemy1.yPlaRel), enemy1.xPlaRel + enemy1.widPla / 2, enemy1.yPlaRel + enemy1.heiPla / 2, 0, -PLAYER_SPEED_MULTIPLIER, 1, 1);
				addBullet(bul_arr, shot, SaveRectInfo(shot, enemy1.xPlaRel, enemy1.yPlaRel), enemy1.xPlaRel + enemy1.widPla / 2, enemy1.yPlaRel + enemy1.heiPla / 2, PLAYER_SPEED_MULTIPLIER, -PLAYER_SPEED_MULTIPLIER, 1, 1);
				addBullet(bul_arr, shot, SaveRectInfo(shot, enemy1.xPlaRel, enemy1.yPlaRel), enemy1.xPlaRel + enemy1.widPla / 2, enemy1.yPlaRel + enemy1.heiPla / 2, -PLAYER_SPEED_MULTIPLIER, -PLAYER_SPEED_MULTIPLIER, 1, 1);
				addBullet(bul_arr, shot, SaveRectInfo(shot, enemy1.xPlaRel, enemy1.yPlaRel), enemy1.xPlaRel + enemy1.widPla / 2, enemy1.yPlaRel + enemy1.heiPla / 2, 0, PLAYER_SPEED_MULTIPLIER, 1, 1);
				addBullet(bul_arr, shot, SaveRectInfo(shot, enemy1.xPlaRel, enemy1.yPlaRel), enemy1.xPlaRel + enemy1.widPla / 2, enemy1.yPlaRel + enemy1.heiPla / 2, PLAYER_SPEED_MULTIPLIER, PLAYER_SPEED_MULTIPLIER, 1, 1);
				addBullet(bul_arr, shot, SaveRectInfo(shot, enemy1.xPlaRel, enemy1.yPlaRel), enemy1.xPlaRel + enemy1.widPla / 2, enemy1.yPlaRel + enemy1.heiPla / 2, -PLAYER_SPEED_MULTIPLIER, PLAYER_SPEED_MULTIPLIER, 1, 1);
				addBullet(bul_arr, shot, SaveRectInfo(shot, enemy1.xPlaRel, enemy1.yPlaRel), enemy1.xPlaRel + enemy1.widPla / 2, enemy1.yPlaRel + enemy1.heiPla / 2, -PLAYER_SPEED_MULTIPLIER, 0, 1, 1);
				addBullet(bul_arr, shot, SaveRectInfo(shot, enemy1.xPlaRel, enemy1.yPlaRel), enemy1.xPlaRel + enemy1.widPla / 2, enemy1.yPlaRel + enemy1.heiPla / 2, PLAYER_SPEED_MULTIPLIER, 0, 1, 1);
				enemy1.timeReload = 60;
			}
			viewBullets(screen, bul_arr);
			DrawRelativeSurface(screen, enemy1.texture, enemy1.xPlaRel, enemy1.yPlaRel);
			// otrzymanie trafienia przez przeciwnika
			if (dealDamage(enemy1, bul_arr) == 1) {
				enemy1.health -= DAMAGE_DEALT;
				enemyHits++;
				if(playerHitsAtTheMoment == playerHits){
					hitsInArow++;
					score += enemyHits * (1000 + hitsInArow *100);
				}
				else {
					playerHitsAtTheMoment = playerHits;
					score += enemyHits * 1000;
				}
			}
			else if (immunity == 0 && dealDamage(spaceship, bul_arr) == 1) {
				spaceship.health -= DAMAGE_DEALT * DAMAGE_MULTIPLIER;
				playerHits++; // liczba trafien gracza
				if (score > playerHits * 100) {
					score -= playerHits * 100;
					hitsInArow = 0;
				}
				else { score = 0; } // zabezpieczenie przed ujemnym wynikiem
				immunity = 100; // okolo dwie sekundy ochorny przed kolejnym otrzymaniem trafienia
			}

			fpsTimer += delta;
			if (fpsTimer > 0.5) {
				fps = frames * 2;
				frames = 0;
				fpsTimer -= 0.5;
			};

			// rysowanie HUD
			DrawRectangle(screen, HUDX, 0, SCREEN_WIDTH-HUDX, SCREEN_HEIGHT, niebieski, niebieski);
			sprintf(text, "Czas trwania = %.1lf s", worldTime);
			if (worldTime < 100.0) { DrawString(screen, HUDX + 10, 10, text, charset); }
			else { DrawString(screen, HUDX + 5, 10, text, charset); }
			// sterowanie
			sprintf(text, "Esc - wyjscie");
			DrawString(screen, HUDX + 30, 26, text, charset);
			sprintf(text, "\030 - gora, \031 - dol");
			DrawString(screen, HUDX + 20, 42, text, charset);
			sprintf(text, "\032 - lewo, \033 - prawo");
			DrawString(screen, HUDX + 5, 58, text, charset);
			sprintf(text, "spacja - strzal");
			DrawString(screen, HUDX + 5, 74, text, charset);
			sprintf(text, "n - nowa gra");
			DrawString(screen, HUDX + 45, SCREEN_HEIGHT - 16, text, charset);
			// placeholdery otrzymanych/trafionych pociskow
			sprintf(text, "Otrzymane pociski: %d", playerHits);
			DrawString(screen, HUDX + strlen(text) / 2, 135, text, charset);
			sprintf(text, "Liczba trafien: %d", enemyHits);
			DrawString(screen, HUDX + strlen(text) / 2, 151, text, charset);
			// pasek zdrowia przeciwnika
			DrawRectangle(screen, HUDX + 10, 170, SCREEN_WIDTH - HUDX - 10 * 2, 16, niebieski, czarny);
			DrawRectangle(screen, HUDX + 10, 170, (SCREEN_WIDTH - HUDX - 10 * 2)* enemy1.health / 100, 16, czarny, czerwony);
			sprintf(text, "przeciwnik");
			DrawString(screen, HUDX + strlen(text)*5, 174, text, charset);
			// wyswietlanie wyniku gracza
			sprintf(text, "wynik: %d", score);
			DrawString(screen, HUDX + (SCREEN_WIDTH - HUDX) / 5, 195, text, charset);
			sprintf(text, "mnoznik punktow: %.1fx", 1+hitsInArow/10);
			DrawString(screen, HUDX + (SCREEN_WIDTH - HUDX) / 18, 210, text, charset);

			// rysowanie pierwszego serduszka
			if (spaceship.health > 20) { DrawSurface(screen, heart, HUDX, 90); }
			else if (spaceship.health > 10) { DrawSurface(screen, heartHalf, HUDX, 90); }
			else { DrawSurface(screen, heartEmpty, HUDX, 90); }
			// rysowanie drugiego serduszka
			if (spaceship.health > 40) { DrawSurface(screen, heart, HUDX + 35, 90); }
			else if (spaceship.health > 30) { DrawSurface(screen, heartHalf, HUDX + 35, 90); }
			else { DrawSurface(screen, heartEmpty, HUDX + 35, 90); }
			// rysowanie trzeciego serduszka
			if (spaceship.health > 60) { DrawSurface(screen, heart, HUDX + 70, 90); }
			else if (spaceship.health > 50) { DrawSurface(screen, heartHalf, HUDX + 70, 90); }
			else { DrawSurface(screen, heartEmpty, HUDX + 70, 90); }
			// rysowanie czwartego serduszka
			if (spaceship.health > 80) { DrawSurface(screen, heart, HUDX + 105, 90); }
			else if (spaceship.health > 70) { DrawSurface(screen, heartHalf, HUDX + 105, 90); }
			else { DrawSurface(screen, heartEmpty, HUDX + 105, 90); }
			// rysowanie piatego serduszka
			if (spaceship.health == 100) { DrawSurface(screen, heart, HUDX + 140, 90); }
			else if (spaceship.health > 90) { DrawSurface(screen, heartHalf, HUDX + 140, 90); }
			else { DrawSurface(screen, heartEmpty, HUDX + 140, 90); }

			// rysowanie oceny
			sprintf(text, "Ocena Twojej gry:");
			DrawString(screen, HUDX + strlen(text), 235, text, charset);
			if (score >= 23500000) { 
				if (shiftingImages == 8) { DrawAnimation(screen, gradeAPlus, ANIMATION_FRAMES_TOTAL, animationFrames, HUDX, 250); }
				else { descendingFrames = 1; DrawAnimation(screen, gradeA, ANIMATION_FRAMES_TOTAL, animationFrames, HUDX, 250); }
			}
			else if (score >= 18500000) { 
				if (shiftingImages == 7) { DrawAnimation(screen, gradeA, ANIMATION_FRAMES_TOTAL, animationFrames, HUDX, 250); }
				else { descendingFrames = 1; DrawAnimation(screen, gradeBPlus, ANIMATION_FRAMES_TOTAL, animationFrames, HUDX, 250); }
			}
			else if (score >= 15000000) { 
				if (shiftingImages == 6) { DrawAnimation(screen, gradeBPlus, ANIMATION_FRAMES_TOTAL, animationFrames, HUDX, 250); }
				else { descendingFrames = 1; DrawAnimation(screen, gradeB, ANIMATION_FRAMES_TOTAL, animationFrames, HUDX, 250); }
			}
			else if (score >= 12500000) { 
				if (shiftingImages == 5) { DrawAnimation(screen, gradeB, ANIMATION_FRAMES_TOTAL, animationFrames, HUDX, 250); }
				else { descendingFrames = 1; DrawAnimation(screen, gradeCPlus, ANIMATION_FRAMES_TOTAL, animationFrames, HUDX, 250); }
			}
			else if (score >= 10000000) { 
				if (shiftingImages == 4) { DrawAnimation(screen, gradeCPlus, ANIMATION_FRAMES_TOTAL, animationFrames, HUDX, 250); }
				else { descendingFrames = 1; DrawAnimation(screen, gradeC, ANIMATION_FRAMES_TOTAL, animationFrames, HUDX, 250); }
			}
			else if (score >= 7500000) {
				if (shiftingImages == 3) { DrawAnimation(screen, gradeC, ANIMATION_FRAMES_TOTAL, animationFrames, HUDX, 250); }
				else { descendingFrames = 1; DrawAnimation(screen, gradeDPlus, ANIMATION_FRAMES_TOTAL, animationFrames, HUDX, 250); }
			}
			else if (score >= 5000000) {
				if (shiftingImages == 2) { DrawAnimation(screen, gradeDPlus, ANIMATION_FRAMES_TOTAL, animationFrames, HUDX, 250); }
				else { descendingFrames = 1; DrawAnimation(screen, gradeD, ANIMATION_FRAMES_TOTAL, animationFrames, HUDX, 250); }
			}
			else if (score >= 2000000) {
				if (shiftingImages == 1) { DrawAnimation(screen, gradeD, ANIMATION_FRAMES_TOTAL, animationFrames, HUDX, 250); }
				else { descendingFrames = 1; DrawAnimation(screen, gradeF, ANIMATION_FRAMES_TOTAL, animationFrames, HUDX, 250); }
			}
			else { DrawAnimation(screen, gradeF, ANIMATION_FRAMES_TOTAL, animationFrames, HUDX, 250); }
			

			SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
			//		SDL_RenderClear(renderer);
			SDL_RenderCopy(renderer, scrtex, NULL, NULL);
			SDL_RenderPresent(renderer);

			// obs³uga zdarzeñ (o ile jakieœ zasz³y) / handling of events (if there were any)
			while (SDL_PollEvent(&event)) {
				switch (event.type) {
				case SDL_KEYDOWN:
					// odczytywanie naciskanych klawiszy
					if (event.key.repeat == 0)
					{
						if (event.key.keysym.scancode == SDL_SCANCODE_UP) { movement.up = 1; }
						else if (event.key.keysym.scancode == SDL_SCANCODE_DOWN) { movement.down = 1; }
						else if (event.key.keysym.scancode == SDL_SCANCODE_LEFT) { movement.left = 1; }
						else if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT) { movement.right = 1; }
						else if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) { movement.fire = 1; }
					}
					if (event.key.keysym.sym == SDLK_ESCAPE) {
						elapsedTime = worldTime;
						menuState = 1;
						gameState = 3;
					}
					else if (event.key.keysym.sym == SDLK_w) etiSpeed = 2.0;
					else if (event.key.keysym.sym == SDLK_s) etiSpeed = 0.3;
					else if (event.key.keysym.sym == SDLK_n) { gameState = 1; }
					break;
				case SDL_KEYUP:
					// odczytywanie puszczanych klawiszy
					if (event.key.repeat == 0)
					{
						if (event.key.keysym.scancode == SDL_SCANCODE_UP) { movement.up = 0; }
						else if (event.key.keysym.scancode == SDL_SCANCODE_DOWN) { movement.down = 0; }
						else if (event.key.keysym.scancode == SDL_SCANCODE_LEFT) { movement.left = 0; }
						else if (event.key.keysym.scancode == SDL_SCANCODE_RIGHT) { movement.right = 0; }
						else if (event.key.keysym.scancode == SDL_SCANCODE_SPACE) { movement.fire = 0; }
					}
					etiSpeed = 1.0;
					break;
				case SDL_QUIT:
					quit = 1;
					break;
				};
			};
			frames++;
			if (descendingFrames == 0 && animationFrames < ANIMATION_FRAMES_TOTAL - 1) {
				animationFrames++;
			}
			else if (descendingFrames == 1 && animationFrames > 0) {
				animationFrames--;
				if (animationFrames == 0) {
					shiftingImages++;
					descendingFrames = 0;
				}
			}
			if (spaceship.timeReload > 0) {
				spaceship.timeReload--;
			}
			if (enemy1.timeReload > 0) {
				enemy1.timeReload--;
			}
			if (immunity > 0) {
				immunity--;
			}
			if (spaceship.health <= 0) {
				gameState = 3;
				menuState = 2;
			}
			if (enemy1.health <= 0) {
				gameState = 3;
				menuState = 3;
			}
			for (int i = 0; i < PILLS_AMOUNT; i++) {
				if (health[i].timeHel > 0) {
					health[i].timeHel--;
				}
			}
			SDL_Delay(1000/60);
		}
		else if (gameState == 3) {
		int animationDelay = 5; // opoznienie ukazania nastepnej klatki animacji
			animationFrames = 0; // liczba wyswietlanej klatki
			menuTime = SDL_GetTicks(); // czas w menu
			int menuQuit = 0; // umozliwia wyjscie z tego stanu gry
			int whichOneSelected = 0; // umozliwia wybor konkretnej opcji w menu
			const int MENULENGHT = 3; // liczba napisow w menu w przypadku wlaczenia menu
			const int OTHERLENGHT = 2; // liczba napisow w menu w przypadku wygranej/przegranej
			int whichLenght; // umozliwia wybor liczby napisow w menu
			if (menuState == 1) { whichLenght = MENULENGHT; }
			else if (menuState >= 2) { whichLenght = OTHERLENGHT; }
			const char* menu[MENULENGHT] = { "Kontynuuj", "Nowa gra", "Wyjdz z gry" };
			const char* other[OTHERLENGHT] = { "Nowa gra", "Wyjdz do menu" };
			const char* state[OTHERLENGHT] = { "Przegrales", "Wygrales"  };
			const char* end = "Wyjdz z gry";
			// kolory
			int zielony = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
			int czerwony = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
			int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
			
			while (!menuQuit) {
				if (menuState == 1) {
					DrawRectangle(screen, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, czarny, czarny);
					DrawString(screen, SCREEN_WIDTH / 2 - strlen(menu[0]) * 4, SCREEN_HEIGHT / 2 - 30, menu[0], charset);// tekst pierwszej opcji
					if (whichOneSelected == 0) {
						DrawLine(screen, SCREEN_WIDTH / 2 - strlen(menu[0]) * 4, SCREEN_HEIGHT / 2 - 15, strlen(menu[0]) * 8, 1, 0, czerwony);// rysowanie prostej pod pierwszym napisem
					}
					DrawString(screen, SCREEN_WIDTH / 2 - strlen(menu[1]) * 4, SCREEN_HEIGHT / 2, menu[1], charset);// teskt drugiej opcji
					if (whichOneSelected == 1) {
						DrawLine(screen, SCREEN_WIDTH / 2 - strlen(menu[1]) * 4, SCREEN_HEIGHT / 2 + 15, strlen(menu[1]) * 8, 1, 0, czerwony);// rysowanie prostej pod drugim napisem
					}
					DrawString(screen, SCREEN_WIDTH / 2 - strlen(menu[2]) * 4, SCREEN_HEIGHT / 2 + 30, menu[2], charset);// teskt drugiej opcji
					if (whichOneSelected == 2) {
						DrawLine(screen, SCREEN_WIDTH / 2 - strlen(menu[2]) * 4, SCREEN_HEIGHT / 2 + 45, strlen(menu[2]) * 8, 1, 0, czerwony);// rysowanie prostej pod trzecim napisem
					}
				}
				else if (menuState >=2) {
					if (menuState == 2) {
						DrawRectangle(screen, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, czerwony, czerwony);
						DrawString(screen, SCREEN_WIDTH / 2 - strlen(state[0]) * 4, 50, state[0], charset);
					}
					else if (menuState == 3) {
						DrawRectangle(screen, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, zielony, zielony);
						DrawString(screen, SCREEN_WIDTH / 2 - strlen(state[1]) * 4, 50, state[1], charset);
					}
					else if (menuState == 4) {
						DrawRectangle(screen, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, czarny, czarny);
						DrawAnimation(screen, title, 5, animationFrames, 0, 50);
					}
					if (menuState == 2 || menuState == 3) {
					DrawString(screen, SCREEN_WIDTH / 2 - strlen(other[0]) * 4, SCREEN_HEIGHT / 2 - 30, other[0], charset);// tekst pierwszej opcji
					if (whichOneSelected == 0) {
						DrawLine(screen, SCREEN_WIDTH / 2 - strlen(other[0]) * 4, SCREEN_HEIGHT / 2 - 15, strlen(other[0]) * 8, 1, 0, niebieski);// rysowanie prostej pod pierwszym napisem
					}
					
						DrawString(screen, SCREEN_WIDTH / 2 - strlen(other[1]) * 4, SCREEN_HEIGHT / 2, other[1], charset);// teskt drugiej opcji
						if (whichOneSelected == 1) {
							DrawLine(screen, SCREEN_WIDTH / 2 - strlen(other[1]) * 4, SCREEN_HEIGHT / 2 + 15, strlen(other[1]) * 8, 1, 0, niebieski);// rysowanie prostej pod drugim napisem
						}
						sprintf(text, "Twoj wynik koncowy:");
						DrawString(screen, SCREEN_WIDTH / 2 - strlen(text) * 4, 275, text, charset);
						sprintf(text, "wynik: %d", score);
						DrawString(screen, SCREEN_WIDTH / 2 - strlen(text) * 4, 290, text, charset);
						if (score >= 23500000) { DrawAnimation(screen, gradeAPlus, ANIMATION_FRAMES_TOTAL, ANIMATION_FRAMES_TOTAL - 1, SCREEN_WIDTH / 2 - (gradeAPlus->w / ANIMATION_FRAMES_TOTAL) / 2, 300); }
						else if (score >= 18500000) { DrawAnimation(screen, gradeA, ANIMATION_FRAMES_TOTAL, ANIMATION_FRAMES_TOTAL - 1, SCREEN_WIDTH / 2 - (gradeA->w / ANIMATION_FRAMES_TOTAL) / 2, 300); }
						else if (score >= 15000000) { DrawAnimation(screen, gradeBPlus, ANIMATION_FRAMES_TOTAL, ANIMATION_FRAMES_TOTAL - 1, SCREEN_WIDTH / 2 - (gradeBPlus->w / ANIMATION_FRAMES_TOTAL) / 2, 300); }
						else if (score >= 12500000) { DrawAnimation(screen, gradeB, ANIMATION_FRAMES_TOTAL, ANIMATION_FRAMES_TOTAL - 1, SCREEN_WIDTH / 2 - (gradeB->w / ANIMATION_FRAMES_TOTAL) / 2, 300); }
						else if (score >= 10000000) { DrawAnimation(screen, gradeCPlus, ANIMATION_FRAMES_TOTAL, ANIMATION_FRAMES_TOTAL - 1, SCREEN_WIDTH / 2 - (gradeCPlus->w / ANIMATION_FRAMES_TOTAL) / 2, 300); }
						else if (score >= 7500000) { DrawAnimation(screen, gradeC, ANIMATION_FRAMES_TOTAL, ANIMATION_FRAMES_TOTAL - 1, SCREEN_WIDTH / 2 - (gradeC->w / ANIMATION_FRAMES_TOTAL) / 2, 300); }
						else if (score >= 5000000) { DrawAnimation(screen, gradeDPlus, ANIMATION_FRAMES_TOTAL, ANIMATION_FRAMES_TOTAL - 1, SCREEN_WIDTH / 2 - (gradeDPlus->w / ANIMATION_FRAMES_TOTAL) / 2, 300); }
						else if (score >= 2000000) { DrawAnimation(screen, gradeD, ANIMATION_FRAMES_TOTAL, ANIMATION_FRAMES_TOTAL - 1, SCREEN_WIDTH / 2 - (gradeD->w / ANIMATION_FRAMES_TOTAL) / 2, 300); }
						else { DrawAnimation(screen, gradeF, ANIMATION_FRAMES_TOTAL, ANIMATION_FRAMES_TOTAL-1, SCREEN_WIDTH / 2 - (gradeF->w / ANIMATION_FRAMES_TOTAL)/2, 300); }
					}
					else {
						DrawString(screen, SCREEN_WIDTH / 2 - strlen(other[0]) * 4, SCREEN_HEIGHT / 2 - 30, other[0], charset);// tekst pierwszej opcji
						if (whichOneSelected == 0) {
							DrawLine(screen, SCREEN_WIDTH / 2 - strlen(other[0]) * 4, SCREEN_HEIGHT / 2 - 15, strlen(other[0]) * 8, 1, 0, czerwony);// rysowanie prostej pod pierwszym napisem
						}
						DrawString(screen, SCREEN_WIDTH / 2 - strlen(end) * 4, SCREEN_HEIGHT / 2, end, charset);// teskt drugiej opcji
						if (whichOneSelected == 1) {
							DrawLine(screen, SCREEN_WIDTH / 2 - strlen(end) * 4, SCREEN_HEIGHT / 2 + 15, strlen(end) * 8, 1, 0, czerwony);// rysowanie prostej pod drugim napisem
						}
					}

				}
				SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
				//		SDL_RenderClear(renderer);
				SDL_RenderCopy(renderer, scrtex, NULL, NULL);
				SDL_RenderPresent(renderer);	
				while (SDL_PollEvent(&event)) {
					switch (event.type) {
					case SDL_KEYDOWN:
						// odczytywanie naciskanych klawiszy
						if (event.key.keysym.sym == SDLK_ESCAPE) { 
							worldTime = elapsedTime;
							movement = { 0,0,0,0 }; 
							spaceship.xPlaVel = 0;
							spaceship.yPlaVel = 0;
							if (menuState == 1) {
								gameState = 2;
							}
							else { quit = 1; }
							menuQuit = 1;
						}
						else if (event.key.keysym.sym == SDLK_DOWN) if (whichOneSelected < whichLenght -1) whichOneSelected++; else whichOneSelected = 0;
						else if (event.key.keysym.sym == SDLK_UP) if (whichOneSelected > 0) whichOneSelected--; else whichOneSelected = whichLenght - 1;
						else if (event.key.keysym.sym == SDLK_SPACE) {
							movement = { 0,0,0,0 };
							spaceship.xPlaVel = 0;
							spaceship.yPlaVel = 0;
							if (menuState == 1) {
								if (whichOneSelected == 0) { gameState = 2; } // powrot do gry
								else if (whichOneSelected == 1) { gameState = 1; } // nowa gra
								else if (whichOneSelected == 2) { gameState = 2; ; quit = 1; } // wyjscie z gry
								menuQuit = 1;
							}
							else if (menuState == 2 || menuState == 3) {
								if (whichOneSelected == 0) { gameState = 1;} // nowa gra
								else if (whichOneSelected == 1) { menuState = 4; } // wyjdz z gry
								menuQuit = 1;
							}
							else if (menuState == 4) {
								if (whichOneSelected == 0) { gameState = 1; } // nowa gra
								else if (whichOneSelected == 1) { gameState = 2; quit = 1; } // wyjdz z gry
								menuQuit = 1;
							}
						}
						break;
					case SDL_KEYUP:
						if (event.key.keysym.sym == SDLK_ESCAPE);
						else if (event.key.keysym.sym == SDLK_UP);
						else if (event.key.keysym.sym == SDLK_DOWN);
						break;
					case SDL_QUIT:
						menuQuit = 1;
						quit = 1;
						break;
					};
					worldTime = elapsedTime;

				};
				if (animationDelay > 0) {
					animationDelay--;
				}
				if (animationDelay == 0 && animationFrames < 5) {
					animationFrames++;
					animationDelay = 5;
					if (animationFrames == 5) { animationFrames = 0; }
				}
			}
		}
		
	};
	// zwolnienie pamieci wykorzystanej na pociski
	if (bul_arr != NULL) {
		free(bul_arr->bb);
	}
	// zwolnienie powierzchni / freeing all surfaces
	SDL_FreeSurface(charset);
	SDL_FreeSurface(screen);
	SDL_DestroyTexture(scrtex);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();
	return 0;
	};
