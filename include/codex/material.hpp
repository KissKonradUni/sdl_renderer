#pragma once

#include "codex/resource.hpp"
#include "codex/texture.hpp"
#include "codex/shader.hpp"

#include <map>

namespace Codex {

struct MaterialData {
    std::string name;
    std::map<std::string, Texture*> textures;
};

class Material : public IResource<MaterialData> {
public:
    Material(std::string& name, std::map<std::string, Texture*>& textures);
    Material();
    virtual ~Material();

    void loadData(const FileNode* file) override;
    void loadResource() override;

    void bindTextures(Shader* shader) const;

    inline constexpr const std::string& getName() const { return m_data->name; }

    void setTexture(const std::string& name, Texture* texture);
    void removeTexture(const std::string& name);
    const Texture* getTexture(const std::string& name) const;
    const std::map<std::string, Texture*>& getTextures() const { return m_data->textures; }
protected:
    std::string m_name;
    std::map<std::string, Texture*> m_textures;
};

}; // namespace Codex