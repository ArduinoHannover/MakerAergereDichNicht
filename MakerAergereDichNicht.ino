/*
MakerAergereDichNicht.ino
Autor: Luca Zimmermann
http://sndstrm.de/
http://arduino-hannover.de/

Lizensiert unter GNU GPL v3, eine Kopie der Lizenz liegt in LICENSE bei.

Spielweise:
Zu Beginn blinken alle LEDs im Regenbogen.
Zum Starten eines neuen Spiels den SELECT Button druecken. (Tasten sind in der Software nicht entprellt)
Nachfolgend kann die Anzahl der Spieler gewaehlt werden (SELECT = +1, lange SELECT = bestaetigen)
Die Farbe wird dann mit den Spielertasten gewaehlt.
*/

#define SELECT_BUTTON 4
/*
Feld: Start im inneren oben rechts entgegen des Uhrzeigersinns:
       ___
      |   |
      v   ^
 __<__|   | _<__
|         ^     |
|__>__     __>__|
      |   |
      v   ^
      |___|
Fig. 1
*/
#define FIELD_OUT 6
/*
Ziel startet immer innen.
Zuerst nach rechts (vgl. Fig. 1), dann jedoch im Uhrzeigersinn:
         P4 15
            14
            13
P3          12          P1
11 10 09 08   >00 01 02 03
            04
            05
            06
         P2 07
Fig. 2
*/
#define FINISH_OUT 7
/*
Pixelanordnung des Wuerfels:
Arduino -> 0-----1
                 |
           4--3--2
           |
           5-----6
Fig. 3
*/
#define DICE_OUT 5

//Positionen der Spieler in Fig. 2
#define BUTTON_P1 8
#define BUTTON_P2 11
#define BUTTON_P3 10
#define BUTTON_P4 9



#define MINBRIGHTNESS 30
#define MAXBRIGHTNESS 255

#define HOME1   -1
#define HOME2   -2
#define HOME3   -3
#define HOME4   -4
#define EXIT     0
#define FINISH1 40
#define FINISH2 41
#define FINISH3 42
#define FINISH4 43

#include <Adafruit_NeoPixel.h>

