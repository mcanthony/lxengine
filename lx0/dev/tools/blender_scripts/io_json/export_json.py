# <pep8 compliant>

# Derived from io_mesh_ply plug-in provide with Blender 2.54.
# Copyright information for that plug-in:
#
#   Copyright (C) 2004, 2005: Bruce Merry, bmerry@cs.uct.ac.za
#   Contributors: Bruce Merry, Campbell Barton

"""
This script exports LxEngine JSON files from Blender. It supports normals,
colours, and texture coordinates per face or per vertex.
Only one mesh can be exported at a time.
"""

import bpy
import os


def save(operator, context, filepath="", use_modifiers=True, use_normals=True, use_uv_coords=True, use_colors=True):
    
    def rvec3d(v):
        return round(v[0], 6), round(v[1], 6), round(v[2], 6)


    def rvec2d(v):
        return round(v[0], 6), round(v[1], 6)
    
    scene = context.scene
    obj = context.object

    if not obj:
        raise Exception("Error, Select 1 active object")

    file = open(filepath, 'w')

    if scene.objects.active:
        bpy.ops.object.mode_set(mode='OBJECT')

    if use_modifiers:
        mesh = obj.create_mesh(scene, True, 'PREVIEW')
    else:
        mesh = obj.data

    if not mesh:
        raise Exception("Error, could not get mesh data from active object")

    # mesh.transform(obj.matrix_world) # XXX

    faceUV = (len(mesh.uv_textures) > 0)
    vertexUV = (len(mesh.sticky) > 0)
    vertexColors = len(mesh.vertex_colors) > 0

    if (not faceUV) and (not vertexUV):
        use_uv_coords = False
    if not vertexColors:
        use_colors = False

    if not use_uv_coords:
        faceUV = vertexUV = False
    if not use_colors:
        vertexColors = False

    if faceUV:
        active_uv_layer = mesh.uv_textures.active
        if not active_uv_layer:
            use_uv_coords = False
            faceUV = None
        else:
            active_uv_layer = active_uv_layer.data

    if vertexColors:
        active_col_layer = mesh.vertex_colors.active
        if not active_col_layer:
            use_colors = False
            vertexColors = None
        else:
            active_col_layer = active_col_layer.data

    # incase
    color = uvcoord = uvcoord_key = normal = normal_key = None

    mesh_verts = mesh.vertices # save a lookup
    ply_verts = [] # list of dictionaries
    # vdict = {} # (index, normal, uv) -> new index
    vdict = [{} for i in range(len(mesh_verts))]
    ply_faces = [[] for f in range(len(mesh.faces))]
    vert_count = 0
    for i, f in enumerate(mesh.faces):


        smooth = f.use_smooth
        if not smooth:
            normal = tuple(f.normal)
            normal_key = rvec3d(normal)

        if faceUV:
            uv = active_uv_layer[i]
            uv = uv.uv1, uv.uv2, uv.uv3, uv.uv4 # XXX - crufty :/
        if vertexColors:
            col = active_col_layer[i]
            col = col.color1, col.color2, col.color3, col.color4

        f_verts = f.vertices

        pf = ply_faces[i]
        for j, vidx in enumerate(f_verts):
            v = mesh_verts[vidx]

            if smooth:
                normal = tuple(v.normal)
                normal_key = rvec3d(normal)

            if faceUV:
                uvcoord = uv[j][0], 1.0 - uv[j][1]
                uvcoord_key = rvec2d(uvcoord)
            elif vertexUV:
                uvcoord = v.uvco[0], 1.0 - v.uvco[1]
                uvcoord_key = rvec2d(uvcoord)

            if vertexColors:
                color = col[j]
                color = int(color[0] * 255.0), int(color[1] * 255.0), int(color[2] * 255.0)


            key = normal_key, uvcoord_key, color

            vdict_local = vdict[vidx]
            pf_vidx = vdict_local.get(key) # Will be None initially

            if pf_vidx == None: # same as vdict_local.has_key(key)
                pf_vidx = vdict_local[key] = vert_count
                ply_verts.append((vidx, normal, uvcoord, color))
                vert_count += 1

            pf.append(pf_vidx)

    file.write('{\n')
    file.write('    "type" : "quad_list",\n')

    file.write('    "vertices" : [\n')
    for i, v in enumerate(ply_verts):
        file.write('           [ %.6f, %.6f, %.6f ],\n' % tuple(mesh_verts[v[0]].co)) # co
    file.write('    ],\n')
    
    file.write('    "faces" : [\n')
    for pf in ply_faces:
        if len(pf) == 3:
            file.write('           [ %d, %d, %d ],\n' % tuple(pf))
        else:
            file.write('           [ %d, %d, %d, %d ],\n' % tuple(pf))
    file.write('    ],\n')
    file.write('}\n')
    file.close()
    print("writing %r done" % filepath)

    if use_modifiers:
        bpy.data.meshes.remove(mesh)

    # XXX
    """
    if is_editmode:
        Blender.Window.EditMode(1, '', 0)
    """
    
    return {'FINISHED'}
