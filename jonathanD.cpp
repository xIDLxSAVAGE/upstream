// Author: Jonathan Dinh
// Date written: 4/18/2016
// Last Modified: 5/02/2016
// Purpose: This file is responsible for all lilypads related functions such
// as spawning lilypads, deleting lilypads, and behaviors of lilypads
//

#include <GL/glx.h>
#include "jonathanD.h"
#include "ppm.h"
#include "game.h"
#include <string>
#include <sstream>
extern "C" {
#include "fonts.h"
}

using namespace std;
struct Game game;
unsigned char *buildAlphaData2(Ppmimage *img);

float fade = 1.0;
void drawRipple(int x, int y) {
    if (fade <= 0)
        fade = 1.0;
    float radius = 20;
    int detail = 400;
    float radian = 2.0 * 3.14;
    glEnable(GL_BLEND);
	glPushMatrix();
	glBindTexture(GL_TEXTURE_2D, 0);
	glColor4f (1.0,0.0,0.0,fade-=0.01 );
	glBegin ( GL_LINE_LOOP );
	for ( int i = 0; i <= detail; i++ ) {
		glVertex2f (
		    x + ( radius * cos ( (float)i * radian / (float)detail ) ),
		    y + ( radius * sin ( (float)i * radian / (float)detail ) )
		);
	}
	glEnd();
	glPopMatrix();
    glDisable(GL_BLEND);
}

void createLily(const int n, Game *game)
{
    int wid;
	if (game->score < 300) {
        wid = 35;
	}
	if (game->score >= 300) {
        wid = 30;
	}
	if (game->score >= 600) {
        wid = 25;
	}
    if (game->score >= 1000) {
        wid = 20;
	}
	if (game->score >= 1500) {
        wid = 15;
	}

	//if (game->nlily >= 1) return;
	for (int i =0; i < n; i++) {
		Lilypad *node = new Lilypad;
		if (node == NULL) {
			//Log("error allocating node.\n");
			exit(EXIT_FAILURE);
		}
		node->size = wid;
		node->prev = NULL;
		node->next = NULL;
		int random = rand() % (game->windowWidth - 120) + 60;
		node->pos[0] = game->windowWidth - random;
		node->pos[1] = game->lilyspawnpoint;
		node->vel[1] = -2.0f;
		node->next = game->ihead;
		if (game->ihead != NULL) {
			game->ihead->prev = node;
		}
		game->ihead = node;
		game->nlily++;
	}
}

void checkLilies(Game *game)
{
	//game timer for when to spawn new lily
	game->timer++;
	if (game->timer >= game->lilytimer) {
		createLily(1,game);
		game->nlily++;
		game->timer = 0;
	}
	//lilies falling down
	Lilypad *node = game->ihead;
	while (node) {
		node->pos[1] += node->vel[1];
		//delete lily if it falls below certain height
		if (node->pos[1] <= 0.0) {
			deleteLily(node, game);
		}
		node = node->next;
		//check y pos to set score to 0
		if (game->c.center[1] == 15.0) {

		}
	}
}

void deleteLily(Lilypad *node, Game *game)
{
	if (node) {
        //cout << node->pos[0] << " " << node->pos[1] << endl;
        drawRipple((int)node->pos[0],(int)node->pos[1]);
		if (node->next == NULL && node->prev == NULL) {
			game->ihead = NULL;
			//delete(node);
			//node = NULL;
		} else if (node->next && node->prev) {
			node->prev->next = node->next;
			node->next->prev = node->prev;
			delete(node);
			node = NULL;
		} else if (node->prev == NULL && node->next) {
			game->ihead = node->next;
			node->next->prev = NULL;
			delete(node);
			node = NULL;
		} else if (node->next == NULL && node->prev) {
			node->prev->next = NULL;
			delete(node);
			node = NULL;
		}
	}
	game->nlily--;
}

void clearLilies(Game *game)
{
	Lilypad *node = game->ihead;
	while (node) {
		deleteLily(node, game);
		node = node->next;
	}
}

int check = 0;
void drawLilies(Game *game)
{
	Lilypad *node = game->ihead;
	while (node) {

		glPushMatrix();
		glTranslatef(node->pos[0], node->pos[1], 0);

		if (check>=40 && check <80) {
			//if (r>9)
			glBindTexture(GL_TEXTURE_2D, game->lily->lillyTexture[0]);
			check++;
		}
		if (check >= 80) {
			//if (r<=1)
			glBindTexture(GL_TEXTURE_2D, game->lily->lillyTexture[2]);
			check++;
		}
		if (check>100)
			check =0;

		if (check<40) {
			glBindTexture(GL_TEXTURE_2D, game->lily->lillyTexture[1]);
			check++;
		}

		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER, 0.0f);
		glColor4ub(255,255,255,255);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 1.0f);
		glVertex2i(-node->size,-node->size);
		glTexCoord2f(0.0f, 0.0f);
		glVertex2i(-node->size, node->size);
		glTexCoord2f(1.0f, 0.0f);
		glVertex2i( node->size, node->size);
		glTexCoord2f(1.0f, 1.0f);
		glVertex2i( node->size,-node->size);
		glEnd();
		glPopMatrix();
		glDisable(GL_ALPHA_TEST);
		glDisable(GL_TEXTURE_2D);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_TEXTURE_2D);
		node = node->next;
	}
}

unsigned char *buildAlphaData2(Ppmimage *img)
{
	//add 4th component to RGB stream...
	int i;
	int a,b,c;
	unsigned char *newdata, *ptr;
	unsigned char *data = (unsigned char *)img->data;
	newdata = (unsigned char *)malloc(img->width * img->height * 4);
	ptr = newdata;
	for (i=0; i<img->width * img->height * 3; i+=3) {
		a = *(data+0);
		b = *(data+1);
		c = *(data+2);
		*(ptr+0) = a;
		*(ptr+1) = b;
		*(ptr+2) = c;
		*(ptr+3) = (a|b|c);
		ptr += 4;
		data += 3;
	}
	return newdata;
}

void drawScore(int s, Game *game,int wid)
{
	if (wid==0) {
		wid =20;
	}
	//drawing score using sprites
	string score;
	stringstream out;
	int size;
	int xpos = 20;
	//int ypos = game->windowHeight-50;
	out << s;
	score = out.str();
	size = score.length();
	for (int i = 0; i < size; i++) {
		char cdigit = score[i];
		int idigit = cdigit - '0'; //ghetto atoi
		//draw score
		glPushMatrix();
		glTranslatef(xpos+=30, HEIGHT-100, 0);
		glBindTexture(GL_TEXTURE_2D, game->hscore->scoreTexture[idigit]);
		glEnable(GL_ALPHA_TEST);
		glAlphaFunc(GL_GREATER, 0.0f);
		glColor4ub(255,255,255,255);
		glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 1.0f);
		glVertex2i(-wid,-wid);
		glTexCoord2f(0.0f, 0.0f);
		glVertex2i(-wid, wid);
		glTexCoord2f(1.0f, 0.0f);
		glVertex2i( wid, wid);
		glTexCoord2f(1.0f, 1.0f);
		glVertex2i( wid,-wid);
		glEnd();
		glPopMatrix();
		glDisable(GL_ALPHA_TEST);
		glDisable(GL_TEXTURE_2D);
		glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_TEXTURE_2D);
	}
}
