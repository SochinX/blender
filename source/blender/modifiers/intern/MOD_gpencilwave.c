/*
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software  Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Original Code is Copyright (C) 2005 by the Blender Foundation.
 * All rights reserved.
 *
 * Contributor(s): Antonio Vazquez
 *
 * ***** END GPL LICENSE BLOCK *****
 *
 */

/** \file blender/modifiers/intern/MOD_gpencilwave.c
 *  \ingroup modifiers
 */

#include <stdio.h>

#include "DNA_scene_types.h"
#include "DNA_object_types.h"
#include "DNA_gpencil_types.h"

#include "BLI_utildefines.h"
#include "BKE_DerivedMesh.h"
#include "BKE_gpencil.h"

#include "MOD_modifiertypes.h"

static void initData(ModifierData *md)
{
	GpencilWaveModifierData *gpmd = (GpencilWaveModifierData *)md;
	gpmd->amplitude = 6.0f;
	gpmd->period = 0.2f;
	gpmd->phase = 11.0f;
	gpmd->orientation = 1;
	
	BKE_gpencil_batch_cache_alldirty();
}

ModifierTypeInfo modifierType_GpencilWave = {
	/* name */              "Wave Distorsion",
	/* structName */        "GpencilWaveModifierData",
	/* structSize */        sizeof(GpencilWaveModifierData),
	/* type */             	eModifierTypeType_Gpencil,
	/* flags */             eModifierTypeFlag_GpencilMod | eModifierTypeFlag_SupportsEditmode 
							| eModifierTypeFlag_GpencilVFX | eModifierTypeFlag_Single,

	/* copyData */          NULL,
	/* deformVerts */       NULL,
	/* deformMatrices */    NULL,
	/* deformVertsEM */     NULL,
	/* deformMatricesEM */  NULL,
	/* applyModifier */     NULL,
	/* applyModifierEM */   NULL,
	/* initData */          initData,
	/* requiredDataMask */  NULL,
	/* freeData */          NULL,
	/* isDisabled */        NULL,
	/* updateDepsgraph */   NULL,
	/* dependsOnTime */     NULL,
	/* dependsOnNormals */	NULL,
	/* foreachObjectLink */ NULL,
	/* foreachIDLink */     NULL,
	/* foreachTexLink */    NULL,
};
