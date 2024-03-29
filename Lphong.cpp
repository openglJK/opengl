#include <GLTools.h>            // OpenGL toolkit
#include <stdio.h>
#include <GLMatrixStack.h>
#include <GLFrame.h>
#include <GLFrustum.h>
#include <GLGeometryTransform.h>
#include <vector>   
#include <StopWatch.h>
   
#include <math3d.h>
#ifdef __APPLE__
#include <glut/glut.h>          // OS X version of GLUT
#else
#define FREEGLUT_STATIC
#include <GL/glut.h>            // Windows FreeGlut equivalent
#endif
// poczatek deklaracji
//using namespace std;
struct PointLight {
   float position[3];
   float idiffuse[3];
   float ispecular[3];
   float attenuation[3];

   void set_position(float x, float y, float z) {
      position[0] = x;
      position[1] = y;
      position[2] = z;
   }

   void set_idiffuse(float r, float g, float b) {
      idiffuse[0] = r;
      idiffuse[1] = g;
      idiffuse[2] = b;
   }

   void set_ispecular(float r, float g, float b) {
      ispecular[0] = r;
      ispecular[1] = g;
      ispecular[2] = b;
   }

   // attenuation = 1 / (attenuation[0] + attenuation[1] * distance_to_light + attenuation[2] * pow(distance_to_light, 2))
   void set_attenuation(float attenuation_0, float attenuation_1, float attenuation_2) {
      attenuation[0] = attenuation_0;
      attenuation[1] = attenuation_1;
      attenuation[2] = attenuation_2;
   }
};
struct Material {
   float ka;    // the ratio of reflection of the ambient term present in all points in the scene rendered, [0, 1]
   float kd;    // the ratio of reflection of the diffuse term of incoming light, [0, 1]
   float ks;    // the ratio of reflection of the specular term of incoming light, [0, 1]
   float alpha; // shininess constant, which is larger for surfaces that are smoother and more mirror-like. When this constant is large the specular highlight is small

   void set_parameters(float ka, float kd, float ks, float alpha) {
      this->ka = ka;
      this->kd = kd;
      this->ks = ks;
      this->alpha = alpha;
   }
};

GLuint shader, Gshader, Pshader, swiatlo0_location,
       swiatlo0_idiffuse_location,
       swiatlo0_ispecular_location,
       swiatlo0_attenuation_location,
       material0_ka_location,
       material0_kd_location,
       material0_ks_location,
       material0_alpha_location,
	   iambient_component_location;

GLTriangleBatch     sphereBatch;
PointLight swiatlo0;
Material material0;
GLFrame viewFrame;
GLFrustum  viewFrustum;
M3DMatrix44f mModelViewProjection;
M3DMatrix44f mViewProjection;
M3DMatrix44f mCamera;
double PI=3.14159;
M3DVector3f upVector;
M3DVector3f posVector;
M3DVector3f atVector;
M3DVector3f position, color;
GLfloat angle;
GLint MVPMatrixLocation, 
		MVMatrixLocation, 
		NMatrixLocation, 
		VMmatrixLocation;
		
M3DMatrix44f mM;
CStopWatch timer;
//GLTriangleBatch     sphereBatch;
GLGeometryTransform geometryPipeline;
GLMatrixStack modelView;
GLMatrixStack projection;
float iambient_component[] = {0.2f, 0.2f, 0.2f};

   
// koniec deklaracji

void DrawPramid(float bok,float podst,float wys){
    glBegin(GL_QUADS);
        glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 0.5, 0.5, 0.5);  
        glVertex3f(bok, bok,     podst);   
        glVertex3f(-bok,    bok,    podst);
        glVertex3f(-bok,    -bok,    podst);
        glVertex3f(bok, -bok,    podst);   
    glEnd();
    // N- zolty
    glBegin(GL_TRIANGLES);
        glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 1.0, 1.0, 0.0);
        glVertex3f(bok, bok,    podst);
        glVertex3f(-bok,    bok,    podst);
        glVertex3f(podst,   podst,  wys);
    glEnd();
  //W -purp
    glBegin(GL_TRIANGLES);
        glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 1.0, 0.0, 1.0);
        glVertex3f(-bok,    bok,    podst);
        glVertex3f(-bok,    -bok,   podst);
        glVertex3f(podst,   podst,  wys);
    glEnd();
