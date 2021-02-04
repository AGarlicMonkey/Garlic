#include "Clove/ModelLoader.hpp"

#include "Clove/Rendering/Renderables/Mesh.hpp"
#include "Clove/Rendering/RenderingConstants.hpp"

#include <Clove/Expected.hpp>
#include <Clove/Log/Log.hpp>
#include <Clove/Maths/Maths.hpp>
#include <Clove/Maths/Matrix.hpp>
#include <Clove/Maths/Quaternion.hpp>
#include <Clove/Maths/Vector.hpp>
#include <Clove/Platform/Window.hpp>
#include <Clove/Timer.hpp>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <map>
#include <set>

namespace garlic::clove::ModelLoader {
    namespace {
        mat4f convert(aiMatrix4x4 const &aiMat) {
            mat4f garlicMat;

            garlicMat[0][0] = aiMat.a1;
            garlicMat[0][1] = aiMat.b1;
            garlicMat[0][2] = aiMat.c1;
            garlicMat[0][3] = aiMat.d1;

            garlicMat[1][0] = aiMat.a2;
            garlicMat[1][1] = aiMat.b2;
            garlicMat[1][2] = aiMat.c2;
            garlicMat[1][3] = aiMat.d2;

            garlicMat[2][0] = aiMat.a3;
            garlicMat[2][1] = aiMat.b3;
            garlicMat[2][2] = aiMat.c3;
            garlicMat[2][3] = aiMat.d3;

            garlicMat[3][0] = aiMat.a4;
            garlicMat[3][1] = aiMat.b4;
            garlicMat[3][2] = aiMat.c4;
            garlicMat[3][3] = aiMat.d4;

            return garlicMat;
        }

        vec3f convert(aiVector3D const &rhs) {
            return { rhs.x, rhs.y, rhs.z };
        }
        aiVector3D convert(vec3f const &rhs) {
            return { rhs.x, rhs.y, rhs.z };
        }

        quatf convert(aiQuaternion const &rhs) {
            return { rhs.w, rhs.x, rhs.y, rhs.z };
        }
        aiQuaternion convert(quatf const &rhs) {
            return { rhs.w, rhs.x, rhs.y, rhs.z };
        }

        void buildNodeNameMap(std::unordered_map<std::string, aiNode *> &map, aiNode *rootNode) {
            map[rootNode->mName.C_Str()] = rootNode;
            for(size_t i = 0; i < rootNode->mNumChildren; ++i) {
                buildNodeNameMap(map, rootNode->mChildren[i]);
            }
        }

        /**
         * @brief Recursively searches the node tree until a valid parent within the skeleton is found.
         */
        std::optional<JointIndexType> getJointIndex(aiNode const *const node, std::unordered_map<std::string, JointIndexType> const &jointNameIdMap) {
            if(node == nullptr) {
                return std::nullopt;
            }

            if(jointNameIdMap.contains(node->mName.C_Str())){
                return jointNameIdMap.at(node->mName.C_Str());
            }else{
                return getJointIndex(node->mParent, jointNameIdMap);
            }
        }

