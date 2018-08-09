// Pong.cpp : Defines the entry point for the console application.
//


#include "stdafx.h"

#define WHITE al_map_rgb(255, 255, 255)
#define BLACK al_map_rgb(0, 0, 0)

const int P1 = 1;
const int P2 = 2;
const int NUM_PLAYERS = 2;

const int HALF_FACTOR = 2;
const int FRAMERATE = 60;
const int DISPLAY_WIDTH = 900;
const int DISPLAY_LENGTH = 600;
const int EDGE_MARGIN = 50;

const int FONT_SIZE = 60;
const int CENTER_MARGIN = 10;
const int LINE_THICKNESS = 5;
const int MESSAGE_OFFSET = 180;

const int NUM_SOUNDS = 2;

const double PLAYER_LENGTH = 175;
const double PLAYER_WIDTH = 30;
const double PLAYER_SPEED = .0005;

const int BALL_RADIUS = 20;
const int BALL_HIT_RADIUS = 15;
const int BALL_X_SPEED = 7;
const int BALL_Y_SPEED_MAX = 7;
const int REBOUND_SPEED_FACTOR = 6;

class Player {
	
	double topX, topY;
	double yVel;
	int player;
	int score;
	bool running;

	public:

		Player(const double tX, const double tY, const int playNum) {
			topX = tX;
			topY = tY;

			player = playNum;

			score = 0;

			running = true;
		}

		void run() {

			while (running) {
				ALLEGRO_KEYBOARD_STATE keyState;
				al_get_keyboard_state(&keyState);

				if ( ((player == P1 && al_key_down(&keyState, ALLEGRO_KEY_W)) || (player == P2 && al_key_down(&keyState, ALLEGRO_KEY_UP))) && topY > 0) {
					yVel = -1 * PLAYER_SPEED;
				}
				else if ( ((player == P1 && al_key_down(&keyState, ALLEGRO_KEY_S)) || (player == P2 && al_key_down(&keyState, ALLEGRO_KEY_DOWN))) && topY + PLAYER_LENGTH < DISPLAY_LENGTH) {
					yVel = PLAYER_SPEED;
				}
				else {
					yVel = 0;
				}

				topY += yVel;
			}
		}

		void end() {
			running = false;
		}

		std::pair<int, int> getTop() {
			return std::make_pair(topX, topY);
		}

		double getYVel() {
			return yVel;
		}

		int getScore() {
			return score;
		}

		void incScore() {
			score++;
		}
};

static void* Run_Player(ALLEGRO_THREAD* thr, void* arg) {
	((Player*)arg)->run();

	return NULL;
}

class Ball {
	private:
		double xVel;
		double yVel;
		double x, y;
		Player* p1;
		Player* p2;

		ALLEGRO_SAMPLE * high_blip;
		ALLEGRO_SAMPLE * low_blip;
	
	public:
		Ball(Player* play1, Player* play2) {
			p1 = play1;
			p2 = play2;

			al_reserve_samples(NUM_SOUNDS);
			high_blip = al_load_sample("C:/Allegro Projects/Pong/Debug/blip_f5.wav");
			low_blip = al_load_sample("C:/Allegro Projects/Pong/Debug/blip_f4.wav");
		}

		void reset() {
			xVel = 0.0;
			yVel = 0.0;
			x = DISPLAY_WIDTH / HALF_FACTOR;
			y = DISPLAY_LENGTH / HALF_FACTOR;
		}

		void start(int direction) {
			if (direction == P1) {
				xVel = -1 * BALL_X_SPEED;
			}
			else {
				xVel = BALL_X_SPEED;
			}
			yVel = (rand() % BALL_Y_SPEED_MAX) + 1;
			if (rand() % 2 == 0) {
				yVel = yVel *= -1;
			}
		}

		void update() {
			x += xVel;
			y += yVel;
		}

		int checkCollisions() {
			if (y <= BALL_RADIUS || y >= DISPLAY_LENGTH - BALL_RADIUS) {
				yVel *= -1;
				playBlip();
			}
			if (EDGE_MARGIN + (PLAYER_WIDTH / HALF_FACTOR) <= x && x <= EDGE_MARGIN + PLAYER_WIDTH + BALL_RADIUS && xVel < 0) {
				std::pair<int, int> p1Top = p1->getTop();
				if (y >= p1Top.second - BALL_HIT_RADIUS && y <= p1Top.second + PLAYER_LENGTH + BALL_HIT_RADIUS) { //hit p1 paddle
					xVel *= -1;
					yVel += (p1->getYVel() / PLAYER_SPEED * REBOUND_SPEED_FACTOR);
					playBlip();
				}
			}
			else if (DISPLAY_WIDTH - EDGE_MARGIN - PLAYER_WIDTH - BALL_RADIUS <= x && x <= DISPLAY_WIDTH - EDGE_MARGIN - PLAYER_WIDTH && xVel > 0) {
				std::pair<int, int> p2Top = p2->getTop();
				if (y >= p2Top.second - BALL_HIT_RADIUS && y <= p2Top.second + PLAYER_LENGTH + BALL_HIT_RADIUS) { //hit p2 paddle
					xVel *= -1;
					yVel += (p2->getYVel() / PLAYER_SPEED * REBOUND_SPEED_FACTOR);
					playBlip();
				}
			}
			else if (x <= BALL_RADIUS) {
				return P1;
			}
			else if (x >= DISPLAY_WIDTH - BALL_RADIUS) {
				return P2;
			}
			return 0;
		}

