#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<string.h>
#include <SDL2/SDL.h>


/*extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}*/

#define TRUE 1
#define FALSE 0
#define RIGHT 1
#define LEFT 0

#define SCREEN_WIDTH    640
#define SCREEN_HEIGHT    480

#define PLATFORMA_WIDTH 80
#define PLATFORMA_HEIGHT 20

#define DRABINA_WIDTH 20
#define DRABINA_HEIGHT 60

#define BMP_SIZE 32

#define PLAYER_SPEED 250.0
#define JUMP_SPEED 500.0
#define BARREL_SPEEDX 100.0
#define BARREL_SPEEDY 100.0
#define GRAVITY 15.0

#define MAX_BARRELS 10
#define MAX_CHERRY 2


typedef struct {
    int x;
    int y;
} drabina;

typedef struct {
    int x;
    int y;
} platforma;

typedef struct {
    int x;
    int y;
    int active;
} cherry;

typedef struct {
    double x;
    double y;
    double velocityX;
    double velocityY;
    
    int active;
    int side;
} barrel;

// Define the gra structure
typedef struct {
    double yPlayer;
    double xPlayer;
    int goodY, goodX, badY, badX;
    
    double startingXBarrels;
    double startingYBarrels;
    int lives;
    int pkt;
    int currentFloor;
    int maxFloors;
    int poziom;
    
    int numDrabin;
    drabina *drabiny; 
    
    int numPlatform;
    platforma *platformy;
    
    barrel barrels[MAX_BARRELS];
    cherry cherrys[MAX_CHERRY];
} gra;

typedef struct {
    char pseudonim[20];
    int wynik;
} wynikGracza;

typedef struct {
    int iloscWynikow;
    wynikGracza wyniki[100];
} ranking;


void DrawString(SDL_Surface *screen, int x, int y, const char *text, SDL_Surface *charset) {
    int px, py, c;
    SDL_Rect s, d;
    s.w = 8;
    s.h = 8;
    d.w = 8;
    d.h = 8;
    while(*text) {
        c = *text & 255;
        px = (c % 16) * 8;
        py = (c / 16) * 8;
        s.x = px;
        s.y = py;
        d.x = x;
        d.y = y;
        SDL_BlitSurface(charset, &s, screen, &d);
        x += 8;
        text++;
        };
    };

void DrawSurface(SDL_Surface *screen, SDL_Surface *sprite, int x, int y) {
    SDL_Rect dest;
    dest.x = x - sprite->w / 2;
    dest.y = y - sprite->h / 2;
    dest.w = sprite->w;
    dest.h = sprite->h;
    SDL_BlitSurface(sprite, NULL, screen, &dest);
};

void DrawPixel(SDL_Surface *surface, int x, int y, Uint32 color) {
    int bpp = surface->format->BytesPerPixel;
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
    *(Uint32 *)p = color;
    };

void DrawLine(SDL_Surface *screen, int x, int y, int l, int dx, int dy, Uint32 color) {
    for(int i = 0; i < l; i++) {
        DrawPixel(screen, x, y, color);
        x += dx;
        y += dy;
        };
    };

void DrawRectangle(SDL_Surface *screen, int x, int y, int l, int k, Uint32 outlineColor, Uint32 fillColor) {
    int i;
    DrawLine(screen, x, y, k, 0, 1, outlineColor);
    DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
    DrawLine(screen, x, y, l, 1, 0, outlineColor);
    DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
    for(i = y + 1; i < y + k - 1; i++)
        DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
    };

int checkPlatform (gra *gra, int x, int y){
    for (int j = 0; j< gra->numPlatform; j++){
        if (x >= gra->platformy[j].x - PLATFORMA_WIDTH/2 &&
            x <= gra->platformy[j].x + PLATFORMA_WIDTH/2 &&
            y == gra->platformy[j].y) return TRUE;
    }
    return FALSE;
}

void correctPlatform (gra *gra){
    for (int j = 0; j< gra->numPlatform; j++){
        if (gra->xPlayer >= gra->platformy[j].x - PLATFORMA_WIDTH/2 &&
            gra->xPlayer <= gra->platformy[j].x + PLATFORMA_WIDTH/2 &&
            gra->yPlayer + BMP_SIZE/2 > gra->platformy[j].y &&
            gra->yPlayer + BMP_SIZE/2 < gra->platformy[j].y + PLATFORMA_HEIGHT){
            gra->yPlayer = gra->platformy[j].y - BMP_SIZE/2;
        }
    }
}

void correctPlatformBarrel (gra *gra){
    for (int j = 0; j< gra->numPlatform; j++){
        for (int i = 0; i< MAX_BARRELS ; i++){
            if (gra->barrels[i].x >= gra->platformy[j].x - PLATFORMA_WIDTH/2 &&
                gra->barrels[i].x <= gra->platformy[j].x + PLATFORMA_WIDTH/2 &&
                gra->barrels[i].y + BMP_SIZE/2 >= gra->platformy[j].y - 4 &&
                gra->barrels[i].y + BMP_SIZE/2 <= gra->platformy[j].y){
                gra->barrels[i].y = gra->platformy[j].y - BMP_SIZE/2;
            }
        }
    }
}

int barrelsDown(gra *gra, int barrelNum){
    for (int i = 0; i < gra->numDrabin; i++){
        if (gra->barrels[barrelNum].x > gra->drabiny[i].x - 1 && gra->barrels[barrelNum].x < gra->drabiny[i].x + 1 &&
            gra->barrels[barrelNum].y + BMP_SIZE/2 >= gra->drabiny[i].y - DRABINA_HEIGHT/2 - PLATFORMA_HEIGHT/2 &&
            gra->barrels[barrelNum].y + BMP_SIZE/2 < gra->drabiny[i].y + DRABINA_HEIGHT/2){
            return TRUE;
        }
    }
    return FALSE;
}

