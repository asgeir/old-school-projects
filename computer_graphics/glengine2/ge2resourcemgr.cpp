#include "ge2resourcemgr.h"

#include "ge2fsquad.h"
#include "ge2geometry.h"
#include "ge2material.h"
#include "ge2mesh.h"
#include "ge2shader.h"
#include "ge2texture2d.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <cassert>
#include <fstream>
#include <iostream>
#include <iterator>
#include <list>
#include <sstream>

using namespace ge2;

namespace {

const std::string kDefaultShaderName = "_ge_internal_default_shader";

const std::string kDefaultVertexShader = R"(
	#version 150

	in vec4 ge_position;

	uniform mat4 ge_modelViewProjection;

	void main()
	{
		gl_Position = ge_modelViewProjection * ge_position;
	}
)";

const std::string kDefaultFragmentShader = R"(
	#version 150

	out vec4 ge_fragmentColor;

	void main()
	{
		ge_fragmentColor = vec4(0.6, 0.6, 0.6, 1.0);
	}
)";

std::string parseInclude(const std::string &line)
{
	std::string capture;

	// Stupid libc++ with gcc < 4.8 not supporting regex
	int state = 0;
	for (int i = 0, e = line.size(); i < e; ++i) {
		if (state ==  0 && (line[i] == ' ' || line[i] == '\t')) continue;
		if (state ==  0 &&  line[i] != ' ' && line[i] != '\t') ++state;
		if (state ==  1 &&  line[i] == '#') { ++state; continue; } else if (state ==  1) { capture = std::string{}; break; }
		if (state ==  2 &&  line[i] == 'g') { ++state; continue; } else if (state ==  2) { capture = std::string{}; break; }
		if (state ==  3 &&  line[i] == 'e') { ++state; continue; } else if (state ==  3) { capture = std::string{}; break; }
		if (state ==  4 &&  line[i] == '_') { ++state; continue; } else if (state ==  4) { capture = std::string{}; break; }
		if (state ==  5 &&  line[i] == 'i') { ++state; continue; } else if (state ==  5) { capture = std::string{}; break; }
		if (state ==  6 &&  line[i] == 'n') { ++state; continue; } else if (state ==  6) { capture = std::string{}; break; }
		if (state ==  7 &&  line[i] == 'c') { ++state; continue; } else if (state ==  7) { capture = std::string{}; break; }
		if (state ==  8 &&  line[i] == 'l') { ++state; continue; } else if (state ==  8) { capture = std::string{}; break; }
		if (state ==  9 &&  line[i] == 'u') { ++state; continue; } else if (state ==  9) { capture = std::string{}; break; }
		if (state == 10 &&  line[i] == 'd') { ++state; continue; } else if (state == 10) { capture = std::string{}; break; }
		if (state == 11 &&  line[i] == 'e') { ++state; continue; } else if (state == 11) { capture = std::string{}; break; }
		if (state == 12 && (line[i] == ' ' || line[i] == '\t')) { ++state; continue; } else if (state == 12) { capture = std::string{}; break; }
		if (state == 13 && (line[i] == ' ' || line[i] == '\t')) continue;
		if (state == 13 &&  line[i] != ' ' && line[i] != '\t') ++state;
		if (state == 14 &&  line[i] == '"') { ++state; continue; } else if (state == 14) { capture = std::string{}; break; }
		if (state == 15 &&  line[i] == '"') { ++state; continue; } else if (state == 15) capture = capture + line[i];
		if (state == 16 &&  line[i] != ' ' && line[i] != '\t') { capture = std::string{}; break; }
	}

	return capture;
}

std::string readShaderFile(const std::string &filePath)
{
	std::ifstream in(filePath, std::ios::in | std::ios::binary);
	if (in) {
		std::string contents;
		in.seekg(0, std::ios::end);
		contents.resize(in.tellg());
		in.seekg(0, std::ios::beg);
		in.read(&contents[0], contents.size());
		in.close();
		return std::move(contents);
	}
	return {};
}

std::string compositorMaterialRealName(const std::string name)
{
	return "compositor_effect_" + name;
}

class Line : public std::string
{
	friend std::istream & operator>>(std::istream & is, Line & line)
	{
		return std::getline(is, line);
	}
};

} // namespace

ResourceManager::ResourceManager()
{
	loadShaderFromStrings(kDefaultShaderName, kDefaultVertexShader, kDefaultFragmentShader, { "ge_modelViewProjection" });
}

ResourceManager::~ResourceManager()
{
}

std::string ResourceManager::assetDirectory() const
{
	return m_assetDirectory;
}

