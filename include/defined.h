#pragma once

const static int period = 120;
const static int radix  = 5;
const static int pieces = 40;
const static int pieces_max = 40;
const static float zoomScale = 0.1;
const static unsigned int mills = 20;

const static unsigned int resizeArea = 2;
const static unsigned int programListMin = 10;

#define CURSOR_STATUS_IDLE	     (0)
#define CURSOR_STATUS_VRESIZE_IDLE   (1)
#define CURSOR_STATUS_VRESIZE_ACTIVE (2)
