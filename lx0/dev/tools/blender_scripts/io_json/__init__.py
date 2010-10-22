# Derived from io_mesh_ply plug-in provide with Blender 2.54.
# Copyright information for that plug-in:
#
#   Copyright (C) 2004, 2005: Bruce Merry, bmerry@cs.uct.ac.za
#   Contributors: Bruce Merry, Campbell Barton


# To support reload properly, try to access a package var, if it's there, reload everything
if "bpy" in locals():
    import sys
    reload(sys.modules.get("io_json.export_json", sys))


import bpy
from bpy.props import *
from io_utils import ExportHelper


class ExportJSON(bpy.types.Operator, ExportHelper):
    '''Export a single object as a LxEngine mesh.'''
    bl_idname = "export.json"
    bl_label = "Export JSON"
    
    filename_ext = ".json"

    use_modifiers = BoolProperty(name="Apply Modifiers", description="Apply Modifiers to the exported mesh", default=True)
    use_normals = BoolProperty(name="Normals", description="Export Normals for smooth and hard shaded faces", default=True)
    use_uv_coords = BoolProperty(name="UVs", description="Exort the active UV layer", default=True)
    use_colors = BoolProperty(name="Vertex Colors", description="Exort the active vertex color layer", default=True)

    @classmethod
    def poll(cls, context):
        return context.active_object != None

    def execute(self, context):
        filepath = self.filepath
        #filepath = bpy.path.ensure_ext(filepath, self.filename_ext)
        import io_json.export_json
        return io_json.export_json.save(self, context, **self.properties)

    def draw(self, context):
        layout = self.layout

        row = layout.row()
        row.prop(self.properties, "use_modifiers")
        row.prop(self.properties, "use_normals")
        row = layout.row()
        row.prop(self.properties, "use_uv_coords")
        row.prop(self.properties, "use_colors")


def menu_func(self, context):
    self.layout.operator(ExportJSON.bl_idname, text="LxEngine (.json)")


def register():
    bpy.types.INFO_MT_file_export.append(menu_func)


def unregister():
    bpy.types.INFO_MT_file_export.remove(menu_func)

if __name__ == "__main__":
    register()
