/*
 CSCI 420 Computer Graphics
 Assignment 2: Roller Coasters
 Junmeng Xu
*/

#include <stdio.h>
#include <stdlib.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#include "pic.h"
#include <cmath>
#include <vector>
#include "jerror.h"

int g_iMenuId;

int g_vMousePos[2] = {0, 0};
int g_iLeftMouseButton = 0;    /* 1 if pressed, 0 if not */
int g_iMiddleMouseButton = 0;
int g_iRightMouseButton = 0;

typedef enum { ROTATE, TRANSLATE, SCALE } CONTROLSTATE;
CONTROLSTATE g_ControlState = ROTATE;

/* state of the world */
float g_vLandRotate[3] = {0.0, 0.0, 0.0};
float g_vLandTranslate[3] = {0.0, 0.0, 0.0};
float g_vLandScale[3] = {1.0, 1.0, 1.0};

/* set the window size */
int windowLength = 640;
int windowWidth = 480;

char * windowTitle = "Roller Coasters";

/* Catmull-Rom spline */
double s = 0.5f;
double basis[16] = { -s, 2.0f-s, s-2.0f, s,
                    2.0f*s, s-3.0f, 3.0f-2.0f*s, -s,
                    -s, 0.0f, s, 0.0f,
                    0.0f, 1.0f, 0.0f, 0.0f };

/* represents one control point along the spline */
struct point {
   double x;
   double y;
   double z;
};

/* represents one face along the rail */
struct face{
    point p0;
    point p1;
    point p2;
    point p3;
    point p4;
    point p5;
    point p6;
    point p7;
};

/* spline struct which contains how many control points, and an array of control points */
struct spline {
   int numControlPoints;
   struct point *points;
};

/* the spline array */
struct spline *g_Splines;

/* total number of splines */
int g_iNumOfSplines;

/* all points in splines */
std::vector<point> splinesPoints;

/* unit tangent vector in all points */
std::vector<point> splinesPointsTangent;

/* N and B vector in all points */
std::vector<point> splinesPointsN;
std::vector<point> splinesPointsB;

/* all faces for left and right rails, also T part of rail */
std::vector<face> railLeft;
std::vector<face> railLeftT;
std::vector<face> railRight;
std::vector<face> railRightT;

/* store picture data of input images */
Pic * g_groundPic, * g_skyPicFront, * g_skyPicTop, * g_skyPicBack, * g_skyPicLeft, * g_skyPicRight, * g_woodenPic;

/* texture handler */
GLuint g_groundTex, g_skyTexFront, g_skyTexTop, g_skyTexBack, g_skyTexLeft, g_skyTexRight, g_woodenTex;

/* Cosmic Cool Cloud Style */
Pic * g_cosmicPicBottom, * g_cosmicPicFront, * g_cosmicPicTop, * g_cosmicPicBack, * g_cosmicPicLeft, * g_cosmicPicRight;
/* texture handler */
GLuint g_cosmicTexBottom, g_cosmicTexFront, g_cosmicTexTop, g_cosmicTexBack, g_cosmicTexLeft, g_cosmicTexRight;

bool cosmic = false;

/* control whether the camera follows the roller coaster or it can move free */
bool followRollerCoaster = true;

/* camera index in splines points */
int camIndex = 0;

/* store the highest point height value for controling speed of camera */
double h_max = 0;

/* gravity constant */
double g = 9.8;

/* free camera vector */
point freeCamEyes, freeCamFocus, freeCamUp;

/* take screenshots */
int screenshotsIndex = 0;
bool screenshotsOn = false;

/* sky cube vertices */
GLfloat cubeVertices[8][3] =
    {{-1.0, -1.0, -1.0}, {1.0, -1.0, -1.0},
    {1.0, 1.0, -1.0}, {-1.0, 1.0, -1.0}, {-1.0, -1.0, 1.0},
    {1.0, -1.0, 1.0}, {1.0, 1.0, 1.0}, {-1.0, 1.0, 1.0}};

/* images struct which stores its width, height and other data */
struct imgRawImage {
    unsigned int numComponents;
    unsigned long int width, height;

    unsigned char* lpData;
};


/* load all splines from the input file */
int loadSplines(char *argv) {
  char *cName = (char *)malloc(128 * sizeof(char));
  FILE *fileList;
  FILE *fileSpline;
  int iType, i = 0, j, iLength;

  /* load the track file */
  fileList = fopen(argv, "r");
  if (fileList == NULL) {
    printf ("can't open file\n");
    exit(1);
  }
  
  /* stores the number of splines in a global variable */
  fscanf(fileList, "%d", &g_iNumOfSplines);

  g_Splines = (struct spline *)malloc(g_iNumOfSplines * sizeof(struct spline));

  /* reads through the spline files */
  for (j = 0; j < g_iNumOfSplines; j++) {
    i = 0;
    fscanf(fileList, "%s", cName);
    fileSpline = fopen(cName, "r");

    if (fileSpline == NULL) {
      printf ("can't open file\n");
      exit(1);
    }

    /* gets length for spline file */
    fscanf(fileSpline, "%d %d", &iLength, &iType);

    /* allocate memory for all the points */
    g_Splines[j].points = (struct point *)malloc(iLength * sizeof(struct point));
    g_Splines[j].numControlPoints = iLength;

    /* saves the data to the struct */
    while (fscanf(fileSpline, "%lf %lf %lf", 
	   &g_Splines[j].points[i].x, 
	   &g_Splines[j].points[i].y, 
	   &g_Splines[j].points[i].z) != EOF) {
       i++;
    }
  }

  free(cName);

  return 0;
}

/* Write a screenshot to the specified filename */
void saveScreenshot (char *filename)
{
    int i, j;
    Pic *in = NULL;

    if (filename == NULL)
    return;

    /* Allocate a picture buffer */
    in = pic_alloc(640, 480, 3, NULL);

    printf("File to save to: %s\n", filename);

    for (i=479; i>=0; i--) {
        glReadPixels(0, 479-i, 640, 1, GL_RGB, GL_UNSIGNED_BYTE,
                     &in->pix[i*in->nx*in->bpp]);
    }

    if (jpeg_write(filename, in))
        printf("File saved Successfully\n");
    else
        printf("Error in Saving\n");

    pic_free(in);
}

/*
 *  Define some basic functions for vector calculation
 *  cross product for vectors
 */
point vectorCross(point p1, point p2)
{
    point p = {0.0f, 0.0f, 0.0f};
    p.x = p1.y*p2.z - p1.z*p2.y;
    p.y = p1.z*p2.x - p1.x*p2.z;
    p.z = p1.x*p2.y - p1.y*p2.x;
    return p;
}

/* calculate the normal vector of input vector */
point vectorNormal(point p)
{
    float x = p.x;
    float y = p.y;
    float z = p.z;
    float len = sqrt(x*x + y*y + z*z);
    p.x = x/len;
    p.y = y/len;
    p.z = z/len;
    return p;
}

