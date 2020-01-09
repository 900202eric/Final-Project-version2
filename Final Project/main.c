// [main.c]
// this template is provided for the 2D shooter game.

#include <stdio.h>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_audio.h>
#include <allegro5/allegro_acodec.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

// If defined, logs will be shown on console and written to file.
// If commented out, logs will not be shown nor be saved.
#define LOG_ENABLED

/* Constants. */

// Frame rate (frame per second)
const int FPS = 60;
// Display (screen) width.
const int SCREEN_W = 800;
// Display (screen) height.
const int SCREEN_H = 600;
// At most 4 audios can be played at a time.
const int RESERVE_SAMPLES = 4;
// Same as:
// const int SCENE_MENU = 1;
// const int SCENE_START = 2;
enum {
    SCENE_MENU = 1,
    SCENE_START = 2,
    // [HACKATHON 3-7]
    // TODO: Declare a new scene id.
    SCENE_SETTINGS = 3,
    SCENE_GAMEOVER = 4,
    SCENE_SELECT = 5
};

/* Input states */

// The active scene id.
int active_scene;
// Keyboard state, whether the key is down or not.
bool key_state[ALLEGRO_KEY_MAX];
// Mouse state, whether the key is down or not.
// 1 is for left, 2 is for right, 3 is for middle.
bool *mouse_state;
// Mouse position.
int mouse_x, mouse_y;
// TODO: More variables to store input states such as joysticks, ...

/* Variables for allegro basic routines. */

ALLEGRO_DISPLAY* game_display;
ALLEGRO_EVENT_QUEUE* game_event_queue;
ALLEGRO_TIMER* game_update_timer;

/* Shared resources*/

ALLEGRO_FONT* font_pirulen_32;
ALLEGRO_FONT* font_pirulen_24;
// TODO: More shared resources or data that needed to be accessed
// across different scenes.

/* Menu Scene resources*/
ALLEGRO_BITMAP* main_img_background;
// [HACKATHON 3-1]
// TODO: Declare 2 variables for storing settings images.
ALLEGRO_BITMAP* img_settings;
ALLEGRO_BITMAP* img_settings2;
ALLEGRO_SAMPLE* main_bgm;
ALLEGRO_SAMPLE_ID main_bgm_id;

/* Start Scene resources*/
ALLEGRO_BITMAP* start_img_background;
ALLEGRO_BITMAP* start_img_plane;
ALLEGRO_BITMAP* start_img_enemy;
ALLEGRO_SAMPLE* start_bgm;
ALLEGRO_SAMPLE_ID start_bgm_id;
// [HACKATHON 2-1]
// TODO: Declare a variable to store your bullet's image.
ALLEGRO_BITMAP* img_bullet;
ALLEGRO_BITMAP* img_enemy_bullets;

/* Setting Scene resources*/
ALLEGRO_BITMAP* setting_img_return;
ALLEGRO_BITMAP* setting_background;

typedef struct {
    // The center coordinate of the image.
    float x, y;
    // The width and height of the object.
    float w, h;
    // The velocity in x, y axes.
    float vx, vy;
    // Should we draw this object on the screen.
    bool hidden;
    // The pointer to the object’s image.
    ALLEGRO_BITMAP* img;
} MovableObject;

void draw_movable_object(MovableObject obj);
#define MAX_ENEMY 3
// [HACKATHON 2-2]
// TODO: Declare the max bullet count that will show on screen.
// You can try max 4 bullets here and see if you needed more.
#define MAX_BULLET 4
#define MAX_ENEMY_BULLET 4
#define PLANE_SPEED 1
#define sp enemies[j].x%3
#define ENEMY_SPEED (int) sp
#define BULLET_SPEED 5
#define ENEMY_BULLET_SPEED ENEMY_SPEED/3+5
MovableObject plane;
MovableObject plane_2p;
MovableObject enemies[MAX_ENEMY];
// [HACKATHON 2-3]
// TODO: Declare an array to store bullets with size of max bullet count.
MovableObject bullets[MAX_BULLET];
MovableObject bullets_2p[MAX_BULLET];
MovableObject enemy_bullets[MAX_ENEMY][MAX_ENEMY_BULLET];

// [HACKATHON 2-4]
// TODO: Set up bullet shooting cool-down variables.
// 1) Declare your shooting cool-down time as constant. (0.2f will be nice)
// 2) Declare your last shoot timestamp.
const float MAX_COOLDOWN = 0.2f;
const float MAX_ENEMY_COOLDOWN = 0.9f;
double last_shoot_timestamp;
double last_shoot_timestamp_2p;
double enemy_last_shoot_timestamp[MAX_ENEMY];

/* declare collision variable*/
bool collide_state = false;

/* declare score point variable*/
#define enemy_point 10
int score_point = 0;

/* declare health point variable*/
#define MAX_LIFE 4
#define MAX_HP 100
#define bullet_damage 10
int health_point = MAX_HP;
int life_point = MAX_LIFE;

/* declare the record variable*/
FILE *fp;
int total_record;
typedef struct{
    int no;
    char name[5];
    int score;
} record_source;
record_source record_struct_array[100];
bool name_editing = false;
int name_edit_position;
char player_name[30]={'e', 'n', 't', 'e', 'r', ' ', 'n', 'a', 'm', 'e', ' ', 'f', 'o', 'r', '5',' ', 'c', 'h', 'a', 'r', '\0'};

/* declare the compare record function*/
int compare(const void *, const void *);

/* declare mouse/keyboard swtich*/
bool mouse_control = false;

/* declare player state switch*/
bool player_control = false;

/* Declare function prototypes. */

// Initialize allegro5 library
void allegro5_init(void);
// Initialize variables and resources.
// Allows the game to perform any initialization it needs before
// starting to run.
void game_init(void);
// Process events inside the event queue using an infinity loop.
void game_start_event_loop(void);
// Run game logic such as updating the world, checking for collision,
// switching scenes and so on.
// This is called when the game should update its logic.
void game_update(void);
// Draw to display.
// This is called when the game should draw itself.
void game_draw(void);
// Release resources.
// Free the pointers we allocated.
void game_destroy(void);
// Function to change from one scene to another.
void game_change_scene(int next_scene);
// Load resized bitmap and check if failed.
ALLEGRO_BITMAP *load_bitmap_resized(const char *filename, int w, int h);
// [HACKATHON 3-2]
// TODO: Declare a function.
// Determines whether the point (px, py) is in rect (x, y, w, h).
bool pnt_in_rect(int px, int py, int x, int y, int w, int h);

/* Event callbacks. */
void on_key_down(int keycode);
void on_mouse_down(int btn, int x, int y);

/* Declare function prototypes for debugging. */

// Display error message and exit the program, used like 'printf'.
// Write formatted output to stdout and file from the format string.
// If the program crashes unexpectedly, you can inspect "log.txt" for
// further information.
void game_abort(const char* format, ...);
// Log events for later debugging, used like 'printf'.
// Write formatted output to stdout and file from the format string.
// You can inspect "log.txt" for logs in the last run.
void game_log(const char* format, ...);
// Log using va_list.
void game_vlog(const char* format, va_list arg);

