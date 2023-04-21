
#include <iostream>
#include <fstream>
#include <math.h>
#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"


using namespace std;
const float pi = 3.14159;
// Einstellungen
const int screen_size[2] = { 800, 800 };																		//Hoehe, Breite
const int half = int(screen_size[0] / 2);
int pixel_size = 1;


class Example : public olc::PixelGameEngine
{
public:
	Example()
	{
		sAppName = " Map Level Editor";
	}

	//Attribute
	float player_rot_speed = 2.0;																				// Grad pro Tastendruck
	int player_mov_speed = 5;																					// Pixel pro Tastendruck
	float player_angle;																							// Ausrichtung des Spielers beim Spawnen
	int player_coordinates[2] = { outY * tile_s + int(tile_s / 2), outX * tile_s + int(tile_s / 2) };			// Koordinaten des Spielers 
	float player_FOV = 40.0;																					// Sehfeld des Spielers
	float player_FOVH = player_FOV / 2;																			// Haelfte des Sehfelds des Spielers (Nur für berechnungen wichtig)
	float screen_depth = half / tan(angle2Radians(player_FOVH));
	int num_rays = int(screen_size[0] / 2);
	float delta_angle = (player_FOV / 2) / num_rays;
	float scale_fac = int(screen_size[0] / num_rays);
	int current_color = 0;
	int map_tile = 10;
	int m_end = 10 * map_tile;
	bool map_active = 1;
	bool horizn = 0;
	int spawnX;
	int spawnY;
	int outY;
	int outX;
	bool exit_hit = 0;
	double max_depth = sqrt(((tile_s * 10) * (tile_s * 10)) + ((tile_s * 10) * (tile_s * 10))) + 1;
	int tile_s = int(screen_size[0] / 10);
	int tile_x;
	int tile_y;
	int end_tile = 0;
	int end_tile_vorher = 0;
	int mouse_x;
	int mouse_y;
	int mouse_l;
	int mouse_r;
	int map[10][10];
	int change;
	int out;
	int spawn;
	int target_mode = 0;

	int color[5][3] =
	{
		{  0,   0,   0},  //Schwarz
		{255, 255, 255},  //Weiß
		{255,   0,   0},  //Rot
		{  0, 255,   0},  //Gruen
		{  0,   0, 255}   //Blau
	};

	//Methoden

	void setSpawn() {
		if (spawnX == 0) spawn = 1;
		else if (spawnX == 9) spawn = 3;
		else if (spawnY == 9) spawn = 0;
		else if (spawnY == 0) spawn = 2;
		else spawn = 4;
	}
	//Ausgang Setzen
	void setOut() {
		if (outX == 0) out = 3;
		else if (outX == 9) out = 1;
		else {
			if (outY == 9) out = 2;
			else out = 0;
		}
	}

	//Spieler Koordinaten Setzen
	void getSpawnFromCFG() {
		printf("Spieler wird gespawnt...\n");
		player_coordinates[0] = (spawnX * tile_s) + tile_s / 2;
		player_coordinates[1] = (spawnY * tile_s) + tile_s / 2;
		printf("Spieler erfolgreich gespawnt.\n");
	}

	int exists(int x, int y) {
		if ((x >= 0 && x <= tile_s * 10) && (y >= 0 && y <= tile_s * 10)) return 1;
		else return 0;
	}

	bool isWall(int x, int y) {

		int tile_x = int(x / tile_s);
		int tile_y = int(y / tile_s);
		bool ans = map[tile_y][tile_x];
		current_color = map[tile_y][tile_x];
		return ans;

	}

	void createColoredCircle(int x, int y, int r, int g, int b) {
		int start_angle = 0;
		int end_angle = 360;


		while (start_angle != end_angle) {
			createColoredLine(start_angle, x, y, r, g, b, 5);
			start_angle = incrementGivenAngle(start_angle, 1);
		}
	}

