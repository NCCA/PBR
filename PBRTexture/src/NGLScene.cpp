#include "NGLScene.h"
#include <QGuiApplication>
#include <QMouseEvent>

#include <ngl/NGLInit.h>
#include <ngl/NGLStream.h>
#include <ngl/Random.h>
#include <ngl/ShaderLib.h>
#include <ngl/VAOPrimitives.h>
#include <ngl/Texture.h>
#include "TexturePack.h"

// This demo is based on code from here https://learnopengl.com/#!PBR/Lighting
NGLScene::NGLScene()
{
  setTitle( "PBR with GLSL" );
  m_timer.start();
}


NGLScene::~NGLScene()
{
  std::cout << "Shutting down NGL, removing VAO's and Shaders\n";
}



void NGLScene::resizeGL( int _w, int _h )
{
  m_cam.setProjection( 45.0f, static_cast<float>( _w ) / _h, 0.05f, 350.0f );
  m_win.width  = static_cast<int>( _w * devicePixelRatio() );
  m_win.height = static_cast<int>( _h * devicePixelRatio() );
}


// lights
static std::array<ngl::Vec3,4> g_lightPositions = {{
        ngl::Vec3(-5.0f,  4.0f, -5.0f),
        ngl::Vec3( 5.0f,  4.0f, -5.0f),
        ngl::Vec3(-5.0f,  4.0f, 5.0f),
        ngl::Vec3( 5.0f,  4.0f, 5.0f)
    }};
void NGLScene::initializeGL()
{
  // we must call that first before any other GL commands to load and link the
  // gl commands from the lib, if that is not done program will crash
  ngl::NGLInit::initialize();


  glClearColor( 0.4f, 0.4f, 0.4f, 1.0f ); // Grey Background
  // enable depth testing for drawing
  glEnable( GL_DEPTH_TEST );
  // enable multisampling for smoother drawing
  #ifndef USINGIOS_
  glEnable( GL_MULTISAMPLE );
  #endif
  // we are creating a shader called Phong to save typos
  // in the code create some constexpr
  constexpr auto shaderProgram = "PBR";
  constexpr auto vertexShader  = "PBRVertex";
  constexpr auto fragShader    = "PBRFragment";
  // create the shader program
/*  ngl::ShaderLib::createShaderProgram( shaderProgram );
  // now we are going to create empty shaders for Frag and Vert
  ngl::ShaderLib::attachShader( vertexShader, ngl::ShaderType::VERTEX );
  ngl::ShaderLib::attachShader( fragShader, ngl::ShaderType::FRAGMENT );
  // attach the source
  ngl::ShaderLib::loadShaderSource( vertexShader, "shaders/PBRVertex.glsl" );
  ngl::ShaderLib::loadShaderSource( fragShader, "shaders/PBRFragment.glsl" );
  // compile the shaders
  ngl::ShaderLib::compileShader( vertexShader );
  ngl::ShaderLib::compileShader( fragShader );
  // add them to the program
  ngl::ShaderLib::attachShaderToProgram( shaderProgram, vertexShader );
  ngl::ShaderLib::attachShaderToProgram( shaderProgram, fragShader );


  // now we have associated that data we can link the shader
  ngl::ShaderLib::linkProgramObject( shaderProgram );
  // and make it active ready to load values
  */
  ngl::ShaderLib::loadShader(shaderProgram, "shaders/PBRVertex.glsl", "shaders/PBRFragment.glsl");
  ngl::ShaderLib::use(shaderProgram);
  // Now we will create a basic Camera from the graphics library
  // This is a static camera so it only needs to be set once
  // First create Values for the camera position
  ngl::Vec3 from( 0, 5, 13 );
  ngl::Vec3 to( 0, 0, 0 );
  ngl::Vec3 up( 0, 1, 0 );
  // now load to our new camera
  m_cam.set( from, to, up );
  // set the shape using FOV 45 Aspect Ratio based on Width and Height
  // The final two are near and far clipping planes of 0.5 and 10
  m_cam.setProjection( 45.0f, 720.0f / 576.0f, 0.05f, 350.0f );


  ngl::ShaderLib::setUniform("camPos",m_cam.getEye());

      std::array<ngl::Vec3,4>  lightColors = {{
          ngl::Vec3(250.0f, 250.0f, 250.0f),
          ngl::Vec3(250.0f, 250.0f, 250.0f),
          ngl::Vec3(250.0f, 250.0f, 250.0f),
          ngl::Vec3(250.0f, 250.0f, 250.0f)

      }};

  for(size_t i=0; i<g_lightPositions.size(); ++i)
  {
    ngl::ShaderLib::setUniform(("lightPositions[" + std::to_string(i) + "]").c_str(),g_lightPositions[i]);
    ngl::ShaderLib::setUniform(("lightColors[" + std::to_string(i) + "]").c_str(),lightColors[i]);
  }
  ngl::ShaderLib::setUniform("albedoMap", 0);
  ngl::ShaderLib::setUniform("normalMap", 1);
  ngl::ShaderLib::setUniform("metallicMap", 2);
  ngl::ShaderLib::setUniform("roughnessMap", 3);
  ngl::ShaderLib::setUniform("aoMap", 4);



  ngl::ShaderLib::use(ngl::nglColourShader);
  ngl::ShaderLib::setUniform("Colour",1.0f,1.0f,1.0f,1.0f);



  ngl::VAOPrimitives::createSphere("sphere",0.5,20.0f);
  ngl::VAOPrimitives::createTrianglePlane("floor",25,25,10,10,ngl::Vec3::up());
  ngl::Random::setSeed(m_seed);

  TexturePack tp;
  tp.loadJSON("textures/textures.json");

}


