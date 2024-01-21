#include "raylib.h"
#include "screens.h"
#include <math.h>

//----------------------------------------------------------------------------------
// Module Variables Definition (local)
//----------------------------------------------------------------------------------


#define MAX_ENEMIES 100

#define PLAYER_MAX_HEALTH 100
#define MAX_BULLETS 50

#define MAX_ORBS 5



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


} Player;

typedef struct {
	Vector2 position;
	int size;
	int health;
	Vector2 speed;
	Color color;
	float hitTimer;
	bool active;
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




static int finishScreen = 0;
Enemy enemies[MAX_ENEMIES];
Bullet bullets[MAX_BULLETS];
Orb orbs[MAX_ORBS];
Player player;
float playerSpeed = 2.0f;
int needXP = 4;
const float moveSpeed = 2.0f;
float enemySpeed = 1.0f;
int difficulty;
static float bulletFireTimer = 0.0f;
static float worldTime = 0.0f;


Camera2D camera = { 0 };



//----------------------------------------------------------------------------------
// Gameplay Screen Functions Definition
//----------------------------------------------------------------------------------

void AbsorbOrbs(Player* player) {
	for (int i = 0; i < MAX_ORBS; i++) {
		if (orbs[i].active) {
			if (CheckCollisionCircles(player->position, player->absorptionRadius, orbs[i].position, orbs[i].size)) {
				//make orbs move towards the player
				Vector2 direction = { 0.0f, 0.0f };

				direction.x = (player->position.x - orbs[i].position.x);
				direction.y = (player->position.y - orbs[i].position.y);

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
				if (CheckCollisionCircles(player->position, player->size, orbs[i].position, orbs[i].size)) {
					player->xp += orbs[i].exp;
					orbs[i].active = false;
				}
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
			orbs[i].size = 10; // Set the size of the orb
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

void UpdateEnemy(Enemy* enemy)
{
	static float nextEnemySpawnTime = 0.2f;

	if (enemy->health <= 0 && enemy->active) {

		SpawnOrb(enemy->position);
		enemy->active = false;
		enemy->position = (Vector2){ 0.0f, 0.0f };
		
	}
	if (!enemy->active && worldTime>= nextEnemySpawnTime) {

		


		
        

        //respawn enemy outside of the camera range of vision
        int respawnOffset = 100; // Adjust as needed
        float angle = (float)GetRandomValue(0, 360); // Random angle in degrees
        float distance = (float)(GetScreenWidth() / 2 + respawnOffset); // Distance from the center of the screen

        // Convert polar coordinates to Cartesian coordinates
        enemy->position = (Vector2){ 
            camera.target.x + cos(angle * PI / 180.0f) * distance,
            camera.target.y + sin(angle * PI / 180.0f) * distance
        };

		enemy->health = 2; // Reset health
		enemy->active = true;

		nextEnemySpawnTime += 0.2f;

	}


	Vector2 direction = { 0.0f, 0.0f };
	direction.x = (player.position.x-enemy->position.x);
	direction.y = (player.position.y-enemy->position.y);

    // Normalize the direction vector to ensure consistent speed in all directions
	
	float length = (float)sqrt((double)(direction.x * direction.x + direction.y * direction.y));
	if (length > 1.0f) {
		direction.x /= length;
		direction.y /= length;
	}
	// Apply the player's speed to their position
	enemy->position.x += direction.x * enemySpeed;
	enemy->position.y += direction.y * enemySpeed;



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

	if (bulletFireTimer >= 0.2f) { // Change 0.2f to your desired fire interval
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
			if (bullets[i].position.x < camera.target.x - GetScreenWidth() / 2 ||
				bullets[i].position.y < camera.target.y - GetScreenHeight() / 2 ||
				bullets[i].position.x > camera.target.x + GetScreenWidth() / 2 ||
				bullets[i].position.y > camera.target.y + GetScreenHeight() / 2) {
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
					//make a hit sound 
					
				}
			}
		}
	}
}

void DrawBullets(void) {
	for (int i = 0; i < MAX_BULLETS; i++) {
		if (bullets[i].active) {
			DrawCircleV(bullets[i].position, 2, WHITE);
		}
	}
}

void DrawHealthBar(Player* player) {
	int healthBarWidth = 50; // Adjust as needed
	int healthBarHeight = 10; // Adjust as needed
	int healthBarOffsetY = -30; // Adjust as needed

	Vector2 healthBarPosition = { player->position.x - healthBarWidth / 2, player->position.y - healthBarOffsetY };

	DrawRectangle(healthBarPosition.x, healthBarPosition.y, healthBarWidth, healthBarHeight, RED); // Background
	DrawRectangle(healthBarPosition.x, healthBarPosition.y, (healthBarWidth * player->health) / PLAYER_MAX_HEALTH, healthBarHeight, GREEN); // Foreground
}

void DrawXPBar(Player* player) {
	int xpBarWidth = GetScreenWidth(); // Full width of the screen
	int xpBarHeight = 40; // Adjust as needed
	int XPBarOffsetY = GetScreenHeight()/2; // Adjust as needed

	Vector2 xpBarPosition = { player->position.x - xpBarWidth / 2, player->position.y - XPBarOffsetY };

	DrawRectangle(xpBarPosition.x, xpBarPosition.y, xpBarWidth, xpBarHeight, DARKGRAY); // Background
	DrawRectangle(xpBarPosition.x, xpBarPosition.y, (xpBarWidth * player->xp) / needXP, xpBarHeight, BLUE); // Foreground

	char levelText[32];
	sprintf(levelText, "Level: %d", player->level);
	int textWidth = MeasureText(levelText, 35); // Measure the width of the text
	DrawText(levelText, xpBarPosition.x + xpBarWidth - textWidth - 60, xpBarPosition.y+4, 35, GRAY); // Adjust size and color as needed

}




void InitGameplayScreen(void)
{
	// Initialize player
	player.position = (Vector2){ GetScreenWidth() / 2, GetScreenHeight() / 2 };
	player.direction = (Vector2){ 0.0f, 0.0f };
	player.speed = (Vector2){ 0.0f, 0.0f };
	player.size = 20;
	player.color = BLUE;
	player.xp = 0;
	player.level = 1;
	player.health = PLAYER_MAX_HEALTH;
	player.absorptionRadius = 100;


	finishScreen = 0;

	//init camera
	camera.target = player.position;
	camera.offset = (Vector2){ GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
	camera.rotation = 0.0f;
	camera.zoom = 1.0f;

	//init enemies
	for (int i = 0; i < MAX_ENEMIES; i++)
	{
		
		enemies[i].speed = (Vector2){ 0.0f, 0.0f };
		enemies[i].size = 20;
		enemies[i].color = RED;
		enemies[i].health = 2;
		enemies[i].active = false;


	}

	//init bullets
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
		UpdateEnemy(&enemies[i], &player);
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
	DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), BLACK);

	BeginMode2D(camera);

	int gridSize = 40; // Change this to change the size of the grid cells

	int minX = (int)(camera.target.x - GetScreenWidth() / 2) / gridSize - 1;
	int minY = (int)(camera.target.y - GetScreenHeight() / 2) / gridSize - 1;
	int maxX = (int)(camera.target.x + GetScreenWidth() / 2) / gridSize + 1;
	int maxY = (int)(camera.target.y + GetScreenHeight() / 2) / gridSize + 1;

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
	// TODO: Unload GAMEPLAY screen variables here!
}

int FinishGameplayScreen(void)
{
	return finishScreen;
}