void loadBarrel(gra *gra, int num){
    gra->barrels[num].x = gra->startingXBarrels;
    gra->barrels[num].y = gra->startingYBarrels;
    gra->barrels[num].active = TRUE;
    if (gra->poziom == 1) gra->barrels[num].velocityX = BARREL_SPEEDX;
    else gra->barrels[num].velocityX = -BARREL_SPEEDX;
    gra->barrels[num].velocityX = BARREL_SPEEDY;
    if(gra->poziom != 2) gra->barrels[num].side = RIGHT;
    else gra->barrels[num].side = LEFT;
}

int wGore(gra *gra) {
    for (int i = 0; i < gra->numDrabin; i++){
        if (gra->xPlayer < gra->drabiny[i].x + DRABINA_WIDTH/2 && 
            gra->xPlayer > gra->drabiny[i].x - DRABINA_WIDTH/2 &&
            gra->yPlayer + BMP_SIZE/2 >= gra->drabiny[i].y - DRABINA_HEIGHT/2 - PLATFORMA_HEIGHT/2 &&
            gra->yPlayer + BMP_SIZE/2 <= gra->drabiny[i].y + DRABINA_HEIGHT/2)
        {
            return TRUE;
            /*for (int j = 0; j< gra->numPlatform; j++){
                if (gra->platformy[j].x == gra->drabiny[i].x && gra->platformy[j].y + PLATFORMA_HEIGHT/2 == gra->drabiny[i].y - DRABINA_HEIGHT/2){
                    if (gra->yPlayer + PLAYER_SIZE/2 > gra->platformy[j].y) return TRUE;
                }
            }*/
        }
    }
    return FALSE;
}

int wDol(gra *gra) {
    for (int i = 0; i < gra->numDrabin; i++){
        if (gra->xPlayer < gra->drabiny[i].x + DRABINA_WIDTH/2 &&
            gra->xPlayer > gra->drabiny[i].x - DRABINA_WIDTH/2 &&
            gra->yPlayer + BMP_SIZE/2 >= gra->drabiny[i].y - DRABINA_HEIGHT/2 - PLATFORMA_HEIGHT/2 &&
            gra->yPlayer + BMP_SIZE/2 <= gra->drabiny[i].y + DRABINA_HEIGHT/2)
        {
            return TRUE;
            /*for (int j = 0; j< gra->numPlatform; j++){
                if (gra->platformy[j].x == gra->drabiny[i].x && gra->platformy[j].y == gra->drabiny[i].y + DRABINA_HEIGHT/2){
                    if (gra->yPlayer + PLAYER_SIZE/2 < gra->platformy[j].y) return TRUE;
                }
            }*/
        }
    }
    return FALSE;
}

void poziomDraw(SDL_Window *window, SDL_Renderer *renderer, SDL_Surface *screen, SDL_Surface *charset, SDL_Texture *scrtex, int *game, gra *gra) {
    SDL_Surface *platformaBmp, *drabinaBmp, *heartBmp, *cherryBmp, *goodBmp, *badBmp;
    
    drabinaBmp = SDL_LoadBMP("./drabina.bmp");
    platformaBmp = SDL_LoadBMP("./platforma.bmp");
    heartBmp = SDL_LoadBMP("./heart.bmp");
    cherryBmp = SDL_LoadBMP("./cherry.bmp");
    goodBmp = SDL_LoadBMP("./good.bmp");
    badBmp = SDL_LoadBMP("./bad.bmp");
    if(platformaBmp == NULL || drabinaBmp == NULL || heartBmp == NULL || cherryBmp == NULL) {
        printf("SDL_LoadBMP(platforma.bmp) error: %s\n", SDL_GetError());
        SDL_FreeSurface(charset);
        SDL_FreeSurface(screen);
        SDL_DestroyTexture(scrtex);
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        SDL_Quit();
        *game = FALSE;
    };
    

    for (int i = 0; i< gra->lives; i++){
        DrawSurface(screen, heartBmp, 20 + i * 40, 20);
    }
    for (int i = 0; i< gra->numPlatform; i++){
        DrawSurface(screen, platformaBmp, gra->platformy[i].x, gra->platformy[i].y + PLATFORMA_HEIGHT/2);
    }
    for (int i = 0; i< gra->numDrabin; i++){
        DrawSurface(screen, drabinaBmp, gra->drabiny[i].x, gra->drabiny[i].y);
    }
    for (int i = 0; i< MAX_CHERRY; i++){
        if (gra->cherrys[i].active){
            DrawSurface(screen, cherryBmp, gra->cherrys[i].x, gra->cherrys[i].y);
        }
    }
    DrawSurface(screen, goodBmp, gra->goodX, gra->goodY);
    DrawSurface(screen, badBmp, gra->badX, gra->badY);
    
    SDL_FreeSurface(platformaBmp);
    SDL_FreeSurface(drabinaBmp);
    SDL_FreeSurface(heartBmp);
}

void checkCherry(gra *gra){
    for (int i = 0; i < MAX_CHERRY; i++){
        if (gra->cherrys[i].active && gra->xPlayer >= gra->cherrys[i].x - BMP_SIZE/2 && gra->xPlayer <= gra->cherrys[i].x + BMP_SIZE/2 &&
            gra->yPlayer == gra->cherrys[i].y){
            gra->pkt += 600;
            gra->cherrys[i].active = FALSE;
        }
    }
}

void checkBarrels(gra *gra, int num){
    if (gra->barrels[num].active && 
        gra->xPlayer > gra->barrels[num].x - BMP_SIZE/2 &&
        gra->xPlayer < gra->barrels[num].x + BMP_SIZE/2 &&
        gra->yPlayer <= gra->barrels[num].y + BMP_SIZE/2 &&
        gra->yPlayer >= gra->barrels[num].y - BMP_SIZE/2){
        gra->barrels[num].active = FALSE;
        gra->lives--;
    }
}

