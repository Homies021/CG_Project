// ============================================================
//  Pirate Ship Adventure — OpenGL / GLUT
//  Features: Rocks, Sharks, 3 Lives, High Score
// ============================================================
#include <GL/glut.h>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <vector>
#include <string>

const int WIN_W = 800, WIN_H = 600;
const float BOAT_SPEED = 5.0f, ROCK_SPEED_BASE = 2.0f;

struct Obstacle { float x, y, w, h, speed; bool isShark; };
struct { float x, y, w=90, h=45; bool up,down,left,right; } boat;

std::vector<Obstacle> obstacles;
int  level=1, score=0, highScore=0, rocksToPass=5, rocksPassed=0, lives=3;
bool gameOver=false, levelClear=false, gameStarted=false, enteringName=false;
int  invTimer=0;
std::string highScoreName="---", nameInput="";

void spawnObstacle() {
    Obstacle o;
    o.isShark = (rand()%4==0) && level>=2;
    o.w = o.isShark ? 50 : 40+rand()%30;
    o.h = o.isShark ? 30 : 40+rand()%30;
    o.x = WIN_W+o.w;
    o.y = 60+rand()%(WIN_H-120-(int)o.h);
    o.speed = ROCK_SPEED_BASE+(level-1)*0.8f+(float)(rand()%10)/10.0f;
    if(o.isShark) o.speed+=1.0f;
    obstacles.push_back(o);
}

bool aabb(float ax,float ay,float aw,float ah,float bx,float by,float bw,float bh){
    return ax<bx+bw && ax+aw>bx && ay<by+bh && ay+ah>by;
}

void resetLevel(){
    obstacles.clear(); rocksPassed=0; levelClear=false; invTimer=0;
    boat.x=80; boat.y=WIN_H/2.0f-boat.h/2.0f;
}

void startGame(){
    level=1; score=0; lives=3; rocksToPass=5;
    gameOver=false; levelClear=false; gameStarted=true; enteringName=false;
    srand((unsigned)time(nullptr)); resetLevel();
}

// ---------- draw helpers ----------
void drawRect(float x,float y,float w,float h,float r,float g,float b){
    glColor3f(r,g,b); glBegin(GL_QUADS);
    glVertex2f(x,y); glVertex2f(x+w,y); glVertex2f(x+w,y+h); glVertex2f(x,y+h);
    glEnd();
}

void drawCircle(float cx,float cy,float rad,float r,float g,float b,int s=20){
    glColor3f(r,g,b); glBegin(GL_TRIANGLE_FAN); glVertex2f(cx,cy);
    for(int i=0;i<=s;i++){ float a=2*3.14159f*i/s; glVertex2f(cx+rad*cosf(a),cy+rad*sinf(a)); }
    glEnd();
}

void drawText(float x,float y,const std::string& s,float r=1,float g=1,float b=1){
    glColor3f(r,g,b); glRasterPos2f(x,y);
    for(char c:s) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18,c);
}

void drawTextLarge(float x,float y,const std::string& s,float r=1,float g=1,float b=1){
    glColor3f(r,g,b); glRasterPos2f(x,y);
    for(char c:s) glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24,c);
}

