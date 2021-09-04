#define GLEW_STATIC
#include <GL/glew.h>
#include <map>
#include <algorithm>
#include <vector>
#include "mesh.h"
#include "skeletal_animation.h"
#include "shaders.h"
#include <math.h>
#include "math.h"
#include <stdio.h>
#include <string.h>

struct PackedVertex {
    Vector3 position;
    Vector2 uv;
    Vector3 normal;
    bool operator<(const PackedVertex that) const{
        return memcmp((void*)this, (void*)&that, sizeof(PackedVertex))>0;
    };
};

static void ReadImage(FILE *fp, const char *fileName, Image *image){
    char path[512];
    fscanf_s(fp, "%s", path);

    int k;
    for(k = strlen(fileName); k > 0; k --)
        if(fileName[k] == '/') break;

    char fullPath[512] = {};

    memcpy(fullPath, fileName, k+1);
    strcat_s(fullPath, path);

    image->LoadPNG(std::string(fullPath), false);

    image->SetParameters(GL_NEAREST,GL_NEAREST,GL_REPEAT,GL_REPEAT);
    // image->SetParameters(GL_LINEAR,GL_LINEAR,GL_REPEAT,GL_REPEAT);
}

void Mesh::LoadMTL(const std::string &filePath){
	FILE *fp;
	fopen_s(&fp, filePath.c_str(), "r");
    if(!fp) { printf("Failed to read MTL %s\n",filePath.c_str()); return; }

    char lineType[512];

    while(!feof(fp)){

        fscanf_s(fp, "%s ", lineType);

        if(strcmp(lineType, "newmtl") == 0){

            char name[254];
            fscanf_s(fp, "%s", name);
            materials.resize(materials.size()+1);
            materials[materials.size()-1].name = name;

        } else if(strcmp(lineType, "Ns") == 0){

        } else if(strcmp(lineType, "Ka") == 0){

        } else if(strcmp(lineType, "Kd") == 0){

        } else if(strcmp(lineType, "Ks") == 0){

        } else if(strcmp(lineType, "Ni") == 0){

        } else if(strcmp(lineType, "map_Kd") == 0){
            if(!materials.size()) continue;
            ReadImage(fp, filePath.c_str(), &materials[materials.size()-1].diffuseMapImage);

        } else if(strcmp(lineType, "map_d") == 0){

        }

        while(!feof(fp) && fgetc(fp) != '\n'){}
    }

    fclose(fp);
}

static bool sortMaterials(Material a, Material b){
    if(a.startingElementIndex > b.startingElementIndex) return false;
    if(a.startingElementIndex < b.startingElementIndex) return true;
    return false;
}

