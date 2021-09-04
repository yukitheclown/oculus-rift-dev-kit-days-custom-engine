#include "stdafx.h"
#include "vao_vbo.h"
#define GLEW_STATIC
#include <GL/glew.h>

Vao::Data::Data() : vao(0) {}
Vao::Data::~Data(){
    if(!vao) return;
    glDeleteVertexArrays(1, &vao);
}

void Vao::Create(){
	if(data) return;
	data = std::shared_ptr<Data>(new Data());
	glGenVertexArrays(1, &data->vao);
}

void Vao::Bind(){
	if(!data) return;
	glBindVertexArray(data->vao);
}

void Vao::UnBind(){
	if(!data) return;
	glBindVertexArray(0);
}

void Vao::Delete(){
	if(!data) return;
	glDeleteVertexArrays(1, &data->vao);
	data->vao = 0;
}

Vbo::Data::Data(VboType t) : vbo(0), type(t) {}
Vbo::Data::~Data(){
    if(!vbo) return;
    glDeleteVertexArrays(1, &vbo);
}

void Vbo::Create(VboType type){
	if(data) return;
	data = std::shared_ptr<Data>(new Data(type));
	glGenBuffers(1, &data->vbo);
}

void Vbo::Bind(){
	if(!data) return;
	if(data->type == VboType::Vertex)
		glBindBuffer(GL_ARRAY_BUFFER,data->vbo);
	else if(data->type == VboType::Element)
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,data->vbo);
}

void Vbo::SetData(int size, const void *data, int usage){
	if(this->data->type == VboType::Vertex)
	    glBufferData( GL_ARRAY_BUFFER, size, data, usage);
	else if(this->data->type == VboType::Element)
	    glBufferData( GL_ELEMENT_ARRAY_BUFFER, size, data, usage);
}

void Vbo::UpdateData(int offset, int size, const void *data){
	if(this->data->type == VboType::Vertex)
	    glBufferSubData( GL_ARRAY_BUFFER, offset, size, data);
	else if(this->data->type == VboType::Element)
	    glBufferSubData( GL_ELEMENT_ARRAY_BUFFER, offset, size, data);
 }

void Vbo::UnBind(){
	if(!data) return;
	if(data->type == VboType::Vertex)
		glBindBuffer(GL_ARRAY_BUFFER,0);
	else if(data->type == VboType::Element)
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
}

void Vbo::Delete(){
	if(!data) return;
	glDeleteBuffers(1, &data->vbo);
	data->vbo = 0;
}