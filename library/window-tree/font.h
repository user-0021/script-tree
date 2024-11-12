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
	memset(buffer,255,(glyphSize << 4) *( glyphSize << 3));

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

	memset(buffer,1,(glyphSize << 4) *( glyphSize << 3));
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
					printf("  ");
			}
			putchar('\n');
		}
			putchar('\n');
			putchar('B');
			printf("%d\n",(int)c);
			putchar('\n');
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


	//free
	FT_Done_Face(face);

	return 1;
}
void _wtDrawText(const char *text,unsigned int shader, float x, float y, float scale) {

	float x2 = 1.0f;
	float y2 = 1.0f;
	float x1 = -1.0f;
	float y1 = -1.0f;
	
	//create data
	GLfloat points[4][4] = {{x1,y1,x1,y1},{x2,y1,x2,y1},{x2,y2,x2,y2},{x1,y2,x1,y2}};
	
	//generate vao & vbo
	unsigned int vbo, vao;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glUniform3f(glGetUniformLocation(shader, "textColor"), 1.0, 0,0);
    glActiveTexture(GL_TEXTURE0);

    //bind vao & vbo
    glBindVertexArray(vao);
    glBindTexture(GL_TEXTURE_2D, asciiAtlas);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(points),points,GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER,0, sizeof(points),points);

    //attach
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE,0,0);
    glEnableVertexAttribArray(0);

	glDrawArrays(GL_TRIANGLE_FAN,0,4);

    //dettach
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);	
	
	//deleate
    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);

}
