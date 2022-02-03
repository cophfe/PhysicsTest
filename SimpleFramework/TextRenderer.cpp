#include "TextRenderer.h"
#define FONT_HEIGHT_PX 48

void TextRenderer::Initialise(const char* font, ShaderProgram& shader)
{
	//initialize freetype library
	FT_Library fT;
	if (FT_Init_FreeType(&fT)) //error returns non 0 int
	{
		std::cout << "Could not initialize FreeType library" << std::endl;
		return;
	}

	//load font in (called a face for some reason?)
	FT_Face face;
	if (FT_New_Face(fT, font, 0, &face)) //error returns non 0 int
	{
		std::cout << "Failed to load font at location " << font << std::endl;
		return;
	}

	//width will be set dynamically based on height
	FT_Set_Pixel_Sizes(face, 0, FONT_HEIGHT_PX);

	//textures usually have a 4 byte alignment, but since this is 8 bit grayscale it uses one byte per pixel and its better to align to that
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	

	//load every char from 32 to 12
	//currently makes them seperate textures, should probably modify it to blit it onto the same texture.
	//(once I figure out how to do that, is probably hard since would have to optimise for minimal space)

	//skip first 32 and last
	for (unsigned char c = 32; c < 127; c++)
	{
		//creates a grayscale bitmap in face->glyph->bitmap
		if (FT_Load_Char(face, c, FT_LOAD_RENDER)) //error returns non 0 int
		{
			std::cout << "FreeType: failed to load glyph" << std::endl;
			continue;
		}

		//generate texture
		unsigned int texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(
			GL_TEXTURE_2D,				//target texture
			0,							//lod number
			GL_RED,						//number of color components (one, this is a grayscale image)
			face->glyph->bitmap.width,	//width of texture
			face->glyph->bitmap.rows,	//height of texture
			0,							//it literally says this value must be 0 in the docs, wtf
			GL_RED,						//format of pixel data (gray scale, so one byte per pixel)
			GL_UNSIGNED_BYTE,			//data type of data
			face->glyph->bitmap.buffer	//the data in question
			);
		//set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		auto data = CharacterData{
			texture,
			Vector2Int(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			Vector2Int(face->glyph->bitmap_left, face->glyph->bitmap_top),
			(unsigned int)face->glyph->advance.x};

		charData.insert(std::make_pair(c, data));

		glBindTexture(GL_TEXTURE_2D, 0);
	}
	
	//now the glyphs have been processed into data we can free FreeType
	FT_Done_Face(face);
	FT_Done_FreeType(fT);

	shader.UseShader();
	//needs to be updated with width and height of window
	textProjectionMatrix = glm::ortho(0.0f, 1280.0f, 0.0f, 720.0f);
	textColourUniform = glGetUniformLocation(shader.GetShaderProgram(), "textColour");
	projectionUniform = glGetUniformLocation(shader.GetShaderProgram(), "textProjection");
	glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, (float*)&textProjectionMatrix);

	//create vertex array and buffer objects for holding vertex data. we only need one VBO for vertex coordinates
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//6 vertices for one quad, each vertex is a vector4
	//dynamic because the data will change often, between each char
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW); 
	glEnableVertexAttribArray(0);
	//describes how data should be interpreted
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);

	
	//clear bound VAO and VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	//set up vertex data
	vertices[0][2] = 0;		vertices[0][3] = 0;
	vertices[1][2] = 0;		vertices[1][3] = 1;
	vertices[2][2] = 1;		vertices[2][3] = 1;
	vertices[3][2] = 0;		vertices[3][3] = 0;
	vertices[4][2] = 1;		vertices[4][3] = 1;
	vertices[5][2] = 1;		vertices[5][3] = 0;

}

void TextRenderer::RefreshTextData()
{
	textInfo.clear();
}

void TextRenderer::QueueText(std::string text, Vector2 minXY, float scale, glm::vec3 colour)
{
	textInfo.push_back(TextData{ text, minXY, scale, colour });
}

void TextRenderer::Draw(ShaderProgram& shader)
{
	if (textInfo.size() <= 0) {
		return;
	}

	shader.UseShader();
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(VAO);

	for (size_t i = 0; i < textInfo.size(); i++)
	{
		DrawText(textInfo[i]);
	}

	//clear bindings
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void TextRenderer::UpdateWindowMatrix(ShaderProgram& shader, int width, int height)
{
	shader.UseShader();
	//needs to be updated with width and height of window
	textProjectionMatrix = glm::ortho(0.0f, (float)width, 0.0f, (float)height);
	glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, (float*)&textProjectionMatrix);

}

TextRenderer::~TextRenderer()
{
	glDeleteBuffers(1, &VBO);
}

void TextRenderer::DrawText(TextData& data)
{
	//set colour
	glUniform3f(textColourUniform, data.colour.x, data.colour.y, data.colour.z);
	
	float x = data.minXY.x;
	//go through all characters
	for (char c : data.text) 
	{
		auto& glyph = charData[c];
		//need to make sure character is in the correct place
		float xPos = x + glyph.bearing.x * data.scale;
		//some characters are offset downward by an amount equal to height - bearing.y
		float yPos = data.minXY.y - (glyph.size.y - glyph.bearing.y) * data.scale;
		float width = glyph.size.x * data.scale;
		float height = glyph.size.y * data.scale;
		
		//set up vertex buffer, to be copied into VBO
		vertices[0][0] = xPos;				vertices[0][1] = yPos + height;
		vertices[1][0] = xPos;				vertices[1][1] = yPos;
		vertices[2][0] = xPos + width;		vertices[2][1] = yPos;
		vertices[3][0] = xPos;				vertices[3][1] = yPos + height;
		vertices[4][0] = xPos + width;		vertices[4][1] = yPos;
		vertices[5][0] = xPos + width;		vertices[5][1] = yPos + height;

		//set active texture to the char texture
		glBindTexture(GL_TEXTURE_2D, glyph.textureID);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		//clear binding
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		//actually render
		glDrawArrays(GL_TRIANGLES, 0, 6);
		//now advance x (for some reason advance is stored in 1/64th of a pixel, probably some weird ass unit)
		x += (glyph.advance / 64) * data.scale;
	}

}
