
#include <Arduboy2.h>

Arduboy2 arduboy;

#define MAX_NUM_FIREWORKS 6
#define NUM_STARS 17

#define FW_ONE   0
#define FW_TWO   1
#define FW_THREE 2
#define FW_FOUR  3
#define FW_FIVE  4
#define FW_SIX   5


class Fuze
{

public:
	// No args constructor just sets defaults for now
	Fuze()
	{

		length = 1;
		t = 0;

		tick_count = 0;
		burn_speed = 25;

		status = false;
		lit = false;

	}


	void light()
	{

		lit = true;

	}


	bool islit()
	{

		return lit;

	}


	// true on completed
	bool tick()
	{

		if (status)
		{

			return true;

		} else {

			if (t >= length)
			{

				status = true;
				return true;

			}

		}

		if (lit)
		{

			tick_count++;

			if (tick_count % burn_speed == 0)
			{

				t++;

			}

		}

		return false;

	}


	void draw(int8_t x, int8_t y)
	{

		if (lit)
		{

			return;

		}

		int8_t y_offset = -1;

		
		arduboy.drawPixel(x, y + y_offset);
		

	}


private:
	uint8_t length;
	uint8_t t;

	uint8_t tick_count;
	uint8_t burn_speed;

	bool status;
	bool lit;

};



// Base of firework used as location
class Morter
{

public:

	Morter()
	{
		x = 0;
		y = 0;

	}

	int16_t get_base() 
	{

		return y;

	}


	int16_t get_horizontal()
	{

		return x;

	}


	Morter(int16_t x_init, int16_t y_init)
	{
		x = x_init;
		y = y_init;

	}

	void draw()
	{

		arduboy.drawLine(x, y, x, y + 1);

	}

private:
	int16_t x;
	int16_t y;

};



class AerielShell
{


public:
	AerielShell()
	{

		num_stars = NUM_STARS;
		gun_powder = 10; 
		trigger_height = 10; 
		y_delta = 0;

		tick_count = 0;
		tick_rate = 7;

		radius = 1;

		sustain = 7; 
		p_twinkle = 255;


		init_boom = true;
		launch_finished = false;
		finished = false;

	}

	void set(uint8_t t_height, uint8_t g_powder, uint8_t sust)
	{

		trigger_height = t_height;
		gun_powder = g_powder;
		sustain = sust;

	}


	uint8_t get_trigger_height()
	{

		return trigger_height;

	}


	void thrust()
	{

		y_delta--;

	}


	// The part fun part
	bool explode(int16_t x, int16_t y)
	{

		y += y_delta;

		if (init_boom && radius < gun_powder / 2)
		{

			for (uint8_t i = 0; i < NUM_STARS; ++i)
			{

				int16_t x_p = static_cast<int16_t>(cos(i) * radius);
				int16_t y_p = static_cast<int16_t>(sin(i) * radius);
				arduboy.drawLine(x, y, x + x_p, y + y_p);

			}

			if (tick_count % 2 == 0)
			{

				radius++;

			}

			tick_count++;

			return false;
		
		}

		init_boom = false;

		if (radius < gun_powder)
		{

			for (uint8_t i = 0; i < NUM_STARS; ++i)
			{

				int16_t xsign = (rand() % 2) * -2 + 1;
				int16_t ysign = (rand() % 2) * -2 + 1;

				int16_t x_offset = xsign * (rand() % radius);
				int16_t y_offset = ysign * (rand() % radius);

				int16_t distance = (int16_t)sqrt((x - (x + x_offset)) * (x - (x + x_offset)) + (y - (y + y_offset)) * (y - (y + y_offset)));


				if ((rand() % 255) <= p_twinkle && distance < radius)
				{

					arduboy.drawPixel(x + x_offset, y + y_offset);

				}

			}

			if (tick_count % sustain == 0)
			{

				radius++;

			}
			
			if (tick_count % sustain && p_twinkle - 5 >= 0)
			{

				p_twinkle -= 5;

			}

			tick_count++;

			return false;

		}


		return true;

	}


	// return true on complete
	bool tick(int16_t y_base)
	{

		if (launch_finished)
		{

			return true;

		}

		// On launch
		if (y_delta < 0)
		{

			if (y_delta + y_base <= trigger_height)
			{

				// Boom!
				launch_finished = true;
				return true;

			}

		} 

		return false;

	}