//E - cyjan
  glBegin(GL_TRIANGLES);
  //  glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 0.0, 1.0, 1.0);
    glVertex3f(bok,     bok,    podst);
    glVertex3f(bok,     -bok,   podst);
    glVertex3f(podst,   podst,  wys);
  glEnd();
//S
  glBegin(GL_TRIANGLES);
    glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 0.0, 1.0, 0.0);
    glVertex3f(-bok,    -bok,   podst);
    glVertex3f(bok, -bok,   podst);
    glVertex3f(podst,   podst,  wys);
  glEnd();
    }
// funkcje setUpFrame oraz LookAt do obs³ugi kamery
void SetUpFrame(GLFrame &frame,const M3DVector3f origin, const M3DVector3f forward, const M3DVector3f up){
         frame.SetOrigin(origin);
         frame.SetForwardVector(forward);
         M3DVector3f side,oUp;
         m3dCrossProduct3(side,forward,up);
         m3dCrossProduct3(oUp,side,forward);
         frame.SetUpVector(oUp);
         frame.Normalize();
      
        };
    void LookAt(GLFrame &frame, const M3DVector3f eye, const M3DVector3f at, const M3DVector3f up){
        M3DVector3f forward;
        m3dSubtractVectors3(forward, at, eye);
        SetUpFrame(frame, eye, forward, up);   
     };
  
  
void TriangleFace(M3DVector3f a, M3DVector3f b, M3DVector3f c) {
      M3DVector3f normal, bMa, cMa;
      m3dSubtractVectors3(bMa, b, a);
      m3dSubtractVectors3(cMa, c, a);
      m3dCrossProduct3(normal, bMa, cMa);
      m3dNormalizeVector3(normal);
      glVertexAttrib3fv(GLT_ATTRIBUTE_NORMAL, normal);
      glVertex3fv(a);
      glVertex3fv(b);
      glVertex3fv(c);
}
  
float ico_vertices[3 * 12] = {
      0.0, 0.0, -0.9510565162951536,
      0.0, 0.0, 0.9510565162951536,
      -0.85065080835204, 0., -0.42532540417601994,
      0.85065080835204, 0., 0.42532540417601994,
      0.6881909602355868, -0.5, -0.42532540417601994,
      0.6881909602355868, 0.5, -0.42532540417601994,
      -0.6881909602355868, -0.5, 0.42532540417601994,
      -0.6881909602355868, 0.5, 0.42532540417601994,
      -0.2628655560595668, -0.8090169943749475, -0.42532540417601994,
      -0.2628655560595668, 0.8090169943749475, -0.42532540417601994,
      0.2628655560595668, -0.8090169943749475, 0.42532540417601994,
      0.2628655560595668, 0.8090169943749475, 0.42532540417601994
      };
int ico_faces[3*20]={
      1 ,            11 ,            7 ,
      1 ,            7 ,             6 ,
      1 ,            6 ,             10 ,
      1 ,            10 ,            3 ,
      1 ,            3 ,             11 ,
      4 ,            8 ,             0 ,
      5 ,            4 ,             0 ,
      9 ,            5 ,             0 ,
      2 ,            9 ,             0 ,
      8 ,            2 ,             0 ,
      11 ,           9 ,             7 ,
      7 ,            2 ,             6 ,
      6 ,            8 ,             10 ,
      10 ,           4 ,             3 ,
      3 ,            5 ,             11 ,
      4 ,            10 ,            8 ,
      5 ,            3 ,             4 ,
      9 ,            11 ,            5 ,
      2 ,            7 ,             9 ,
      8 ,            6 ,             2 
};

void drawTriangles(int n_faces, float *vertices, int *faces) {
      for (int i = 0; i < n_faces; i++) {
      glBegin(GL_TRIANGLES);
      TriangleFace(vertices + 3 * faces[3 * i], vertices + 3 * faces[3 * i + 1], vertices + 3 * faces[3 * i + 2]);
      glEnd();
      }
      }
  
void drawSmoothTriangles(int n_faces, float *vertices, int *faces) {
      M3DVector3f normal;
      for (int i = 0; i < n_faces; i++) {
      glBegin(GL_TRIANGLES);
      for(int j=0;j<3;++j) {
      m3dCopyVector3(normal,vertices+3*faces[i*3+j]);
      m3dNormalizeVector3(normal);
      glVertexAttrib3fv(GLT_ATTRIBUTE_NORMAL, normal);
      glVertex3fv(vertices+3*faces[i*3+j]);
       
      }
      glEnd();
      }
      }
  
 ///////////////////////////////////////////////////////////////////////////////