// ---------- draw pirate ship ----------
void drawBoat(){
    float x=boat.x, y=boat.y, W=90.0f, H=110.0f;
    if(invTimer>0 && (invTimer/6)%2==0) return; // blink on hit

    // hull
    glColor3f(0.35f,0.18f,0.05f); glBegin(GL_POLYGON);
    glVertex2f(x,y+H*.18f); glVertex2f(x+W,y+H*.10f);
    glVertex2f(x+W+10,y+H*.22f); glVertex2f(x+W,y+H*.40f);
    glVertex2f(x,y+H*.42f); glEnd();

    // wood stripe
    glColor3f(0.55f,0.30f,0.10f); glBegin(GL_QUADS);
    glVertex2f(x,y+H*.30f); glVertex2f(x+W,y+H*.24f);
    glVertex2f(x+W,y+H*.30f); glVertex2f(x,y+H*.36f); glEnd();

    // cannon holes
    for(int i=0;i<3;i++)
        drawRect(x+W*(0.18f+i*0.22f),y+H*.20f,W*.08f,H*.07f,0.1f,0.05f,0);

    // stern castle
    glColor3f(0.40f,0.20f,0.06f); glBegin(GL_QUADS);
    glVertex2f(x,y+H*.42f); glVertex2f(x+W*.28f,y+H*.42f);
    glVertex2f(x+W*.28f,y+H*.58f); glVertex2f(x,y+H*.58f); glEnd();

    // masts
    glColor3f(0.30f,0.15f,0.04f); glLineWidth(4.0f);
    glBegin(GL_LINES); glVertex2f(x+W*.18f,y+H*.42f); glVertex2f(x+W*.18f,y+H*1.05f); glEnd();
    glBegin(GL_LINES); glVertex2f(x+W*.62f,y+H*.40f); glVertex2f(x+W*.62f,y+H*1.10f); glEnd();
    glLineWidth(3.0f);
    glBegin(GL_LINES); glVertex2f(x+W*.80f,y+H*.42f); glVertex2f(x+W*1.18f,y+H*.62f); glEnd();

    // rear sail + shadow
    glColor3f(0.92f,0.88f,0.75f); glBegin(GL_POLYGON);
    glVertex2f(x+W*.18f,y+H*1.04f); glVertex2f(x+W*.44f,y+H*1.00f);
    glVertex2f(x+W*.50f,y+H*.78f);  glVertex2f(x+W*.44f,y+H*.58f);
    glVertex2f(x+W*.18f,y+H*.58f);  glEnd();
    glColor3f(0.75f,0.70f,0.58f); glBegin(GL_POLYGON);
    glVertex2f(x+W*.18f,y+H*1.04f); glVertex2f(x+W*.24f,y+H*1.02f);
    glVertex2f(x+W*.28f,y+H*.72f);  glVertex2f(x+W*.18f,y+H*.58f); glEnd();

    // front sail + shadow
    glColor3f(0.90f,0.86f,0.72f); glBegin(GL_POLYGON);
    glVertex2f(x+W*.62f,y+H*1.08f); glVertex2f(x+W*.92f,y+H*1.02f);
    glVertex2f(x+W*.98f,y+H*.78f);  glVertex2f(x+W*.90f,y+H*.56f);
    glVertex2f(x+W*.62f,y+H*.56f);  glEnd();
    glColor3f(0.72f,0.68f,0.56f); glBegin(GL_POLYGON);
    glVertex2f(x+W*.62f,y+H*1.08f); glVertex2f(x+W*.68f,y+H*1.05f);
    glVertex2f(x+W*.70f,y+H*.68f);  glVertex2f(x+W*.62f,y+H*.56f); glEnd();

    // pirate flag
    drawRect(x+W*.18f-1,y+H*1.04f,2,H*.10f,0.25f,0.12f,0.03f);
    glColor3f(0.05f,0.05f,0.05f); glBegin(GL_QUADS);
    glVertex2f(x+W*.19f,y+H*1.14f); glVertex2f(x+W*.42f,y+H*1.14f);
    glVertex2f(x+W*.42f,y+H*1.06f); glVertex2f(x+W*.19f,y+H*1.06f); glEnd();
    drawCircle(x+W*.30f,y+H*1.10f,4,0.9f,0.9f,0.9f,8);
}

// ---------- draw shark ----------
void drawShark(const Obstacle& o){
    float cy=o.y+o.h/2;
    glColor3f(0.3f,0.35f,0.4f); glBegin(GL_POLYGON);
    glVertex2f(o.x,cy); glVertex2f(o.x+o.w*.4f,o.y+o.h*.15f);
    glVertex2f(o.x+o.w,cy+o.h*.1f); glVertex2f(o.x+o.w*.4f,o.y+o.h*.85f); glEnd();
    // dorsal fin
    glColor3f(0.25f,0.30f,0.35f); glBegin(GL_TRIANGLES);
    glVertex2f(o.x+o.w*.4f,cy+o.h*.3f);
    glVertex2f(o.x+o.w*.5f,cy+o.h*.85f);
    glVertex2f(o.x+o.w*.6f,cy+o.h*.3f); glEnd();
    // eye
    drawCircle(o.x+o.w*.75f,cy+o.h*.05f,3,0.9f,0.9f,0.9f,8);
    // teeth
    glColor3f(0.95f,0.95f,0.95f); glBegin(GL_TRIANGLES);
    for(int i=0;i<3;i++){
        float tx=o.x+o.w*.82f+i*6;
        glVertex2f(tx,cy+o.h*.12f); glVertex2f(tx+3,cy-o.h*.05f); glVertex2f(tx+6,cy+o.h*.12f);
    }
    glEnd();
}

// ---------- draw rock ----------
void drawRock(const Obstacle& o){
    float cx=o.x+o.w/2, cy=o.y+o.h/2, rx=o.w/2, ry=o.h/2;
    glColor3f(0.45f,0.42f,0.40f); glBegin(GL_POLYGON);
    float p[8][2]={{1,.1f},{.7f,.8f},{0,1.1f},{-.8f,.85f},{-1.1f,0},{-.75f,-.9f},{0,-1.05f},{.9f,-.8f}};
    for(auto& v:p) glVertex2f(cx+v[0]*rx,cy+v[1]*ry); glEnd();
    glColor3f(0.62f,0.60f,0.58f); glBegin(GL_POLYGON);
    glVertex2f(cx-rx*.2f,cy+ry*.5f); glVertex2f(cx+rx*.3f,cy+ry*.6f);
    glVertex2f(cx+rx*.1f,cy+ry*.2f); glEnd();
}