int Mesh::LoadObjFile(const char *fileName, bool loadUvs, bool loadNorms, bool loadElements, RiggedMesh *riggedMesh){

	FILE *fp;
	fopen_s(&fp, fileName, "r");
    if(fp == NULL) { printf("Error loading model: %s\n", fileName); return 0; }

    char lineType[512] = {};

    std::vector<Vector3> tempBoneWeights;
    std::vector<Vector3> tempBoneIndices;
    std::vector<Vector3> tempVerts;
    std::vector<Vector3> tempNorms;
    std::vector<Vector2> tempUvs;
    std::vector<Vector3> myverts;
    std::vector<Vector3> mynorms;
    std::vector<Vector2> mycoords;
    std::vector<Vector3> myBoneWeights;
    std::vector<Vector3> myBoneIndices;

    while(!feof(fp)){
        fscanf_s( fp , "%s ", lineType );

        // if(strcmp(lineType, "mtllib") == 0){
        if(strcmp(lineType, "tex") == 0){

            // char path[512] = {}, fullPath[512] = {};
            // fscanf(fp, "%s", path);

            // int k;
            // for(k = strlen(fileName); k > 0; k--)
            //     if(fileName[k] == '/') break;

            // memcpy(fullPath, fileName, k+1);
            // strcat(fullPath, path);

            Material mat;
            ReadImage(fp, fileName, &mat.diffuseMapImage);

            materials.push_back(mat);


            // char path[512] = {}, fullPath[512] = {};
            // fscanf(fp, "%s", path);

            // int k;
            // for(k = strlen(fileName); k > 0; k--)
            //     if(fileName[k] == '/') break;

            // memcpy(fullPath, fileName, k+1);
            // strcat(fullPath, path);
            // LoadMTL(fullPath);

        // } else if( strcmp(lineType, "usemtl" ) == 0 ){

        //     char name[512];
        //     fscanf(fp, "%s", name);

        //     for(int k = 0; k < (int)this->materials.size(); k++)
        //         if(this->materials[k].name == name)
        //             if(!this->materials[k].startingElementIndex)
        //                 this->materials[k].startingElementIndex = myverts.size();

        } else if( strcmp(lineType, "texIndex" ) == 0 ){

            int index = 0;
            fscanf_s(fp, "%i", &index);

            if(index >= 0 && index < (int)this->materials.size()){
                this->materials[index].startingElementIndex = myverts.size();
                this->materials[index].texIndex = index;
            }

        } else if( strcmp(lineType, "v" ) == 0 ){
            Vector3 vert = Vector3();
            fscanf_s( fp,"%f %f %f", &vert.x, &vert.y, &vert.z);

            tempVerts.push_back(vert);

            if(riggedMesh){
                tempBoneIndices.push_back({-1,-1,-1});
                tempBoneWeights.push_back({0,0,0});
            }

        } else if( strcmp(lineType, "vw" ) == 0 ){
            if(riggedMesh){

                char name[64];
                float weight = 0;
                fscanf_s( fp,"%s : ", name);
                fscanf_s(fp, "%f", &weight);


                int boneIndex = riggedMesh->GetBoneIndexFromName(name);
                if(boneIndex != -1){

                    float *bi = &tempBoneIndices[tempBoneIndices.size()-1].x;
                    float *bw = &tempBoneWeights[tempBoneWeights.size()-1].x;

                    int k;
                    for(k = 0; k < 3 && bi[k] != -1; k++){}

                    if(k == 3) {

                        int smallestIndex = -1;
                        float smallest = 1;

                        for(k = 0; k < 3; k++){
                            if(bw[k] < smallest){
                                smallest = bw[k];
                                smallestIndex = k;
                            }
                        }

                        if(weight > bw[smallestIndex]){
                            bi[smallestIndex] = boneIndex;
                            bw[smallestIndex] = weight;
                        }

                    } else {
                        bi[k] = boneIndex;
                        bw[k] = weight;
                    }
                }
            }

        } else if( strcmp(lineType, "vt" ) == 0 && loadUvs){
            Vector2 texCoord;
            fscanf_s( fp,"%f %f", &texCoord.x, &texCoord.y );

            tempUvs.push_back(texCoord);

        } else if( strcmp(lineType, "vn" ) == 0 && loadNorms){
            Vector3 norm = Vector3();
            fscanf_s( fp,"%f %f %f", &norm.x, &norm.y, &norm.z );

            tempNorms.push_back(norm);

        } else if( strcmp(lineType, "f" ) == 0 ){
            int v[3] = {}, u[3] = {}, n[3] = {};

            if(loadUvs && loadNorms)
                fscanf_s( fp,"%i/%i/%i %i/%i/%i %i/%i/%i", &v[0], &u[0], &n[0], &v[1], &u[1], &n[1], &v[2], &u[2], &n[2]);
            else if(loadUvs)
                fscanf_s( fp,"%i/%i/ %i/%i/ %i/%i/", &v[0], &u[0], &v[1], &u[1], &v[2], &u[2]);
            else if(loadNorms)
                fscanf_s( fp,"%i//%i %i//%i %i//%i", &v[0], &n[0], &v[1], &n[1], &v[2], &n[2]);
            else
                fscanf_s( fp,"%i// %i// %i//", &v[0], &v[1], &v[2]);

            int f;
            for(f = 0; f < 3; f++) if(v[f] > 0 && v[f] - 1 < (int)tempVerts.size()) myverts.push_back(tempVerts[v[f] - 1]);
            for(f = 0; f < 3; f++) if(u[f] > 0 && u[f] - 1 < (int)tempUvs.size())   mycoords.push_back(tempUvs[u[f] - 1]);
            for(f = 0; f < 3; f++) if(n[f] > 0 && n[f] - 1 < (int)tempNorms.size()) mynorms.push_back(tempNorms[n[f] - 1]);

            if(riggedMesh){
                for(f = 0; f < 3; f++) if(v[f] > 0 && v[f] - 1 < (int)tempBoneIndices.size()) myBoneIndices.push_back(tempBoneIndices[v[f] - 1]);
                for(f = 0; f < 3; f++) if(v[f] > 0 && v[f] - 1 < (int)tempBoneWeights.size()) myBoneWeights.push_back(tempBoneWeights[v[f] - 1]);
            }
        }

        while( !feof(fp) && fgetc(fp) != '\n' ) {}
    }

    fclose(fp);

    if(loadElements){

        int topElementIndex = 0;
        std::map<PackedVertex,unsigned int> vertMap;
        modelSize = 0;

        for(int k = 0; k < (int)myverts.size(); k++){

            PackedVertex packed = {myverts[k], Vector2(), Vector3()};
            if(loadUvs && k < (int)mycoords.size()) packed.uv = mycoords[k];
            if(loadNorms && k < (int)mynorms.size()) packed.normal = mynorms[k];

            std::map<PackedVertex,unsigned int>::iterator it = vertMap.find(packed);

            if(it != vertMap.end()){

                this->modelElementsSize++;
                this->elements.push_back(it->second);

            } else {
                this->modelElementsSize++;
                modelSize++;

                this->verts.push_back(myverts[k]);
                if(loadNorms && k < (int)mynorms.size()) this->norms.push_back(mynorms[k]);
                if(loadUvs && k < (int)mycoords.size()) this->texCoords.push_back(mycoords[k]);
                if(riggedMesh){
                    this->weights.push_back(myBoneWeights[k]);
                    this->boneIndices.push_back(myBoneIndices[k]);
                }

                this->elements.push_back(topElementIndex);

                vertMap[ packed ] = topElementIndex;

                topElementIndex++;
            }
        }

        drawingElements = true;

    } else {

        verts = std::move(myverts);
        norms = std::move(mynorms);
        texCoords = std::move(mycoords);
        drawingElements = false;
    }

    std::sort(this->materials.begin(), this->materials.end(), sortMaterials);

    Vector3 modelSpace;
    float width = 0, height = 0, depth = 0;

    for(int k = 0; k < (int)this->verts.size(); k++){
        if(verts[k].x < modelSpace.x)          modelSpace.x = verts[k].x;
        if(verts[k].y < modelSpace.y)          modelSpace.y = verts[k].y;
        if(verts[k].z < modelSpace.z)          modelSpace.z = verts[k].z;
        if(verts[k].x > modelSpace.x+width)    width  = verts[k].x-modelSpace.x;
        if(verts[k].y > modelSpace.y+height)   height = verts[k].y-modelSpace.y;
        if(verts[k].z > modelSpace.z+depth)    depth  = verts[k].z-modelSpace.z;
    }

    this->bb = BoundingBox(nullptr, width, height, depth, modelSpace);

    return 1;
}