void NGLScene::loadMatricesToShader()
{
  ngl::Mat4 MV;
  ngl::Mat4 MVP;
  ngl::Mat3 normalMatrix;
  ngl::Mat4 M;
  M            = m_transform.getMatrix() ;
  MV           = m_cam.getView() * M;
  MVP          = m_cam.getVP() * M;

  normalMatrix = MV;
  normalMatrix.inverse().transpose();
  ngl::ShaderLib::setUniform( "MVP", MVP );
  ngl::ShaderLib::setUniform( "normalMatrix", normalMatrix );
  ngl::ShaderLib::setUniform( "M", M );
  //ngl::ShaderLib::setUniform("textureRotation",);
  ngl::Real textureRotation=ngl::radians((ngl::Random::randomNumber(180.0f)));
  float cosTheta=cosf(textureRotation);
  float sinTheta=sinf(textureRotation);
  ngl::Real texRot[4]={cosTheta,sinTheta,-sinTheta,cosTheta};
  ngl::ShaderLib::setUniformMatrix2fv("textureRotation",&texRot[0]);
  ngl::ShaderLib::setUniform("camPos",m_cam.getEye());

}

void NGLScene::paintGL()
{
  float currentFrame = m_timer.elapsed()*0.001f;
  std::cout<<"Current Frame "<<currentFrame<<'\n';
  m_deltaTime = currentFrame - m_lastFrame;
  m_lastFrame = currentFrame;

  glViewport( 0, 0, m_win.width, m_win.height );
  // clear the screen and depth buffer
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  ngl::ShaderLib::use("PBR");

  /// first we reset the movement values
  float xDirection=0.0;
  float yDirection=0.0;
  // now we loop for each of the pressed keys in the the set
  // and see which ones have been pressed. If they have been pressed
  // we set the movement value to be an incremental value
  foreach(Qt::Key key, m_keysPressed)
  {
    switch (key)
    {
      case Qt::Key_Left :  { yDirection=-1.0f; break;}
      case Qt::Key_Right : { yDirection=1.0f; break;}
      case Qt::Key_Up :		 { xDirection=1.0f; break;}
      case Qt::Key_Down :  { xDirection=-1.0f; break;}
      default : break;
    }
  }
  // if the set is non zero size we can update the ship movement
  // then tell openGL to re-draw
  if(m_keysPressed.size() !=0)
  {
    m_cam.move(xDirection,yDirection,m_deltaTime);
  }

  // render rows*column number of spheres with varying metallic/roughness values scaled by rows and columns respectively
  int nrRows    = 10;
  int nrColumns = 10;
  float spacing = 2.0;

  ngl::Random::setSeed(m_seed);
  TexturePack tp;
  tp.activateTexturePack("copper");
  static  std::string textures[]=
  {
    "copper",
    "greasy",
    "panel",
    "rusty",
    "wood"
  };
  for (int row = 0; row < nrRows; ++row)
  {
      for (int col = 0; col < nrColumns; ++col)
      {
        tp.activateTexturePack(textures[static_cast<int>(ngl::Random::randomPositiveNumber(5))]);
        m_transform.setPosition(static_cast<float>(col - (nrColumns / 2)) * spacing,
                                0.0f,
                                static_cast<float>(row - (nrRows / 2)) * spacing);
        m_transform.setRotation(0.0f,ngl::Random::randomPositiveNumber()*360.0f,0.0f);

        loadMatricesToShader();
        ngl::VAOPrimitives::draw("teapot");
      }
  }
  // draw floor
  tp.activateTexturePack("greasy");

//  ngl::ShaderLib::setUniform("roughnessScale",0.0f);
  m_transform.reset();
  m_transform.setPosition(0.0f,-0.5f,0.0f);
  loadMatricesToShader();
  ngl::VAOPrimitives::draw("floor");


  // Draw Lights
  if(m_drawLights)
  {
    ngl::ShaderLib::use(ngl::nglColourShader);
    ngl::Mat4 MVP;
    ngl::Transformation tx;
    ngl::ShaderLib::setUniform("Colour",1.0f,1.0f,1.0f,1.0f);

    for(size_t i=0; i<g_lightPositions.size(); ++i)
    {
      if(m_lightOn[i]==true)
      {
        tx.setPosition(g_lightPositions[i]);
        MVP=m_cam.getVP()* m_mouseGlobalTX * tx.getMatrix() ;
        ngl::ShaderLib::setUniform("MVP",MVP);
        ngl::VAOPrimitives::draw("sphere");
      }
    }
  }

}