/* calculate the length of one vector */
float vectorDistance(point p)
{
    float x = p.x;
    float y = p.y;
    float z = p.z;
    float len = sqrt(x*x + y*y + z*z);
    return len;
}

/* use Catmull-Rom method to get splines */
void computeCatmullRom(float control[12])
{
    for(float u = 0.000f; u <= 1.000f; u += 0.001f){
        double u3 = pow(u, 3);
        double u2 = pow(u, 2);
        float UCrossBasis[4] = { basis[0]*u3 + basis[4]*u2 + basis[8]*u + basis[12],
                                 basis[1]*u3 + basis[5]*u2 + basis[9]*u + basis[13],
                                 basis[2]*u3 + basis[6]*u2 + basis[10]*u + basis[14],
                                 basis[3]*u3 + basis[7]*u2 + basis[11]*u + basis[15] };
        float UBCrossControl[3] = { control[0]*UCrossBasis[0] + control[3]*UCrossBasis[1] + control[6]*UCrossBasis[2] + control[9]*UCrossBasis[3],
                                    control[1]*UCrossBasis[0] + control[4]*UCrossBasis[1] + control[7]*UCrossBasis[2] + control[10]*UCrossBasis[3],
                                    control[2]*UCrossBasis[0] + control[5]*UCrossBasis[1] + control[8]*UCrossBasis[2] + control[11]*UCrossBasis[3]};

        point curPoint;
        curPoint.x = UBCrossControl[0];
        curPoint.y = UBCrossControl[1];
        curPoint.z = UBCrossControl[2];
        
        // update the height of the highest point in all splines
        if(h_max < curPoint.y)
        {
            h_max = curPoint.y;
        }
        
        splinesPoints.push_back(curPoint);
        // printf (" %f %f %f\n", UBCrossControl[0],UBCrossControl[1],UBCrossControl[2]);
        
        // calculate the tangent vector for each u
        double u3_t = 3 * u2;
        double u2_t = 2 * u;
        float UCrossBasis_t[4] = { basis[0]*u3_t + basis[4]*u2_t + basis[8],
                                 basis[1]*u3_t + basis[5]*u2_t + basis[9],
                                 basis[2]*u3_t + basis[6]*u2_t + basis[10],
                                 basis[3]*u3_t + basis[7]*u2_t + basis[11] };
        float UBCrossControl_t[3] = { control[0]*UCrossBasis_t[0] + control[3]*UCrossBasis_t[1] + control[6]*UCrossBasis_t[2] + control[9]*UCrossBasis_t[3],
                                    control[1]*UCrossBasis_t[0] + control[4]*UCrossBasis_t[1] + control[7]*UCrossBasis_t[2] + control[10]*UCrossBasis_t[3],
                                    control[2]*UCrossBasis_t[0] + control[5]*UCrossBasis_t[1] + control[8]*UCrossBasis_t[2] + control[11]*UCrossBasis_t[3]};

        point tangentPoint = {UBCrossControl_t[0], UBCrossControl_t[1], UBCrossControl_t[2]};
        tangentPoint = vectorNormal(tangentPoint);
        splinesPointsTangent.push_back(tangentPoint);
    }
}

/* initial splines */
void initSplines()
{
    int k = 0;
    for(int i=0; i<g_iNumOfSplines; i++)
    {
        for(int j=0; j<g_Splines[i].numControlPoints-3; j++)
        {
            float control[12] = {
                g_Splines[i].points[j].x, g_Splines[i].points[j].y, g_Splines[i].points[j].z,
                g_Splines[i].points[j+1].x, g_Splines[i].points[j+1].y, g_Splines[i].points[j+1].z,
                g_Splines[i].points[j+2].x, g_Splines[i].points[j+2].y, g_Splines[i].points[j+2].z,
                g_Splines[i].points[j+3].x, g_Splines[i].points[j+3].y, g_Splines[i].points[j+3].z
            };
            computeCatmullRom(control);
        }
        
        /* Initialization for rail cross-section, when k=0: T0, N0, B0
         * Pick an arbitrary V. N0 = unit(T0 x V) and B0 = unit(T0 x N0).
         * Others, N1 = unit(B0 x T1) and B1 = unit(T1 x N1)
         */
        point v = {0.0f, 1.0f, 0.5f};
        if(v.x == splinesPointsTangent[k].x && v.y == splinesPointsTangent[k].y && v.z == splinesPointsTangent[k].z){
            v.x = 1.0f;
            v.y = 0.0f;
            v.z = 0.0f;
        }
        point N = vectorNormal(vectorCross(splinesPointsTangent[k], v));
        splinesPointsN.push_back(N);
        point B = vectorNormal(vectorCross(splinesPointsTangent[k], N));
        splinesPointsB.push_back(B);
        k++;
        while(k < splinesPoints.size()){
            point N1 = vectorNormal(vectorCross(splinesPointsB[k-1], splinesPointsTangent[k]));
            splinesPointsN.push_back(N1);
            point B1 = vectorNormal(vectorCross(splinesPointsTangent[k], splinesPointsN[k]));
            splinesPointsB.push_back(B1);
            k++;
        }
        
    }
}

/* simply draw splines */
//void drawSplines()
//{
//    glLineWidth(20.0f);
//    glBegin(GL_LINES);
//
//    for(int i=0; i<splinesPoints.size()-1; i++)
//    {
//        glColor3f(1, 1, 1);
//        glVertex3f(splinesPoints[i].x, splinesPoints[i].y, splinesPoints[i].z);
//        glColor3f(1, 1, 1);
//        glVertex3f(splinesPoints[i+1].x, splinesPoints[i+1].y, splinesPoints[i+1].z);
//    }
//
//    glEnd();
//}