int main(int argc, char** argv) {
    // Set random seed for better random outcome.
    if ((fp = fopen("./resource/record.txt", "r+")) == NULL) {
        perror("Error: ");
        game_abort("failed to load file: record.txt");
    }
    srand((int)time(NULL));
    allegro5_init();
    game_log("Allegro5 initialized");
    game_log("Game begin");
    // Initialize game variables.
    game_init();
    game_log("Game initialized");
    // Draw the first frame.
    game_draw();
    game_log("Game start event loop");
    // This call blocks until the game is finished.
    game_start_event_loop();
    game_log("Game end");
    fclose(fp);
    fp = NULL;
    game_destroy();
    return 0;
}

void allegro5_init(void) {
    if (!al_init())
        game_abort("failed to initialize allegro");
    
    // Initialize add-ons.
    if (!al_init_primitives_addon())
        game_abort("failed to initialize primitives add-on");
    if (!al_init_font_addon())
        game_abort("failed to initialize font add-on");
    if (!al_init_ttf_addon())
        game_abort("failed to initialize ttf add-on");
    if (!al_init_image_addon())
        game_abort("failed to initialize image add-on");
    if (!al_install_audio())
        game_abort("failed to initialize audio add-on");
    if (!al_init_acodec_addon())
        game_abort("failed to initialize audio codec add-on");
    if (!al_reserve_samples(RESERVE_SAMPLES))
        game_abort("failed to reserve samples");
    if (!al_install_keyboard())
        game_abort("failed to install keyboard");
    if (!al_install_mouse())
        game_abort("failed to install mouse");
    // TODO: Initialize other addons such as video, ...
    
    // Setup game display.
    game_display = al_create_display(SCREEN_W, SCREEN_H);
    if (!game_display)
        game_abort("failed to create display");
    al_set_window_title(game_display, "I2P(I)_2019 Final Project <student_id>");
    
    // Setup update timer.
    game_update_timer = al_create_timer(1.0f / FPS);
    if (!game_update_timer)
        game_abort("failed to create timer");
    
    // Setup event queue.
    game_event_queue = al_create_event_queue();
    if (!game_event_queue)
        game_abort("failed to create event queue");
    
    // Malloc mouse buttons state according to button counts.
    const unsigned m_buttons = al_get_mouse_num_buttons();
    game_log("There are total %u supported mouse buttons", m_buttons);
    // mouse_state[0] will not be used.
    mouse_state = malloc((m_buttons + 1) * sizeof(bool));
    memset(mouse_state, false, (m_buttons + 1) * sizeof(bool));
    
    // Register display, timer, keyboard, mouse events to the event queue.
    al_register_event_source(game_event_queue, al_get_display_event_source(game_display));
    al_register_event_source(game_event_queue, al_get_timer_event_source(game_update_timer));
    al_register_event_source(game_event_queue, al_get_keyboard_event_source());
    al_register_event_source(game_event_queue, al_get_mouse_event_source());
    // TODO: Register other event sources such as timer, video, ...
    
    // Start the timer to update and draw the game.
    al_start_timer(game_update_timer);
}

void game_init(void) {
    /* Shared resources*/
    font_pirulen_32 = al_load_font("resource/pirulen.ttf", 32, 0);
    if (!font_pirulen_32)
        game_abort("failed to load font: pirulen.ttf with size 32");
    
    font_pirulen_24 = al_load_font("resource/pirulen.ttf", 24, 0);
    if (!font_pirulen_24)
        game_abort("failed to load font: pirulen.ttf with size 24");
    
    /* Menu Scene resources*/
    main_img_background = load_bitmap_resized("resource/main-bg.jpg", SCREEN_W, SCREEN_H);
    
    main_bgm = al_load_sample("resource/S31-Night Prowler.ogg");
    if (!main_bgm)
        game_abort("failed to load audio: S31-Night Prowler.ogg");
    
    img_settings = al_load_bitmap("resource/settings.png");
    if (!img_settings)
        game_abort("failed to load image: settings.png");
    img_settings2 = al_load_bitmap("resource/settings2.png");
    if (!img_settings2)
        game_abort("failed to load image: settings2.png");
    
    /* Start Scene resources*/
    start_img_background = load_bitmap_resized("resource/start-bg.jpg", SCREEN_W, SCREEN_H);
    
    start_img_plane = al_load_bitmap("resource/plane.png");
    if (!start_img_plane)
        game_abort("failed to load image: plane.png");
    
    start_img_enemy = al_load_bitmap("resource/smallfighter0006.png");
    if (!start_img_enemy)
        game_abort("failed to load image: smallfighter0006.png");
    
    start_bgm = al_load_sample("resource/mythica.ogg");
    if (!start_bgm)
        game_abort("failed to load audio: mythica.ogg");
    
    // [HACKATHON 2-5]
    // TODO: Initialize bullets.
    // 1) Search for a bullet image online and put it in your project.
    //    You can use the image we provided.
    // 2) Load it in by 'al_load_bitmap' or 'load_bitmap_resized'.
    // 3) If you use 'al_load_bitmap', don't forget to check its return value.
    img_bullet = al_load_bitmap("resource/image12.png");
    if (!img_bullet)
        game_abort("failed to load image: image12.png");
    
    img_enemy_bullets = load_bitmap_resized("resource/enemy_bullet.png", 17, 34);
    if(!img_enemy_bullets){
        game_abort("failed to load image: enemy_bullet.png");
    }
    
    /* Setting Scene resources*/
    setting_img_return = load_bitmap_resized("resource/return1.jpg", 50, 50);
    //setting_img_return = al_load_bitmap("resource/return.png");
    if(!setting_img_return){
        game_abort("failed to load image: return1.jpg");
    }
    
    setting_background = al_load_bitmap("resource/setting-bg.jpg");
    if(!setting_background){
        game_abort("failed to load image: setting-bg.jpg");
    }
    
    // Change to first scene.
    game_change_scene(SCENE_MENU);
}