        /**
         * @brief Gets the current pose transform for a given node. Recursively searching for a valid joint if need be.
         */
        aiMatrix4x4 getJointPoseTransform(aiNode const *const node, aiNode const *const parentToStopAt, float const frame, aiAnimation const *const animation, std::unordered_map<std::string, JointIndexType> const &jointNameIdMap) {
            if(node == parentToStopAt) {
                return {};
            }

            aiNodeAnim const *channel{ nullptr };
            for(size_t channelIndex = 0; channelIndex < animation->mNumChannels; ++channelIndex) {
                if(animation->mChannels[channelIndex]->mNodeName == node->mName) {
                    channel = animation->mChannels[channelIndex];
                    break;
                }
            }

            aiMatrix4x4 const parentTransformation{ getJointPoseTransform(node->mParent, parentToStopAt, frame, animation, jointNameIdMap) };

            if(!channel) {
                return parentTransformation * node->mTransformation;
            }

            aiVector3D position{};
            aiQuaternion rotation{};
            aiVector3D scale{};

            //Set the position for the current pose
            {
                bool positionFound{ false };
                std::optional<size_t> prevKey{};
                std::optional<size_t> nextKey{};
                for(size_t key = 0; key < channel->mNumPositionKeys; ++key) {
                    if(channel->mPositionKeys[key].mTime == frame) {
                        position      = channel->mPositionKeys[key].mValue;
                        positionFound = true;
                        break;
                    }

                    if(channel->mPositionKeys[key].mTime < frame) {
                        prevKey = key;
                    }
                    if(channel->mPositionKeys[key].mTime > frame) {
                        nextKey = key;
                        break;//If we're past our current frame then stop searching
                    }
                }
                //Interpolate the missing frame
                if(!positionFound) {
                    if(prevKey.has_value() && nextKey.has_value()) {
                        aiVectorKey const &prevPos{ channel->mPositionKeys[*prevKey] };
                        aiVectorKey const &nextPos{ channel->mPositionKeys[*nextKey] };

                        double const timeBetweenPoses{ nextPos.mTime - prevPos.mTime };
                        double const timeFromPrevPose{ frame - prevPos.mTime };
                        double const normTime{ timeFromPrevPose / timeBetweenPoses };

                        position = convert(lerp(convert(prevPos.mValue), convert(nextPos.mValue), static_cast<float>(normTime)));
                    } else if(prevKey.has_value()) {
                        position = channel->mPositionKeys[*prevKey].mValue;
                    } else if(nextKey.has_value()) {
                        position = channel->mPositionKeys[*nextKey].mValue;
                    }
                }
            }

            //Set the rotation for the current pose
            {
                bool rotationFound{ false };
                std::optional<size_t> prevKey{};
                std::optional<size_t> nextKey{};
                for(size_t key = 0; key < channel->mNumRotationKeys; ++key) {
                    if(channel->mRotationKeys[key].mTime == frame) {
                        rotation      = channel->mRotationKeys[key].mValue;
                        rotationFound = true;
                        break;
                    }

                    if(channel->mRotationKeys[key].mTime < frame) {
                        prevKey = key;
                    }
                    if(channel->mRotationKeys[key].mTime > frame) {
                        nextKey = key;
                        break;//If we're past our current frame then stop searching
                    }
                }
                if(!rotationFound) {
                    if(prevKey.has_value() && nextKey.has_value()) {
                        aiQuatKey const &prevRot{ channel->mRotationKeys[*prevKey] };
                        aiQuatKey const &nextRot{ channel->mRotationKeys[*nextKey] };

                        double const timeBetweenPoses{ nextRot.mTime - prevRot.mTime };
                        double const timeFromPrevPose{ frame - prevRot.mTime };
                        double const normTime{ timeFromPrevPose / timeBetweenPoses };

                        rotation = convert(slerp(convert(prevRot.mValue), convert(nextRot.mValue), static_cast<float>(normTime)));
                    } else if(prevKey.has_value()) {
                        rotation = channel->mRotationKeys[*prevKey].mValue;
                    } else if(nextKey.has_value()) {
                        rotation = channel->mRotationKeys[*nextKey].mValue;
                    }
                }
            }

            //Set the scale for the current pose
            {
                bool scaleFound{ false };
                std::optional<size_t> prevKey{};
                std::optional<size_t> nextKey{};
                for(size_t key = 0; key < channel->mNumScalingKeys; ++key) {
                    if(channel->mScalingKeys[key].mTime == frame) {
                        scale      = channel->mScalingKeys[key].mValue;
                        scaleFound = true;
                        break;
                    }

                    if(channel->mScalingKeys[key].mTime < frame) {
                        prevKey = key;
                    }
                    if(channel->mScalingKeys[key].mTime > frame) {
                        nextKey = key;
                        break;//If we're past our current frame then stop searching
                    }
                }
                if(!scaleFound) {
                    if(prevKey.has_value() && nextKey.has_value()) {
                        aiVectorKey const &prevScale{ channel->mScalingKeys[*prevKey] };
                        aiVectorKey const &nextScale{ channel->mScalingKeys[*nextKey] };

                        double const timeBetweenPoses{ nextScale.mTime - prevScale.mTime };
                        double const timeFromPrevPose{ frame - prevScale.mTime };
                        double const normTime{ timeFromPrevPose / timeBetweenPoses };

                        scale = convert(lerp(convert(prevScale.mValue), convert(nextScale.mValue), static_cast<float>(normTime)));
                    } else if(prevKey.has_value()) {
                        scale = channel->mScalingKeys[*prevKey].mValue;
                    } else if(nextKey.has_value()) {
                        scale = channel->mScalingKeys[*nextKey].mValue;
                    }
                }
            }

            return parentTransformation * aiMatrix4x4{ scale, rotation, position };
        }