/* initial points and faces for left and right rail cross section, also T part */
void initRailCrossSection()
{
    for(int i=0; i<splinesPoints.size()-2; i++)
    {
        // Left side
        point cur = splinesPoints[i];
        point curN = splinesPointsN[i];
        point curB = splinesPointsB[i];
        curN.x *= 0.02f;
        curN.y *= 0.02f;
        curN.z *= 0.02f;
        curB.x *= 0.02f;
        curB.y *= 0.02f;
        curB.z *= 0.02f;
        cur.x -= curN.x*8;
        cur.y -= curN.y*8;
        cur.z -= curN.z*8;
        
        point next = splinesPoints[i+1];
        point nextN = splinesPointsN[i+1];
        point nextB = splinesPointsB[i+1];
        nextN.x *= 0.02f;
        nextN.y *= 0.02f;
        nextN.z *= 0.02f;
        nextB.x *= 0.02f;
        nextB.y *= 0.02f;
        nextB.z *= 0.02f;
        next.x -= nextN.x*8;
        next.y -= nextN.y*8;
        next.z -= nextN.z*8;
        
        face railVerticesLeft =
            {{next.x-nextN.x-nextB.x, next.y-nextN.y-nextB.y, next.z-nextN.z-nextB.z},
                {next.x-nextN.x+nextB.x, next.y-nextN.y+nextB.y, next.z-nextN.z+nextB.z},
                {next.x+nextN.x+nextB.x, next.y+nextN.y+nextB.y, next.z+nextN.z+nextB.z},
                {next.x+nextN.x-nextB.x, next.y+nextN.y-nextB.y, next.z+nextN.z-nextB.z},
                {cur.x-curN.x-curB.x, cur.y-curN.y-curB.y, cur.z-curN.z-curB.z},
                {cur.x-curN.x+curB.x, cur.y-curN.y+curB.y, cur.z-curN.z+curB.z},
                {cur.x+curN.x+curB.x, cur.y+curN.y+curB.y, cur.z+curN.z+curB.z},
                {cur.x+curN.x-curB.x, cur.y+curN.y-curB.y, cur.z+curN.z-curB.z}};
        
        railLeft.push_back(railVerticesLeft);
        
        float curNx = curN.x * 0.2f;
        float curNy = curN.y * 0.2f;
        float curNz = curN.z * 0.2f;
        float curBx = curB.x * 2.0f;
        float curBy = curB.y * 2.0f;
        float curBz = curB.z * 2.0f;
        
        float nextNx = nextN.x * 0.2f;
        float nextNy = nextN.y * 0.2f;
        float nextNz = nextN.z * 0.2f;
        float nextBx = nextB.x * 2.0f;
        float nextBy = nextB.y * 2.0f;
        float nextBz = nextB.z * 2.0f;
        
//        face railVerticesLeftT =
//            {{next.x-nextNx-nextBx, next.y-nextNy-nextBy, next.z-nextNz-nextBz},
//                {next.x-nextNx+nextBx, next.y-nextNy+nextBy, next.z-nextNz+nextBz},
//                {next.x+nextNx+nextBx, next.y+nextNy+nextBy, next.z+nextNz+nextBz},
//                {next.x+nextNx-nextBx, next.y+nextNy-nextBy, next.z+nextNz-nextBz},
//                {cur.x-curNx-curBx, cur.y-curNy-curBy, cur.z-curNz-curBz},
//                {cur.x-curNx+curBx, cur.y-curNy+curBy, cur.z-curNz+curBz},
//                {cur.x+curNx+curBx, cur.y+curNy+curBy, cur.z+curNz+curBz},
//                {cur.x+curNx-curBx, cur.y+curNy-curBy, cur.z+curNz-curBz}};
        
        face railVerticesLeftT =
            {{next.x-nextNx-nextBx, next.y-nextNy-nextBy, next.z-nextNz-nextBz},
                {next.x-nextNx, next.y-nextNy, next.z-nextNz},
                {next.x+nextNx, next.y+nextNy, next.z+nextNz},
                {next.x+nextNx-nextBx, next.y+nextNy-nextBy, next.z+nextNz-nextBz},
                {cur.x-curNx-curBx, cur.y-curNy-curBy, cur.z-curNz-curBz},
                {cur.x-curNx, cur.y-curNy, cur.z-curNz},
                {cur.x+curNx, cur.y+curNy, cur.z+curNz},
                {cur.x+curNx-curBx, cur.y+curNy-curBy, cur.z+curNz-curBz}};
        
        railLeftT.push_back(railVerticesLeftT);
        
        // Right side
        cur.x += curN.x*16;
        cur.y += curN.y*16;
        cur.z += curN.z*16;

        next.x += nextN.x*16;
        next.y += nextN.y*16;
        next.z += nextN.z*16;
        
        face railVerticesRight =
            {{next.x-nextN.x-nextB.x, next.y-nextN.y-nextB.y, next.z-nextN.z-nextB.z},
                {next.x-nextN.x+nextB.x, next.y-nextN.y+nextB.y, next.z-nextN.z+nextB.z},
                {next.x+nextN.x+nextB.x, next.y+nextN.y+nextB.y, next.z+nextN.z+nextB.z},
                {next.x+nextN.x-nextB.x, next.y+nextN.y-nextB.y, next.z+nextN.z-nextB.z},
                {cur.x-curN.x-curB.x, cur.y-curN.y-curB.y, cur.z-curN.z-curB.z},
                {cur.x-curN.x+curB.x, cur.y-curN.y+curB.y, cur.z-curN.z+curB.z},
                {cur.x+curN.x+curB.x, cur.y+curN.y+curB.y, cur.z+curN.z+curB.z},
                {cur.x+curN.x-curB.x, cur.y+curN.y-curB.y, cur.z+curN.z-curB.z}};

        railRight.push_back(railVerticesRight);
        
//        face railVerticesRightT =
//            {{next.x-nextNx-nextBx, next.y-nextNy-nextBy, next.z-nextNz-nextBz},
//                {next.x-nextNx+nextBx, next.y-nextNy+nextBy, next.z-nextNz+nextBz},
//                {next.x+nextNx+nextBx, next.y+nextNy+nextBy, next.z+nextNz+nextBz},
//                {next.x+nextNx-nextBx, next.y+nextNy-nextBy, next.z+nextNz-nextBz},
//                {cur.x-curNx-curBx, cur.y-curNy-curBy, cur.z-curNz-curBz},
//                {cur.x-curNx+curBx, cur.y-curNy+curBy, cur.z-curNz+curBz},
//                {cur.x+curNx+curBx, cur.y+curNy+curBy, cur.z+curNz+curBz},
//                {cur.x+curNx-curBx, cur.y+curNy-curBy, cur.z+curNz-curBz}};
        
        face railVerticesRightT =
            {{next.x-nextNx-nextBx, next.y-nextNy-nextBy, next.z-nextNz-nextBz},
                {next.x-nextNx, next.y-nextNy, next.z-nextNz},
                {next.x+nextNx, next.y+nextNy, next.z+nextNz},
                {next.x+nextNx-nextBx, next.y+nextNy-nextBy, next.z+nextNz-nextBz},
                {cur.x-curNx-curBx, cur.y-curNy-curBy, cur.z-curNz-curBz},
                {cur.x-curNx, cur.y-curNy, cur.z-curNz},
                {cur.x+curNx, cur.y+curNy, cur.z+curNz},
                {cur.x+curNx-curBx, cur.y+curNy-curBy, cur.z+curNz-curBz}};

        railRightT.push_back(railVerticesRightT);
    }
}

/*
    draw rail section and T part,
    also support structure
 
    *** Extrax ***
    Render a T-shaped rail cross-section
 
    *** Extrax ***
    Render double rail (like in real railroad tracks)
 
    *** Extrax ***
    Draw additional scene elements: a support structure that looks realistic
 */