	void createCircle(int x, int y) {
		int start_angle = 0;
		int end_angle = 360;


		while (start_angle != end_angle) {
			createLine(start_angle, x, y, 7);
			start_angle = incrementGivenAngle(start_angle, 1);
		}
	}

	void fillSquare(int ty, int tx, int r, int g, int b) {
		int d = 0;
		int end = tile_s - 1;
		int start = tx * tile_s + 1;

		while (d != end) {
			createColoredVerticalLine(start + d, ty * tile_s, end, r, g, b, 1);
			d++;
		}
	}

	float rayCasting(float given) {
		float v = angle2Radians(given);
		float horiz = cos(v);
		float abs_horiz = abs(horiz);
		int px = int(player_coordinates[0]);
		int py = int(player_coordinates[1]);
		int hs;
		float vert = sin(v);
		float abs_vert = abs(vert);
		int vs;
		float scal_x = sqrt(1 + ((abs_vert / abs_horiz) * (abs_vert / abs_horiz))); //Imaginäre Zahlen
		float scal_y = sqrt(1 + ((abs_horiz / abs_vert) * (abs_horiz / abs_vert)));
		bool wallHit = 0;
		bool x = 0;
		bool y = 0;



		float x_hyp;
		float y_hyp;

		int x_len;			// x Länge bis zum Schnittpukt
		int y_len;			// y länge bis zum Schnittpunkt

		if (horiz < 0) {
			x_len = px - (tile_x * tile_s);
			hs = -1;
		}
		else {
			x_len = ((tile_x + 1) * tile_s) - px;
			hs = 1;
		}

		if (vert > 0) {
			y_len = py - (tile_y * tile_s);
			vs = -1;
		}

		else {
			y_len = ((tile_y + 1) * tile_s) - py;
			vs = 1;
		}

		x_hyp = x_len * scal_x;
		y_hyp = y_len * scal_y;


		if (x_hyp > y_hyp) {
			if (tile_y + vs <= 10) {
				current_color = map[tile_y + vs][tile_x];
				wallHit = current_color;
				y_len += tile_s;
				y = 1;
			}
			else {
				return 10000;
			}
		}

		else {
			if (tile_x + hs <= 10) {
				current_color = map[tile_y][tile_x + hs];
				wallHit = current_color;
				x_len += tile_s;
				x = 1;
			}
			else {
				return 10000;
			}
		}


		while (!wallHit) {
			x_hyp = x_len * scal_x;
			y_hyp = y_len * scal_y;

			y = 0;
			x = 0;


			if (x_hyp > y_hyp) {
				if (exists(px + (hs * (y_hyp * abs_horiz)), py + (vs * (y_len + 10)))) {
					wallHit = isWall(px + (hs * (y_hyp * abs_horiz)), py + (vs * (y_len + 10)));
					y = 1;
					y_len += tile_s;
					horizn = 1;
				}
				else {
					return 10000;
				}
			}

			else {
				if (exists(px + (hs * (x_len + 10)), py + (vs * (x_hyp * abs_vert)))) {
					wallHit = isWall(px + (hs * (x_len + 10)), py + (vs * (x_hyp * abs_vert)));
					x = 1;
					x_len += tile_s;
					horizn = 0;
				}
				else {
					return 10000;
				}
			}

		}


		//printf("Durchlauf: %i, tile_x: %i, tile_y: %i \n", durchlauf, tile_x, tile_y);

		if (x) return x_hyp;

		else return y_hyp;
	}

	void wipe() {
		for (int i = 0; i != screen_size[1]; i++) {
			for (int j = 0; j != screen_size[0]; j++) {
				Draw(i, j, olc::Pixel(0, 0, 0));
			}
		}
	}

	void map_wipe(int resolution) {
		for (int i = 0; i != m_end; i++) {
			for (int k = 0; k != m_end * map_tile; k++) {
				int x = screen_size[0] - m_end + k;
				int y = screen_size[0] - m_end + i;
				Draw(x, y, olc::Pixel(0, 0, 0));
				//createCircle(k*tile_s, i*tile_s);       
			}
		}
	}

