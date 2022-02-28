#pragma once
#include <memory>

class Vao {
public:
	Vao() : data(nullptr) {}
	void Create();
	void Bind();
	void UnBind();
	void Delete();
private:
	struct Data {
		Data();
		~Data();
		Data(const Data&) = delete;
		Data(Data &&) = delete;
		Data &operator=(const Data&) = delete;
		Data &operator=(Data&&) = delete;
		unsigned int vao;
	};

	std::shared_ptr<Data> data;
};

enum class VboType {
	Vertex,
	Element,
};

class Vbo {
public:
	Vbo(): data(nullptr) {}
	void Create(VboType type);
	void Bind();
	void UnBind();
	void Delete();
	void SetData(int size, const void *data, int usage);
	void UpdateData(int offset, int size, const void *data);

private:
	struct Data {
		Data(VboType t);
		~Data();
		Data(const Data&) = delete;
		Data(Data &&) = delete;
		Data &operator=(const Data&) = delete;
		Data &operator=(Data&&) = delete;
		unsigned int vbo;
		VboType type;
	};

	std::shared_ptr<Data> data;
};