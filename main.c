#include <stdbool.h>
#include <stdio.h>
#include <raylib.h>
#include <math.h>
#include <stdlib.h>

#define CORNFLOWER_BLUE (Color) {99, 149, 238, 255} 
#define TRANSPARENT_WHITE (Color) {255, 255, 255, 200}
#define SRC_WIDTH 320
#define SRC_HEIGHT 180
#define WINDOW_WIDTH 960
#define WINDOW_HEIGHT 540
#define TILE_SIZE 16
#define N_TILES_ROW (int) (SRC_WIDTH / TILE_SIZE)
#define N_TILES_COL (int) ((SRC_HEIGHT / TILE_SIZE) - 1)
#define N_TILES (N_TILES_ROW * N_TILES_COL)
#define TIME 60

// Yes, I know there is a standard bool header. I'm using this.
typedef enum { FALSE, TRUE } Bool;

typedef enum { NORTH, EAST, SOUTH, WEST, NONE } Direction;

typedef enum { MENU, TUTORIAL, GAME, SCORECARD } GameState;

typedef struct {
	Texture2D tex; // Could probably optimize by storing an index instead of copying
								 // the texture data into every tile
	Bool destroyed; // Could be a enum now that I'm thinking of it.
	Bool conveyer;
	Bool spawner;
	Bool collector;
	Direction dir;
	int rand;
} Tile;

typedef struct {
	Texture2D tex;
	Vector2 pos;
	Vector2 dest_pos;
	int used_tile_i;
	Direction dir;
	Bool active;
	Bool bomb;
	float time;
} Item;

typedef struct {
	int spawners[2];
	int collectors[2];
	int paths;
	int difficulty;
} Level;

void ResetItem(int, Level);
void DrawTile(Texture2D, Vector2, Vector2, int);
void LoadLevel(Level);

Tile tiles[N_TILES] = {0};

Item items[N_TILES] = {0};

Level level_1 = {.spawners = {22}, .collectors = {178}, .paths = 1, .difficulty = 4};

int score;
float timer;
float interval;
Vector2 cursor_pos;
int active_tile_index;
int last_active_tile_index;
float degree;

Texture2D item_tex;
Image tile_img;
Texture2D tile_tex;

Texture2D bottom_bar_tex;
Direction selected_tile_dir = NORTH;

Texture2D menu_bg_tex;
Texture2D scorecard_tex;
Texture2D web_start_tex;

GameState gs = MENU;
Level cur_level;

Music caketown;
Music stage_select;
Sound explosion;
Sound pickup;
Sound placement;

