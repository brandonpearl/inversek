
#include <string.h>
#include <fstream>
#include <cmath>

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/time.h>
#endif

#ifdef OSX
#include <GLUT/glut.h>
#include <OpenGL/glu.h>
#else
#include <GL/glut.h>
#include <GL/glu.h>
#endif

#include <time.h>
#include <math.h>
#include <iostream>
#include <algorithm>
#include <iostream>
#include <iterator>
#include <ostream>
#include <numeric>
#include <cstdlib>
#include <vector>
#include <cmath>
#include <string.h>

#include "inversek_util.h"
#ifndef _Eigen_System
#define _Eigen_System
#include "Eigen/SVD"
#endif

#define PI 3.14159265  // Should be used from mathlib
inline float sqr(float x) { return x*x; }
inline float min(float x, float y) {if (x < y) { return x;} else {return y;}}



using namespace std;

//****************************************************
// Some Classes
//****************************************************
class Viewport;

class Viewport {
    public:
        int w, h; // width and height
};


//****************************************************
// Global Variables
//****************************************************
Viewport    viewport;
float numCurves = 0;
int curFrame = 0;
double ustep = .01;
vector<Bezier> curves;
vector<Scene*> frames;

//****************************************************
// Simple init function
//****************************************************
void initScene(){
    GLfloat mat_specular[] = { 0.1, 0.1, 0.1, 1.0 };
    GLfloat mat_shininess[] = { 350 };
    GLfloat mat_amb_diff[] = { 0.1, 0.5, 0.8, 1.0 };
    glClearColor (0.0, 0.0, 0.0, 0.0);
    glShadeModel (GL_FLAT);

    glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, mat_amb_diff);


    GLfloat light_ambient[] = { 0.5, 0.5, 0.5, 1.0 };
    GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };
    GLfloat light_position[] = { 0.0, 0.0, 1.0, 0.0 };

    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
    glLightfv(GL_LIGHT0, GL_POSITION, light_position);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);


}


//****************************************************
// reshape viewport if the window is resized
//****************************************************
void myReshape(int w, int h) {
    viewport.w = w;
    viewport.h = h;

    glViewport (0,0,viewport.w,viewport.h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0f, ((float)w)/((float)h), .1f, 1500.0f);
}

//****************************************************
// function that does the actual drawing of stuff
//***************************************************
void myDisplay() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);               // clear the color buffer

    glMatrixMode(GL_MODELVIEW);                 // indicate we are specifying camera transformations
    glLoadIdentity();
    gluLookAt(0, 0, 10, 0, 0, 0, 0, 1, 0);
    
    // PUT GLBEGINS AND GLENDS HERE.
    glLineWidth(1.5); 
    glColor3f(1.0, 0.0, 0.0);
    glBegin(GL_LINES);

    Arm curArm = *(frames[curFrame]->rootArm);
    matrix rootT = matrix(curArm.rotation[0], curArm.rotation[1], curArm.rotation[2], 2);
    matrix* temp = new matrix(curArm.length, 0, 0, 0);
    rootT.multiplym(*temp);
    delete temp;
    Arm a2 = *(curArm.getNext());
    matrix a2T = matrix(a2.rotation[0], a2.rotation[1], a2.rotation[2], 2);
    matrix* temp1 = new matrix(a2.length, 0, 0, 0);
    a2T.multiplym(*temp1);
    a2T.multiplym(rootT);
    delete temp1;
    Arm a3 = *(a2.getNext());
    matrix a3T = matrix(a3.rotation[0], a3.rotation[1], a3.rotation[2], 2);
    matrix* temp2 = new matrix(a2.length, 0, 0, 0);
    a3T.multiplym(*temp2);
    a3T.multiplym(a2T);
    delete temp2;
    Arm a4 = *(a3.getNext());
    matrix a4T = matrix(a4.rotation[0], a4.rotation[1], a4.rotation[2], 2);
    matrix* temp3 = new matrix(a4.length, 0, 0, 0);
    a4T.multiplym(*temp3);
    a4T.multiplym(a3T);
    delete temp3;

    Vector4* start = new Vector4(0, 0, 0, 1);
    Vector4 p1 = rootT.multiplyv(*start);
    Vector4 p2 = a2T.multiplyv(*start);
    Vector4 p3 = a3T.multiplyv(*start);
    Vector4 p4 = a4T.multiplyv(*start);
    delete start;

    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(p1.xc(), p1.yc(), p1.zc());
    
    glEnd();
    glBegin(GL_LINES);

    glVertex3f(p1.xc(), p1.yc(), p1.zc()); 
    glVertex3f(p2.xc(), p2.yc(), p2.zc());
    
    glEnd();
    glBegin(GL_LINES);

    glVertex3f(p2.xc(), p2.yc(), p2.zc());
    glVertex3f(p3.xc(), p3.yc(), p3.zc());

    glEnd();
    glBegin(GL_LINES);

    glVertex3f(p3.xc(), p3.yc(), p3.zc());
	glVertex3f(p4.xc(), p4.yc(), p4.zc());

    glEnd();
         
    glFlush();
    glutSwapBuffers();                  // swap buffers (we earlier set double buffer)
    curFrame = (curFrame + 1) % frames.size();
}

