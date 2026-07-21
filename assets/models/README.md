# Runtime models

Export Blender models into this directory as glTF 2.0 Binary (`.glb`) files.
Keep editable `.blend` source files outside the runtime asset directory.

Models can be loaded after the OpenGL context has been initialized:

```cpp
Model spaceship;
if (!spaceship.loadFromFile("assets/models/spaceship.glb"))
{
    std::cerr << spaceship.getLastError() << '\n';
}
```

Render a loaded model with a world transform and fixed color:

```cpp
renderer.drawModel(spaceship, transform.getMatrix(), glm::vec3(0.7f, 0.8f, 0.9f));
```

The current importer supports triangle primitives with positions, normals,
texture coordinates, and 8-, 16-, or 32-bit unsigned indices. Materials,
textures, animations, sparse accessors, and non-triangle primitives are not
rendered yet.