Adafruit_NeoPixel field = Adafruit_NeoPixel(56,	FIELD_OUT, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel finish = Adafruit_NeoPixel(16, FINISH_OUT, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel dice = Adafruit_NeoPixel(7, DICE_OUT, NEO_GRB + NEO_KHZ800);

const uint8_t MAXPLAYERS = 4;
const uint8_t MINPLAYERS = 2;
const uint8_t FIGURES = 4;
uint32_t player_colors[MAXPLAYERS] = {0xFF0000, 0xFFFF00, 0x00FF00, 0x0000FF};
uint32_t undimmed_colors[MAXPLAYERS] = {0xFF0000, 0xFFFF00, 0x00FF00, 0x0000FF};
int8_t player_positions[MAXPLAYERS][FIGURES];
uint8_t home_players[FIGURES];
uint8_t activePlayer = 0;
uint8_t players = MAXPLAYERS;
uint8_t activeFigure = 0;
int8_t selection = 0;
boolean autoPlay[MAXPLAYERS];
uint8_t playerRanking[MAXPLAYERS];
uint8_t wonPlayers;
uint8_t dimmed = MINBRIGHTNESS;
byte player_wheel[] = {0, 64, 128, 192};

void setup() {
	field.begin();
	field.show();
	finish.begin();
	finish.show();
	dice.begin();
	dice.show();
	pinMode(SELECT_BUTTON, INPUT_PULLUP);
	digitalWrite(8, HIGH);
	digitalWrite(9, HIGH);
	digitalWrite(10, HIGH);
	digitalWrite(11, HIGH);
	byte c = 0;
	while (!select()) {
		uint32_t color = dim(Wheel(c+=8), 20);
		for (uint8_t i = 0; i < 5; i++) {
			for (uint8_t j = 0; j < 4; j++) {
				field.setPixelColor(i+14*j, color);
				field.setPixelColor(14*(4-j)-i, color);
				finish.setPixelColor(i+j*4, color);
			}
			
			field.show();
			finish.show();
			delay(50);
			if (select()) break;
		}
		if (select()) break;
		for (uint8_t i = 0; i < 3; i++) {
			for (uint8_t j = 0; j < 4; j++) {
				field.setPixelColor(4+i+14*j, color);
				field.setPixelColor(14*(4-j)-5-i, color);
			}
			field.show();
			delay(50);
			if (select()) break;
		}
		if (select()) break;
		for (uint8_t i = 2; i < 3; i--) {
			for (uint8_t j = 0; j < 4; j++) {
				field.setPixelColor(4+i+14*j, 0);
				field.setPixelColor(14*(4-j)-5-i, 0);
			}
			field.show();
			delay(50);
			if (select()) break;
		}
		if (select()) break;
		for (uint8_t i = 4; i < 5; i--) {
			for (uint8_t j = 0; j < 4; j++) {
				field.setPixelColor(i+14*j, 0);
				field.setPixelColor(14*(4-j)-i, 0);
				if (i < 4)
					finish.setPixelColor(i+j*4, 0);
			}
			field.show();
			finish.show();
			delay(50);
			if (select()) break;
		}
	}

	resetGame();
	setDefault();
	show();
}

void resetGame() {
	while(select()) delay(100);
	randomSeed(millis());
	wonPlayers = 0;
	for (uint8_t player = 0; player < MAXPLAYERS; player++) {
		player_positions[player][0] = -1;
		player_positions[player][1] = -2;
		player_positions[player][2] = -3;
		player_positions[player][3] = -4;
		playerRanking[player] = 0;
		undimmed_colors[player] = Wheel(player_wheel[player]);
		player_colors[player] = dim(undimmed_colors[player], dimmed);
	}
	setDefault();
	show();
	selection = 0;

	while(select()) delay(100);
	while (true) {
		if (select()) {
			uint32_t startTime = millis();
			while (select());
			if (startTime + 300 < millis()) {
				break;
			}
			dimmed += 5;
			if (dimmed < MINBRIGHTNESS) //Overflow
				dimmed = MINBRIGHTNESS;
			for (uint8_t player = 0; player < MAXPLAYERS; player++)
				player_colors[player] = dim(Wheel(player_wheel[player]), dimmed);
			setDefault();
			show();
		}
		int8_t b = millis()/10;
		for (uint8_t i = 0; i < 7; i++) {
			dice.setPixelColor(i, dim(0xFF0000, abs(b)));
		}
		dice.show();
	}
	while(select()) delay(100);

	diceNumber(players);
	while (true) {
		if (select()) {
			uint32_t startTime = millis();
			while (select());
			if (startTime + 300 < millis()) {
				break;
			}
			players++;
			if (players > MAXPLAYERS) players = MINPLAYERS;
			setDefault();
			diceNumber(players);
			show();
		}
	}
	
	diceNumber(0);
	while(select()) delay(100);
	delay(400);
	byte c = 0;
	while (!select()) {
		for (uint8_t player = 0; player < players; player++) {
			if (getTouch(player)) {
				player_wheel[player]++;
				undimmed_colors[player] = Wheel(player_wheel[player]);
				player_colors[player] = dim(undimmed_colors[player], dimmed);
			}
			setDefault();
			show();
		}
		uint32_t color = dim(Wheel(c++), dimmed);
		uint8_t m = (c/4) % 4;
		dice.setPixelColor(0, dim(color, 255*(m==0)));
		dice.setPixelColor(1, dim(color, 255*(m==1)));
		dice.setPixelColor(6, dim(color, 255*(m==2)));
		dice.setPixelColor(5, dim(color, 255*(m==3)));
		dice.show();
		delay(20);
	}
	while(select()) delay(100);
	delay(400);
	
	diceNumber(0);
	
	boolean touched[players];
	autoPlay[0] = false;
	autoPlay[1] = false;
	autoPlay[2] = false;
	autoPlay[3] = false;
	while(!select()) {
		setDefault();
		uint8_t s = (millis()/200)%5;
		for (uint8_t player = 0; player < players; player++) {
			boolean v = getTouch(player);
			if (v != touched[player]) {
				touched[player] = v;
				if (v) {
					autoPlay[player] = !autoPlay[player];
				}
			}
			if (autoPlay[player]) {
				for (uint8_t si = 0; si < s; si++) {
					setPixel(player, FINISH1+si, player_colors[player]);
				}
			}
		}
		show();
	}
}

void loop() {
	unsigned long delayTo;
	boolean br;
	for (uint8_t player = 0; player < players; player++) {
		if (playerRanking[player] == 0) {
			br = false;
			for (uint8_t figure = 0; figure < FIGURES; figure++) {
				if (player_positions[player][figure] < 40) {
					br = true;
					break;
				}
			}
			if (!br) {
				playerRanking[player] = ++wonPlayers;
			}
		}
	}
	if (wonPlayers + 1 >= players) {
		for (uint8_t px = 0; px < field.numPixels(); px++)
			field.setPixelColor(px, 0);
		for (uint8_t px = 0; px < finish.numPixels(); px++)
			finish.setPixelColor(px, 0);
		for (uint8_t px = 0; px < dice.numPixels(); px++)
			dice.setPixelColor(px, 0);
		for (uint8_t player = 0; player < players; player++) {
			for (uint8_t px = 40; px < 40 + (playerRanking[player]==0?players:playerRanking[player]); px++) {
				setPixel(player, px, player_colors[player]);
			}
		}
		show();
		while(!select());
		resetGame();
		return;
	}
	diceNumber(0);
	while (true) {
		br = true;
		for (uint8_t figure = 0; figure < FIGURES; figure++) {
			if (player_positions[activePlayer][figure] < 40) {
				br = false;
				break;
			}
		}
		if (br) {
			break;
		}
		uint8_t movement = 0;
		//Maximal 3 Mal würfeln zum rauskommen
		for (uint8_t rolledTimes = 0; rolledTimes < 3; rolledTimes++) {
			movement = rollDice(rolledTimes);
			//Nur, wenn keine Figur bewegbar ist (alle im Ziel oder Haus und keine 6), dann noch einmal versuchen
			if (nextFigure(movement) || nextFigure(1)) break;
		}

		activeFigure = 0;
		//Wenn keine Figur mit der Augenzahl setzbar ist
		if (nextFigure(movement) == false) {
			delayTo = millis() + 500;
			//Blinken
			while (millis() < delayTo) {
				blink(activePlayer);
			}
			setDefault();
			show();
			break;
		}
		previousFigure(movement); // Reset to next playable figure
		
		if (autoPlay[activePlayer]) {
			// Computerspieler
			boolean collides = false;
			//Zuerst versuchen, jemanden zu schlagen
			for (uint8_t i = 0; i < 4 && !collides; i++;) {
				for (uint8_t f = 0; f < FIGURES; f++) {
					for (uint8_t p = 0; p < players; p++) {
						for (uint8_t pf = 0; pf < FIGURES; pf++) {
							collides |= collision(activePlayer, activeFigure, p, pf, movement);
							if (collides) break;
						}
						if (collides) break;
					}
					if (collides) break;
					nextFigure(movement);
				}
			}
			//Wenn das nicht geht, versuchen ins Ziel zu setzen
			for (uint8_t f = 0; f < FIGURES && !collides; f++) {
				if (player_positions[activePlayer][f] < FINISH1 && player_positions[activePlayer][f]+movement >= FINISH1) {
					collides = true; //Variable setzen, damit nicht zufaellig gewaehlt wird
					break;
				}
				nextFigure(movement);
			}
			//Ansonsten einfach irgendeine Figur waehlen
			for (uint8_t f = 0; f < random(FIGURES) && !collides; f++) {
				nextFigure(movement);
			}
		} else {
			// Normaler Spieler
			boolean isTouched = false;
			unsigned long touchStart;
			while (true) {
				if (isTouched && touchStart+500 < millis()) {
					// 500ms druecken setzt den ausgewaehlten Spieler
					break;
				}
				if (getTouch(activePlayer) != isTouched) {
					isTouched = getTouch(activePlayer);
					if (isTouched) {
						touchStart = millis();
					} else {
						nextFigure(movement);
					}
				}
				animateFigure(activePlayer, activeFigure, movement);
			}
		}
		moveFigure(activePlayer, activeFigure, movement);
		if (movement != 6) {
			break;
		}
		while (getTouch(activePlayer) && !autoPlay[activePlayer]);
	}
	activePlayer = (activePlayer + players - 1) % players;
}

void blink(uint8_t player) {
	uint8_t f = (millis() / 100) % 2;
	setDefault();
	setPlayerBrightness(player, f * 255);
	show();
}

void fade(uint8_t player) {
	int8_t f = 127 - (millis()/3) % 256;
	setDefault();
	setPlayerBrightness(player, abs(f) + 127);
	show();
}

void setPlayerBrightness(uint8_t player, uint8_t brightness) {
	uint32_t color = dim(player_colors[player], brightness);
	for (uint8_t figure = 0; figure < FIGURES; figure++) {
		setPixel(player, player_positions[player][figure], color);
	}
}

void setOtherPlayersBrightness(uint8_t player, uint8_t brightness) {
	for (uint8_t p = 0; p < players; p++) {
		if (p == player) continue;
		uint32_t color = dim(player_colors[p], brightness);
		for (uint8_t figure = 0; figure < FIGURES; figure++) {
			setPixel(p, player_positions[p][figure], color);
		}
	}
}

void setDefault(void) {
	for (uint8_t px = 0; px < field.numPixels(); px++) {
		field.setPixelColor(px, 0);
	}
	for (uint8_t px = 0; px < finish.numPixels(); px++) {
		finish.setPixelColor(px, 0);
	}
	for (uint8_t player = 0; player < players; player++) {
		for (uint8_t figure = 0; figure < FIGURES; figure++) {
			setPixel(player, player_positions[player][figure], player_colors[player]);
		}
	}
}

void show(void) {
	finish.show();
	field.show();
	dice.show();
}

boolean getTouch(uint8_t player) {
	uint8_t p = player;
	if (players == 2 && player == 1) p++;
	switch (p) {
		case 0:
			return !digitalRead(8);
		case 1:
			return !digitalRead(11);
		case 2:
			return !digitalRead(10);
		case 3:
			return !digitalRead(9);
	}
	return 0;
}

uint8_t rollDice(void) {
	return rollDice(false);
}
uint8_t rollDice(boolean again) {
	if (!again)
		diceNumber(0);
	uint8_t number = random(6);
	uint8_t i = 0;
	if (autoPlay[activePlayer]) {
		for (char c = 0; c < 100+number; c++) {
			i++;
			i%=6;
			diceNumber(i+1);
			delay(1); //Schnelle Animation
		}
	} else {
		while (!getTouch(activePlayer)) {
			fade(activePlayer);
			if (!again)
				animateDice();
		}
		setDefault();
		show();
		while (getTouch(activePlayer)) {
			i++;
			i%=6;
			diceNumber(i+1);
			delay(1); //Schnelle Animation
		}
	}
	for (char c = 0; c<11; c++) {
		diceNumber(i%6+1);
		delay(c);
		i++;
	}
	for (char c = 1; c<8; c++) {
		diceNumber(i%6+1);
		delay(10 * c);
		i++;
	}
	for (char c = 1; c<9; c++) {
		diceNumber(i%6+1);
		delay(80 * c);
		i++;
	}
	diceNumber(i%6+1);
	if (autoPlay[activePlayer]) {
		delay(700);
	}
	return i%6+1;
}

void diceNumber(uint8_t number) {
	diceNumber(number, 255);
}
void diceNumber(uint8_t number, uint8_t brightness) {
	dice.setPixelColor(0, dim(player_colors[activePlayer], brightness*(number & 4)));
	dice.setPixelColor(6, dim(player_colors[activePlayer], brightness*(number & 4)));
	dice.setPixelColor(2, dim(player_colors[activePlayer], brightness*(number == 6)));
	dice.setPixelColor(4, dim(player_colors[activePlayer], brightness*(number == 6)));
	dice.setPixelColor(3, dim(player_colors[activePlayer], brightness*(number & 1)));
	dice.setPixelColor(1, dim(player_colors[activePlayer], brightness*((number & 2)|(number & 4))));
	dice.setPixelColor(5, dim(player_colors[activePlayer], brightness*((number & 2)|(number & 4))));
	dice.show();
}
void animateDice(void) {
	uint8_t m = (millis()/80) % 4;
	dice.setPixelColor(0, dim(player_colors[activePlayer], 255*(m==0)));
	dice.setPixelColor(1, dim(player_colors[activePlayer], 255*(m==1)));
	dice.setPixelColor(6, dim(player_colors[activePlayer], 255*(m==2)));
	dice.setPixelColor(5, dim(player_colors[activePlayer], 255*(m==3)));
	dice.show();
}

boolean select(void) {
	return !digitalRead(SELECT_BUTTON);
}

void animateFigure(uint8_t player, uint8_t figure, uint8_t movement) {
	uint8_t step = (millis() / 250) % (movement + 2);
	int8_t destination = player_positions[player][figure] + step - (step == movement + 1);
	if (player_positions[player][figure] < 0) {
		destination = 0;
		step = (millis() / 250) % 3;
	}
	setDefault();
	if (step > 0) {
		setPixel(player, destination, dim(player_colors[player], 127)); //override color
	}
	show();
}

void moveFigure(uint8_t player, uint8_t figure, uint8_t movement) {
	int8_t destination = player_positions[player][figure] + movement;
	uint8_t remainingMoves = movement;
	if (player_positions[player][figure] < 0) {
		destination = 0;
		remainingMoves = 1;
	}
	while (player_positions[player][figure] < destination) {
		if (player_positions[player][figure] < 0) {
			player_positions[player][figure] = 0;
		} else {
			player_positions[player][figure]++;
		}

		setDefault();
		setPixel(player, player_positions[player][figure], player_colors[player]); //override color
		diceNumber(--remainingMoves);
		show();
		delay(250);
	}
	diceNumber(0);
	for (uint8_t p = 0; p < players; p++) {
		if (p == player) continue;
		for (uint8_t f = 0; f < FIGURES; f++) {
			if (collision(player, figure, p, f, 0)) {
				goHome(p, f);
				return;
			}
		}
	}
	setDefault();
	show();
}

void goHome(uint8_t player, uint8_t figure) {
	uint8_t homed = homeFigures(player) + 1;
	while (player_positions[player][figure] >= 0) {
		player_positions[player][figure]--;
		if (player_positions[player][figure] < 0) player_positions[player][figure] = -homed; //skip homed
		delay(50);
		setDefault();
		setPixel(player, player_positions[player][figure], player_colors[player]); //override color
		show();
	}
}

uint8_t homeFigures(uint8_t player) {
	uint8_t figures = 0;
	for (uint8_t i = 0; i < FIGURES; i++) {
		figures += player_positions[player][i] < 0;
	}
	return figures;
}

void setPixel(uint8_t player, int8_t position, uint32_t color) {
	if (position < 40) {
		field.setPixelColor(getPixel(player, position), color);
	} else {
		if (players == 2 && player == 1) player++;
		switch (player) {
			case 0:
				finish.setPixelColor(((43 - position) + FIGURES * 0) & 0b1111, color);
				break;
			case 1:
				finish.setPixelColor(((43 - position) + FIGURES * 3) & 0b1111, color);
				break;
			case 2:
				finish.setPixelColor(((43 - position) + FIGURES * 2) & 0b1111, color);
				break;
			case 3:
				finish.setPixelColor(((43 - position) + FIGURES * 1) & 0b1111, color);
				break;
		}
	}
}

uint8_t getPixel(uint8_t player, int8_t position) {
	if (players == 2 && player == 1) player++;
	uint8_t pixel = 74;
	player = (player + 2) & 0b11;
	pixel -= position;
	pixel += field.numPixels() / MAXPLAYERS * player;	//Turn field by 90deg depending on player
	if (position >= 0)
		pixel -= position / 10 * FIGURES;				//compensate home fields
	return pixel % field.numPixels();
}

uint32_t dim(uint32_t color, uint8_t factor) {
	uint32_t r, g, b;
	r = (color >> 16) & 0xFF;
	g = (color >> 8)	& 0xFF;
	b =	color				& 0xFF;
	r = (r * factor) >> 8;
	g = (g * factor) >> 8;
	b = (b * factor) >> 8;
	color = (r << 16) + (g << 8) + b;
	return color;
}

//Naechste bewegbare Figur waehlen, false falls es keine gibt
boolean nextFigure(uint8_t movement) {
	uint8_t figure = activeFigure;
	for (uint8_t i = 1; i <= FIGURES; i++) {
		figure = (activeFigure + i) % FIGURES;
		if (isMoveable(activePlayer, figure, movement)) {
			activeFigure = figure;
			return true;
		}
	}
	return false;
}
//Und vorherige
boolean previousFigure(uint8_t movement) {
	for (int8_t i = 3; i >= 0; i--) {
		if (isMoveable(activePlayer, (activeFigure + i) % FIGURES, movement)) {
			activeFigure = (activeFigure + i) % FIGURES;
			return true;
		}
	}
	return false;
}

//Hier passiert die mathematische Magie...
boolean isMoveable(uint8_t player, uint8_t figure, uint8_t movement) {
	boolean startBlocked = false;
	uint8_t homed = homeFigures(player);
	for (uint8_t i = 0; i < FIGURES; i++) {
		if (player_positions[player][i] == 0) {
			startBlocked = true;
			if (homed > 0 && i != figure && isMoveable(player, i, movement)) {
				//Startposition muss geraeumt werden
				return false;
			}
		}
	}
	if (movement == 6 && !startBlocked && homed != 0) {
		//Haus muss leer werden, sofern die Startposition nicht versperrt ist
		return player_positions[player][figure] == -homed;
	}
	if ((movement != 6 || startBlocked) && player_positions[player][figure] < 0) {
		//Startoisition versperrt oder keine 6
		return false;
	}
	if (player_positions[player][figure] + movement > FINISH4) {
		//Nicht ueber das Ziel hinausschiessen!
		return false;
	}
	for (uint8_t i = 0; i < FIGURES; i++) {
		//Der Weg wird von einer eigenen Figur blockiert
		if (player_positions[player][figure] + movement == player_positions[player][i]) {
			return false;
		}
	}
	for (uint8_t i = 0; i < FIGURES; i++) {
		if (i == figure) continue;
		//Im Ziel nicht ueberspringen, auskommentieren, wenn man es doch duerfen soll
		if (player_positions[player][figure] + movement >= FINISH1 && player_positions[player][i] >= FINISH1 && player_positions[player][figure] < player_positions[player][i] && player_positions[player][figure] + movement > player_positions[player][i]) {
			return false;
		}
	}
	return true;
}

boolean collision(uint8_t player1, uint8_t figure1, uint8_t player2, uint8_t figure2, uint8_t movement) {
	//Simple Kollisionserkennung zwischen P1F1+Wuerfelaugen und P2F2
	if (player_positions[player1][figure1] + movement >= FINISH1
			|| player_positions[player2][figure2] >= FINISH1
			|| player_positions[player1][figure1] <= HOME1
			|| player_positions[player2][figure2] <= HOME1) {
		return false;
	}
	return getPixel(player1, player_positions[player1][figure1] + movement) == getPixel(player2, player_positions[player2][figure2]);
}

uint32_t Wheel(byte WheelPos) {
	//HSB Hue zu RGB
	if(WheelPos < 85) {
	 return field.Color(WheelPos * 3, 255 - WheelPos * 3, 0);
	} else if(WheelPos < 170) {
	 WheelPos -= 85;
	 return field.Color(255 - WheelPos * 3, 0, WheelPos * 3);
	} else {
	 WheelPos -= 170;
	 return field.Color(0, WheelPos * 3, 255 - WheelPos * 3);
	}
}
