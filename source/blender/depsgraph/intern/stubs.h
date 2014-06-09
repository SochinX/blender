/* This file defines some stub function defs used 
 * for various operation callbacks which have not
 * been implemented in Blender yet. It should
 * eventually disappear instead of remaining as
 * part of the code base.
 */

#ifndef __DEPSGRAPH_FN_STUBS_H__
#define __DEPSGRAPH_FN_STUBS_H__

#pragma message("DEPSGRAPH PORTING XXX: There are still some undefined stubs")

void BKE_animsys_eval_driver();

void BKE_constraints_evaluate();
void BKE_pose_iktree_evaluate();
void BKE_pose_splineik_evaluate();
void BKE_pose_eval_bone();

void BKE_pose_rebuild_op();
void BKE_pose_eval_init();
void BKE_pose_eval_flush();

void BKE_particle_system_eval();

void BKE_rigidbody_rebuild_sim(); // BKE_rigidbody_rebuild_sim
void BKE_rigidbody_eval_simulation(); // BKE_rigidbody_do_simulation
void BKE_rigidbody_object_sync_transforms(); // BKE_rigidbody_sync_transforms

void BKE_object_eval_local_transform();
void BKE_object_eval_parent();
void BKE_object_eval_modifier();

void BKE_mesh_eval_geometry();  // wrapper around makeDerivedMesh() - which gets BMesh, etc. data...
void BKE_mball_eval_geometry(); // BKE_displist_make_mball
void BKE_curve_eval_geometry(); // BKE_displist_make_curveTypes
void BKE_curve_eval_path();
void BKE_lattice_eval_geometry(); // BKE_lattice_modifiers_calc

#endif //__DEPSGRAPH_FN_STUBS_H__

