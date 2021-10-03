#include "Utils.hpp"
#include <iostream>
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

int read_file(const char*  file_path, char* file_content)
{
	std::ifstream _file;
	_file.open(file_path, std::fstream::in);
	std::stringstream _file_stream;
	_file_stream << _file.rdbuf();

	const std::string& str = _file_stream.str();
	if (str.length() == 0)
		return 0;
	const char* c_str = str.c_str();
	std::memcpy(file_content, c_str, str.size() + 1);
	return 1;

}

Vec3f calculate_tangent_in_tangent_space(Vec3f pos1, Vec3f pos2, Vec3f pos3, Vec2f uv1, Vec2f uv2, Vec2f uv3)
{
	Vec3f tangent = {};
	Float_32 d_u1 = uv2.x - uv1.x;
	Float_32 d_u2 = uv3.x - uv1.x;

	Float_32 d_v1 = uv2.y - uv1.y;
	Float_32 d_v2 = uv3.y - uv1.y;

	Float_32 det = (d_u1*d_v2 - d_u2 * d_v1);
	if (abs(det) <= EPS)	// determinant almost 0, can't calculate inverse
		return tangent;

	Float_32 det_inv = 1.f / det;

	Vec3f edge_1 = pos2 - pos1;
	Vec3f edge_2 = pos3 - pos1;

	tangent.x = det_inv * (d_v2 * edge_1.x - d_v1 * edge_2.x);
	tangent.y = det_inv * (d_v2 * edge_1.y - d_v1 * edge_2.y);
	tangent.z = det_inv * (d_v2 * edge_1.z - d_v1 * edge_2.z);
	return tangent.GetNormalized();

}


// vertices contain flattened Vertex data ( see Vertex struct for layout )
bool load_model(const std::string& pFile, std::vector<Float_32>& vertices, std::vector<Int_32> &indices, Float_32 import_scale) {
	// Create an instance of the Importer class
	Assimp::Importer importer;

	// And have it read the given file with some example postprocessing
	// Usually - if speed is not the most important aspect for you - you'll
	// probably to request more postprocessing than we do in this example.
	const aiScene* scene = importer.ReadFile(pFile,
		aiProcess_CalcTangentSpace |
		aiProcess_Triangulate |
		aiProcess_JoinIdenticalVertices |
		aiProcess_SortByPType);

	// If the import failed, report it
	if (nullptr == scene) {
		std::cout << importer.GetErrorString();
		return false;
	}

	for (int i = 0; i < scene->mNumMeshes; i++)
	{
		aiMesh* _mesh = scene->mMeshes[i];
		if (_mesh->HasFaces() == false)
		{
			std::cout << "Mesh "<<pFile<<":"<<i<<" , doesn't have faces, skipping" << std::endl;
		}
		else
		{
			for (int j = 0; j < _mesh->mNumFaces; j++)
			{
				aiFace _face = _mesh->mFaces[j];
				for (int k = 0; k < _face.mNumIndices; k++)
				{
					indices.push_back(_face.mIndices[k]);
				}
			}

			for (int j = 0; j < _mesh->mNumVertices; j++)
			{

				if (_mesh->HasPositions())
				{
					aiVector3D pos = _mesh->mVertices[j];
					vertices.push_back(pos.x * import_scale);
					vertices.push_back(pos.y * import_scale);
					vertices.push_back(pos.z * import_scale);
				}
				else
				{
					assert(0);
				}

				if (_mesh->HasVertexColors(0))
				{
					aiColor4D col = _mesh->mColors[0][j];
					vertices.push_back(col.r);
					vertices.push_back(col.g);
					vertices.push_back(col.b);
				}
				else
				{
					vertices.push_back(0);
					vertices.push_back(0);
					vertices.push_back(0);
				}

				if (_mesh->HasNormals())
				{
					aiVector3D normal = _mesh->mNormals[j];
					vertices.push_back(normal.x);
					vertices.push_back(normal.y);
					vertices.push_back(normal.z);
				}
				else
				{
					vertices.push_back(0);
					vertices.push_back(0);
					vertices.push_back(0);
				}

				if (_mesh->HasTextureCoords(0))
				{
					aiVector3D tex_co = _mesh->mTextureCoords[0][j];
					vertices.push_back(tex_co.x);
					vertices.push_back(tex_co.y);
				}
				else
				{
					vertices.push_back(0);
					vertices.push_back(0);
				}

				if (_mesh->HasTangentsAndBitangents())
				{
					aiVector3D tangent = _mesh->mTangents[j];
					vertices.push_back(tangent.x);
					vertices.push_back(tangent.y);
					vertices.push_back(tangent.z);
				}
				else
				{
					vertices.push_back(0);
					vertices.push_back(0);
					vertices.push_back(0);
				}
			}
		}
	}

	std::cout << scene->mNumMeshes;
	// Now we can access the file's contents.
	//DoTheSceneProcessing(scene);

	// We're done. Everything will be cleaned up by the importer destructor
	return true;
}

