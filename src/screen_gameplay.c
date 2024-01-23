#include "raylib.h"
#include "screens.h"
#include <math.h>
#include <stdio.h>
#include "raymath.h"

//----------------------------------------------------------------------------------
// Module Variables Definition (local)
//----------------------------------------------------------------------------------


#define MAX_ENEMIES 100

#define PLAYER_MAX_HEALTH 100
#define MAX_BULLETS 50

#define MAX_ORBS 100

#define HEIGHT 144
#define WIDTH 256


typedef struct {
	Vector2 position;
	Vector2 direction;
	int size;
	Vector2 speed;
	Color color;
	int xp;
	int level;
	int health;
	float absorptionRadius;
	float fireRate;

} Player;

typedef struct {
	Vector2 position;
	int size;
	int health;
	Vector2 speed;
	Color color;
	float hitTimer;
	bool active;
	bool shouldMove;
} Enemy;

typedef struct Bullet {
	Vector2 position;
	Vector2 direction;
	float speed;
	int remainingHits;
	int hits;
	int dmg;
	bool active;
	bool hitEnemies[MAX_ENEMIES]; // New field
} Bullet;

typedef struct Orb {
	Vector2 position;
	int size;
	int exp;
	Color color;
	bool active;
} Orb;


//----------------------------------------------------------------------------------
//TEXTURES
// 
Texture2D bulletTexture;

//----------------------------------------------------------------------------------


static int finishScreen = 0;
Enemy enemies[MAX_ENEMIES];
Bullet bullets[MAX_BULLETS];
Orb orbs[MAX_ORBS];
Player player;
float playerSpeed = 1.0f;
int needXP = 4;
const float moveSpeed = 1.0f;
float enemySpeed = 0.6f;
int difficulty;
static float bulletFireTimer = 0.0f;
static float worldTime = 0.0f;


Camera2D camera = { 0 };



//----------------------------------------------------------------------------------
// Gameplay Screen Functions Definition
//----------------------------------------------------------------------------------

void AbsorbOrbs(Player* plyr) {
	for (int i = 0; i < MAX_ORBS; i++) {
		if (orbs[i].active && CheckCollisionCircles(plyr->position, plyr->absorptionRadius, orbs[i].position, orbs[i].size)) {
			//make orbs move towards the player
			Vector2 direction = { 0.0f, 0.0f };

			direction.x = (plyr->position.x - orbs[i].position.x);
			direction.y = (plyr->position.y - orbs[i].position.y);

			// Normalize the direction vector to ensure consistent speed in all directions

			float length = sqrt((double)(direction.x * direction.x + direction.y * direction.y));
			if (length > 1.0f) {
				direction.x /= length;
				direction.y /= length;
			}
			// Apply the player's speed to their position
			orbs[i].position.x += direction.x * 2;
			orbs[i].position.y += direction.y * 2;

			//check if orb is absorbed
			if (CheckCollisionCircles(plyr->position, plyr->size, orbs[i].position, orbs[i].size)) {
				plyr->xp += orbs[i].exp;
				orbs[i].active = false;
			}
		}
	}
}
void UpdatePlayer(Player* player)
{
	// Create a direction vector based on keyboard input
	Vector2 direction = { 0.0f, 0.0f };
	if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) direction.y -= 1.0f;
	if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) direction.y += 1.0f;
	if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) direction.x -= 1.0f;
	if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) direction.x += 1.0f;

	// Normalize the direction vector to ensure consistent speed in all directions
	float length = (float)sqrt((double)direction.x * direction.x + direction.y * direction.y);
	if (length > 1.0f) {
		direction.x /= length;
		direction.y /= length;
	}

	player->direction.x = direction.x;
	player->direction.y = direction.y;

	playerSpeed = IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT) ? 3.0f : 2.0f;

	// Apply the direction to the player's speed
	player->speed.x = direction.x * moveSpeed * playerSpeed;
	player->speed.y = direction.y * moveSpeed * playerSpeed;

	// Apply the player's speed to their position
	player->position.x += player->speed.x;
	player->position.y += player->speed.y;


	needXP=player->level * player->level * 4;
	if (player->xp>=needXP) {
		player->level++;
		player->xp = 0;
		player->health = PLAYER_MAX_HEALTH;
	}
}

void DrawPlayer(Player* player)
{
	// Draw the player
	DrawCircleV(player->position, (float)player->size, player->color);
}
void SpawnOrb(Vector2 position) {
	for (int i = 0; i < MAX_ORBS; i++) {
		if (!orbs[i].active) {
			orbs[i].position = position;
			orbs[i].size = 3; // Set the size of the orb
			orbs[i].active = true;
			orbs[i].exp = 1;
			orbs[i].color = GREEN;
			break;
		}
	}
}

void DrawOrbs(void) {
	for (int i = 0; i < MAX_ORBS; i++) {
		if (orbs[i].active) {
			// Draw an orb at the orb's position
			DrawCircleV(orbs[i].position, (float)orbs[i].size, orbs[i].color); // Change color as needed
		}
	}
}