int main(void)
{

	InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Mini Jam 214");
	InitAudioDevice();
	SetTargetFPS(60);

	RenderTexture2D target;
	Rectangle target_src_rec;
	Rectangle target_dst_rec;
	target = LoadRenderTexture(SRC_WIDTH, SRC_HEIGHT);
	target_src_rec = (Rectangle) {0, 0, SRC_WIDTH, -SRC_HEIGHT};
	target_dst_rec = (Rectangle) {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};

	bottom_bar_tex = LoadTexture("assets/bottom_bar.png");

	menu_bg_tex = LoadTexture("assets/menu_background.png");	

	scorecard_tex = LoadTexture("assets/scorecard.png");

	web_start_tex = LoadTexture("assets/web_start.png");

	item_tex = LoadTexture("assets/firework_sprites.png");
	// Move into seperate TileSetup function?
	// Could load the texture in one line, but I'm pretty sure it's the same either
	// way under the hood?
	tile_img = LoadImage("assets/tile_sprites.png");
	tile_tex = LoadTextureFromImage(tile_img);

	caketown = LoadMusicStream("assets/caketown.mp3");
	stage_select = LoadMusicStream("assets/stage_select.wav");
	SetMusicVolume(stage_select, 0.075);
	explosion = LoadSound("assets/explosion.wav");
	pickup = LoadSound("assets/pickup.wav");
	placement = LoadSound("assets/placement.wav");
	SetSoundVolume(placement, 0.5);
	SetSoundVolume(pickup, 0.5);

	SetRandomSeed((int) GetTime());

	// Load level 1
	LoadLevel(level_1);

	// PlayMusicStream(caketown);

	while (!WindowShouldClose())
	{
		while (!WindowShouldClose()) {
			if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
				break;
			}
			BeginTextureMode(target);
				ClearBackground(RAYWHITE);
				DrawTexture(web_start_tex, 0, 0, WHITE);
			EndTextureMode();
			BeginDrawing();
				ClearBackground(RAYWHITE);
				DrawTexturePro(target.texture, target_src_rec, target_dst_rec, (Vector2) { 0.0f, 0.0f }, 0.0f, WHITE);
			EndDrawing();
		}
		while (gs == MENU && !WindowShouldClose()) {

			if (!IsMusicStreamPlaying(caketown)) PlayMusicStream(caketown);
			UpdateMusicStream(caketown);
			if (IsKeyPressed(KEY_ENTER)) {
				StopMusicStream(caketown);
				// load level
				LoadLevel(level_1);
				gs = GAME;
			}			
			BeginTextureMode(target);
				ClearBackground(RAYWHITE);
				DrawTexture(menu_bg_tex, 0, 0, WHITE);
			EndTextureMode();
			BeginDrawing();
				ClearBackground(RAYWHITE);
				DrawTexturePro(target.texture, target_src_rec, target_dst_rec, (Vector2) { 0.0f, 0.0f }, 0.0f, WHITE);
			EndDrawing();
		}

		while (gs == GAME && !WindowShouldClose()) {
			timer += GetFrameTime();
			interval += GetFrameTime();

			if (!IsMusicStreamPlaying(stage_select)) PlayMusicStream(stage_select);
			UpdateMusicStream(stage_select);

			// REMEMBER TO REMOVE THIS
			if (timer > TIME || IsKeyPressed(KEY_P)) gs = SCORECARD;

			if (IsKeyPressed(KEY_ONE)) {
				selected_tile_dir = NORTH;
			}
			if (IsKeyPressed(KEY_TWO)) {
				selected_tile_dir = EAST;
			}
			if (IsKeyPressed(KEY_THREE)) {
				selected_tile_dir = SOUTH;
			}
			if (IsKeyPressed(KEY_FOUR)) {
				selected_tile_dir = WEST;
			}
			if (IsKeyPressed(KEY_TAB) || GetMouseWheelMove() > 0) {
					if (selected_tile_dir < 3) {
						selected_tile_dir++;
					} else {
						selected_tile_dir = NORTH;
					}
			}
			if (GetMouseWheelMove() < 0) {
				if (selected_tile_dir > 0) {
					selected_tile_dir--;
				} else {
					selected_tile_dir = WEST;
				}
			}

			if (floorf(interval) > N_TILES) interval = 0.0f;

			if (GetMousePosition().x < WINDOW_WIDTH && GetMousePosition().y < WINDOW_HEIGHT) {
				cursor_pos = (Vector2) {GetMousePosition().x / 3, GetMousePosition().y / 3};
			}

			// Could optimize using sorting algorithim
			// Must use the index, because simply getting the active tile makes a copy.
			// I wonder if I could get the pointer to the active tile. However, this works
			// for now.
			active_tile_index = (int) (cursor_pos.y / TILE_SIZE) * N_TILES_ROW + (int) (cursor_pos.x / TILE_SIZE);
			if (active_tile_index >= 0 && active_tile_index < N_TILES) {
				if (tiles[active_tile_index].spawner == FALSE && tiles[active_tile_index].collector == FALSE) {
					if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON) && active_tile_index != last_active_tile_index && tiles[active_tile_index].conveyer != TRUE && tiles[active_tile_index].destroyed == FALSE) {
						if (!IsSoundPlaying(placement)) PlaySound(placement);
						tiles[active_tile_index].conveyer = TRUE;
						tiles[active_tile_index].dir = selected_tile_dir;
						last_active_tile_index = active_tile_index;
					} else if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON) && tiles[active_tile_index].conveyer == TRUE) {
						if (!IsSoundPlaying(placement)) PlaySound(placement);
						if (tiles[active_tile_index].dir < 3) {
							tiles[active_tile_index].dir++;
						} else {
							tiles[active_tile_index].dir = NORTH;
						}
						last_active_tile_index = active_tile_index;
					}
				}
				if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
					if (!IsSoundPlaying(placement)) PlaySound(placement);
					if (tiles[active_tile_index].conveyer == TRUE) {
						tiles[active_tile_index].conveyer = FALSE;
						last_active_tile_index = active_tile_index;
					}
				}
			}
			if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON) == 0) last_active_tile_index = -1;

			// Iterate over all spawners in level
			int i = cur_level.spawners[0];
			if (i + 1 > N_TILES_ROW && tiles[i-20].conveyer == TRUE) {
				tiles[cur_level.spawners[0]].dir = NORTH;
			} else if ((i + 1) % 20 != 0 && tiles[i+1].conveyer == TRUE) {
				tiles[cur_level.spawners[0]].dir = EAST;
			} else if (i + 21 < N_TILES - 20 && tiles[i+20].conveyer == TRUE) {
					tiles[cur_level.spawners[0]].dir = SOUTH;
			} else if (i % N_TILES_ROW > 0 && tiles[i-1].conveyer == TRUE) {
					tiles[cur_level.spawners[0]].dir = WEST;
			}

			// printf("%d\n", tiles[cur_level.spawners[0]].dir);
			// printf("%f\n", interval);
			if (interval > N_TILES) interval -= N_TILES;

			if (tiles[cur_level.spawners[0]].dir == NONE) interval = -1;

			if (interval > 0 && items[(int) floorf(interval)].active == FALSE) {
				items[(int) floorf(interval)].dir = tiles[cur_level.spawners[0]].dir;
				items[(int) floorf(interval)].active = TRUE;
				items[(int) floorf(interval)].dest_pos = items[(int) floorf(interval)].pos; // Because the rendertexture is flipped
				switch (items[(int) floorf(interval)].dir) {
					case NORTH:
						items[(int) floorf(interval)].dest_pos.y -= TILE_SIZE; // Because the rendertexture is flipped
						break;
					case EAST:
						items[(int) floorf(interval)].dest_pos.x += TILE_SIZE;
						break;
					case SOUTH:
						items[(int) floorf(interval)].dest_pos.y += TILE_SIZE;
						break;
					case WEST:
						items[(int) floorf(interval)].dest_pos.x -= TILE_SIZE;
						break;
					default:
						break;
				}
				if (GetRandomValue(0, cur_level.difficulty) == 0) {
					items[(int) floorf(interval)].bomb = TRUE;
					items[(int) floorf(interval)].time = 0;
				}
			}
			// Debug
			// printf("Debug 2:\n");
			// printf("Pos: %f, %f\n", items[5].pos.x, items[5].pos.y);
			// printf("Dest Pos: %f, %f\n", items[5].dest_pos.x, items[5].dest_pos.y);
			// printf("Guide tile: %d\n", items[5].used_tile_i);
			// printf("Score: %d\n", score);
			float dt = GetFrameTime();
			for (int i = 0; i < N_TILES; i++) {
				if (items[i].active == TRUE) {
					if (items[i].bomb == TRUE) {
						items[i].time += GetFrameTime();
						// Disarm bomb, currently also deletes tile
						// if (CheckCollisionPointRec(cursor_pos, 
						// 			(Rectangle) {
						// 				items[i].pos.x, 
						// 				items[i].pos.y,
						// 				TILE_SIZE,
						// 				TILE_SIZE})
						// 		&& IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
						// 	ResetItem(i, cur_level);
						// }
					}
					// Explode bomb
					if (items[i].time > 6 && items[i].bomb == TRUE) {
						// ADD if time allows, a larger area, for now just one tile.
						// check if out of bounds
						PlaySound(explosion);
						tiles[items[i].used_tile_i].destroyed = TRUE;
						tiles[items[i].used_tile_i].conveyer = FALSE;
						// reset item
						ResetItem(i, cur_level);
					}
					// Handle item reaching next node destination
					if ((int) items[i].dest_pos.x == (int) items[i].pos.x && (int) items[i].dest_pos.y == (int) items[i].pos.y) {
						// Fix for multiple collectors
						switch (items[i].dir) {
							case NORTH:
								if (items[i].used_tile_i > N_TILES_ROW - 1) items[i].used_tile_i -= 20;
								break;
							case EAST:
								if ((items[i].used_tile_i + 1) % N_TILES_ROW < 20) items[i].used_tile_i++;
								break;
							case SOUTH:
								if (items[i].used_tile_i < N_TILES - N_TILES_ROW - 1) items[i].used_tile_i += 20;
								break;
							case WEST:
								// if ((items[i].used_tile_i + 1) % N_TILES_ROW > 0) items[i].used_tile_i--;
								if ((items[i].used_tile_i) % N_TILES_ROW > 0) items[i].used_tile_i--;
								break;
							default:
								break;
						}
						// Collect item
						if (items[i].used_tile_i == cur_level.collectors[0]) {
							if (items[i].bomb == FALSE) {
								PlaySound(pickup);
								score++;
							}
							ResetItem(i, cur_level);
						}
						items[i].dir = tiles[items[i].used_tile_i].dir;
						switch (items[i].dir) {
							case NORTH:
								items[i].dest_pos.y -= TILE_SIZE; // Because the rendertexture is flipped
								break;
							case EAST:
								items[i].dest_pos.x += TILE_SIZE;
								break;
							case SOUTH:
								items[i].dest_pos.y += TILE_SIZE;
								break;
							case WEST:
								items[i].dest_pos.x -= TILE_SIZE;
								break;
							default:
								break;
						}
					}
					// Handle item over destroyed area
					if (tiles[items[i].used_tile_i].destroyed == TRUE) {
						ResetItem(i, cur_level);
					}
					// Move item
					Tile t = tiles[items[i].used_tile_i];
					if (t.conveyer == TRUE || t.spawner == TRUE) {
						switch (items[i].dir) {
							case NORTH:
								items[i].pos.y -= TILE_SIZE * 1.5 * dt; // Because the rendertexture is flipped
								break;
							case EAST:
								items[i].pos.x += TILE_SIZE * 1.5 * dt;
								break;
							case SOUTH:
								items[i].pos.y += TILE_SIZE * 1.5 * dt;
								break;
							case WEST:
								items[i].pos.x -= TILE_SIZE * 1.5 * dt;
								break;
							default:
								break;
						}
					}
					// Position bounds
					if (items[i].dest_pos.y < 0) items[i].dest_pos.y = 0;
					if (items[i].dest_pos.y > SRC_HEIGHT) items[i].dest_pos.y = SRC_HEIGHT;
					if (items[i].dest_pos.x < 0) items[i].dest_pos.x = 0;
					if (items[i].dest_pos.x > SRC_WIDTH) items[i].dest_pos.x = SRC_WIDTH;
					if (items[i].pos.y < 0) items[i].pos.y = 0;
					if (items[i].pos.y > SRC_HEIGHT) items[i].pos.y = SRC_HEIGHT;
					if (items[i].pos.x < 0) items[i].pos.x = 0;
					if (items[i].pos.x > SRC_WIDTH) items[i].pos.x = SRC_WIDTH;
				}
			}

			BeginTextureMode(target);
				ClearBackground(CORNFLOWER_BLUE);
				// Draw Tiles
				// Could simplify drawing code by passing an x value to a drawing function
				// since that's the only differentiating aspect
				for (int i = 0; i < N_TILES_COL; i++) {
					for (int j = 0; j < N_TILES_ROW; j++) {
						Tile t = tiles[N_TILES_ROW * i + j];
						if (t.conveyer == FALSE) {
							if (t.spawner == TRUE) {
								DrawTile(tile_tex, (Vector2) {j * TILE_SIZE, i * TILE_SIZE}, (Vector2) {6 * TILE_SIZE, t.dir * TILE_SIZE}, 0);
							} else if (t.collector == TRUE) {
								// printf("%d\n", N_TILES_ROW * i + j);
								// printf("%d\n", cur_level.collectors[1]);
								DrawTile(tile_tex, (Vector2) {j * TILE_SIZE, i * TILE_SIZE}, (Vector2) {7 * TILE_SIZE, 0}, 0);
							} else if (t.destroyed == FALSE) {
								DrawTile(tile_tex, (Vector2) {j * TILE_SIZE, i * TILE_SIZE}, (Vector2) {0, 0}, t.rand);
							} else {
								DrawTile(tile_tex, (Vector2) {j * TILE_SIZE, i * TILE_SIZE}, (Vector2) {TILE_SIZE, 0}, t.rand);
							}	
						} else {
							DrawTile(tile_tex, (Vector2) {j * TILE_SIZE, i * TILE_SIZE}, (Vector2) {TILE_SIZE * 2 + TILE_SIZE * t.dir, 0}, t.rand);
						}	
					}
				}
				
				// Draw items on conveyers
				for (int i = 0; i < N_TILES; i++) {
					if (items[i].active == TRUE) {
						if (items[i].bomb == FALSE) {
							switch (items[i].dir) {
								case NORTH:
									degree = 0.0f;
									break;
								case EAST:
									degree = 90.0f;
									break;
								case SOUTH:
									degree = 180.0f;
									break;
								case WEST:
									degree = 270.0f;
									break;
								default:
									break;
							}

							// I think turning the texture messes it up a bit, might be rotated
							// based on the texture, not the source rect.
							// DrawTexturePro(item_tex, (Rectangle) {0, 0, TILE_SIZE, TILE_SIZE}, (Rectangle) {items[i].pos.x, items[i].pos.y, TILE_SIZE, TILE_SIZE}, (Vector2) { 0, 0 }, degree, WHITE);
							DrawTexturePro(item_tex, (Rectangle) {0, 0, TILE_SIZE, TILE_SIZE}, (Rectangle) {items[i].pos.x, items[i].pos.y, TILE_SIZE, TILE_SIZE}, (Vector2) { 0, 0 }, 0.0f, WHITE);
						} else {
							DrawTexturePro(item_tex, (Rectangle) {TILE_SIZE * 3, 0, TILE_SIZE, TILE_SIZE}, (Rectangle) {items[i].pos.x, items[i].pos.y, TILE_SIZE, TILE_SIZE}, (Vector2) { 0, 0 }, 0.0f, WHITE);
						}
					}
				}
				DrawTexture(bottom_bar_tex, 0, 160, WHITE);
				// DrawText(TextFormat("Score: %d", score), 16, 160, 12, WHITE);
				DrawText(TextFormat("%d", TIME - (int) timer), 18, 163, 16, WHITE);

				// if (tiles[(int) floor(cursor_pos.x / TILE_SIZE) + (int) floor(cursor_pos.y / TILE_SIZE) * N_TILES_ROW - 1].conveyer == FALSE) {
				if (tiles[active_tile_index].conveyer == FALSE && tiles[active_tile_index].spawner == FALSE && tiles[active_tile_index].collector == FALSE){
					DrawTile(tile_tex, (Vector2) {floorf(cursor_pos.x / TILE_SIZE) * TILE_SIZE, floorf(cursor_pos.y / TILE_SIZE) * TILE_SIZE}, (Vector2) {TILE_SIZE * 2 + selected_tile_dir * TILE_SIZE, 0}, 67);
				}
			EndTextureMode();

			BeginDrawing();
				ClearBackground(RAYWHITE);
				DrawTexturePro(target.texture, target_src_rec, target_dst_rec, (Vector2) { 0.0f, 0.0f }, 0.0f, WHITE);
			EndDrawing();
		}

		while (gs == SCORECARD && !WindowShouldClose()) {
			if (IsKeyPressed(KEY_ENTER)) {
				gs = MENU;
			}
			BeginTextureMode(target);
				DrawTexture(scorecard_tex, 112, 16, WHITE);
				const char* s = TextFormat("%d", score);
				DrawText(s, SRC_WIDTH / 2 - MeasureText(s, 16) / 2, 64, 16, WHITE);
			EndTextureMode();
			BeginDrawing();
				ClearBackground(RAYWHITE);
				DrawTexturePro(target.texture, target_src_rec, target_dst_rec, (Vector2) { 0.0f, 0.0f }, 0.0f, WHITE);
			EndDrawing();
		}
	
	}
	UnloadMusicStream(caketown);   // Unload music stream buffers from RAM
	UnloadMusicStream(stage_select);   // Unload music stream buffers from RAM
	UnloadSound(explosion);
	UnloadSound(pickup);
	UnloadSound(placement);

	CloseAudioDevice();         // Close audio device (music streaming is automatically stopped)
	UnloadRenderTexture(target);
	CloseWindow();
	return 0;
}

