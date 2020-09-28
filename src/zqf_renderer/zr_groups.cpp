#ifndef ZR_GROUPS_CPP
#define ZR_GROUPS_CPP

#include "zr_groups.h"

internal void ZDraw_FindLightsForObject(
    ZRDrawObj* objects, i32 numObjects,
    ZRDrawObj* obj, ZRSceneView* view,
    ZRDrawObjLightData* lights)
{
	*lights = {};
    // TODO: No object <-> light grouping: Iterating all lights for all objects
    // Pick top four lights!
    i32 lightsAdded = 0;
    for (i32 i = 0; i < view->numLights; ++i)
    {
		i32 lightIndex = view->lights[i];
        ZRDrawObj* lightObj = &objects[lightIndex];
        Vec3 potentialPos = lightObj->t.pos;
        f32 potentialDist = Vec3_Distance(potentialPos, obj->t.pos);
        if (lightsAdded < ZR_MAX_POINT_LIGHTS_PER_MODEL)
        {
            // easy case, add light and carry on
            lights->pointPositions[lightsAdded] = potentialPos;
            lights->colours[lightsAdded].x = lightObj->data.pointLight.colour.r;
            lights->colours[lightsAdded].y = lightObj->data.pointLight.colour.g;
            lights->colours[lightsAdded].z = lightObj->data.pointLight.colour.b;
            lights->distances[lightsAdded] = potentialDist;
            f32 multiplier = lightObj->data.pointLight.multiplier;
            f32 range = lightObj->data.pointLight.range;
            lights->settings[lightsAdded] = { multiplier, range, 0, 0 };
            lightsAdded++;
            continue;
        }
        // harder case. Find a light to replace
        i32 replaceIndex = -1;
        f32 replaceDistance = 0;
        for (i32 j = 0; j < ZR_MAX_POINT_LIGHTS_PER_MODEL; ++j)
        {
            f32 dist = lights->distances[j];
            // is the potential light closer?
            if (potentialDist < dist)
            {
                // we may already have a light to replace. Is this a better candidate?
                if (dist > replaceDistance)
                {
                    replaceIndex = j;
                    replaceDistance = dist;
                }
            }
        }
        if (replaceIndex >= 0)
        {
            lights->pointPositions[replaceIndex] = potentialPos;
            lights->colours[replaceIndex].x = lightObj->data.pointLight.colour.r;
            lights->colours[replaceIndex].y = lightObj->data.pointLight.colour.g;
            lights->colours[replaceIndex].z = lightObj->data.pointLight.colour.b;
            lights->distances[replaceIndex] = potentialDist;
            f32 multiplier = lightObj->data.pointLight.multiplier;
            f32 range = lightObj->data.pointLight.range;
            lights->settings[replaceIndex] = { multiplier, range, 0, 0 };
        }
    }
    // Patch empty lights
    for (i32 i = lightsAdded; i < ZR_MAX_POINT_LIGHTS_PER_MODEL; ++i)
    {
        lights->settings[i].x = 1;
    }
}

///////////////////////////////////////////////////////
// External
///////////////////////////////////////////////////////

extern "C" Point2 ZR_IndexToPixel(int index, int imageWidth)
{
    return {
        index % imageWidth,
        int(index / imageWidth)
    };
}

extern "C" i32 ZR_PixelToIndex(i32 x, i32 y, int imageWidth)
{
    return x + (y * imageWidth);
}