void zapiszStan (gra *gra){
    char plikN[6] = "1.bin";
    FILE *plik = fopen(plikN, "wb");

    if (plik != NULL) {
        fwrite(gra, sizeof(*gra), 1, plik);
        fclose(plik);
    }
}

void wczytajStan (gra *gra){
    char plikN[6] = "1.bin";

    FILE *plik = fopen(plikN, "rb");

    if (plik != NULL) {
        fread(gra, sizeof(*gra), 1, plik);
        fclose(plik);
    }
}

void zapiszWynik(SDL_Window *window, SDL_Renderer *renderer, SDL_Surface *screen, SDL_Surface *charset, SDL_Texture *scrtex, gra *gra){
    wynikGracza wynik;
    wynik.wynik = gra->pkt;
    
    int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
    SDL_Event e;
    int inputCompleted = FALSE;
    int cursorPos = 0;
    char text[120], zmiennaText;
    wynik.pseudonim[0] = '\0';
    
    while (!inputCompleted) {
        SDL_FillRect(screen, NULL, czarny);
        snprintf(text, sizeof(text), "Wpisz swoj pseudonim");
        DrawString(screen, SCREEN_WIDTH/2 - sizeof(text), 100, text, charset);
        
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                SDL_Quit();
                exit(0);
            }
            else if (e.type == SDL_KEYDOWN) {
                if (e.key.keysym.sym == SDLK_RETURN) {
                    inputCompleted = true;
                }
                else if (e.key.keysym.sym == SDLK_BACKSPACE) {
                    if (cursorPos > 0) {
                        cursorPos--;
                        wynik.pseudonim[cursorPos] = '\0';
                    }
                }
                else if (e.key.keysym.sym == SDLK_DELETE) {
                    if (cursorPos > 0) {
                        cursorPos--;
                        wynik.pseudonim[cursorPos] = '\0';
                    }
                }
            }
            else if (e.type == SDL_TEXTINPUT) {
                // Handle text input events
                if (cursorPos < sizeof(wynik.pseudonim) - 1) {
                    strcat(wynik.pseudonim, e.text.text);
                    cursorPos += strlen(e.text.text);
                }
            }
        }

        snprintf(text,sizeof(text), "%s", wynik.pseudonim);
        DrawString(screen, SCREEN_WIDTH/2 - strlen(wynik.pseudonim), SCREEN_HEIGHT/2, text, charset);
        
        SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
        SDL_RenderCopy(renderer, scrtex, NULL, NULL);
        SDL_RenderPresent(renderer);
    }
    
    FILE *plik = fopen("wyniki.txt", "a");

    if (plik != NULL) {
            fprintf(plik, "%s %d\n", wynik.pseudonim, wynik.wynik);
            fclose(plik);
    }
}

void rankingFunction(SDL_Window *window, SDL_Renderer *renderer, SDL_Surface *screen, SDL_Surface *charset, SDL_Texture *scrtex) {
    int rankingBool = TRUE, zmienna = 0;
    char text[128];
    SDL_Event event;
    int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
    ranking ranking;
    
    FILE *plik = fopen("wyniki.txt", "r");

    if (plik != NULL) {
        int wyniki[100]; // Zakładamy, że może być maksymalnie 100 wyników
        int iloscWynikow = 0;
        
        while (fscanf(plik, "%s %d", ranking.wyniki[iloscWynikow].pseudonim, &ranking.wyniki[iloscWynikow].wynik) == 2) {
            iloscWynikow++;
        }
        
        // Sortowanie wyników
        for (int i = 0; i < iloscWynikow - 1; i++) {
            for (int j = 0; j < iloscWynikow - i - 1; j++) {
                if (ranking.wyniki[j].wynik < ranking.wyniki[j+1].wynik) {
                    wynikGracza temp = ranking.wyniki[j];
                    ranking.wyniki[j] = ranking.wyniki[j+1];
                    ranking.wyniki[j+1] = temp;
                }
            }
        }
        
        fclose(plik);
        
        while (rankingBool) {
            SDL_FillRect(screen, NULL, czarny);
            
            snprintf(text, sizeof(text),"RANKING");
            DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 20, text, charset);
            
            int limitWynikowNaEkranie = 6; // liczba wyników do wyświetlenia
            for (int i = 0 + zmienna; i < iloscWynikow && i < limitWynikowNaEkranie + zmienna; i++) {
                snprintf(text, sizeof(text),"%s %d", ranking.wyniki[i].pseudonim, ranking.wyniki[i].wynik);
                DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 100 + 40*( i - zmienna), text, charset);
            }
            
            SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
            SDL_RenderCopy(renderer, scrtex, NULL, NULL);
            SDL_RenderPresent(renderer);
            
            
            while(SDL_PollEvent(&event)) {
                switch(event.type) {
                    case SDL_KEYDOWN:
                        if (event.key.keysym.sym == SDLK_LEFT){
                            if (zmienna> 5 ) zmienna -= limitWynikowNaEkranie;
                        }
                        else if (event.key.keysym.sym == SDLK_RIGHT){
                            if (zmienna + limitWynikowNaEkranie < iloscWynikow ) zmienna += limitWynikowNaEkranie;
                        }
                        else if (event.key.keysym.sym == SDLK_ESCAPE){
                            rankingBool = FALSE;
                        }
                        break;
                    case SDL_QUIT:
                        rankingBool = FALSE;
                        break;
                };
            }
        }
    }
}

