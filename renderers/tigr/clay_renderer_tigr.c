// Copyright (c) 2024 Justin Andreas Lacoste (@27justin)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the
// use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
//     1. The origin of this software must not be misrepresented; you must not
//     claim that you wrote the original software. If you use this software in a
//     product, an acknowledgment in the product documentation would be
//     appreciated but is not required.
//
//     2. Altered source versions must be plainly marked as such, and must not
//     be misrepresented as being the original software.
//
//     3. This notice may not be removed or altered from any source
//     distribution.
//
// SPDX-License-Identifier: Zlib

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "../../clay.h"

#include "tigr.h"
#include "utils.h"  // for our rounded borders

// Render the command queue to the `Tigr*` instance provided
void Clay_Tigr_Render(Clay_RenderCommandArray commands, Tigr* ctx);

#define CLAY_TO_TPIXEL(color) (TPixel){color.r, color.g, color.b, color.a}

/* Return a null-terminated copy of Clay_String `str` 
 * caller is required to free
 * */
static inline char *Clay_to_Cstr(Clay_String *str) {
	char* copy = (char*) malloc(str->length + 1);
	if (!copy) {
		fprintf(stderr, "Memory allocation failed\n");
		return NULL;
	}
	memcpy(copy, str->chars, str->length);
	copy[str->length] = '\0';
	return copy;
}

// Measure text using built-in Tigr functions
static inline Clay_Dimensions Clay_Tigr_MeasureText(Clay_StringSlice str, Clay_TextElementConfig *config, void *userData) {

    Clay_String toTerminate = (Clay_String){ .chars = str.chars, .length = str.length, .isStaticallyAllocated = false };

    char* cstr = Clay_to_Cstr(&toTerminate); 

    int text_width  = tigrTextWidth(tfont, cstr);
    int text_height = tigrTextHeight(tfont, cstr);

    free(cstr);

	// Return dimensions
	return (Clay_Dimensions){
		.width =  (float)text_width,
		.height = (float)text_height
	};
}

void Clay_Tigr_Render(Clay_RenderCommandArray commands, Tigr* ctx) {
	for(size_t i = 0; i < commands.length; i++) {
		Clay_RenderCommand *command = Clay_RenderCommandArray_Get(&commands, i);

		switch(command->commandType) {
		case CLAY_RENDER_COMMAND_TYPE_RECTANGLE: {
            Clay_RectangleRenderData* config = &command->renderData.rectangle;
			Clay_BoundingBox bb = command->boundingBox;

            int box_radius = config->cornerRadius.topLeft;  // only take take top left radius

            tigrFillArcRect(ctx, bb.x, bb.y, bb.width, bb.height, box_radius, CLAY_TO_TPIXEL(config->backgroundColor));

			break;
		}
		case CLAY_RENDER_COMMAND_TYPE_TEXT: {
            Clay_TextRenderData* config = &command->renderData.text;
            Clay_String toTerminate = (Clay_String){ .chars = config->stringContents.chars, .length = config->stringContents.length, .isStaticallyAllocated = false };
			char* text = Clay_to_Cstr(&toTerminate);

			Clay_BoundingBox bb = command->boundingBox;
			Clay_Color color = config->textColor;

			//cairo_set_font_size(cr, config->fontSize);
            tigrPrint(ctx, tfont, bb.x, bb.y, CLAY_TO_TPIXEL(color), text);

			free(text);
			break;
		}
		case CLAY_RENDER_COMMAND_TYPE_BORDER: {
            Clay_BorderRenderData* config = &command->renderData.border;
			Clay_BoundingBox bb = command->boundingBox;

			int box_radius = config->cornerRadius.topLeft;

            tigrArcRect(ctx, bb.x, bb.y, bb.width, bb.height, box_radius, CLAY_TO_TPIXEL(config->color));
			
			break;
		}
		case CLAY_RENDER_COMMAND_TYPE_IMAGE: {
            Clay_ImageRenderData *config = &command->renderData.image;
			Clay_BoundingBox bb = command->boundingBox;

			char* path = config->imageData;

			Tigr* img = tigrLoadImage(path);

			double image_w = img->w;
            double image_h = img->h;

			/* Calculate the scaling factor to fit within the bounding box while preserving aspect ratio */
			double scale_w = bb.width / image_w;
			double scale_h = bb.height / image_h;
			double scale = (scale_w < scale_h) ? scale_w : scale_h; // Use the smaller scaling factor

			/* Apply the same scale to both dimensions to preserve aspect ratio */
			double scale_x = scale;
			double scale_y = scale;

			/* Calculate the scaled image dimensions */
			double scaled_w = image_w * scale_x;
			double scaled_h = image_h * scale_y;

			/* Adjust the x and y coordinates to center the scaled image within the bounding box */
			double centered_x = bb.x + (bb.width - scaled_w) / 2.0;
			double centered_y = bb.y + (bb.height - scaled_h) / 2.0;

			/* Blit the scaled and centered image */
            tigrBlit(ctx, img, centered_x, centered_y, 0, 0, scaled_w, scaled_h);

			/* Clean up the source surface */
            tigrFree(img);
			break;
		}
        case CLAY_RENDER_COMMAND_TYPE_SCISSOR_START: {
			Clay_BoundingBox bb = command->boundingBox;
            tigrClip(ctx, bb.x, bb.y, bb.width, bb.height);
            break;
        }
        case CLAY_RENDER_COMMAND_TYPE_SCISSOR_END: {
            tigrClip(ctx, 0, 0, -1, -1);
            break;
        }
		case CLAY_RENDER_COMMAND_TYPE_CUSTOM: {
			// Slot your custom elements in here.
		}
		default: {
			fprintf(stderr, "Unknown command type %d\n", (int) command->commandType);
		}
		}
	}
}
