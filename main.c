#include <stdio.h>
#include <raylib.h>

int main(void)
{
	printf("Hello World!");
	InitWindow(320, 180, "Mini Jam 214");
	SetTargetFPS(60);

	while (!WindowShouldClose())
	{
		BeginDrawing();
			ClearBackground(RAYWHITE);
			DrawText("Hello Raylib!", 0, 0, 20, LIGHTGRAY);
		EndDrawing();
	}

	CloseWindow();
	return 0;
}
