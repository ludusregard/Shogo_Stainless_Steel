
#include "plasma.h"


char g_SinTable[256];
DBYTE *g_DistanceGrid;

int g_PlasmaSizeX = 20;
int g_PlasmaSizeY = 20;


void cs_PrecalculateData(ClientDE *pClientDE, HLOCALOBJ hPolyGrid)
{
	DDWORD width, height;
	int halfWidth, halfHeight, x, y;
	char *pData;
	DVector *pColorTable;
	float scale, val, maxDistSqr, testDist, t;
	int i;


	// Create the (scaled) sin table.
	scale = (MATH_CIRCLE / 255.0f) * 3.0f;
	for(i=0; i < 256; i++)
	{
		val = (float)i * scale;
		g_SinTable[i] = (char)(sin(val) * 128.0f);
	}


	// Fill in the distance grid.
	pClientDE->GetPolyGridInfo(hPolyGrid, &pData, &width, &height, &pColorTable);


	g_DistanceGrid = (DBYTE*)malloc(width*height);
	halfWidth = width >> 1;
	halfHeight = height >> 1;
	maxDistSqr = (float)(halfWidth*halfWidth + halfHeight*halfHeight) + 1.0f;
	for(y=0; y < (int)height; y++)
	{
		for(x=0; x < (int)width; x++)
		{
			testDist = (float)((x-halfWidth)*(x-halfWidth) + (y-halfHeight)*(y-halfHeight));
			t = testDist / maxDistSqr;
			g_DistanceGrid[y*height+x] = (DBYTE)(t * 255.0f);
		}
	}
}

float g_fCount=0.0f, g_Dir;

void cs_RandomizePolyGrid(ClientDE *pClientDE, HLOCALOBJ hPolyGrid)
{
	DDWORD width, height, x, y;
	char *pData, *pCur;
	DVector *pColorTable;
	DBYTE count;

	g_fCount += 50.0f * g_Dir * pClientDE->GetFrameTime();
	if(g_fCount >= 254.0f)
		g_Dir = -1.0f;
	else if(g_fCount <= 1.0f)
		g_Dir = 1.0f;

	count = (DBYTE)g_fCount;

	// Randomize the poly grid values.
	pClientDE->GetPolyGridInfo(hPolyGrid, &pData, &width, &height, &pColorTable);
	for(y=0; y < height; y++)
	{
		pCur = pData + (y*width);

		for(x=0; x < width; x++)
		{
			*pCur++ = g_SinTable[g_DistanceGrid[y*width+x]+count];
		}
	}
}


typedef struct
{
	DVector m_Color;
	int m_Index;
} ColorRamp;


ColorRamp g_ColorRamps[] = 
{
	0.0f, 0.0f, 0.0f, 0,
	0.0f, 0.0f, 255.0f, 128,
	255.0f, 255.0f, 255.0f, 256,
};

#define NUM_COLORRAMPS() (sizeof(g_ColorRamps) / sizeof(ColorRamp))


void cs_SetPolyGridPalette(ClientDE *pClientDE, HLOCALOBJ hPolyGrid)
{
	float t;
	DDWORD width, height;
	char *pData;
	DVector *pColorTable, color1, color2;
	int ramp, i, index1, index2;

	// Randomize the poly grid values.
	pClientDE->GetPolyGridInfo(hPolyGrid, &pData, &width, &height, &pColorTable);

	// Make the color table go from white to black.
	for(ramp=0; ramp < NUM_COLORRAMPS()-1; ramp++)
	{
		index1 = g_ColorRamps[ramp].m_Index;
		index2 = g_ColorRamps[ramp+1].m_Index;
		VEC_COPY(color1, g_ColorRamps[ramp].m_Color);
		VEC_COPY(color2, g_ColorRamps[ramp+1].m_Color);

		for(i=index1; i < index2; i++)
		{
			t = (float)(i - index1) / (index2 - index1);
			VEC_LERP(pColorTable[i], color1, color2, t);
		}
	}
}


