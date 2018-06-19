#include "NGLScene.h"
#include <QGuiApplication>
#include <QMouseEvent>

//#include <ngl/Camera.h>
#include <ngl/NGLInit.h>
#include <ngl/NGLStream.h>
#include <ngl/Random.h>
#include <ngl/ShaderLib.h>
#include <ngl/VAOPrimitives.h>

// This demo is based on code from here https://learnopengl.com/#!PBR/Lighting
NGLScene::NGLScene()
{
  setTitle( "PBR with GLSL" );
}


NGLScene::~NGLScene()
{
  std::cout << "Shutting down NGL, removing VAO's and Shaders\n";
}



void NGLScene::resizeGL( int _w, int _h )
{
  m_projection=ngl::perspective(45.0f, static_cast<float>( _w ) / _h, 0.05f, 350.0f );
  m_win.width  = static_cast<int>( _w * devicePixelRatio() );
  m_win.height = static_cast<int>( _h * devicePixelRatio() );
}


// lights
std::array<ngl::Vec3,4> g_lightPositions = {{
        ngl::Vec3(-10.0f,  4.0f, -10.0f),
        ngl::Vec3( 10.0f,  4.0f, -10.0f),
        ngl::Vec3(-10.0f,  4.0f, 10.0f),
        ngl::Vec3( 10.0f,  4.0f, 10.0f)
    }};

void NGLScene::initializeGL()
{
  // we must call that first before any other GL commands to load and link the
  // gl commands from the lib, if that is not done program will crash
  ngl::NGLInit::instance();
  glClearColor( 0.4f, 0.4f, 0.4f, 1.0f ); // Grey Background
  // enable depth testing for drawing
  glEnable( GL_DEPTH_TEST );
// enable multisampling for smoother drawing
#ifndef USINGIOS_
  glEnable( GL_MULTISAMPLE );
#endif
  // now to load the shader and set the values
  // grab an instance of shader manager
  ngl::ShaderLib* shader = ngl::ShaderLib::instance();
  // we are creating a shader called Phong to save typos
  // in the code create some constexpr
  constexpr auto shaderProgram = "PBR";
  constexpr auto vertexShader  = "PBRVertex";
  constexpr auto fragShader    = "PBRFragment";
  // create the shader program
  shader->createShaderProgram( shaderProgram );
  // now we are going to create empty shaders for Frag and Vert
  shader->attachShader( vertexShader, ngl::ShaderType::VERTEX );
  shader->attachShader( fragShader, ngl::ShaderType::FRAGMENT );
  // attach the source
  shader->loadShaderSource( vertexShader, "shaders/PBRVertex.glsl" );
  shader->loadShaderSource( fragShader, "shaders/PBRFragment.glsl" );
  // compile the shaders
  shader->compileShader( vertexShader );
  shader->compileShader( fragShader );
  // add them to the program
  shader->attachShaderToProgram( shaderProgram, vertexShader );
  shader->attachShaderToProgram( shaderProgram, fragShader );


  // now we have associated that data we can link the shader
  shader->linkProgramObject( shaderProgram );
  // and make it active ready to load values
  ( *shader )[ shaderProgram ]->use();
  // Now we will create a basic Camera from the graphics library
  // This is a static camera so it only needs to be set once
  // First create Values for the camera position
  ngl::Vec3 from( 0, 5, 13 );
  ngl::Vec3 to( 0, 0, 0 );
  ngl::Vec3 up( 0, 1, 0 );
  // now load to our new camera
  m_view=ngl::lookAt( from, to, up );
  m_projection=ngl::perspective(45.0f, 1024.0f  / 720.0f, 0.05f, 350.0f );


  shader->setUniform("albedo",0.5f, 0.0f, 0.0f);
  shader->setUniform("ao",1.0f);
  shader->setUniform("camPos",from);
  shader->setUniform("exposure",2.2f);

      std::array<ngl::Vec3,4>  lightColors = {{
          ngl::Vec3(300.0f, 300.0f, 300.0f),
          ngl::Vec3(300.0f, 300.0f, 300.0f),
          ngl::Vec3(300.0f, 300.0f, 300.0f),
          ngl::Vec3(300.0f, 300.0f, 300.0f)
      }};

  for(size_t i=0; i<g_lightPositions.size(); ++i)
  {
    shader->setUniform(("lightPositions[" + std::to_string(i) + "]").c_str(),g_lightPositions[i]);
    shader->setUniform(("lightColors[" + std::to_string(i) + "]").c_str(),lightColors[i]);
  }
  ( *shader )[ ngl::nglColourShader ]->use();

  ngl::VAOPrimitives::instance()->createTrianglePlane("floor",20,20,10,10,ngl::Vec3::up());
  ngl::VAOPrimitives::instance()->createSphere("sphere",1.0f,20);

}


