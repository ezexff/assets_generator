#include "assets_generator.h"

#define MAX_NUM_BONES_PER_VERTEX 4
#define BIG_NUMBER 999999 // значение BoneID, заполняем большим числом чтобы не ассоциироваться с нулевой костью
#define ASSIMP_LOAD_FLAGS (aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals | aiProcess_LimitBoneWeights | aiProcess_CalcTangentSpace)
//#define ASSIMP_LOAD_FLAGS (aiProcess_Triangulate | aiProcess_GenNormals |  aiProcess_JoinIdenticalVertices)
/*
    Основные флаги ASSIMP:
    aiProcess_FixInfacingNormals | // 	    // tries to determine which meshes have normal vectors that are facing inwards and inverts them
    aiProcess_JoinIdenticalVertices |		// join identical vertices/ optimize indexing
    aiProcess_ValidateDataStructure |		// perform a full validation of the loader's output
    aiProcess_ImproveCacheLocality |		// improve the cache locality of the output vertices
    aiProcess_RemoveRedundantMaterials |	// remove redundant materials
    aiProcess_GenUVCoords |					// convert spherical, cylindrical, box and planar mapping to proper UVs
    aiProcess_TransformUVCoords |			// pre-process UV transformations (scaling, translation ...)
    aiProcess_FindInstances |				// search for instanced meshes and remove them by references to one master
    aiProcess_LimitBoneWeights |			// limit bone weights to 4 per vertex
    aiProcess_OptimizeMeshes |				// join small meshes, if possible;
    aiProcess_PreTransformVertices |        // removes the node graph and pre-transforms all vertices with the local transformation matrices of their nodes
    aiProcess_GenSmoothNormals |			// generate smooth normal vectors if not existing
    aiProcess_SplitLargeMeshes |			// split large, unrenderable meshes into sub-meshes
    aiProcess_Triangulate |					// triangulate polygons with more than 3 edges
    aiProcess_ConvertToLeftHanded |			// convert everything to D3D left handed space
    aiProcess_SortByPType);					// make 'clean' meshes which consist of a single type of primitives);
    aiProcess_CalcTangentSpace              // calculates the tangents and bitangents for the imported meshes.
    http://assimp.sourceforge.net/lib_html/postprocess_8h.html#a64795260b95f5a4b3f3dc1be4f52e410a8857a0e30688127a82c7b8939958c6dc
*/

void ProcessBoneTransformsHierarchy(aiNode *Node, FILE *Out)
{
    WriteAiStringIntoFile(Node->mName, Out);
    fwrite(&Node->mTransformation, sizeof(aiMatrix4x4), 1, Out);
    fwrite(&Node->mNumChildren, sizeof(u32), 1, Out);

    for (u32 i = 0; i < Node->mNumChildren; i++)
    {
        ProcessBoneTransformsHierarchy(Node->mChildren[i], Out);
    }
}

void ProcessSingleBone(u32 BoneID, const aiBone *Bone, FILE *Out, u32 *BoneIDs, r32 *Weights)
{
    Assert(Bone->mName.length > 0);
    WriteAiStringIntoFile(Bone->mName, Out);
    fwrite(&Bone->mOffsetMatrix, sizeof(aiMatrix4x4), 1, Out);

    for (u32 i = 0; i < Bone->mNumWeights; i++)
    {
        u32 VertexIndex = Bone->mWeights[i].mVertexId * 4; // позиция в одномерном массиве вида: vec4 Weights
        for (u32 j = 0; j < MAX_NUM_BONES_PER_VERTEX; j++)
        {
            if (Weights[VertexIndex + j] == 0.0f)
            {
                BoneIDs[VertexIndex + j] = BoneID;
                Weights[VertexIndex + j] = Bone->mWeights[i].mWeight;
                break;
            }

        }
    }
}