Material *ResourceManager::compositorMaterial(const std::string &name)
{
	std::string realName = compositorMaterialRealName(name);
	auto it = m_materials.find(realName);
	if (it != m_materials.end()) {
		return it->second;
	}
	return nullptr;
}

Geometry *ResourceManager::geometry(const std::string &name)
{
	auto it = m_geometries.find(name);
	if (it != m_geometries.end()) {
		return it->second;
	}
	return nullptr;
}

Material *ResourceManager::material(const std::string &name)
{
	auto it = m_materials.find(name);
	if (it != m_materials.end()) {
		return it->second;
	}
	return nullptr;
}

Mesh *ResourceManager::mesh(const std::string &name)
{
	auto it = m_meshes.find(name);
	if (it != m_meshes.end()) {
		return it->second;
	}
	return nullptr;
}

Shader *ResourceManager::shader(const std::string &name)
{
	auto it = m_shaders.find(name);
	if (it != m_shaders.end()) {
		return it->second;
	}
	return nullptr;
}

Texture2D *ResourceManager::texture2D(const std::string &name)
{
	auto it = m_texture2ds.find(name);
	if (it != m_texture2ds.end()) {
		return it->second;
	}
	return nullptr;
}

void ResourceManager::setAssetDirectory(const std::string &directory)
{
	m_assetDirectory = directory;
}

Mesh *ResourceManager::createCube(const std::string &name, float size)
{
	if (size <= 0 || m_meshes.find(name) != m_meshes.end()) {
		return nullptr;
	}

	Geometry *cube = Geometry::createCube(size);
	m_geometries["_ge_internal_" + name] = cube;

	Mesh *cubeMesh = new Mesh{cube, nullptr};
	m_meshes[name] = cubeMesh;

	return cubeMesh;
}

Mesh *ResourceManager::createCylinder(const std::string &name, float radius, float height, int sides)
{
	if (radius <= 0 || height <= 0 || sides < 3 || m_meshes.find(name) != m_meshes.end()) {
		return nullptr;
	}

	Geometry *cylinder = Geometry::createCylinder(radius, height, sides);
	m_geometries["_ge_internal_" + name] = cylinder;

	Mesh *cylinderMesh = new Mesh{cylinder, nullptr};
	m_meshes[name] = cylinderMesh;

	return cylinderMesh;
}

Geometry *ResourceManager::createGeometry(const std::string &name)
{
	if (m_geometries.find(name) != m_geometries.end()) {
		return nullptr;
	}

	Geometry *geometry = new Geometry;
	m_geometries[name] = geometry;

	return geometry;
}

Material *ResourceManager::createMaterial(const std::string &name, Shader *shader)
{
	if (!shader || m_materials.find(name) != m_materials.end()) {
		return nullptr;
	}

	Material *material = new Material{shader};
	m_materials[name] = material;

	return material;
}

Mesh *ResourceManager::createMesh(const std::string &name, Geometry *geometry, Material *material)
{
	if (!geometry || !material || m_meshes.find(name) != m_meshes.end()) {
		return nullptr;
	}

	Mesh *mesh = new Mesh{geometry, material};
	m_meshes[name] = mesh;

	return mesh;
}

Mesh *ResourceManager::createQuad(const std::string &name, float width, float height)
{
	if (width <= 0 || height <= 0 || m_meshes.find(name) != m_meshes.end()) {
		return nullptr;
	}

	Geometry *quad = Geometry::createQuad(width, height);
	m_geometries["_ge_internal_" + name] = quad;

	Mesh *quadMesh = new Mesh{quad, nullptr};
	m_meshes[name] = quadMesh;

	return quadMesh;
}

Mesh *ResourceManager::createSphere(const std::string &name, float radius, int subdivisions)
{
	if (radius < 0.0f || subdivisions < 0 || m_meshes.find(name) != m_meshes.end()) {
		return nullptr;
	}

	Geometry *sphere = Geometry::createSphere(radius, subdivisions);
	m_geometries["_ge_internal_" + name] = sphere;

	Mesh *sphereMesh = new Mesh{sphere, nullptr};
	m_meshes[name] = sphereMesh;

	return sphereMesh;
}

Texture2D *ResourceManager::createTexture2D(const std::string &name)
{
	if (m_texture2ds.find(name) != m_texture2ds.end()) {
		return nullptr;
	}

	Texture2D *texture = new Texture2D;
	m_texture2ds[name] = texture;

	return texture;
}

