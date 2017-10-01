/*

Map Drawer (2D) v1 by NaS

Draws a Bitmap of the World in 2D.

*/

#define FILTERSCRIPT

#include <a_samp>

// Plugins
#include <ColAndreas>
#include <streamer>

// Misc
#include <OC>

// Settings

#define GRAB_OBJECTS			false // Grabs streamer objects from specified world and temporarily creates them as CA Objects
#define GRAB_WORLD_ID			-1 // -1 = any world
#define MAX_COL_OBJECTS			25000

#define BMP_SIZE_X				1000 // X and Y must be even numbers
#define BMP_SIZE_Y				1000

#define COL_WEIGHT_STEEPNESS	1.0 // 0.0 - 1.0 | weight of terrain angle for colors
#define COL_WEIGHT_HEIGHT		1.0 // 0.0 - 1.0 | terrain height
#define COL_WEIGHT_CAT			1.0 // 0.0 - 1.0 | category color

#define BASE_COLOR_R			155 // Base color (if categories are disabled)
#define BASE_COLOR_G			155
#define BASE_COLOR_B			155

#define WATER_COLOR_R			0 // Color for water
#define WATER_COLOR_G			0
#define WATER_COLOR_B			0

// Other defines

#define BMP_SIZE_TOTAL	(BMP_SIZE_X * BMP_SIZE_Y)

// Vars, etc

new gBitmap[BMP_SIZE_TOTAL];

#if GRAB_OBJECTS == true
new gCAObjectIDs[MAX_COL_OBJECTS] = {-1, ...}, gNumCAObjectIDs = 0;
#endif

// Callbacks