void game_start_event_loop(void) {
    bool done = false;
    ALLEGRO_EVENT event;
    int redraws = 0;
    while (!done) {
        al_wait_for_event(game_event_queue, &event);
        if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            // Event for clicking the window close button.
            game_log("Window close button clicked");
            done = true;
        } else if (event.type == ALLEGRO_EVENT_TIMER) {
            // Event for redrawing the display.
            if (event.timer.source == game_update_timer)
                // The redraw timer has ticked.
                redraws++;
        } else if (event.type == ALLEGRO_EVENT_KEY_DOWN) {
            // Event for keyboard key down.
            game_log("Key with keycode %d down", event.keyboard.keycode);
            key_state[event.keyboard.keycode] = true;
            on_key_down(event.keyboard.keycode);
        } else if (event.type == ALLEGRO_EVENT_KEY_UP) {
            // Event for keyboard key up.
            game_log("Key with keycode %d up", event.keyboard.keycode);
            key_state[event.keyboard.keycode] = false;
        } else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) {
            // Event for mouse key down.
            game_log("Mouse button %d down at (%d, %d)", event.mouse.button, event.mouse.x, event.mouse.y);
            mouse_state[event.mouse.button] = true;
            on_mouse_down(event.mouse.button, event.mouse.x, event.mouse.y);
        } else if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
            // Event for mouse key up.
            game_log("Mouse button %d up at (%d, %d)", event.mouse.button, event.mouse.x, event.mouse.y);
            mouse_state[event.mouse.button] = false;
        } else if (event.type == ALLEGRO_EVENT_MOUSE_AXES) {
            if (event.mouse.dx != 0 || event.mouse.dy != 0) {
                // Event for mouse move.
                //game_log("Mouse move to (%d, %d)", event.mouse.x, event.mouse.y);
                mouse_x = event.mouse.x;
                mouse_y = event.mouse.y;
            } else if (event.mouse.dz != 0) {
                // Event for mouse scroll.
                game_log("Mouse scroll at (%d, %d) with delta %d", event.mouse.x, event.mouse.y, event.mouse.dz);
            }
        }
        // TODO: Process more events and call callbacks by adding more
        // entries inside Scene.
        
        // Redraw
        if (redraws > 0 && al_is_event_queue_empty(game_event_queue)) {
            // if (redraws > 1)
            //     game_log("%d frame(s) dropped", redraws - 1);
            // Update and draw the next frame.
            game_update();
            game_draw();
            redraws = 0;
        }
    }
}