void koniec(SDL_Window *window, SDL_Renderer *renderer, SDL_Surface *screen, SDL_Surface *charset, SDL_Texture *scrtex, gra *gra, int *poziomInit){
    int yInfo, xInfo;
    SDL_Event event;
    
    char text[128];
    int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
    int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);

    yInfo = SCREEN_HEIGHT/2 - 24;
    xInfo = SCREEN_WIDTH/2 - SCREEN_WIDTH/4;
    
    int koniecInfo = TRUE;
    
    while(koniecInfo){
        SDL_FillRect(screen, NULL, czarny);
        
        DrawRectangle(screen, xInfo, yInfo, SCREEN_WIDTH/2, 18, niebieski, niebieski);
        
        snprintf(text,sizeof(text), "Zapisz gre");
        DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, SCREEN_HEIGHT/2 - 18, text, charset);
        
        snprintf(text,sizeof(text), "Wyjdz");
        DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, SCREEN_HEIGHT/2 + 12, text, charset);

        SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
        SDL_RenderCopy(renderer, scrtex, NULL, NULL);
        SDL_RenderPresent(renderer);
        
        while(SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_DOWN){
                        if (yInfo != SCREEN_HEIGHT/2 + 6) yInfo += 30;
                        else yInfo = SCREEN_HEIGHT/2 - 24;
                    }
                    else if (event.key.keysym.sym == SDLK_UP){
                        if (yInfo != SCREEN_HEIGHT/2 - 24)yInfo -= 30;
                        else yInfo = SCREEN_HEIGHT/2 + 6;
                    }
                    else if (event.key.keysym.sym == SDLK_RETURN){
                        switch (yInfo) {
                            case SCREEN_HEIGHT/2 - 24:
                                zapiszWynik(window, renderer, screen, charset, scrtex, gra);
                                koniecInfo = FALSE;
                                *poziomInit = FALSE;
                                break;
                            default:
                                koniecInfo = FALSE;
                                *poziomInit = FALSE;
                                break;
                        }
                    }
                    break;
                case SDL_QUIT:
                    koniecInfo = FALSE;
                    *poziomInit = FALSE;
                    break;
            };
        }
    }
}

void przerwa(SDL_Window *window, SDL_Renderer *renderer, SDL_Surface *screen, SDL_Surface *charset, SDL_Texture *scrtex, gra *gra, int *poziomInit){
    int yInfo, xInfo;
    SDL_Event event;
    
    char text[128];
    int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
    int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);

    yInfo = SCREEN_HEIGHT/2 - 24;
    xInfo = SCREEN_WIDTH/2 - SCREEN_WIDTH/4;
    
    int przerwaInfo = TRUE;
    
    while(przerwaInfo){
        SDL_FillRect(screen, NULL, czarny);
        
        DrawRectangle(screen, xInfo, yInfo, SCREEN_WIDTH/2, 18, niebieski, niebieski);
        
        snprintf(text,sizeof(text), "Zapisz stan Gry");
        DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, SCREEN_HEIGHT/2 - 18, text, charset);
        
        snprintf(text,sizeof(text), "Wyjdz");
        DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, SCREEN_HEIGHT/2 + 12, text, charset);

        SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
        SDL_RenderCopy(renderer, scrtex, NULL, NULL);
        SDL_RenderPresent(renderer);
        
        while(SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_DOWN){
                        if (yInfo != SCREEN_HEIGHT/2 + 6) yInfo += 30;
                        else yInfo = SCREEN_HEIGHT/2 - 24;
                    }
                    else if (event.key.keysym.sym == SDLK_UP){
                        if (yInfo != SCREEN_HEIGHT/2 - 24)yInfo -= 30;
                        else yInfo = SCREEN_HEIGHT/2 + 6;
                    }
                    else if (event.key.keysym.sym == SDLK_ESCAPE){
                        przerwaInfo = FALSE;
                    }
                    else if (event.key.keysym.sym == SDLK_RETURN){
                        switch (yInfo) {
                            case SCREEN_HEIGHT/2 - 24:
                                zapiszStan(gra);
                                przerwaInfo = FALSE;
                                *poziomInit = FALSE;
                                break;
                            default:
                                przerwaInfo = FALSE;
                                *poziomInit = FALSE;
                                break;
                        }
                    }
                    break;
                case SDL_QUIT:
                    przerwaInfo = FALSE;
                    *poziomInit = FALSE;
                    break;
            };
        }
    }
}

void strataZycia(SDL_Window *window, SDL_Renderer *renderer, SDL_Surface *screen, SDL_Surface *charset, SDL_Texture *scrtex, gra *gra, int *poziomInit, int *game){
    int yInfo, xInfo;
    SDL_Event event;
    
    char text[128];
    int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
    int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);

    yInfo = SCREEN_HEIGHT/2 - 24;
    xInfo = SCREEN_WIDTH/2 - SCREEN_WIDTH/4;
    
    int zycInfo = TRUE;
    
    while(zycInfo){
        SDL_FillRect(screen, NULL, czarny);
        
        DrawRectangle(screen, xInfo, yInfo, SCREEN_WIDTH/2, 18, niebieski, niebieski);
        
        snprintf(text,sizeof(text), "Kontynuuj");
        DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, SCREEN_HEIGHT/2 - 18, text, charset);
        
        snprintf(text,sizeof(text), "Zakoncz");
        DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, SCREEN_HEIGHT/2 + 12, text, charset);

        SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
        SDL_RenderCopy(renderer, scrtex, NULL, NULL);
        SDL_RenderPresent(renderer);
        
        while(SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_DOWN){
                        if (yInfo != SCREEN_HEIGHT/2 + 6) yInfo += 30;
                        else yInfo = SCREEN_HEIGHT/2 - 24;
                    }
                    else if (event.key.keysym.sym == SDLK_UP){
                        if (yInfo != SCREEN_HEIGHT/2 - 24)yInfo -= 30;
                        else yInfo = SCREEN_HEIGHT/2 + 6;
                    }
                    else if (event.key.keysym.sym == SDLK_RETURN){
                        switch (yInfo) {
                            case SCREEN_HEIGHT/2 - 24:
                                zycInfo = FALSE;
                                break;
                            default:
                                koniec(window, renderer, screen, charset, scrtex, gra, poziomInit);
                                zycInfo = FALSE;
                                *game = FALSE;
                                break;
                        }
                    }
                    break;
                case SDL_QUIT:
                    zycInfo = FALSE;
                    *poziomInit = FALSE;
                    break;
            };
        }
    }
}