void drawRailSection()
{
    // draw left rail
    glBegin(GL_TRIANGLE_STRIP);
    for(int i=0; i<railLeft.size(); i++){
        glColor3f(1, 0, 0);
        glVertex3f(railLeft[i].p7.x, railLeft[i].p7.y, railLeft[i].p7.z);

        glColor3f(1, 0, 0);
        glVertex3f(railLeft[i].p6.x, railLeft[i].p6.y, railLeft[i].p6.z);
    }
    glEnd();

    glBegin(GL_TRIANGLE_STRIP);
    for(int i=0; i<railLeft.size(); i++){
        glColor3f(1, 0, 0);
        glVertex3f(railLeft[i].p5.x, railLeft[i].p5.y, railLeft[i].p5.z);

        glColor3f(1, 0, 0);
        glVertex3f(railLeft[i].p4.x, railLeft[i].p4.y, railLeft[i].p4.z);
    }
    glEnd();
    
    glBegin(GL_TRIANGLE_STRIP);
    for(int i=0; i<railLeft.size(); i++){
        glColor3f(1, 0, 0);
        glVertex3f(railLeft[i].p5.x, railLeft[i].p5.y, railLeft[i].p5.z);
        
        glColor3f(1, 0, 0);
        glVertex3f(railLeft[i].p6.x, railLeft[i].p6.y, railLeft[i].p6.z);
    }
    glEnd();
    
    glBegin(GL_TRIANGLE_STRIP);
    for(int i=0; i<railLeft.size(); i++){
        glColor3f(1, 0, 0);
        glVertex3f(railLeft[i].p4.x, railLeft[i].p4.y, railLeft[i].p4.z);

        glColor3f(1, 0, 0);
        glVertex3f(railLeft[i].p7.x, railLeft[i].p7.y, railLeft[i].p7.z);
    }
    glEnd();
    
    // draw T part of left rail
    glBegin(GL_TRIANGLE_STRIP);
    for(int i=0; i<railLeft.size(); i++){
        glColor3f(0.5, 0, 0);
        glVertex3f(railLeftT[i].p7.x, railLeftT[i].p7.y, railLeftT[i].p7.z);

        glColor3f(0.5, 0, 0);
        glVertex3f(railLeftT[i].p6.x, railLeftT[i].p6.y, railLeftT[i].p6.z);
    }
    glEnd();

    glBegin(GL_TRIANGLE_STRIP);
    for(int i=0; i<railLeft.size(); i++){
        glColor3f(0.5, 0, 0);
        glVertex3f(railLeftT[i].p5.x, railLeftT[i].p5.y, railLeftT[i].p5.z);

        glColor3f(0.5, 0, 0);
        glVertex3f(railLeftT[i].p4.x, railLeftT[i].p4.y, railLeftT[i].p4.z);
    }
    glEnd();
    
    glBegin(GL_TRIANGLE_STRIP);
    for(int i=0; i<railLeft.size(); i++){
        glColor3f(0.5, 0, 0);
        glVertex3f(railLeftT[i].p5.x, railLeftT[i].p5.y, railLeftT[i].p5.z);
        
        glColor3f(0.5, 0, 0);
        glVertex3f(railLeftT[i].p6.x, railLeftT[i].p6.y, railLeftT[i].p6.z);
    }
    glEnd();
    
    glBegin(GL_TRIANGLE_STRIP);
    for(int i=0; i<railLeft.size(); i++){
        glColor3f(0.5, 0, 0);
        glVertex3f(railLeftT[i].p4.x, railLeftT[i].p4.y, railLeftT[i].p4.z);

        glColor3f(0.5, 0, 0);
        glVertex3f(railLeftT[i].p7.x, railLeftT[i].p7.y, railLeftT[i].p7.z);
    }
    glEnd();
    
    // draw right rail
    glBegin(GL_TRIANGLE_STRIP);
    for(int i=0; i<railLeft.size(); i++){
        glColor3f(1, 0, 0);
        glVertex3f(railRight[i].p7.x, railRight[i].p7.y, railRight[i].p7.z);

        glColor3f(1, 0, 0);
        glVertex3f(railRight[i].p6.x, railRight[i].p6.y, railRight[i].p6.z);
    }
    glEnd();

    glBegin(GL_TRIANGLE_STRIP);
    for(int i=0; i<railLeft.size(); i++){
        glColor3f(1, 0, 0);
        glVertex3f(railRight[i].p5.x, railRight[i].p5.y, railRight[i].p5.z);

        glColor3f(1, 0, 0);
        glVertex3f(railRight[i].p4.x, railRight[i].p4.y, railRight[i].p4.z);
    }
    glEnd();
    
    glBegin(GL_TRIANGLE_STRIP);
    for(int i=0; i<railLeft.size(); i++){
        glColor3f(1, 0, 0);
        glVertex3f(railRight[i].p5.x, railRight[i].p5.y, railRight[i].p5.z);
        
        glColor3f(1, 0, 0);
        glVertex3f(railRight[i].p6.x, railRight[i].p6.y, railRight[i].p6.z);
    }
    glEnd();
    
    glBegin(GL_TRIANGLE_STRIP);
    for(int i=0; i<railLeft.size(); i++){
        glColor3f(1, 0, 0);
        glVertex3f(railRight[i].p4.x, railRight[i].p4.y, railRight[i].p4.z);

        glColor3f(1, 0, 0);
        glVertex3f(railRight[i].p7.x, railRight[i].p7.y, railRight[i].p7.z);
    }
    glEnd();
    
    // draw T part of right rail
    glBegin(GL_TRIANGLE_STRIP);
    for(int i=0; i<railLeft.size(); i++){
        glColor3f(0.5, 0, 0);
        glVertex3f(railRightT[i].p7.x, railRightT[i].p7.y, railRightT[i].p7.z);

        glColor3f(0.5, 0, 0);
        glVertex3f(railRightT[i].p6.x, railRightT[i].p6.y, railRightT[i].p6.z);
    }
    glEnd();

    glBegin(GL_TRIANGLE_STRIP);
    for(int i=0; i<railLeft.size(); i++){
        glColor3f(0.5, 0, 0);
        glVertex3f(railRightT[i].p5.x, railRightT[i].p5.y, railRightT[i].p5.z);

        glColor3f(0.5, 0, 0);
        glVertex3f(railRightT[i].p4.x, railRightT[i].p4.y, railRightT[i].p4.z);
    }
    glEnd();
    
    glBegin(GL_TRIANGLE_STRIP);
    for(int i=0; i<railLeft.size(); i++){
        glColor3f(0.5, 0, 0);
        glVertex3f(railRightT[i].p5.x, railRightT[i].p5.y, railRightT[i].p5.z);
        
        glColor3f(0.5, 0, 0);
        glVertex3f(railRightT[i].p6.x, railRightT[i].p6.y, railRightT[i].p6.z);
    }
    glEnd();
    
    glBegin(GL_TRIANGLE_STRIP);
    for(int i=0; i<railLeft.size(); i++){
        glColor3f(0.5, 0, 0);
        glVertex3f(railRightT[i].p4.x, railRightT[i].p4.y, railRightT[i].p4.z);

        glColor3f(0.5, 0, 0);
        glVertex3f(railRightT[i].p7.x, railRightT[i].p7.y, railRightT[i].p7.z);
    }
    glEnd();
    
    // draw support structure
    
    // left support
    int step = railLeft.size()/80;
    for(int i=0; i<splinesPoints.size()-2; i+=step){
        if(splinesPointsB[i].y < 0)
        {
            glBegin(GL_POLYGON);
            glColor3f(0.7, 0.7, 0.7);
            glVertex3f(railLeft[i].p5.x, railLeft[i].p5.y, railLeft[i].p5.z);

            glColor3f(0.7, 0.7, 0.7);
            glVertex3f(railLeft[i].p6.x, railLeft[i].p6.y, railLeft[i].p6.z);

            glColor3f(0.7, 0.7, 0.7);
            glVertex3f(railLeft[i].p6.x, railLeft[i].p6.y-60, railLeft[i].p6.z);

            glColor3f(0.7, 0.7, 0.7);
            glVertex3f(railLeft[i].p5.x, railLeft[i].p5.y-60, railLeft[i].p5.z);
            glEnd();
            
            glBegin(GL_POLYGON);
            glColor3f(0.7, 0.7, 0.7);
            glVertex3f(railLeft[i].p4.x, railLeft[i].p4.y, railLeft[i].p4.z);

            glColor3f(0.7, 0.7, 0.7);
            glVertex3f(railLeft[i].p5.x, railLeft[i].p5.y, railLeft[i].p5.z);

            glColor3f(0.7, 0.7, 0.7);
            glVertex3f(railLeft[i].p5.x, railLeft[i].p5.y-60, railLeft[i].p5.z);

            glColor3f(0.7, 0.7, 0.7);
            glVertex3f(railLeft[i].p4.x, railLeft[i].p4.y-60, railLeft[i].p4.z);
            glEnd();

            glBegin(GL_POLYGON);
            glColor3f(0.7, 0.7, 0.7);
            glVertex3f(railLeft[i].p3.x, railLeft[i].p3.y, railLeft[i].p3.z);

            glColor3f(0.7, 0.7, 0.7);
            glVertex3f(railLeft[i].p6.x, railLeft[i].p6.y, railLeft[i].p6.z);

            glColor3f(0.7, 0.7, 0.7);
            glVertex3f(railLeft[i].p6.x, railLeft[i].p6.y-60, railLeft[i].p6.z);

            glColor3f(0.7, 0.7, 0.7);
            glVertex3f(railLeft[i].p3.x, railLeft[i].p3.y-60, railLeft[i].p3.z);
            glEnd();

            glBegin(GL_POLYGON);
            glColor3f(0.7, 0.7, 0.7);
            glVertex3f(railLeft[i].p4.x, railLeft[i].p4.y, railLeft[i].p4.z);

            glColor3f(0.7, 0.7, 0.7);
            glVertex3f(railLeft[i].p3.x, railLeft[i].p3.y, railLeft[i].p3.z);

            glColor3f(0.7, 0.7, 0.7);
            glVertex3f(railLeft[i].p4.x, railLeft[i].p4.y-60, railLeft[i].p4.z);

            glColor3f(0.7, 0.7, 0.7);
            glVertex3f(railLeft[i].p3.x, railLeft[i].p3.y-60, railLeft[i].p3.z);
            glEnd();
        }
    }
    
    // right support
    for(int i=0; i<splinesPoints.size()-2; i+=step){
        if(splinesPointsB[i].y < 0)
        {
            glBegin(GL_POLYGON);
            glColor3f(0.7, 0.7, 0.7);
            glVertex3f(railRight[i].p5.x, railRight[i].p5.y, railRight[i].p5.z);

            glColor3f(0.7, 0.7, 0.7);
            glVertex3f(railRight[i].p6.x, railRight[i].p6.y, railRight[i].p6.z);

            glColor3f(0.7, 0.7, 0.7);
            glVertex3f(railRight[i].p6.x, railRight[i].p6.y-60, railRight[i].p6.z);

            glColor3f(0.7, 0.7, 0.7);
            glVertex3f(railRight[i].p5.x, railRight[i].p5.y-60, railRight[i].p5.z);
            glEnd();
            
            glBegin(GL_POLYGON);
            glColor3f(0.7, 0.7, 0.7);
            glVertex3f(railRight[i].p4.x, railRight[i].p4.y, railRight[i].p4.z);

            glColor3f(0.7, 0.7, 0.7);
            glVertex3f(railRight[i].p5.x, railRight[i].p5.y, railRight[i].p5.z);

            glColor3f(0.7, 0.7, 0.7);
            glVertex3f(railRight[i].p5.x, railRight[i].p5.y-60, railRight[i].p5.z);

            glColor3f(0.7, 0.7, 0.7);
            glVertex3f(railRight[i].p4.x, railRight[i].p4.y-60, railRight[i].p4.z);
            glEnd();

            glBegin(GL_POLYGON);
            glColor3f(0.7, 0.7, 0.7);
            glVertex3f(railRight[i].p3.x, railRight[i].p3.y, railRight[i].p3.z);

            glColor3f(0.7, 0.7, 0.7);
            glVertex3f(railRight[i].p6.x, railRight[i].p6.y, railRight[i].p6.z);

            glColor3f(0.7, 0.7, 0.7);
            glVertex3f(railRight[i].p6.x, railRight[i].p6.y-60, railRight[i].p6.z);

            glColor3f(0.7, 0.7, 0.7);
            glVertex3f(railRight[i].p3.x, railRight[i].p3.y-60, railRight[i].p3.z);
            glEnd();

            glBegin(GL_POLYGON);
            glColor3f(0.7, 0.7, 0.7);
            glVertex3f(railRight[i].p4.x, railRight[i].p4.y, railRight[i].p4.z);

            glColor3f(0.7, 0.7, 0.7);
            glVertex3f(railRight[i].p3.x, railRight[i].p3.y, railRight[i].p3.z);

            glColor3f(0.7, 0.7, 0.7);
            glVertex3f(railRight[i].p4.x, railRight[i].p4.y-60, railRight[i].p4.z);

            glColor3f(0.7, 0.7, 0.7);
            glVertex3f(railRight[i].p3.x, railRight[i].p3.y-60, railRight[i].p3.z);
            glEnd();
        }
    }
}

