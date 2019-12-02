#include <fstream>
#include <cstdlib>
#include <vector>
#include <string>
#include <ctime>
#include <iostream>
#include <stdio.h>
#include <math.h>
#include <cmath>
#include <algorithm>
#include <Windows.h>
#include <GL\freeglut.h>

using namespace std;


//set up some constants
#define X_CENTRE 0.0      /* centre point of square */
#define Y_CENTRE 0.0
#define LENGTH   1.0      /* lengths of sides of square */
using namespace std;
int gMenu;
void writeString(void* font, const char* string)
{
	//from tutorial code [bars and lines]
	const char* c;
	for (c = string; *c != '\0'; c++) glutBitmapCharacter(font, *c);
}
#pragma region classes here

class baseObj {
public:
	//bb1 and bb2 arethe coords of the bounding box and centre is the mid point
	pair<GLfloat, GLfloat> coords, bb1, bb2, velocity;
	GLfloat angle, radius, cf = 3.14 / 180;
	vector<pair<GLfloat, GLfloat>> points;
	bool active;
	baseObj(pair<GLfloat, GLfloat> _startCoords) {
		coords = _startCoords;
		velocity = { 0,0 };
		active = true;
	}
	virtual void update() { cout << "base" << endl; }
	virtual void draw() {
		glBegin(GL_POLYGON);
		if (!active) { glColor3f(0, 1, 0); }
		for (int i = 0; i < points.size(); i++) {
			glVertex2f(points[i].first, points[i].second);
		}
		glEnd();
	}
	void set_active(bool _new) {
		active = _new;
	}
	void destroy() {
		points.clear();
		set_active(false);
	}

};
class player :public baseObj {
public:
	player(pair<GLfloat, GLfloat> _startCoords) :baseObj(_startCoords) {
		angle = 0;
		radius = 20;
		points = {
			{_startCoords.first, _startCoords.second},
			{_startCoords.first + radius * cos(cf * (angle + 240)), _startCoords.second + radius * sin(cf * (angle + 240))},
			{_startCoords.first + radius * cos(cf * (angle)), _startCoords.second + radius * sin(cf * (angle))},
			{_startCoords.first + radius * cos(cf * (angle + 120)), _startCoords.second + radius * sin(cf * (angle + 120))},
		};
	};
	void update() override {
		points = {
			{coords.first, coords.second},
			{coords.first + radius * cos(cf * (angle + 240)), coords.second + radius * sin(cf * (angle + 240))},
			{coords.first + radius * cos(cf * (angle)), coords.second + radius * sin(cf * (angle))},
			{coords.first + radius * cos(cf * (angle + 120)), coords.second + radius * sin(cf * (angle + 120))},
		};
	}
};
class asteroid :public baseObj {
public:
	GLfloat sides, rotSpeed;
	//random color for asteroid
	float color[3] = { static_cast <float> (rand()) / static_cast <float> (RAND_MAX), 
		static_cast <float> (rand()) / static_cast <float> (RAND_MAX) ,
		static_cast <float> (rand()) / static_cast <float> (RAND_MAX)};
	asteroid(pair<GLfloat, GLfloat> _startCoords) :baseObj(_startCoords) {
		//randomise size
		radius = 20 - (rand() % 10)+1;
		//randomise side number and speed of rotation
		sides = rand()%10+3;
		rotSpeed = ((rand() % 10) +1)*0.01;
		angle = 0;

		//make regular sided polygon
		for (int i = 1; i < sides+1; i++) {
			
			points.push_back({ 
				_startCoords.first + radius * cos(cf * (angle + 360 /sides * i)),
				_startCoords.second + radius * sin(cf * (angle + 360 / sides * i)) });
		}
	}
	void update() {
		//make asteroid rotate slowly
		if (angle < 360) { angle+=rotSpeed; }
		else { angle = 0; }
		//updating points
		points.clear();
		for (int i = 0; i < sides; i++) {
			points.push_back({
				coords.first + radius * cos(cf * (angle + 360 / sides * i)),
				coords.second + radius * sin(cf * (angle + 360 / sides * i))
				});
		}
	}
};
class bullet :public baseObj {
public:
	bullet(pair<GLfloat, GLfloat> _startCoords, GLfloat _angle) :baseObj(_startCoords) {
		radius = 20;
		//randomise side number and speed of rotation
		angle = _angle;
		//diamond shaped projectile
		points = {
			{_startCoords.first+5*cos(cf*angle),_startCoords.second + 5 * sin(cf * angle)},
			{_startCoords.first + 5 * cos(cf * (angle + 90)),_startCoords.second + 5 * sin(cf * (angle + 90))},
			{_startCoords.first + 10 * cos(cf * (angle + 180)),_startCoords.second + 10 * sin(cf * (angle + 180))},
			{_startCoords.first + 5 * cos(cf * (angle - 90)),_startCoords.second + 5 * sin(cf * (angle - 90))},
		};
	}
	void update(vector<asteroid> &_asteroidList) {
		checkifout();
		checkforTouch(_asteroidList);

		if (active) {
			coords.first += cos(cf * angle) * 0.1;
			coords.second += sin(cf * angle) * 0.1;
			points = {
				{coords.first + 5 * cos(cf * angle),coords.second + 5 * sin(cf * angle)},
				{coords.first + 5 * cos(cf * (angle + 90)),coords.second + 5 * sin(cf * (angle + 90))},
				{coords.first + 10 * cos(cf * (angle + 180)),coords.second + 10 * sin(cf * (angle + 180))},
				{coords.first + 5 * cos(cf * (angle - 90)),coords.second + 5 * sin(cf * (angle - 90))},
			};
		}
	}
	void checkforTouch(vector<asteroid>& _asteroidList) {
		for (asteroid& asteroid : _asteroidList) {
			GLfloat dis = sqrt(pow(asteroid.coords.first - coords.first, 2) + pow(asteroid.coords.second - coords.second, 2));
			if (dis < 20) {
				//destroy self and other object
				asteroid.destroy();
				destroy();
			}
		}
	}
	void checkifout() {
		if (coords.first < -10 || coords.first > 510 || coords.second < -10 || coords.second > 510) {
			destroy();
		}
	}
};
class game {
public:
	GLint score;
	player p = player({ 250,250 });
	asteroid a = asteroid({100,100});
	vector<asteroid> asteroidList;
	vector<bullet> bulletList;
	bool debugAim,grid,pattern;
	game() {
		//making the numbers more random
		srand((time(NULL)));
		score = 0;
		debugAim = grid =  false;
		pattern = true;
		cout << "WELCOME TO ASTEROIDS" << endl;
		cout << "------------------------------" << endl;
		cout << "LEFT KEY/A-rotate left" << endl;
		cout << "RIGHT KEY/D-rotate right" << endl;
		cout << "UP KEY/SPACE-shoot" << endl;
		cout << "RIGHT MOUSE - open menu" << endl;
	}
	void Update() {
		//initialize asteroids
		if(asteroidList.size() < 1){ createAsteroids(rand() % 10 + 1); }
		//update player
		p.update();
		
		
		//update bullets
		for (bullet& bull : bulletList) {
			bull.update(asteroidList);
		}
		for (int i = 0; i < bulletList.size(); i++) {
			if (!bulletList[i].active) {
				bulletList.erase(bulletList.begin() + i);
			}
		}
		//*check if asteroids and bullets are still active
		for (int i = 0; i < asteroidList.size(); i++) {
			if (!asteroidList[i].active) {
				asteroidList.erase(asteroidList.begin() + i);
				score += 10;
			}
		}
		//*/
		for (asteroid& asteroid : asteroidList) {
				asteroid.update();
		}
		

		
	}
	void Draw() {
		//drawing two of the extras
		if (grid) {
			glColor3f(0, 0.5, 0.5);
			glBegin(GL_LINES);
			for (int i = 0; i < 500; i += 50) {
				glVertex2f(i, 0);
				glVertex2f(i, 500);

				glVertex2f(0, i);
				glVertex2f(500, i);
			}
			glEnd();
		}
		if (pattern) {
			glColor3f(0.5, 0, 0.5);
			glBegin(GL_POLYGON);
			for (int i = 0; i < 500; i += 50) {
				for (int j = 0; j < 500; j += 50) {
					glBegin(GL_POLYGON);
					glVertex2f(i + 25, j);
					glVertex2f(i + 50, j + 25);
					glVertex2f(i + 25, j + 50);
					glVertex2f(i, j + 25);
					glEnd();
				}
			}
			
		}
		
		//draw asteroids and bullets
		glColor3f(0.5, 0.5, 0.5);
		for (bullet& bull : bulletList) {
			bull.draw();
		}
		for (asteroid& asteroid : asteroidList) {
			glColor3f(asteroid.color[0], asteroid.color[1], asteroid.color[2]);
			asteroid.draw();

			if (debugAim) {
				glColor3f(1, 0, 0);
				glBegin(GL_LINES);
				glVertex2f(asteroid.coords.first, asteroid.coords.second);
				glVertex2f(p.coords.first, p.coords.second);
				glEnd();
			}
		}

		//draw Player
		glColor3f(0.1, 0.3, 0.2);
		p.draw();

		//draw the HUD
		#pragma region backboard
		glColor3f(103.0/255, 154.0/255, 168.0/255);
		glBegin(GL_POLYGON);
		glVertex2f(0,530);
		glVertex2f(500,530);
		glVertex2f(500,500);
		glVertex2f(0,500);
		glEnd();
		#pragma endregion
		#pragma region score
		glColor3f(0, 0, 0);
		glRasterPos2f(10,510);
		string dis = "Score:";
		dis += to_string(score);
		writeString(GLUT_BITMAP_HELVETICA_18, dis.c_str());
		
		#pragma endregion
	}
	void createAsteroids(int number) {
		for (int i = 0; i < number; i++) {
			bool placed = false;
			int xPlace, yPlace;
			while (!placed) {
				xPlace =  (rand() % 10) * 48;
				yPlace =  (rand() % 10) * 48;
				if (!(xPlace > 20 && xPlace < 200) && !(xPlace > 300 && xPlace < 480)) { continue; }
				if (!(yPlace > 20 && yPlace < 200) && !(yPlace > 300 && yPlace < 480)) { continue; }
				placed = true;
			}
			asteroidList.push_back(asteroid({ xPlace , yPlace }));
		}
	}
	void createBullet() {
		bulletList.push_back(bullet({250,250},p.angle));
	}
};

