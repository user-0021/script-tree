#pragma once
#include <ft2build.h>
#include FT_FREETYPE_H

typedef struct {
	unsigned int textureID;
	int sizeX, sizeY;		
	int bearingX, bearingY;  
 	unsigned int advance;
} Character;

Character asciiCharacters[128];
unsigned int asciiAtlas;

int _wtLoadASCII(FT_Library ft,const char* const path) {
	FT_Face face;
	
	//loade face
	if (FT_New_Face(ft, path, 0, &face)) {
		printf("ERROR::FREETYPE: Failed to load font\n");
		return 0;
	}
	//set pixel
	FT_Set_Pixel_Sizes(face, 0, 48);
	
	//set aligment
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	//genelate atlas map
	glGenTextures(1, &asciiAtlas);
	glBindTexture(GL_TEXTURE_2D, asciiAtlas);
	glTexImage2D(
	GL_TEXTURE_2D,
		0,
		GL_RED,
		face->glyph->bitmap.width,
		face->glyph->bitmap.rows,
		0,
		GL_RED,
		GL_UNSIGNED_BYTE,
		face->glyph->bitmap.buffer
	);

	uint8_t c;
	for (c = 0; c < 128; c++) {
		if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
			fprintf(stderr,"%s: Failed to load Glyph\n",__func__);
			continue;
		}

		
		printf("%d\n",face->glyph->bitmap.width);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		Character character = {
			texture,
			face->glyph->bitmap.width,
			face->glyph->bitmap.rows,
			face->glyph->bitmap_left,
			face->glyph->bitmap_top,
			face->glyph->advance.x
		};
		asciiCharacters[c] = character;
	}
	return 1;
}
void render_text(const char *text, float x, float y, float scale) {
	for (const char* c = text; *c; c++) {
		Character ch = asciiCharacters[*c];

		float xpos = 0;
		float ypos = 0;

		float w = 0.1 * scale;
		float h = 0.1 * scale;

		// 四角形を描画するための頂点データ
		float vertices[6][4] = {
			{ 0,	0.5,   0.0f, 0.0f },
			{ 0,   0,	   0.0f, 1.0f },
			{ 0.5,0,	   1.0f, 1.0f },

			{ 0,	0.5,   0.0f, 0.0f },
			{ 0.5, 0,	   1.0f, 1.0f },
			{ 0.5, 0.5,   1.0f, 0.0f }
		};


		unsigned int VAO, VBO;
glGenVertexArrays(1, &VAO);
glGenBuffers(1, &VBO);
glBindVertexArray(VAO);
glBindBuffer(GL_ARRAY_BUFFER, VBO);
glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
glEnableVertexAttribArray(0);
glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);

		// テクスチャをバインドしてテキストを描画
		glBindTexture(GL_TEXTURE_2D, ch.textureID);
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
		glDrawArrays(GL_TRIANGLES, 0, 6);

		// 次の文字の位置に移動
		x += (ch.advance >> 6) * scale;  // ビットシフトでピクセル単位に変換
	}
glBindBuffer(GL_ARRAY_BUFFER, 0);
glBindVertexArray(0);
}
