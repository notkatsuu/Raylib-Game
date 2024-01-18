/**********************************************************************************************
*
*   raylib - Advance Game template
*
*   Gameplay Screen Functions Definitions (Init, Update, Draw, Unload)
*
*   Copyright (c) 2014-2022 Ramon Santamaria (@raysan5)
*
*   This software is provided "as-is", without any express or implied warranty. In no event
*   will the authors be held liable for any damages arising from the use of this software.
*
*   Permission is granted to anyone to use this software for any purpose, including commercial
*   applications, and to alter it and redistribute it freely, subject to the following restrictions:
*
*     1. The origin of this software must not be misrepresented; you must not claim that you
*     wrote the original software. If you use this software in a product, an acknowledgment
*     in the product documentation would be appreciated but is not required.
*
*     2. Altered source versions must be plainly marked as such, and must not be misrepresented
*     as being the original software.
*
*     3. This notice may not be removed or altered from any source distribution.
*
**********************************************************************************************/

#include "raylib.h"
#include "screens.h"

//----------------------------------------------------------------------------------
// Module Variables Definition (local)
//----------------------------------------------------------------------------------
const float drag = 0.9f;
const float moveSpeed = 1.0f;


typedef struct {
	Vector2 position;
	int size;
	Vector2 speed;
	float acceleration;
	float rotation;
	Vector3 collider;
	Color color;
	void (*UpdatePosition)(struct Player*);
	void (*Draw)(struct Player*);

} Player;


static int framesCounter = 0;
static int finishScreen = 0;

Player player;

//----------------------------------------------------------------------------------
// Gameplay Screen Functions Definition
//----------------------------------------------------------------------------------


// Gameplay Screen Initialization logic


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

	// Apply the direction to the player's speed
	player->speed.x += direction.x * moveSpeed;
	player->speed.y += direction.y * moveSpeed;

	// Apply drag to the player's speed (this creates the momentum effect)
	player->speed.x *= drag;
	player->speed.y *= drag;

	// Apply the player's speed to their position
	player->position.x += player->speed.x;
	player->position.y += player->speed.y;
}

void DrawPlayer(Player* player)
{
	// Draw the player
	DrawCircleV(player->position, player->size, player->color);
}

void InitGameplayScreen(void)
{
	// Initialize player
	player.position = (Vector2){ GetScreenWidth() / 2, GetScreenHeight() / 2 };
	player.speed = (Vector2){0.0f, 0.0f};
	
	player.size = 20;
	player.acceleration = 0.0f;
	player.rotation = 0.0f;
	player.collider = (Vector3){ 0.0f, 0.0f, 0.0f };
	player.color = BLUE;
	player.UpdatePosition = UpdatePlayerPosition;
	player.Draw = DrawPlayer;


	finishScreen = 0;


}



// Gameplay Screen Update logic
void UpdateGameplayScreen(void)
{
	// TODO: Update GAMEPLAY screen variables here!

	// Update player position

	player.UpdatePosition(&player);


	// Press enter or tap to change to ENDING screen
	/*if (has to end)
	{
		finishScreen = 1;
		//PlaySound(fxCoin);
	}*/
}




// Gameplay Screen Draw logic
void DrawGameplayScreen(void)
{
	// TODO: Draw GAMEPLAY screen here!
	DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), BLACK);
	Vector2 pos = { 20, 10 };
	//DrawTextEx(font, "GAMEPLAY SCREEN", pos, font.baseSize * 3.0f, 4, MAROON);
	//DrawText("PRESS ENTER or TAP to JUMP to ENDING SCREEN", 130, 220, 20, MAROON);


	// Draw the player
	player.Draw(&player);



}

// Gameplay Screen Unload logic
void UnloadGameplayScreen(void)
{
	// TODO: Unload GAMEPLAY screen variables here!
}

// Gameplay Screen should finish?
int FinishGameplayScreen(void)
{
	return finishScreen;
}