#include "Bulb/ModelLoader.hpp"

#include "Bulb/Rendering/Renderables/Mesh.hpp"

#include <Clove/Graphics/Texture.hpp>
#include <Clove/Graphics/VertexLayout.hpp>
#include <Clove/Platform/Window.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

using namespace clv;

namespace blb::ModelLoader {
	static mth::mat4f convertToGarlicMatrix(const aiMatrix4x4& aiMat){
        mth::mat4f garlicMat;

        garlicMat[0][0] = aiMat.a1;
        garlicMat[1][0] = aiMat.b1;
        garlicMat[2][0] = aiMat.c1;
        garlicMat[3][0] = aiMat.d1;

        garlicMat[0][1] = aiMat.a2;
        garlicMat[1][1] = aiMat.b2;
        garlicMat[2][1] = aiMat.c2;
        garlicMat[3][1] = aiMat.d2;

        garlicMat[0][2] = aiMat.a3;
        garlicMat[1][2] = aiMat.b3;
        garlicMat[2][2] = aiMat.c3;
        garlicMat[3][2] = aiMat.d3;

        garlicMat[0][3] = aiMat.a4;
        garlicMat[1][3] = aiMat.b4;
        garlicMat[2][3] = aiMat.c4;
        garlicMat[3][3] = aiMat.d4;

        return garlicMat;
	}

	static mth::vec3f convertToGarlicVec(const aiVector3D& aiVec){
        return { aiVec.x, aiVec.y, aiVec.z };
	}

	static mth::quatf convertToGarlicQuat(const aiQuaternion& aiQuat){
        return { aiQuat.w, aiQuat.x, aiQuat.y, aiQuat.z };
	}

	static void buildNodeNameMap(std::unordered_map<std::string_view, aiNode*>& map, aiNode* rootNode){
        map[rootNode->mName.C_Str()] = rootNode;
        for(size_t i = 0; i < rootNode->mNumChildren; ++i) {
            buildNodeNameMap(map, rootNode->mChildren[i]);
		}
	}

	static std::optional<rnd::JointIndexType> getJointParentId(const rnd::Skeleton& skeleton, std::string_view jointName, std::unordered_map<std::string_view, aiNode*>& nodeNameMap) {
        aiNode* jointNode = nodeNameMap[jointName];
        if(jointNode->mParent == nullptr) {
            return {};
        }

		aiString* parentName = &jointNode->mParent->mName;
        for(size_t i = 0; i < skeleton.joints.size(); ++i) {
            if(skeleton.joints[i].name == parentName->C_Str()) {
                return i;
			}
		}
		GARLIC_LOG(garlicLogContext, Log::Level::Warning, "{0}| Joint: {1} has a parent but it couldn't be found in the skeleton", CLV_FUNCTION_NAME_PRETTY, jointName);

		return {};
	}

	static rnd::JointIndexType getJointIndex(const rnd::Skeleton& skeleton, std::string_view jointName){
        for(size_t i = 0; i < skeleton.joints.size(); ++i) {
            if(skeleton.joints[i].name == jointName) {
                return i;
            }
        }
		//TODO: Error
        return std::numeric_limits<rnd::JointIndexType>::max();
	}

	static std::shared_ptr<gfx::Texture> loadMaterialTexture(aiMaterial* material, aiTextureType type, const std::shared_ptr<clv::gfx::GraphicsFactory>& graphicsFactory) {
		std::shared_ptr<gfx::Texture> texture;

		//TODO: Support multiple textures of the same type
		if(material->GetTextureCount(type) > 0) {
			aiString path;
			material->GetTexture(type, 0, &path);

			gfx::TextureDescriptor descriptor{};
			texture = graphicsFactory->createTexture(descriptor, path.C_Str());
		}

		return texture;
	}

