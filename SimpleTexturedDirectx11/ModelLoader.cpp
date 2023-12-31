#include "ModelLoader.h"

#define _EXPORT_EMBEDDED_TEXTURES  1

ModelLoader::ModelLoader() :
        dev_(nullptr),
        devcon_(nullptr),
        meshes_(),
        directory_(),
        textures_loaded_(),
        hwnd_(nullptr) {
    // empty
}


ModelLoader::~ModelLoader() {
    // empty
}

bool ModelLoader::Load(HWND hwnd, ID3D11Device * dev, ID3D11DeviceContext * devcon, std::string filename) {
	Assimp::Importer importer;

	const aiScene* pScene = importer.ReadFile(filename,
		aiProcess_Triangulate |
		aiProcess_ConvertToLeftHanded);

	if (pScene == nullptr)
		return false;

	this->directory_ = filename.substr(0, filename.find_last_of("/\\"));

	this->dev_ = dev;
	this->devcon_ = devcon;
	this->hwnd_ = hwnd;

	processNode(pScene->mRootNode, pScene);

	return true;
}

void ModelLoader::Draw(ID3D11DeviceContext * devcon) {
	for (size_t i = 0; i < meshes_.size(); ++i ) {
		meshes_[i].Draw(devcon);
	}
}

Mesh ModelLoader::processMesh(aiMesh * mesh, const aiScene * scene) {
	// Data to fill
	std::vector<VERTEX> vertices;
	std::vector<UINT> indices;
	std::vector<Texture> textures;

	// Walk through each of the mesh's vertices
	for (UINT i = 0; i < mesh->mNumVertices; i++) {
		VERTEX vertex;

		vertex.X = mesh->mVertices[i].x;
		vertex.Y = mesh->mVertices[i].y;
		vertex.Z = mesh->mVertices[i].z;

		if (mesh->mTextureCoords[0]) {
			vertex.texcoord.x = (float)mesh->mTextureCoords[0][i].x;
			vertex.texcoord.y = (float)mesh->mTextureCoords[0][i].y;
		}

		vertices.push_back(vertex);
	}

	for (UINT i = 0; i < mesh->mNumFaces; i++) {
		aiFace face = mesh->mFaces[i];

		for (UINT j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	if (mesh->mMaterialIndex >= 0) {
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

		// 텍스처 모든 종류 다 처리해서 내장된 텍스처 저장하게 한다. 셰이더는 구현하지 않았으므로 맵핑소스를 전부 사용하지는 않음
		for (UINT typeIndex = aiTextureType_DIFFUSE; typeIndex <= (UINT)aiTextureType_AMBIENT_OCCLUSION; typeIndex++)
		{
			std::vector<Texture> diffuseMaps = this->loadMaterialTextures(material, (aiTextureType)typeIndex, "texture_diffuse", scene);
			textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
		}
	}

	return Mesh(dev_, vertices, indices, textures);
}

std::vector<Texture> ModelLoader::loadMaterialTextures(aiMaterial * mat, aiTextureType type, std::string typeName, const aiScene * scene) {
	std::vector<Texture> textures;
	// aiMaterial에는 특정타입의 texture가 여러개 있을수 있다. 
	// 그 개수를 얻고 인덱스로 접근해서 텍스처 경로를 얻는다.
	for (UINT i = 0; i < mat->GetTextureCount(type); i++) 
	{
		aiString str;
		
		mat->GetTexture(type, i, &str);	
		// 중복 로딩 방지 코드 제거 

		HRESULT hr;
		Texture texture;
		std::wstring wsDirectory_(directory_.length(), 0);
		MultiByteToWideChar(CP_UTF8, 0, directory_.c_str(), -1, &wsDirectory_[0], directory_.length());

		const aiTexture* embeddedTexture = scene->GetEmbeddedTexture(str.C_Str());
		if (embeddedTexture != nullptr)
		{
			//texture.texture = loadEmbeddedTexture(embeddedTexture);	//메모리로 부터 텍스처 생성을 변경했다.
			bool result = saveEmbeddedTexture(embeddedTexture);
			assert(result == true);
		}
		std::filesystem::path path = std::string(str.C_Str());
		std::wstring filenamews = wsDirectory_ + L"/" + path.filename().wstring();
		hr = CreateWICTextureFromFile(dev_, devcon_, filenamews.c_str(), nullptr, &texture.texture); //wstring
		if (FAILED(hr))
			MessageBox(hwnd_, "Texture couldn't be loaded", "Error!", MB_ICONERROR | MB_OK);

		texture.type = typeName;
		texture.path = str.C_Str();
		textures.push_back(texture);
		// 중복 로딩 방지 코드 제거
	}
	return textures;
}

void ModelLoader::Close() {
	for (auto& t : textures_loaded_)
		t.Release();

	for (size_t i = 0; i < meshes_.size(); i++) {
		meshes_[i].Close();
	}
}

void ModelLoader::processNode(aiNode * node, const aiScene * scene) {
	for (UINT i = 0; i < node->mNumMeshes; i++) {
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		meshes_.push_back(this->processMesh(mesh, scene));
	}

	for (UINT i = 0; i < node->mNumChildren; i++) {
		this->processNode(node->mChildren[i], scene);
	}
}

ID3D11ShaderResourceView * ModelLoader::loadEmbeddedTexture(const aiTexture* embeddedTexture) {
	HRESULT hr;
	ID3D11ShaderResourceView *texture = nullptr;

	if (embeddedTexture->mHeight != 0) {
		// Load an uncompressed ARGB8888 embedded texture
		D3D11_TEXTURE2D_DESC desc;
		desc.Width = embeddedTexture->mWidth;
		desc.Height = embeddedTexture->mHeight;
		desc.MipLevels = 1;
		desc.ArraySize = 1;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
		desc.CPUAccessFlags = 0;
		desc.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA subresourceData;
		subresourceData.pSysMem = embeddedTexture->pcData;
		subresourceData.SysMemPitch = embeddedTexture->mWidth * 4;
		subresourceData.SysMemSlicePitch = embeddedTexture->mWidth * embeddedTexture->mHeight * 4;

		ID3D11Texture2D *texture2D = nullptr;
		hr = dev_->CreateTexture2D(&desc, &subresourceData, &texture2D);
		if (FAILED(hr))
			MessageBox(hwnd_, "CreateTexture2D failed!", "Error!", MB_ICONERROR | MB_OK);

		hr = dev_->CreateShaderResourceView(texture2D, nullptr, &texture);
		if (FAILED(hr))
			MessageBox(hwnd_, "CreateShaderResourceView failed!", "Error!", MB_ICONERROR | MB_OK);

		return texture;
	}

	// mHeight is 0, so try to load a compressed texture of mWidth bytes
	const size_t size = embeddedTexture->mWidth;

	hr = CreateWICTextureFromMemory(dev_, devcon_, reinterpret_cast<const unsigned char*>(embeddedTexture->pcData), size, nullptr, &texture);
	if (FAILED(hr))
		MessageBox(hwnd_, "Texture couldn't be created from memory!", "Error!", MB_ICONERROR | MB_OK);

	return texture;
}

bool ModelLoader::saveEmbeddedTexture(const aiTexture* embeddedTexture)
{
	std::filesystem::path path = embeddedTexture->mFilename.C_Str();
	std::string filename = directory_ + "/" + path.filename().string();

	if (embeddedTexture->mHeight == 0) 
	{
		// Save a compressed texture of mWidth bytes
		std::ofstream file(filename.c_str(), std::ios::binary);
		file.write(reinterpret_cast<const char*>(embeddedTexture->pcData), embeddedTexture->mWidth);
		file.close();
	}
	else
	{
		// Save an uncompressed ARGB8888 embedded texture
		std::ofstream file(filename.c_str(), std::ios::binary);
		file.write(reinterpret_cast<const char*>(embeddedTexture->pcData), embeddedTexture->mWidth * embeddedTexture->mHeight * 4);
		file.close();
	}
	return true;
}
