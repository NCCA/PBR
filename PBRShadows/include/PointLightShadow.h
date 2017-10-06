#ifndef POINTLIGHTSHADOW_H_
#define POINTLIGHTSHADOW_H_
#include <ngl/Vec3.h>
#include <ngl/Mat4.h>
#include <array>
// based on demo code from here https://learnopengl.com/#!Advanced-Lighting/Shadows/Point-Shadows

class PointLightShadow
{
  public :
    PointLightShadow()=default;
    PointLightShadow(ngl::Vec3 _pos, ngl::Vec3 _colour,int _width=1024,int _height=1024);
    PointLightShadow(const PointLightShadow &)=delete;
    PointLightShadow &operator =(const PointLightShadow &)=delete;
    void bindForRender() noexcept;
    void unbind() noexcept;
    ~PointLightShadow();
    static const char* c_shaderName;
    GLuint getCubeMapID() const {return m_depthCubeMapID;}
    void setPos(const ngl::Vec3 &_pos);
    ngl::Vec3 pos() const {return m_pos;}
    void setColour(const ngl::Vec3 _colour){m_colour=_colour;}
    ngl::Vec3 colour() const {return  m_colour;}
    void debug();
    void setNearFar(ngl::Real _near, ngl::Real _far){ m_near=_near; m_far=_far;}
  private :
    void buildCubeMap() noexcept;
    void buildProjections() noexcept;
    void loadShaders() noexcept;
    ngl::Vec3 m_pos={0.0f,0.0f,0.0f};
    ngl::Vec3 m_colour={1.0f,1.0f,1.0f};
    ngl::Real m_near=1.0f;
    ngl::Real m_far=25.0f;
    // withd and height of texture maps
    int m_width=1024;
    int m_height=1024;
    GLuint m_depthCubeMapID=0;
    GLuint m_depthMapFBO=0;
    std::array<ngl::Mat4,6> m_shadowTransforms;
    static bool s_hasShaderLoaded;
};


#endif
