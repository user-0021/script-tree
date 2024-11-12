#pragma once
#include <ft2build.h>
#include FT_FREETYPE_H
#include <string.h>

static const uint8_t glyphSize = 48;
static const uint8_t atlasXCount = 16;
static const uint8_t atlasYCount = 8;
static const uint8_t atlasCount = atlasXCount * atlasYCount;

unsigned int asciiAtlas;

int _wtLoadASCII(FT_Library ft,const char* const path) {
	FT_Face face;
	
	//loade face
	if (FT_New_Face(ft, path, 0, &face)) {
		printf("ERROR::FREETYPE: Failed to load font\n");
		return 0;
	}

	//set pixel
	FT_Set_Pixel_Sizes(face, 0, glyphSize);

	//malloc buffer
	uint8_t * buffer = malloc((glyphSize << 4) *( glyphSize << 3));
	memset(buffer,0,(glyphSize << 4) *( glyphSize << 3));

	uint8_t c;
	for (c = 0; c < atlasCount; c++) {
		if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
			fprintf(stderr,"%s: Failed to load Glyph\n",__func__);
			continue;
		}

		uint8_t x,y;
		x = c & 0xF;
		y = (c >> 4) & 0x7;
		unsigned int offset = (glyphSize - face->glyph->bitmap.width) >> 1;
		offset += (x + (y << 4))*glyphSize;
		unsigned char* charTexture = face->glyph->bitmap.buffer;

		uint8_t i;
		for(i = 0;i < glyphSize;i++){
			memcpy(buffer + offset + (i << 4) * glyphSize,
					charTexture,
					face->glyph->bitmap.width);
			charTexture += face->glyph->bitmap.width;
		}
	}

	for (c = 'A'; c < 'B'; c++) {
		uint8_t x,y;
		x = c & 0xF;
		y = (c >> 4) & 0x7;
		unsigned int offset = (x + (y << 4))*glyphSize;

		putchar('\n');
		uint8_t i;
		for(i = 0;i < glyphSize;i++){
			unsigned char* text = buffer + offset + (i << 4)*glyphSize;
			uint8_t j;
			for(j = 0;j < glyphSize;j++){
				if(text[j])
					printf("AA");
				else
					putchar(' ');
			}
			putchar('\n');
		}
	}
	putchar('\n');

	//set aligment
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	//genelate atlas map
	glGenTextures(1, &asciiAtlas);
	glBindTexture(GL_TEXTURE_2D, asciiAtlas);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RED,glyphSize<<4,
		glyphSize<<3,0,GL_RED,GL_UNSIGNED_BYTE,buffer);

	//set 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	//reset aligment
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);


	return 1;
}
void render_text(const char *text, float x, float y, float scale) {
	}