vector<double> getEndPoint(double u_val){
    double u = u_val;
    bool assigned = false;
    Bezier curve;
    for(int i = 0; i < curves.size(); i++){
        if(u <= 1){
            assigned = true;
            curve = curves[i];
            break;
        }
        u -=1;
    }
    if(assigned){
        return curve.getPoint(u);
    }
    vector<double> rv;
    rv.push_back(INFINITY);
    rv.push_back(INFINITY);
    rv.push_back(INFINITY);
    return rv;

}

void generateFrames() {
	int steps = (int)(1/ustep);
	for (int curve=0; curve<curves.size(); curve++) {
		for (int i=0; i<steps; i++) {
			if (frames.size()==0) {
				
			}
		}
	}
}


//****************************************************
// the usual stuff, nothing exciting here
//****************************************************
int main(int argc, char *argv[]) {
    
     /*
     * INSERT PARSER HERE
     */
    const int MAX_CHARS_PER_LINE = 512;
    const int MAX_TOKENS_PER_LINE = 17;
    const char* const DELIMITER = " ";

    string readFile;
    if (argc < 2) {
        cout << "No input file specified." << endl;
        exit(0);
    }
    string arg1 = string(argv[1]);
    string arg2 = string(argv[2]);
    if(strlen(arg1.c_str()) >= 4){
        string last4 = arg1.substr(strlen(arg1.c_str())-4,string::npos);
        if(last4 == ".bez"){
            readFile = arg1;
        }
        else{
            cout << "input file not in .bez format" << endl;
            exit(0);
        }
    }
    else {
        cout << "Unrecognized argument. Please review usage." << endl;
        exit(0);
    }
    //start parsing readFile
    ifstream myFile;
    myFile.open(readFile);
    if(readFile == ""){
        cout << "No input provided. Please review usage." << endl;
        exit(0);
    }
    else{
        int lineNumber= 1;
        vector<int> patchNum; // when == 4, parse current set of patches into surfaces
        patchNum.push_back(0);
        double patchOne[4][3];
        cout << "Parsing BEZ file" << endl;
        std::string str;
        string to_add = " ";
        std::vector<std::string> vec, result_vec;
        while (std::getline(myFile, str)) {
            vec.push_back(str);
        }
        for(std::vector<string>::iterator it = vec.begin(); it != vec.end(); ++it) {
            string s = *it;
            s.insert(s.length(),1, to_add[0]);
            s.erase(std::remove(s.begin(), s.end(), '\r'), s.end());
            result_vec.push_back(s);
        }
        std::ofstream out_file(readFile);
        std::copy(result_vec.begin(), result_vec.end(), std::ostream_iterator<std::string>(out_file, "\n"));
        out_file.clear();
        out_file.close();
        
        myFile.clear();
        myFile.seekg(0, ios::beg);
        
        while (!myFile.eof()){
            char buf[MAX_CHARS_PER_LINE];
            myFile.getline(buf, MAX_CHARS_PER_LINE);
            const char* token[MAX_TOKENS_PER_LINE] = {}; 
            token[0] = strtok(buf, DELIMITER); // first token

            if (token[0]){
                int length = 0;
                for (int n = 1; n < MAX_TOKENS_PER_LINE; n++) {
                    token[n] = strtok(0,DELIMITER);
                    length +=1;
                    if (!token[n]){
                        break;
                    }
                }

                string first = string(token[0]).c_str();
                if(lineNumber == 1){
                    numCurves = atof(string(token[0]).c_str());
                }
                else{
                    double totalPatch[4][3];
                    totalPatch[0][0] = atof(string(token[0]).c_str());
                    totalPatch[0][1] = atof(string(token[1]).c_str());
                    totalPatch[0][2] = atof(string(token[2]).c_str());

                    totalPatch[1][0] = atof(string(token[3]).c_str());
                    totalPatch[1][1] = atof(string(token[4]).c_str());
                    totalPatch[1][2] = atof(string(token[5]).c_str());

                    totalPatch[2][0] = atof(string(token[6]).c_str());
                    totalPatch[2][1] = atof(string(token[7]).c_str());
                    totalPatch[2][2] = atof(string(token[8]).c_str());

                    totalPatch[3][0] = atof(string(token[9]).c_str());
                    totalPatch[3][1] = atof(string(token[10]).c_str());
                    totalPatch[3][2] = atof(string(token[11]).c_str());
                    //if(patchNum[0] == 0){
                    for(int i = 0; i < 4; i ++){
                        for (int j = 0; j < 3; j++){
                            patchOne[i][j] =  totalPatch[i][j];
                        }
                    }
                    Bezier curBez = Bezier(patchOne);
                    curves.push_back(curBez);
                }
                lineNumber+=1;
            } // end of if(token[0])
        } // end of while(!myFile.eof())
        
    } //end of parsing

    

       //This initializes glut
    glutInit(&argc, argv);
    //This tells glut to use a double-buffered window with red, green, and blue channels 

    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

    // Initalize theviewport size
    viewport.w = 400;
    viewport.h = 400;

    //The size and position of the window
    glutInitWindowSize(viewport.w, viewport.h);
    glutInitWindowPosition(0,0);
    glutCreateWindow(argv[0]);

    initScene();                            // quick function to set up scene

    glutDisplayFunc(myDisplay);             // function to run when its time to draw something
    glutReshapeFunc(myReshape);             // function to run when the window gets resized
    //glutKeyboardFunc(myKey);
    //glutSpecialFunc(specialKey);
    glEnable(GL_DEPTH_TEST | GL_LIGHTING);
    glDepthFunc(GL_LEQUAL);
    glutMainLoop();                         // infinite loop that will keep drawing and resizing


    return 0;
}












