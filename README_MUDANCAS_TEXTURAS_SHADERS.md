# Mudanças feitas: múltiplas texturas + shaders

## Arquivos principais alterados

- `src/model.h`
- `src/model.cpp`
- `Makefile`
- `assets/models/street_trees.mtl`
- `assets/models/street_trees_low.mtl`
- `assets/models/manga.mtl`

## Arquivos adicionados

- `src/shader.h`
- `src/shader.cpp`
- `assets/shaders/model.vert`
- `assets/shaders/model.frag`

## O que mudou no Model

Antes cada malha tinha apenas:

```cpp
bool hasTexture;
GLuint textureID;
```

Agora cada malha pode ter:

```cpp
hasDiffuse  / diffuseMap
hasNormal   / normalMap
hasSpecular / specularMap
hasRoughness / roughnessMap
```

O loader agora tenta ler:

```cpp
aiTextureType_DIFFUSE   // map_Kd
aiTextureType_SPECULAR  // map_Ks
aiTextureType_HEIGHT    // map_Bump
aiTextureType_NORMALS   // normal map
```

## Shaders

Os modelos agora tentam usar:

```txt
assets/shaders/model.vert
assets/shaders/model.frag
```

Se o shader falhar, o código volta para o modo antigo do OpenGL usando a textura diffuse.

## Importante

O projeto ainda usa desenho com `glBegin/glEnd`, então o shader foi feito em GLSL 120 para ser compatível com esse estilo.

Para normal map ficar correto, foi adicionado:

```cpp
aiProcess_CalcTangentSpace
```

E o `Vertex` agora guarda tangentes.

## Assets corrigidos

### street_trees.mtl e street_trees_low.mtl
Foram adicionados mapas normais e speculares onde havia textura correspondente:

```txt
map_Bump ../textures/Bark_normal.png
map_Ks ../textures/Bark_specularGlossiness.png
map_Bump ../textures/Leaf_normal.png
map_Ks ../textures/Leaf_specularGlossiness.png
map_Bump ../textures/Branch_normal.png
```

### manga.mtl
O arquivo apontava para uma normal inexistente:

```txt
lambert2SG_normal.jpeg
```

Foi trocado para uma normal existente:

```txt
../textures/material_normal.png
```

### bus_401_.mtl
Ainda existe `map_Kd .`, mas o código agora ignora esse caminho inválido em vez de tentar carregar.

## Compilação

Use:

```bash
make clean
make
make run
```

Dependências esperadas:

```txt
OpenGL
GLU
GLUT
GLEW
Assimp
```
