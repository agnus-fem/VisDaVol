/*
 * transf.cpp
 *
 *  Created on: 01/11/2011
 *      Author: Agnus
 */

#include "transf.h"

//! Esta função realiza a leitura de um mapa de cores que representa a função de transferência
/*!
/*!
* \author Agnus A. Horta.
* \since 02/11/2011
* \version 1.0
* \param fn uma cadeia de caracteres que representa o nome do arquivo que contém o mapa de cores
* \param cm representa o ponteiro que indica onde será armazenado o mapa de cores
*/
void colorMapRead(char *fn, COLORMAP cm)
{
	FILE *fp;

	int count=0;

	if (fn != NULL) {

		fp = fopen(fn,"rb");

		if (fp != NULL) {

			fseek(fp, 0, SEEK_END);
			count = ftell(fp)/sizeof(COLORMAP_ITEM);
			rewind(fp);

			if (count > 0) {
				//cm = (COLORMAP *)malloc(sizeof(COLORMAP_ITEM) * (count));
				count = fread(cm,sizeof(COLORMAP_ITEM),count,fp);
			}

			fclose(fp);

		}
	}

}

//! Esta função realiza escrita de um mapa de cores que representa a função de transferência
/*!
/*!
* \author Agnus A. Horta.
* \since 02/11/2011
* \version 1.0
* \param fn uma cadeia de caracteres que representa o nome do arquivo que conterá o mapa de cores
* \param cm representa o ponteiro que indica onde esta armazenado o mapa de cores
*/

void colorMapWrite(char *fn, COLORMAP cm)
{
	FILE *fp;

	int count=255;

	if (fn != NULL) {

		fp = fopen(fn,"wb");

		if (fp != NULL) {

			count = fwrite(cm,count,sizeof(COLORMAP_ITEM),fp);

			fclose(fp);

		}
	}

}

//! Esta função cria um mapa de cores que representa a função de transferência
/*!
/*!
* \author Agnus A. Horta.
* \since 25/11/2011
* \version 1.0
* \param cm representa o ponteiro que indica onde esta armazenado o mapa de cores
*/

void createColorMap (COLORMAP cm)
{
	int i, cr, cg, cb, a;

	for(i=0;i<255;i++) {

		//rgb
		if(i>130 && i<180) {
			cr=i;
			cg=i;
			cb=0;
		} else if(i>90 && i<130) {
			cr=i;
			cg=i/3;
			cb=i/2;
		} else {
		    cr=i;
			cg=i;
			cb=i;
	    }

		//alpha
		if(i>15) a=0x20;
        else a=0x00;

        cm[i].r = cr;    // red
        cm[i].g = cg;    // green
        cm[i].b = cb;    // blue
        cm[i].alfa = a;	  // alfa

	}

}
