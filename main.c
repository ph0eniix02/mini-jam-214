#include <stdbool.h>
#include <stdio.h>
#include <raylib.h>

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
	Texture2D tex;
	Bool destroyed;
	Bool conveyer;
	Direction dir;
} Tile;

Tile tiles[N_TILES] = {0};

int main(void)
{

	printf("Hello World!");
	InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Mini Jam 214");
	SetTargetFPS(60);

	RenderTexture2D target;
	Rectangle target_src_rec;
	Rectangle target_dst_rec;
	target = LoadRenderTexture(SRC_WIDTH, SRC_HEIGHT);
	target_src_rec = (Rectangle) {0, 0, SRC_WIDTH, -SRC_HEIGHT};
	target_dst_rec = (Rectangle) {0, 0, WINDOW_WIDTH, WINDOW_HEIGHT};

	// Move into seperate TileSetup function?
	// Could load the texture in one line, but I'm pretty sure it's the same either
	// way under the hood?
	Image tile_img = LoadImage("assets/tile_sprites.png");
	Texture2D tile_tex = LoadTextureFromImage(tile_img);
	for (int i = 0; i < N_TILES; i++) {
		tiles[i].tex = tile_tex;
	}
	
	Vector2 cursor_pos = {0};
	int active_tile_index = 0;
	int last_active_tile_index = -1;

	while (!WindowShouldClose())
	{
		if (GetMousePosition().x < WINDOW_WIDTH && GetMousePosition().y < WINDOW_HEIGHT) {
			cursor_pos = (Vector2) {GetMousePosition().x / 3, GetMousePosition().y / 3};
		}

		// Could optimize using sorting algorithim
		// Must use the index, because simply getting the active tile makes a copy.
		// I wonder if I could get the pointer to the active tile. However, this works
		// for now.
		active_tile_index = (int) (cursor_pos.y / TILE_SIZE) * N_TILES_ROW + (int) (cursor_pos.x / TILE_SIZE);
		if (IsMouseButtonDown(MOUSE_LEFT_BUTTON) && active_tile_index != last_active_tile_index) {
			if (tiles[active_tile_index].conveyer != TRUE) {
				tiles[active_tile_index].conveyer = TRUE;
			} else if (tiles[active_tile_index].conveyer == TRUE) {
				tiles[active_tile_index].conveyer = FALSE;
			}
			last_active_tile_index = active_tile_index;
		} else if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON) && tiles[active_tile_index].conveyer == TRUE) {
			if (tiles[active_tile_index].dir < 3) {
				tiles[active_tile_index].dir++;
			} else {
				tiles[active_tile_index].dir = NORTH;
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
						if (t.destroyed == FALSE) {
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