void game_update(void) {
    if (active_scene == SCENE_START) {
        /* location for player1*/
        plane.vx = plane.vy = 0;
        if (key_state[ALLEGRO_KEY_UP])
            plane.vy -= PLANE_SPEED;
        if (key_state[ALLEGRO_KEY_DOWN])
            plane.vy += PLANE_SPEED;
        if (key_state[ALLEGRO_KEY_LEFT])
            plane.vx -= PLANE_SPEED;
        if (key_state[ALLEGRO_KEY_RIGHT])
            plane.vx += PLANE_SPEED;
        // 0.71 is (1/sqrt(2)).
        if (mouse_control && !player_control){
            plane.y = mouse_y;
            plane.x = mouse_x;
        }
        plane.y += plane.vy * 4 * (plane.vx ? 0.71f : 1);
        plane.x += plane.vx * 4 * (plane.vy ? 0.71f : 1);
        
        //plane 1's boarder
        if (plane.x < plane.w/2)
            plane.x = plane.w/2;
        else if (plane.x > SCREEN_W-plane.w/2)
            plane.x = SCREEN_W-plane.w/2;
        if (plane.y < plane.h/2)
            plane.y = plane.h/2;
        else if (plane.y > SCREEN_H-plane.h/2)
            plane.y = SCREEN_H-plane.h/2;
        
        // plane 2 moving
        if(player_control){
            plane_2p.vx = plane_2p.vy = 0;
            if (key_state[ALLEGRO_KEY_W])
                plane_2p.vy -= PLANE_SPEED;
            if (key_state[ALLEGRO_KEY_S])
                plane_2p.vy += PLANE_SPEED;
            if (key_state[ALLEGRO_KEY_A])
                plane_2p.vx -= PLANE_SPEED;
            if (key_state[ALLEGRO_KEY_D])
                plane_2p.vx += PLANE_SPEED;
            plane_2p.y += plane_2p.vy * 4 * (plane_2p.vx ? 0.71f : 1);
            plane_2p.x += plane_2p.vx * 4 * (plane_2p.vy ? 0.71f : 1);
        }
        
        //plane 2's boarder
        if(player_control){
            if (plane_2p.x < plane_2p.w/2)
                plane_2p.x = plane_2p.w/2;
            else if (plane_2p.x > SCREEN_W-plane_2p.w/2)
                plane_2p.x = SCREEN_W-plane_2p.w/2;
            if (plane_2p.y < plane_2p.h/2)
                plane_2p.y = plane_2p.h/2;
            else if (plane_2p.y > SCREEN_H-plane_2p.h/2)
                plane_2p.y = SCREEN_H-plane_2p.h/2;
        }
        
        /* player's bullet*/
        int i;
        for (i=0;i<MAX_BULLET;i++) {
            if (bullets[i].hidden)
                continue;
            bullets[i].x += bullets[i].vx;
            bullets[i].y += bullets[i].vy;
            if (bullets[i].y - bullets[i].h/2 < 0)
                bullets[i].hidden = true;
        }
        
        /* player_2p's bullet*/
        if(player_control){
            for (i=0;i<MAX_BULLET;i++) {
                if (bullets_2p[i].hidden)
                    continue;
                bullets_2p[i].x += bullets_2p[i].vx;
                bullets_2p[i].y += bullets_2p[i].vy;
                if (bullets_2p[i].y - bullets_2p[i].h/2 < 0)
                    bullets_2p[i].hidden = true;
            }
        }
        
        // [HACKATHON 2-8]
        // TODO: Shoot if key is down and cool-down is over.
        // 1) Get the time now using 'al_get_time'.
        // 2) If Space key is down in 'key_state' and the time
        //    between now and last shoot is not less that cool
        //    down time.
        // 3) Loop through the bullet array and find one that is hidden.
        //    (This part can be optimized.)
        // 4) The bullet will be found if your array is large enough.
        // 5) Set the last shoot time to now.
        // 6) Set hidden to false (recycle the bullet) and set its x, y to the
        //    front part of your plane.
        double now = al_get_time();
        if (key_state[ALLEGRO_KEY_SPACE] && now - last_shoot_timestamp >= MAX_COOLDOWN) {
            for (i = 0; i<MAX_BULLET;i++) {
                if (bullets[i].hidden)
                    break;
            }
            if (i < MAX_BULLET) {
                last_shoot_timestamp = now;
                bullets[i].hidden = false;
                bullets[i].x = plane.x;
                bullets[i].y = plane.y - plane.h/2;
            }
        }
        
        /* player_2p's shooting*/
        if (key_state[ALLEGRO_KEY_Q] && now - last_shoot_timestamp_2p >= MAX_COOLDOWN) {
            for (i = 0; i<MAX_BULLET;i++) {
                if (bullets_2p[i].hidden)
                    break;
            }
            if (i < MAX_BULLET) {
                last_shoot_timestamp_2p = now;
                bullets_2p[i].hidden = false;
                bullets_2p[i].x = plane_2p.x;
                bullets_2p[i].y = plane_2p.y - plane_2p.h/2;
            }
        }
        
        /* location for enemy team*/
        int j;
        for (j=0;j<MAX_ENEMY;j++){
            if(enemies[j].hidden)
                continue;
            enemies[j].vx = 0;
            enemies[j].vy = ENEMY_SPEED;      //twice fast then player
            if(ENEMY_SPEED == 0){
                enemies[j].vy = 1;
            }
            enemies[j].y += enemies[j].vy * 4 * (enemies[j].vx ? 0.71f : 1);
            enemies[j].x += enemies[j].vx * 4 * (enemies[j].vy ? 0.71f : 1);
            if(enemies[j].x < enemies[j].w/2 || enemies[j].x > SCREEN_W-enemies[j].w/2 || enemies[j].y < enemies[j].h/2 || enemies[j].y > SCREEN_H-enemies[j].h/2)
                enemies[j].hidden = true;
        }
        
        for(j=0;j<MAX_ENEMY;j++){
            if(enemies[j].hidden)
                break;
        }
        if(j < MAX_ENEMY){
            enemies[j].hidden = false;
            enemies[j].x = enemies[j].w / 2 + (float)rand() / RAND_MAX * (SCREEN_W - enemies[j].w);
            enemies[j].y = 40;
        }
        
        /* enemy's bullet*/
        double enemy_now = al_get_time();
        for(int j=0;j<MAX_ENEMY;j++){
            int a;
            for (a=0;a<MAX_ENEMY_BULLET;a++) {
                if (enemy_bullets[j][a].hidden)
                    continue;
                enemy_bullets[j][a].x += enemy_bullets[j][a].vx;
                enemy_bullets[j][a].y += enemy_bullets[j][a].vy;
                if (enemy_bullets[j][a].y >= SCREEN_H - enemy_bullets[j][a].h/2 )
                    enemy_bullets[j][a].hidden = true;
            }
        }
        
        for(int j=0;j<MAX_ENEMY;j++){
            if(enemy_now - enemy_last_shoot_timestamp[j] >= MAX_ENEMY_COOLDOWN){
                for (i = 0; i<MAX_ENEMY_BULLET;i++) {
                    if (enemy_bullets[j][i].hidden)
                        break;
                }
                if (i < MAX_ENEMY_BULLET) {
                    enemy_last_shoot_timestamp[j] = enemy_now;
                    enemy_bullets[j][i].hidden = false;
                    enemy_bullets[j][i].x = enemies[j].x;
                    enemy_bullets[j][i].y = enemies[j].y + enemies[j].h/2;
                }
            }
        }
        
        /* Collision process for enemies*/
        for(int j=0;j<MAX_ENEMY;j++){
            if(!enemies[j].hidden){
                if((plane.x <= enemies[j].x + enemies[j].w/2 && plane.x >= enemies[j].x - enemies[j].w/2) && (plane.y+plane.h/2 <= enemies[j].y + enemies[j].h/2 + 10 && plane.y+plane.h/2 >= enemies[j].y + enemies[j].h/2 - 10)){
                    enemies[j].hidden = true;
                    life_point--;
                    game_change_scene(SCENE_START);
                    /*collide_state = true;
                     game_log("Enemy Collision Happen");*/
                    break;
                }
            }
        }
        
        /* Collision process for enemies (player 2p)*/
        for(int j=0;j<MAX_ENEMY;j++){
            if(!enemies[j].hidden){
                if((plane_2p.x <= enemies[j].x + enemies[j].w/2 && plane_2p.x >= enemies[j].x - enemies[j].w/2) && (plane_2p.y+plane_2p.h/2 <= enemies[j].y + enemies[j].h/2 + 10 && plane_2p.y+plane_2p.h/2 >= enemies[j].y + enemies[j].h/2 - 10)){
                    enemies[j].hidden = true;
                    //TODO: to finish life substrating and collision
                    //life_point--;
                    //game_change_scene(SCENE_START);
                    //temporary: reset to the initial location
                    plane_2p.x = 400-200;
                    plane_2p.y = 500;
                    /*collide_state = true;
                     game_log("Enemy Collision Happen");*/
                    break;
                }
            }
        }
        
        /* Collision process for enemies' bullets*/
        for(int j=0;j<MAX_ENEMY;j++){
            for(int a=0;a<MAX_ENEMY_BULLET;a++){
                if(!enemy_bullets[j][a].hidden){
                    if((plane.x <= enemy_bullets[j][a].x + enemy_bullets[j][a].w/2 && plane.x >= enemy_bullets[j][a].x - enemy_bullets[j][a].w/2) && (plane.y+plane.h/2 <= enemy_bullets[j][a].y + enemy_bullets[j][a].h/2 + 10 && plane.y+plane.h/2 >= enemy_bullets[j][a].y + enemy_bullets[j][a].h/2 - 10)){
                        health_point -= bullet_damage;
                        enemy_bullets[j][a].hidden = true;
                        /*collide_state = true;
                         game_log("Bullet Collision Happen");*/
                        break;
                    }
                }
            }
        }
        
        //TODO
        /* Collision process for enemies' bullets for player 2*/
        for(int j=0;j<MAX_ENEMY;j++){
            for(int a=0;a<MAX_ENEMY_BULLET;a++){
                if(!enemy_bullets[j][a].hidden){
                    if((plane_2p.x <= enemy_bullets[j][a].x + enemy_bullets[j][a].w/2 && plane_2p.x >= enemy_bullets[j][a].x - enemy_bullets[j][a].w/2) && (plane_2p.y+plane_2p.h/2 <= enemy_bullets[j][a].y + enemy_bullets[j][a].h/2 + 10 && plane_2p.y+plane_2p.h/2 >= enemy_bullets[j][a].y + enemy_bullets[j][a].h/2 - 10)){
                        //TODO:to finish the blood consuming
                        //health_point -= bullet_damage;
                        enemy_bullets[j][a].hidden = true;
                        /*collide_state = true;
                         game_log("Bullet Collision Happen");*/
                        break;
                    }
                }
            }
        }
        
        /* Shooting the enemies*/
        for(int b=0;b<MAX_BULLET;b++){
            for(int j=0;j<MAX_ENEMY;j++){
                if(!enemies[j].hidden){
                    if((bullets[b].x <= enemies[j].x + enemies[j].w/2 && bullets[b].x >= enemies[j].x - enemies[j].w/2) && (bullets[b].y+bullets[b].h/2 <= enemies[j].y + enemies[j].h/2 + 10 && bullets[b].y+bullets[b].h/2 >= enemies[j].y + enemies[j].h/2 - 10)){
                        enemies[j].hidden = true;
                        bullets[b].hidden = true;
                        /* shoot one enemy for enemy_point point*/
                        score_point += enemy_point;
                        game_log("Enemy has been defeated");
                    }
                }
            }
        }
        
        /* Shooting the enemies for plater 2*/
        for(int b=0;b<MAX_BULLET;b++){
            for(int j=0;j<MAX_ENEMY;j++){
                if(!enemies[j].hidden){
                    if((bullets_2p[b].x <= enemies[j].x + enemies[j].w/2 && bullets_2p[b].x >= enemies[j].x - enemies[j].w/2) && (bullets_2p[b].y+bullets_2p[b].h/2 <= enemies[j].y + enemies[j].h/2 + 10 && bullets_2p[b].y+bullets_2p[b].h/2 >= enemies[j].y + enemies[j].h/2 - 10)){
                        enemies[j].hidden = true;
                        bullets_2p[b].hidden = true;
                        /* shoot one enemy for enemy_point point*/
                        //TODO: to finish score point for player 2
                        //score_point += enemy_point;
                        game_log("Enemy has been defeated");
                    }
                }
            }
        }
        
        /* Destroy the enemies' bullets*/
        for(int b=0;b<MAX_BULLET;b++){
            for(int j=0;j<MAX_ENEMY;j++){
                for(int a=0;a<MAX_ENEMY_BULLET;a++){
                    if(!enemy_bullets[j][a].hidden){
                        if((bullets[b].x <= enemy_bullets[j][a].x + enemy_bullets[j][a].w/2 && bullets[b].x >= enemy_bullets[j][a].x - enemy_bullets[j][a].w/2) && (bullets[b].y+bullets[b].h/2 <= enemy_bullets[j][a].y + enemy_bullets[j][a].h/2 + 10 && bullets[b].y+bullets[b].h/2 >= enemy_bullets[j][a].y + enemy_bullets[j][a].h/2 - 10)){
                            enemy_bullets[j][a].hidden = true;
                            bullets[b].hidden = true;
                            game_log("Enemies' bullet has been destroyed");
                        }
                    }
                }
            }
        }
        
        /* Destroy the enemies' bullets for player 2*/
        for(int b=0;b<MAX_BULLET;b++){
            for(int j=0;j<MAX_ENEMY;j++){
                for(int a=0;a<MAX_ENEMY_BULLET;a++){
                    if(!enemy_bullets[j][a].hidden){
                        if((bullets_2p[b].x <= enemy_bullets[j][a].x + enemy_bullets[j][a].w/2 && bullets_2p[b].x >= enemy_bullets[j][a].x - enemy_bullets[j][a].w/2) && (bullets_2p[b].y+bullets_2p[b].h/2 <= enemy_bullets[j][a].y + enemy_bullets[j][a].h/2 + 10 && bullets_2p[b].y+bullets_2p[b].h/2 >= enemy_bullets[j][a].y + enemy_bullets[j][a].h/2 - 10)){
                            enemy_bullets[j][a].hidden = true;
                            bullets_2p[b].hidden = true;
                            game_log("Enemies' bullet has been destroyed");
                        }
                    }
                }
            }
        }
    }
}

