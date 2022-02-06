#pragma once
#include <iostream>
#include "Maths.h"
#include "Graphics.h"
#include "ShaderProgram.h"
#include <ft2build.h>
#include <map>
#include <vector>
#include FT_FREETYPE_H
#define TEXT_RENDERER_FONT_HEIGHT_PX 48
#include "Shape.h"

//this text renderer was made using this tutorial, using the freetype library to load the font
//https://learnopengl.com/In-Practice/Text-Rendering
//I assume it is okay to use a tutorial and an additional library for rendering stuff because this is a physics assingment

struct CharacterData {
	Vector2Int size;
	Vector2Int bearing; //bitmap_left && bitmap_top
	Vector2Int advance;
	float xOffset;
};

struct TextData 
{
	std::string text;
	Vector2 minXY;
	float scale;
	glm::vec3 colour;
};

struct FontInfo 
{
	std::string fontName;
	std::map<char, CharacterData> charData;
	unsigned int textureWidth;
	unsigned int textureHeight;
	unsigned int textureID;
};

class TextRenderer
{
public:
	void Initialise(const char* font, ShaderProgram& shader, bool staticText);

	//text data will probably not have to update every frame, just every time it is changed
	//There will be a UI manager that helps with that
	void RefreshTextData();
	void QueueText(std::string text, Vector2 minXY, float scale, glm::vec3 colour);
	void Draw();
	void ChangeFont(const char* font);
	void GetTextWidthHeight(std::string string, float& width, float& height);

	void Build();
	~TextRenderer();
private:
	void BufferTextData(TextData& data);
	
	static int BuildFontTexture(const char* font);
	static std::vector<FontInfo> font;
	static int textRendererCount;
	int fontIndex;

	std::vector<TextData> textData;

	//vertex array object, vertex and uv buffer id
	unsigned int VAO, vertexBufferID;
	int textColourUniform;

	//quad vertex array
	std::vector<glm::vec4> vertices;
	int lastSize = -1;

	bool staticText = false;
	bool initialized = false;
};

