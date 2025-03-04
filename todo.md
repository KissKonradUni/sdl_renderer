2. **Constants and Magic Numbers:**
```cpp
// Add these as static constexpr members:
class Camera {
    static constexpr float MIN_FOV = 1.0f;
    static constexpr float MAX_FOV = 179.0f;
    static constexpr float MOUSE_SENSITIVITY = 0.314f;
    // ...
};
```

**Major Concerns:**

1. The camera system stores global state (`m_cameras` vector) which could lead to issues
2. No clear ownership model for shared resources (shaders, meshes)
!!! 3. Hard-coded vertex attribute layout in `Mesh` class
!!! 4. String-based uniform system could be replaced with a more type-safe approach

**Suggestions for Improvement:**

1. **Resource Management:**
```cpp
class ResourceManager {
    std::unordered_map<std::string, std::shared_ptr<Shader>> shaders;
    std::unordered_map<std::string, std::shared_ptr<Mesh>> meshes;
public:
    std::shared_ptr<Shader> getShader(const std::string& name);
    std::shared_ptr<Mesh> getMesh(const std::string& name);
};
```

3. **Type-safe Uniforms:**
```cpp
template<typename T>
class Uniform {
    GLint location;
public:
    void set(const T& value);
};
```

Module names:

Main: Cinder
Assets: Codex
IO / Logging: Echo
Logic: Hex
Network: Whisper
Audio: Chant
Animation: Puppet
Rendering: Prism
Physics: Force


TODO (Major):
Add "named" assets. Basically make every loaded asset have some reference to the file it was loaded from.