void game_draw(void) {
    if (active_scene == SCENE_MENU) {
        al_draw_bitmap(main_img_background, 0, 0, 0);
        al_draw_text(font_pirulen_32, al_map_rgb(255, 255, 255), SCREEN_W / 2, 30, ALLEGRO_ALIGN_CENTER, "Space Shooter");
        al_draw_text(font_pirulen_24, al_map_rgb(255, 255, 255), 20, SCREEN_H - 50, 0, "Press enter key to start");
        // [HACKATHON 3-5]
        // TODO: Draw settings images.
        // The settings icon should be located at (x, y, w, h) =
        // (SCREEN_W - 48, 10, 38, 38).
        // Change its image according to your mouse position.
        if (pnt_in_rect(mouse_x, mouse_y, SCREEN_W-48, 10, 38, 38))
            al_draw_bitmap(img_settings2, SCREEN_W-48, 10, 0);
        else
            al_draw_bitmap(img_settings, SCREEN_W-48, 10, 0);
        /* reset the collide state*/
        if(collide_state)
            collide_state = false;
        /* reset the player's HP*/
        health_point = MAX_HP;
        life_point = MAX_LIFE;
        /* reset the player's score*/
        score_point = 0;
        /* reset the player's seletion*/
        player_control = false;
    } else if (active_scene == SCENE_START) {
        int i;
        al_draw_bitmap(start_img_background, 0, 0, 0);
        // [HACKATHON 2-9]
        // TODO: Draw all bullets in your bullet array.
        // Uncomment and fill in the code below.
        for (i=0;i<MAX_BULLET;i++)
            draw_movable_object(bullets[i]);
        draw_movable_object(plane);
        for (i = 0; i < MAX_ENEMY; i++)
            draw_movable_object(enemies[i]);
        for(i = 0;i < MAX_ENEMY;i++)
            for(int j=0;j<MAX_ENEMY_BULLET;j++)
                draw_movable_object(enemy_bullets[i][j]);
        
        // player 2
        if(player_control){
            for (i=0;i<MAX_BULLET;i++)
                draw_movable_object(bullets_2p[i]);
            draw_movable_object(plane_2p);
        }
        
        if(collide_state){
            game_change_scene(SCENE_GAMEOVER);
        }
        
        if(health_point <= 0 && life_point <= 0){
            game_change_scene(SCENE_GAMEOVER);
        }
        
        if(health_point <= 0 && life_point > 0){
            life_point--;
            health_point = MAX_HP;
            game_change_scene(SCENE_START);
        }
        
        if(life_point <= 0){
            game_change_scene(SCENE_GAMEOVER);
        }
        
        /* show the player's score*/
        al_draw_textf(font_pirulen_24, al_map_rgb(0, 0, 0), 10, 10, 0, "Score: %d",score_point);
        /* shoe the player's hp*/
        al_draw_textf(font_pirulen_24, al_map_rgb(0, 0, 0), 30, 40, 0, "HP: %d",health_point);
        al_draw_textf(font_pirulen_24, al_map_rgb(0, 0, 0), SCREEN_W-150, 10, 0, "LIFE: %d",life_point);
    }
    // [HACKATHON 3-9]
    // TODO: If active_scene is SCENE_SETTINGS.
    // Draw anything you want, or simply clear the display.
    else if (active_scene == SCENE_SETTINGS) {
        al_clear_to_color(al_map_rgb(0, 0, 0));
        //al_draw_bitmap(setting_background, 0, 0, 0);
        al_draw_bitmap(setting_img_return, 10, 10, 0);
        al_draw_text(font_pirulen_32, al_map_rgb(255, 255, 255), SCREEN_W / 2, 30, ALLEGRO_ALIGN_CENTER, "Setting");
        al_draw_text(font_pirulen_24, al_map_rgb(255, 255, 255), SCREEN_W / 2-150, 100, ALLEGRO_ALIGN_CENTER, "Mouse Control");
        /*if(mouse_control){
         if (pnt_in_rect(mouse_x, mouse_y, SCREEN_W/2 + 150, 100, 100, 50) && mouse_control)
         al_draw_text(font_pirulen_24, al_map_rgb(255, 255, 255), SCREEN_W / 2+200, 100, ALLEGRO_ALIGN_CENTER, "OFF");
         else
         al_draw_text(font_pirulen_24, al_map_rgb(255, 255, 255), SCREEN_W / 2+200, 100, ALLEGRO_ALIGN_CENTER, "ON");
         } else {
         if (pnt_in_rect(mouse_x, mouse_y, SCREEN_W/2 + 150, 100, 100, 50) && !mouse_control)
         al_draw_text(font_pirulen_24, al_map_rgb(255, 255, 255), SCREEN_W / 2+200, 100, ALLEGRO_ALIGN_CENTER, "ON");
         else
         al_draw_text(font_pirulen_24, al_map_rgb(255, 255, 255), SCREEN_W / 2+200, 100, ALLEGRO_ALIGN_CENTER, "OFF");
         }*/
        if(mouse_control){
            al_draw_text(font_pirulen_24, al_map_rgb(255, 255, 255), SCREEN_W / 2+150, 100, ALLEGRO_ALIGN_CENTER, "ON");
            al_draw_text(font_pirulen_24, al_map_rgb(50, 50, 50), SCREEN_W / 2+220, 100, ALLEGRO_ALIGN_CENTER, "OFF");
        } else {
            al_draw_text(font_pirulen_24, al_map_rgb(50, 50, 50), SCREEN_W / 2+150, 100, ALLEGRO_ALIGN_CENTER, "ON");
            al_draw_text(font_pirulen_24, al_map_rgb(255, 255, 255), SCREEN_W / 2+220, 100, ALLEGRO_ALIGN_CENTER, "OFF");
        }
    }
    else if (active_scene == SCENE_GAMEOVER) {
        al_clear_to_color(al_map_rgb(0, 0, 0));
        al_draw_text(font_pirulen_32, al_map_rgb(255, 255, 255), SCREEN_W / 2, SCREEN_H / 2-30, ALLEGRO_ALIGN_CENTER, "GAME OVER");
        al_draw_text(font_pirulen_24, al_map_rgb(255, 255, 255), 20, SCREEN_H - 50, 0, "Press enter key to back to menu");
        al_draw_textf(font_pirulen_24, al_map_rgb(255, 255, 255), 10, 10, 0, "Score: %d",score_point);
    }
    else if(active_scene == SCENE_SELECT){
        al_clear_to_color(al_map_rgb(0, 0, 0));
        al_draw_text(font_pirulen_32, al_map_rgb(255, 255, 255), SCREEN_W/2, 30, ALLEGRO_ALIGN_CENTER, "SELECT YOUR TEAM");
        al_draw_text(font_pirulen_24, al_map_rgb(255, 255, 255), SCREEN_W / 2-200, 100, ALLEGRO_ALIGN_CENTER, "MODE");
        if(!player_control){
            al_draw_text(font_pirulen_24, al_map_rgb(255, 255, 255), SCREEN_W / 2+50, 100, ALLEGRO_ALIGN_CENTER, "1 PLAYER");
            al_draw_text(font_pirulen_24, al_map_rgb(50, 50, 50), SCREEN_W / 2+220, 100, ALLEGRO_ALIGN_CENTER, "2 PLAYER");
        } else {
            al_draw_text(font_pirulen_24, al_map_rgb(50, 50, 50), SCREEN_W / 2+50, 100, ALLEGRO_ALIGN_CENTER, "1 PLAYER");
            al_draw_text(font_pirulen_24, al_map_rgb(255, 255, 255), SCREEN_W / 2+220, 100, ALLEGRO_ALIGN_CENTER, "2 PLAYER");
        }
        // record player's name
        al_draw_text(font_pirulen_24, al_map_rgb(255, 255, 255), SCREEN_W / 2-200, SCREEN_H/2 + 100, ALLEGRO_ALIGN_CENTER, "PLAYER");
        
        al_draw_textf(font_pirulen_24, al_map_rgb(200, 200, 200), SCREEN_W/2+150, SCREEN_H/2+100, ALLEGRO_ALIGN_CENTER, "%s",player_name);
        if(pnt_in_rect(mouse_x, mouse_y, SCREEN_W/2-100, SCREEN_H/2+100, 450, 50)){
            al_draw_textf(font_pirulen_24, al_map_rgb(255, 255, 255), SCREEN_W/2+150, SCREEN_H/2+100, ALLEGRO_ALIGN_CENTER, "%s",player_name);
        }
    }
    al_flip_display();
}

