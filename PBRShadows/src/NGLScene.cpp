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

constexpr float znear=1.0f;
constexpr float zfar=45.0f;


void NGLScene::resizeGL( int _w, int _h )
{
  m_cam.setProjection( 45.0f, static_cast<float>( _w ) / _h, znear, zfar );
  m_win.width  = static_cast<int>( _w * devicePixelRatio() );
  m_win.height = static_cast<int>( _h * devicePixelRatio() );
}


// lights

void NGLScene::initializeGL()
{
  // we must call that first before any other GL commands to load and link the
  // gl commands from the lib, if that is not done program will crash
  ngl::NGLInit::instance();
  ngl::Vec3 lightColour(250.0f, 250.0f, 250.0f);
  m_lights[0].reset(new PointLightShadow(ngl::Vec3(-5.0f,4.0f,-5.0f),lightColour));
  m_lights[1].reset(new PointLightShadow(ngl::Vec3( 5.0f,4.0f,-5.0f),lightColour));
  m_lights[2].reset(new PointLightShadow(ngl::Vec3(-5.0f,4.0f, 5.0f),lightColour));
  m_lights[3].reset(new PointLightShadow(ngl::Vec3( 5.0f,4.0f, 5.0f),lightColour));

  TexturePack tp;
  tp.loadJSON("textures/textures.json");

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
  m_cam.set( from, to, up );
  // set the shape using FOV 45 Aspect Ratio based on Width and Height
  // The final two are near and far clipping planes of 0.5 and 10
  m_cam.setProjection( 45.0f, 720.0f / 576.0f, znear, zfar );


  shader->setUniform("camPos",m_cam.getEye());

  for(size_t i=0; i<m_lights.size(); ++i)
  {
    shader->setUniform(("lightPositions[" + std::to_string(i) + "]").c_str(),m_lights[i]->pos());
    shader->setUniform(("lightColors[" + std::to_string(i) + "]").c_str(),m_lights[i]->colour());
  }
  shader->setUniform("albedoMap", 0);
  shader->setUniform("normalMap", 1);
  shader->setUniform("metallicMap", 2);
  shader->setUniform("roughnessMap", 3);
  shader->setUniform("aoMap", 4);

  shader->setUniform("depthMap[0]",5);
  shader->setUniform("depthMap[1]",6);
  shader->setUniform("depthMap[2]",7);
  shader->setUniform("depthMap[3]",8);


  ( *shader )[ ngl::nglColourShader ]->use();
  shader->setUniform("Colour",1.0f,1.0f,1.0f,1.0f);

  ngl::VAOPrimitives::instance()->createSphere("sphere",0.5,20.0f);
  ngl::VAOPrimitives::instance()->createTrianglePlane("floor",25,25,10,10,ngl::Vec3::up());
 // ngl::Random::instance()->setSeed(m_seed);


}


void NGLScene::loadMatricesToShader()
{
  ngl::ShaderLib* shader = ngl::ShaderLib::instance();
  shader->use("PBR");
  ngl::Mat4 MV;
  ngl::Mat4 MVP;
  ngl::Mat3 normalMatrix;
  ngl::Mat4 M;
  M   = m_transform.getMatrix() ;
  MV  = m_cam.getView() * M;
  MVP = m_cam.getVP() * M;

  normalMatrix = MV;
  normalMatrix.inverse().transpose();
  shader->setUniform( "MVP", MVP );
  shader->setUniform( "normalMatrix", normalMatrix );
  shader->setUniform( "M", M );
  //shader->setUniform("textureRotation",);
  ngl::Real textureRotation=ngl::radians(180.0f);
  float cosTheta=cosf(textureRotation);
  float sinTheta=sinf(textureRotation);
  ngl::Real texRot[4]={cosTheta,sinTheta,-sinTheta,cosTheta};
  shader->setUniformMatrix2fv("textureRotation",&texRot[0]);
  shader->setUniform("camPos",m_cam.getEye());
  shader->setUniform("far_plane",zfar);
}