	void draw(int16_t x, int16_t y_base)
	{

		if (y_delta < 0 && !(launch_finished))
		{

			if (!(launch_finished) && (tick_count % tick_rate == 0))
			{

				arduboy.drawPixel(x, y_base + y_delta);

			}


			tick_count++;

		}

	}


private:
	uint8_t num_stars;
	uint8_t gun_powder;
	uint8_t trigger_height;
	int16_t y_delta;

	// Used to create a flashing effect for the aerial as it shoots from the morter.
	uint8_t tick_count;
	uint8_t tick_rate;

	int16_t radius;

	uint8_t sustain;

	uint8_t p_twinkle;

	bool init_boom;
	bool launch_finished;
	bool finished;

};



class Firework
{

public:

	Firework() 
	{

		launched = false;
		exploding = false;
		done = false;

	}


	void draw()
	{

		morter.draw();
		fuze.draw(morter.get_horizontal(), morter.get_base());
		aeriel.draw(morter.get_horizontal(), morter.get_base());

	}


	Firework(int16_t x, int16_t y)
	{

		// base of firework
		morter = Morter(x, y);
		fuze = Fuze();
		aeriel = AerielShell();

		launched = false;
		exploding = false;
		done = false;

	}


	void set(uint8_t t_height, uint8_t g_powder, uint8_t sust)
	{

		aeriel.set(t_height, g_powder, sust);

	}


	void tick()
	{


		if (!(launched) && (!done))
		{

			launched = fuze.tick();

		}
		

		if (launched && (!exploding))
		{

			aeriel.thrust();

		}

		exploding = aeriel.tick(morter.get_base());

		done = false;

		if (exploding)
		{

			// explosion logic
			done = aeriel.explode(morter.get_horizontal(), morter.get_base());

		}

	}


	void light()
	{

		fuze.light();

	}

	bool completed()
	{

		return done;

	}

 	int16_t get_x()
	{

		return morter.get_horizontal();

	}


	int16_t get_y()
	{

		return morter.get_base();

	}	


private:
	Morter morter;
	Fuze fuze;
	AerielShell aeriel;

	bool launched;
	bool exploding;
	bool done;

};


Firework fireworks[MAX_NUM_FIREWORKS];

class Pyromaniac
{

public:
	Pyromaniac() = default;

	// Really just does button logic
	void play()
	{

		if (arduboy.justPressed(B_BUTTON)) {

			fireworks[FW_SIX].light();

		} 

		if (arduboy.justPressed(A_BUTTON)) {

			fireworks[FW_FIVE].light();

		} 

		if (arduboy.justPressed(LEFT_BUTTON)) {

			fireworks[FW_ONE].light();

		} 

		if (arduboy.justPressed(RIGHT_BUTTON)) {

			fireworks[FW_THREE].light();

		} 

		if (arduboy.justPressed(DOWN_BUTTON)) {

			fireworks[FW_FOUR].light();

		} 

		if (arduboy.justPressed(UP_BUTTON)) {

			fireworks[FW_TWO].light();

		}		

	}

private:

};


Pyromaniac pyromaniac;


class FireworksShow
{

public:
	FireworksShow() = default;
	
	void enjoy()
	{

		for (uint8_t i = 0; i < MAX_NUM_FIREWORKS; ++i)
		{

			fireworks[i].tick();
			fireworks[i].draw();

			if (fireworks[i].completed())
			{

				fireworks[i] = Firework(fireworks[i].get_x(), fireworks[i].get_y());

				uint8_t gun_powder = ((rand() % 20) + 15);
				fireworks[i].set((rand() % 14) + 5 ,gun_powder, 5);

			}

		}

	}

};


FireworksShow fireworks_show;


void setup()
{

	arduboy.begin();
	arduboy.setFrameRate(60);
	arduboy.initRandomSeed();
	arduboy.clear();

	for (int16_t i = 0; i < MAX_NUM_FIREWORKS; ++i) 
	{
		uint8_t gun_powder = ((rand() % 20) + 15);
		
		fireworks[i] = Firework((i + 1) * 16 + 8, 58);
		fireworks[i].set((rand() % 14) + 5 ,gun_powder, 5);
	}


}



void loop()
{

	if (!(arduboy.nextFrame()))
	{

		return;

	}

	arduboy.clear();
	

	arduboy.pollButtons();
	
	pyromaniac.play();

	arduboy.drawLine(0, 60, 128, 60);
	
	fireworks_show.enjoy();
	
	
	arduboy.display();

}