void ResetItem(int i, Level l) {
	items[i].used_tile_i = l.spawners[0];
	items[i].active = FALSE;
	items[i].pos = (Vector2) {l.spawners[0] % N_TILES_ROW * TILE_SIZE, floorf(l.spawners[0] / N_TILES_ROW) * TILE_SIZE};
	items[i].used_tile_i = l.spawners[0];
	items[i].dir = tiles[l.spawners[0]].dir;
	// items[i].dest_pos = items[i].pos;
	items[i].time = 0;
}

void DrawTile(Texture2D t, Vector2 pos, Vector2 offset, int r) {
	// SIX SEEEEEVVVVEEEEN
	if (r == 67) {
		DrawTextureRec(
			t, 
			(Rectangle) {
				offset.x, 
				0, 
				TILE_SIZE, 
				TILE_SIZE
			}, 
			pos,
			TRANSPARENT_WHITE
		);
	} else if (offset.x == 0) {
		DrawTextureRec(
			tile_tex, 
			(Rectangle) {
				0, 
				r * TILE_SIZE, 
				TILE_SIZE, 
				TILE_SIZE
			}, 
			pos,
			WHITE
		);
	} else if (offset.x == 6 * TILE_SIZE) {
		DrawTextureRec(
			t, 
			(Rectangle) {
				offset.x, 
				offset.y, 
				TILE_SIZE, 
				TILE_SIZE
			}, 
			pos,
			WHITE
		);
	} else {
		DrawTextureRec(
			t, 
			(Rectangle) {
				offset.x, 
				0, 
				TILE_SIZE, 
				TILE_SIZE
			}, 
			pos,
			WHITE
		);
	}
}

