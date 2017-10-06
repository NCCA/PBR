#include "PointLightShadow.h"
#include <ngl/ShaderLib.h>
#include <ngl/Util.h>
#include <ngl/NGLStream.h>
#include <iostream>

bool PointLightShadow::s_hasShaderLoaded=false;
const char* PointLightShadow::c_shaderName = "PointLightShadow";

PointLightShadow::PointLightShadow(ngl::Vec3 _pos, ngl::Vec3 _colour, int _width, int _height)
{
  m_pos=_pos;
  m_colour=_colour;
  m_width=_width;
  m_height=_height;
  buildCubeMap();
  buildProjections();
  if(s_hasShaderLoaded ==false)
  {
    loadShaders();
    s_hasShaderLoaded=true;

  }
}

PointLightShadow::~PointLightShadow()
{

}

void PointLightShadow::buildCubeMap() noexcept
{
  glGenFramebuffers(1, &m_depthMapFBO);
  // create depth cubemap texture
  glGenTextures(1, &m_depthCubeMapID);
  glBindTexture(GL_TEXTURE_CUBE_MAP, m_depthCubeMapID);
  for (unsigned int i = 0; i < 6; ++i)
  {
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  }
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  // attach depth texture as FBO's depth buffer
  glBindFramebuffer(GL_FRAMEBUFFER, m_depthCubeMapID);
  glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, m_depthCubeMapID, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

}

void PointLightShadow::buildProjections() noexcept
{
  ngl::Mat4 shadowProj = ngl::perspective(90.0f,static_cast<float>(m_width) / static_cast<float>(m_height), m_near, m_far);
  m_shadowTransforms[0]=shadowProj * ngl::lookAt(m_pos, m_pos +ngl::Vec3( 1.0f,  0.0f,  0.0f),ngl::Vec3(0.0f, -1.0f,  0.0f));
  m_shadowTransforms[1]=shadowProj * ngl::lookAt(m_pos, m_pos +ngl::Vec3(-1.0f,  0.0f,  0.0f),ngl::Vec3(0.0f, -1.0f,  0.0f));
  m_shadowTransforms[2]=shadowProj * ngl::lookAt(m_pos, m_pos +ngl::Vec3( 0.0f,  1.0f,  0.0f),ngl::Vec3(0.0f,  0.0f,  1.0f));
  m_shadowTransforms[3]=shadowProj * ngl::lookAt(m_pos, m_pos +ngl::Vec3( 0.0f, -1.0f,  0.0f),ngl::Vec3(0.0f,  0.0f, -1.0f));
  m_shadowTransforms[4]=shadowProj * ngl::lookAt(m_pos, m_pos +ngl::Vec3( 0.0f,  0.0f,  1.0f),ngl::Vec3(0.0f, -1.0f,  0.0f));
  m_shadowTransforms[5]=shadowProj * ngl::lookAt(m_pos, m_pos +ngl::Vec3( 0.0f,  0.0f, -1.0f),ngl::Vec3(0.0f, -1.0f,  0.0f));

}

void PointLightShadow::loadShaders() noexcept
{
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  shader->loadShader(c_shaderName,"shaders/PointShadowVertex.glsl",
                                  "shaders/PointShadowFragment.glsl",
                                  "shaders/PointShadowGeometry.glsl",
                                  false);
  shader->linkProgramObject(c_shaderName);
  shader->use(c_shaderName);

}

void PointLightShadow::bindForRender() noexcept
{
  glViewport(0, 0, m_width, m_height);
  glBindFramebuffer(GL_FRAMEBUFFER, m_depthMapFBO);
  glClear(GL_DEPTH_BUFFER_BIT);
  ngl::ShaderLib *shader=ngl::ShaderLib::instance();
  shader->use(c_shaderName);

  for (unsigned int i = 0; i < 6; ++i)
  {
    shader->setUniform("shadowMatrices[" + std::to_string(i) + "]", m_shadowTransforms[i]);
    shader->setUniform("far_plane", m_far);
    shader->setUniform("lightPos", m_pos);
   }
}


void PointLightShadow::unbind() noexcept
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void PointLightShadow::setPos(const ngl::Vec3 &_pos)
{
  m_pos=_pos;
  buildProjections();
}

void PointLightShadow::debug()
{
  for(auto s : m_shadowTransforms)
    std::cout<<s<<'\n';

}
