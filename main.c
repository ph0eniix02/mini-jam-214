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
#define N_TILES_COL (int) (SRC_HEIGHT / TILE_SIZE)
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
} Tile;

typedef struct {
	Texture2D tex;
	Vector2 pos;
	Vector2 dest_pos;
	int used_tile_i;
	Direction dir;
	Bool active;
	Bool bomb;
} Item;

typedef struct {
	int spawners[2];
	int collectors[2];
	int paths;
} Level;

Tile tiles[N_TILES] = {0};

Item items[N_TILES] = {0};

Level level_1 = {.spawners = {20}, .collectors = {200}, .paths = 1};


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

	Texture2D item_tex = LoadTexture("assets/firework_sprites.png");
	// Move into seperate TileSetup function?
	// Could load the texture in one line, but I'm pretty sure it's the same either
	// way under the hood?
	Image tile_img = LoadImage("assets/tile_sprites.png");
	Texture2D tile_tex = LoadTextureFromImage(tile_img);
	for (int i = 0; i < N_TILES; i++) {
		tiles[i].tex = tile_tex;
	}
	
	// Load level 1
	Level cur_level = level_1;
	tiles[cur_level.spawners[0]].spawner = TRUE;
	tiles[cur_level.spawners[0]].dir = NORTH;
	tiles[cur_level.collectors[0]].collector = TRUE;
	for (int i = 0; i < N_TILES; i++) {
		items[i].pos = (Vector2) {cur_level.spawners[0] % N_TILES_ROW * TILE_SIZE, floorf(cur_level.spawners[0] / N_TILES_ROW) * TILE_SIZE};
		items[i].used_tile_i = cur_level.spawners[0];
		items[i].dir = tiles[cur_level.spawners[0]].dir;
		items[i].dest_pos = items[i].pos;
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

	Vector2 cursor_pos = {0};
	int active_tile_index = 0;
	int last_active_tile_index = -1;

	float timer = 0.0f;

	while (!WindowShouldClose())
	{
		timer += GetFrameTime();
		if (items[(int) floorf(timer)].active == FALSE) items[(int) floorf(timer)].active = TRUE;
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
			if (IsMouseButtonDown(MOUSE_RIGHT_BUTTON) && active_tile_index != last_active_tile_index && tiles[active_tile_index].conveyer != TRUE) {
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

		printf("Debug 2:\n");
		printf("Pos: %f, %f\n", items[0].pos.x, items[0].pos.y);
		printf("Dest Pos: %f, %f\n", items[0].dest_pos.x, items[0].dest_pos.y);
		printf("Guide tile: %d\n", items[0].used_tile_i);
		float dt = GetFrameTime();
		for (int i = 0; i < N_TILES; i++) {
			if (items[i].active == TRUE) {

				if (abs((int) (items[i].dest_pos.x - items[i].pos.x)) < 1 && 
						abs((int) (items[i].dest_pos.y - items[i].pos.y)) < 1) {
					switch (items[i].dir) {
						case NORTH:
							if (items[i].used_tile_i > 19) items[i].used_tile_i -= 20;
							break;
						case EAST:
							if ((items[i].used_tile_i + 1) % N_TILES_ROW < 20) items[i].used_tile_i++;
							break;
						case SOUTH:
							if (items[i].used_tile_i < 199) items[i].used_tile_i += 20;
							break;
						case WEST:
							if ((items[i].used_tile_i + 1) % N_TILES_ROW > 0) items[i].used_tile_i--;
							break;
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
			DrawText("Hello Raylib!", 0, 0, 20, LIGHTGRAY);

			// Draw Tiles
			// Could simplify drawing code by passing an x value to a drawing function
			// since that's the only differentiating aspect
			for (int i = 0; i < N_TILES_COL; i++) {
				for (int j = 0; j < N_TILES_ROW; j++) {
					Tile t = tiles[N_TILES_ROW * i + j];
					if (t.conveyer == FALSE) {
						if (t.spawner == TRUE) {
							DrawTextureRec(
								t.tex, 
								(Rectangle) {
									TILE_SIZE * 2 + TILE_SIZE * t.dir, 
									0, 
									TILE_SIZE, 
									TILE_SIZE
								}, 
								(Vector2) {j * TILE_SIZE, i * TILE_SIZE}, 
								GREEN
							);
						} else if (t.collector == TRUE) {
							DrawTextureRec(
								t.tex, 
								(Rectangle) {
									TILE_SIZE * 2 + TILE_SIZE * t.dir, 
									0, 
									TILE_SIZE, 
									TILE_SIZE
								}, 
								(Vector2) {j * TILE_SIZE, i * TILE_SIZE}, 
								RED
							);
						} else if (t.destroyed == FALSE) {
							DrawTextureRec(
								t.tex, 
								(Rectangle) {
									0, 
									0, 
									TILE_SIZE, 
									TILE_SIZE
								}, 
								(Vector2) {j * TILE_SIZE, i * TILE_SIZE}, 
								WHITE
							);
						} else {
							DrawTextureRec(
								t.tex, 
								(Rectangle) {
									TILE_SIZE, 
									0, 
									TILE_SIZE, 
									TILE_SIZE
								}, 
								(Vector2) {j * TILE_SIZE, i * TILE_SIZE}, 
								WHITE
							);
						}	
					} else {
						DrawTextureRec(
							t.tex, 
							(Rectangle) {
								TILE_SIZE * 2 + TILE_SIZE * t.dir, 
								0, 
								TILE_SIZE, 
								TILE_SIZE
							}, 
							(Vector2) {j * TILE_SIZE, i * TILE_SIZE}, 
							WHITE
						);
					}	
				}
			}
			
			// Draw items on conveyers
			for (int i = 0; i < N_TILES; i++) {
				if (items[i].active == TRUE) {
					DrawTexturePro(item_tex, (Rectangle) {0, 0, TILE_SIZE, TILE_SIZE}, (Rectangle) {items[i].pos.x, items[i].pos.y, TILE_SIZE, TILE_SIZE}, (Vector2) { 0, 0 }, 0.0f, WHITE);
				}
			}
			DrawCircle(cursor_pos.x, cursor_pos.y, 4.0f, RED);
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