Material *ResourceManager::loadCompositorMaterialFromFile(const std::string &name, const std::string &fragmentShaderPath, const StringList &uniforms)
{
	std::string realName = compositorMaterialRealName(name);
	if (m_materials.find(realName) != m_materials.end()) {
		return nullptr;
	}

	std::string realFragmentShaderPath;
	if (!m_assetDirectory.empty()) {
		realFragmentShaderPath = m_assetDirectory + "/" + fragmentShaderPath;
	} else {
		realFragmentShaderPath = fragmentShaderPath;
	}

	std::string fragmentShader{readShaderFile(realFragmentShaderPath)};

	if (fragmentShader.empty()) {
		return nullptr;
	}

	return loadCompositorMaterialFromString(name, fragmentShader, uniforms);
}

Material *ResourceManager::loadCompositorMaterialFromString(const std::string &name, const std::string &fragmentShader, const StringList &uniforms)
{
	std::string realName = compositorMaterialRealName(name);
	if (fragmentShader.empty() || m_materials.find(realName) != m_materials.end()) {
		return nullptr;
	}

	Shader *shader = loadShaderFromStrings("_ge_internal_" + realName, FullscreenQuad::defaultVertexShader(), fragmentShader, uniforms);
	return createMaterial(realName, shader);
}

MeshList ResourceManager::loadMeshListFromFile(const std::string &name, const std::string &fileName)
{
	std::string realName;
	if (!m_assetDirectory.empty()) {
		realName = m_assetDirectory + "/" + fileName;
	} else {
		realName = fileName;
	}

	Assimp::Importer importer;
	const aiScene *scene = importer.ReadFile(realName, aiProcessPreset_TargetRealtime_Fast|aiProcess_PreTransformVertices);
	if (!scene) {
		return MeshList{};
	}

	Shader *defaultShader = shader(kDefaultShaderName);

	Material *materials[scene->mNumMaterials];
	for (size_t i = 0; i < scene->mNumMaterials; ++i) {
		const aiMaterial *materialInfo = scene->mMaterials[i];

		std::stringstream nameConstructor;
		nameConstructor << "_ge_internal_" << name << "_" << i;

		materials[i] = createMaterial(nameConstructor.str(), defaultShader);

		aiColor3D color;
		materialInfo->Get(AI_MATKEY_COLOR_AMBIENT, color);
		// materials[i]->setAmbient(glm::vec3{color.r, color.g, color.b});

		materialInfo->Get(AI_MATKEY_COLOR_DIFFUSE, color);
		// materials[i]->setDiffuse(glm::vec3{color.r, color.g, color.b});

		materialInfo->Get(AI_MATKEY_COLOR_SPECULAR, color);
		// materials[i]->setSpecular(glm::vec3{color.r, color.g, color.b});

		float f;
		materialInfo->Get(AI_MATKEY_SHININESS, f);
		// materials[i]->setShininess(f);

		int texIndex = 0;
		aiReturn texFound = AI_SUCCESS;
		aiString path;
		while (texFound == AI_SUCCESS) {
			texFound = materialInfo->GetTexture(aiTextureType_DIFFUSE, texIndex, &path);
			if (texFound == AI_SUCCESS) {
				std::stringstream nameConstructor;
				nameConstructor << "_ge_internal_mesh_" << name << "_" << texIndex;

				Texture2D *texture = loadTexture2DFromFile(nameConstructor.str(), path.C_Str());

				std::stringstream textureNameConstructor;
				textureNameConstructor << "ge_texture_mesh_" << texIndex;

				materials[i]->setUniform(textureNameConstructor.str(), texture);
			}
			texIndex++;
		}
	}

	MeshList meshes;
	for (size_t i = 0; i < scene->mNumMeshes; ++i) {
		const aiMesh *meshInfo = scene->mMeshes[i];

		std::stringstream nameConstructor;
		nameConstructor << "_ge_internal_" << name << "_" << i;
		std::string meshName = nameConstructor.str();

		Geometry *geometry = createGeometry("_ge_internal_mesh_" + meshName);
		VertexList vertices;
		UVList uvs;
		VertexList normals;
		for (size_t v = 0; v < meshInfo->mNumVertices; ++v) {
			vertices.push_back(glm::vec3{meshInfo->mVertices[v].x, meshInfo->mVertices[v].y, meshInfo->mVertices[v].z});
			if (meshInfo->mTextureCoords[0]) {
				uvs.push_back(glm::vec2{meshInfo->mTextureCoords[0][v].x, meshInfo->mTextureCoords[0][v].y});
			}
			if (meshInfo->mNormals) {
				normals.push_back(glm::vec3{meshInfo->mNormals[v].x, meshInfo->mNormals[v].y, meshInfo->mNormals[v].z});
			}
		}
		IndexList indices;
		for (size_t j = 0; j < meshInfo->mNumFaces; ++j) {
			if (meshInfo->mFaces[j].mNumIndices == 3) {
				indices.push_back(meshInfo->mFaces[j].mIndices[0]);
				indices.push_back(meshInfo->mFaces[j].mIndices[1]);
				indices.push_back(meshInfo->mFaces[j].mIndices[2]);
			} else {
				std::cerr << "Face with more than 3 indices: " << meshInfo->mFaces[j].mNumIndices << std::endl;
			}
		}

		geometry->setVertices(vertices);
		geometry->setIndices(indices);
		if (!uvs.empty()) {
			geometry->setUVs(uvs);
		}
		if (!normals.empty()) {
			geometry->setNormals(normals);
		}

		Mesh *mesh = createMesh(nameConstructor.str(), geometry, materials[meshInfo->mMaterialIndex]);
		mesh->construct();
		meshes.push_back(mesh);
	}

	m_meshLists[name] = meshes;
	return meshes;
}