	void whiteNoise() {
		for (int x = 0; x < ScreenWidth(); x++)
			for (int y = 0; y < ScreenHeight(); y++)
				Draw(x, y, olc::Pixel(rand() % 255, rand() % 255, rand() % 255));
	}

	void displayPlayer() {
		int px = round(player_coordinates[0] / 8);
		int py = round(player_coordinates[1] / 8);
		createCircle(screen_size[0] - m_end + px, screen_size[0] - m_end + py);
		createLine(player_angle, screen_size[0] - m_end + px, screen_size[0] - m_end + py, 10);
	}

	void createMap(int resolution) {
		for (int i = 0; i < resolution + 1; i++) {
			for (int k = 0; k < resolution + 1; k++) {
				if (map[i][k]) {
					createSquare(int(k * tile_s), int(i * tile_s), int(tile_s));
					//createCircle(k*tile_s, i*tile_s);       
				}
			}
		}
	}

	void createLine(float angle, int x_or, int y_or, int l) {
		float v = angle2Radians(angle);
		int d = 1;
		int intervall = l + 1;
		float horiz = cos(v);
		float vert = sin(v);
		while (d < intervall) {
			Draw(x_or + int(horiz * d), y_or - int(vert * d), olc::Pixel(255, 255, 255));
			d++;
		}
	}

	void createColoredLine(float angle, int x_or, int y_or, int r, int g, int b, int l) {
		float v = angle2Radians(angle);
		int d = 1;
		int intervall = l + 1;
		float horiz = cos(v);
		float vert = sin(v);
		while (d < intervall) {
			Draw(x_or + int(horiz * d), y_or - int(vert * d), olc::Pixel(r, g, b));
			d++;
		}
	}

	void createHorizontalLine(int x_or, int y_or, int l, bool p) {
		if (p) {
			for (int i = 0; i < l; i++) {
				Draw(x_or + i, y_or, olc::Pixel(255, 255, 255));
			}
			return;
		}
		for (int i = 0; i < l; i++) {
			Draw(x_or - i, y_or, olc::Pixel(255, 255, 255));
		}
	}

	void createColoredHorizontalLine(int x_or, int y_or, int l, int r, int g, int b, bool p) {
		if (p) {
			for (int i = 0; i < l; i++) {
				Draw(x_or + i, y_or, olc::Pixel(r, g, b));
			}
			return;
		}
		for (int i = 0; i < l; i++) {
			Draw(x_or - i, y_or, olc::Pixel(r, g, b));
		}
	}

	void createColoredVerticalLine(int x_or, int y_or, int l, int r, int g, int b, bool p) {
		if (p) {
			for (int i = 0; i < l; i++) {
				Draw(x_or, y_or + i, olc::Pixel(r, g, b));
			}
			return;
		}
		for (int i = 0; i < l; i++) {
			Draw(x_or, y_or - i, olc::Pixel(r, g, b));
		}
	}

	void createVerticalLine(int x_or, int y_or, int l, bool p) {
		if (p) {
			for (int i = 0; i < l; i++) {
				Draw(x_or, y_or + i, olc::Pixel(255, 255, 255));
			}
			return;
		}
		for (int i = 0; i < l; i++) {
			Draw(x_or, y_or - i, olc::Pixel(255, 255, 255));
		}

	}

	void createSquare(int x, int y, int h) {
		createHorizontalLine(x, y, h, 1);
		createHorizontalLine(x, y + h, h, 1);
		createVerticalLine(x, y, h, 1);
		createVerticalLine(x + h, y, h, 1);
	}