void wygrana(SDL_Window *window, SDL_Renderer *renderer, SDL_Surface *screen, SDL_Surface *charset, SDL_Texture *scrtex, gra *gra, int *poziomInit){
    int yInfo, xInfo;
    SDL_Event event;
    
    char text[128];
    int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
    int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);

    yInfo = SCREEN_HEIGHT/2 - 24;
    xInfo = SCREEN_WIDTH/2 - SCREEN_WIDTH/4;
    
    int wygranaInfo = TRUE;
    
    while(wygranaInfo){
        SDL_FillRect(screen, NULL, czarny);
        
        DrawRectangle(screen, xInfo, yInfo, SCREEN_WIDTH/2, 18, niebieski, niebieski);
        
        snprintf(text,sizeof(text), "Uzyskales %d punktow", gra->pkt);
        DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, SCREEN_HEIGHT/2 - 60, text, charset);
        
        snprintf(text,sizeof(text), "Zapisz gre");
        DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, SCREEN_HEIGHT/2 - 18, text, charset);
        
        snprintf(text,sizeof(text), "Przejdz do nastepnego poziomu");
        DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, SCREEN_HEIGHT/2 + 12, text, charset);
        
        snprintf(text,sizeof(text), "wyjdz");
        DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, SCREEN_HEIGHT/2 + 42, text, charset);

        SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
        //SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, scrtex, NULL, NULL);
        SDL_RenderPresent(renderer);
        
        while(SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_KEYDOWN:
                    if (event.key.keysym.sym == SDLK_DOWN){
                        if (yInfo != SCREEN_HEIGHT/2 + 36) yInfo += 30;
                        else yInfo = SCREEN_HEIGHT/2 - 24;
                    }
                    else if (event.key.keysym.sym == SDLK_UP){
                        if (yInfo != SCREEN_HEIGHT/2 - 24)yInfo -= 30;
                        else yInfo = SCREEN_HEIGHT/2 + 36;
                    }
                    else if (event.key.keysym.sym == SDLK_RETURN){
                        switch (yInfo) {
                            case SCREEN_HEIGHT/2 - 24:
                                zapiszWynik(window, renderer, screen, charset, scrtex, gra);
                                wygranaInfo = FALSE;
                                *poziomInit = FALSE;
                                break;
                            case SCREEN_HEIGHT/2 + 6:
                                if (gra->poziom<3){
                                    gra->poziom++;
                                    wygranaInfo = FALSE;
                                }
                                else koniec(window, renderer, screen, charset, scrtex, gra, poziomInit);
                                break;
                            case SCREEN_HEIGHT/2 + 36:
                                wygranaInfo = FALSE;
                                *poziomInit = FALSE;
                                break;
                            default:
                                wygranaInfo = FALSE;
                                break;
                        }
                    }
                    break;
                case SDL_QUIT:
                    wygranaInfo = FALSE;
                    break;
            };
        }
    }
}

int infoZPliku(const char* filename, gra* gra) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file for reading");
        return 0; 
    }

    char objectType[10];
    int index;
    double x, y;

    while (fscanf(file, "%s %d: %lf %lf", objectType, &index, &x, &y) == 4) {
        if (strcmp(objectType, "Platform") == 0) {
            if (index <= gra->numPlatform) {
                gra->platformy[index-1].x = x;
                gra->platformy[index-1].y = y;
            }
        } else if (strcmp(objectType, "Ladder") == 0) {
            if (index <= gra->numDrabin) {
                gra->drabiny[index-1].x = x;
                gra->drabiny[index-1].y = y;
            }
        }
        else if (strcmp(objectType, "Cherry") == 0) {
            if (index <= MAX_CHERRY) {
                gra->cherrys[index-1].x = x;
                gra->cherrys[index-1].y = y;
            }
        }
        else if (strcmp(objectType, "Good") == 0) {
            gra->goodX = x;
            gra->goodY = y;
        }
        else if (strcmp(objectType, "Bad") == 0) {
            gra->badX = x;
            gra->badY = y;
        }
    }

    fclose(file);

    return 1;
}