// Window has changed size, or has just been created. In either case, we need
// to use the window dimensions to set the viewport and the projection matrix.
   
  void ChangeSize(int w, int h){
    if(h == 0)  { h = 1; }
    glViewport(0, 0, w, h);      
    viewFrustum.SetPerspective(35.0f, float(w)/float(h), 1.0f, 1000.0f);
    projection.LoadMatrix(viewFrustum.GetProjectionMatrix());
	geometryPipeline.SetMatrixStacks(modelView, projection);
   }
   
///////////////////////////////////////////////////////////////////////////////
// This function does any needed initialization on the rendering context.
// This is the first opportunity to do any OpenGL related tasks.
   
void SetupRC() {
    // Blue background
   glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
   shader =gltLoadShaderPairWithAttributes("pass_thru_shader.vp", "pass_thru_shader.fp", 2, GLT_ATTRIBUTE_VERTEX, "vertex_position", GLT_ATTRIBUTE_COLOR, "vertex_color");
   //Gshader =gltLoadShaderPairWithAttributes("Gouraud_Shader.vp", "Gouraud_Shader.fp", 3, GLT_ATTRIBUTE_VERTEX, "vertex_position", GLT_ATTRIBUTE_COLOR, "vertex_color", GLT_ATTRIBUTE_NORMAL, "vertex_normal");
   Pshader =gltLoadShaderPairWithAttributes("PhongShader.vp", "PhongShader.fp", 3, GLT_ATTRIBUTE_VERTEX, "vertex_position", GLT_ATTRIBUTE_COLOR, "vertex_color", GLT_ATTRIBUTE_NORMAL, "vertex_normal");
  
   glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    upVector[0] = 0.0f;
    upVector[1] = 0.0f;
    upVector[2] = 1.0f;
   
    atVector[0] = 0.0f;
    atVector[1] = 0.0f;
    atVector[2] = 0.0f;
   
    posVector[0] = 6.8f;
    posVector[1] = 6.0f;
    posVector[2] = 5.0f;
    // gltMakeSphere(sphereBatch, 1.0f, 26, 13);
  
    color[0] = 100.0f;
    color[1] = 0.0f;
    color[2] = 1.0f;
    position[0] = 10.0f;
    position[1] = 20.0f;
    position[2] = 100.0f;
   
  
    MVPMatrixLocation = glGetUniformLocation(Pshader,"MVPMatrix");
    if(MVPMatrixLocation == -1){
        fprintf(stderr,"uniform MVPMatrix could not be found\n");
        }
    MVMatrixLocation = glGetUniformLocation(Pshader,"MVMatrix");
    if(MVPMatrixLocation == -1){
        fprintf(stderr,"uniform MVMatrix could not be found\n");
        }    
    NMatrixLocation = glGetUniformLocation(Pshader,"NMatrix");
    if(MVPMatrixLocation == -1){
        fprintf(stderr,"uniform normalMatrix could not be found\n");
        }
     
    VMmatrixLocation= glGetUniformLocation(Pshader,"VMatrix");
    if(MVPMatrixLocation == -1){
        fprintf(stderr,"uniform VMatrix could not be found\n");
        }
	 iambient_component_location = glGetUniformLocation(Pshader, "iambient_component");
	if(iambient_component_location== -1){
        fprintf(stderr,"uniform iambient_component_location could not be found\n");
        }
	 swiatlo0_location = glGetUniformLocation(Pshader, "swiatlo0.position");
	 if(swiatlo0_location== -1){
        fprintf(stderr,"uniform swiatlo0_location could not be found\n");
        }
   swiatlo0_idiffuse_location = glGetUniformLocation(Pshader, "swiatlo0.idiffuse");
 if(swiatlo0_idiffuse_location== -1){
        fprintf(stderr,"uniform swiatlo0_idiffuse_location could not be found\n");
        }
   swiatlo0_ispecular_location = glGetUniformLocation(Pshader, "swiatlo0.ispecular");
    if(swiatlo0_ispecular_location== -1){
        fprintf(stderr,"uniform swiatlo0_ispecular_location could not be found\n");
        }
   swiatlo0_attenuation_location = glGetUniformLocation(Pshader, "swiatlo0.attenuation");
    if(swiatlo0_attenuation_location== -1){
        fprintf(stderr,"uniform swiatlo0_attenuation_location could not be found\n");
        }
   material0_ka_location = glGetUniformLocation(Pshader, "material0.ka");
    if(material0_ka_location== -1){
        fprintf(stderr,"uniform material0_ka_location could not be found\n");
        }
   material0_kd_location = glGetUniformLocation(Pshader, "material0.kd");
    if(material0_kd_location== -1){
        fprintf(stderr,"uniform material0_kd_location could not be found\n");
        }
   material0_ks_location = glGetUniformLocation(Pshader, "material0.ks");
    if(material0_ks_location== -1){
        fprintf(stderr,"uniform material0_ks_location could not be found\n");
        }
   material0_alpha_location = glGetUniformLocation(Pshader, "material0.alpha");
    if(material0_alpha_location== -1){
        fprintf(stderr,"uniform material0_alpha_location could not be found\n");
        }
	swiatlo0.set_position(0.0f, 0.0f, 0.0f);
   swiatlo0.set_idiffuse(1.0f, 1.0f, 1.0f);
   swiatlo0.set_ispecular(1.0f, 1.0f, 1.0f);
   swiatlo0.set_attenuation(0.0f, 0.1f, 0.0f);
   material0.set_parameters(1.0f, 1.0f, 1.0f, 200.0f);


    }
  
   
