#include <stdbool.h>
#include <stdio.h>
#include <raylib.h>
#include <math.h>
#include <stdlib.h>

#define CORNFLOWER_BLUE (Color) {99, 149, 238, 255} 
#define SRC_WIDTH 320
#define SRC_HEIGHT 180
#define WINDOW_WIDTH 960
#define WINDOW_HEIGHT 540
#define TILE_SIZE 16
#define N_TILES_ROW (int) (SRC_WIDTH / TILE_SIZE)
#define N_TILES_COL (int) ((SRC_HEIGHT / TILE_SIZE) - 1)
#define N_TILES (N_TILES_ROW * N_TILES_COL)

// Yes, I know there is a standard bool header. I'm using this.
typedef enum { FALSE, TRUE } Bool;

typedef enum { NORTH, EAST, SOUTH, WEST } Direction;

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
void DrawTile(Texture2D, Vector2, int, int);

Tile tiles[N_TILES] = {0};

Item items[N_TILES] = {0};

Level level_1 = {.spawners = {20}, .collectors = {180}, .paths = 1, .difficulty = 5};

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

int main(void)
{

	InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Mini Jam 214");
	SetTargetFPS(60);

	RenderTexture2D target;
	Rectangle target_src_rec;
	Rectangle target_dst_rec;
	target = LoadRenderTexture(SRC_WIDTH, SRC_HEIGHT);
	target_src_rec = (Rectangle) {0, 0, SRC_WIDTH, -SRC_HEIGHT};
	target_dst_rec = (Rectangle) {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};

	item_tex = LoadTexture("assets/firework_sprites.png");
	// Move into seperate TileSetup function?
	// Could load the texture in one line, but I'm pretty sure it's the same either
	// way under the hood?
	tile_img = LoadImage("assets/tile_sprites.png");
	tile_tex = LoadTextureFromImage(tile_img);
	for (int i = 0; i < N_TILES; i++) {
		tiles[i].tex = tile_tex;
		tiles[i].rand = GetRandomValue(0, 12); 
		if (tiles[i].rand < 6) {
			tiles[i].rand = 0;
		} else if (tiles[i].rand < 9) {
			tiles[i].rand = 1;
		} else if (tiles[i].rand < 11) {
			tiles[i].rand = 2;
		} else if (tiles[i].rand < 12) {
			tiles[i].rand = 3;
		} else {
			tiles[i].rand = 4;
		}
	}
	
	SetRandomSeed((int) GetTime());
	// Load level 1
	Level cur_level = level_1;
	score = 0;
	timer = 0.0f;
	interval = 0.0f;
	cursor_pos = (Vector2) {0};
	active_tile_index = 0;
	last_active_tile_index = -1;
	tiles[cur_level.spawners[0]].spawner = TRUE;
	tiles[cur_level.spawners[0]].dir = NORTH;
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
		}
	}


	while (!WindowShouldClose())
	{
		timer += GetFrameTime();
		interval += GetFrameTime();
		if (interval > N_TILES) interval -= N_TILES;
		if (items[(int) floorf(interval)].active == FALSE) {
			items[(int) floorf(interval)].active = TRUE;
			if (GetRandomValue(0, cur_level.difficulty) == 0) {
				items[(int) floorf(interval)].bomb = TRUE;
				items[(int) floorf(interval)].time = 0;
			}
		}
		if (floorf(timer) > N_TILES) timer = 0.0f;

		if (GetMousePosition().x < WINDOW_WIDTH && GetMousePosition().y < WINDOW_HEIGHT) {
			cursor_pos = (Vector2) {GetMousePosition().x / 3, GetMousePosition().y / 3};
		}

		// Could optimize using sorting algorithim
		// Must use the index, because simply getting the active tile makes a copy.
		// I wonder if I could get the pointer to the active tile. However, this works
		// for now.
		active_tile_index = (int) (cursor_pos.y / TILE_SIZE) * N_TILES_ROW + (int) (cursor_pos.x / TILE_SIZE);
		if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON) == 0) last_active_tile_index = -1;

		if (tiles[active_tile_index].spawner == FALSE && tiles[active_tile_index].collector == FALSE) {
			if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON) && active_tile_index != last_active_tile_index && tiles[active_tile_index].conveyer != TRUE && tiles[active_tile_index].destroyed == FALSE) {
				tiles[active_tile_index].conveyer = TRUE;
				last_active_tile_index = active_tile_index;
			} else if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON) && tiles[active_tile_index].conveyer == TRUE) {
				if (tiles[active_tile_index].dir < 3) {
					tiles[active_tile_index].dir++;
				} else {
					tiles[active_tile_index].dir = NORTH;
				}
				last_active_tile_index = active_tile_index;
			}
		}
		if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
			if (tiles[active_tile_index].conveyer == TRUE) {
				tiles[active_tile_index].conveyer = FALSE;
				last_active_tile_index = active_tile_index;
			}
		}

		// Debug
		// printf("Debug 2:\n");
		// printf("Pos: %f, %f\n", items[0].pos.x, items[0].pos.y);
		// printf("Dest Pos: %f, %f\n", items[0].dest_pos.x, items[0].dest_pos.y);
		// printf("Guide tile: %d\n", items[0].used_tile_i);
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
				if (items[i].time > 10 && items[i].bomb == TRUE) {
					// ADD if time allows, a larger area, for now just one tile.
					// check if out of bounds
					// if (items[i].used_tile_i - 20)
					tiles[items[i].used_tile_i].destroyed = TRUE;
					tiles[items[i].used_tile_i].conveyer = FALSE;
					printf("Boom!");
					// tiles[items[i].used_tile_i-1].destroyed = TRUE;
					// tiles[items[i].used_tile_i-1].conveyer = FALSE;
					// tiles[items[i].used_tile_i+1].destroyed = TRUE;
					// tiles[items[i].used_tile_i+1].conveyer = FALSE;
					// tiles[items[i].used_tile_i+21].destroyed = TRUE;
					// tiles[items[i].used_tile_i+21].conveyer = FALSE;
					// tiles[items[i].used_tile_i+20].destroyed = TRUE;
					// tiles[items[i].used_tile_i+20].conveyer = FALSE;
					// tiles[items[i].used_tile_i+19].destroyed = TRUE;
					// tiles[items[i].used_tile_i+19].conveyer = FALSE;
					// reset item
					ResetItem(i, cur_level);
				}
				// Handle item reaching next node destination
				if (abs((int) (items[i].dest_pos.x - items[i].pos.x)) < 1 && 
						abs((int) (items[i].dest_pos.y - items[i].pos.y)) < 1) {
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
							if ((items[i].used_tile_i + 1) % N_TILES_ROW > 0) items[i].used_tile_i--;
							break;
					}
					// Handle item over destroyed area
					if (tiles[items[i].used_tile_i].destroyed == TRUE) {
						ResetItem(i, cur_level);
					}
					// Collect item
					if (items[i].used_tile_i == cur_level.collectors[0]) {
						score++;
						ResetItem(i, cur_level);
					}
					// switch (items[i].dir) {
					// 	case NORTH:
					// 		items[i].dest_pos.y -= TILE_SIZE; // Because the rendertexture is flipped
					// 		break;
					// 	case EAST:
					// 		items[i].dest_pos.x += TILE_SIZE;
					// 		break;
					// 	case SOUTH:
					// 		items[i].dest_pos.y += TILE_SIZE;
					// 		break;
					// 	case WEST:
					// 		items[i].dest_pos.x -= TILE_SIZE;
					// 		break;
					// }
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
				// Move item
				Tile t = tiles[items[i].used_tile_i];
				if (t.conveyer == TRUE || t.spawner == TRUE) {
					switch (items[i].dir) {
						case NORTH:
							items[i].pos.y -= TILE_SIZE * dt; // Because the rendertexture is flipped
							break;
						case EAST:
							items[i].pos.x += TILE_SIZE * dt;
							break;
						case SOUTH:
							items[i].pos.y += TILE_SIZE * dt;
							break;
						case WEST:
							items[i].pos.x -= TILE_SIZE * dt;
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
							DrawTile(tile_tex, (Vector2) {j * TILE_SIZE, i * TILE_SIZE}, 6 * TILE_SIZE, 0);
						} else if (t.collector == TRUE) {
							DrawTile(tile_tex, (Vector2) {j * TILE_SIZE, i * TILE_SIZE}, 7 * TILE_SIZE, 0);
						} else if (t.destroyed == FALSE) {
							DrawTile(tile_tex, (Vector2) {j * TILE_SIZE, i * TILE_SIZE}, 0, t.rand);
						} else {
							DrawTile(tile_tex, (Vector2) {j * TILE_SIZE, i * TILE_SIZE}, TILE_SIZE, t.rand);
						}	
					} else {
						DrawTile(tile_tex, (Vector2) {j * TILE_SIZE, i * TILE_SIZE}, TILE_SIZE * 2 + TILE_SIZE * t.dir, t.rand);
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
			// DrawText(TextFormat("Score: %d", score), 0, 0, 20, WHITE);
			// DrawText(TextFormat("Time: %d", 120 - (int) timer), 0, 24, 20, WHITE);
			// DrawCircle(cursor_pos.x, cursor_pos.y, 4.0f, RED);
		EndTextureMode();

		BeginDrawing();
			ClearBackground(RAYWHITE);
			DrawTexturePro(target.texture, target_src_rec, target_dst_rec, (Vector2) { 0.0f, 0.0f }, 0.0f, WHITE);
		EndDrawing();
	}

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
	items[i].dest_pos = items[i].pos;
	items[i].time = 0;
}

void DrawTile(Texture2D t, Vector2 pos, int offset, int r) {
	if (offset == 0) {
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
	} else {
		DrawTextureRec(
			t, 
			(Rectangle) {
				offset, 
				0, 
				TILE_SIZE, 
				TILE_SIZE
			}, 
			pos,
			WHITE
		);
	}
}
