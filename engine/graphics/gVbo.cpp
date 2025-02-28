/*
 * gVbo.cpp
 *
 *  Created on: 25 Kas 2020
 *      Author: Acer
 */

#include "gVbo.h"
#include "gLight.h"

gVbo::gVbo() {
	glGenVertexArrays(1, &vao);
	vbo = 0;
	ebo = 0;
	isenabled = true;
	isvertexdataallocated = false;
	verticesptr = nullptr;
	vertexarrayptr = nullptr;
	vertexdatacoordnum = 0;
	totalvertexnum = 0;
	isindexdataallocated = false;
	indexarrayptr = nullptr;
	totalindexnum = 0;
	sli = 0;
	scenelight = nullptr;
	colorshader = nullptr;

	isAMD = false;
    const unsigned char* vendorname = glGetString(GL_VENDOR);
    std::string glven(reinterpret_cast<const char*>(vendorname));

    if (glven.find("ATI") != std::string::npos) isAMD = true;
}

gVbo::~gVbo() {
}

void gVbo::setVertexData(gVertex* vertices, int coordNum, int total) {
    glBindVertexArray(vao);
	if (!isvertexdataallocated) glGenBuffers(1, &vbo);
	verticesptr = &vertices[0];
	vertexarrayptr = &vertices[0].position.x;
	vertexdatacoordnum = coordNum;
	totalvertexnum = total;
	isvertexdataallocated = true;

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, totalvertexnum * sizeof(gVertex), vertexarrayptr, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(gVertex), (void*)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(gVertex), (void*)offsetof(gVertex, normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(gVertex), (void*)offsetof(gVertex, texcoords));
    // vertex tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(gVertex), (void*)offsetof(gVertex, tangent));
    // vertex bitangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(gVertex), (void*)offsetof(gVertex, bitangent));
    glBindVertexArray(0);

    /* Because of a bug with AMD drivers, glDeleteBuffers function must be called
     * only while exiting the application.
     */
    if(!isAMD) glDeleteBuffers(1, &vbo);
}

void gVbo::setVertexData(const float* vert0x, int coordNum, int total, int usage, int stride) {
    glBindVertexArray(vao);
	if (!isvertexdataallocated) glGenBuffers(1, &vbo);
	vertexarrayptr = vert0x;
	vertexdatacoordnum = coordNum;
	totalvertexnum = total;

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	GLsizeiptr size = (stride == 0) ? vertexdatacoordnum * sizeof(float) : stride;
	glBufferData(GL_ARRAY_BUFFER, totalvertexnum * size, vertexarrayptr, usage);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, vertexdatacoordnum, GL_FLOAT, GL_FALSE, vertexdatacoordnum * sizeof(float), (void*)0);
	glBindVertexArray(0);
	glDeleteBuffers(1, &vbo);
	isvertexdataallocated = true;
}

void gVbo::setIndexData(unsigned int* indices, int total) {
    glBindVertexArray(vao);
	if (!isindexdataallocated) glGenBuffers(1, &ebo);
	indexarrayptr = indices;
	totalindexnum = total;
	isindexdataallocated = true;

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, totalindexnum * sizeof(unsigned int), indexarrayptr, GL_STATIC_DRAW);
    glBindVertexArray(0);
    if(!isAMD) glDeleteBuffers(1, &ebo);
}

void gVbo::clear() {
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
}

gVertex* gVbo::getVertices() const {
	return verticesptr;
}

unsigned int* gVbo::getIndices() const {
	return indexarrayptr;
}

void gVbo::bind() const {
	glBindVertexArray(vao);
}

void gVbo::unbind() const {
	glBindVertexArray(0);
}

void gVbo::draw() {
	draw(GL_TRIANGLES);
}

/**
 * drawMode: GL_TRIANGLES
 */