game newGame;

#pragma endregion
// reshape callback function executed when window is moved or resized.
void reshape(GLsizei w, GLsizei h)
{

	glMatrixMode(GL_PROJECTION);

	// Set Viewport to window dimensions
	glViewport(0, 0, w, h);

	// Reset coordinate system
	glLoadIdentity();

	glOrtho(0,500,0,530,-1,1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}
void menuFunc(int buttonid) {
	switch (buttonid) {
	case 1:
		newGame.debugAim = !newGame.debugAim;
		break;
	case 2:
		newGame.grid = !newGame.grid;
		break;
	case 3:
		newGame.pattern = !newGame.pattern;
		break;
	}
}
// all drawing code goes in here
void display()
{
	// Clear the window with current clearing colour
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColor3f(1.0, 1.0, 1.0); 

	/* white drawing objects */
	//drawing code goes here
	newGame.Update();
	newGame.Draw();
	
	glutSwapBuffers();
	glutPostRedisplay();
}
void keyboard(int key, int x, int y) {
	if (key == GLUT_KEY_LEFT){
			newGame.p.angle += 2;
	}
	if(key == GLUT_KEY_RIGHT){
		newGame.p.angle-=2;
	}
	if (key == GLUT_KEY_UP) {
		if (newGame.bulletList.size() < 3) {
			newGame.createBullet();
		}
	}
}
void charkeys(unsigned char key, int x, int y) {
	switch (key) {
	case 'a':
		newGame.p.angle += 2;
		break;
	case 'd':
		newGame.p.angle -= 2;
		break;
	case ' ':
		if (newGame.bulletList.size() < 3) {
			newGame.createBullet();
		}
			break;
	}
}
void init(void)
{
	//background colour
	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
}
//rename this to main(...) and change example 2 to run this main function
int main(int argc, char** argv)
{
	
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_DOUBLE);
	glutInitWindowSize(500, 530);
	glutCreateWindow("Daniel Eji Asteroids");
	#pragma region menu
	//*
	glutCreateMenu(menuFunc);
	glutAddMenuEntry("toggle debug aim", 1);
	glutAddMenuEntry("toggle grid", 2);
	glutAddMenuEntry("toggle pattern", 3);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
	//*/
#pragma endregion
	glutDisplayFunc(display);

	init();



	//keyboard inputs
	glutSpecialFunc(keyboard);
	glutKeyboardFunc(charkeys);
	glutReshapeFunc(reshape);

	glutMainLoop();
	return 0;
}