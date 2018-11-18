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
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Original Code is Copyright (C) 2013 Blender Foundation
 * All rights reserved.
 *
 * The Original Code is: all of this file.
 *
 * Contributor(s): Joshua Leung, Sergej Reich
 *
 * ***** END GPL LICENSE BLOCK *****
 */

/** \file DNA_rigidbody_types.h
 *  \ingroup DNA
 *  \brief Types and defines for representing Rigid Body entities
 */

#ifndef __DNA_RIGIDBODY_TYPES_H__
#define __DNA_RIGIDBODY_TYPES_H__

#include "DNA_listBase.h"

struct Collection;

struct EffectorWeights;

/* ******************************** */
/* RigidBody World */

/* Container for data shared by original and evaluated copies of RigidBodyWorld */
typedef struct RigidBodyWorld_Shared {
	/* cache */
	struct PointCache *pointcache;
	struct ListBase ptcaches;

	/* References to Physics Sim objects. Exist at runtime only ---------------------- */
	void *physics_world;		/* Physics sim world (i.e. btDiscreteDynamicsWorld) */

} RigidBodyWorld_Shared;

/* RigidBodyWorld (rbw)
 *
 * Represents a "simulation scene" existing within the parent scene.
 */
struct RigidBodyOb;

typedef struct RigidBodyWorld {
	/* Sim World Settings ------------------------------------------------------------- */
	struct EffectorWeights *effector_weights; /* effectors info */

	struct Collection *group;		/* Group containing objects to use for Rigid Bodies */
	struct Object **objects;		/* Array to access group objects by index, only used at runtime */

	struct Collection *constraints;	/* Group containing objects to use for Rigid Body Constraints*/

	int pad;
	float ltime;				/* last frame world was evaluated for (internal) */

	struct RigidBodyWorld_Shared *shared; /* This pointer is shared between all evaluated copies */
	struct PointCache *pointcache DNA_DEPRECATED; /* Moved to shared->pointcache */
	struct ListBase ptcaches DNA_DEPRECATED; /* Moved to shared->ptcaches */
	int numbodies;				/* number of objects in rigid body group */

	short steps_per_second;		/* number of simulation steps thaken per second */
	short num_solver_iterations;/* number of constraint solver iterations made per simulation step */

	int flag;					/* (eRigidBodyWorld_Flag) settings for this RigidBodyWorld */
	float time_scale;			/* used to speed up or slow down the simulation */
} RigidBodyWorld;

/* Flags for RigidBodyWorld */
typedef enum eRigidBodyWorld_Flag {
	/* should sim world be skipped when evaluating (user setting) */
	RBW_FLAG_MUTED				= (1 << 0),
	/* sim data needs to be rebuilt */
	RBW_FLAG_NEEDS_REBUILD		= (1 << 1),
	/* use split impulse when stepping the simulation */
	RBW_FLAG_USE_SPLIT_IMPULSE	= (1 << 2),
	/* Flag changes to objects (especially those with modifiers)*/
	RBW_FLAG_OBJECT_CHANGED		= (1 << 3),
	/* If we have rigidbody modifiers, time to refresh them if flag is set*/
	RBW_FLAG_REFRESH_MODIFIERS	= (1 << 4),
	/* Flag rebuild of constraints in fracture modifier objects */
	RBW_FLAG_REBUILD_CONSTRAINTS = (1 << 5),
	/* Visualize physics objects like in game engine*/
	RBW_FLAG_VISUALIZE_PHYSICS = (1 << 6),
} eRigidBodyWorld_Flag;

/* ******************************** */
/* RigidBody Object */

/* Container for data that is shared among CoW copies.
 *
 * This is placed in a separate struct so that, for example, the physics_shape
 * pointer can be replaced without having to update all CoW copies. */
#
#
typedef struct RigidBodyOb_Shared {
	/* References to Physics Sim objects. Exist at runtime only */
	void *physics_object;	/* Physics object representation (i.e. btRigidBody) */
	void *physics_shape;	/* Collision shape used by physics sim (i.e. btCollisionShape) */
} RigidBodyOb_Shared;

/* RigidBodyObject (rbo)
 *
 * Represents an object participating in a RigidBody sim.
 * This is attached to each object that is currently
 * participating in a sim.
 */