	enum class MeshType{
		Default,
		Animated
	};
	static std::shared_ptr<rnd::Mesh> processMesh(aiMesh* mesh, const aiScene* scene, const std::shared_ptr<clv::gfx::GraphicsFactory>& graphicsFactory, const MeshType meshType) {
		gfx::VertexLayout layout;
		if(mesh->HasPositions()) {
			layout.add(gfx::VertexElementType::position3D);
		}
		if(mesh->HasNormals()) {
			layout.add(gfx::VertexElementType::normal);
		}
		//if(mesh->HasTextureCoords(0)) {
		layout.add(gfx::VertexElementType::texture2D);
		//}
        if(meshType == MeshType::Animated) {
            layout.add(gfx::VertexElementType::jointIds);
            layout.add(gfx::VertexElementType::weights);
		}
		//Skipping colours for now
		/*if(mesh->HasVertexColors(0)) {
			layout.add(gfx::VertexElementType::colour3D);
		}*/

		gfx::VertexBufferData vertexBufferData{ layout };
		std::vector<uint32_t> indices;
		std::shared_ptr<rnd::Material> meshMaterial = std::make_shared<rnd::Material>(graphicsFactory);

		const size_t vertexCount = mesh->mNumVertices;
		vertexBufferData.resize(vertexCount);

		//Build the map of jointIds + weights for each vertex
        std::unordered_map<size_t, std::vector<std::pair<rnd::JointIndexType, float>>> vertWeightPairs;
		if(meshType == MeshType::Animated) {
            for(rnd::JointIndexType i = 0; i < mesh->mNumBones; ++i) {
                aiBone* bone = mesh->mBones[i];
                for(size_t j = 0; j < bone->mNumWeights; ++j) {
                    const aiVertexWeight& vertexWeight = bone->mWeights[j];
                    vertWeightPairs[vertexWeight.mVertexId].emplace_back(i, vertexWeight.mWeight);
                }
            }
        }

		for(size_t i = 0; i < vertexCount; ++i) {
			if(mesh->HasPositions()) {
				vertexBufferData[i].getAttribute<gfx::VertexElementType::position3D>() = {
					mesh->mVertices[i].x,
					mesh->mVertices[i].y,
					mesh->mVertices[i].z
				};
			}
			if(mesh->HasNormals()) {
				vertexBufferData[i].getAttribute<gfx::VertexElementType::normal>() = {
					mesh->mNormals[i].x,
					mesh->mNormals[i].y,
					mesh->mNormals[i].z
				};
			}
			if(mesh->HasTextureCoords(0)) {
				vertexBufferData[i].getAttribute<gfx::VertexElementType::texture2D>() = {
					mesh->mTextureCoords[0][i].x,
					mesh->mTextureCoords[0][i].y
				};
			} else {
				vertexBufferData[i].getAttribute<gfx::VertexElementType::texture2D>() = {
					0.0f,
					0.0f
				};
			}
            if(meshType == MeshType::Animated) {
                mth::vec4i jointIds = vertexBufferData[i].getAttribute<gfx::VertexElementType::jointIds>();
                mth::vec4f weights  = vertexBufferData[i].getAttribute<gfx::VertexElementType::weights>();
                std::vector<std::pair<rnd::JointIndexType, float>> weightPairs = vertWeightPairs[i];

                for(size_t j = 0; j < 4; ++j){
                    if(j < weightPairs.size()) {
                        jointIds[j] = weightPairs[j].first;
                        weights[j]  = weightPairs[j].second;
                    } else {
                        jointIds[j] = 0;
                        weights[j]  = 0.0f;
					}
				}
			}
		}

		for(size_t i = 0; i < mesh->mNumFaces; ++i) {
			aiFace face = mesh->mFaces[i];
			for(size_t j = 0; j < face.mNumIndices; ++j) {
				indices.emplace_back(face.mIndices[j]);
			}
		}

		if(mesh->mMaterialIndex >= 0) {
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

			auto diffuseTexture = loadMaterialTexture(material, aiTextureType_DIFFUSE, graphicsFactory);
			auto specularTexture = loadMaterialTexture(material, aiTextureType_SPECULAR, graphicsFactory);

			if(diffuseTexture) {
				meshMaterial->setAlbedoTexture(std::move(diffuseTexture));
			}
			if(specularTexture) {
				meshMaterial->setSpecularTexture(std::move(specularTexture));
			}
		}

		return std::make_shared<rnd::Mesh>(vertexBufferData, indices, meshMaterial->createInstance());
	}

	rnd::Model loadModel(std::string_view modelFilePath, const std::shared_ptr<clv::gfx::GraphicsFactory>& graphicsFactory) {
		CLV_PROFILE_FUNCTION();

		std::vector<std::shared_ptr<rnd::Mesh>> meshes;

		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(modelFilePath.data(), aiProcess_Triangulate | aiProcess_FlipUVs);
		if(scene == nullptr || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || scene->mRootNode == nullptr) {
			GARLIC_LOG(garlicLogContext, Log::Level::Error, "Assimp Error: {0}", importer.GetErrorString());
			return { meshes };
		}

		for(size_t i = 0; i < scene->mNumMeshes; ++i) {
            aiMesh* mesh = scene->mMeshes[i];
            meshes.emplace_back(processMesh(mesh, scene, graphicsFactory, MeshType::Default));
        }

		return { meshes };
	}