	float calcTS(float ts) {
		if (horizn) {
			if (ts >= 0.1) {
				return ts - 0.1;
			}
			else return 0;
		}
		if (ts <= 0.9) return ts + 0.1;
		return 1;
	}
	//Projektion
	void createFOV() {
		// 100 ist tile_s * 10
		float start_angle = incrementGivenAngle(player_angle, player_FOVH);
		float end_angle = decrementGivenAngle(player_angle, player_FOVH);
		int durchlauf = 0;
		float depth;
		float ts = 0;
		int p_height;

		while (durchlauf != screen_size[0]) {
			depth = rayCasting(start_angle);
			if (current_color > 3)current_color = 0;
			if (depth > 5000 || current_color == 0) {
				createColoredVerticalLine(durchlauf, half, half, color[3][0], color[3][1], color[3][2], 1);
				createColoredVerticalLine(durchlauf, half, half, color[4][0], color[4][1], color[4][2], 0);
				durchlauf++;
				start_angle = decrementGivenAngle(start_angle, delta_angle);
				continue;
			}

			depth *= cos(angle2Radians(decrementGivenAngle(player_angle, start_angle)));
			ts = 1 - (depth / (tile_s * 11));
			ts = calcTS(ts);
			p_height = int(tile_s / 2 * screen_depth / depth);

			if (p_height < screen_size[0]) {
				createColoredVerticalLine(durchlauf, half - p_height, half - p_height, color[4][0], color[4][1], color[4][2], 0);
				createColoredVerticalLine(durchlauf, half + p_height, half - p_height, color[3][0], color[3][1], color[3][2], 1);
				createColoredVerticalLine(durchlauf, half, p_height, int(color[current_color][0] * ts), int(color[current_color][1] * ts), int(color[current_color][2] * ts), 0);
				createColoredVerticalLine(durchlauf, half, p_height, int(color[current_color][0] * ts), int(color[current_color][1] * ts), int(color[current_color][2] * ts), 1);
				durchlauf++;
				start_angle = decrementGivenAngle(start_angle, delta_angle);
				continue;
			}
			else {
				createColoredVerticalLine(durchlauf, half, half, int(color[current_color][0] * ts), int(color[current_color][1] * ts), int(color[current_color][2] * ts), 0);
				createColoredVerticalLine(durchlauf, half, half, int(color[current_color][0] * ts), int(color[current_color][1] * ts), int(color[current_color][2] * ts), 1);
				durchlauf++;
				start_angle = decrementGivenAngle(start_angle, delta_angle);
				continue;
			}


		}

	}

public:

	void incrementPlayerAngle(float w) {
		if (player_angle == 360.0) {
			player_angle = w;
			return;
		}

		else if (player_angle + w > 360) {
			player_angle = w - (360 - player_angle);
			return;
		}
		player_angle += w;
		return;

	}

	void decrementPlayerAngle(float w) {
		if (player_angle == 0) {
			player_angle = 360.0 - w;
			return;
		}

		else if (player_angle - w < 0) {

			player_angle = 360 - (w - player_angle);
			return;
		}
		player_angle -= w;

		return;
	}

	float incrementGivenAngle(float a, float b) {
		if (a == 360.0) {
			a = b;
			return a;
		}

		else if (a + b > 360) {
			return b - (360 - a);
		}
		a += b;
		return a;
	}

	float decrementGivenAngle(float a, float b) {
		if (a == 0) {
			a = 360.0 - b;

			return a;
		}

		else if (a - b < 0) {

			return 360 - (b - a);
		}
		a -= b;

		return a;
	}

	float angle2Radians(float angle) {
		//1 Grad = pi over 180
		float OneDeg = pi / 180;

		return OneDeg * angle;
	}
	
	//Bereich des Spielers
	void update_tiles(int x, int y) {

		tile_x = int(x / tile_s);
		tile_y = int(y / tile_s);
		/*if (tile_x == outX && tile_y == outY) {
			end_tile = 1;
		}
		if (end_tile_vorher == 0 && end_tile == 1) printf("Spiel wurde erfolgreich durchgespielt!\n");
		end_tile_vorher = end_tile;*/
	}