void game_destroy(void) {
    // Destroy everything you have created.
    // Free the memories allocated by malloc or allegro functions.
    // Destroy shared resources.
    al_destroy_font(font_pirulen_32);
    al_destroy_font(font_pirulen_24);
    
    /* Menu Scene resources*/
    al_destroy_bitmap(main_img_background);
    al_destroy_sample(main_bgm);
    // [HACKATHON 3-6]
    // TODO: Destroy the 2 settings images.
    // Uncomment and fill in the code below.
    al_destroy_bitmap(img_settings);
    al_destroy_bitmap(img_settings2);
    
    /* Start Scene resources*/
    al_destroy_bitmap(start_img_background);
    al_destroy_bitmap(start_img_plane);
    al_destroy_bitmap(start_img_enemy);
    al_destroy_sample(start_bgm);
    // [HACKATHON 2-10]
    // TODO: Destroy your bullet image.
    al_destroy_bitmap(img_bullet);
    al_destroy_bitmap(img_enemy_bullets);
    
    /* Setting Scene resources*/
    al_destroy_bitmap(setting_img_return);
    al_destroy_bitmap(setting_background);
    
    al_destroy_timer(game_update_timer);
    al_destroy_event_queue(game_event_queue);
    al_destroy_display(game_display);
    free(mouse_state);
}

