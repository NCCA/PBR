#ifndef TEXTUREPACK_H_
#define TEXTUREPACK_H_

#include <unordered_map>
#include <string>
#include <vector>
#include <ngl/Types.h>

class TexturePack
{
  public :
    TexturePack();
    TexturePack(const TexturePack &)=delete;
    ~TexturePack();
    static bool loadJSON(const std::string &_filename);
    static bool activateTexturePack(const std::string &_tname);
  private :

    struct Texture
    {
      GLuint id;
      std::string name;
      GLint location;
      Texture(GLint _location, const std::string &_name, const std::string &_path);
    };
    struct Textures
    {
      std::vector<Texture> pack;
    };

    static std::unordered_map<std::string,Textures> s_textures;
};


#endif