void LoadLevel(Level l) {
	cur_level = l;
	score = 0;
	timer = 0.0f;
	interval = 0.0f;
	cursor_pos = (Vector2) {0};
	active_tile_index = 0;
	last_active_tile_index = -1;
	for (int i = 0; i < N_TILES; i++) {
		tiles[i].collector = FALSE;
	}
	tiles[cur_level.spawners[0]].spawner = TRUE;
	tiles[cur_level.spawners[0]].dir = NONE;
	tiles[cur_level.collectors[0]].collector = TRUE;
	for (int i = 0; i < N_TILES; i++) {
		ResetItem(i, cur_level);
		switch (items[i].dir) {
			case NORTH:
				items[i].dest_pos.y -= TILE_SIZE; // Because the rendertexture is flipped
				break;
			case EAST:
				items[i].dest_pos.x += TILE_SIZE;
				break;
			case SOUTH:
				items[i].dest_pos.y += TILE_SIZE;
				break;
			case WEST:
				items[i].dest_pos.x -= TILE_SIZE;
				break;
			default:
				break;
		}
	}

	for (int i = 0; i < N_TILES; i++) {
		tiles[i].tex = tile_tex;
		tiles[i].rand = GetRandomValue(0, 15); 
		if (tiles[i].rand < 11) {
			tiles[i].rand = 0;
		} else if (tiles[i].rand < 12) {
			tiles[i].rand = 1;
		} else if (tiles[i].rand < 14) {
			tiles[i].rand = 2;
		} else if (tiles[i].rand < 15) {
			tiles[i].rand = 3;
		} else {
			tiles[i].rand = 4;
		}
	}
}