void UpdateEnemy(Enemy* enemy,int i)
{
	static float nextEnemySpawnTime = 0.2f;

	if (enemy->health <= 0 && enemy->active) {

		SpawnOrb(enemy->position);
		enemy->active = false;
		enemy->position = (Vector2){ -500.0f, -500.0f };
		
	}
	if (!enemy->active && worldTime>= nextEnemySpawnTime) {

		
        //respawn enemy outside of the camera range of vision
        int respawnOffset = 100; // Adjust as needed
        float angle = (float)GetRandomValue(0, 360); // Random angle in degrees
        float distance = (float)( WIDTH / 2 + respawnOffset); // Distance from the center of the screen

        // Convert polar coordinates to Cartesian coordinates
        enemy->position = (Vector2){ 
            camera.target.x + cos(angle * PI / 180.0f) * distance,
            camera.target.y + sin(angle * PI / 180.0f) * distance
        };

		enemy->health = 2; // Reset health
		enemy->active = true;

		nextEnemySpawnTime += 0.2f;

	}
	enemy->shouldMove = true;
	for (int j = 0; j < MAX_ENEMIES; j++) 
	{
		Enemy other = enemies[j];
		if (j == i || !other.active)continue;
		if (Vector2Distance(enemy->position, other.position) < enemy->size * 2)
		{
			bool condition1 = (Vector2Distance(enemy->position, player.position) > Vector2Distance(other.position, player.position));
			if (condition1)
			{
				enemy->shouldMove = false;
			}
			break;
		}
	}
	if (enemy->shouldMove)
	{
		enemy->position = Vector2Add(Vector2Normalize(Vector2Subtract(player.position ,enemy->position)),enemy->position);
	}
		

	if (enemy->hitTimer > 0) {
		enemy->hitTimer -= GetFrameTime();
		if (enemy->hitTimer <= 0) {
			enemy->color = RED;
		}
	}
}

void DrawEnemy(Enemy* enemy)
{
	// Draw the enemy
	if (enemy->active) DrawCircleV(enemy->position, (float)enemy->size, enemy->color);
}


void throwDagger(void) {

	bulletFireTimer += GetFrameTime(); // Increment timer

	if (bulletFireTimer >= player.fireRate) { // Change 0.2f to your desired fire interval
		bulletFireTimer = 0.0f; // Reset timer

		for (int i = 0; i < MAX_BULLETS; i++) {
			if (!bullets[i].active) {
				if (player.direction.x == 0 && player.direction.y == 0) return;
				bullets[i].position = player.position;

				// Use player's direction
				bullets[i].direction = player.direction;

				bullets[i].active = true;
				bullets[i].remainingHits = bullets[i].hits;

				// Initialize hitEnemies array
				for (int j = 0; j < MAX_ENEMIES; j++) {
					bullets[i].hitEnemies[j] = false;
				}

				break;
			}
		}
	}
}
void UpdateDagger(void) {
	for (int i = 0; i < MAX_BULLETS; i++) {
		if (bullets[i].remainingHits <= 0) {
			bullets[i].active = false;
		}
		if (bullets[i].active) {
			bullets[i].position.x += bullets[i].direction.x * bullets[i].speed;
			bullets[i].position.y += bullets[i].direction.y * bullets[i].speed;

			// Deactivate bullet if it reaches the border of the camera.
			if (bullets[i].position.x < camera.target.x - WIDTH / 2 ||
				bullets[i].position.y < camera.target.y - HEIGHT / 2 ||
				bullets[i].position.x > camera.target.x + WIDTH / 2 ||
				bullets[i].position.y > camera.target.y + HEIGHT / 2) {
				bullets[i].active = false;
			}

			// Check for collisions with all enemies
			for (int j = 0; j < MAX_ENEMIES; j++) {
				if (!bullets[i].hitEnemies[j] && CheckCollisionCircles(bullets[i].position, 2, enemies[j].position, (float)enemies[j].size)) {
					enemies[j].health -= bullets[i].dmg;
					bullets[i].remainingHits--;
					bullets[i].hitEnemies[j] = true; // Mark this enemy as hit
					enemies[j].color = WHITE;
					enemies[j].hitTimer = 0.2f; // Start the hit timer
					if (bullets[i].remainingHits <= 0)
						break;
					//make a hit sound 
					
				}
			}
		}
	}
}

void DrawBullets(void) {
	for (int i = 0; i < MAX_BULLETS; i++) {
		if (bullets[i].active) {
			Rectangle sourceRec = { 0.0f, 0.0f, (float)bulletTexture.width, (float)bulletTexture.height };
			Rectangle destRec = { bullets[i].position.x, bullets[i].position.y, (float)bulletTexture.width, (float)bulletTexture.height };
			Vector2 origin = { (float)bulletTexture.width / 2, (float)bulletTexture.height / 2 };

			// Calculate rotation
			float rotation = atan2f(bullets[i].direction.y, bullets[i].direction.x) * (180.0f / PI);

			DrawTexturePro(bulletTexture, sourceRec, destRec, origin, rotation, WHITE);
		}
	}
}