void ProcessMesh(aiMesh *Mesh, const aiScene *Scene, FILE *Out)
{
    Assert(Mesh->mNumVertices > 0);
    Assert(Mesh->mName.length > 0);

    WriteAiStringIntoFile(Mesh->mName, Out);          // имя меша
    fwrite(&Mesh->mNumVertices, sizeof(u32), 1, Out); // число вершин меша

    u32 VerticesCount = Mesh->mNumVertices;

    // вершины
    if (Mesh->mVertices)
    {
        for (u32 i = 0; i < VerticesCount; i++)
        {
            v3 Position = V3(Mesh->mVertices[i].x, Mesh->mVertices[i].y, Mesh->mVertices[i].z);
            fwrite(&Position, sizeof(v3), 1, Out);
        }
    }
    else
    {
        InvalidCodePath;
    }

    // нормали
    if (Mesh->mNormals)
    {
        for (u32 i = 0; i < VerticesCount; i++)
        {
            v3 Normal = V3(Mesh->mNormals[i].x, Mesh->mNormals[i].y, Mesh->mNormals[i].z);
            fwrite(&Normal, sizeof(v3), 1, Out);
        }
    }
    else
    {
        InvalidCodePath;
    }

    // текстурные координаты
    s32 TextureIndex = 0; // TODO(me): нужна ли поддержка разных индексов текстур?
    if (Mesh->mTextureCoords[TextureIndex])
    {
        for (u32 i = 0; i < VerticesCount; i++)
        {
            v2 TexCoord = V2(Mesh->mTextureCoords[TextureIndex][i].x,//
                           Mesh->mTextureCoords[TextureIndex][i].y);
            fwrite(&TexCoord, sizeof(v2), 1, Out);
        }
    }
    else
    {
        for (u32 i = 0; i < VerticesCount; i++)
        {
            v2 TexCoord = V2(0.0f, 0.0f);
            fwrite(&TexCoord, sizeof(v2), 1, Out);
        }
    }

    // касательные (для шейдерной реализации нормал маппинга)
    if (Mesh->mTangents)
    {
        for (u32 i = 0; i < VerticesCount; i++)
        {
            v3 Tangent = V3(Mesh->mTangents[i].x, Mesh->mTangents[i].y, Mesh->mTangents[i].z);
            fwrite(&Tangent, sizeof(v3), 1, Out);
        }
    }
    else
    {
        InvalidCodePath;
    }

    // индексы
    u32 IndicesCount = Mesh->mNumFaces * 3;
    Assert(IndicesCount == Mesh->mNumVertices);
    u32 TmpVertexIndex = 0;
    for (u32 i = 0; i < IndicesCount; i = i + 3) {
        Assert(Mesh->mFaces[TmpVertexIndex].mNumIndices == 3);

        fwrite(&Mesh->mFaces[TmpVertexIndex].mIndices[0], sizeof(u32), 1, Out);
        fwrite(&Mesh->mFaces[TmpVertexIndex].mIndices[1], sizeof(u32), 1, Out);
        fwrite(&Mesh->mFaces[TmpVertexIndex].mIndices[2], sizeof(u32), 1, Out);

        TmpVertexIndex++;
    }

    // материал
    if (Mesh->mMaterialIndex >= 0)
    {
        b32 WithMaterial = true;
        fwrite(&WithMaterial, sizeof(b32), 1, Out);

        aiMaterial *Material = Scene->mMaterials[Mesh->mMaterialIndex];

        aiString MatName;
        if (AI_SUCCESS == Material->Get(AI_MATKEY_NAME, MatName))
        {
            WriteAiStringIntoFile(MatName, Out);
        }
        else
        {
            string TmpMatName = PushString("Empty");
            WriteStringIntoFile(&TmpMatName, Out);
        }

        aiColor3D Ambient = aiColor3D(0.0f, 0.0f, 0.0f);
        if (AI_SUCCESS == Material->Get(AI_MATKEY_COLOR_AMBIENT, Ambient))
        {
            v4 TmpAmbient = V4(Ambient.r, Ambient.g, Ambient.b, 1.0f);
            fwrite(&TmpAmbient, sizeof(v4), 1, Out);
        }
        else
        {
            v4 TmpAmbient = V4(0.2f, 0.2f, 0.2f, 1.0f);
            fwrite(&TmpAmbient, sizeof(v4), 1, Out);
        }

        aiColor3D Diffuse = aiColor3D(0.0f, 0.0f, 0.0f);
        if (AI_SUCCESS == Material->Get(AI_MATKEY_COLOR_DIFFUSE, Diffuse))
        {
            v4 TmpDiffuse = V4(Diffuse.r, Diffuse.g, Diffuse.b, 1.0f);
            fwrite(&TmpDiffuse, sizeof(v4), 1, Out);
        }
        else
        {
            v4 TmpDiffuse = V4(0.8f, 0.8f, 0.8f, 1.0f);
            fwrite(&TmpDiffuse, sizeof(v4), 1, Out);
        }

        aiColor3D Specular = aiColor3D(0.0f, 0.0f, 0.0f);
        if (AI_SUCCESS == Material->Get(AI_MATKEY_COLOR_SPECULAR, Specular))
        {
            v4 TmpSpecular = V4(Specular.r, Specular.g, Specular.b, 1.0f);
            fwrite(&TmpSpecular, sizeof(v4), 1, Out);
        }
        else
        {
            v4 TmpSpecular = V4(0.0f, 0.0f, 0.0f, 1.0f);
            fwrite(&TmpSpecular, sizeof(v4), 1, Out);
        }

        aiColor3D Emission = aiColor3D(0.0f, 0.0f, 0.0f);
        if (AI_SUCCESS == Material->Get(AI_MATKEY_COLOR_EMISSIVE, Emission))
        {
            v4 TmpEmission = V4(Emission.r, Emission.g, Emission.b, 1.0f);
            fwrite(&TmpEmission, sizeof(v4), 1, Out);
        }
        else
        {
            v4 TmpEmission = V4(0.0f, 0.0f, 0.0f, 1.0f);
            fwrite(&TmpEmission, sizeof(v4), 1, Out);
        }

        r32 Shininess = 0.0f;
        if (AI_SUCCESS == Material->Get(AI_MATKEY_SHININESS, Shininess))
        {
            fwrite(&Shininess, sizeof(r32), 1, Out);
        }
        else
        {
            r32 TmpShininess = 0.0f;
            fwrite(&TmpShininess, sizeof(r32), 1, Out);
        }

        //if (Material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
        // TODO(me): на данный момент запоминается лишь одна текстура на меш: нужна ли поддержка нескольких текстур?
        aiString TmpPath;
        if (Material->GetTexture(aiTextureType_DIFFUSE, 0, &TmpPath, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
        {
            b32 WithTexture = true;
            fwrite(&WithTexture, sizeof(b32), 1, Out);
            WriteAiStringIntoFile(TmpPath, Out);
        }
        else 
        {
            b32 WithTexture = false;
            fwrite(&WithTexture, sizeof(b32), 1, Out);
        }

        // TODO(me): добавить импорт карт нормалей?
        /*if (Material->GetTexture(aiTextureType_NORMALS, 0, &TmpPath, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
        {
            int sdfdsf = 0;
            //aiTextureType_NORMAL 
            b32 WithTexture = true;
            //fwrite(&WithTexture, sizeof(b32), 1, Out);
            //WriteAiStringIntoFile(TmpPath, Out);
        }
        else
        {
            if (Material->GetTexture(aiTextureType_HEIGHT, 0, &TmpPath, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS)
            {
                int sdfdsf = 0;
            }
            else
            {
                int sdfdsf = 0;
            }
        }*/
    }
    else
    {
        b32 WithMaterial = false;
        fwrite(&WithMaterial, sizeof(b32), 1, Out);
    }

    // анимации
    if (Mesh->HasBones() && (Scene->mNumAnimations > 0))
    {
        b32 WithAnimations = true;
        fwrite(&WithAnimations, sizeof(b32), 1, Out);

        // число костей
        Assert(Mesh->mNumBones < BIG_NUMBER);
        fwrite(&Mesh->mNumBones, sizeof(r32), 1, Out);

        // размер массивов для VBO
        u32 BoneIDsAndWeightsArraySize = Mesh->mNumVertices * 4;

        // Промежуточный результат весов вершин
        u32 *BoneIDs = (u32 *)malloc(sizeof(u32) * BoneIDsAndWeightsArraySize);
        r32 *Weights = (r32 *)malloc(sizeof(r32) * BoneIDsAndWeightsArraySize);
        for (int i = 0; i < BoneIDsAndWeightsArraySize; i = i + 4)
        {
            BoneIDs[i] = BIG_NUMBER;
            BoneIDs[i + 1] = BIG_NUMBER;
            BoneIDs[i + 2] = BIG_NUMBER;
            BoneIDs[i + 3] = BIG_NUMBER;

            Weights[i] = 0.0f;
            Weights[i + 1] = 0.0f;
            Weights[i + 2] = 0.0f;
            Weights[i + 3] = 0.0f;
        }

        // смещения костей меша
        for (u32 i = 0; i < Mesh->mNumBones; i++)
        {
            ProcessSingleBone(i, Mesh->mBones[i], Out, BoneIDs, Weights);
        }

        fwrite(BoneIDs, sizeof(u32), BoneIDsAndWeightsArraySize, Out);
        fwrite(Weights, sizeof(r32), BoneIDsAndWeightsArraySize, Out);

        free(BoneIDs);
        free(Weights);

        // иерархия с преобразованиями костей
        ProcessBoneTransformsHierarchy(Scene->mRootNode, Out);

        // маркеры времени у анимаций
        fwrite(&Scene->mNumAnimations, sizeof(u32), 1, Out);
        for (u32 i = 0; i < Scene->mNumAnimations; i++)
        {
            if (Scene->mAnimations[i]->mName.length != 0)
            {
                WriteAiStringIntoFile(Scene->mAnimations[i]->mName, Out);
            }
            else
            {
                string TmpAnimName = PushString("Unnamed");
                WriteStringIntoFile(&TmpAnimName, Out);
            }

            fwrite(&Scene->mAnimations[i]->mDuration, sizeof(r64), 1, Out);
            fwrite(&Scene->mAnimations[i]->mTicksPerSecond, sizeof(r64), 1, Out);
            
            fwrite(&Scene->mAnimations[i]->mNumChannels, sizeof(u32), 1, Out);

            for (u32 j = 0; j < Scene->mAnimations[i]->mNumChannels; j++)
            {
                WriteAiStringIntoFile(Scene->mAnimations[i]->mChannels[j]->mNodeName, Out);

                fwrite(&Scene->mAnimations[i]->mChannels[j]->mNumPositionKeys, sizeof(u32), 1, Out);
                for (u32 k = 0; k < Scene->mAnimations[i]->mChannels[j]->mNumPositionKeys; k++)
                {
                    fwrite(&Scene->mAnimations[i]->mChannels[j]->mPositionKeys[k].mTime, sizeof(r64), 1, Out);
                    fwrite(&Scene->mAnimations[i]->mChannels[j]->mPositionKeys[k].mValue, sizeof(aiVector3D), 1, Out);
                }

                fwrite(&Scene->mAnimations[i]->mChannels[j]->mNumRotationKeys, sizeof(u32), 1, Out);
                for (u32 k = 0; k < Scene->mAnimations[i]->mChannels[j]->mNumRotationKeys; k++)
                {
                    fwrite(&Scene->mAnimations[i]->mChannels[j]->mRotationKeys[k].mTime, sizeof(r64), 1, Out);
                    fwrite(&Scene->mAnimations[i]->mChannels[j]->mRotationKeys[k].mValue.x, sizeof(r32), 1, Out);
                    fwrite(&Scene->mAnimations[i]->mChannels[j]->mRotationKeys[k].mValue.y, sizeof(r32), 1, Out);
                    fwrite(&Scene->mAnimations[i]->mChannels[j]->mRotationKeys[k].mValue.z, sizeof(r32), 1, Out);
                    fwrite(&Scene->mAnimations[i]->mChannels[j]->mRotationKeys[k].mValue.w, sizeof(r32), 1, Out);
                }

                fwrite(&Scene->mAnimations[i]->mChannels[j]->mNumScalingKeys, sizeof(u32), 1, Out);
                for (u32 k = 0; k < Scene->mAnimations[i]->mChannels[j]->mNumScalingKeys; k++)
                {
                    fwrite(&Scene->mAnimations[i]->mChannels[j]->mScalingKeys[k].mTime, sizeof(r64), 1, Out);
                    fwrite(&Scene->mAnimations[i]->mChannels[j]->mScalingKeys[k].mValue, sizeof(aiVector3D), 1, Out);
                }
            }
        }
    }
    else
    {
        b32 WithAnimations = false;
        fwrite(&WithAnimations, sizeof(b32), 1, Out);
    }
}

void ProcessModel(aiNode *Node, const aiScene *Scene, FILE *Out)
{
    // звено, содержащее меши, является моделью
    if (Node->mNumMeshes > 0)
    {
        // имя модели
        if (Node->mName.length != 0)
        {
            WriteAiStringIntoFile(Node->mName, Out);
        }
        else
        {
            string TmpAnimName = PushString("Unnamed");
            WriteStringIntoFile(&TmpAnimName, Out);
        }

        fwrite(&Node->mNumMeshes, sizeof(u32), 1, Out);

        for (u32 i = 0; i < Node->mNumMeshes; i++)
        {
            aiMesh *Mesh = Scene->mMeshes[i];
            ProcessMesh(Mesh, Scene, Out);
        }
    }

    // TODO(me): возможно добавить выгрузку не только мешей, но и других элементов сцены: источники света, камеры и т.д.

    for (u32 i = 0; i < Node->mNumChildren; i++)
    {
        ProcessModel(Node->mChildren[i], Scene, Out);
    }
}

void CreateModelFile(char *SourceFileName, char *DestFileName)
{
    Assimp::Importer importer;
    const aiScene *Scene = importer.ReadFile(SourceFileName, ASSIMP_LOAD_FLAGS);
    if (!Scene || Scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !Scene->mRootNode)
    {
        printf("ERROR::ASSIMP:: %s", importer.GetErrorString());
        InvalidCodePath;
    }

    FILE *Out = fopen(DestFileName, "wb");
    if (Out)
    {
        printf("Start processing model: ");
        printf(SourceFileName);
        printf("\n");

        // обходим иерархию сцены и все имеющиеся модели сохраняем в файл
        ProcessModel(Scene->mRootNode, Scene, Out);

        fclose(Out);

        printf("Ended processing model: ");
        printf(DestFileName);
        printf("\n");
    }
    else
    {
        printf("ERROR: Couldn't open file :(\n");
    }
}

int main()
{
    CreateModelFile("../data/obj/Tree/Tree.obj",
                    "../data/assets/test_tree.spm");

    CreateModelFile("../data/obj/Trump/Trump.obj",
                    "../data/assets/test_trump.spm");

    CreateModelFile("../data/dae/RunningMan/model.dae",
                    "../data/assets/test_cowboy.spm");
                    
    CreateModelFile("../data/dae/Barrel/barrel.dae",
                    "../data/assets/test_barrel.spm");

    CreateModelFile("../data/dae/Vase/vase.dae",
                    "../data/assets/test_vase.spm");

    CreateModelFile("../data/md5mesh/boblampclean/boblampclean.md5mesh",
                    "../data/assets/test_guard.spm");

    return (0);
}