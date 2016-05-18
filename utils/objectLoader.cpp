/*
 * @brief obejctLoader that implements object load interface using assimp
 *
 * @author alei  mailto:rayingecho@hotmail.com
 */
#include "objectLoader.hpp"
#include <vector>
#include <stdio.h>
#include <string>
#include <glm/glm.hpp>

#include <assimp/Importer.hpp>
#include <assimp/scene.h> 
#include <assimp/postprocess.h> 



ObjectLoader& ObjectLoader::loadObj(std::string objPath) {
    ObjectLoader* loader = new ObjectLoader();
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(objPath, 0);
    if (!scene) {
        throw std::runtime_error("ObjectLoader - loadObj, cannot load object" + objPath);
    }
    const aiMesh* mesh = scene->mMeshes[0]; 

    loader->_vertices = std::vector<glm::vec3>();
    loader->_vertices.reserve(mesh->mNumVertices);
    for (unsigned int i = 0; i<mesh->mNumVertices; i++) {
        aiVector3D pos = mesh->mVertices[i];
        loader->_vertices.push_back(glm::vec3(pos.x, pos.y, pos.z));
    }

    loader->_uvs = std::vector<glm::vec2>();
    loader->_uvs.reserve(mesh->mNumVertices);
    for (unsigned int i = 0; i<mesh->mNumVertices; i++) {
        aiVector3D UVW = mesh->mTextureCoords[0][i];
        loader->_uvs.push_back(glm::vec2(UVW.x, UVW.y));
    }

    loader->_normals = std::vector<glm::vec3>();
    loader->_normals.reserve(mesh->mNumVertices);
    for (unsigned int i = 0; i<mesh->mNumVertices; i++) {
        aiVector3D n = mesh->mNormals[i];
        loader->_normals.push_back(glm::vec3(n.x, n.y, n.z));
    }


    loader->_indices = std::vector<unsigned short>();
    loader->_indices.reserve(3 * mesh->mNumFaces);
    for (unsigned int i = 0; i<mesh->mNumFaces; i++) {
        // Assume the model has only triangles.
        loader->_indices.push_back(mesh->mFaces[i].mIndices[0]);
        loader->_indices.push_back(mesh->mFaces[i].mIndices[1]);
        loader->_indices.push_back(mesh->mFaces[i].mIndices[2]);
    }
    return *loader;
}

std::vector<unsigned short>& ObjectLoader::getIndices() {
    return this->_indices;
}

std::vector<glm::vec3>& ObjectLoader::getVertices() {
    return this->_vertices;
}

std::vector<glm::vec2>& ObjectLoader::getUvs() {
    return this->_uvs;
}

std::vector<glm::vec2>& ObjectLoader::getReversedUvs() {
    std::vector<glm::vec2> ret = std::vector<glm::vec2>();
    ret.reserve(_uvs.size());
    for (glm::vec2 uv : _uvs) {
        ret.push_back(glm::vec2(uv.x, 1.0 - uv.y));
    }
    return ret;
}

std::vector<glm::vec3>& ObjectLoader::getNormals() {
    return this->_normals;
}
