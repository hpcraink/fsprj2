#!/usr/bin/env python3

###
### This file uses code generated automatically by SALOME v9.4.0 with dump python functionality
###
### The script has been modified so that it launches a SALOME instance to get access to 
### GEOM and MESH functionality that reside in a CORBA server as part of SALOME.
### Use at own risk.
### Author: Josh Bowden INRIA
### To Execute:
'''
export PATHTOSALOME=/tank/prj_parapps/salome/SALOME-9.8.0-MPI-CO7-SRC
export PATH=$PATHTOSALOME:$PATH
. $PATHTOSALOME/env_launch.sh  # set up the SALOME python environment
# --force   Forces creation of the directory if it does not exist and
#           the creaion/overwriting of the file if it does exist. Use with care.
'''

import sys, getopt, getpass
import os, io
# from os import path
from pathlib import Path
import time

try:
  import salome
except ModuleNotFoundError as err:
    print("ERROR: ")
    print(err)
    print("INFO: You will need to provide the path to aworking installation of SALOME on PATH variable")
    print("INFO: and also source the $PATHTOSALOME/env_launch.sh script to set up all modules")
    exit() 
    

USAGE_OPT='''
DESCRIPTION:  Script to invoke SALOME to create a 3D meshed cubic hexahedral that can be used by 
              Code_Saturne in a 'moving face' experiment. Used for testing scallability of Code_Saturne.
              The xz plane at y = (side_length / 2) is a 'moving surface' (moving_face_1) that alows
              a Code_Saturne computation to interact with the fluid within the volumne.
              The dimension in the z direction is extended to create a larger model, and a cross section
              in the xy plane, through the centre of the elongated solid will hopefully give similar results
              for the velocity profile for differing sizes in z. MPI decomposition via Code_Saturne also folows
              sections along the z direction.
              
USAGE: mesh_cube_xyz.py [--forceall] -o <outputfilename> -d <outputdir> -x 64 -y 64 -z 128 -s 10.0

--force       -f   Forces creation of the directory if it does not exist and
                   the creaion/overwriting of the file if it does exist. Use with care.
--outfile     -o   The output filename (with or without .med extension
                   Default is mesh_cube_unit_side. The number of segements will be added to the name
                   and .med appended: 
                      e.g. mesh_cube_unit_side_<segments>.med
--outdir      -d   The output directory (will be created if -f specified)
                   Default is users home diretory.
--x_segments  -x   The number of segments for the x sides.                    
--y_segments  -y   The number of segments for the y sides. 
--z_segments  -z   The number of segments for the z sides. 
--side_length -s   The length of the side in the x direction 
                     x_length = side_length
                     y_length = x_length/2; 
                     z_length = (z_segments / x_segments) * side_length

    The total number of hexahedral segments will be x * z * y. Each element takes about 73 bytes of storage, 
    so the expected file size can be determined.
    
EXAMPLE:
# We require an installed SALOME stack
export PATHTOSALOME=/opt/SALOME-9.4.0-DB10-SRC
export PATH=$PATHTOSALOME:$PATH
. $PATHTOSALOME/env_launch.sh  # set up the SALOME python environment
python mesh_cube_xyz.py  -o mesh_xtend_z.med -x 64 -y 64 -z 128 -s 1.0
'''