Shader *ResourceManager::loadShaderFromFiles(const std::string &name, const std::string &vertexShaderPath, const std::string &fragmentShaderPath, const StringList &uniforms)
{
	if (m_shaders.find(name) != m_shaders.end()) {
		return nullptr;
	}

	std::string realVertexShaderPath;
	if (!m_assetDirectory.empty()) {
		realVertexShaderPath = m_assetDirectory + "/" + vertexShaderPath;
	} else {
		realVertexShaderPath = vertexShaderPath;
	}

	std::string realFragmentShaderPath;
	if (!m_assetDirectory.empty()) {
		realFragmentShaderPath = m_assetDirectory + "/" + fragmentShaderPath;
	} else {
		realFragmentShaderPath = fragmentShaderPath;
	}

	std::string vertexShader{readShaderFile(realVertexShaderPath)};
	std::string fragmentShader{readShaderFile(realFragmentShaderPath)};

	if (vertexShader.empty() || fragmentShader.empty()) {
		return nullptr;
	}

	return loadShaderFromStrings(name, vertexShader, fragmentShader, uniforms);
}

Shader *ResourceManager::loadShaderFromStrings(const std::string &name, const std::string &vertexShader, const std::string &fragmentShader, const StringList &uniforms)
{
	if (vertexShader.empty() || fragmentShader.empty() || m_shaders.find(name) != m_shaders.end()) {
		return nullptr;
	}

	Shader *shader = Shader::loadFromString(preprocessShader(vertexShader), preprocessShader(fragmentShader), uniforms);
	if (shader->hasError()) {
		std::cerr << "Shader compilation error" << std::endl;
		std::cerr << shader->errorString() << std::endl;

		delete shader;
		return nullptr;
	}
	m_shaders[name] = shader;

	return shader;
}

Texture2D *ResourceManager::loadTexture2DFromFile(const std::string &name, const std::string &fileName)
{
	std::string realName;
	if (!m_assetDirectory.empty()) {
		realName = m_assetDirectory + "/" + fileName;
	} else {
		realName = fileName;
	}

	if (m_texture2ds.find(name) != m_texture2ds.end()) {
		return nullptr;
	}

	Texture2D *texture = new Texture2D;
	texture->construct(realName);

	m_texture2ds[name] = texture;
	return texture;
}

std::string ResourceManager::preprocessShader(const std::string &shader)
{
	std::stringstream preprocessedShader;

	std::istringstream shaderStream{shader};
	std::list<std::string> shaderLines{std::istream_iterator<Line>{shaderStream}, std::istream_iterator<Line>{}};
	while (!shaderLines.empty()) {
		std::string line = shaderLines.front();
		shaderLines.pop_front();

		std::string shaderPath = parseInclude(line);
		if (!shaderPath.empty()) {
			std::string realShaderPath;
			if (!m_assetDirectory.empty()) {
				realShaderPath = m_assetDirectory + "/" + shaderPath;
			} else {
				realShaderPath = shaderPath;
			}

			std::string includedShaderStr = readShaderFile(realShaderPath);
			if (includedShaderStr.empty()) {
				std::cerr << "Unable to include shader file '" << shaderPath << "'" << std::endl;
				return "";
			}

			std::istringstream includedShader{includedShaderStr};
			shaderLines.insert(shaderLines.begin(), std::istream_iterator<Line>{includedShader}, std::istream_iterator<Line>{});
		} else {
			preprocessedShader << line << std::endl;
		}
	}

	return preprocessedShader.str();
}