void DrawHealthBar(Player* player) {
	int healthBarWidth = 20; // Adjust as needed
	int healthBarHeight = 4; // Adjust as needed
	int healthBarOffsetY = -10; // Adjust as needed

	Vector2 healthBarPosition = { player->position.x - healthBarWidth / 2, player->position.y - healthBarOffsetY };

	DrawRectangle(healthBarPosition.x, healthBarPosition.y, healthBarWidth, healthBarHeight, RED); // Background
	DrawRectangle(healthBarPosition.x, healthBarPosition.y, (healthBarWidth * player->health) / PLAYER_MAX_HEALTH, healthBarHeight, GREEN); // Foreground
}

void DrawXPBar(Player* player) {
	int xpBarWidth = WIDTH; // Full width of the screen
	int xpBarHeight = 8; // Adjust as needed
	int XPBarOffsetY = HEIGHT/2; // Adjust as needed

	Vector2 xpBarPosition = { player->position.x - xpBarWidth / 2, player->position.y - XPBarOffsetY };

	DrawRectangle(xpBarPosition.x, xpBarPosition.y, xpBarWidth, xpBarHeight, DARKGRAY); // Background
	DrawRectangle(xpBarPosition.x, xpBarPosition.y, (xpBarWidth * player->xp) / needXP, xpBarHeight, BLUE); // Foreground

	char levelText[32];
	sprintf(levelText, "Level: %d", player->level);
	int textWidth = MeasureText(levelText, 5); // Measure the width of the text
	DrawText(levelText, xpBarPosition.x + xpBarWidth - textWidth - 5, xpBarPosition.y, 5, GRAY); // Adjust size and color as needed

}




void InitGameplayScreen(void)
{
	// Initialize player
	player.position = (Vector2){ WIDTH / 2, HEIGHT / 2 };
	player.direction = (Vector2){ 0.0f, 0.0f };
	player.speed = (Vector2){ 0.0f, 0.0f };
	player.size = 6;
	player.color = BLUE;
	player.xp = 0;
	player.level = 1;
	player.health = PLAYER_MAX_HEALTH;
	player.absorptionRadius = 20;
	player.fireRate = 0.2f;


	finishScreen = 0;

	//init camera
	camera.target = player.position;
	camera.offset = (Vector2){ WIDTH / 2.0f, HEIGHT / 2.0f };
	camera.rotation = 0.0f;
	camera.zoom = 1.0f;

	//init enemies
	for (int i = 0; i < MAX_ENEMIES; i++)
	{
		
		enemies[i].speed = (Vector2){ 0.0f, 0.0f };
		enemies[i].size = 6;
		enemies[i].color = RED;
		enemies[i].health = 2;
		enemies[i].active = false;
		enemies[i].shouldMove = true;


	}

	//init bullets
	bulletTexture = LoadTexture("../resources/dagger.png");
	printf("Texture Width: %d, Height: %d\n", bulletTexture.width, bulletTexture.height);
	for (int i = 0; i < MAX_BULLETS; i++) {
		bullets[i].speed = 8;
		bullets[i].active = false;
		bullets[i].hits = 3;
		bullets[i].dmg = 1;
	}
}


void UpdateGameplayScreen(void)
{

	worldTime += GetFrameTime();
	// Update player position
	UpdatePlayer(&player);

	//update enemies position
	for (int i = 0; i < MAX_ENEMIES; i++)
	{
		UpdateEnemy(&enemies[i],i);
	}

	// Update camera target
	camera.target = player.position;

	// Move bulelts
	UpdateDagger();
	AbsorbOrbs(&player);
	throwDagger();
}

void DrawGameplayScreen(void)
{
	DrawRectangle(0, 0, WIDTH, HEIGHT, BLACK);

	BeginMode2D(camera);
	

	int gridSize = 40; // Change this to change the size of the grid cells

	int minX = (int)(camera.target.x - WIDTH / 2) / gridSize - 1;
	int minY = (int)(camera.target.y - HEIGHT / 2) / gridSize - 1;
	int maxX = (int)(camera.target.x + WIDTH / 2) / gridSize + 1;
	int maxY = (int)(camera.target.y + HEIGHT / 2) / gridSize + 1;

	for (int x = minX; x < maxX; x++)
	{
		for (int y = minY; y < maxY; y++)
		{
			DrawRectangleLines(x * gridSize, y * gridSize, gridSize, gridSize, DARKGRAY);
		}
	}

	// Draw the enemies 
	for (int i = 0; i < MAX_ENEMIES; i++)
	{
		DrawEnemy(&enemies[i]);
	}
	DrawOrbs();
	// Draw the player
	DrawPlayer(&player);
	DrawHealthBar(&player);

	// Draw bullets
	DrawBullets();
	DrawXPBar(&player);
	EndMode2D();
}

void UnloadGameplayScreen(void)
{
	UnloadTexture(bulletTexture);
}

int FinishGameplayScreen(void)
{
	return finishScreen;
}
