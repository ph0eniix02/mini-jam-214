#include <stdio.h>
#include <raylib.h>

#define CORNFLOWER_BLUE (Color) {99, 149, 238, 255} 
int main(void)
{
	printf("Hello World!");
	InitWindow(960, 540, "Mini Jam 214");
	SetTargetFPS(60);

	while (!WindowShouldClose())
	{
		BeginDrawing();
			ClearBackground(CORNFLOWER_BLUE);
			DrawText("Hello Raylib!", 0, 0, 20, LIGHTGRAY);
		EndDrawing();
	}

	CloseWindow();
	return 0;
}