	void checkMouse() {
		mouse_x = GetMouseX();
		mouse_y = GetMouseY();
		mouse_l = GetMouse(0).bHeld;
		mouse_r = GetMouse(1).bHeld;

		update_tiles(mouse_x, mouse_y);

	}

	void checkKeys() {

		// 1
		if (GetKey(olc::Key::K1).bHeld) {
			current_color = 0;
			target_mode = 0;
			return;
		}

		// 2
		else if (GetKey(olc::Key::K2).bHeld) {

			current_color = 1;
			target_mode = 0;
			return;
		}

		// 3
		else if (GetKey(olc::Key::K3).bHeld) {

			current_color = 2;
			target_mode = 0;
			return;
		}

		// 4
		else if (GetKey(olc::Key::K4).bHeld) {

			current_color = 3;
			target_mode = 0;
			return;
		}

		// 5
		else if (GetKey(olc::Key::K5).bHeld) {
			current_color = 4;
			target_mode = 0;
			return;
		}

		//6
		else if (GetKey(olc::Key::K6).bHeld) {
			target_mode = 1;
			return;
		}

		//Reloade Map
		else if (GetKey(olc::Key::R).bHeld) {
			readMap();
			color_tiles();
			setSpawn();
			setOut();
			drawExit();
			drawSpawn();
			target_mode = 0;
			return;
		}

		//Clear
		else if (GetKey(olc::Key::C).bHeld) {
			for (int i = 0; i != 10; i++) {
				for (int j = 0; j != 10; j++) {
					map[i][j] = 0;
				}
			}
			outX = 0;
			outY = 0;
			spawnX = 0;
			spawnY = 0;
			target_mode = 0;
			change = 1;
			return;
		}

		//Save
		else if (GetKey(olc::Key::S).bHeld) {
			saveMap();
			target_mode = 0;
			return;
		}

		//Exit  Game
		else if (GetKey(olc::Key::P).bHeld) {
			printf("Spiel wird beendet...\n");
			exit(0);
		}

	}

	void color_tiles() {
		for (int i = 0; i <= 9; i++) {
			for (int j = 0; j <= 9; j++) {
				fillSquare(i, j, color[map[i][j]][0], color[map[i][j]][1], color[map[i][j]][2]);
			}
		}
	}

	void drawArrowRight(int x, int y) {

		createColoredHorizontalLine(x + 10, y + 40, 60, 255, 255, 255, 1);
		createColoredLine(150.0, x + 70, y + 40, 255, 255, 255, 20);
		createColoredLine(210.0, x + 70, y + 40, 255, 255, 255, 20);
	}

	void drawArrowLeft(int x, int y) {
		createColoredHorizontalLine(x + 10, y + 40, 60, 255, 255, 255, 1);
		createColoredLine(30.0, x + 10, y + 40, 255, 255, 255, 20);
		createColoredLine(330.0, x + 10, y + 40, 255, 255, 255, 20);
	}

	void drawArrowDown(int x, int y) {
		createColoredVerticalLine(x + 40, y + 10, 60, 255, 255, 255, 1);
		createColoredLine(60.0, x + 40, y + 70, 255, 255, 255, 20);
		createColoredLine(120.0, x + 40, y + 70, 255, 255, 255, 20);
	}

	void drawArrowUp(int x, int y) {
		createColoredVerticalLine(x + 40, y + 10, 60, 255, 255, 255, 1);
		createColoredLine(300.0, x + 40, y + 10, 255, 255, 255, 20);
		createColoredLine(240.0, x + 40, y + 10, 255, 255, 255, 20);
	}

	void drawExit() {
		switch (out) {
		case 0: drawArrowUp(outX * tile_s, outY * tile_s); break;
		case 1: drawArrowRight(outX * tile_s, outY * tile_s); break;
		case 2: drawArrowDown(outX * tile_s, outY * tile_s); break;
		case 3: drawArrowLeft(outX * tile_s, outY * tile_s); break;
		}
	}