void poziom(SDL_Window *window, SDL_Renderer *renderer, SDL_Surface *screen, SDL_Surface *charset, SDL_Texture *scrtex, gra *gra, int *poziomInit){
    int game, t1, t2, space, barrelNum, frames, zmiennaSide[MAX_BARRELS], zmiennaZyc;
    double worldTime, delta, skokCzas = 0, elapsedTime = 0, velocityX, velocityY, jumpTime, yJumpStart;
    SDL_Surface *marioSL, *marioSR, *mario, *marioJumpL, *marioJumpR, *marioZmienna, *marioRunR, *marioRunL, *barrelBmp, *barrel1Bmp, *barrel2Bmp;
    SDL_Event event;
    
    game = TRUE;
    
    barrel1Bmp = SDL_LoadBMP("./barrel.bmp");
    barrel2Bmp = SDL_LoadBMP("./barrel2.bmp");
    marioSL = SDL_LoadBMP("./marioSL.bmp");
    marioSR = SDL_LoadBMP("./marioSR.bmp");
    marioJumpR = SDL_LoadBMP("./marioJumpR.bmp");
    marioJumpL = SDL_LoadBMP("./marioJumpL.bmp");
    marioRunR = SDL_LoadBMP("./marioRunR.bmp");
    marioRunL = SDL_LoadBMP("./marioRunL.bmp");
    if(marioSL == NULL || marioSR == NULL || marioJumpR == NULL || marioJumpL == NULL || marioRunR == NULL || marioRunL == NULL) {
        printf("SDL_LoadBMP(marioS.bmp) error: %s\n", SDL_GetError());
        SDL_FreeSurface(charset);
        SDL_FreeSurface(screen);
        SDL_DestroyTexture(scrtex);
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        SDL_Quit();
        game = FALSE;
    };
    
    char text[128];
    int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
    
    space = FALSE;
    frames = 0;
    yJumpStart = 0;
    worldTime = 0;
    barrelNum = 0;
    velocityX = 0;
    velocityY = 0;
    jumpTime = 0;
    mario = marioSR;
    marioZmienna = marioSR;
    barrelBmp = barrel1Bmp;
    t1 = SDL_GetTicks();

    while (game) {
        t2 = SDL_GetTicks();
        delta = (t2 - t1) * 0.001;
        t1 = t2;
        worldTime += delta;
        
        gra->xPlayer += velocityX * delta;
        gra->xPlayer = fmax(0 + BMP_SIZE/2, fmin(SCREEN_WIDTH - BMP_SIZE/2, gra->xPlayer));
        gra->yPlayer += velocityY * delta;
        if (!space && !wDol(gra)) correctPlatform(gra);
        
        for (int i = 0; i < MAX_BARRELS; i++){
            if (gra->barrels[i].active == TRUE){
                gra->barrels[i].y += gra->barrels[i].velocityY*delta;
                gra->barrels[i].x += gra->barrels[i].velocityX*delta;
            }
        }
        
        checkCherry(gra);
        
        SDL_FillRect(screen, NULL, czarny);
        poziomDraw(window, renderer, screen, charset, scrtex, &game, gra);
        DrawSurface(screen, mario, gra->xPlayer, gra->yPlayer);
        
        snprintf(text, sizeof(text),"Poziom %d - Czas: %.1lf s  Punkty: %d pkt", gra->poziom, worldTime, gra->pkt);
        DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 20, text, charset);
        snprintf(text, sizeof(text),"Wykonane podpunktu: obowiazkowe + A-I");
        DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 40, text, charset);

       /* snprintf(text, sizeof(text),"gor %d dol %d checkplat %d space %d x %.2f y %.2f", wGore(gra), wDol(gra), checkPlatform(gra, gra->xPlayer, gra->yPlayer + BMP_SIZE/2), space, gra->xPlayer, gra->yPlayer);
        DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 60, text, charset);*/

        while(SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_KEYDOWN:
                    if(event.key.keysym.sym == SDLK_ESCAPE) {
                        przerwa(window, renderer, screen, charset, scrtex, gra, poziomInit);
                        game = FALSE;
                    }
                    else if(event.key.keysym.sym == SDLK_1) {
                        gra->poziom = 1;
                        game = FALSE;
                    }
                    else if(event.key.keysym.sym == SDLK_2) {
                        gra->poziom = 2;
                        game = FALSE;
                    }
                    else if(event.key.keysym.sym == SDLK_3) {
                        gra->poziom = 3;
                        game = FALSE;
                    }
                    else if(event.key.keysym.sym == SDLK_RIGHT){
                        frames++;
                        if (frames % 2 == 0) mario = marioSR;
                        else mario = marioRunR;
                        if (gra->xPlayer + BMP_SIZE/2 < SCREEN_WIDTH){
                            velocityX = PLAYER_SPEED;
                        }
                        velocityX = PLAYER_SPEED;
                    }
                    else if(event.key.keysym.sym == SDLK_LEFT){
                        frames++;
                        if (frames % 2 == 0) mario = marioSL;
                        else mario = marioRunL;
                        if (gra->xPlayer - BMP_SIZE/2 > 0){
                            velocityX = -PLAYER_SPEED;
                        }
                        else gra->xPlayer = BMP_SIZE/2;
                    }
                    else if(event.key.keysym.sym == SDLK_SPACE && !space){
                        skokCzas = worldTime;
                        if (mario == marioSL || mario == marioRunL) {
                            marioZmienna = mario;
                            mario = marioJumpL;
                        }
                        else if (mario == marioSR || mario == marioRunR){
                            marioZmienna = mario;
                            mario = marioJumpR;
                        }
                        space = TRUE;
                        yJumpStart = gra->yPlayer;
                    }
                    else if(event.key.keysym.sym == SDLK_UP && wGore(gra)){
                        mario = marioJumpL;
                        velocityY = -PLAYER_SPEED;
                    }
                    else if(event.key.keysym.sym == SDLK_DOWN && wDol(gra)){
                        mario = marioJumpL;
                        velocityY = PLAYER_SPEED;
                    }
                    break;
                case SDL_KEYUP:
                    if (event.key.keysym.sym == SDLK_RIGHT || event.key.keysym.sym == SDLK_LEFT) {
                        velocityX = 0.0;
                    }
                    if (event.key.keysym.sym == SDLK_UP || event.key.keysym.sym == SDLK_DOWN) {
                        velocityY = 0.0;
                    }
                    break;
                case SDL_QUIT:
                    game = FALSE;
                    *poziomInit = FALSE;
                    break;
            };
        }
        if (!checkPlatform(gra, gra->xPlayer, gra->yPlayer + BMP_SIZE/2) && !wGore(gra) && !wDol(gra)  && !space){
                velocityY = PLAYER_SPEED;
                gra->yPlayer += velocityY *delta;
            }
        
        if (space) {
            jumpTime = worldTime - skokCzas;

            if (gra->yPlayer < yJumpStart) {
                velocityY += GRAVITY * jumpTime;
            }
            else if (jumpTime == 0){  //poczatek skoku
                velocityY = -JUMP_SPEED;
            }
            else {
                space = FALSE;
                velocityY = 0;
                if (marioZmienna == marioSR || marioZmienna == marioRunR) mario = marioSR;
                else if (marioZmienna == marioSL || marioZmienna == marioRunL) mario = marioSL;
            }

            for (int i = 0; i< gra->numPlatform; i++){
                if (gra->xPlayer > gra->barrels[i].x - 0.4 && gra->xPlayer < gra->barrels[i].x + 0.4 && 
                gra->barrels[i].y > gra->yPlayer && gra->barrels[i].y < gra->yPlayer + PLATFORMA_HEIGHT*4){
                    gra->pkt += 50;
                }
            }
        }
        
        if (worldTime-elapsedTime >= 4.0){
            loadBarrel(gra, barrelNum);
            barrelNum = (barrelNum + 1) % MAX_BARRELS;
            elapsedTime = worldTime;
        }
        
        zmiennaZyc = gra->lives;
        for (int i = 0; i < MAX_BARRELS; i++){
            if (gra->barrels[i].active == TRUE) {
                if (barrelsDown(gra, i) == TRUE){
                    barrelBmp = barrel2Bmp;
                    gra->barrels[i].velocityY = BARREL_SPEEDY;
                    gra->barrels[i].velocityX = 0;
                    if (zmiennaSide[i] == RIGHT) gra->barrels[i].side = LEFT;
                    else  gra->barrels[i].side = RIGHT;
                }
                else {
                    barrelBmp = barrel1Bmp;
                    if (gra->barrels[i].side == RIGHT) gra->barrels[i].velocityX = BARREL_SPEEDX;
                    else gra->barrels[i].velocityX = -BARREL_SPEEDX;
                    gra->barrels[i].velocityY = 0;
                    zmiennaSide[i] = gra->barrels[i].side;
                }
                correctPlatformBarrel(gra);
                if (!space) checkBarrels(gra, i);
                DrawSurface(screen, barrelBmp, gra->barrels[i].x, gra->barrels[i].y);
            }
        }
        if (zmiennaZyc != gra->lives){
            velocityX = 0;
            velocityY = 0;
            strataZycia(window, renderer, screen, charset, scrtex, gra, poziomInit, &game);
        }
        
        
        if (gra->lives == 0) {
            koniec(window, renderer, screen, charset, scrtex, gra, poziomInit);
            game = FALSE;
        }
        
        if (gra->xPlayer >= gra->goodX - BMP_SIZE/2 &&  gra->xPlayer <= gra->goodX + BMP_SIZE/2 && gra->yPlayer == gra->goodY) {
            gra->pkt += 1000;
            wygrana(window, renderer, screen, charset, scrtex, gra, poziomInit);
            game= FALSE;
        }
        
        SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);

        SDL_RenderCopy(renderer, scrtex, NULL, NULL);
        SDL_RenderPresent(renderer);
        
    }
}