//
//void load_obj(const char* obj_name, std::vector<Float_32>& vertices, std::vector < std::vector<Int_32 >> &indices, Float_32 import_scale)
//{
//	tinyobj::attrib_t attrib;
//	std::vector<tinyobj::shape_t> shapes;
//	std::vector<tinyobj::material_t> materials;
//	std::string warn;
//	std::string err;
//
//	bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, obj_name,
//		NULL, true);
//
//	if (ret)
//		printf("Model loaded\n");
//	else
//		printf("Model loading failed\n");
//
//	if (ret)
//	{
//		printf("Vertices %d\n", attrib.vertices.size() / 3);
//		printf("Colors %d\n", attrib.colors.size() / 3);
//		printf("Normals %d\n", attrib.normals.size() / 3);
//		printf("Texcos %d\n", attrib.texcoords.size() / 2);
//		printf("Shapes %d\n", shapes.size());
//		printf("Materials %d\n", materials.size());
//	}
//	if (!warn.empty())
//		printf("Warning: %d\n", warn.c_str());
//	else if (!err.empty())
//		printf("Error: %d\n", err.c_str());
//
//
//	// objs don't have concept of indices explicitely
//	// easiest way to handle is to add every vertex in face to vertices array 
//	// and add a new index for the vertex. This duplicates the vertex and doesn't take advantage of index buffer
//	// but it's simple
//	int indice = 0;
//	// Loop over shapes
//	for (size_t s = 0; s < shapes.size(); s++) {
//		std::vector<int> shape_indices;
//
//
//
//		// Loop over faces(polygon)
//		size_t index_offset = 0;
//		for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
//			size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);
//
//			//for calculating tangent
//			Vec3f pos[3];
//			Vec2f uv[3];
//
//			// Loop over vertices in the face. first pass to calculate tangent
//			for (size_t v = 0; v < fv; v++) {
//				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
//				pos[v].x = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
//				pos[v].y = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
//				pos[v].z = attrib.vertices[3 * size_t(idx.vertex_index) + 2];
//
//				uv[v].x = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
//				uv[v].y = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
//			}
//			Vec3f tangent = calculate_tangent_in_tangent_space(pos[0], pos[1], pos[2], uv[0], uv[1], uv[2]);
//
//			// 2nd pass
//			// Loop over vertices in the face.
//			for (size_t v = 0; v < fv; v++) {
//				// access to vertex
//				tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
//				tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
//				tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
//				tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];
//				vertices.push_back(vx*import_scale);
//				vertices.push_back(vy*import_scale);
//				vertices.push_back(vz*import_scale);
//
//				tinyobj::real_t red = attrib.colors[3 * size_t(idx.vertex_index) + 0];
//				tinyobj::real_t green = attrib.colors[3 * size_t(idx.vertex_index) + 1];
//				tinyobj::real_t blue = attrib.colors[3 * size_t(idx.vertex_index) + 2];
//				vertices.push_back(red);
//				vertices.push_back(green);
//				vertices.push_back(blue);
//
//				// Check if `normal_index` is zero or positive. negative = no normal data
//				if (idx.normal_index >= 0) {
//					tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
//					tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
//					tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
//					vertices.push_back(nx);
//					vertices.push_back(ny);
//					vertices.push_back(nz);
//				}
//				else
//				{
//					vertices.push_back(0);
//					vertices.push_back(0);
//					vertices.push_back(0);
//				}
//
//				// Check if `texcoord_index` is zero or positive. negative = no texcoord data
//				if (idx.texcoord_index >= 0) {
//					tinyobj::real_t tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
//					tinyobj::real_t ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
//					vertices.push_back(tx);
//					vertices.push_back(ty);
//
//				}
//				else
//				{
//					vertices.push_back(0);
//					vertices.push_back(0);
//				}
//
//				vertices.push_back(tangent.x);
//				vertices.push_back(tangent.y);
//				vertices.push_back(tangent.z);
//
//				shape_indices.push_back(indice);
//				indice++;
//			}
//			index_offset += fv;
//
//
//
//
//			// per-face material
//			shapes[s].mesh.material_ids[f];
//		}
//		indices.push_back(shape_indices);
//
//	}
//
//}
//

Float_32 get_rand(float min, float max)
{
	// add random deviations for more interesting behaviour
	return (((rand() % 100) / 100.f) * (max-min)) + min; // random between -1.f and 1.f
}

Float_32 clamp(Float_32 val, Float_32 min, Float_32 max)
{
	if (val < min) return min;
	else if (val > max) return max;
	else return val;
}


Vec3f lerp(const Vec3f& s, const Vec3f& e, float t)
{
	return s * t + e * (1.f - t);
}

Vec4f lerp(const Vec4f& s, const Vec4f& e, float t)
{
	return s * t + e * (1.f - t);
}