def run_mesher(fullpath, x_segments, y_segments, z_segments, x_side_length):
   
   fullpath = fullpath.strip().encode('unicode-escape').decode() 
   # import salome_notebook
   # notebook = salome_notebook.NoteBook()
   # sys.path.insert(0, r'/home/jbowden/C_S/saturne_examples/MESH')
   
   # This is added by jcb after reading: https://docs.salome-platform.org/9/tui/KERNEL/salome_command.html#salome_launcher
   from salome_instance import SalomeInstance
   # create a text trap and redirect stdout
   text_trap  = io.StringIO()
   sys.stdout = text_trap
   sys.stderr = text_trap
   instance   = SalomeInstance.start()
   # now restore stdout function
   sys.stdout = sys.__stdout__
   sys.stderr = sys.__stderr__
   print("INFO: Instance created and now running on port", instance.get_port())
   time.sleep( 2 )
   print("INFO: Initiating SALOME GEOM")

   salome.salome_init()

   ###
   ### GEOM component
   ###

   import GEOM
   from salome.geom import geomBuilder
   import math
   import SALOMEDS
   
   y_side_length = x_side_length / 2
   z_side_length = (z_segments / x_segments) * x_side_length


  
   geompy = geomBuilder.New()

   O = geompy.MakeVertex(0, 0, 0)
   OX = geompy.MakeVectorDXDYDZ(1, 0, 0)
   OY = geompy.MakeVectorDXDYDZ(0, 1, 0)
   OZ = geompy.MakeVectorDXDYDZ(0, 0, 1)
   Box_1 = geompy.MakeBoxDXDYDZ(x_side_length, y_side_length, z_side_length)
   [zdir, ydir, xdir] = geompy.Propagate(Box_1)
   moving_face = geompy.CreateGroup(Box_1, geompy.ShapeType["FACE"])
   geompy.UnionIDs(moving_face, [27])
   walls = geompy.CreateGroup(Box_1, geompy.ShapeType["FACE"])
   geompy.UnionIDs(walls, [3, 31, 23, 33, 13])
   vol = geompy.CreateGroup(Box_1, geompy.ShapeType["SOLID"])
   geompy.UnionIDs(vol, [1])
   geompy.addToStudy( O, 'O' )
   geompy.addToStudy( OX, 'OX' )
   geompy.addToStudy( OY, 'OY' )
   geompy.addToStudy( OZ, 'OZ' )
   geompy.addToStudy( Box_1, 'Box_1' )
   geompy.addToStudyInFather( Box_1, moving_face, 'moving_face' )
   geompy.addToStudyInFather( Box_1, walls, 'walls' )
   geompy.addToStudyInFather( Box_1, vol, 'vol' )
   geompy.addToStudyInFather( Box_1, zdir, 'zdir' )
   geompy.addToStudyInFather( Box_1, ydir, 'ydir' )
   geompy.addToStudyInFather( Box_1, xdir, 'xdir' )

   ###
   ### SMESH component
   ###

   import  SMESH, SALOMEDS
   from salome.smesh import smeshBuilder

   smesh = smeshBuilder.New()
   #smesh.SetEnablePublish( False ) # Set to False to avoid publish in study if not needed or in some particular situations:
                                    # multiples meshes built in parallel, complex and numerous mesh edition (performance)

   Mesh_1 = smesh.Mesh(Box_1)
   Regular_1D = Mesh_1.Segment()
   num_segments = Regular_1D.NumberOfSegments(y_segments)
   Quadrangle_2D = Mesh_1.Quadrangle(algo=smeshBuilder.QUADRANGLE)
   Hexa_3D = Mesh_1.Hexahedron(algo=smeshBuilder.Hexa)
   smesh.SetName(Mesh_1, 'Mesh_1')

   Regular_1D_1 = Mesh_1.Segment(geom=xdir)
   Number_of_Segments_1 = Regular_1D_1.NumberOfSegments(x_segments)
   isDone = Mesh_1.Compute()
   Regular_1D_2 = Mesh_1.Segment(geom=zdir)
   Number_of_Segments_2 = Regular_1D_2.NumberOfSegments(z_segments)
   isDone = Mesh_1.Compute()
   moving_face_1 = Mesh_1.GroupOnGeom(moving_face,'moving_face',SMESH.FACE)
   walls_1 = Mesh_1.GroupOnGeom(walls,'walls',SMESH.FACE)
   vol_1 = Mesh_1.GroupOnGeom(vol,'vol',SMESH.VOLUME)
   smesh.SetName(Mesh_1, 'Mesh_1')
      

   print("INFO: Saving file: ", fullpath)

   try:
      Mesh_1.ExportMED(fullpath,auto_groups=0,minor=40,overwrite=1,meshPart=None,autoDimension=1)
      pass
   except:
      print('ExportMED() failed. Invalid file name?')

   # if salome.sg.hasDesktop():
   #   salome.sg.updateObjBrowser()
   
   instance.stop()