void poziomInit(SDL_Window *window, SDL_Renderer *renderer, SDL_Surface *screen, SDL_Surface *charset, SDL_Texture *scrtex, int poziomNr){
    gra gra;
    int poziomInit = TRUE;
    char filename[15];
    
    gra.poziom = poziomNr;
    gra.pkt = 0;
    gra.lives = 3;
    gra.currentFloor = 0;
    gra.maxFloors = 4;
    
    while (poziomInit){
        gra.cherrys[0].active = TRUE;
        gra.cherrys[1].active = TRUE;
        for (int i = 0; i< MAX_BARRELS; i ++){
            gra.barrels[i].active = FALSE;
        }
        switch (gra.poziom) {
            case 1:
                gra.xPlayer = 20;
                gra.yPlayer = SCREEN_HEIGHT - PLATFORMA_HEIGHT - BMP_SIZE/2;
                gra.numDrabin = 4;
                gra.numPlatform = 32;
                gra.startingXBarrels = 40;
                gra.startingYBarrels = 202;
                
                gra.platformy = (platforma *)malloc(gra.numPlatform * sizeof(platforma));
                gra.drabiny = (drabina *)malloc(gra.numDrabin * sizeof(drabina));
                break;
            case 2:
                gra.xPlayer = SCREEN_WIDTH/2;
                gra.yPlayer = SCREEN_HEIGHT - PLATFORMA_HEIGHT - BMP_SIZE/2;
                gra.numDrabin = 6;
                gra.numPlatform = 24;
                gra.startingXBarrels = 600;
                gra.startingYBarrels = 224;
                
                gra.platformy = (platforma *)malloc(gra.numPlatform * sizeof(platforma));
                gra.drabiny = (drabina *)malloc(gra.numDrabin * sizeof(drabina));
                break;
            case 3:
                gra.xPlayer = 20;
                gra.yPlayer = SCREEN_HEIGHT - PLATFORMA_HEIGHT - BMP_SIZE/2;
                gra.numDrabin = 4;
                gra.numPlatform = 28;
                gra.startingXBarrels = 40;
                gra.startingYBarrels = 214;
                
                gra.drabiny = (drabina *)malloc(gra.numDrabin * sizeof(drabina));
                gra.platformy = (platforma *)malloc(gra.numPlatform * sizeof(platforma));
                break;
            case 4:
                wczytajStan(&gra);
                gra.drabiny = (drabina *)malloc(gra.numDrabin * sizeof(drabina));
                gra.platformy = (platforma *)malloc(gra.numPlatform * sizeof(platforma));
                break;
            default:
                break;
        }
        
        snprintf(filename, sizeof(filename), "poziom%d.txt", gra.poziom);
        infoZPliku(filename, &gra);
        poziom(window, renderer, screen, charset, scrtex, &gra, &poziomInit);
        
        free(gra.platformy);
        free(gra.drabiny);
    }
}
// main
#ifdef __cplusplus
extern "C"
#endif