void Mesh::ReloadData(){
    if(!drawingInitialized) return;

    Shaders_UseProgram(shader);

    GLuint positionAttribute = glGetAttribLocation(Shaders_GetProgram(shader), SHADERS_POSITION_ATTRIB);
    GLuint uvAttribute = glGetAttribLocation(Shaders_GetProgram(shader), SHADERS_COORD_ATTRIB);
    GLuint normAttribute = glGetAttribLocation(Shaders_GetProgram(shader), SHADERS_NORM_ATTRIB);

    glEnableVertexAttribArray(positionAttribute);
    glEnableVertexAttribArray(uvAttribute);
    glEnableVertexAttribArray(normAttribute);

    this->vao.Bind();

    this->posVbo.Bind();
    this->posVbo.SetData(this->verts.size()*sizeof(Vector3), &this->verts[0].x, GL_STATIC_DRAW);
    glVertexAttribPointer( positionAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);

    this->uvVbo.Bind();
    this->posVbo.SetData(this->texCoords.size()*sizeof(Vector2), &this->texCoords[0].x, GL_STATIC_DRAW);
    glVertexAttribPointer( uvAttribute, 2, GL_FLOAT, GL_FALSE, 0, 0);

    this->normVbo.Bind();
    this->posVbo.SetData(this->norms.size()*sizeof(Vector3), &this->norms[0].x, GL_STATIC_DRAW);
    glVertexAttribPointer( normAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);


    if(this->weights.size()){
        int indexAttribute = glGetAttribLocation(Shaders_GetProgram(SKELETAL_ANIMATION_SHADER), "boneIndices");
        int weightsAttribute = glGetAttribLocation(Shaders_GetProgram(SKELETAL_ANIMATION_SHADER), "boneWeights");

        glEnableVertexAttribArray(indexAttribute);
        glEnableVertexAttribArray(weightsAttribute);

        this->boneWeightVbo.Bind();
        glVertexAttribPointer( weightsAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
        this->boneWeightVbo.SetData(weights.size()*sizeof(Vector3), &weights[0].x, GL_STATIC_DRAW);

        this->boneIndexVbo.Bind();
        glVertexAttribPointer( indexAttribute, 3, GL_FLOAT, GL_FALSE, 0, 0);
        this->boneIndexVbo.SetData(boneIndices.size()*sizeof(Vector3), &boneIndices[0].x, GL_STATIC_DRAW);
    }

    this->vao.UnBind();
}

void Mesh::InitializeDrawing(const char *texturePath, int shader){
    if(drawingInitialized) return;

    Shaders_UseProgram(shader);

    this->vao.Create();
    this->vao.Bind();
    this->posVbo.Create(VboType::Vertex);
    this->uvVbo.Create(VboType::Vertex);
    this->normVbo.Create(VboType::Vertex);

    if(this->weights.size()){
        this->boneIndexVbo.Create(VboType::Vertex);
        this->boneWeightVbo.Create(VboType::Vertex);
    }

    if(strlen(texturePath) && !this->materials.size())
        this->img.LoadPNG(texturePath);

    this->shader = shader;

    this->drawingInitialized = 1;

    ReloadData();
}

void Mesh::DrawElements(std::vector<Material> *overrideMaterials){

    // printf("%i\n",this->modelElementsSize/3 ); // tri count
    // printf("%i\n",this->modelSize ); // vert count

    glActiveTexture(GL_TEXTURE0);

    if(!this->materials.size()){

        if(drawingElements)
            glDrawElements(GL_TRIANGLES, this->modelElementsSize, GL_UNSIGNED_INT, &this->elements[0]);
        else
            glDrawArrays(GL_TRIANGLES, 0, verts.size());

    } else {

        for(int k = 0; k < (int)this->materials.size(); k++){

            int size = 0;

            if(k == (int)this->materials.size()-1)
                size = this->modelElementsSize - this->materials[k].startingElementIndex;
            else
                size = this->materials[k+1].startingElementIndex - this->materials[k].startingElementIndex;

            unsigned int tex = this->materials[k].diffuseMapImage.GetTexture();

            if(overrideMaterials != nullptr){
                for(int m = 0; m < (int)overrideMaterials->size(); m++){
                    if(overrideMaterials->at(m).texIndex == k){
                        tex = overrideMaterials->at(m).diffuseMapImage.GetTexture();
                        break;
                    }
                }
            }

            glBindTexture(GL_TEXTURE_2D, tex);
            // printf("%i %i %i %i\n", this->materials[k].diffuseMapImage.GetTexture(), this->materials[k].startingElementIndex, size, this->modelElementsSize);

            if(drawingElements)
                glDrawElements(GL_TRIANGLES, size, GL_UNSIGNED_INT, &this->elements[this->materials[k].startingElementIndex]);
            else
                glDrawArrays(GL_TRIANGLES, this->materials[k].startingElementIndex, size);
        }
    }
}

void Mesh::Draw(std::vector<Material> *overrideMaterials){
    if(!drawingInitialized) return;
    this->vao.Bind();
    this->DrawElements(overrideMaterials);
}

void MeshInstance::Draw(std::vector<Material> *overrideMaterials){
    Shaders_SetModelMatrix(&GetModelMatrix().m[0]);
    Shaders_UpdateModelMatrix();
    mesh->Draw(overrideMaterials);
}