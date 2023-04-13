
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
		sAppName = "  Pseudo 3D Labyrinth Spiel";
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
	int current_color;
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
	int map[10][10];

	int color[5][3] =
	{
		{  0,   0,   0},  //Schwarz
		{255, 255, 255},  //Weiß
		{255,   0,   0},  //Rot
		{  0, 255,   0},  //Gruen
		{  0,   0, 255}   //Blau
	};

	//Methoden
	void setPlayerAngle() {
		if (spawnX == 0) player_angle = 270.0;
		else if (spawnX == 9) player_angle = 90.0;
		else if (spawnY == 9) player_angle = 0.0;
		else if (spawnY == 0) player_angle = 180.0;
		else player_angle = 30.0;
	}

	void setSpawn() {
		printf("Spieler wird gespawnt...\n");
		player_coordinates[0] = (spawnX * tile_s) + tile_s / 2;
		player_coordinates[1] = (spawnY * tile_s) + tile_s / 2;
		printf("Spieler erfolgreich gespawnt.\n");
	}

	int exists(int x, int y) {
		if ((x >= 0 && x <= tile_s * 10) && (y >= 0 && y <= tile_s * 10)) return 1;
		else return 0;
	}

	void readMap() {

		ifstream inputFile("map.cfg", ios::binary);

		// Get the file size
		inputFile.seekg(0, ios::end);
		int fileSize = inputFile.tellg();
		inputFile.seekg(0, ios::beg);

		// Read the file contents to a byte array
		char* buffer = new char[fileSize];
		inputFile.read(buffer, fileSize);
		inputFile.close();
		int chr;
		int o = 0;
		int x = 0;
		int y = 0;

		// Print the byte array
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
			case 53: outX = x; outY = y; map[y][x] = 0; break;
			case 54: spawnX = x; spawnY = y; map[y][x] = 0; break;
			case 32: x++; break;
			default: break;
			}
		}
		delete[] buffer;
	}

	bool isWall(int x, int y) {

		int tile_x = int(x / tile_s);
		int tile_y = int(y / tile_s);
		bool ans = map[tile_y][tile_x];
		current_color = map[tile_y][tile_x];
		return ans;

	}

	void createCircle(int x, int y) {
		int start_angle = 0;
		int end_angle = 360;


		while (start_angle != end_angle) {
			createLine(start_angle, x, y, 3);
			start_angle = incrementGivenAngle(start_angle, 1);
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

	int round(float n) {
		if (int(n) + 0.5 >= n) {
			return int(n) + 1;
		}
		return n;
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
		map_wipe(resolution);
		for (int i = 0; i != resolution; i++) {
			for (int k = 0; k != resolution; k++) {
				if (map[i][k]) {
					createSquare(screen_size[0] - int((resolution - k) * map_tile), screen_size[0] - int((resolution - i) * map_tile), int(map_tile));
					//createCircle(k*tile_s, i*tile_s);       
				}
			}
		}
		displayPlayer();
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
			if (current_color > 4)current_color = 0;
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
		//1 Degree = pi over 180
		float OneDeg = pi / 180;

		return OneDeg * angle;
	}

	void update_tiles() {

		tile_x = int(player_coordinates[0] / tile_s);
		tile_y = int(player_coordinates[1] / tile_s);
		if (tile_x == outX && tile_y == outY) {
			end_tile = 1;
		}
		if (end_tile_vorher == 0 && end_tile == 1) printf("Spiel wurde erfolgreich durchgespielt!\n");
		end_tile_vorher = end_tile;
	}

	void checkKeys() {



		// e
		if (GetKey(olc::Key::E).bHeld) {
			decrementPlayerAngle(player_rot_speed);
			return;
		}

		// q
		else if (GetKey(olc::Key::Q).bHeld) {

			incrementPlayerAngle(player_rot_speed);
			return;
		}

		// w
		else if (GetKey(olc::Key::W).bHeld) {

			int distance = (int)rayCasting(player_angle);
			int testX;
			int testY;

			if (distance > 65) {
				testY = player_coordinates[1] - (sin(angle2Radians(player_angle)) * player_mov_speed);
				testX = player_coordinates[0] + (cos(angle2Radians(player_angle)) * player_mov_speed);
				if (exists(testX, testY)) {
					player_coordinates[1] = testY;
					player_coordinates[0] = testX;
				}
				return;
			}
		}

		// s
		else if (GetKey(olc::Key::S).bHeld) {

			int distance = (int)rayCasting(incrementGivenAngle(player_angle, 180.0));
			int testX;
			int testY;

			if (distance > 65) {
				testY = player_coordinates[1] + (sin(angle2Radians(player_angle)) * player_mov_speed);
				testX = player_coordinates[0] - (cos(angle2Radians(player_angle)) * player_mov_speed);
				if (exists(testX, testY)) {
					player_coordinates[1] = testY;
					player_coordinates[0] = testX;
				}
				return;

			}
		}

		// a
		else if (GetKey(olc::Key::D).bHeld) {
			int distance = (int)rayCasting(decrementGivenAngle(player_angle, 90.0));
			int testX;
			int testY;

			if (distance > 65) {
				testY = player_coordinates[1] - (sin(angle2Radians(decrementGivenAngle(player_angle, 90.0))) * player_mov_speed);
				testX = player_coordinates[0] + (cos(angle2Radians(decrementGivenAngle(player_angle, 90.0))) * player_mov_speed);
				if (exists(testX, testY)) {
					player_coordinates[1] = testY;
					player_coordinates[0] = testX;
				}
				return;
			}
		}

		// d
		else if (GetKey(olc::Key::A).bHeld) {
			int distance = (int)rayCasting(incrementGivenAngle(player_angle, 90.0));
			int testX;
			int testY;

			if (distance > 65) {
				testY = player_coordinates[1] - (sin(angle2Radians(incrementGivenAngle(player_angle, 90.0))) * player_mov_speed);
				testX = player_coordinates[0] + (cos(angle2Radians(incrementGivenAngle(player_angle, 90.0))) * player_mov_speed);
				if (exists(testX, testY)) {
					player_coordinates[1] = testY;
					player_coordinates[0] = testX;
				}
				return;
			}
		}
		else if (GetKey(olc::Key::M).bHeld) {
			map_active = 1;
		}

		else if (GetKey(olc::Key::N).bHeld) {
			map_active = 0;
		}

		else if (GetKey(olc::Key::P).bHeld) {
			printf("Spiel wird beendet...\n");
			exit(0);
		}

	}

	bool OnUserCreate() override
	{
		readMap();
		setSpawn();
		setPlayerAngle();
		return true;
	}

	//Spiel-Schleife

	bool OnUserUpdate(float fElapsedTime) override
	{

		checkKeys();
		update_tiles();
		if (end_tile == 1) {
			whiteNoise();
			return true;
		}
		createFOV();
		if (map_active) createMap(10);
		return true;

	}

};


// Funktion, die ein Objekt von "Example" instanziiert und somit das Spiel startet
int main()
{

	Example demo;
	if (demo.Construct(screen_size[0], screen_size[1], pixel_size, pixel_size))
		demo.Start();


	return 0;
}
