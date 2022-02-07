#include "TextRenderer.h"

int TextRenderer::textRendererCount = 0;
std::vector<FontInfo> TextRenderer::font;

void TextRenderer::Initialise(const char* font, ShaderProgram& shader, bool staticText)
{
	fontIndex = BuildFontTexture(font);

	shader.UseShader();
	//needs to be updated with width and height of window
	textColourUniform = glGetUniformLocation(shader.GetShaderProgram(), "textColour");

	//create vertex array and buffer objects for holding vertex data. we only need one VBO for vertex coordinates
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &vertexBufferID);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
	//6 vertices for one quad, each vertex is a vector4
	//dynamic because the data will change often, between each char
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW); 
	glEnableVertexAttribArray(0);
	//describes how data should be interpreted
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);

	//clear bound VAO and VBO
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	initialized = true;
	textRendererCount++;
}

void TextRenderer::RefreshTextData()
{
	textData.clear();
	staticText = false;
}

void TextRenderer::QueueText(std::string text, Vector2 minXY, float scale, glm::vec3 colour)
{
	textData.push_back(TextData{ text, minXY, scale, colour });
	staticText = false;
}

void TextRenderer::Draw()
{
	if (textData.size() <= 0 || !initialized) {
		return;
	}

	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(VAO);
	glEnableVertexAttribArray(0); //location 0 in shader is used for vertex data
	glBindTexture(GL_TEXTURE_2D, font[fontIndex].textureID);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);

	//if not static, update VBO with new vertex information
	if (!staticText)
	{

		vertices.clear();
		for (size_t i = 0; i < textData.size(); i++)
		{
			BufferTextData(textData[i]);
		}
		//export data to gpu
		//if data is less then the size of the buffer, we don't have to reallocate
		if (vertices.size() <= lastSize)
		{
			glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(glm::vec4), vertices.data());
		}
		{
			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec4), vertices.data(), GL_DYNAMIC_DRAW);
		}
		//here should do glDrawArrays for each individual text line, calling 
		//glUniform3f(textColourUniform, data.colour.x, data.colour.y, data.colour.z);
		//to change the text colour
		for (size_t i = 0; i < textData.size(); i++)
		{
			glUniform3f(textColourUniform, textData[i].colour.x, textData[i].colour.y, textData[i].colour.z);

		}
	}
	
	//actually render
	glDrawArrays(GL_TRIANGLES, 0, vertices.size());
	//clear 
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);
	
	lastSize = vertices.size();
}

void TextRenderer::ChangeFont(const char* font)
{
	//other font will stay saved
	fontIndex = BuildFontTexture(font);
}

void TextRenderer::Build()
{
	if (!initialized) return;

	glBindVertexArray(VAO);
	glEnableVertexAttribArray(0); //location 0 in shader is used for vertex data
	glBindTexture(GL_TEXTURE_2D, font[fontIndex].textureID);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);

	//(this area is copied from draw)
	vertices.clear();
	for (size_t i = 0; i < textData.size(); i++)
	{
		BufferTextData(textData[i]);
	}
	//export data to gpu
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec4), vertices.data(), GL_DYNAMIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
	glBindTexture(GL_TEXTURE_2D, 0);

	staticText = true;
}

TextRenderer::~TextRenderer()
{
	if (!initialized) return;

	glDeleteBuffers(1, &vertexBufferID);
	glDeleteVertexArrays(1, &VAO);

	// if this is the last text renderer, handle deletion of textures.
	textRendererCount--;
	if (textRendererCount == 0)
	{
		for (size_t i = 0; i < font.size(); i++)
		{
			glDeleteTextures(1, &font[i].textureID);
		}
	}
}

