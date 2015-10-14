// TAGRELEASE: PUBLIC

#include "NvModel/NvModel.h"

//fix for non-standard naming
#ifdef WIN32
#define strcasecmp _stricmp
#endif

using std::vector;

//
//
////////////////////////////////////////////////////////////
bool NvModel::hasNormals() const {
    return _normals.size() > 0;
}

//
//
////////////////////////////////////////////////////////////
bool NvModel::hasTexCoords() const {
    return _texCoords.size() > 0;
}

//
//
////////////////////////////////////////////////////////////
bool NvModel::hasTangents() const {
    return _sTangents.size() > 0;
}

//
//
////////////////////////////////////////////////////////////
bool NvModel::hasColors() const {
    return _colors.size() > 0;
}

//
//
////////////////////////////////////////////////////////////
int32_t NvModel::getPositionSize() const {
    return _posSize;
}

//
//
////////////////////////////////////////////////////////////
int32_t NvModel::getNormalSize() const {
    return 3;
}

//
//
////////////////////////////////////////////////////////////
int32_t NvModel::getTexCoordSize() const {
    return _tcSize;
}

//
//
////////////////////////////////////////////////////////////
int32_t NvModel::getTangentSize() const {
    return 3;
}

//
//
////////////////////////////////////////////////////////////
int32_t NvModel::getColorSize() const {
    return _cSize;
}


//raw data access functions
//  These are to be used to get the raw array data from the file, each array has its own index

//
//
////////////////////////////////////////////////////////////
const float* NvModel::getPositions() const {
    return ( _positions.size() > 0) ? &(_positions[0]) : 0;
}

//
//
////////////////////////////////////////////////////////////
const float* NvModel::getNormals() const {
    return ( _normals.size() > 0) ? &(_normals[0]) : 0;
}

//
//
////////////////////////////////////////////////////////////
const float* NvModel::getTexCoords() const {
    return ( _texCoords.size() > 0) ? &(_texCoords[0]) : 0;
}

//
//
////////////////////////////////////////////////////////////
const float* NvModel::getTangents() const {
    return ( _sTangents.size() > 0) ? &(_sTangents[0]) : 0;
}

//
//
////////////////////////////////////////////////////////////
const float* NvModel::getColors() const {
    return ( _colors.size() > 0) ? &(_colors[0]) : 0;
}

//
//
////////////////////////////////////////////////////////////
const uint32_t* NvModel::getPositionIndices() const {
    return ( _pIndex.size() > 0) ? &(_pIndex[0]) : 0;
}

//
//
////////////////////////////////////////////////////////////
const uint32_t* NvModel::getNormalIndices() const {
    return ( _nIndex.size() > 0) ? &(_nIndex[0]) : 0;
}

//
//
////////////////////////////////////////////////////////////
const uint32_t* NvModel::getTexCoordIndices() const {
    return ( _tIndex.size() > 0) ? &(_tIndex[0]) : 0;
}

//
//
////////////////////////////////////////////////////////////
const uint32_t* NvModel::getTangentIndices() const {
    return ( _tanIndex.size() > 0) ? &(_tanIndex[0]) : 0;
}

//
//
////////////////////////////////////////////////////////////
const uint32_t* NvModel::getColorIndices() const {
    return ( _cIndex.size() > 0) ? &(_cIndex[0]) : 0;
}

//
//
////////////////////////////////////////////////////////////
int32_t NvModel::getPositionCount() const {
    return (_posSize > 0) ? (int32_t)_positions.size() / _posSize : 0;
}

//
//
////////////////////////////////////////////////////////////
int32_t NvModel::getNormalCount() const {
    return (int32_t)_normals.size() / 3;
}

//
//
////////////////////////////////////////////////////////////
int32_t NvModel::getTexCoordCount() const {
    return (_tcSize > 0) ? (int32_t)_texCoords.size() / _tcSize : 0;
}

//
//
////////////////////////////////////////////////////////////
int32_t NvModel::getTangentCount() const {
    return (int32_t)_sTangents.size() / 3;
}

//
//
////////////////////////////////////////////////////////////
int32_t NvModel::getColorCount() const {
    return (_cSize > 0) ? (int32_t)_colors.size() / _cSize : 0;
}

//
//
////////////////////////////////////////////////////////////
int32_t NvModel::getIndexCount() const {
    return (int32_t)_pIndex.size();
}

//compiled data access functions

//
//
////////////////////////////////////////////////////////////
const float* NvModel::getCompiledVertices() const {
    return (_vertices.size() > 0) ? &_vertices[0] : 0;
}

//
//
////////////////////////////////////////////////////////////
const uint32_t* NvModel::getCompiledIndices( NvModelPrimType::Enum prim) const {
    switch (prim) {
        case NvModelPrimType::POINTS:
            return (_indices[0].size() > 0) ? &_indices[0][0] : 0;
        case NvModelPrimType::EDGES:
            return (_indices[1].size() > 0) ? &_indices[1][0] : 0;
        case NvModelPrimType::TRIANGLES:
            return (_indices[2].size() > 0) ? &_indices[2][0] : 0;
        case NvModelPrimType::TRIANGLES_WITH_ADJACENCY:
            return (_indices[3].size() > 0) ? &_indices[3][0] : 0;
    }

    return 0; 
}

//
//
////////////////////////////////////////////////////////////
int32_t NvModel::getCompiledPositionOffset() const {
    return _pOffset;
}

//
//
////////////////////////////////////////////////////////////
int32_t NvModel::getCompiledNormalOffset() const {
    return _nOffset;
}

//
//
////////////////////////////////////////////////////////////
int32_t NvModel::getCompiledTexCoordOffset() const {
    return _tcOffset;
}

//
//
////////////////////////////////////////////////////////////
int32_t NvModel::getCompiledTangentOffset() const {
    return _sTanOffset;
}

//
//
////////////////////////////////////////////////////////////
int32_t NvModel::getCompiledColorOffset() const {
    return _cOffset;
}

// returns the size of the merged vertex in # of floats
//
//
////////////////////////////////////////////////////////////
int32_t NvModel::getCompiledVertexSize() const {
    return _vtxSize;
}

//
//
////////////////////////////////////////////////////////////
int32_t NvModel::getCompiledVertexCount() const {
    return (_vtxSize > 0) ? (int32_t)_vertices.size() / _vtxSize : 0;
}

//
//
////////////////////////////////////////////////////////////
int32_t NvModel::getCompiledIndexCount( NvModelPrimType::Enum prim) const {
    switch (prim) {
        case NvModelPrimType::POINTS:
            return (int32_t)_indices[0].size();
        case NvModelPrimType::EDGES:
            return (int32_t)_indices[1].size();
        case NvModelPrimType::TRIANGLES:
            return (int32_t)_indices[2].size();
        case NvModelPrimType::TRIANGLES_WITH_ADJACENCY:
            return (int32_t)_indices[3].size();
    }

    return 0;
}

//
//
////////////////////////////////////////////////////////////
int32_t NvModel::getOpenEdgeCount() const {
    return _openEdges;
}
