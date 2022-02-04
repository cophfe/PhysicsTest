#include "TextRenderer.h"
#define FONT_HEIGHT_PX 48

void TextRenderer::Initialise(const char* font, ShaderProgram& shader, bool staticText)
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

	
	

	//load every char from 32 to 127

	textureWidth = 0;
	textureHeight = 0;

	//create a texture strip containing all the characters (packed together would be more efficient, but much harder)
	for (unsigned char c = 32; c < 127; c++)
	{
		if (FT_Load_Char(face, c, FT_LOAD_RENDER)) //error returns non 0 int
		{
			std::cout << "FreeType: failed to load glyph" << std::endl;
			continue;
		}
		textureWidth += face->glyph->bitmap.width;
		textureHeight = std::max(textureHeight, face->glyph->bitmap.rows);
	}
	std::cout << "width: " << textureWidth << ", height: " << textureHeight;

	//now put them all onto this texture strip
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	//textures usually have a 4 byte alignment, but since this is 8 bit grayscale it uses one byte per pixel and its better to align to that
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	//create empty texture
	glTexImage2D(
		GL_TEXTURE_2D,			  //target texture
		0,						  //lod number
		GL_RED,					  //number of color components (one, this is a grayscale image)
		textureWidth,			  //width of texture
		textureHeight,						  //height of texture
		0,						  //it literally says this value must be 0 in the docs, wtf
		GL_RED,					  //format of pixel data (gray scale, so one byte per pixel)
		GL_UNSIGNED_BYTE,		  //data type of data
		0);						  //pointer to the data buffer (none, we will fill it in later)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//now draw to the texture
	int xOffset = 0;
	for (unsigned char c = 32; c < 127; c++)
	{
		if (FT_Load_Char(face, c, FT_LOAD_RENDER)) //error returns non 0 int
		{
			std::cout << "FreeType: failed to load glyph" << std::endl;
			continue;
		}

		//this allows you to draw to the active texture, so you can draw the glyph to it
		glTexSubImage2D(GL_TEXTURE_2D, 0, xOffset, 0, face->glyph->bitmap.width, face->glyph->bitmap.rows, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);

		auto data = CharacterData{
			Vector2Int(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			Vector2Int(face->glyph->bitmap_left, face->glyph->bitmap_top),
			Vector2Int(face->glyph->advance.x / 64, face->glyph->advance.y / 64),
			(float)xOffset / textureWidth};

		xOffset += face->glyph->bitmap.width;
		charData.insert(std::make_pair(c, data));
	}
	glBindTexture(GL_TEXTURE_2D, 0);

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
}

void TextRenderer::RefreshTextData()
{
	textData.clear();
}

void TextRenderer::QueueText(std::string text, Vector2 minXY, float scale, glm::vec3 colour)
{
	textData.push_back(TextData{ text, minXY, scale, colour });
}

void TextRenderer::Draw(ShaderProgram& shader)
{
	if (textData.size() <= 0) {
		return;
	}

	shader.UseShader();
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(VAO);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	for (size_t i = 0; i < textData.size(); i++)
	{
		DrawText(textData[i]);
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

void TextRenderer::Build()
{

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
		if (glyph.size.x != 0 && glyph.size.y != 0)
		{
			//need to make sure character is in the correct place
			float xPos = x + glyph.bearing.x * data.scale;
			//some characters are offset downward by an amount equal to height - bearing.y
			float yPos = data.minXY.y - (glyph.size.y - glyph.bearing.y) * data.scale;
			float width = glyph.size.x * data.scale;
			float height = glyph.size.y * data.scale;

			float rightSide = glyph.xOffset + (float)glyph.size.x / textureWidth;
			float top = (float)glyph.size.y / textureHeight;
			//set up vertex buffer, to be copied into VBO
			float vertices[6][4] = {
				{ xPos,			yPos + height,  glyph.xOffset,	0.0f },
				{ xPos,			yPos,			glyph.xOffset,	top  },
				{ xPos + width, yPos,			rightSide,		top  },
														
				{ xPos,			yPos + height,  glyph.xOffset,	0.0f },
				{ xPos + width, yPos,			rightSide,		top  },
				{ xPos + width, yPos + height,  rightSide,		0.0f }
			};

			//set active texture to the char texture

			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
			//actually render
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}
		//now advance x
		x += (glyph.advance.x) * data.scale;
	}
}
