
//gcc universe.c `sdl2-config --cflags --libs` -o canard -lm

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <unistd.h>

#define TIMESTEP 0.01
#define GRAVITATIONCONST 10000

#define SCREENX 1920
#define SCREENY 1080

#define RECTSIZE 4
#define DEMIRECTSIZE 2

#define RANGEMASS 10
#define RANGESPEED 30

//structure vecteur
struct vector {
  float x;
  float y;
};

//structure etoile
struct etoile {
  float mass;
  struct vector position;
  struct vector speed;
  struct vector acceleration;
  SDL_Rect rect;
};

//explicite
void print_vector(struct vector* vect) {
  printf("x: %f y: %f\n", vect->x, vect->y);
}
//explicite
void print_etoile(struct etoile* etle) {
  printf("masse: %f\n", etle->mass);
  printf("position: ");
  print_vector(&(etle->position));
  printf("speed: ");
  print_vector(&(etle->speed));
  printf("acceleration: ");
  print_vector(&(etle->acceleration));
  printf("\n");
}

//explicite
float norm_vect(struct vector u){
  return sqrt(u.x*u.x + u.y*u.y);
}
//init un vecteur a zero
void vector_init(struct vector* vect) {
  vect->x = 0;
  vect->y = 0;
}
//init les vecteurs position dans la range de l'ecran
void vector_init_position_random(struct vector* vect) {
  vect->x = rand() % SCREENX;
  vect->y = rand() % SCREENY;
}

//init les vecteurs speed dans une range
void vector_init_random(struct vector* vect) {
  vect->x = (rand() % RANGESPEED *2) - RANGESPEED;
  vect->y = (rand() % RANGESPEED *2) - RANGESPEED;
}

//init une etoile
void etoile_init (struct etoile* etle) {
  etle->mass = 1;
  vector_init(&(etle->position));
  vector_init(&(etle->speed));
  vector_init(&(etle->acceleration));
}

//init une etoile avec des para random
void etoile_init_random(struct etoile* etle) {
  etle->mass = (rand()%(RANGEMASS-1)) + 1;
  vector_init_position_random(&(etle->position));
  vector_init_random(&(etle->speed));
  vector_init(&(etle->acceleration));
}

//init une etoile avec des para random sans vitesse
void etoile_init_random_no_speed(struct etoile* etle) {
  etle->mass = 1;
  vector_init_position_random(&(etle->position));
  vector_init(&(etle->speed));
  vector_init(&(etle->acceleration));
}

//permet d'integrer les veteurs
void integator(struct vector* integrant, struct vector* integrated){
  integrated->x += integrant->x * TIMESTEP;
  integrated->y += integrant->y * TIMESTEP;
}

//init un array d'etoire
void etoile_array_init(struct etoile * tab, int nb_etoile) {
  int i;
  for(i=0; i<nb_etoile; i++){
    etoile_init(&tab[i]);
    //print_etoile(&tab[i]);
  }
}

//init un array etoile radnom
void etoile_array_init_random(struct etoile * tab, int nb_etoile) {
  int i;
  for(i=0; i<nb_etoile; i++){
    etoile_init_random(&tab[i]);
    //print_etoile(&tab[i]);
  }
}
//init un array d'etoile random no speed
void etoile_array_init_random_no_speed(struct etoile * tab, int nb_etoile) {
  int i;
  for(i=0; i<nb_etoile; i++){
    etoile_init_random_no_speed(&tab[i]);
    //print_etoile(&tab[i]);
  }
}

//update le rectangle rep des étoiles
void update_etoile_rect(struct etoile * tab) {

    tab->rect.x = tab->position.x - DEMIRECTSIZE*tab->mass;
    tab->rect.y = tab->position.y - DEMIRECTSIZE*tab->mass;
    tab->rect.w = RECTSIZE*tab->mass ;
    tab->rect.h = RECTSIZE*tab->mass;
}

//dessine les etoile (mdr)
void draw_etoile(SDL_Renderer * renderer, struct etoile * tab, int nb_etoile) {
  int i;
  float valeur_couleur;
  for(i=0; i<nb_etoile; i++){
    valeur_couleur = 255*(exp(-norm_vect(tab[i].speed)/1000));
    SDL_SetRenderDrawColor(renderer, 255, valeur_couleur, valeur_couleur, 255);
    update_etoile_rect(&tab[i]);
    SDL_RenderFillRect(renderer, &tab[i].rect);
  }
}