int main(int argc, char **argv) {
    int menu, rc, yInfo, xInfo, infoMain;
    SDL_Event event;
    SDL_Surface *screen, *charset, *king;
    SDL_Texture *scrtex;
    SDL_Window *window;
    SDL_Renderer *renderer;

    if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        printf("SDL_Init error: %s\n", SDL_GetError());
        return 1;
        }

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

    SDL_SetWindowTitle(window, "King Donkey");


    screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
                                  0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

    scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                               SDL_TEXTUREACCESS_STREAMING,
                               SCREEN_WIDTH, SCREEN_HEIGHT);


    // wyzπczenie widocznoúci kursora myszy
    SDL_ShowCursor(SDL_DISABLE);

    // wczytanie obrazka bmp
    charset = SDL_LoadBMP("./cs8x8.bmp");
    king = SDL_LoadBMP("./king.bmp");
    
    if(charset == NULL || king == NULL) {
        printf("error: %s\n", SDL_GetError());
        SDL_FreeSurface(screen);
        SDL_DestroyTexture(scrtex);
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        SDL_Quit();
        return 1;
        };
    SDL_SetColorKey(charset, true, 0x000000);
    
    char text[128];
    int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
    int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);

    menu = TRUE;
    yInfo = SCREEN_HEIGHT/2 - 24;
    xInfo = SCREEN_WIDTH/2 - SCREEN_WIDTH/4;
    infoMain = TRUE;
    
    while(menu) {
        SDL_FillRect(screen, NULL, czarny);

        DrawSurface(screen, king, SCREEN_WIDTH / 2, SCREEN_HEIGHT/2 - 45);
        
        //prostokat pod tekstem do przewijania w menu
        DrawRectangle(screen, xInfo, yInfo, SCREEN_WIDTH/2, 18, niebieski, niebieski);
        
        snprintf(text,sizeof(text), "Esc - wyjscie     poruszanie po menu: \030, \031    Enter - Wybierz");
        DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 26, text, charset);
        
        //info menu wlaczone to te glowne, gdy !infoMenu, gracz wybiera poziom
        if (infoMain){
            snprintf(text,sizeof(text), "Nowa gra");
            DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, SCREEN_HEIGHT/2 - 18, text, charset);
            snprintf(text,sizeof(text), "Wczytaj gre");
            DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, SCREEN_HEIGHT/2 + 12, text, charset);
            snprintf(text,sizeof(text), "Ranking");
            DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, SCREEN_HEIGHT/2 + 42, text, charset);
        }
        else {
            snprintf(text,sizeof(text), "1 Poziom");
            DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, SCREEN_HEIGHT/2 - 18, text, charset);
            snprintf(text,sizeof(text), "2 Poziom");
            DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, SCREEN_HEIGHT/2 + 12, text, charset);
            snprintf(text,sizeof(text), "3 Poziom");
            DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, SCREEN_HEIGHT/2 + 42, text, charset);
        }
        
        SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
        SDL_RenderCopy(renderer, scrtex, NULL, NULL);
        SDL_RenderPresent(renderer);

        while(SDL_PollEvent(&event)) {
            switch(event.type) {
                case SDL_KEYDOWN:
                    if(event.key.keysym.sym == SDLK_ESCAPE){
                        if (infoMain) menu = FALSE;
                        else infoMain = TRUE;
                    }
                    else if (event.key.keysym.sym == SDLK_n){
                        poziomInit(window, renderer, screen, charset, scrtex, 1);
                        menu=FALSE;
                    }
                    else if (event.key.keysym.sym == SDLK_DOWN){
                        if (yInfo != SCREEN_HEIGHT/2 + 36) yInfo += 30;
                        else yInfo = SCREEN_HEIGHT/2 - 24;
                    }
                    else if (event.key.keysym.sym == SDLK_UP){
                        if (yInfo != SCREEN_HEIGHT/2 - 24)yInfo -= 30;
                        else yInfo = SCREEN_HEIGHT/2 + 36;
                    }
                    else if (event.key.keysym.sym == SDLK_RETURN){
                        switch (yInfo) {
                            case SCREEN_HEIGHT/2 - 24:
                                if (infoMain) infoMain = FALSE;
                                else {
                                    poziomInit(window, renderer, screen, charset, scrtex, 1);
                                    menu=FALSE;
                                }
                                break;
                            case SCREEN_HEIGHT/2 + 6:
                                if (infoMain) {
                                    poziomInit(window, renderer, screen, charset, scrtex, 4);
                                    infoMain = FALSE;
                                }
                                else {
                                    poziomInit(window, renderer, screen, charset, scrtex, 2);
                                    menu=FALSE;
                                }
                                break;
                            case SCREEN_HEIGHT/2 + 36:
                                if (infoMain){
                                    rankingFunction(window, renderer, screen, charset, scrtex);
                                }
                                else {
                                    poziomInit(window, renderer, screen, charset, scrtex, 3);
                                    menu=FALSE;
                                }
                                break;
                            default:
                                break;
                        }
                    }
                    else if (event.key.keysym.sym == SDLK_1){
                        poziomInit(window, renderer, screen, charset, scrtex, 1);
                        menu=FALSE;
                    }
                    else if (event.key.keysym.sym == SDLK_2){
                        poziomInit(window, renderer, screen, charset, scrtex, 2);
                        menu=FALSE;
                    }
                    else if (event.key.keysym.sym == SDLK_3){
                        poziomInit(window, renderer, screen, charset, scrtex, 3);
                        menu=FALSE;
                    }
                    break;
                case SDL_QUIT:
                    menu = FALSE;
                    break;
            };
        };
    };

    SDL_FreeSurface(charset);
    SDL_FreeSurface(screen);
    SDL_DestroyTexture(scrtex);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
    };
