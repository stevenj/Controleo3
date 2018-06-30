#!/usr/bin/env python3

import os
import sys
import multiprocessing

import fabricate.fabricate  as FAB
import fabricate.util       as util  
import fabricate.generators as gen

####### BUILD TYPE
# Convention:
#   Keys are the names of the various build types (unique)
#   The value is the destination base directory for the build (also unique)
BUILD_TYPES = {
    "OPT-SIZE" : "build/optimized/size",
    "OPT-SPEED": "build/optimized/speed",
    "DEBUG"    : "build/debug",
}

TARGET={
    "DEVICE"    : "SAMD21J18A",    # Name of device building for.
    "CPU"       : "cortex-m0plus", # sets -mcpu and -mtune see gcc for valid values
}

####### COMMON PROJECT BUILD DEFINITIONS
# Saves Redundancy in BUILD definition.


####### PROJECT BUILD DEFINITIONS
# Convention:
#    KEYS are Declared 'KEY'
#    PARAMS are Declared "PARAM"
BUILD={
    # Build Type Control - List all the builds we want to do by default.
    'BUILDS'        : BUILD_TYPES,

#    'DEBUG_BUILD'   : [True,False], # [True] to build debug-able version.
                                    # [False] to build production version.
                                    # [True,False] to build both a debug-able and production version.

    # Binary Built Files Destination
#    'BUILD_DIR'     : "build/optimized",   # Place where Binaries are placed (optimized build)
#    'DEBUG_DIR'     : "build/debug",       # Place where Binaries are placed (debug build)

    # Define which tools are used to build which extensions
    'EXT' :   {
        'GCC'       : [".c"],
        'GXX'       : [".cpp"],
        'GAS'       : [".S"],
    },

    # The Tools we use, by architecture.
    #   'AVR' toolchain avr-gcc
    #   'ARM' toolchain arm-gcc
    #   'TOOL' is a Command, typically a code generator, it doesn't belong to a
    #               tool chain per-se.
    #   'SCRIPT' is a Build Script.
    'TOOLS' : {
        'PATH': {
            'ARM'    : "/opt/arm/gcc-arm-none-eabi-7-2017-q4-major/bin",
            'TOOL'   : "../Tools",
            'SCRIPT' : "fabricate/scripts",
            'UF2CONV': "fabricate/scripts"
        },
        'GCC'      : {'ARM' : "arm-none-eabi-gcc"},
        'GXX'      : {'ARM' : "arm-none-eabi-gcc"},
        'GAS'      : {'ARM' : "arm-none-eabi-gcc"},
        'OBJ-COPY' : {'ARM' : "arm-none-eabi-objcopy"},
        'OBJ-DUMP' : {'ARM' : "arm-none-eabi-objdump"},
        'UF2CONV'  : {'ARM' : "uf2conv.py"}
    }, # END TOOLS

    # The Options we pass to the Tools.
    'OPTS'  : {
        'GCC:GXX' : { # Options common to both C Builds and C++ builds.
            'WARN' : [
                "-Wall",
            ],
            'ARM'       : {
                'CFLAGS' : [
                    "-mcpu="+TARGET['CPU'],
                    "-mtune="+TARGET['CPU'],
                    "-mthumb",
                    "-fstrict-volatile-bitfields",
                    "-ffunction-sections",
                    "-fdata-sections",
                    "-ffreestanding",
#                    "-Winline",
                ],
                'CFLAGS:DEBUG' : [
                    "-Og",
                    "-ggdb3",
                ],
                'CFLAGS:OPT-SIZE' : [
                    "-Os",
                ],
                'CFLAGS:OPT-SPEED' : [
                    "-O3",
                ],

                
            },
        },
        'GCC' : { # Options unique to C Compiles
            'WARN' : [
            ],
            'ARM'       : {
                'CFLAGS' : [
                    "-std=gnu11",
                ],
                'LDFLAGS' : [
                ],
            },
        }, # END GCC

        'GXX' : { # Options unique to C++ Compiles
            'WARN' : [
                "-Wno-reorder",
            ],
            'ARM'       : {
                'CFLAGS' : [
                    "-std=gnu++14",
                    "-fno-exceptions",
                    "-fno-unwind-tables",
                    "-fno-rtti",
                    "-fno-use-cxa-atexit",
                    #"-fno-threadsafe-statics",
                ],
                'LDFLAGS' : [
                ],
            },
        }, # END GXX

        'GAS' : {
            'WARN' : [
                "-Wall",
            ],
            'ARM'       : {
                'CFLAGS' : [
                    "-x","assembler-with-cpp",
                    "-mcpu="+TARGET['CPU'],
                    "-mtune="+TARGET['CPU'],
                    "-save-temps"
                ],
            },
            'DEFINES' : [
                "__ASSEMBLY__",
            ],
        }, # END GAS

        'LD' : {
            'WARN' : [
                "-Wall",
            ],
            'ARM'   : {
                'CFLAGS' : [ # Passed to GCC when linking.
                    "-mcpu="+TARGET['CPU'],
                    "-mtune="+TARGET['CPU'],
                    "--specs=nano.specs",
                ],
                'CFLAGS:DEBUG' : [
                    "-ggdb3",
                ],
                'CFLAGS:OPT-SIZE' : [
                ],
                'LDFLAGS' : [ # These are passed to the linker (by GCC), using "-Wl"
                    "-T","OvenACE/samd21a/gcc/gcc/samd21j18a_flash.ld",
                    "--relax",
                    "--gc-sections",
                ],
            }
        },

    }, # END OPTS

    # Targets to skip Building.  USED DURING DEVELOPMENT.
    'SKIP' : [
    ],

    # External Sources to Build/Preliminary Operations
    #  { <Name> : { Options }, ... }
    #     'VERSION'     - Displayed Version of the Module/Library
    #     'BASEDIR'     - (Optional) Specifies a Directory which contains the source if it is not "VERSION",
    #     'MAKE'        - List of commands used to "make" these external sources
    #
    #   EXTERNAL is used for:
    #       1: to build "build" tools from source
    #       2: to fetch or update externally maintained source repositories.
    #       3: To generate code files from data.
    #       4: Any other preliminary or utility pre-build function
    #           Technically autotools type configuration tests could be
    #           integrated at this point.
    #   All External builds are completed before the main build starts.
    'EXTERNAL'  : {
    },

    # Sources to Build
    #   FORMAT:
    #     { <MODULE/LIBRARY Name> : { Options }, ... }
    #   OPTIONS:
    #     'VERSION' - Displayed Version of the Module/Library
    #     'ARCH'    - The Architecture to build the Module/Library with. (Must Match 'TOOLS')
    #     'CORE'    - The CORE of the processor to build for. (Must Match OPTS[<TOOL>][<ARCH>])
    #     'PREFIX'  - Directory Library/Module is found under.
    #     'BASEDIR' - (Optional) Specifies a Directory which contains the source if it is not "VERSION",
    #     'LIBRARY' - Optional - Package as a Library, called this name.
    #     'MODULE'  - Optional - Marks a Package as a module of the main application.
    #     'APP'     - Optional - Marks a Package as the main application.
    #     'LISTING' - Optional - Generates an assembler listing of each file.  Option is passed to the assembler to control the listing.
    #     'SRC'     - List of Source Files to Build.
    #                 Each element may be a single file, or a tuple.
    #                 the single file is equivalent to a tuple (file, BUILD[BUILD_DIR], 0)
    #                 the tuple is:
    #                     (src file, dest directory, the number of path elements to strip from source to destination)
    #     'INCLUDE' - List of Include Directories specific to Building the Module/Library
    #     'USES'    - List of Modules/Libraries used by this module
    #                   : Brings in necessary includes and defines from the module.
    #
    'SOURCE'  : {
        'OvenACE' : {
            'VERSION' : 'V1.00',
            'ARCH'    : "ARM",
            'CORE'    : TARGET['DEVICE'],
            'PREFIX'  : "",
            'BASEDIR' : "",
            'APP'     : "OvenACE.elf",
            'MAP'     : "OvenACE.map",
            'DUMP'    : "OvenACE.dump",
            'HEX'     : "OvenACE.hex",
            'BIN'     : "OvenACE.bin",
            'UF2'     : "OvenACE.uf2",
            'HEX_FLAGS' : [
                "-j",".text",
                "-j",".data",
                "-O","ihex",
            ],
            'BIN_FLAGS' : [
                "-j",".text",
                "-j",".data",
                "-O","binary",
            ],
            'UF2_FLAGS' : [
                "-b", "0x2000",
                "-c",
            ],
            'DUMP_FLAGS' : [
                "-xdSs",
            ],
            'LISTING'   : "-ahls",
            'DEFINES' : [
                "__"+TARGET['DEVICE']+"__",
            ],
            'DEFINES:DEBUG' : [
                "DEBUG",
            ],

            'LDFLAGS' : [ # These are passed to the linker, using "-Wl"
            ],

            'SRC'     : [
            ] + util.all_files_in('OvenACE','.c',True)
#              + util.all_files_in('OvenACE','.cpp',True)
              + util.all_files_in('OvenACE','.S',True),
            'INCLUDE' : [
            ] + util.all_directories_of('OvenACE','.h'),

            'SYSINCLUDE' : [
            ],

            'ORDER' : 1,
        },
   },

    'DOCS'  : ["docs/Doxyfile_html"],
}