	void drawSpawn() {
		createCircle(spawnX * tile_s + 40, spawnY * tile_s + 40);
	}


	bool OnUserCreate() override
	{
		readMap();
		color_tiles();
		setOut();
		setSpawn();
		drawExit();
		drawSpawn();
		return true;
	}

	//Spiel-Schleife

	bool OnUserUpdate(float fElapsedTime) override
	{
		if (change) { color_tiles(); change = 0; drawExit(); drawSpawn(); }
		checkMouse();
		checkKeys();
		if (mouse_l && !target_mode) { map[tile_y][tile_x] = current_color; change = 1; }
		else if (mouse_l && target_mode) { map[tile_y][tile_x] = 0; outY = tile_y; outX = tile_x; setOut(); change = 1; }
		else if (mouse_r) { spawnY = tile_y; spawnX = tile_x; setSpawn(); change = 1; }
		return true;
	}

	void readMap() {

		ifstream inputFile("map.cfg", ios::binary);

		// Get Dateigroesse
		inputFile.seekg(0, ios::end);
		int fileSize = inputFile.tellg();
		inputFile.seekg(0, ios::beg);

		// Lese Dateinhalte in ein bytearray
		char* buffer = new char[fileSize];
		inputFile.read(buffer, fileSize);
		inputFile.close();
		int chr;
		int o = 0;
		int x = 0;
		int y = 0;

		// Print bytearray
		for (int i = 0; i < fileSize; i++) {
			chr = (int)buffer[i];
			switch (chr) {
			case 123: x = 0; break;
			case 125: y += 1; break;
			case 48: map[y][x] = 0; break;
			case 49: map[y][x] = 1; break;
			case 50: map[y][x] = 2; break;
			case 51: map[y][x] = 3; break;
			case 52: map[y][x] = 4; break;
			case 53: outX = x; outY = y; map[y][x] = 0; setOut(); break;
			case 54: spawnX = x; spawnY = y; map[y][x] = 0; setSpawn(); break;
			case 32: x++; break;
			default: break;
			}
		}
		delete[] buffer;
	}

	uint8_t getByte(int ty, int tx) {
		if (ty == outY && tx == outX)  return (uint8_t)53;
		else if (ty == spawnY && tx == spawnX) return (uint8_t)54;
		else return (uint8_t)map[ty][tx] + 48;
	}

	void saveMap()
	{	
		//Schreibe Byte
		uint8_t wbyte;

		std::ofstream outfile("map.cfg", std::ios::binary);

		for (int i = 0; i != 10; i++) {
			for (int j = 0; j != 10; j++) {
				if (j == 0) {
					wbyte = 123;
					outfile.write(reinterpret_cast<const char*>(&wbyte), sizeof(wbyte));
					wbyte = getByte(i, j);
					outfile.write(reinterpret_cast<const char*>(&wbyte), sizeof(wbyte));
					wbyte = 32;
					outfile.write(reinterpret_cast<const char*>(&wbyte), sizeof(wbyte));
				}
				else if (j == 9) {
					wbyte = getByte(i, j);
					outfile.write(reinterpret_cast<const char*>(&wbyte), sizeof(wbyte));
					wbyte = 125;
					outfile.write(reinterpret_cast<const char*>(&wbyte), sizeof(wbyte));
				}
				else {
					wbyte = getByte(i, j);
					outfile.write(reinterpret_cast<const char*>(&wbyte), sizeof(wbyte));
					wbyte = 32;
					outfile.write(reinterpret_cast<const char*>(&wbyte), sizeof(wbyte));
				}
			}
		}
		outfile.close();
	}
};



// Hauptfunktion, die die oben Programmierte Klasse "Example" aufruft, und somit das Spiel startet
int main()
{

	Example demo;
	if (demo.Construct(screen_size[0], screen_size[1], pixel_size, pixel_size))
		demo.Start();

	return 0;
}