//----------------------------------------------------------------------------------------------------------------------

void NGLScene::keyPressEvent( QKeyEvent* _event )
{
  // add to our keypress set the values of any keys pressed
  m_keysPressed += static_cast<Qt::Key>(_event->key());
  // that method is called every time the main window recives a key event.
  // we then switch on the key value and set the camera in the GLWindow
  auto setLight=[](std::string _num,bool _mode)
  {
    ngl::ShaderLib::use("PBR");
    if(_mode == true)
    {
      ngl::Vec3 colour={255.0f,255.0f,255.0f};
      ngl::ShaderLib::setUniform(_num,colour);
    }
    else
    {
      ngl::Vec3 colour={0.0f,0.0f,0.0f};
      ngl::ShaderLib::setUniform(_num,colour);

    }

  };
  switch ( _event->key() )
  {
    // escape key to quit
    case Qt::Key_Escape:
      QGuiApplication::exit( EXIT_SUCCESS );
      break;
    case Qt::Key_R :
      m_seed=static_cast<unsigned int>(ngl::Random::randomPositiveNumber(100000));
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
    case Qt::Key_L :
      m_drawLights^=true;
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
  case Qt::Key_1 :
    setLight("lightColors[0]",m_lightOn[0]^=true); break;
  case Qt::Key_2 :
    setLight("lightColors[1]",m_lightOn[1]^=true); break;
  case Qt::Key_3 :
    setLight("lightColors[2]",m_lightOn[2]^=true); break;
  case Qt::Key_4 :
    setLight("lightColors[3]",m_lightOn[3]^=true); break;

    default:
      break;
  }
  update();
}

void NGLScene::keyReleaseEvent( QKeyEvent *_event	)
{
  // remove from our key set any keys that have been released
  m_keysPressed -= static_cast<Qt::Key>(_event->key());
}