/*
    draw wooden crossbars
 
    *** Extrax ***
    Draw additional scene elements: texture-mapped wooden crossbars
 */
void drawRailSectionCrossBar(){
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    glBindTexture(GL_TEXTURE_2D, g_woodenTex);
    
    int step = railLeft.size()/160;
    for(int i=0; i<railLeft.size(); i+=step){
        
        glBegin(GL_TRIANGLES);
        
        // draw horizontal plane
        glTexCoord2f(0.0,0.0);
        glVertex3f(railRight[i].p5.x, railRight[i].p5.y, railRight[i].p5.z);

        glTexCoord2f(0.0,1.0);
        glVertex3f(railRight[i].p4.x, railRight[i].p4.y, railRight[i].p4.z);

        glTexCoord2f(1.0,1.0);
        glVertex3f(railLeft[i].p7.x, railLeft[i].p7.y, railLeft[i].p7.z);

        glTexCoord2f(0.0,0.0);
        glVertex3f(railRight[i].p5.x, railRight[i].p5.y, railRight[i].p5.z);

        glTexCoord2f(1.0,1.0);
        glVertex3f(railLeft[i].p7.x, railLeft[i].p7.y, railLeft[i].p7.z);

        glTexCoord2f(1.0,0.0);
        glVertex3f(railLeft[i].p6.x, railLeft[i].p6.y, railLeft[i].p6.z);
        
        // draw vertical plane
        glTexCoord2f(0.0,0.0);
        glVertex3f(railRight[i+20].p0.x, railRight[i+20].p0.y, railRight[i+20].p0.z);

        glTexCoord2f(0.0,1.0);
        glVertex3f(railRight[i].p4.x, railRight[i].p4.y, railRight[i].p4.z);

        glTexCoord2f(1.0,1.0);
        glVertex3f(railLeft[i].p7.x, railLeft[i].p7.y, railLeft[i].p7.z);

        glTexCoord2f(0.0,0.0);
        glVertex3f(railRight[i+20].p0.x, railRight[i+20].p0.y, railRight[i+20].p0.z);

        glTexCoord2f(1.0,1.0);
        glVertex3f(railLeft[i].p7.x, railLeft[i].p7.y, railLeft[i].p7.z);

        glTexCoord2f(1.0,0.0);
        glVertex3f(railLeft[i+20].p3.x, railLeft[i+20].p3.y, railLeft[i+20].p3.z);
        
        // draw vertical plane
        glTexCoord2f(0.0,0.0);
        glVertex3f(railRight[i+20].p1.x, railRight[i+20].p1.y, railRight[i+20].p1.z);

        glTexCoord2f(0.0,1.0);
        glVertex3f(railRight[i+20].p0.x, railRight[i+20].p0.y, railRight[i+20].p0.z);

        glTexCoord2f(1.0,1.0);
        glVertex3f(railLeft[i+20].p3.x, railLeft[i+20].p3.y, railLeft[i+20].p3.z);

        glTexCoord2f(0.0,0.0);
        glVertex3f(railRight[i+20].p1.x, railRight[i+20].p1.y, railRight[i+20].p1.z);

        glTexCoord2f(1.0,1.0);
        glVertex3f(railLeft[i+20].p3.x, railLeft[i+20].p3.y, railLeft[i+20].p3.z);

        glTexCoord2f(1.0,0.0);
        glVertex3f(railLeft[i+20].p2.x, railLeft[i+20].p2.y, railLeft[i+20].p2.z);
        
        // draw horizontal plane
        glTexCoord2f(0.0,0.0);
        glVertex3f(railRight[i].p5.x, railRight[i].p5.y, railRight[i].p5.z);

        glTexCoord2f(0.0,1.0);
        glVertex3f(railRight[i+20].p1.x, railRight[i+20].p1.y, railRight[i+20].p1.z);

        glTexCoord2f(1.0,1.0);
        glVertex3f(railLeft[i+20].p2.x, railLeft[i+20].p2.y, railLeft[i+20].p2.z);

        glTexCoord2f(0.0,0.0);
        glVertex3f(railRight[i].p5.x, railRight[i].p5.y, railRight[i].p5.z);

        glTexCoord2f(1.0,1.0);
        glVertex3f(railLeft[i+20].p2.x, railLeft[i+20].p2.y, railLeft[i+20].p2.z);

        glTexCoord2f(1.0,0.0);
        glVertex3f(railLeft[i].p6.x, railLeft[i].p6.y, railLeft[i].p6.z);
        
        glEnd();
    }
     
}


