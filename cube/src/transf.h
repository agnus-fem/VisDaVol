/*
 * transf.h
 *
 *  Created on: 31/10/2011
 *      Author: Agnus
 */

#ifndef TRANSF_H_
#define TRANSF_H_

#include <stdio.h>
#include <stdlib.h>

struct colorMap_item {
	int r,g,b;
	int alfa;
};

typedef struct colorMap_item COLORMAP_ITEM;
typedef COLORMAP_ITEM COLORMAP[256];

void colorMapRead(char *fn, COLORMAP cm);
void colorMapWrite(char *fn, COLORMAP cm);
void createColorMap(COLORMAP cm);

#endif /* TRANSF_H_ */