void gVbo::draw(int drawMode) {
	if (!isenabled) return;
	if(!isvertexdataallocated) {
		logw("Vertex data is not allocated!");
		return;
	}

	colorshader = renderer->getColorShader();
	colorshader->use();

    // Set scene properties
    colorshader->setVec4("renderColor", renderer->getColor()->r, renderer->getColor()->g, renderer->getColor()->b, renderer->getColor()->a);

    // Set lights
    if (renderer->isLightingEnabled()) {
    	for (sli = 0; sli < renderer->getSceneLightNum(); sli++) {
    		scenelight = renderer->getSceneLight(sli);
    	    colorshader->setInt("light.type", scenelight->getType());
    	    colorshader->setVec3("light.position", scenelight->getPosition());
    	    colorshader->setVec3("light.direction", scenelight->getDirection());
    	    colorshader->setFloat("light.cutOff", scenelight->getSpotCutOffAngle());
    	    colorshader->setFloat("light.outerCutOff", scenelight->getSpotOuterCutOffAngle());
    	    colorshader->setVec4("light.ambient", scenelight->getAmbientColorRed(), scenelight->getAmbientColorGreen(), scenelight->getAmbientColorBlue(), scenelight->getAmbientColorAlpha());
    	    colorshader->setVec4("light.diffuse",  scenelight->getDiffuseColorRed(), scenelight->getDiffuseColorGreen(), scenelight->getDiffuseColorBlue(), scenelight->getDiffuseColorAlpha());
    	    colorshader->setVec4("light.specular", scenelight->getSpecularColorRed(), scenelight->getSpecularColorGreen(), scenelight->getSpecularColorBlue(), scenelight->getSpecularColorAlpha());
    	    colorshader->setFloat("light.constant", scenelight->getAttenuationConstant());
    	    colorshader->setFloat("light.linear", scenelight->getAttenuationLinear());
    	    colorshader->setFloat("light.quadratic", scenelight->getAttenuationQuadratic());
    	}
    } else {
	    colorshader->setInt("light.type", gLight::LIGHTTYPE_AMBIENT);
	    colorshader->setVec4("light.ambient", renderer->getLightingColor()->r, renderer->getLightingColor()->g, renderer->getLightingColor()->b, renderer->getLightingColor()->a);
    }

    // Set matrices
    colorshader->setMat4("projection", glm::mat4(1.0f));
	colorshader->setMat4("view", glm::mat4(1.0f));
	colorshader->setMat4("model", glm::mat4(1.0f));

    bind();
	if (isindexdataallocated) {
	    glDrawElements(drawMode, totalindexnum, GL_UNSIGNED_INT, 0);
	} else {
		glDrawArrays(drawMode, 0, totalvertexnum);
	}
    unbind();
}

void gVbo::enable() {
	isenabled = true;
}

void gVbo::disable() {
	isenabled = false;
}

bool gVbo::isVertexDataAllocated() const {
	return isvertexdataallocated;
}

bool gVbo::isIndexDataAllocated() const {
	return isindexdataallocated;
}

void gVbo::setupVbo() {
    // create buffers/arrays
//    glGenVertexArrays(1, &vao);
//    glGenBuffers(1, &vbo);
//    glGenBuffers(1, &ebo);

//    glBindVertexArray(vao);
    // load data into vertex buffers
//    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    // A great thing about structs is that their memory layout is sequential for all its items.
    // The effect is that we can simply pass a pointer to the struct and it translates perfectly to a glm::vec3/2 array which
    // again translates to 3/2 floats which translates to a byte array.
//    glBufferData(GL_ARRAY_BUFFER, totalvertexnum * sizeof(gVertex), vertexarrayptr, GL_STATIC_DRAW);

//    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
//    glBufferData(GL_ELEMENT_ARRAY_BUFFER, totalindexnum * sizeof(unsigned int), indexarrayptr, GL_STATIC_DRAW);
/*
    // set the vertex attribute pointers
    // vertex Positions
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(gVertex), (void*)0);
    // vertex normals
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(gVertex), (void*)offsetof(gVertex, normal));
    // vertex texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(gVertex), (void*)offsetof(gVertex, texcoords));
    // vertex tangent
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(gVertex), (void*)offsetof(gVertex, tangent));
    // vertex bitangent
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(gVertex), (void*)offsetof(gVertex, bitangent));

    glBindVertexArray(0);
*/
}

//void gVbo::setMesh(const gMesh* mesh, int usage) {
//}

void gVbo::setVertexData(const glm::vec3* vertices, int total, int usage) {
	setVertexData(&vertices[0].x, 3, total, usage);
}

void gVbo::setVertexData(const glm::vec2* vertices, int total, int usage) {
	setVertexData(&vertices[0].x, 2, total, usage);
}

void gVbo::setColorData(const gColor* colors, int total, int usage) {
	setColorData(&colors[0].r, total, usage);
}

void gVbo::setColorData(const float* color0r, int total, int usage, int stride) {

}

void gVbo::setTexCoordData(const glm::vec2* texCoords, int total, int usage) {
	setTexCoordData(&texCoords[0].x, total, usage);
}

void gVbo::setTexCoordData(const float* texCoord0x, int total, int usage, int stride) {

}

void gVbo::setNormalData(const glm::vec3* normals, int total, int usage) {
	setNormalData(&normals[0].x, total, usage);
}

void gVbo::setNormalData(const float* normal0x, int total, int usage, int stride) {

}

void gVbo::setIndexData(const int* indices, int total, int usage) {

}

int gVbo::getVAOid() const {
	return vao;
}

int gVbo::getVerticesNum() const {
	return totalvertexnum;
}

int gVbo::getIndicesNum() const {
	return totalindexnum;
}