typedef struct RigidBodyOb {
	/* General Settings for this RigidBodyOb */
	short type;				/* (eRigidBodyOb_Type) role of RigidBody in sim  */
	short shape;			/* (eRigidBody_Shape) collision shape to use */

	int is_fractured;
	int flag;				/* (eRigidBodyOb_Flag) */
	int col_groups;			/* Collision groups that determines which rigid bodies can collide with each other */
	short mesh_source;		/* (eRigidBody_MeshSource) mesh source for mesh based collision shapes */
	short pad;

	/* Physics Parameters */
	float mass;				/* how much object 'weighs' (i.e. absolute 'amount of stuff' it holds) */

	float friction;			/* resistance of object to movement */
	float restitution;		/* how 'bouncy' object is when it collides */

	float margin;			/* tolerance for detecting collisions */

	float lin_damping;		/* damping for linear velocities */
	float ang_damping;		/* damping for angular velocities */

	float lin_sleep_thresh;	/* deactivation threshold for linear velocities */
	float ang_sleep_thresh;	/* deactivation threshold for angular velocities */

	float orn[4];			/* rigid body orientation */
	float pos[3];			/* rigid body position */

	float lin_vel[3];
	float ang_vel[3];

	float force_thresh;
	char pad2[4];

	struct RigidBodyOb_Shared *shared; /* This pointer is shared between all evaluated copies */

} RigidBodyOb;


/* Participation types for RigidBodyOb */
typedef enum eRigidBodyOb_Type {
	/* active geometry participant in simulation. is directly controlled by sim */
	RBO_TYPE_ACTIVE	= 0,
	/* passive geometry participant in simulation. is directly controlled by animsys */
	RBO_TYPE_PASSIVE
} eRigidBodyOb_Type;

/* Flags for RigidBodyOb */
typedef enum eRigidBodyOb_Flag {
	/* rigidbody is kinematic (controlled by the animation system) */
	RBO_FLAG_KINEMATIC			= (1 << 0),
	/* rigidbody needs to be validated (usually set after duplicating and not hooked up yet) */
	RBO_FLAG_NEEDS_VALIDATE		= (1 << 1),
	/* rigidbody shape needs refreshing (usually after exiting editmode) */
	RBO_FLAG_NEEDS_RESHAPE		= (1 << 2),
	/* rigidbody can be deactivated */
	RBO_FLAG_USE_DEACTIVATION	= (1 << 3),
	/* rigidbody is deactivated at the beginning of simulation */
	RBO_FLAG_START_DEACTIVATED	= (1 << 4),
	/* rigidbody is not dynamically simulated */
	RBO_FLAG_DISABLED			= (1 << 5),
	/* collision margin is not embedded (only used by convex hull shapes for now) */
	RBO_FLAG_USE_MARGIN			= (1 << 6),
	/* collision shape deforms during simulation (only for passive triangle mesh shapes) */
	RBO_FLAG_USE_DEFORM			= (1 << 7),
	/* rebuild object after collision, (change kinematic state) */
	RBO_FLAG_KINEMATIC_REBUILD	= (1 << 8),
	/* enable / disable kinematic state change after collision */
	RBO_FLAG_IS_TRIGGERED = (1 << 9),
	/* ghost flag, do not collide with object (but can activate although) */
	RBO_FLAG_IS_GHOST = (1 << 10),
	/* trigger flag, trigger kinematic state change on other objects */
	RBO_FLAG_IS_TRIGGER = (1 << 11),
	/* propagate trigger flag, pass the trigger impulse through to other objects nearby / touched */
	RBO_FLAG_PROPAGATE_TRIGGER = (1 << 12),
	/* dissolve constraints on activated shards */
	RBO_FLAG_CONSTRAINT_DISSOLVE = (1 << 13),
	/* trigger a dynamic fracture with this type */
	RBO_FLAG_DYNAMIC_TRIGGER = (1 << 14),
	/* dissolve plastic constraints too (if any) */
	RBO_FLAG_PLASTIC_DISSOLVE = (1 << 15),
	/* anti (stop) trigger flag, make simulated objects kinematic again */
	RBO_FLAG_ANTI_TRIGGER = (1 << 16),
	/* marks bound kinematic rigidbodies (to properly handle restoreKinematic for them) */
	RBO_FLAG_KINEMATIC_BOUND = (1 << 17),

} eRigidBodyOb_Flag;

/* RigidBody Collision Shape */
typedef enum eRigidBody_Shape {
		/* simple box (i.e. bounding box) */
	RB_SHAPE_BOX = 0,
		/* sphere */
	RB_SHAPE_SPHERE,
		/* rounded "pill" shape (i.e. calcium tablets) */
	RB_SHAPE_CAPSULE,
		/* cylinder (i.e. pringles can) */
	RB_SHAPE_CYLINDER,
		/* cone (i.e. party hat) */
	RB_SHAPE_CONE,

		/* convex hull (minimal shrinkwrap encompassing all verts) */
	RB_SHAPE_CONVEXH,
		/* triangulated mesh */
	RB_SHAPE_TRIMESH,

	/* concave mesh approximated using primitives */
	RB_SHAPE_COMPOUND,

} eRigidBody_Shape;