		void playBlip() {
			if (abs(xVel) + abs(yVel) > BALL_X_SPEED + BALL_Y_SPEED_MAX + REBOUND_SPEED_FACTOR) {
				al_play_sample(high_blip, 1.0, 0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
			}
			else {
				al_play_sample(low_blip, 1.0, 0, 1.0, ALLEGRO_PLAYMODE_ONCE, NULL);
			}
		}

		std::pair<int, int> getCenter() {
			return std::make_pair(x, y);
		}
};

int main()
{

	ALLEGRO_DISPLAY * display;
	ALLEGRO_EVENT_QUEUE * queue;
	ALLEGRO_TIMER * timer;
	ALLEGRO_THREAD * p1_thread;
	ALLEGRO_THREAD * p2_thread;
	ALLEGRO_FONT * font;

	bool waiting = false;
	bool starting = true;
	double timeCount = 0;

	al_init();
	al_init_primitives_addon();
	al_init_font_addon();
	al_init_ttf_addon();
	al_init_acodec_addon();
	al_install_audio();
	al_install_keyboard();

	Player p1 = Player(EDGE_MARGIN, (DISPLAY_LENGTH / HALF_FACTOR) - (PLAYER_LENGTH / HALF_FACTOR), P1);
	Player p2 = Player(DISPLAY_WIDTH - EDGE_MARGIN - PLAYER_WIDTH, (DISPLAY_LENGTH / HALF_FACTOR) - (PLAYER_LENGTH / HALF_FACTOR), P2);
	Ball ball = Ball(&p1, &p2);

	p1_thread = al_create_thread(Run_Player, &p1);
	al_start_thread(p1_thread);
	p2_thread = al_create_thread(Run_Player, &p2);
	al_start_thread(p2_thread);

	display = al_create_display(DISPLAY_WIDTH, DISPLAY_LENGTH);
	queue = al_create_event_queue();
	timer = al_create_timer(1.0 / FRAMERATE);
	
	al_register_event_source(queue, al_get_display_event_source(display));
	al_register_event_source(queue, al_get_timer_event_source(timer));

	font = al_load_ttf_font("C:/Allegro Projects/Pong/Debug/Litebulb.ttf", FONT_SIZE, 0);
	if (font == nullptr) {
		fprintf(stderr, "font error");
	}

	bool running = true;
	int startPlayer = 0;

	al_start_timer(timer);
	ball.reset();
	while (running) {

		ALLEGRO_EVENT event;
		al_wait_for_event(queue, &event);
		if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
			p1.end();
			p2.end();
			running = false;
		}
		if (event.type == ALLEGRO_EVENT_TIMER) {
			al_clear_to_color(BLACK);

			ball.update();
			if (starting) {
				al_draw_text(font, WHITE, (DISPLAY_WIDTH / HALF_FACTOR) - MESSAGE_OFFSET, (DISPLAY_LENGTH / HALF_FACTOR), 0, "Press SPACEBAR to begin");

				ALLEGRO_KEYBOARD_STATE keyState;
				al_get_keyboard_state(&keyState);
				if (al_key_down(&keyState, ALLEGRO_KEY_SPACE)) {
					starting = false;
					ball.start((rand() % NUM_PLAYERS) + 1);
				}
			}
			else if (int p = ball.checkCollisions()) { //checkCollisions returns 1 on p2 score, 2 on p1 score, 0 otherwise
				ball.reset();
				if (p == P1) {
					p2.incScore();
				}
				else {
					p1.incScore();
				}
				startPlayer = p;
				waiting = true;
			}
			if (waiting && timeCount < FRAMERATE) {
				timeCount++;
			}
			else if (waiting && timeCount >= FRAMERATE) {
				waiting = false;
				timeCount = 0;
				if (startPlayer == P1) {
					ball.start(P1);
				}
				else {
					ball.start(P2);
				}
			}
			al_draw_line(DISPLAY_WIDTH / HALF_FACTOR, 0, DISPLAY_WIDTH / HALF_FACTOR, DISPLAY_LENGTH, WHITE, LINE_THICKNESS);
			al_draw_textf(font, WHITE, (DISPLAY_WIDTH / HALF_FACTOR) - FONT_SIZE, CENTER_MARGIN, 0, "%d", p1.getScore());
			al_draw_textf(font, WHITE, (DISPLAY_WIDTH / HALF_FACTOR) + FONT_SIZE - CENTER_MARGIN - CENTER_MARGIN, CENTER_MARGIN, 0, "%d", p2.getScore());
			std::pair<int, int> p1Top = p1.getTop();
			al_draw_filled_rectangle(p1Top.first, p1Top.second, p1Top.first + PLAYER_WIDTH, p1Top.second + PLAYER_LENGTH, WHITE);
			std::pair<int, int> p2Top = p2.getTop();
			al_draw_filled_rectangle(p2Top.first, p2Top.second, p2Top.first + PLAYER_WIDTH, p2Top.second + PLAYER_LENGTH, WHITE);
			std::pair<int, int> ballCenter = ball.getCenter();
			al_draw_filled_circle(ballCenter.first, ballCenter.second, BALL_RADIUS, WHITE);
			al_flip_display();
		}
	}

	al_destroy_thread(p1_thread);
	al_destroy_thread(p2_thread);
	al_uninstall_keyboard();
	al_destroy_display(display);
	al_destroy_timer(timer);
	return 0;
}

