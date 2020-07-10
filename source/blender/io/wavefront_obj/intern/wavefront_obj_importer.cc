/*
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
 * The Original Code is Copyright (C) 2020 Blender Foundation.
 * All rights reserved.
 */

/** \file
 * \ingroup obj
 */

#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <optional>

#include "BKE_collection.h"
#include "BKE_customdata.h"
#include "BKE_object.h"

#include "BLI_array.hh"
#include "BLI_path_util.h"
#include "BLI_string.h"
#include "BLI_string_ref.hh"

#include "bmesh.h"

#include "DEG_depsgraph.h"
#include "DEG_depsgraph_query.h"

#include "wavefront_obj_file_handler.hh"
#include "wavefront_obj_importer.hh"

namespace blender::io::obj {
OBJImporter::OBJImporter(const OBJImportParams &import_params) : import_params_(import_params)
{
  infile_.open(import_params_.filepath);
}

void OBJImporter::parse_and_store(Vector<std::unique_ptr<OBJRawObject>> &list_of_objects)
{
  std::string line;
  std::unique_ptr<OBJRawObject> *curr_ob;
  while (std::getline(infile_, line)) {
    std::stringstream s_line(line);
    std::string line_key;
    s_line >> line_key;

    if (line_key == "o") {
      /* Update index offsets if an object has been processed already. */
      if (list_of_objects.size() > 0) {
        index_offsets[VERTEX_OFF] += (*curr_ob)->vertices.size();
        index_offsets[UV_VERTEX_OFF] += (*curr_ob)->texture_vertices.size();
      }
      list_of_objects.append(std::make_unique<OBJRawObject>(s_line.str()));
      curr_ob = &list_of_objects.last();
    }
    /* TODO ankitm Check that an object exists. */
    else if (line_key == "v") {
      MVert curr_vert;
      s_line >> curr_vert.co[0] >> curr_vert.co[1] >> curr_vert.co[2];
      (*curr_ob)->vertices.append(curr_vert);
    }
    else if (line_key == "vn") {
      (*curr_ob)->tot_normals++;
    }
    else if (line_key == "vt") {
      MLoopUV curr_tex_vert;
      s_line >> curr_tex_vert.uv[0] >> curr_tex_vert.uv[1];
      (*curr_ob)->texture_vertices.append(curr_tex_vert);
    }
    else if (line_key == "f") {
      Vector<OBJFaceCorner> curr_face;
      while (s_line) {
        OBJFaceCorner corner;
        if (!(s_line >> corner.vert_index)) {
          break;
        }
        /* Base 1 in OBJ to base 0 in C++. */
        corner.vert_index--;
        /* Adjust for index offset of previous objects. */
        corner.vert_index -= index_offsets[VERTEX_OFF];

        // TODO texture coords handling. It's mostly string manipulation. Normal indices will be
        // ignored and calculated depending on the smooth flag.
        // s_line >> corner.tex_vert_index;
        curr_face.append(corner);
      }
      (*curr_ob)->face_elements.append(curr_face);
      (*curr_ob)->tot_loop += curr_face.size();
    }
    else if (line_key == "usemtl") {
      (*curr_ob)->material_name.append(s_line.str());
    }
    else if (line_key == "#") {
    }
  }
}

void OBJImporter::print_obj_data(Vector<std::unique_ptr<OBJRawObject>> &list_of_objects)
{
  for (std::unique_ptr<OBJRawObject> &curr_ob : list_of_objects) {
    for (const MVert &curr_vert : curr_ob->vertices) {
      print_v3("vert", curr_vert.co);
    }
    printf("\n");
    for (const MLoopUV &curr_tex_vert : curr_ob->texture_vertices) {
      print_v2("tex vert", curr_tex_vert.uv);
    }
    printf("\n");
    for (const Vector<OBJFaceCorner> &curr_face : curr_ob->face_elements) {
      for (OBJFaceCorner a : curr_face) {
        printf("%d ", a.vert_index);
      }
      printf("\n");
    }
    printf("\n");
    for (StringRef b : curr_ob->material_name) {
      printf("%s", b.data());
    }
  }
}

static Mesh *mesh_from_raw_obj(Main *bmain, OBJRawObject &curr_object)
{
  /* verts_len is 0 since BMVerts will be added later. So avoid duplication of vertices in
   * BM_mesh_bm_from_me */
  Mesh *mesh = BKE_mesh_new_nomain(
      0, 0, 0, curr_object.tot_loop, curr_object.face_elements.size());

  /* -------------------- */
  struct BMeshFromMeshParams bm_convert_params = {true, 0, 0, 0};
  BMAllocTemplate bat = {0,
                         0,
                         static_cast<int>(curr_object.tot_loop),
                         static_cast<int>(curr_object.face_elements.size())};
  BMeshCreateParams bcp = {1};
  BMesh *bm_new = BM_mesh_create(&bat, &bcp);
  BM_mesh_bm_from_me(bm_new, mesh, &bm_convert_params);
  //  BKE_mesh_free(mesh);
  /* Vertex creation. */
  Array<BMVert *> all_vertices(curr_object.vertices.size());
  for (int i = 0; i < curr_object.vertices.size(); i++) {
    MVert &curr_vert = curr_object.vertices[i];
    all_vertices[i] = BM_vert_create(bm_new, curr_vert.co, NULL, BM_CREATE_SKIP_CD);
  }

  BM_mesh_elem_table_ensure(bm_new, BM_VERT);

  /* Face and edge creation. */
  for (const Vector<OBJFaceCorner> &curr_face : curr_object.face_elements) {

    Array<BMVert *> verts_of_face(curr_face.size());
    for (int i = 0; i < curr_face.size(); i++) {
      //      verts_of_face[i] = BM_vert_at_index(bm_new, curr_face[i].vert_index);
      verts_of_face[i] = all_vertices[curr_face[i].vert_index];
    }
    BM_face_create_ngon_verts(
        bm_new, &verts_of_face[0], verts_of_face.size(), NULL, BM_CREATE_SKIP_CD, false, true);
  }

  /* Add mesh to object. */
  BMeshToMeshParams bmtmp = {0, 0, {0, 0, 0, 0, 0}};
  BM_mesh_bm_to_me(bmain, bm_new, mesh, &bmtmp);
  //  Mesh *mesh1 = BKE_mesh_new_nomain(0, 0, 0, 0, 0);
  //  BM_mesh_bm_to_me_for_eval(bm_new, mesh, NULL);
  BM_mesh_free(bm_new);

  return mesh;
  /* -------------------- */
  /* Vertex creation. */
  for (int i = 0; i < curr_object.vertices.size(); i++) {
    MVert &curr_vert = curr_object.vertices[i];
    copy_v3_v3(mesh->mvert[i].co, curr_vert.co);
  }

  /* Face and loop creation. */
  for (int i = 0; i < curr_object.face_elements.size(); i++) {
    const Vector<OBJFaceCorner> &curr_face = curr_object.face_elements[i];
    MPoly &mpoly = mesh->mpoly[i];
    mpoly.loopstart = curr_face[0].vert_index;
    mpoly.totloop = curr_face.size();

    MLoop *mloop = mesh->mloop;
    for (int j = 0; j < mpoly.totloop; j++) {
      mloop[mpoly.loopstart + j].v = curr_face[j].vert_index;
    }
  }

  return mesh;
}

OBJParentCollection::OBJParentCollection(Main *bmain, Scene *scene) : bmain_(bmain), scene_(scene)
{
  parent_collection_ = BKE_collection_add(
      bmain_, scene_->master_collection, "OBJ import collection");
}

void OBJParentCollection::add_object_to_parent(OBJRawObject &ob_to_add, Mesh *mesh)
{
  Object *b_object = BKE_object_add_only_object(bmain_, OB_MESH, ob_to_add.object_name.c_str());
  b_object->data = BKE_object_obdata_add_from_type(bmain_, OB_MESH, ob_to_add.object_name.c_str());

  //  BKE_mesh_validate(mesh, false, true);
  BKE_mesh_nomain_to_mesh(mesh, (Mesh *)b_object->data, b_object, &CD_MASK_EVERYTHING, true);

  BKE_collection_object_add(bmain_, parent_collection_, b_object);
  id_fake_user_set(&parent_collection_->id);

  DEG_id_tag_update(&parent_collection_->id, ID_RECALC_COPY_ON_WRITE);
  DEG_relations_tag_update(bmain_);
}

void OBJImporter::make_objects(Main *bmain,
                               Scene *scene,
                               Vector<std::unique_ptr<OBJRawObject>> &list_of_objects)
{
  OBJParentCollection parent{bmain, scene};
  for (std::unique_ptr<OBJRawObject> &curr_object : list_of_objects) {
    Mesh *mesh = mesh_from_raw_obj(bmain, *curr_object);

    parent.add_object_to_parent(*curr_object, mesh);
  }
}

void importer_main(bContext *C, const OBJImportParams &import_params)
{
  Main *bmain = CTX_data_main(C);
  Scene *scene = CTX_data_scene(C);
  Vector<std::unique_ptr<OBJRawObject>> list_of_objects;
  OBJImporter importer = OBJImporter(import_params);
  importer.parse_and_store(list_of_objects);
  importer.print_obj_data(list_of_objects);
  importer.make_objects(bmain, scene, list_of_objects);
}
}  // namespace blender::io::obj