	rnd::SkeletalMesh loadAnimatedModel(std::string_view modelFilePath, const std::shared_ptr<clv::gfx::GraphicsFactory>& graphicsFactory){
        CLV_PROFILE_FUNCTION();

        std::vector<std::shared_ptr<rnd::Mesh>> meshes;

        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(modelFilePath.data(), aiProcess_Triangulate | aiProcess_FlipUVs);
        if(scene == nullptr || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || scene->mRootNode == nullptr) {
            GARLIC_LOG(garlicLogContext, Log::Level::Error, "Assimp Error: {0}", importer.GetErrorString());
            return { meshes };
        }

		//TODO: Support multiple skeletons?
		rnd::Skeleton skeleton;

		//Build scene map
        std::unordered_map<std::string_view, aiNode*> nodeNameMap;
        buildNodeNameMap(nodeNameMap, scene->mRootNode);

		//Build skeleton
        bool skeletonSet = false;
        for(size_t i = 0; i < scene->mNumMeshes; ++i) {
            aiMesh* mesh = scene->mMeshes[i];
            meshes.emplace_back(processMesh(mesh, scene, graphicsFactory, MeshType::Animated));

            if(mesh->mNumBones <= 0 || skeletonSet) {
                continue;
			}

            skeleton.joints.resize(mesh->mNumBones);
            for(size_t i = 0; i < mesh->mNumBones; i++) {
                aiBone* bone = mesh->mBones[i];
				skeleton.joints[i].name            = bone->mName.C_Str();
				skeleton.joints[i].inverseBindPose = convertToGarlicMatrix(bone->mOffsetMatrix);
			}

			//Only doing one sekelton for now
            skeletonSet = true;
        }

		//Set parents
        for(auto& joint : skeleton.joints) {
            joint.parentIndex = getJointParentId(skeleton, joint.name, nodeNameMap);
		}

		//Load animations
        std::vector<rnd::AnimationClip> animationClips(scene->mNumAnimations);
        for(size_t i = 0; i < scene->mNumAnimations; ++i) {
            aiAnimation* animation = scene->mAnimations[i];

			/*
			aiNodeAnim represents the entire timeline for one joint
			AnimationPose represents the current point in time for all joints
			*/

			//We have one pose for each pos/rot/scale key
			//TODO: Assuming they are all the same size (numpos numscale numrot)
			//TODO: Skipping the first one because it's 'Armature', this is assumptive though
			std::vector<rnd::AnimationPose> poses(animation->mChannels[1]->mNumPositionKeys);
            for(size_t key = 0; key < animation->mChannels[1]->mNumPositionKeys; ++key) {
                //TODO: Basing everything off of the first frame, this needs to change
                poses[key].poses.resize(skeleton.joints.size()); //Pose for every joint
                poses[key].timeStamp = animation->mChannels[1]->mPositionKeys[key].mTime;

                for(size_t j = 1; j < animation->mNumChannels; ++j) {//TODO: Skipping the first one as that's the root node (I think)
                    aiNodeAnim* channel = animation->mChannels[j];

                    //This needs to happen outside the loop basically
                    const rnd::JointIndexType jointIndex = getJointIndex(skeleton, channel->mNodeName.C_Str());

                    poses[key].poses[jointIndex].position = convertToGarlicVec(channel->mPositionKeys[key].mValue);
                    poses[key].poses[jointIndex].rotation = convertToGarlicQuat(channel->mRotationKeys[key].mValue);
                    poses[key].poses[jointIndex].scale    = convertToGarlicVec(channel->mScalingKeys[key].mValue);
                }
            }

            //TODO: Assuming each animation is for our skeleton
            rnd::AnimationClip clip{};
            clip.skeleton = &skeleton; //TODO: Move will break this
            clip.duration = scene->mAnimations[i]->mDuration;
            clip.poses    = std::move(poses);
		}

		//TODO: Store the animation clips somewhere

		rnd::SkeletalMesh animatedModel{ meshes };
		animatedModel.skeleton = std::move(skeleton);

        return animatedModel;
	}
}