typedef enum eRigidBody_MeshSource {
	/* base mesh */
	RBO_MESH_BASE = 0,
	/* only deformations */
	RBO_MESH_DEFORM,
	/* final derived mesh */
	RBO_MESH_FINAL,
	/* solidifed final mesh (physics only) */
	RBO_MESH_FINAL_SOLID
} eRigidBody_MeshSource;

/* ******************************** */
/* RigidBody Constraint */

/* RigidBodyConstraint (rbc)
 *
 * Represents an constraint connecting two rigid bodies.
 */
typedef struct RigidBodyCon {
	struct Object *ob1;			/* First object influenced by the constraint */
	struct Object *ob2;			/* Second object influenced by the constraint */

	/* General Settings for this RigidBodyCon */
	short type;					/* (eRigidBodyCon_Type) role of RigidBody in sim  */
	short num_solver_iterations;/* number of constraint solver iterations made per simulation step */

	int flag;					/* (eRigidBodyCon_Flag) */

	float breaking_threshold;	/* breaking impulse threshold */
    char spring_type;       	/* spring implementation to use */
    char name[66];
    char pad[5];

    float orn[4];
    float pos[3];

	/* limits */
	/* translation limits */
	float limit_lin_x_lower;
	float limit_lin_x_upper;
	float limit_lin_y_lower;
	float limit_lin_y_upper;
	float limit_lin_z_lower;
	float limit_lin_z_upper;
	/* rotation limits */
	float limit_ang_x_lower;
	float limit_ang_x_upper;
	float limit_ang_y_lower;
	float limit_ang_y_upper;
	float limit_ang_z_lower;
	float limit_ang_z_upper;

	/* spring settings */
	/* resistance to deformation */
	float spring_stiffness_x;
	float spring_stiffness_y;
	float spring_stiffness_z;
	float spring_stiffness_ang_x;
	float spring_stiffness_ang_y;
	float spring_stiffness_ang_z;
	/* amount of velocity lost over time */
	float spring_damping_x;
	float spring_damping_y;
	float spring_damping_z;
	float spring_damping_ang_x;
	float spring_damping_ang_y;
	float spring_damping_ang_z;

	/* motor settings */
	float motor_lin_target_velocity;	/* linear velocity the motor tries to hold */
	float motor_ang_target_velocity;	/* angular velocity the motor tries to hold */
	float motor_lin_max_impulse;		/* maximum force used to reach linear target velocity */
	float motor_ang_max_impulse;		/* maximum force used to reach angular target velocity */

	/* References to Physics Sim object. Exist at runtime only */
	void *physics_constraint;	/* Physics object representation (i.e. btTypedConstraint) */
} RigidBodyCon;

/* RigidBodyConstraint (rbc)
 *
 * Represents an constraint connecting two shard rigid bodies.
 */
typedef struct RigidBodyShardCon {

	struct RigidBodyShardCon *next, *prev;
	struct MeshIsland *mi1;			/* First meshisland influenced by the constraint */
	struct MeshIsland *mi2;			/* Second meshisland influenced by the constraint */

	/* References to Physics Sim object. Exist at runtime only */
	void *physics_constraint;	/* Physics object representation (i.e. btTypedConstraint) */

	/* General Settings for this RigidBodyCon */
	short type;					/* (eRigidBodyCon_Type) role of RigidBody in sim  */
	short num_solver_iterations;/* number of constraint solver iterations made per simulation step */

	int flag;					/* (eRigidBodyCon_Flag) */
	char name[66]; /* MAX_ID_NAME */
	char pad[2];

	float breaking_threshold;	/* breaking impulse threshold */
	float start_angle;			//needed for breaking by angle and dist
	float start_dist;
	float breaking_angle;
	float breaking_dist;
	float plastic_angle;
	float plastic_dist;
	float start_angle_deform;
	float start_dist_deform;

	float orn[4];
	float pos[3];

	/* limits */
	/* translation limits */
	float limit_lin_x_lower;
	float limit_lin_x_upper;
	float limit_lin_y_lower;
	float limit_lin_y_upper;
	float limit_lin_z_lower;
	float limit_lin_z_upper;
	/* rotation limits */
	float limit_ang_x_lower;
	float limit_ang_x_upper;
	float limit_ang_y_lower;
	float limit_ang_y_upper;
	float limit_ang_z_lower;
	float limit_ang_z_upper;

	/* spring settings */
	/* resistance to deformation */
	float spring_stiffness_x;
	float spring_stiffness_y;
	float spring_stiffness_z;
	float spring_stiffness_ang_x;
	float spring_stiffness_ang_y;
	float spring_stiffness_ang_z;
	/* amount of velocity lost over time */
	float spring_damping_x;
	float spring_damping_y;
	float spring_damping_z;
	float spring_damping_ang_x;
	float spring_damping_ang_y;
	float spring_damping_ang_z;

	/* motor settings */
	float motor_lin_target_velocity;	/* linear velocity the motor tries to hold */
	float motor_ang_target_velocity;	/* angular velocity the motor tries to hold */
	float motor_lin_max_impulse;		/* maximum force used to reach linear target velocity */
	float motor_ang_max_impulse;		/* maximum force used to reach angular target velocity */

	char pad2[4];
} RigidBodyShardCon;