void TextRenderer::BufferTextData(TextData& data)
{
	float x = data.minXY.x;

	//go through all characters
	for (char c : data.text)
	{
		auto& glyph = font[fontIndex].charData[c];
		if (glyph.size.x != 0 && glyph.size.y != 0)
		{
			//need to make sure character is in the correct place
			float xPos = x + glyph.bearing.x * data.scale;
			//some characters are offset downward by an amount equal to height - bearing.y
			float yPos = data.minXY.y - (glyph.size.y - glyph.bearing.y) * data.scale;
			float width = glyph.size.x * data.scale;
			float height = glyph.size.y * data.scale;

			float rightSide = glyph.xOffset + (float)glyph.size.x / font[fontIndex].textureWidth;
			float top = (float)glyph.size.y / font[fontIndex].textureHeight;

			//could instead send 2 vector4s
			//#1 containing pos and scale
			//#2 containing uvPos and uvScale
			
			//set up vertex buffer, to be copied into VBO
			/*float vertices[6][4] = {
				{ xPos,			yPos + height,  glyph.xOffset,	0.0f },
				{ xPos,			yPos,			glyph.xOffset,	top  },
				{ xPos + width, yPos,			rightSide,		top  },
														
				{ xPos,			yPos + height,  glyph.xOffset,	0.0f },
				{ xPos + width, yPos,			rightSide,		top  },
				{ xPos + width, yPos + height,  rightSide,		0.0f }
			};*/
			vertices.push_back({ xPos,			yPos + height,  glyph.xOffset,	0.0f });
			vertices.push_back({ xPos,			yPos,			glyph.xOffset,	top  });
			vertices.push_back({ xPos + width,	yPos,			rightSide,		top  });
							   										
			vertices.push_back({ xPos,			yPos + height,  glyph.xOffset,	0.0f });
			vertices.push_back({ xPos + width,	yPos,			rightSide,		top  });
			vertices.push_back({ xPos + width,	yPos + height,  rightSide,		0.0f });
			
			//set active texture to the char texture

			//remember: should never do a bunch of calls to glBufferSubData for the same area of data, 
			// it has to WAIT for the last one to end before it begins, if it is a different area it is MUUUCH faster
			//glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
			
		}
		//now advance x
		x += (glyph.advance.x) * data.scale;
	}
}

int TextRenderer::BuildFontTexture(const char* font)
{
	//check if font already exists in vector
	for (size_t i = 0; i < TextRenderer::font.size(); i++)
	{
		if (TextRenderer::font[i].fontName == font) {
			//already exists in vector, so just return index of that font
			return i;
		}
	}

	FontInfo info;
	info.fontName = font;

	//initialize freetype library
	FT_Library fT;
	if (FT_Init_FreeType(&fT)) //error returns non 0 int
	{
		std::cout << "Could not initialize FreeType library" << std::endl;
		return 0;
	}

	//load font in (called a face for some reason?)
	FT_Face face;
	if (FT_New_Face(fT, font, 0, &face)) //error returns non 0 int
	{
		std::cout << "Failed to load font at location " << font << std::endl;
		return 0;
	}

	//width will be set dynamically based on height
	FT_Set_Pixel_Sizes(face, 0, TEXT_RENDERER_FONT_HEIGHT_PX);

	//load every char from 32 to 127

	info.textureWidth = 0;
	info.textureHeight = 0;

	//create a texture strip containing all the characters (packed together like a proper texture atlas would be better, but much harder)
	for (unsigned char c = 32; c < 127; c++)
	{
		if (FT_Load_Char(face, c, FT_LOAD_RENDER)) //error returns non 0 int
		{
			std::cout << "FreeType: failed to load glyph" << std::endl;
			continue;
		}
		info.textureWidth += face->glyph->bitmap.width;
		info.textureHeight = std::max(info.textureHeight, face->glyph->bitmap.rows);
	}

	glGenTextures(1, &info.textureID);
	glBindTexture(GL_TEXTURE_2D, info.textureID);
	//textures usually have a 4 byte alignment, but since this is 8 bit grayscale it uses one byte per pixel and its better to align to that
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	//create empty texture
	glTexImage2D(
		GL_TEXTURE_2D,			  //target texture
		0,						  //lod number
		GL_RED,					  //number of color components (one, this is a grayscale image)
		info.textureWidth,			  //width of texture
		info.textureHeight,						  //height of texture
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
			(float)xOffset / info.textureWidth };

		xOffset += face->glyph->bitmap.width;
		info.charData.insert(std::make_pair(c, data));
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	//now the glyphs have been processed into data we can free FreeType
	FT_Done_Face(face);
	FT_Done_FreeType(fT);

	//now save FontInfo into the font info vector and return it's index
	TextRenderer::font.push_back(info);
	return TextRenderer::font.size() - 1;
}

//width and height of text bounding box 
void TextRenderer::GetTextWidthHeight(std::string string, float& width, float& height)
{
	width = 0;
	for (size_t i = 0; i < string.size(); i++)
	{
		char character = (char)string[i];

		if (character < 32 || character > 127)
			return;

		height = std::max(height, (float)font[fontIndex].charData[character].bearing.y);
		width += font[fontIndex].charData[character].advance.x;
	}
}