public OnFilterScriptInit()
{
	CA_Init();
	
	printf("Drawing BMP with the size of "#BMP_SIZE_X"x"#BMP_SIZE_Y" px");

	#if GRAB_OBJECTS == true

		new Float:ox, Float:oy, Float:oz, Float:orx, Float:ory, Float:orz, model;

		for(new i = 0, j = Streamer_GetUpperBound(STREAMER_TYPE_OBJECT); i < j && gNumCAObjectIDs != MAX_COL_OBJECTS; i ++)
		{
			if(!IsValidDynamicObject(i) || (GRAB_WORLD_ID != -1 && !Streamer_IsInArrayData(STREAMER_TYPE_OBJECT, i, E_STREAMER_WORLD_ID, GRAB_WORLD_ID))) continue;

			GetDynamicObjectPos(i, ox, oy, oz);
			GetDynamicObjectRot(i, orx, ory, orz);
			model = Streamer_GetIntData(STREAMER_TYPE_OBJECT, i, E_STREAMER_MODEL_ID);

			gCAObjectIDs[gNumCAObjectIDs] = CA_CreateObject(model, ox, oy, oz, orx, ory, orz, true);

			gNumCAObjectIDs ++;
		}

	#endif

	DrawBitmap(-3050.0, -3050.0, -50.0, 3050.0, 3050.0, 550.0);
	WriteBitmap("MapDraw2D-Overview ["#BMP_SIZE_X"x"#BMP_SIZE_Y"].bmp");

	#if GRAB_OBJECTS == true

		if(gNumCAObjectIDs > 0) for(new i = 0; i < gNumCAObjectIDs; i ++) CA_DestroyObject(gCAObjectIDs[i]);
		gNumCAObjectIDs = 0;

	#endif

	return 1;
}

// I use seperate functions for setting/getting values of the Bitmap, so that it's easy to replace the Array with a Memory Plugin.
// That allows for bigger images and no endless compiling.

stock Bitmap_Get(x, y, &r, &g, &b)
{
	if(x < 0 || x >= BMP_SIZE_X || y < 0 || y >= BMP_SIZE_Y) return 0;
		
    new rgb;

    rgb = gBitmap[getid(x, y)];

    r = (rgb >> 16) & 255;
    g = (rgb >> 8) & 255;
    b = rgb & 255;
    
    return 1;
}

stock Bitmap_Set(x, y, r, g, b)
{
	if(x < 0 || x >= BMP_SIZE_X || y < 0 || y >= BMP_SIZE_Y) return 0;
		
	if(r > 255) r = 255;
	else if(r < 0) r = 0;

	if(g > 255) g = 255;
	else if(g < 0) g = 0;

	if(b > 255) b = 255;
	else if(b < 0) b = 0;

    new rgb = (r * 256 * 256) + (g * 256) + b;

    gBitmap[getid(x, y)] = rgb;
    
    return 1;
}

getid(x, y)
{
	return (y * BMP_SIZE_X) + x;
}

DrawBitmap(Float:minx = -3000.0, Float:miny = -3000.0, Float:minz = -50.0, Float:maxx = 3000.0, Float:maxy = 3000.0, Float:maxz = 550.0)
{
	new Float:px, Float:py, ret, Float:fwaste, Float:cz, Float:rx, Float:ry, Float:steepness, Float:height, r, g, b, count, ptick = GetTickCount();

	for(new x = 0; x < BMP_SIZE_X; x ++) for(new y = 0; y < BMP_SIZE_Y; y ++)
	{
		count ++;

		if(GetTickCount() - ptick > 10000)
		{
			ptick = GetTickCount();

			printf("%.1f%% ...", float(count) / float(BMP_SIZE_TOTAL) * 100.0);
		}

		px = minx + float(x) / float(BMP_SIZE_X) * (maxx - minx);
		py = miny + float(y) / float(BMP_SIZE_Y) * (maxy - miny);

		ret = CA_RayCastLineAngle(px, py, maxz, px, py, minz, fwaste, fwaste, cz, rx, ry, fwaste);

		if(ret == WATER_OBJECT)
		{
			Bitmap_Set(x, y, WATER_COLOR_R, WATER_COLOR_G, WATER_COLOR_B);

			continue;
		}

		steepness = COL_WEIGHT_STEEPNESS * ((rx == 0.0 && ry == 0.0) ? 0.0 : VectorSize(rx, ry, 0.0) / 90.0);

		height = COL_WEIGHT_HEIGHT * ((cz - minz) / (maxz - minz));

		if(GetModelColor(ret, r, g, b))
		{
			r = floatround((BASE_COLOR_R * (1.0 - COL_WEIGHT_CAT)) + (r * COL_WEIGHT_CAT));
			g = floatround((BASE_COLOR_G * (1.0 - COL_WEIGHT_CAT)) + (g * COL_WEIGHT_CAT));
			b = floatround((BASE_COLOR_B * (1.0 - COL_WEIGHT_CAT)) + (b * COL_WEIGHT_CAT));
		}
		else
		{
			r = BASE_COLOR_R;
			g = BASE_COLOR_G;
			b = BASE_COLOR_B;
		}

		r = floatround((float(r) + (r * steepness) + (r * height)) / (1.0 + COL_WEIGHT_STEEPNESS + COL_WEIGHT_HEIGHT));
		g = floatround((float(g) + (g * steepness) + (g * height)) / (1.0 + COL_WEIGHT_STEEPNESS + COL_WEIGHT_HEIGHT));
		b = floatround((float(b) + (b * steepness) + (b * height)) / (1.0 + COL_WEIGHT_STEEPNESS + COL_WEIGHT_HEIGHT));

		Bitmap_Set(x, y, r, g, b);
	}
}

WriteBitmap(const filename[ ], Float:sat = 1.0)
{
	if(strlen(filename) < 1) return 0;

	new File:FOut = fopen(filename, io_write);
	
	if(!FOut) return 0;
	
	new headers[13], extrabytes, paddedsize, x, y, n, r, g, b;
	
	extrabytes = 4 - ((BMP_SIZE_X * 3) % 4); // Calculate extra bytes. Must be multiple of 4, and not 4.
	if(extrabytes == 4) extrabytes = 0;
	
	paddedsize = ((BMP_SIZE_X * 3) + extrabytes) * BMP_SIZE_Y; // Calc padded size.
	
	headers[0] = paddedsize + 54; // File size
	headers[1] = 0;
	headers[2] = 54;
	headers[3] = 40;
	headers[4] = BMP_SIZE_X;
	headers[5] = BMP_SIZE_Y;
	
	headers[7]  = 0; // Compression
	headers[8]  = paddedsize; // Image Size
	headers[9]  = 0;
	headers[10] = 0;
	headers[11] = 0;
	headers[12] = 0;
	
	// ----- Write Headers
	
	fwrite(FOut, "BM");
	
	for(n = 0; n <= 5; n ++)
	{
	    fputchar(FOut, headers[n] & 0x000000FF, false);
	    fputchar(FOut, (headers[n] & 0x0000FF00) >> 8, false);
	    fputchar(FOut, (headers[n] & 0x00FF0000) >> 16, false);
	    fputchar(FOut, (headers[n] & 0xFF000000) >> 24, false);
	}

	fputchar(FOut, 1, false); // biPlanes & biBitCount
	fputchar(FOut, 0, false);
	fputchar(FOut, 24, false);
	fputchar(FOut, 0, false);
	
	for(n = 7; n <= 12; n ++)
	{
        fputchar(FOut, headers[n] & 0x000000FF, false);
	    fputchar(FOut, (headers[n] & 0x0000FF00) >> 8, false);
	    fputchar(FOut, (headers[n] & 0x00FF0000) >> 16, false);
	    fputchar(FOut, (headers[n] & 0xFF000000) >> 24, false);
	}
	
	// ----- Write Data
	
	for(y = 0; y < BMP_SIZE_Y; y ++)
	{
	    for(x = 0; x < BMP_SIZE_X; x ++)
	    {
	        Bitmap_Get(x, y, r, g, b);

	        r = floatround(r * sat);
	        g = floatround(g * sat);
	        b = floatround(b * sat);

	        if(r < 0) r = 0;
	        else if(r > 255) r = 255;

	        if(g < 0) g = 0;
	        else if(g > 255) g = 255;

	        if(b < 0) b = 0;
	        else if(b > 255) b = 255;

	        fputchar(FOut, b, false);
	        fputchar(FOut, g, false);
	        fputchar(FOut, r, false);
	    }
	    
	    if(extrabytes) for(n = 0; n < extrabytes; n ++) fputchar(FOut, 0, false);
	}
	
	fclose(FOut);

	printf("> Created Bitmap \"%s\"", filename);
	
	return 1;
}

GetModelColor(model, &r, &g, &b)
{
	new catid = GetModelCategoryID(model);

	if(catid == -1) return 0;

	switch(catid)
	{
		case OC_FACTORIES, OC_SHOPS, OC_GRAVEYARD, OC_HOUSES, OC_SKYSCRAPERS, OC_OTHER_BUILDINGS, OC_RESTAURANTS, OC_STADIUMS, OC_CLUBS:
		{
			r = 155; // Grey
			g = 155;
			b = 155;
		}
		case OC_CONCRETE_ROCK, OC_GRASS_DIRT, OC_ROCKS:
		{
			r = 160; // Brown
			g = 100;
			b = 20;
		}
		case OC_TREES, OC_PLANTS:
		{
			r = 30; // Green
			g = 180;
			b = 50;
		}
		case OC_ROADS, OC_ROAD_ITEMS, OC_CAR_PARKS, OC_GARAGES:
		{
			r = 255;
			g = 0;
			b = 0;
		}
		case OC_RAILROADS:
		{
			r = 0;
			g = 0;
			b = 255;
		}
		case OC_AIRPORT:
		{
			r = 155;
			g = 155;
			b = 155;
		}
		case OC_GENERAL_BEACH:
		{
			r = 255;
			g = 248;
			b = 188;
		}
		case OC_FENCES:
		{
			r = 190;
			g = 150;
			b = 150;
		}

		default:
		{
			return 0;
		}
	}

	return 1;
}