/* Participation types for RigidBodyOb */
typedef enum eRigidBodyCon_Type {
	/* lets bodies rotate around a specified point */
	RBC_TYPE_POINT = 0,
	/* lets bodies rotate around a specified axis */
	RBC_TYPE_HINGE,
	/* simulates wheel suspension */
	RBC_TYPE_HINGE2,
	/* restricts movent to a specified axis */
	RBC_TYPE_SLIDER,
	/* lets object rotate within a cpecified cone */
	RBC_TYPE_CONE_TWIST,
	/* allows user to specify constraint axes */
	RBC_TYPE_6DOF,
	/* like 6DOF but has springs */
	RBC_TYPE_6DOF_SPRING,
	/* simulates a universal joint */
	RBC_TYPE_UNIVERSAL,
	/* glues two bodies together */
	RBC_TYPE_FIXED,
	/* similar to slider but also allows rotation around slider axis */
	RBC_TYPE_PISTON,
	/* Simplified spring constraint with only once axis that's automatically placed between the connected bodies */
	RBC_TYPE_SPRING,
    /* drives bodies by applying linear and angular forces */
	RBC_TYPE_MOTOR,
    RBC_TYPE_COMPOUND,
} eRigidBodyCon_Type;

/* Spring implementation type for RigidBodyOb */
typedef enum eRigidBodyCon_SpringType {
	RBC_SPRING_TYPE1 = 0,	/* btGeneric6DofSpringConstraint */
	RBC_SPRING_TYPE2,   	/* btGeneric6DofSpring2Constraint */
} eRigidBodyCon_SpringType;

/* Flags for RigidBodyCon */
typedef enum eRigidBodyCon_Flag {
	/* constraint influences rigid body motion */
	RBC_FLAG_ENABLED					= (1 << 0),
	/* constraint needs to be validated */
	RBC_FLAG_NEEDS_VALIDATE				= (1 << 1),
	/* allow constrained bodies to collide */
	RBC_FLAG_DISABLE_COLLISIONS			= (1 << 2),
	/* constraint can break */
	RBC_FLAG_USE_BREAKING				= (1 << 3),
	/* constraint use custom number of constraint solver iterations */
	RBC_FLAG_OVERRIDE_SOLVER_ITERATIONS	= (1 << 4),
	/* limits */
	RBC_FLAG_USE_LIMIT_LIN_X			= (1 << 5),
	RBC_FLAG_USE_LIMIT_LIN_Y			= (1 << 6),
	RBC_FLAG_USE_LIMIT_LIN_Z			= (1 << 7),
	RBC_FLAG_USE_LIMIT_ANG_X			= (1 << 8),
	RBC_FLAG_USE_LIMIT_ANG_Y			= (1 << 9),
	RBC_FLAG_USE_LIMIT_ANG_Z			= (1 << 10),
	/* springs */
	RBC_FLAG_USE_SPRING_X				= (1 << 11),
	RBC_FLAG_USE_SPRING_Y				= (1 << 12),
	RBC_FLAG_USE_SPRING_Z				= (1 << 13),
	/* motors */
	RBC_FLAG_USE_MOTOR_LIN				= (1 << 14),
	RBC_FLAG_USE_MOTOR_ANG				= (1 << 15),
	/* angular springs */
	RBC_FLAG_USE_SPRING_ANG_X			= (1 << 16),
	RBC_FLAG_USE_SPRING_ANG_Y			= (1 << 17),
	RBC_FLAG_USE_SPRING_ANG_Z			= (1 << 18),
	/* prevent multiple removal and crash with kinematic deactivation */
	RBC_FLAG_USE_KINEMATIC_DEACTIVATION = (1 << 19),
	/* mark this constraint to be able to go into "plastic" mode */
	RBC_FLAG_USE_PLASTIC				= (1 << 20),
	/* mark already active plastic constraints */
	RBC_FLAG_PLASTIC_ACTIVE				= (1 << 21),
} eRigidBodyCon_Flag;

/* ******************************** */

#endif /* __DNA_RIGIDBODY_TYPES_H__ */
