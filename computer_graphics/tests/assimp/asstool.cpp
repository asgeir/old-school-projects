#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <iostream>
#include <string>

void printNode(const aiNode *node, int indentLevel = 0)
{
	if (!node) {
		return;
	}

	for (int i = 0; i < indentLevel; ++i) {
		std::cout << "\t";
	}
	std::cout << node->mName.C_Str();

	if (node->mNumMeshes) {
		std::cout << " (";

		for (int i = 0; i < node->mNumMeshes; ++i) {
			std::cout << node->mMeshes[i];
			if (i < node->mNumMeshes-1) {
				std::cout << ", ";
			}
		}

		std::cout << ")";
	}
	std::cout << std::endl;

	for (int i = 0; i < node->mNumChildren; ++i) {
		printNode(node->mChildren[i], indentLevel + 1);
	}
}

void printMeshes(const aiScene *scene)
{
	for (int i = 0; i < scene->mNumMeshes; ++i) {
		const aiMesh *mesh = scene->mMeshes[i];
		std::cout << i << " -> " << "vertices: " << mesh->mNumVertices <<
			"; faces: " << mesh->mNumFaces << "; material: " << mesh->mMaterialIndex << std::endl;
	}
}

void printMaterials(const aiScene *scene)
{
	for (int i = 0; i < scene->mNumMaterials; ++i) {
		const aiMaterial *material = scene->mMaterials[i];
		std::cout << i << " -> " << std::endl;

		aiString name;
		material->Get(AI_MATKEY_NAME, name);
		std::cout << "\tname: " << name.C_Str() << std::endl;

		aiColor3D color;
		material->Get(AI_MATKEY_COLOR_AMBIENT, color);
		std::cout << "\tambient: " << color.r << ", " << color.g << ", " << color.b << std::endl;

		material->Get(AI_MATKEY_COLOR_DIFFUSE, color);
		std::cout << "\tdiffuse: " << color.r << ", " << color.g << ", " << color.b << std::endl;

		material->Get(AI_MATKEY_COLOR_SPECULAR, color);
		std::cout << "\tspecular: " << color.r << ", " << color.g << ", " << color.b << std::endl;

		float f;
		material->Get(AI_MATKEY_SHININESS, f);
		std::cout << "\tshinyness: " << f << std::endl;

		int texIndex = 0;
		aiReturn texFound = AI_SUCCESS;
		aiString path;
		while (texFound == AI_SUCCESS) {
			texFound = material->GetTexture(aiTextureType_DIFFUSE, texIndex, &path);
			std::cout << "\ttexture[" << texIndex << "]: " << path.C_Str() << std::endl;
			texIndex++;
		}
	}
}

int printMesh(const char *fileName)
{
	Assimp::Importer importer;
	const aiScene *scene = importer.ReadFile(fileName, aiProcessPreset_TargetRealtime_Fast|aiProcess_PreTransformVertices);
	if (!scene) {
		std::cerr << "Unable to open file: " << fileName << std::endl;
		return -1;
	}

	std::cout << "Scene:" << std::endl;

	printNode(scene->mRootNode);

	std::cout << std::endl << "Meshes:" << std::endl;

	printMeshes(scene);

	std::cout << std::endl << "Materials:" << std::endl;

	printMaterials(scene);

	return 0;
}

int main(int argc, char *argv[])
{
	if (argc > 1) {
		std::cout << "Printing file: " << argv[1] << std::endl;
		return printMesh(argv[1]);
	}

	return -1;
}