///////////////////////////////////////////////////////////////////////////////
// Called to draw scene
   
void RenderScene(void) {
  


    // Clear the window with current clearing color
      
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    // Obracanie widoku
 float angle =timer.GetElapsedSeconds()*PI/4;
	posVector[0] = -8.0f * cos(angle / 2.0f);
    posVector[1] = -8.0f * sin(angle / 2.0f);
    posVector[2] = 5.0f;
  

	 swiatlo0.position[0] = 8.0f * cos(-angle);
   swiatlo0.position[1] = 15.0f * sin(-angle);
   swiatlo0.position[2] = 5.0f;

 LookAt(viewFrame, posVector, atVector, upVector);
     
  
  
geometryPipeline.SetMatrixStacks(modelView,projection);

      
    projection.LoadMatrix(viewFrustum.GetProjectionMatrix());
modelView.PushMatrix();
M3DMatrix44f mCamera;
viewFrame.GetCameraMatrix(mCamera);
modelView.LoadMatrix(mCamera);
modelView.PushMatrix();
//glUseProgram(shader);
modelView.Translate(swiatlo0.position[0], swiatlo0.position[1], swiatlo0.position[2]);
glUniformMatrix4fv(MVPMatrixLocation,1,GL_FALSE,geometryPipeline.GetModelViewProjectionMatrix());
modelView.PopMatrix();


    //mno¿enie macierzy
    //modelView.MultMatrix(mCamera);
   // modelView.PushMatrix();
    //m3dMatrixMultiply44(mModelViewProjection, viewFrustum.GetProjectionMatrix(), mCamera);
  // wrzucanie do shadera
   
glUniformMatrix3fv(NMatrixLocation, 1, GL_FALSE, geometryPipeline.GetNormalMatrix());
   glUniformMatrix4fv(VMmatrixLocation, 1, GL_FALSE, mCamera);
   glUniform3fv(iambient_component_location, 1, iambient_component);
   glUniform3fv(swiatlo0_location, 1, swiatlo0.position);
   glUniform3fv(swiatlo0_idiffuse_location, 1, swiatlo0.idiffuse);
   glUniform3fv(swiatlo0_ispecular_location, 1, swiatlo0.ispecular);
   glUniform3fv(swiatlo0_attenuation_location, 1, swiatlo0.attenuation);
   glUniform1f(material0_ka_location, material0.ka);
   glUniform1f(material0_kd_location, material0.kd);
   glUniform1f(material0_ks_location, material0.ks);
   glUniform1f(material0_alpha_location, material0.alpha);
   //--
   glUniformMatrix4fv(MVPMatrixLocation, 1, GL_FALSE, geometryPipeline.GetModelViewProjectionMatrix());
   glUniformMatrix4fv(MVMatrixLocation, 1, GL_FALSE, geometryPipeline.GetModelViewMatrix());


     //DrawPramid(0.5,0.0,1.0);    
  drawTriangles(20,ico_vertices,ico_faces);
// rysowanie siatki --------------------------------------------------------------------------
    glEnable(GL_CULL_FACE);
    glPolygonOffset(1.0f, 1.0f);
    glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 1.0, 1.0, 1.0);
    glEnable(GL_POLYGON_OFFSET_FILL);
