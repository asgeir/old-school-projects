#pragma once

#include "ge2common.h"

#include <map>
#include <string>

namespace ge2 {

class Geometry;
class Material;
class Mesh;
class Shader;
class Texture2D;

class ResourceManager
{
public:
	ResourceManager();
	~ResourceManager();

	ResourceManager(const ResourceManager &other) = delete;
	ResourceManager &operator=(const ResourceManager &rhs) = delete;

	static ResourceManager *instance();

	std::string assetDirectory() const;
	Material   *compositorMaterial(const std::string &name);
	Geometry   *geometry(const std::string &name);
	Material   *material(const std::string &name);
	Mesh       *mesh(const std::string &name);
	MeshList    meshList(const std::string &name);
	Shader     *shader(const std::string &name);
	Texture2D  *texture2D(const std::string &name);

	void setAssetDirectory(const std::string &directory);

	Mesh      *createCube(const std::string &name, float size);
	Mesh      *createCylinder(const std::string &name, float radius, float height, int sides = 8);
	Geometry  *createGeometry(const std::string &name);
	Material  *createMaterial(const std::string &name, Shader *shader);
	Mesh      *createMesh(const std::string &name, Geometry *geometry, Material *material);
	MeshList   createMeshList(const std::string &name);
	Mesh      *createQuad(const std::string &name, float width, float height);
	Mesh      *createSphere(const std::string &name, float radius, int subdivisions = 2);
	Texture2D *createTexture2D(const std::string &name);

	Material  *loadCompositorMaterialFromFile(const std::string &name, const std::string &fragmentShaderPath, const StringList &uniforms);
	Material  *loadCompositorMaterialFromString(const std::string &name, const std::string &fragmentShader, const StringList &uniforms);
	MeshList   loadMeshListFromFile(const std::string &name, const std::string &fileName);
	Shader    *loadShaderFromFiles(const std::string &name, const std::string &vertexShaderPath, const std::string &fragmentShaderPath, const StringList &uniforms);
	Shader    *loadShaderFromStrings(const std::string &name, const std::string &vertexShader, const std::string &fragmentShader, const StringList &uniforms);
	Texture2D *loadTexture2DFromFile(const std::string &name, const std::string &fileName);

private:
	std::string preprocessShader(const std::string &shader);

	typedef std::map<std::string, Geometry *>  GeometryMap;
	typedef std::map<std::string, Material *>  MaterialMap;
	typedef std::map<std::string, Mesh *>      MeshMap;
	typedef std::map<std::string, MeshList>    MeshListMap;
	typedef std::map<std::string, Shader *>    ShaderMap;
	typedef std::map<std::string, Texture2D *> Texture2DMap;

	std::string  m_assetDirectory;
	GeometryMap  m_geometries;
	MaterialMap  m_materials;
	MeshMap      m_meshes;
	MeshListMap  m_meshLists;
	ShaderMap    m_shaders;
	Texture2DMap m_texture2ds;
};

} // namespace ge2