void NGLScene::loadMatricesToShader()
{
  ngl::ShaderLib* shader = ngl::ShaderLib::instance();
  shader->use("PBR");
  struct transform
  {
    ngl::Mat4 MVP;
    ngl::Mat4 normalMatrix;
    ngl::Mat4 M;
  };

   transform t;
   t.M=m_view* m_mouseGlobalTX*m_transform.getMatrix();

   t.MVP=m_projection*t.M;
   t.normalMatrix=t.M;
   t.normalMatrix.inverse().transpose();
   shader->setUniformBuffer("TransformUBO",sizeof(transform),&t.MVP.m_00);


}

void NGLScene::paintGL()
{
  glViewport( 0, 0, m_win.width, m_win.height );
  // clear the screen and depth buffer
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  // grab an instance of the shader manager
  ngl::ShaderLib* shader = ngl::ShaderLib::instance();
  ( *shader )[ "PBR" ]->use();

  // Rotation based on the mouse position for our global transform
  ngl::Mat4 rotX;
  ngl::Mat4 rotY;
  // create the rotation matrices
  rotX.rotateX( m_win.spinXFace );
  rotY.rotateY( m_win.spinYFace );
  // multiply the rotations
  m_mouseGlobalTX = rotX * rotY;
  // add the translations
  m_mouseGlobalTX.m_m[ 3 ][ 0 ] = m_modelPos.m_x;
  m_mouseGlobalTX.m_m[ 3 ][ 1 ] = m_modelPos.m_y;
  m_mouseGlobalTX.m_m[ 3 ][ 2 ] = m_modelPos.m_z;

  // get the VBO instance and draw the built in teapot
  ngl::VAOPrimitives* prim = ngl::VAOPrimitives::instance();

  // render rows*column number of spheres with varying metallic/roughness values scaled by rows and columns respectively
  int nrRows    = 7;
  int nrColumns = 7;
  float spacing = 2.0;

  shader->setUniform("albedo",0.5f, 0.0f, 0.0f);

  shader->setUniform("ao",1.0f);
  ngl::Random *rng=ngl::Random::instance();
  rng->setSeed(10);
  for (int row = 0; row < nrRows; ++row)
  {
      shader->setUniform("metallic", (float)row / (float)nrRows);
      for (int col = 0; col < nrColumns; ++col)
      {
        // we clamp the roughness to 0.025 - 1.0 as perfectly smooth surfaces (roughness of 0.0) tend to look a bit off
        // on direct lighting.
        shader->setUniform("roughness", std::min(std::max((float)col / (float)nrColumns, 0.05f), 1.0f));
        m_transform.setPosition((float)(col - (nrColumns / 2)) * spacing,
                                0.0f,
                                (float)(row - (nrRows / 2)) * spacing);
        m_transform.setRotation(0.0f,rng->randomPositiveNumber()*360.0f,0.0f);

        loadMatricesToShader();
        prim->draw("teapot");
      }
  }
  // draw floor
  shader->setUniform("albedo",0.1f, 0.1f, 0.1f);
  shader->setUniform("ao",1.0f);
  shader->setUniform("roughness",0.02f);
  m_transform.reset();
  m_transform.setPosition(0.0f,-0.5f,0.0f);
  loadMatricesToShader();
  prim->draw("floor");


  // Draw Lights

  ( *shader )[ ngl::nglColourShader ]->use();
  ngl::Mat4 MVP;
  ngl::Transformation tx;
  shader->setUniform("Colour",1.0f,1.0f,1.0f,1.0f);

  for(size_t i=0; i<g_lightPositions.size(); ++i)
  {
    tx.setPosition(g_lightPositions[i]);
    MVP=m_projection*m_view * m_mouseGlobalTX * tx.getMatrix() ;
    shader->setUniform("MVP",MVP);
    prim->draw("sphere");
  }

}

//----------------------------------------------------------------------------------------------------------------------

void NGLScene::keyPressEvent( QKeyEvent* _event )
{
  // that method is called every time the main window recives a key event.
  // we then switch on the key value and set the camera in the GLWindow
  switch ( _event->key() )
  {
    // escape key to quit
    case Qt::Key_Escape:
      QGuiApplication::exit( EXIT_SUCCESS );
      break;
// turn on wirframe rendering
#ifndef USINGIOS_
    case Qt::Key_W:
      glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
      break;
    // turn off wire frame
    case Qt::Key_S:
      glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
      break;
#endif
    // show full screen
    case Qt::Key_F:
      showFullScreen();
      break;
    // show windowed
    case Qt::Key_N:
      showNormal();
      break;
    case Qt::Key_Space :
      m_win.spinXFace=0;
      m_win.spinYFace=0;
      m_modelPos.set(ngl::Vec3::zero());
    break;
    default:
      break;
  }
  update();
}