glBegin(GL_QUADS);
glVertex3f(10,    -10.0f,   0.0f);
  
  
glVertex3f(-10,    -10.0f,  0.0f);
glVertex3f(-10,    10.0f,   0.0f);
glVertex3f(10,    10.0f,   0.0f);
  
  
glEnd();
glDisable(GL_POLYGON_OFFSET_FILL);
   glVertexAttrib3f(GLT_ATTRIBUTE_COLOR, 1.0, 1.0, 1.0);
    for(int i=-10;i<=10;i++){
    glBegin(GL_LINES);
            glVertex3f((float)i,    -10.0f,  0.0f);
            glVertex3f((float)i,    10.0f,   0.0f);
            glEnd();
    }
for(int i=-10;i<=10;i++){
    glBegin(GL_LINES);
            glVertex3f(-10.0f,  (float)i,    0.0f);
            glVertex3f(10.0f,   (float)i,    0.0f);
    glEnd();
    }
     glDisable(GL_CULL_FACE); 
	 //sphereBatch.Draw();
	 glUniformMatrix4fv(MVPMatrixLocation, 1, GL_FALSE, geometryPipeline.GetModelViewProjectionMatrix());
   glUniformMatrix4fv(MVMatrixLocation, 1, GL_FALSE, geometryPipeline.GetModelViewMatrix());
modelView.PopMatrix();
    // rysowanie siatki - koniec -------------------------------------------------------------------
//matrixStack.Translate(10.0f,1.0f,0.0f);
 
// macierz translacji i macierz rotacji
//M3DMatrix44f mT,mR;
 // m3dTranslationMatrix44(mT,0.0f,3.0f,0.0f);
  //m3dRotationMatrix44(mR, angle, 0.0f, 0.0f, 1.0f);
  
  //mno¿enie  macierzy translacji i macierzy rotacji
//m3dMatrixMultiply44(mM,mT,mR);
  
//mno¿enie macierzy przekszta³ceñ i macierzy projekcji modelwidok
// m3dMatrixMultiply44(mModelViewProjection,mModelViewProjection, mM);
  
 //wysy³anie macierzy projekcji modelwidok do shadera
 //glUniformMatrix4fv(MVPMatrixLocation, 1, GL_FALSE, mModelViewProjection);
  //TriangleFace(position,color,atVector);
 //DrawPramid(0.5,0.0,1.0);
   
     //drawTriangles(20, ico_vertices, ico_faces);
//matrixStack.PopMatrix();
//matrixStack.Rotate(45.0,0,0,1);
//matrixStack.Translate(0.0,0.0,1.0);
//glUniformMatrix4fv(MVPMatrixLocation,1,GL_FALSE,geometryPipeline.GetModelViewProjectionMatrix());
//DrawPramid(0.5,0.0,1.0);
	glUniformMatrix4fv(MVPMatrixLocation, 1, GL_FALSE, geometryPipeline.GetModelViewProjectionMatrix());
   glUniformMatrix4fv(MVMatrixLocation, 1, GL_FALSE, geometryPipeline.GetModelViewMatrix());
modelView.PopMatrix();


    glUseProgram(Pshader);//wywo³anie shadera
   
     
    // Perform the buffer swap to display back buffer
    glutSwapBuffers();
    glutPostRedisplay();
}
   
   
///////////////////////////////////////////////////////////////////////////////
// Main entry point for GLUT based programs
   
int main(int argc, char* argv[]) {
    
   
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_STENCIL);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Triangle Cw 6");
    glutReshapeFunc(ChangeSize);
    glutDisplayFunc(RenderScene);
   
    GLenum err = glewInit();
    if (GLEW_OK != err) {
        fprintf(stderr, "GLEW Error: %s\n", glewGetErrorString(err));
        return 1;
    }
   
    SetupRC();
   
    glutMainLoop();
    return 0;
}