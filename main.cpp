#ifdef _WIN32
#include <windows.h>
#endif

#define GLEW_STATIC 1
#include <GL/glew.h>
#include <stdio.h>
#include <GL/glut.h>
#include <GL/glui.h>
#include <iostream>
#include "GPGPU.h"

static int winWidth = 256;//512;
static int winHeight = 256;//512;
static int winId = -1;
static GLUI *glui;

extern float view_rotate[];	//external view_rotate[] in GPGPU.cpp
extern float trans_z[];		//external trans_z[] in GPGPU.cpp

GPGPU  *gpgpu;

//GUI stuff
GLUI_Rollout *simulator_rollout;
GLUI_RadioGroup	*simulator_radio; //simulator radio buttons
int simulator_type;	// 0=smoke, 1=fire, 2=fire_smoke
extern int _material;

GLUI_Rollout *rotate_trans_rollout;
GLUI_Rotation *object_rotation;

GLUI_Rollout *parameter_rollout;
GLUI_Spinner *TemperatureDissipation_spinner;
GLUI_Spinner *VelocityDissipation_spinner;
GLUI_Spinner *DensityDissipation_spinner;
GLUI_Spinner *TimeStep_spinner;
GLUI_Spinner *NumJacobiIterations_spinner;
GLUI_Spinner *Buoyancy_spinner;
GLUI_Spinner *Weight_spinner;
GLUI_Spinner *forceRadius_spinner;
GLUI_Spinner *forceTemperature_spinner;
GLUI_Spinner *forceDensity_spinner;
extern float TemperatureDissipation;
extern float VelocityDissipation;
extern float DensityDissipation;
extern float dt;
extern int jacobiInter;
extern float buoyancy;
extern float weight;
extern float forceRadius;
extern float forceTemperature;
extern float forceDensity;



void reset_button_pressed(int control)
{
	object_rotation->reset();
}

void simulation_start_button_pressed(int control)
{

	if(simulator_type == 0) { //smoke
		gpgpu->restart();
		TemperatureDissipation_spinner->set_float_val(0.9);
		VelocityDissipation_spinner->set_float_val(0.99);
		DensityDissipation_spinner->set_float_val(0.99);
		TimeStep_spinner->set_float_val(0.4);
		NumJacobiIterations_spinner->set_float_val(5);
		Buoyancy_spinner->set_float_val(1.0);
		Weight_spinner->set_float_val(0.3);
		forceRadius_spinner->set_float_val(3.0);
		forceTemperature_spinner->set_float_val(10.0);
		forceDensity_spinner->set_float_val(1.0);
		gpgpu->setMaterial(0);
	} else if (simulator_type == 1) {	//fire
		gpgpu->restart();
		TemperatureDissipation_spinner->set_float_val(0.9);
		VelocityDissipation_spinner->set_float_val(0.95);
		DensityDissipation_spinner->set_float_val(0.9);
		TimeStep_spinner->set_float_val(0.25);
		NumJacobiIterations_spinner->set_float_val(20);
		Buoyancy_spinner->set_float_val(2.0);
		Weight_spinner->set_float_val(1.5);
		forceRadius_spinner->set_float_val(2.0);
		forceTemperature_spinner->set_float_val(20.0);
		forceDensity_spinner->set_float_val(1.0);
		gpgpu->setMaterial(1);
	} else if (simulator_type == 2) {	//fire+smoke
		gpgpu->restart();
		TemperatureDissipation_spinner->set_float_val(0.9);
		VelocityDissipation_spinner->set_float_val(0.99);
		DensityDissipation_spinner->set_float_val(0.94);
		TimeStep_spinner->set_float_val(0.25);
		NumJacobiIterations_spinner->set_float_val(30);
		Buoyancy_spinner->set_float_val(2.0);
		Weight_spinner->set_float_val(0.3);
		forceRadius_spinner->set_float_val(3.0);
		forceTemperature_spinner->set_float_val(30.0);
		forceDensity_spinner->set_float_val(1.0);
		gpgpu->setMaterial(2);
	}
}
/*****************************************************************************
*****************************************************************************/
void
reshapeCB(int width, int height)
{
	 if (height == 0) height = 1;
    
    glViewport(0, 0, width, height);
    
	// Use orthographic projection
    glMatrixMode(GL_PROJECTION);    
    glLoadIdentity();               
    gluOrtho2D(-1, 1, -1, 1);       
    glMatrixMode(GL_MODELVIEW);     
    glLoadIdentity(); 

	gpgpu->setWindowSize(width, height);
}

/*****************************************************************************
*****************************************************************************/
void
idleFunc()
{		
	if ( glutGetWindow() != winId )
		glutSetWindow(winId);
	glutPostRedisplay();
}

/*****************************************************************************
*****************************************************************************/
void
refreshCB()
{
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	gpgpu->update(); //hand everything to GPU

    glutSwapBuffers();
	glutPostRedisplay();
}

/*****************************************************************************
*****************************************************************************/
void initialize()
{
    // Initialize glew library
    glewInit();

    // Create the gpgpu object
    gpgpu = new GPGPU();
    gpgpu->init(winWidth, winHeight);
}