//dessine les vecteurs speed
void draw_speed_vector(SDL_Renderer * renderer, struct etoile * tab, int nb_etoile){
  int i;
  SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
  for(i=0; i<nb_etoile; i++){
    SDL_RenderDrawLine(renderer, tab[i].position.x, tab[i].position.y, tab[i].position.x + tab[i].speed.x, tab[i].position.y + tab[i].speed.y);
  }
}

//dessine les vecteurs acceleration
void draw_acceleration_vector(SDL_Renderer * renderer, struct etoile * tab, int nb_etoile){
  int i;
  SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
  for(i=0; i<nb_etoile; i++){
    SDL_RenderDrawLine(renderer, tab[i].position.x, tab[i].position.y, tab[i].position.x + tab[i].acceleration.x, tab[i].position.y + tab[i].acceleration.y);
  }
}
//calcule l'acceleration instantanée
void compute_acceleration(struct etoile * tab, int nb_etoile){
  int i, j;
  float distance3;
  struct vector u;
  for(i=0; i<nb_etoile; i++){
    tab[i].acceleration.x = 0;
    tab[i].acceleration.y = 0;
    for(j=0; j<nb_etoile; j++){
      if(j!=i){
        u.x = tab[j].position.x - tab[i].position.x;
        u.y = tab[j].position.y - tab[i].position.y;
        distance3 = pow(u.x*u.x + u.y*u.y, (float)(3)/2);

        tab[i].acceleration.x += (GRAVITATIONCONST*tab[j].mass/distance3)*u.x;
        tab[i].acceleration.y += (GRAVITATIONCONST*tab[j].mass/distance3)*u.y;
      }
    }
  }
}

//integre l'acceleration
void compute_speed(struct etoile * tab, int nb_etoile){
  int i;
  for(i=0; i<nb_etoile; i++){
    integator(&tab[i].acceleration, &tab[i].speed);
  }
}
//inetegre la vitesse
void compute_position(struct etoile * tab, int nb_etoile) {
  int i;
  for(i=0; i<nb_etoile; i++){
    integator(&tab[i].speed, &tab[i].position);
  }
}

int main(int argc, char **argv){

  int i;
  int running = 1;
  int draw_speed = 0;
  int draw_acceleration = 0;
  int nb_etoile = atoi(argv[1]);

  if(argc == 3 && (argv[2][0] == 's' || argv[2][0] == 'f')){draw_speed = 1;}
  if(argc == 3 && (argv[2][0] == 'a' || argv[2][0] == 'f')){draw_acceleration = 1;}

  srand(time(NULL));

  SDL_Window *window = NULL;
  SDL_Renderer *renderer = NULL;
  SDL_Event event;

  if(0 != SDL_Init(SDL_INIT_VIDEO)){fprintf(stderr, "Erreur SDL_Init : %s", SDL_GetError());return EXIT_FAILURE;}
  window = SDL_CreateWindow("UNIVERSE", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREENX, SCREENY, SDL_WINDOW_SHOWN);
  if(window == NULL){fprintf(stderr, "Erreur SDL_CreateWindow : %s", SDL_GetError());return EXIT_FAILURE;}
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if(NULL == renderer){fprintf(stderr, "Erreur SDL_CreateRenderer : %s", SDL_GetError());return EXIT_FAILURE;}
  SDL_SetWindowPosition(window, 0, 0);

  //initialiser les étoiles
  struct etoile liste[nb_etoile];
  etoile_array_init_random(liste, nb_etoile);

  while(running){
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
    //calculer l'acceleration
    compute_acceleration(liste, nb_etoile);
    compute_speed(liste, nb_etoile);
    compute_position(liste, nb_etoile);

    //dessiner les étoiles
    draw_etoile(renderer, liste, nb_etoile);
    if(draw_speed){draw_speed_vector(renderer, liste, nb_etoile);}
    if(draw_acceleration){draw_acceleration_vector(renderer, liste, nb_etoile);}

    //update l'affichage
    SDL_RenderPresent(renderer);
    //permet de quitter avec X
    while(SDL_PollEvent(&event)) {
        if(event.type == SDL_QUIT)
            running = 0;
    }

    SDL_Delay(16);
  }
  SDL_DestroyWindow(window);
  SDL_Quit();
  return EXIT_SUCCESS;
}