        //static std::shared_ptr<gfx::Texture> loadMaterialTexture(aiMaterial* material, aiTextureType type, const std::shared_ptr<garlic::clove::GhaFactory>& graphicsFactory) {
        //	std::shared_ptr<gfx::Texture> texture;

        //	//TODO: Support multiple textures of the same type
        //	if(material->GetTextureCount(type) > 0) {
        //		aiString path;
        //		material->GetTexture(type, 0, &path);

        //		gfx::TextureDescriptor descriptor{};
        //		texture = graphicsFactory->createTexture(descriptor, path.C_Str());
        //	}

        //	return texture;
        //}

        enum class MeshType {
            Default,
            Animated
        };
        std::shared_ptr<Mesh> processMesh(aiMesh *mesh, aiScene const *scene, MeshType const meshType, std::unordered_map<std::string, JointIndexType> const &jointNameIdMap = {}) {
            size_t const vertexCount{ mesh->mNumVertices };

            std::vector<Vertex> vertices(vertexCount);
            std::vector<uint16_t> indices;

            //Build the map of jointIds + weights for each vertex
            std::unordered_map<size_t, std::vector<std::pair<JointIndexType, float>>> vertWeightPairs;
            if(meshType == MeshType::Animated) {
                for(JointIndexType i = 0; i < mesh->mNumBones; ++i) {
                    aiBone const *const bone{ mesh->mBones[i] };
                    for(size_t j = 0; j < bone->mNumWeights; ++j) {
                        aiVertexWeight const &vertexWeight{ bone->mWeights[j] };
                        JointIndexType const jointId{ i };
                        vertWeightPairs[vertexWeight.mVertexId].emplace_back(jointId, vertexWeight.mWeight);
                    }
                }
            }

            //Build the vertex array
            for(size_t i = 0; i < vertexCount; ++i) {
                if(mesh->HasPositions()) {
                    vertices[i].position.x = mesh->mVertices[i].x;
                    vertices[i].position.y = mesh->mVertices[i].y;
                    vertices[i].position.z = mesh->mVertices[i].z;
                }
                if(mesh->HasNormals()) {
                    vertices[i].normal.x = mesh->mNormals[i].x;
                    vertices[i].normal.y = mesh->mNormals[i].y;
                    vertices[i].normal.z = mesh->mNormals[i].z;
                }
                if(mesh->HasTextureCoords(0)) {
                    vertices[i].texCoord.x = mesh->mTextureCoords[0][i].x;
                    vertices[i].texCoord.y = mesh->mTextureCoords[0][i].y;
                }
                if(mesh->HasVertexColors(0)) {
                    aiColor4D const &colour{ mesh->mColors[0][i] };
                    vertices[i].colour = { colour.r, colour.g, colour.b };
                } else {
                    vertices[i].colour = { 1.0f, 1.0f, 1.0f };
                }
                if(meshType == MeshType::Animated) {
                    vec4i &jointIds{ vertices[i].jointIds };
                    vec4f &weights{ vertices[i].weights };

                    std::vector<std::pair<JointIndexType, float>> const &weightPairs{ vertWeightPairs[i] };

                    for(size_t j = 0; j < 4; ++j) {
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

            //Build the index array
            for(size_t i = 0; i < mesh->mNumFaces; ++i) {
                aiFace face = mesh->mFaces[i];
                for(size_t j = 0; j < face.mNumIndices; ++j) {
                    indices.emplace_back(face.mIndices[j]);
                }
            }

            //Set up the Material for the mesh
            if(mesh->mMaterialIndex >= 0) {
                //TODO
                /*aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

			    auto diffuseTexture = loadMaterialTexture(material, aiTextureType_DIFFUSE, graphicsFactory);
			    auto specularTexture = loadMaterialTexture(material, aiTextureType_SPECULAR, graphicsFactory);

			    if(diffuseTexture) {
			    	meshMaterial->setAlbedoTexture(std::move(diffuseTexture));
			    }
			    if(specularTexture) {
			    	meshMaterial->setSpecularTexture(std::move(specularTexture));
			    }*/
            }

            return std::make_shared<Mesh>(std::move(vertices), std::move(indices));
        }

        aiScene const *openFile(std::filesystem::path const &modelFilePath, Assimp::Importer &importer) {
            importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_READ_CAMERAS, false);
            importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_READ_LIGHTS, false);

            unsigned int postProcessFlags{
                aiProcess_ConvertToLeftHanded |
                aiProcessPreset_TargetRealtime_Quality |
                aiProcess_OptimizeGraph
            };

            return importer.ReadFile(modelFilePath.string(), postProcessFlags);
        }
    }