extern "C" void ZR_WriteGroupsToTextureByIndex(
    ZRDrawObj* objects, i32 numObjects,
    Transform* camT, ZRSceneView* groups, 
    ZRDataTexture* tex)
{
	for (i32 i = 0; i < groups->numGroups; ++i)
    {
        ZRDrawGroup* group = groups->groups[i];
        
        if (group->bBatchable == NO)
        {
            continue;
        }
        
		// Record in the group where its data starts
        group->dataPixelIndex = tex->cursor;
        group->pixelsPerItem = ZR_BATCH_DATA_STRIDE;
		
        for (i32 j = 0; j < group->numItems; ++j)
        {
            i32 objIndex = group->indices[j];
            ZRDrawObj* obj = &objects[objIndex];
            
			// Write ModelView data
			i32 indexCheckStart = tex->cursor;
			
			// Build model view matrix
			M4x4_CREATE(model)
            M4x4_CREATE(view)
            M4x4_CREATE(modelView)

			ZR_BuildModelMatrix(&model, &obj->t);

			// Separate view is used for moving lights into view space
			
			ZR_BuildViewMatrix(&view, camT);
			M4x4_Multiply(modelView.cells, view.cells, modelView.cells);
			M4x4_Multiply(modelView.cells, model.cells, modelView.cells);

			// Write model view pixels
			tex->mem[tex->cursor++] = modelView.xAxis;
			tex->mem[tex->cursor++] = modelView.yAxis;
			tex->mem[tex->cursor++] = modelView.zAxis;
			tex->mem[tex->cursor++] = modelView.wAxis;
			

			// Write lighting data
            ZRDrawObjLightData objLights = {};
            ZDraw_FindLightsForObject(objects, numObjects, obj, groups, &objLights);

            // Ambient
            tex->mem[tex->cursor++] = { 0, 0, 0, 1 };
            for (i32 k = 0; k < ZR_MAX_POINT_LIGHTS_PER_MODEL; ++k)
            {
				Vec3 worldPos = objLights.pointPositions[k];
				Vec3 colour = objLights.colours[k];
                Vec4 settings = objLights.settings[k];
                // TODO: light radius of zero causes a divide by 0 in shader
                // make sure all lights have a positive radius before copying in
                ZE_ASSERT(settings.x > 0, "Light has a radius of 0!");
				Vec3 viewPos = Vec3_MultiplyByM4x4(&worldPos, view.cells);

                tex->mem[tex->cursor++] = COM_Vec3ToVec4(viewPos, 1);
                tex->mem[tex->cursor++] = COM_Vec3ToVec4(objLights.colours[k], 1);
                tex->mem[tex->cursor++] = settings;
            }
			i32 indexCheckEnd = tex->cursor;
			ZE_ASSERT(indexCheckEnd - indexCheckStart == ZR_BATCH_DATA_STRIDE,
				"Did not write correct data pixel count")
        }
    }
}

extern "C" ZRSceneView* ZR_BuildDrawGroups(
    ZRDrawObj* objects, i32 numObjects, ZEBuffer* scratch, ZRGroupingStats* stats)
{
    ZRSceneView* drawGroups = (ZRSceneView*)scratch->cursor;
    scratch->cursor += sizeof(ZRSceneView);
    
	// NOTE: Scratch buffer will not have been cleared to zero etc,
	// so garbage will be there instead!
	drawGroups->numGroups = 0;
	drawGroups->numLights = 0;

    // TODO: linear search, no culling!
    for (i32 i = 0; i < numObjects; ++i)
    {
        ZRDrawObj* obj = &objects[i];
        u32 objHash = obj->CalcHash();
        i32 objType = obj->data.type;

        // Most common primitive
        if (objType == ZR_DRAWOBJ_TYPE_MESH
			|| objType == ZR_DRAWOBJ_TYPE_TEXT)
        {
            // Find a group for this object
            // TODO: sort objects and keep current group around?
            ZRDrawGroup* group = NULL;
            for (i32 j = 0; j < drawGroups->numGroups; ++j)
            {
                ZRDrawGroup* potentialGroup = drawGroups->groups[j];
                // TODO: Groups have a maximum size. Can this be changed?
                if (objHash == potentialGroup->hash
                    && potentialGroup->numItems < ZR_MAX_BATCH_SIZE)
                {
                    group = potentialGroup;
                    break;
                }
            }
            if (group == NULL)
            {
                // Create a new group
                group = (ZRDrawGroup*)scratch->cursor;
                group->hash = objHash;
                group->data = obj->data;
		    	group->numItems = 0;
                if (objType == ZR_DRAWOBJ_TYPE_MESH)
                {
                    group->bBatchable = YES;
                }
                //group->id = objGroupId;
                scratch->cursor += sizeof(ZRDrawGroup);
                drawGroups->groups[drawGroups->numGroups++] = group;
            }
            group->indices[group->numItems++] = i;
        }
        else if (objType == ZR_DRAWOBJ_TYPE_BILLBOARD)
        {
            // TODO
            continue;
        }
        // lights have their own groups
        else if (objType == ZR_DRAWOBJ_TYPE_POINT_LIGHT || objType == ZR_DRAWOBJ_TYPE_DIRECT_LIGHT)
        {
            i32 lightIndex = drawGroups->numLights++;
            ZE_ASSERT(lightIndex < ZR_MAX_DRAW_GROUPS, "Too many lights for draw groups")
            drawGroups->lights[lightIndex] = i;
            continue;
        }
        else if (objType == ZR_DRAWOBJ_TYPE_NONE)
        {
            // ignore this object
            continue;
        }
        else
        {
            // Unknown type... no idea what to do...
			#ifdef ZR_REPORT_GROUP_ERRORS
            printf("ZR no grouping for drawobj type %d\n", objType);
			#endif
        }
    }
    return drawGroups;
}

#endif // ZR_GROUPS_CPP