void game_change_scene(int next_scene) {
    game_log("Change scene from %d to %d", active_scene, next_scene);
    // TODO: Destroy resources initialized when creating scene.
    if (active_scene == SCENE_MENU) {
        al_stop_sample(&main_bgm_id);
        game_log("stop audio (bgm)");
    } else if (active_scene == SCENE_START) {
        al_stop_sample(&start_bgm_id);
        game_log("stop audio (bgm)");
    }
    
    active_scene = next_scene;
    
    if (active_scene == SCENE_MENU) {
        
        //read the record file
        rewind(fp);
        printf("======Reading Record.txt to Load the Record======\n\n");
        total_record = 0;
        fscanf(fp, "ALLEGRO 5 SCORE RECORD\n");
        printf("ALLEGRO 5 SCORE RECORD\n");
        fscanf(fp, "Record Total: %d\n",&total_record);
        printf("Record Total: %d\n",total_record);
        fscanf(fp, "NO.| Name  | Score\n");
        printf("NO.| Name  | Score\n");
        for(int aa=0;aa<total_record;aa++){
            fscanf(fp, "%d | %s | %d\n", &record_struct_array[aa].no, record_struct_array[aa].name, &record_struct_array[aa].score);
            printf("%02d | %-5s | %05d\n", record_struct_array[aa].no, record_struct_array[aa].name, record_struct_array[aa].score);
        }
        printf("\n======================END========================\n");
        
        if (!al_play_sample(main_bgm, 1, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, &main_bgm_id))
            game_abort("failed to play audio (bgm)");
    } else if (active_scene == SCENE_START) {
        int i;
        // planes location;
        if(player_control){
            // player 1
            //plane
            plane.img = start_img_plane;
            plane.x = 400+200;
            plane.y = 500;
            plane.w = al_get_bitmap_width(plane.img);
            plane.h = al_get_bitmap_height(plane.img);
            
            // bullet
            for (int i=0;i<MAX_BULLET;i++) {
                bullets[i].w = al_get_bitmap_width(img_bullet);
                bullets[i].h = al_get_bitmap_height(img_bullet);
                bullets[i].img = img_bullet;
                bullets[i].vx = 0;
                bullets[i].vy = -1*BULLET_SPEED;
                bullets[i].hidden = true;
            }
            
            //player 2
            //plane
            plane_2p.img = start_img_plane;
            plane_2p.x = 400-200;
            plane_2p.y = 500;
            plane_2p.w = al_get_bitmap_width(plane_2p.img);
            plane_2p.h = al_get_bitmap_height(plane_2p.img);
            
            // bullet
            for (int i=0;i<MAX_BULLET;i++) {
                bullets_2p[i].w = al_get_bitmap_width(img_bullet);
                bullets_2p[i].h = al_get_bitmap_height(img_bullet);
                bullets_2p[i].img = img_bullet;
                bullets_2p[i].vx = 0;
                bullets_2p[i].vy = -1*BULLET_SPEED;
                bullets_2p[i].hidden = true;
            }
            
        } else {
            //plane
            plane.img = start_img_plane;
            plane.x = 400;
            plane.y = 500;
            plane.w = al_get_bitmap_width(plane.img);
            plane.h = al_get_bitmap_height(plane.img);
            
            // bullet
            for (int i=0;i<MAX_BULLET;i++) {
                bullets[i].w = al_get_bitmap_width(img_bullet);
                bullets[i].h = al_get_bitmap_height(img_bullet);
                bullets[i].img = img_bullet;
                bullets[i].vx = 0;
                bullets[i].vy = -1*BULLET_SPEED;
                bullets[i].hidden = true;
            }
        }
        
        for (i = 0; i < MAX_ENEMY; i++) {
            enemies[i].img = start_img_enemy;
            enemies[i].w = al_get_bitmap_width(start_img_enemy);
            enemies[i].h = al_get_bitmap_height(start_img_enemy);
            enemies[i].x = enemies[i].w / 2 + (float)rand() / RAND_MAX * (SCREEN_W - enemies[i].w);
            enemies[i].y = 40;
            enemies[i].hidden = true;
        }
        
        for(int j=0;j<MAX_ENEMY;j++){
            for(int i=0;i<MAX_ENEMY_BULLET;i++){
                enemy_bullets[j][i].w = al_get_bitmap_width(img_enemy_bullets);
                enemy_bullets[j][i].h = al_get_bitmap_height(img_enemy_bullets);
                enemy_bullets[j][i].img = img_enemy_bullets;
                enemy_bullets[j][i].vx = 0;
                enemy_bullets[j][i].vy = ENEMY_BULLET_SPEED;
                enemy_bullets[j][i].hidden = true;
            }
        }
        
        if (!al_play_sample(start_bgm, 1, 0.0, 1.0, ALLEGRO_PLAYMODE_LOOP, &start_bgm_id))
            game_abort("failed to play audio (bgm)");
    } else if(active_scene == SCENE_GAMEOVER){
        rewind(fp);
        total_record++;
        strncpy(record_struct_array[total_record-1].name, player_name, 5);
        record_struct_array[total_record-1].no = total_record;
        record_struct_array[total_record-1].score = score_point;
        qsort(record_struct_array, total_record, sizeof(record_source), compare);
        printf("======Reading Record.txt to Load the Record======\n\n");
        printf("ALLEGRO 5 SCORE RECORD\n");
        fprintf(fp, "ALLEGRO 5 SCORE RECORD\n");
        printf("Record Total: %d\n",total_record);
        fprintf(fp, "Record Total: %d\n",total_record);
        printf("NO.| Name  | Score\n");
        fprintf(fp, "NO.| Name  | Score\n");
        for(int aa=0;aa<total_record;aa++){
            record_struct_array[aa].no = aa+1;
            printf("%02d | %-5s | %05d\n", record_struct_array[aa].no, record_struct_array[aa].name, record_struct_array[aa].score);
            fprintf(fp, "%02d | %-5s | %05d\n", record_struct_array[aa].no, record_struct_array[aa].name, record_struct_array[aa].score);
        }
        
        if (fflush(fp) != 0)
            printf("Error store file\n");
        printf("\n======================END========================\n");
    }
}