void MakeGUI()
{
	glui = GLUI_Master.create_glui("GUI", 0, 0, 0);
	glui->add_statictext( "Fire & Smoke Simulation" );
	
	//Rotation and translation
	rotate_trans_rollout = glui->add_rollout("Rotation & Translation");
	object_rotation = glui->add_rotation_to_panel(rotate_trans_rollout, "Rotation", view_rotate);
	glui->add_button_to_panel(rotate_trans_rollout, "Reset Rotation", 0, reset_button_pressed);
	glui->add_translation_to_panel(rotate_trans_rollout, "Zoom in/out", GLUI_TRANSLATION_Z, trans_z);

	//simulator options
	simulator_rollout = glui->add_rollout("Simulator");
	simulator_radio = glui->add_radiogroup_to_panel(simulator_rollout, &simulator_type);
	glui->add_radiobutton_to_group(simulator_radio, "Smoke simulation");
	glui->add_radiobutton_to_group(simulator_radio, "Fire simulation");
	glui->add_radiobutton_to_group(simulator_radio, "Smoke and Fire simulation");
	glui->add_button_to_panel(simulator_rollout, "Start", 0, simulation_start_button_pressed);


	//parameter options
	parameter_rollout = glui->add_rollout("Parameters");

	TemperatureDissipation_spinner = glui->add_spinner_to_panel(parameter_rollout, "Temperature Dissipation", GLUI_SPINNER_FLOAT, &TemperatureDissipation);
	TemperatureDissipation_spinner->set_float_limits(0.0, 1.0);
	TemperatureDissipation_spinner->set_speed(0.05);
	TemperatureDissipation_spinner->set_float_val(0.9);


	VelocityDissipation_spinner = glui->add_spinner_to_panel(parameter_rollout, "Velocity Dissipation", GLUI_SPINNER_FLOAT, &VelocityDissipation);
	VelocityDissipation_spinner->set_float_limits(0.0, 1.0);
	VelocityDissipation_spinner->set_speed(0.05);
	VelocityDissipation_spinner->set_float_val(0.99);

	DensityDissipation_spinner = glui->add_spinner_to_panel(parameter_rollout, "Density Dissipation", GLUI_SPINNER_FLOAT, &DensityDissipation);
	DensityDissipation_spinner->set_float_limits(0.0, 1.0);
	DensityDissipation_spinner->set_speed(0.05);
	DensityDissipation_spinner->set_float_val(0.92);

	TimeStep_spinner = glui->add_spinner_to_panel(parameter_rollout, "Time Step", GLUI_SPINNER_FLOAT, &dt);
	TimeStep_spinner->set_float_limits(0.0, 1.0);
	TimeStep_spinner->set_speed(0.1);
	TimeStep_spinner->set_float_val(0.25);
	
	NumJacobiIterations_spinner = glui->add_spinner_to_panel(parameter_rollout, "Jacobi Iterations", GLUI_SPINNER_INT, &jacobiInter);
	NumJacobiIterations_spinner->set_int_limits(1, 100);
	NumJacobiIterations_spinner->set_speed(0.5);
	NumJacobiIterations_spinner->set_float_val(10);

	Buoyancy_spinner = glui->add_spinner_to_panel(parameter_rollout, "Buoyancy", GLUI_SPINNER_FLOAT, &buoyancy);
	Buoyancy_spinner->set_float_limits(0.0, 10.0);
	Buoyancy_spinner->set_speed(0.1);
	Buoyancy_spinner->set_float_val(2.0);

	Weight_spinner = glui->add_spinner_to_panel(parameter_rollout, "Weight", GLUI_SPINNER_FLOAT, &weight);
	Weight_spinner->set_float_limits(0.0, 5.0);
	Weight_spinner->set_speed(0.5);
	Weight_spinner->set_float_val(1.0);

	forceRadius_spinner = glui->add_spinner_to_panel(parameter_rollout, "Force Radius", GLUI_SPINNER_FLOAT, &forceRadius);
	forceRadius_spinner->set_float_limits(0.0, 10.0);
	forceRadius_spinner->set_speed(0.1);
	forceRadius_spinner->set_float_val(2.0);

	forceTemperature_spinner = glui->add_spinner_to_panel(parameter_rollout, "Force Temperature", GLUI_SPINNER_FLOAT, &forceTemperature);
	forceTemperature_spinner->set_float_limits(0.0, 100.0);
	forceTemperature_spinner->set_speed(0.1);
	forceTemperature_spinner->set_float_val(20.0);

	forceDensity_spinner = glui->add_spinner_to_panel(parameter_rollout, "Force Density", GLUI_SPINNER_FLOAT, &forceDensity);
	forceDensity_spinner->set_float_limits(0.0, 10.0);
	forceDensity_spinner->set_speed(0.1);
	forceDensity_spinner->set_float_val(1.0);

	glui->set_main_gfx_window(winId); 

	/* We register the idle callback with GLUI, *not* with GLUT */
	GLUI_Master.set_glutIdleFunc(idleFunc);
}


static void
mouseCB(int button, int state, int x, int y)
{
   /* invert y so that ymax is up */
   int y2 = (winHeight - y - 1);

   printf("mouse @ (%i, %i)\n", x, y2);

   if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
      printf("left down!\n");
   else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
      printf("left up!\n");
   else if (button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN)
      printf("middle down!\n");
   else if (button == GLUT_MIDDLE_BUTTON && state == GLUT_UP)
      printf("middle up!\n");
   else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
      printf("right down!\n");
   else if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP)
      printf("right up!\n");
}

/*****************************************************************************
*****************************************************************************/
int
main(int argc, char *argv[])
{
	// init OpenGL/GLUT
	glutInit(&argc, argv);
	
	// create main window
	glutInitWindowPosition(300, 100);
	glutInitWindowSize(winWidth, winHeight);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	winId = glutCreateWindow("MyWindow");

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_COLOR_MATERIAL);

	initialize();
	
	// setup callbacks
	glutDisplayFunc(refreshCB);
	glutReshapeFunc(reshapeCB);
	//glutMouseFunc(mouseCB);

	// force initial matrix setup
	reshapeCB(winWidth, winHeight);

	// set modelview matrix stack to identity
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// make GLUI GUI
	MakeGUI();
	glutMainLoop();

	return (TRUE);
}