    StaticModel loadStaticModel(std::filesystem::path const &modelFilePath) {
        CLOVE_PROFILE_FUNCTION();

        CLOVE_LOG(LOG_CATEGORY_CLOVE, LogLevel::Debug, "Loading static model: {0}", modelFilePath.string());

        std::vector<std::shared_ptr<Mesh>> meshes;

        Assimp::Importer importer;
        const aiScene *scene{ openFile(modelFilePath, importer) };
        if(scene == nullptr || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || scene->mRootNode == nullptr) {
            CLOVE_LOG(LOG_CATEGORY_CLOVE, LogLevel::Error, "Assimp Error: {0}", importer.GetErrorString());
            return { meshes, std::make_shared<Material>() };
        }

        for(size_t i = 0; i < scene->mNumMeshes; ++i) {
            aiMesh *mesh{ scene->mMeshes[i] };
            meshes.emplace_back(processMesh(mesh, scene, MeshType::Default));
        }

        CLOVE_LOG(LOG_CATEGORY_CLOVE, LogLevel::Info, "Finished loading static model: {0}", modelFilePath.string());
        return { meshes, std::make_shared<Material>() };
    }

    AnimatedModel loadAnimatedModel(std::filesystem::path const &modelFilePath) {
        CLOVE_PROFILE_FUNCTION();

        CLOVE_LOG(LOG_CATEGORY_CLOVE, LogLevel::Debug, "Loading animated model: {0}", modelFilePath.string());

        std::vector<std::shared_ptr<Mesh>> meshes;
        std::unique_ptr<Skeleton> skeleton{ std::make_unique<Skeleton>() };//TODO: Support multiple skeletons?

        Assimp::Importer importer;
        const aiScene *scene{ openFile(modelFilePath, importer) };
        if(scene == nullptr || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) || scene->mRootNode == nullptr) {
            CLOVE_LOG(LOG_CATEGORY_CLOVE, LogLevel::Error, "Assimp Error: {0}", importer.GetErrorString());
            return { meshes, std::make_shared<Material>(), nullptr, {} };
        }

        //Lookup maps
        std::unordered_map<std::string, aiNode *> nodeNameMap;
        std::unordered_map<std::string, JointIndexType> jointNameIdMap;
        buildNodeNameMap(nodeNameMap, scene->mRootNode);