/*  initial textures
    use Pic library to load and initial images
    and bind texture
 
     *** Extrax ***
     Use Pic.h to load images
 */
GLuint initTexture(Pic * g_Pic, int gTextureWrapType) {
    int maxX = g_Pic->nx;
    int maxY = g_Pic->ny;
    
    GLuint textureHandler;
    
    unsigned char * pixelsRGBA = new unsigned char[4 * maxX *  maxY];
    memset(pixelsRGBA, 0, 4 * maxX * maxY);

    for(int i=0;i<maxY;i++){
        for(int j=0;j<maxX;j++){
            pixelsRGBA[4 * (i * maxX + j) + 0] = 0; // R
            pixelsRGBA[4 * (i * maxX + j) + 1] = 0; // G
            pixelsRGBA[4 * (i * maxX + j) + 2] = 0; // B
            pixelsRGBA[4 * (i * maxX + j) + 3] = 255; // Alpha

            // initial the RGBA channels
            int nChannels = g_Pic->bpp;
            for (int c = 0; c < nChannels; c++){
                pixelsRGBA[4 * (i * maxX + j) + c] = PIC_PIXEL(g_Pic, j, i, c);
            }
        }
    }
    
    glGenTextures(1, &textureHandler); // must declare a global variable in program header: GLUint texName
    glBindTexture(GL_TEXTURE_2D, textureHandler); // make texture “texName” the currently active texture
    
    switch( gTextureWrapType )
           {
               case 0:
                   // specify texture parameters (they affect whatever texture is active)
                   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                   // repeat pattern in s
                   glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                   break;
               case 1:
                   // clamp pattern
                   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
                   glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );
                   break;
           }
    
    // use linear fi
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // load image data stored at pointer “pointerToImage” into the currently active texture (“texName”)
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, maxX, maxY, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelsRGBA);
    
    glGenerateMipmap(GL_TEXTURE_2D);
    
    return textureHandler;
    
}

/* draw the ground */
void drawGround()
{
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
    
    // draw normal style
    if(!cosmic){
        glBindTexture(GL_TEXTURE_2D, g_groundTex);
        
        glBegin(GL_TRIANGLES);
       
        glTexCoord2f(1.0,0.0);
        glVertex3f(-101.0f, 25.0f, -101.0f);

        glTexCoord2f(1.0,1.0);
        glVertex3f(-101.0f, 25.0f, 101.0f);

        glTexCoord2f(0.0,1.0);
        glVertex3f(101.0f, 25.0f, 101.0f);

        glTexCoord2f(1.0,0.0);
        glVertex3f(-101.0f, 25.0f, -101.0f);

        glTexCoord2f(0.0,1.0);
        glVertex3f(101.0f, 25.0f, 101.0f);

        glTexCoord2f(0.0,0.0);
        glVertex3f(101.0f, 25.0f, -101.0f);
        
        glEnd();
        
    }else{
        // draw cosmic style
        glBindTexture(GL_TEXTURE_2D, g_cosmicTexBottom);
        
        glBegin(GL_TRIANGLES);
       
        glTexCoord2f(1.0,0.0);
        glVertex3f(-201.0f, -100.0f, -201.0f);

        glTexCoord2f(1.0,1.0);
        glVertex3f(-201.0f, -100.0f, 201.0f);

        glTexCoord2f(0.0,1.0);
        glVertex3f(201.0f, -100.0f, 201.0f);

        glTexCoord2f(1.0,0.0);
        glVertex3f(-201.0f, -100.0f, -201.0f);

        glTexCoord2f(0.0,1.0);
        glVertex3f(201.0f, -100.0f, 201.0f);

        glTexCoord2f(0.0,0.0);
        glVertex3f(201.0f, -100.0f, -201.0f);
        
        glEnd();
    }
    
}

