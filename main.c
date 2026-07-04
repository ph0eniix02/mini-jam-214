#include <stdio.h>
#include <raylib.h>

#define CORNFLOWER_BLUE (Color) {99, 149, 238, 255} 
#define SRC_WIDTH 320
#define SRC_HEIGHT 180
#define WINDOW_WIDTH 960
#define WINDOW_HEIGHT 540

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
	
	while (!WindowShouldClose())
	{
		BeginTextureMode(target);
			ClearBackground(CORNFLOWER_BLUE);
			DrawText("Hello Raylib!", 0, 0, 20, LIGHTGRAY);
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