### Main Build Steps
def external():
    gen.module_maker(BUILD,'EXTERNAL',buildtype = None)

def compile():
    if util.validate_build_types(BUILD_TYPES) :
        for buildtype in BUILD['BUILDS'] :
            if buildtype in BUILD_TYPES:
                gen.module_maker(BUILD,'SOURCE', buildtype)

def package():
    # We would package up our build here, if need be.
    return

def project():
    compile()
    FAB.after()
    package()

def docs():
    """
    for doc in BUILD['DOCS']:
        run(BUILD['TOOLS']['DOC']['GEN'],
            doc,
            group=doc+'_docs')
    """
    return

## Custom Operations

def build():
    external()
    FAB.after() # Do not proceed to build source until external builds finish.
    project()
    FAB.after()
    docs()

    # TODO : Make a size report, showing the size of each build, AND how much space it consumes in the device.
    # TODO : Read that info using the size command, and parsing the link file.

#def debug():
#    BUILD['DEBUG_BUILD'] = [True]
#    build()

def default():
    build()

def clean():
    FAB.autoclean()

def usage():
    help(FAB)

if __name__ == "__main__":
    # parallel_ok = False, prevents running multiple compiles simultaneously.
    # debug = True causes the executed commands to be emitted as the build runs.
    # jobs sets the max number of parallel jobs.
    parallel = False
    if parallel == True :
        jobs = multiprocessing.cpu_count()
    else:
        jobs = 1        
    FAB.main(parallel_ok=parallel,jobs=jobs,debug=True )