/* basic function for sky cube */
void cubeFace(int a, int b, int c, int d, GLuint g_skyTex)
{
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    glBindTexture(GL_TEXTURE_2D, g_skyTex);
    
    glBegin(GL_POLYGON);
    
    glTexCoord2f(0.0,0.0);
    glVertex3fv(cubeVertices[a]);
    glTexCoord2f(0.0,1.0);
    glVertex3fv(cubeVertices[b]);
    glTexCoord2f(1.0,1.0);
    glVertex3fv(cubeVertices[c]);
    glTexCoord2f(1.0,0.0);
    glVertex3fv(cubeVertices[d]);
    
    glEnd();
}

/* draw sky */
void drawSky()
{
    if(!cosmic){
        // draw normal style
        glTranslatef(0, 50, 0);
        glScalef(100, 100, 100);
        cubeFace(2,1,0,3,g_skyTexFront);
        cubeFace(6,2,3,7,g_skyTexTop);
        cubeFace(3,0,4,7,g_skyTexLeft);
        cubeFace(6,5,1,2,g_skyTexRight);
        cubeFace(7,4,5,6,g_skyTexBack);
    }else{
        // draw cosmic style
        glTranslatef(0, 100, 0);
        glScalef(200, 200, 200);
        cubeFace(2,1,0,3,g_cosmicTexFront);
        cubeFace(6,2,3,7,g_cosmicTexTop);
        cubeFace(3,0,4,7,g_cosmicTexLeft);
        cubeFace(6,5,1,2,g_cosmicTexRight);
        cubeFace(7,4,5,6,g_cosmicTexBack);
    }
}

void myinit()
{
    /* initial splines and rails */
    initSplines();
    initRailCrossSection();
    
    /* setup gl view here */
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glShadeModel(GL_SMOOTH); /*enable smooth shading */
    
    // initial camera index
    camIndex = splinesPoints.size()-1;
    
    /*
      *** Extrax ***
      Modify the velocity with which your camera moves to make it physically realistic in terms of gravity
     */
    // initial
    freeCamEyes = splinesPoints[camIndex];
    freeCamEyes.x -= splinesPointsB[camIndex].x * 0.5f;
    freeCamEyes.y -= splinesPointsB[camIndex].y * 0.5f;
    freeCamEyes.z -= splinesPointsB[camIndex].z * 0.5f;
    freeCamFocus.x = freeCamEyes.x - splinesPointsTangent[camIndex].x;
    freeCamFocus.y = freeCamEyes.y - splinesPointsTangent[camIndex].y;
    freeCamFocus.z = freeCamEyes.z - splinesPointsTangent[camIndex].z;
    freeCamUp = splinesPointsB[camIndex];
    
    // let the height max value be more than the real ten
    h_max+=10;
    
    // initial textures
    g_groundTex = initTexture(g_groundPic,1);
    g_skyTexTop = initTexture(g_skyPicTop, 1);
    g_skyTexFront = initTexture(g_skyPicFront,1);
    g_skyTexBack = initTexture(g_skyPicBack, 1);
    g_skyTexLeft = initTexture(g_skyPicLeft, 1);
    g_skyTexRight = initTexture(g_skyPicRight, 1);
    g_woodenTex = initTexture(g_woodenPic,0);
    
    g_cosmicTexTop = initTexture(g_cosmicPicTop, 1);
    g_cosmicTexBottom = initTexture(g_cosmicPicBottom, 1);
    g_cosmicTexFront = initTexture(g_cosmicPicFront, 1);
    g_cosmicTexBack = initTexture(g_cosmicPicBack, 1);
    g_cosmicTexLeft = initTexture(g_cosmicPicLeft, 1);
    g_cosmicTexRight = initTexture(g_cosmicPicRight, 1);
    
    // enable depth buffering
    glEnable(GL_DEPTH_TEST);
    
    // interpolate colors during rasterization
    glShadeModel(GL_SMOOTH);
}

void menufunc(int value)
{
    switch (value)
    {
        case 0:
            exit(0);
            break;
    }
}

void display()
{
    // clear buffers, use depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode (GL_MODELVIEW);
    // reset transformation
    glLoadIdentity();
    
    // reset camera index
    if(camIndex < 0){
        camIndex = splinesPoints.size()-1;
    }

    /*
      *** Extrax ***
      camera can follow the roller coaster, or it can move free
     */
    // define eyes, focus and up vector
    if(followRollerCoaster){
        point eyes = splinesPoints[camIndex];
        eyes.x -= splinesPointsB[camIndex].x * 0.5f;
        eyes.y -= splinesPointsB[camIndex].y * 0.5f;
        eyes.z -= splinesPointsB[camIndex].z * 0.5f;
        point focus = eyes;
        focus.x = eyes.x - splinesPointsTangent[camIndex].x;
        focus.y = eyes.y - splinesPointsTangent[camIndex].y;
        focus.z = eyes.z - splinesPointsTangent[camIndex].z;
        point up = splinesPointsB[camIndex];
        gluLookAt(eyes.x, eyes.y, eyes.z, focus.x, focus.y, focus.z, -up.x, -up.y, -up.z);
    }else{
        glTranslatef(g_vLandTranslate[0], g_vLandTranslate[1], g_vLandTranslate[2]);
        glScalef(g_vLandScale[0], g_vLandScale[1], g_vLandScale[2]);
        glRotatef(g_vLandRotate[0], 1, 0, 0);
        glRotatef(g_vLandRotate[1], 0, 1, 0);
        glRotatef(g_vLandRotate[2], 0, 0, 1);
        gluLookAt(freeCamEyes.x, freeCamEyes.y, freeCamEyes.z, freeCamFocus.x, freeCamFocus.y, freeCamFocus.z, -freeCamUp.x, -freeCamUp.y, -freeCamUp.z);
    }
     
    drawRailSection();
    
    // draw objects with textures
    glEnable( GL_TEXTURE_2D );
    
    drawRailSectionCrossBar();
    drawGround();
    drawSky();
    
    glDisable(GL_TEXTURE_2D);
    
    // double buffer flush
    glutSwapBuffers();
}

void reshape(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f, 1.0 * w/h, 0.01f, 1000.0f);
    glMatrixMode(GL_MODELVIEW);
}