void on_key_down(int keycode) {
    if (active_scene == SCENE_MENU) {
        if (keycode == ALLEGRO_KEY_ENTER){
            game_change_scene(SCENE_SELECT);
            return;
        }
    }
    if (active_scene == SCENE_SELECT) {
        if(keycode == ALLEGRO_KEY_ENTER){
            game_change_scene(SCENE_START);
            return;
        }
        //TODO: still need more key to support
        if(name_editing && name_edit_position <= 4){
            if(keycode == ALLEGRO_KEY_A){
                player_name[name_edit_position] = 'A';
                name_edit_position++;
            }
            if(keycode == ALLEGRO_KEY_B){
                player_name[name_edit_position] = 'B';
                name_edit_position++;
            }
            if(keycode == ALLEGRO_KEY_C){
                player_name[name_edit_position] = 'C';
                name_edit_position++;
            }
            if(keycode == ALLEGRO_KEY_D){
                player_name[name_edit_position] = 'D';
                name_edit_position++;
            }
            if(keycode == ALLEGRO_KEY_E){
                player_name[name_edit_position] = 'E';
                name_edit_position++;
            }
            if(keycode == ALLEGRO_KEY_F){
                player_name[name_edit_position] = 'F';
                name_edit_position++;
            }
            if(keycode == ALLEGRO_KEY_G){
                player_name[name_edit_position] = 'G';
                name_edit_position++;
            }
            if(keycode == ALLEGRO_KEY_H){
                player_name[name_edit_position] = 'H';
                name_edit_position++;
            }
            if(keycode == ALLEGRO_KEY_I){
                player_name[name_edit_position] = 'I';
                name_edit_position++;
            }
            if(keycode == ALLEGRO_KEY_J){
                player_name[name_edit_position] = 'J';
                name_edit_position++;
            }
            if(keycode == ALLEGRO_KEY_K){
                player_name[name_edit_position] = 'K';
                name_edit_position++;
            }
            if(keycode == ALLEGRO_KEY_L){
                player_name[name_edit_position] = 'L';
                name_edit_position++;
            }
            if(keycode == ALLEGRO_KEY_M){
                player_name[name_edit_position] = 'M';
                name_edit_position++;
            }
            if(keycode == ALLEGRO_KEY_N){
                player_name[name_edit_position] = 'N';
                name_edit_position++;
            }
            if(keycode == ALLEGRO_KEY_O){
                player_name[name_edit_position] = 'O';
                name_edit_position++;
            }
            if(keycode == ALLEGRO_KEY_P){
                player_name[name_edit_position] = 'P';
                name_edit_position++;
            }
            if(keycode == ALLEGRO_KEY_Q){
                player_name[name_edit_position] = 'Q';
                name_edit_position++;
            }
            if(keycode == ALLEGRO_KEY_R){
                player_name[name_edit_position] = 'R';
                name_edit_position++;
            }
            if(keycode == ALLEGRO_KEY_S){
                player_name[name_edit_position] = 'S';
                name_edit_position++;
            }
            if(keycode == ALLEGRO_KEY_T){
                player_name[name_edit_position] = 'T';
                name_edit_position++;
            }
            if(keycode == ALLEGRO_KEY_U){
                player_name[name_edit_position] = 'U';
                name_edit_position++;
            }
            if(keycode == ALLEGRO_KEY_V){
                player_name[name_edit_position] = 'V';
                name_edit_position++;
            }
            if(keycode == ALLEGRO_KEY_W){
                player_name[name_edit_position] = 'W';
                name_edit_position++;
            }
            if(keycode == ALLEGRO_KEY_X){
                player_name[name_edit_position] = 'X';
                name_edit_position++;
            }
            if(keycode == ALLEGRO_KEY_Y){
                player_name[name_edit_position] = 'Y';
                name_edit_position++;
            }
            if(keycode == ALLEGRO_KEY_Z){
                player_name[name_edit_position] = 'Z';
                name_edit_position++;
            }
            if(keycode == ALLEGRO_KEY_BACKSPACE && name_edit_position){
                name_edit_position--;
                player_name[name_edit_position] = '\0';
            }
        } else if(name_editing && name_edit_position <= 5 && name_edit_position){
            if(keycode == ALLEGRO_KEY_BACKSPACE){
                name_edit_position--;
                player_name[name_edit_position] = '\0';
            }
        }
    }
    if (active_scene == SCENE_GAMEOVER) {
        if(keycode == ALLEGRO_KEY_ENTER)
            game_change_scene(SCENE_MENU);
    }
}

void on_mouse_down(int btn, int x, int y) {
    /* Menu Scene*/
    if (active_scene == SCENE_MENU) {
        if (btn == mouse_state[1]) {
            if (pnt_in_rect(x, y, SCREEN_W-48, 10, 38, 38))
                game_change_scene(SCENE_SETTINGS);
        }
    }
    
    /* Select Scene*/
    if(active_scene == SCENE_SELECT){
        if(btn == mouse_state[1]){
            if(pnt_in_rect(mouse_x, mouse_y, SCREEN_W/2-100, SCREEN_H/2+100, 450, 50) && !name_editing){
                for(int a1=0;a1<30;a1++){
                    player_name[a1] = '\0';
                    printf("p:%d ",player_name[a1]);
                }
                name_edit_position = 0;
                name_editing = true;
            } else {
                name_editing = false;
            }
        }
    }
    
    /* Setting Scene*/
    if(active_scene == SCENE_SETTINGS){
        if(btn == mouse_state[1]){
            if(pnt_in_rect(x, y, 10, 10, 60, 60))
                game_change_scene(SCENE_MENU);
            if(pnt_in_rect(x, y, SCREEN_W/2 + 125, 100, 200, 50)){
                mouse_control = !mouse_control;
                game_log("Change mouse control");
            }
        }
        return;
    }
    
    /* Selecting Scene*/
    if(active_scene == SCENE_SELECT){
        if(btn == mouse_state[1]){
            if(pnt_in_rect(x, y, SCREEN_W/2, 100, 350, 50)){
                player_control = !player_control;
                game_log("Change player number");
            }
        }
        return;
    }
}

void draw_movable_object(MovableObject obj) {
    if (obj.hidden)
        return;
    al_draw_bitmap(obj.img, round(obj.x - obj.w / 2), round(obj.y - obj.h / 2), 0);
}

ALLEGRO_BITMAP *load_bitmap_resized(const char *filename, int w, int h) {
    ALLEGRO_BITMAP* loaded_bmp = al_load_bitmap(filename);
    if (!loaded_bmp)
        game_abort("failed to load image: %s", filename);
    ALLEGRO_BITMAP *resized_bmp = al_create_bitmap(w, h);
    ALLEGRO_BITMAP *prev_target = al_get_target_bitmap();
    
    if (!resized_bmp)
        game_abort("failed to create bitmap when creating resized image: %s", filename);
    al_set_target_bitmap(resized_bmp);
    al_draw_scaled_bitmap(loaded_bmp, 0, 0,
                          al_get_bitmap_width(loaded_bmp),
                          al_get_bitmap_height(loaded_bmp),
                          0, 0, w, h, 0);
    al_set_target_bitmap(prev_target);
    al_destroy_bitmap(loaded_bmp);
    
    game_log("resized image: %s", filename);
    
    return resized_bmp;
}

int compare(const void *a, const void *b){
    const record_source *fa = (const record_source *)a;
    const record_source *fb = (const record_source *)b;
    
    return (*fb).score - (*fa).score;
}

// [HACKATHON 3-3]
// TODO: Define bool pnt_in_rect(int px, int py, int x, int y, int w, int h)
bool pnt_in_rect(int px, int py, int x, int y, int w, int h) {
    if(px >= x && px <= x+w && py >= y && py <= y+h)
        return true;
    return false;
}


// +=================================================================+
// | Code below is for debugging purpose, it's fine to remove it.    |
// | Deleting the code below and removing all calls to the functions |
// | doesn't affect the game.                                        |
// +=================================================================+

void game_abort(const char* format, ...) {
    va_list arg;
    va_start(arg, format);
    game_vlog(format, arg);
    va_end(arg);
    fprintf(stderr, "error occured, exiting after 2 secs");
    // Wait 2 secs before exiting.
    al_rest(2);
    // Force exit program.
    exit(1);
}

void game_log(const char* format, ...) {
#ifdef LOG_ENABLED
    va_list arg;
    va_start(arg, format);
    game_vlog(format, arg);
    va_end(arg);
#endif
}

void game_vlog(const char* format, va_list arg) {
#ifdef LOG_ENABLED
    static bool clear_file = true;
    vprintf(format, arg);
    printf("\n");
    // Write log to file for later debugging.
    FILE* pFile = fopen("log.txt", clear_file ? "w" : "a");
    if (pFile) {
        //vfprintf(pFile, format, arg);
        fprintf(pFile, "\n");
        fclose(pFile);
    }
    clear_file = false;
#endif
}