// ---------- background ----------
void drawBackground(){
    drawRect(0,0,WIN_W,WIN_H,0.53f,0.81f,0.98f);
    drawRect(0,40,WIN_W,WIN_H-80,0.10f,0.45f,0.75f);
    drawRect(0,0,WIN_W,40,0.25f,0.65f,0.25f);
    drawRect(0,WIN_H-40,WIN_W,40,0.25f,0.65f,0.25f);
    drawCircle(720,540,40,1.0f,0.92f,0.2f);
    glColor3f(0.15f,0.55f,0.85f); glLineWidth(1.5f);
    for(int row=0;row<5;row++){
        float wy=100+row*90;
        glBegin(GL_LINE_STRIP);
        for(int i=0;i<=40;i++){ float wx=i*20.0f; glVertex2f(wx,wy+4*sinf(wx*.05f)); }
        glEnd();
    }
}

// ---------- HUD with hearts ----------
void drawHUD(){
    drawText(10, WIN_H-22, "Level:"+std::to_string(level), 1,1,0);
    drawText(150,WIN_H-22, "Score:"+std::to_string(score), 1,1,0);
    drawText(320,WIN_H-22, "Best:"+std::to_string(highScore)+" ("+highScoreName+")", 0,1,1);
    drawText(490,WIN_H-22, "Pass:"+std::to_string(rocksPassed)+"/"+std::to_string(rocksToPass), 1,1,0);
    for(int i=0;i<3;i++){
        float hx=700+i*28, hy=WIN_H-22;
        float r=(i<lives)?1.0f:0.3f, gb=(i<lives)?0.1f:0.3f;
        drawCircle(hx,  hy+5,6,r,gb,gb);
        drawCircle(hx+9,hy+5,6,r,gb,gb);
        glColor3f(r,gb,gb); glBegin(GL_TRIANGLES);
        glVertex2f(hx-5,hy+3); glVertex2f(hx+14,hy+3); glVertex2f(hx+4.5f,hy-7);
        glEnd();
    }
}

// ---------- screens ----------
void drawStartScreen(){
    drawRect(0,0,WIN_W,WIN_H,0.05f,0.15f,0.30f);
    drawTextLarge(230,385,"PIRATE SHIP ADVENTURE",0.2f,0.9f,1.0f);
    drawText(225,330,"Dodge rocks & sharks — survive the sea!",1,1,0.5f);
    drawText(300,285,"Arrow Keys : Steer Ship",1,1,1);
    drawText(230,255,"3 Lives  |  Sharks appear from Level 2  |  Beat your Best!",1,0.6f,0.6f);
    drawText(335,215,"Press ENTER to Start",0.3f,1.0f,0.3f);
}

void drawNameEntryScreen(){
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0,0,0,0.75f); glBegin(GL_QUADS);
    glVertex2f(145,195); glVertex2f(655,195); glVertex2f(655,430); glVertex2f(145,430); glEnd();
    glDisable(GL_BLEND);
    drawTextLarge(255,400,"NEW HIGH SCORE!",1.0f,0.9f,0.1f);
    drawText(310,355,"Score: "+std::to_string(highScore),1,1,0);
    drawText(245,315,"Enter your name and press ENTER:",0.8f,0.8f,0.8f);
    // name input box
    drawRect(230,255,340,38,0.15f,0.15f,0.25f);
    drawRect(230,255,340,38,0.4f,0.4f,0.7f); // border trick — draw outline
    glColor3f(0.4f,0.4f,0.7f); glLineWidth(2.0f); glBegin(GL_LINE_LOOP);
    glVertex2f(230,255); glVertex2f(570,255); glVertex2f(570,293); glVertex2f(230,293); glEnd();
    drawText(240,270, nameInput+"_", 1,1,1);
    drawText(258,220,"Max 12 characters  |  BACKSPACE to delete",0.6f,0.6f,0.6f);
}

void drawGameOverScreen(){
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0,0,0,0.65f); glBegin(GL_QUADS);
    glVertex2f(145,195); glVertex2f(655,195); glVertex2f(655,415); glVertex2f(145,415); glEnd();
    glDisable(GL_BLEND);
    drawTextLarge(315,372,"GAME OVER",1.0f,0.2f,0.2f);
    drawText(245,318,"Score: "+std::to_string(score)+"     Best: "+std::to_string(highScore),1,1,0);
    drawText(270,278,"High Score Holder: "+highScoreName,0.2f,1.0f,0.8f);
    drawText(295,238,"Press ENTER to Restart",0.8f,0.8f,0.8f);
}

