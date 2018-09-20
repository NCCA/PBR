#include "TexturePack.h"
#include <ngl/rapidjson/document.h>
#include <fstream>
#include <iostream>
#include <ngl/Texture.h>

std::unordered_map<std::string,TexturePack::Textures> TexturePack::s_textures;


TexturePack::TexturePack()
{

}

TexturePack::~TexturePack()
{

}

TexturePack::Texture::Texture(GLint _location, const std::string &_name,const std::string &_path)
{
  auto setTextureParams=[]()
  {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_LINEAR);
  };

  location=_location;
  name=_name;
  ngl::Texture t(_path);
  t.setMultiTexture(GL_TEXTURE0+location);
  id=t.setTextureGL();
  setTextureParams();
}


bool TexturePack::loadJSON(const std::string &_filename)
{
  bool success=false;
  namespace rj=rapidjson;
  std::ifstream file;
  file.open(_filename.c_str(), std::ios::in);
  if (file.fail())
  {
      std::cerr<<"error opening json file\n";
      return false;
  }
  std::unique_ptr<std::string> source( new std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>()) );
  file.close();
  // we need a mutable string for parsing so copy to a char * buffer

  std::unique_ptr<char []> buffer(new char[source->size()]);
  memcpy(buffer.get(), source->c_str(), source->size());
  // null terminate the string!
  buffer[source->size()]='\0';

  rj::Document doc;

  if (doc.ParseInsitu<0>(buffer.get()).HasParseError())
  {
    std::cerr<<"Parse Error for file "<<_filename<<'\n';
    return false;
  }

  if(!doc.HasMember("TexturePack"))
  {
    std::cerr<<"This does not seem to be a valid Texture Pack json file\n";
    return false;
  }
  std::cout<<"***************Loading Texture Pack from JSON*****************\n";
  // Now we iterate through the json and gather our data.
  for (rj::Value::ConstMemberIterator itr = doc.MemberBegin(); itr != doc.MemberEnd(); ++itr)
  {
    Textures pack;
    const rj::Value::Ch* material=itr->value["material"].GetString();
    std::cout<<"found material "<< material<<'\n';
    const rj::Value& textures = itr->value["Textures"];
    for (rj::SizeType i = 0; i < textures.Size(); ++i)
    {
      const rj::Value &currentTexture = textures[i];
      auto location=currentTexture["location"].GetInt();
      const rj::Value::Ch *name=currentTexture["name"].GetString();
      const rj::Value::Ch *path=currentTexture["path"].GetString();
      std::cout<<"Found "<<name<<' '<<location<<' '<<path<<'\n';
      Texture t(location,name,path);
      pack.pack.push_back(t);
    }
    s_textures[material]=pack;
  }

  return success;
}

bool TexturePack::activateTexturePack(const std::string &_tname)
{
  bool success=false;
  auto pack=s_textures.find(_tname);
  // make sure we have a valid shader
  if(pack!=s_textures.end())
  {
    success=true;
    for(auto t : pack->second.pack)
    {
      glActiveTexture(GL_TEXTURE0+t.location);
      glBindTexture(GL_TEXTURE_2D, t.id);
    }
  }


  return success;

}