        bool skeletonSet{ false };
        for(size_t i = 0; i < scene->mNumMeshes; ++i) {
            aiMesh *mesh{ scene->mMeshes[i] };
            meshes.emplace_back(processMesh(mesh, scene, MeshType::Animated));

            if(mesh->mNumBones <= 0) {
                continue;
            } else if(skeletonSet) {
                CLOVE_LOG(LOG_CATEGORY_CLOVE, LogLevel::Warning, "Multiple skeletons detected in {0}. Currently Clove only support one skeleton per model. Animations may be innacrate.", modelFilePath.string());
                continue;
            }

            //Build skeleton
            skeleton->joints.resize(mesh->mNumBones);
            for(size_t i = 0; i < mesh->mNumBones; i++) {
                aiBone *bone{ mesh->mBones[i] };
                skeleton->joints[i].name            = bone->mName.C_Str();
                skeleton->joints[i].inverseBindPose = convert(bone->mOffsetMatrix);

                jointNameIdMap[skeleton->joints[i].name] = i;
            }

            //Only doing one sekelton for now
            skeletonSet = true;
        }

        //Set parents
        for(auto &joint : skeleton->joints) {
            if(auto *node{ nodeNameMap[joint.name] }; node->mParent != nullptr) {
                joint.parentIndex = getJointIndex(node->mParent, jointNameIdMap);
            }
        }

        if(skeleton->joints.size() > MAX_JOINTS) {
            CLOVE_LOG(LOG_CATEGORY_CLOVE, LogLevel::Error, "{0} has too many joints (Max supported {1}, current amount {2}). Could not import animations", modelFilePath.string(), MAX_JOINTS, skeleton->joints.size());
            return { meshes, std::make_shared<Material>(), nullptr, {} };
        }

        //Load animations
        std::vector<AnimationClip> animationClips(scene->mNumAnimations);
        for(size_t animIndex = 0; animIndex < scene->mNumAnimations; ++animIndex) {
            aiAnimation const *const animation{ scene->mAnimations[animIndex] };
            AnimationClip &animClip{ animationClips[animIndex] };

            animClip.skeleton = skeleton.get();
            animClip.duration = animation->mDuration / animation->mTicksPerSecond;//Our clip is in seconds where as the animation is in frames

            //Get all the key frame times for every possible channel. This is required as we want a full skeletal pose for each keyframe.
            std::set<float> frames;
            for(size_t channelIndex = 0; channelIndex < animation->mNumChannels; ++channelIndex) {
                aiNodeAnim const *const channel{ animation->mChannels[channelIndex] };
                for(size_t key = 0; key < channel->mNumPositionKeys; ++key) {
                    frames.emplace(static_cast<float>(channel->mPositionKeys[key].mTime));
                }
                for(size_t key = 0; key < channel->mNumRotationKeys; ++key) {
                    frames.emplace(static_cast<float>(channel->mRotationKeys[key].mTime));
                }
                for(size_t key = 0; key < channel->mNumScalingKeys; ++key) {
                    frames.emplace(static_cast<float>(channel->mScalingKeys[key].mTime));
                }
            }

            //Get each channel's pose at each time
            for(float frame : frames) {
                AnimationPose animPose{};
                animPose.timeStamp = frame / animation->mTicksPerSecond;
                animPose.poses.resize(skeleton->joints.size());

                for(JointIndexType i{ 0 }; i < skeleton->joints.size(); ++i) {
                    JointPose &jointPose{ animPose.poses[i] };
                    Joint &joint{ skeleton->joints[i] };

                    aiNode *nodeToAnimate{ nodeNameMap[joint.name] };
                    aiNode *nodeParentInSkeleton{ joint.parentIndex.has_value() ? nodeNameMap[skeleton->joints[*joint.parentIndex].name] : nodeToAnimate->mParent };

                    aiVector3D position{};
                    aiQuaternion rotation{};
                    aiVector3D scale{};
                    getJointPoseTransform(nodeToAnimate, nodeParentInSkeleton, frame, animation, jointNameIdMap).Decompose(scale, rotation, position);

                    jointPose.position = convert(position);
                    jointPose.rotation = convert(rotation);
                    jointPose.scale    = convert(scale);
                }

                animClip.poses.emplace_back(std::move(animPose));
            }
        }

        CLOVE_LOG(LOG_CATEGORY_CLOVE, LogLevel::Info, "Finished loading animated model: {0}", modelFilePath.string());
        return { meshes, std::make_shared<Material>(), std::move(skeleton), std::move(animationClips) };
    }
}