void doIdle()
{
    /* do some stuff... */
    
    /* control the speed of camera */
    if(camIndex > 0)
    {
        /*
          *** Extrax ***
          Modify the velocity with which your camera moves to make it physically realistic in terms of gravity. Please see [Supplementary:Camera_Volecity.pdf] on how to update u for every time step.
         
          *** Extrax ***
          Derive the steps that lead to the physically realistic equation of updating the u (i.e. u_new = u_old + (dt)(sqrt(2gh)/mag(dp/du)), see [Supplementary:Camera_Volecity.pdf]).
          Please see Extra_derive_equation.pdf
         */
        if(followRollerCoaster){
            camIndex -= sqrt(2*g*(h_max - splinesPoints[camIndex].y)) / vectorDistance(splinesPointsTangent[camIndex]);
        }
    }else{
        camIndex = splinesPoints.size()-1;
    }
    
    /*
       save screenshots automatically
    */
    if((screenshotsIndex >= 0 && screenshotsIndex < 500) && screenshotsOn){
        char filenameIndex[32];
        sprintf(filenameIndex, "screenshots/anim.%03d.jpg", screenshotsIndex);
        saveScreenshot(filenameIndex);
        screenshotsIndex++;
    }
    if(screenshotsIndex >= 500){
        screenshotsIndex = 0;
        screenshotsOn = false;
    }

    /* make the screen update */
    glutPostRedisplay();
}

void initPic()
{
    // initial normal images
    g_groundPic = jpeg_read("images/ground.jpg", NULL);
    g_skyPicFront = jpeg_read("images/MegaSunFront.jpg", NULL);
    g_skyPicBack = jpeg_read("images/MegaSunBack.jpg", NULL);
    g_skyPicLeft = jpeg_read("images/MegaSunLeft.jpg", NULL);
    g_skyPicRight = jpeg_read("images/MegaSunRight.jpg", NULL);
    g_skyPicTop = jpeg_read("images/MegaSunTop.jpg", NULL);
    g_woodenPic = jpeg_read("images/wooden.jpg", NULL);
    
    /*
      initial cosmic cool cloud images
      *** Extras ***
      use two style to render skybox
     */
    g_cosmicPicBottom = jpeg_read("images/CosmicCoolCloudBottom.jpg", NULL);
    g_cosmicPicTop = jpeg_read("images/CosmicCoolCloudTop.jpg", NULL);
    g_cosmicPicLeft = jpeg_read("images/CosmicCoolCloudLeft.jpg", NULL);
    g_cosmicPicRight = jpeg_read("images/CosmicCoolCloudRight.jpg", NULL);
    g_cosmicPicFront = jpeg_read("images/CosmicCoolCloudFront.jpg", NULL);
    g_cosmicPicBack = jpeg_read("images/CosmicCoolCloudBack.jpg", NULL);
}

void mousedrag(int x, int y)
{
    if(!followRollerCoaster)
    {
     
        int vMouseDelta[2] = {x-g_vMousePos[0], y-g_vMousePos[1]};

        switch (g_ControlState)
        {
            case TRANSLATE:
                if (g_iLeftMouseButton)
                {
                    g_vLandTranslate[0] += vMouseDelta[0]*0.01;
                    g_vLandTranslate[1] -= vMouseDelta[1]*0.01;
                }
                if (g_iMiddleMouseButton)
                {
                    g_vLandTranslate[2] += vMouseDelta[1]*0.01;
                }
                break;
            case ROTATE:
                if (g_iLeftMouseButton)
                {
                    g_vLandRotate[0] += vMouseDelta[1];
                    g_vLandRotate[1] += vMouseDelta[0];
                }
                if (g_iMiddleMouseButton)
                {
                    g_vLandRotate[2] += vMouseDelta[1];
                }
                break;
            case SCALE:
                if (g_iLeftMouseButton)
                {
                    g_vLandScale[0] *= 1.0+vMouseDelta[0]*0.01;
                    g_vLandScale[1] *= 1.0-vMouseDelta[1]*0.01;
                }
                if (g_iMiddleMouseButton)
                {
                    g_vLandScale[2] *= 1.0-vMouseDelta[1]*0.01;
                }
                break;
        }
        g_vMousePos[0] = x;
        g_vMousePos[1] = y;
        
    }
}

void mouseidle(int x, int y)
{
    g_vMousePos[0] = x;
    g_vMousePos[1] = y;
}

void mousebutton(int button, int state, int x, int y)
{
    switch (button)
    {
    case GLUT_LEFT_BUTTON:
      g_iLeftMouseButton = (state==GLUT_DOWN);
      break;
    case GLUT_MIDDLE_BUTTON:
      g_iMiddleMouseButton = (state==GLUT_DOWN);
      break;
    case GLUT_RIGHT_BUTTON:
      g_iRightMouseButton = (state==GLUT_DOWN);
      break;
    }

    g_vMousePos[0] = x;
    g_vMousePos[1] = y;
}

void keybutton(unsigned char key, int x, int y)
{
    switch (key){
        // 'z', 'x', 'c' keys are used to control: translate, scale, or rotate graphics.
        case 'z':
            g_ControlState = TRANSLATE;
            break;

        case 'x':
            g_ControlState = SCALE;
            break;

        case 'c':
            g_ControlState = ROTATE;
            break;
            
        case 'f':
            followRollerCoaster = !followRollerCoaster;
            break;
            
        case 'd':
            cosmic = !cosmic;
            break;
            
        case 's':
            screenshotsOn = true;
            break;
        
    default:
         break;
    }
}

int main (int argc, char ** argv)
{
  if (argc<2)
  {  
  printf ("usage: %s <trackfile>\n", argv[0]);
  exit(0);
  }

    /*
        *** Extrax ***
        Create tracks that mimic real world roller coasters such as [Magic Mountain] (close to Los Angeles)
        Create a new ride splines called junmengRide.sp
     */
    loadSplines(argv[1]);

    initPic();

    glutInit(&argc,argv);

    /* set window size */
    glutInitWindowSize(windowLength, windowWidth);
    /* set window position */
    glutInitWindowPosition(0, 0);
    /* create a window */
    glutCreateWindow(windowTitle);
    
    /* tells glut to use a particular display function to redraw */
    glutDisplayFunc(display);

    /* allow the user to quit using the right mouse button menu */
    g_iMenuId = glutCreateMenu(menufunc);
    glutSetMenu(g_iMenuId);
    glutAddMenuEntry("Quit",0);
    glutAttachMenu(GLUT_RIGHT_BUTTON);

    /* replace with any animate code */
    glutIdleFunc(doIdle);

    /* callback for mouse drags */
    glutMotionFunc(mousedrag);
    /* callback for idle mouse movement */
    glutPassiveMotionFunc(mouseidle);
    /* callback for mouse button changes */
    glutMouseFunc(mousebutton);
    /* Due to issues of keyboard mapping on MAC, I use glutKeyboardFunc for g_ControlState switch */
    glutKeyboardFunc(keybutton);
    /* callback for window resize */
    glutReshapeFunc(reshape);
    
    /* do initialization */
    myinit();

    glutMainLoop();

  return 0;
}
