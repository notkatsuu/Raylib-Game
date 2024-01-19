#include "raylib.h"
#include "screens.h"

//----------------------------------------------------------------------------------
// Module Variables Definition (local)
//----------------------------------------------------------------------------------


typedef struct {
	Vector2 position;
	Vector2 direction;
	int size;
	Vector2 speed;
	Color color;
	void (*UpdatePosition)(struct Player*);
	void (*Draw)(struct Player*);

} Player;

typedef struct {
	Vector2 position;
	int size;
	Vector2 speed;
	Color color;
	void (*UpdatePosition)(struct Enemy*, struct Player*);
	void (*Draw)(struct Enemy*);
} Enemy;

typedef struct Bullet {
	Vector2 position;
	Vector2 direction;
	float speed;
	bool active;
} Bullet;



#define MAX_ENEMIES 5
Enemy enemies[MAX_ENEMIES];

#define MAX_BULLETS 50
Bullet bullets[MAX_BULLETS];

static int finishScreen = 0;

Player player;
const float moveSpeed = 2.0f;

Camera2D camera = { 0 };



//----------------------------------------------------------------------------------
// Gameplay Screen Functions Definition
//----------------------------------------------------------------------------------

void UpdatePlayerPosition(Player* player)
{
	// Create a direction vector based on keyboard input
	Vector2 direction = { 0.0f, 0.0f };
	if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP)) direction.y -= 1.0f;
	if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN)) direction.y += 1.0f;
	if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT)) direction.x -= 1.0f;
	if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT)) direction.x += 1.0f;

	// Normalize the direction vector to ensure consistent speed in all directions
	float length = sqrt(direction.x * direction.x + direction.y * direction.y);
	if (length > 1.0f) {
		direction.x /= length;
		direction.y /= length;
	}

	player->direction.x = direction.x;
	player->direction.y = direction.y;

	// Apply the direction to the player's speed
	player->speed.x = direction.x * moveSpeed;
	player->speed.y = direction.y * moveSpeed;

	// Apply the player's speed to their position
	player->position.x += player->speed.x;
	player->position.y += player->speed.y;
}

void DrawPlayer(Player* player)
{
	// Draw the player
	DrawCircleV(player->position, player->size, player->color);
}

void UpdateEnemyPosition(Enemy* enemy)
{
	Vector2 direction = { 0.0f, 0.0f };
	direction.x = (player.position.x-enemy->position.x);
	direction.y = (player.position.y-enemy->position.y);

    // Normalize the direction vector to ensure consistent speed in all directions
	
	float length = sqrt((double)(direction.x * direction.x + direction.y * direction.y));
	if (length > 1.0f) {
		direction.x /= length;
		direction.y /= length;
	}
	// Apply the player's speed to their position
	enemy->position.x += direction.x/200;
	enemy->position.y += direction.y/200;
}


void DrawEnemy(Enemy* enemy)
{
	// Draw the enemy
	DrawCircleV(enemy->position, enemy->size, enemy->color);
}


void FireBullet(void) {
	if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
		for (int i = 0; i < MAX_BULLETS; i++) {
			if (!bullets[i].active) {
				if (player.direction.x == 0 && player.direction.y == 0) return;
				bullets[i].position = player.position;

				// Use player's direction
				bullets[i].direction = player.direction;

				bullets[i].active = true;
				break;
			}
		}
	}
}

void UpdateBullets(void) {
	for (int i = 0; i < MAX_BULLETS; i++) {
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



void InitGameplayScreen(void)
{
	// Initialize player
	player.position = (Vector2){ GetScreenWidth() / 2, GetScreenHeight() / 2 };
	player.direction = (Vector2){ 0.0f, 0.0f };
	player.speed = (Vector2){ 0.0f, 0.0f };
	player.size = 20;
	player.color = BLUE;
	player.UpdatePosition = UpdatePlayerPosition;
	player.Draw = DrawPlayer;

	finishScreen = 0;

	//init camera
	camera.target = player.position;
	camera.offset = (Vector2){ GetScreenWidth() / 2.0f, GetScreenHeight() / 2.0f };
	camera.rotation = 0.0f;
	camera.zoom = 1.0f;

	//init enemies
	for (int i = 0; i < MAX_ENEMIES; i++)
	{
		enemies[i].position = (Vector2){ GetRandomValue(0, GetScreenWidth()), GetRandomValue(0, GetScreenHeight()) };
		enemies[i].speed = (Vector2){ 0.0f, 0.0f };
		enemies[i].size = 20;
		enemies[i].color = RED;
		enemies[i].UpdatePosition = UpdateEnemyPosition;
		enemies[i].Draw = DrawEnemy;
	}

	//init bullets
	for (int i = 0; i < MAX_BULLETS; i++) {
		bullets[i].speed = 3;
		bullets[i].active = false;
	}
}


void UpdateGameplayScreen(void)
{


	// Update player position
	player.UpdatePosition(&player);

	//update enemies position
	for (int i = 0; i < MAX_ENEMIES; i++)
	{
		enemies[i].UpdatePosition(&enemies[i], &player);
	}

	// Update camera target
	camera.target = player.position;

	// Move bulelts
	UpdateBullets();

	FireBullet();
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
		enemies[i].Draw(&enemies[i]);
	}

	// Draw the player
	player.Draw(&player);


	// Draw bullets
	DrawBullets();

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