void drawLevelClearScreen(){
    glEnable(GL_BLEND); glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glColor4f(0,0,0,0.60f); glBegin(GL_QUADS);
    glVertex2f(165,225); glVertex2f(635,225); glVertex2f(635,395); glVertex2f(165,395); glEnd();
    glDisable(GL_BLEND);
    drawTextLarge(258,362,"LEVEL "+std::to_string(level-1)+" CLEAR!",0.2f,1.0f,0.3f);
    drawText(268,308,"Press ENTER for Next Level",1,1,0.5f);
}

// ---------- GLUT callbacks ----------
void display(){
    glClear(GL_COLOR_BUFFER_BIT);
    if(!gameStarted){ drawStartScreen(); glutSwapBuffers(); return; }
    drawBackground();
    for(auto& o:obstacles) o.isShark ? drawShark(o) : drawRock(o);
    drawBoat(); drawHUD();
    if(gameOver){
        if(enteringName) drawNameEntryScreen();
        else             drawGameOverScreen();
    }
    if(levelClear) drawLevelClearScreen();
    glutSwapBuffers();
}

void update(int){
    glutTimerFunc(16,update,0);
    if(!gameStarted||gameOver||levelClear) return;
    if(invTimer>0) invTimer--;

    if(boat.up    && boat.y+boat.h<WIN_H-40) boat.y+=BOAT_SPEED;
    if(boat.down  && boat.y>40)              boat.y-=BOAT_SPEED;
    if(boat.right && boat.x+boat.w<WIN_W-10) boat.x+=BOAT_SPEED;
    if(boat.left  && boat.x>10)              boat.x-=BOAT_SPEED;

    if((int)obstacles.size()<3+level) spawnObstacle();

    for(int i=(int)obstacles.size()-1;i>=0;i--){
        obstacles[i].x-=obstacles[i].speed;
        if(obstacles[i].x+obstacles[i].w<boat.x &&
           obstacles[i].x+obstacles[i].w>boat.x-obstacles[i].speed-1){
            rocksPassed++; score+=10*level;
        }
        if(obstacles[i].x+obstacles[i].w<0) obstacles.erase(obstacles.begin()+i);
    }

    if(invTimer==0){
        for(auto& o:obstacles){
            if(aabb(boat.x+4,boat.y+4,boat.w-8,boat.h-8,o.x,o.y,o.w,o.h)){
                lives--;
                if(lives<=0){
                    gameOver=true;
                    if(score>highScore){ highScore=score; enteringName=true; nameInput=""; }
                    glutPostRedisplay(); return;
                }
                invTimer=90; // ~1.5s invincibility after hit
                break;
            }
        }
    }

    if(rocksPassed>=rocksToPass){
        levelClear=true; level++; rocksToPass=5+level*2; score+=50*(level-1);
        if(score>highScore) highScore=score;
    }
    glutPostRedisplay();
}

void keyDown(int k,int,int){
    if(k==GLUT_KEY_UP)    boat.up=true;
    if(k==GLUT_KEY_DOWN)  boat.down=true;
    if(k==GLUT_KEY_LEFT)  boat.left=true;
    if(k==GLUT_KEY_RIGHT) boat.right=true;
}
void keyUp(int k,int,int){
    if(k==GLUT_KEY_UP)    boat.up=false;
    if(k==GLUT_KEY_DOWN)  boat.down=false;
    if(k==GLUT_KEY_LEFT)  boat.left=false;
    if(k==GLUT_KEY_RIGHT) boat.right=false;
}
void keyboard(unsigned char k,int,int){
    if(enteringName){
        if(k==13){ // ENTER — confirm name
            highScoreName = nameInput.empty() ? "Unknown" : nameInput;
            enteringName=false; glutPostRedisplay();
        } else if(k==8){ // BACKSPACE
            if(!nameInput.empty()) nameInput.pop_back();
            glutPostRedisplay();
        } else if(nameInput.size()<12 && k>=32 && k<=126){
            nameInput+=k; glutPostRedisplay();
        }
        return;
    }
    if(k==13){ if(!gameStarted||gameOver) startGame(); else if(levelClear) resetLevel(); }
    if(k==27) exit(0);
}

int main(int argc,char** argv){
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_DOUBLE|GLUT_RGB);
    glutInitWindowSize(WIN_W,WIN_H);
    glutCreateWindow("Pirate Ship Adventure - OpenGL");
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    gluOrtho2D(0,WIN_W,0,WIN_H);
    glMatrixMode(GL_MODELVIEW);
    glutDisplayFunc(display); glutSpecialFunc(keyDown);
    glutSpecialUpFunc(keyUp); glutKeyboardFunc(keyboard);
    glutTimerFunc(16,update,0);
    glutMainLoop(); return 0;
}