def main(argv):

   medfile_ext = '.med'
   filename_mesh = 'mesh_cube_unit_side'
   output_dir = Path.home()  # default is users home directory
   y_segments = 64 
   x_segments = 64
   z_segments = 64
   side_length = 1
   forceall = False 
   try:
      opts, args = getopt.getopt(argv,"hfo:d:x:y:z:s:",["forceall","outfile=","outdir=","x_segments=","y_segments=","z_segments=","side_length="])
   except getopt.GetoptError:
      print(USAGE_OPT)
      sys.exit(2)
   for opt, arg in opts:
      if opt == '-h':
         print(USAGE_OPT)
         sys.exit()
      elif opt in ("-o", "--outfile"):
         filename_mesh = arg
      elif opt in ("-d", "--outdir"):
         output_dir = arg
      elif opt in ("-y", "--y_segments"):
         y_segments = int(arg)   
      elif opt in ("-x", "--x_segments"):
         x_segments = int(arg)     
      elif opt in ("-z", "--z_segments"):
         z_segments = int(arg)   
      elif opt in ("-s", "--side_length"):
         side_length = int(arg)
      elif opt in ("-f", "--forceall"):
         forceall = True          
         
   total_elements = y_segments*x_segments*z_segments
         
   if os.path.isdir(output_dir) == False:
      if Path.exists(output_dir) == True:
         print('ERROR: The path seems to be a file: ' + output_dir)
         exit(-1)
      else:
         if  forceall:
            Path(output_dir).mkdir(parents=True, exist_ok=True)
         else:
            print('ERROR: The path does not exist (force creation with -f): ' + output_dir)
            exit(-1)
            
   
   if  filename_mesh.lower().endswith('.med') == False:
      filename_mesh = filename_mesh  + '_' + str(x_segments) + '_' + str(y_segments) + '_' + str(z_segments) + '_' + str(side_length)  + medfile_ext
   else:
      filename_mesh = os.path.splitext(filename_mesh)[0] + '_' + str(x_segments) + '_' + str(y_segments) + '_' + str(z_segments) + '_' + str(side_length)  + '.med'

   fullpath= Path(output_dir,filename_mesh) 
   if os.path.isfile(fullpath) == True:
      print('WARNING: File exists! : ', fullpath)
      if forceall == False: 
         continue_anyway = input('Do you want to continue? y/N: ')
         if continue_anyway.lower() != 'y':
            print('INFO: Exiting!')
            exit(0) 
         
   print('INFO: Output file is                         : ', fullpath)
   print('INFO: The X side is being meshed into        : ', str(x_segments),    ' segments')
   print('INFO: The Y side is being meshed into        : ', str(y_segments),    ' segments')
   print('INFO: The Z side is being meshed into        : ', str(z_segments),    ' segments')
   print('INFO: That equates to                        : ', str(total_elements),' hexahedral elements')
   if total_elements > 16777216:
       print('WARNING: That is a lot of elements. It will require about : ', str(72.5 * total_elements / (1024*1024)), ' MB.')
       if forceall == False: 
          continue_anyway = input('Do you want to continue? y/N: ')
          if continue_anyway.lower() != 'y':
             print('INFO: Exiting!')
             exit(0) 
             
   run_mesher(fullpath.as_posix(), x_segments, y_segments, z_segments, side_length)
    

if __name__ == "__main__":
   main(sys.argv[1:])

