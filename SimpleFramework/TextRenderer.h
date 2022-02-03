#pragma once
#include <iostream>
#include "Maths.h"
#include "Graphics.h"
#include "ShaderProgram.h"
#include <ft2build.h>
#include <map>
#include <vector>
#include FT_FREETYPE_H

//this text renderer was made using this tutorial, using the freetype library to load the font
//https://learnopengl.com/In-Practice/Text-Rendering
//I assume it is okay to use a tutorial and an additional library for rendering stuff because this is a physics assingment

struct CharacterData {
	unsigned int textureID;
	Vector2Int size;
	Vector2Int bearing;
	unsigned int advance;
};

struct TextData 
{
	std::string text;
	Vector2 minXY;
	float scale;
	glm::vec3 colour;
};

class TextRenderer
{
public:
	void Initialise(const char* font, ShaderProgram& shader);

	//text data will probably not have to update every frame, just every time it is changed
	//There will be a UI manager that helps with that
	void RefreshTextData();
	void QueueText(std::string text, Vector2 minXY, float scale, glm::vec3 colour);
	void Draw(ShaderProgram& shader);
	void UpdateWindowMatrix(ShaderProgram& shader, int width, int height);

	~TextRenderer();
private:
	void DrawText(TextData& data);
	//stores the data for each character needed for rendering
	//index converted to char is letter
	std::vector<CharacterData> charData;
	std::vector<TextData> textInfo;

	//vertex array object, vertex buffer object
	unsigned int VAO, VBO;
	//the matrix used for rendering, is updated every time the window size changes
	Matrix4x4 textProjectionMatrix;
	int textColourUniform;
	int projectionUniform;

	//quad vertex array
	float vertices[6][4];
};