void NGLScene::shadowPass()
{
  auto loadToShader=[this](size_t _light){
      ngl::ShaderLib *shader=ngl::ShaderLib::instance();
      shader->use(m_lights[_light]->c_shaderName);
      shader->setUniform("M",m_transform.getMatrix());
      shader->setUniform("lightPos",m_lights[_light]->pos());
      shader->setUniform("far_plane",zfar);
};
  //----------------------------------------------------------------------------------------------------------------------
  // Pass 1 render our Depth texture to the FBO
  //----------------------------------------------------------------------------------------------------------------------
  // enable culling
  glEnable(GL_CULL_FACE);
  // Clear previous frame values
  glClear( GL_DEPTH_BUFFER_BIT);
  // as we are only rendering depth turn off the colour / alpha
  glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
  glCullFace(GL_FRONT);
  ngl::VAOPrimitives* prim = ngl::VAOPrimitives::instance();

  for(size_t i=0; i<m_lights.size(); ++i)
  {
    m_lights[i]->bindForRender();

    // render rows*column number of spheres with varying metallic/roughness values scaled by rows and columns respectively
    int nrRows    = 10;
    int nrColumns = 10;
    float spacing = 2.0;

    for (int row = 0; row < nrRows; ++row)
    {
        for (int col = 0; col < nrColumns; ++col)
        {
          // we clamp the roughness to 0.025 - 1.0 as perfectly smooth surfaces (roughness of 0.0) tend to look a bit off
          // on direct lighting.
          m_transform.setPosition(static_cast<float>(col - (nrColumns / 2)) * spacing,
                                  0.0f,
                                  static_cast<float>(row - (nrRows / 2)) * spacing);
          m_transform.setRotation(0.0f,45.0,0.0f);

          loadToShader(i);
          prim->draw("teapot");
        }
    }
    // draw floor
    m_transform.reset();
    m_transform.setPosition(0.0f,-0.5f,0.0f);
    loadToShader(i);
    prim->draw("floor");
    m_lights[i]->unbind();
  }
  glBindFramebuffer(GL_FRAMEBUFFER,defaultFramebufferObject());
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

  // grab an instance of the shader manager
  ngl::ShaderLib* shader = ngl::ShaderLib::instance();
  ( *shader )[ "PBR" ]->use();

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
  // get the VBO instance and draw the built in teapot

  ngl::VAOPrimitives* prim = ngl::VAOPrimitives::instance();
  // Shadow pass
  if(m_lightMoved ==true)
  {
    shadowPass();
    m_lightMoved=false;
  }

  /// Final Pass
  // set the viewport to the screen dimensions
  glViewport(0, 0, width() * devicePixelRatio(), height() * devicePixelRatio());
  // enable colour rendering again (as we turned it off earlier)
  glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
  // clear the screen
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  // bind the shadow texture

  // now only cull back faces
  glCullFace(GL_BACK);

  // render rows*column number of spheres with varying metallic/roughness values scaled by rows and columns respectively
  int nrRows    = 10;
  int nrColumns = 10;
  float spacing = 2.0;

  ngl::Random *rng=ngl::Random::instance();
  ngl::Random::instance()->setSeed(m_seed);
  TexturePack tp;
  // bind textures for lightmaps
  for(size_t i=0; i<m_lights.size(); ++i)
  {
    glActiveTexture(GL_TEXTURE5+i);
    glBindTexture(GL_TEXTURE_CUBE_MAP, m_lights[i]->getCubeMapID());
    std::cout<<"Binding "<<m_lights[i]->getCubeMapID()<<'\n';
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

  }
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
        shader->setUniform("metallic", static_cast<float>(row) / nrRows);
        tp.activateTexturePack(textures[static_cast<int>(rng->randomPositiveNumber(5))]);
        // we clamp the roughness to 0.025 - 1.0 as perfectly smooth surfaces (roughness of 0.0) tend to look a bit off
        // on direct lighting.
        shader->setUniform("roughnessScale", std::min(std::max(static_cast<float>(col) / nrColumns, 0.01f), 1.0f));
        m_transform.setPosition(static_cast<float>(col - (nrColumns / 2)) * spacing,
                                0.0f,
                                static_cast<float>(row - (nrRows / 2)) * spacing);
        m_transform.setRotation(0.0f,45.0f,0.0f);

        loadMatricesToShader();
        prim->draw("teapot");
      }
  }
  // draw floor
  tp.activateTexturePack("greasy");

  shader->setUniform("roughnessScale",0.0f);
  m_transform.reset();
  m_transform.setPosition(0.0f,-0.5f,0.0f);
  loadMatricesToShader();
  prim->draw("floor");


  // Draw Lights
  if(m_drawLights)
  {
    ( *shader )[ ngl::nglColourShader ]->use();
    ngl::Mat4 MVP;
    ngl::Transformation tx;
    shader->setUniform("Colour",1.0f,1.0f,1.0f,1.0f);

    for(size_t i=0; i<m_lights.size(); ++i)
    {
      if(m_lightOn[i]==true)
      {
        tx.setPosition(m_lights[i]->pos());
        MVP=m_cam.getVP()* m_mouseGlobalTX * tx.getMatrix() ;
        shader->setUniform("MVP",MVP);
        prim->draw("sphere");
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
  ngl::Random *rng=ngl::Random::instance();
  auto setLight=[](std::string _num,bool _mode)
  {
    ngl::ShaderLib *shader= ngl::ShaderLib::instance();
    shader->use("PBR");
    if(_mode == true)
    {
      ngl::Vec3 colour={255.0f,255.0f,255.0f};
      shader->setUniform(_num,colour);
    }
    else
    {
      ngl::Vec3 colour={0.0f,0.0f,0.0f};
      shader->setUniform(_num,colour);

    }

  };
  switch ( _event->key() )
  {
    // escape key to quit
    case Qt::Key_Escape:
      QGuiApplication::exit( EXIT_SUCCESS );
      break;
    case Qt::Key_R :
      m_seed=static_cast<unsigned int>(rng->randomPositiveNumber(100000));
      m_lightMoved=true;
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

