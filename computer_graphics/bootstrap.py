#!/usr/bin/env python

# create directory extern in main directory if it doesn't exist
# quit if eigen and sdl2 are already the correct versions and installed
# if they are not the correct version nuke everything except extern/src, then nuke the old library under extern/src
# download the correct versions of eigen and sdl2 into extern/src
# configure them with install prefix == extern
# build sdl2
# install eigen and sdl2 into extern

# qbs configure can use something like this:
# PKG_CONFIG_PATH=extern/lib/pkgconfig/ pkg-config sdl2 --cflags --libs --static
# to discover build flags for sdl
#   --libs          is enough for dynamic linking
#   --libs --static     is needed for static linking

import json
import os
import os.path
import shutil
import subprocess
import sys
import tarfile
import urllib2
import zipfile

GLM_VERSION = '0.9.4'.split('.')
GLM_URL = 'https://github.com/g-truc/glm/archive/0.9.4.zip'

SDL2_VERSION = '2.0.0'.split('.')
SDL2_URL = 'http://www.libsdl.org/release/SDL2-2.0.0.tar.gz'

ASSIMP_VERSION = '3.0.1270'.split('.')
ASSIMP_URL = 'http://downloads.sourceforge.net/project/assimp/assimp-3.0/assimp--3.0.1270-source-only.zip?use_mirror=autoselect'

PROJECT_DIR = os.path.dirname(os.path.abspath(__file__))

def downloadFile(url, targetDir, targetFileName=None):
    try:
        print('Downloading: "{0}"'.format(url))
        f = urllib2.urlopen(url)

        if targetFileName is None:
            targetFileName = os.path.join(targetDir, os.path.basename(url))
        else:
            targetFileName = os.path.join(targetDir, targetFileName)

        with open(targetFileName, "wb") as local_file:
            local_file.write(f.read())
    except HTTPError, e:
        print "HTTP Error:", e.code, url
        return False
    except URLError, e:
        print "URL Error:", e.reason, url
        return False
    return True

def removeOldArtifacts(externDir):
    for each in ['bin', 'doc', 'include', 'lib', 'share']:
        curDir = os.path.join(externDir, each)
        if os.path.exists(curDir):
            shutil.rmtree(curDir)

def createExtern():
    externDir = os.path.join(PROJECT_DIR, 'extern')
    if not os.path.exists(externDir):
        os.mkdir(externDir)

    srcDir = os.path.join(externDir, 'src')
    if not os.path.exists(srcDir):
        os.mkdir(srcDir)

    return (os.path.exists(srcDir), externDir, srcDir)

def installedLibraryVersions(srcDir):
    versionsFile = os.path.join(srcDir, 'versions.json')
    if not os.path.exists(versionsFile):
        return { 'eigen': ['0'], 'glm': ['0'], 'sdl2': ['0'], 'assimp': ['0'] }

    versions = json.loads(open(versionsFile, 'r').read())
    if 'glm' in versions:
        versions['glm'] = versions['glm'].split('.')
    else:
        versions['glm'] = ['0']

    if 'sdl2' in versions:
        versions['sdl2'] = versions['sdl2'].split('.')
    else:
        versions['sdl2'] = ['0']

    if 'assimp' in versions:
        versions['assimp'] = versions['assimp'].split('.')
    else:
        versions['assimp'] = ['0']

    return versions

def installSdl2(externDir, srcDir):
    sdlArchive = os.path.join(srcDir, os.path.basename(SDL2_URL))
    if not os.path.exists(sdlArchive) and not downloadFile(SDL2_URL, srcDir):
        print('Unable to download SDL2 source')
        return ['0']

    # remove '.tar.gz' from filename
    sdlSourceDir = sdlArchive[:-7]
    if os.path.exists(sdlSourceDir):
        shutil.rmtree(sdlSourceDir)

    tar = tarfile.open(sdlArchive)
    tar.extractall(srcDir)
    tar.close()

    if sys.platform.startswith('linux'):
        cmakeBuildDir = sdlSourceDir + '-build'
        if os.path.exists(cmakeBuildDir):
            shutil.rmtree(cmakeBuildDir)
        os.mkdir(cmakeBuildDir)
        os.chdir(cmakeBuildDir)

        subprocess.check_call(['cmake', '-DCMAKE_INSTALL_PREFIX={0}'.format(externDir), sdlSourceDir])
        subprocess.check_call(['make', '-j8'])
        subprocess.check_call(['make', 'install'])
    elif sys.platform == 'darwin':
        os.chdir(sdlSourceDir)

        subprocess.check_call(['./configure', '--prefix={0}'.format(externDir)])
        subprocess.check_call(['make', '-j8'])
        subprocess.check_call(['make', 'install'])
    else:
        print('Unknown platform "{0}"'.format(sys.platform))
        return ['0']

    return SDL2_VERSION

def installGlm(externDir, srcDir):
    glmArchive = os.path.join(srcDir, os.path.basename(GLM_URL))
    if not os.path.exists(glmArchive) and not downloadFile(GLM_URL, srcDir):
        print('Unable to download glm source')
        return ['0']

    # remove '.zip' from filename
    glmSourceDir = os.path.join(srcDir, 'glm-0.9.4')
    if os.path.exists(glmSourceDir):
        shutil.rmtree(glmSourceDir)

    archive = zipfile.ZipFile(glmArchive, 'r')
    archive.extractall(srcDir)
    archive.close()

    dest = os.path.join(externDir, 'include', 'glm')
    source = os.path.join(glmSourceDir, 'glm')

    if os.path.exists(dest):
        shutil.rmtree(dest)

    shutil.copytree(source, dest)

    return GLM_VERSION

def installAssimp(externDir, srcDir):
    assimpArchive = os.path.join(srcDir, os.path.basename(ASSIMP_URL))
    if not os.path.exists(assimpArchive) and not downloadFile(ASSIMP_URL, srcDir):
        print('Unable to download Assimp source')
        return ['0']

    # remove '.tar.gz' from filename
    assimpSourceDir = os.path.join(srcDir, 'assimp--3.0.1270-source-only')
    if os.path.exists(assimpSourceDir):
        shutil.rmtree(assimpSourceDir)

    archive = zipfile.ZipFile(assimpArchive, 'r')
    archive.extractall(srcDir)
    archive.close()

    if sys.platform == 'darwin':
        # patch code/STEPFile.h
        unpatched = None
        with open(os.path.join(assimpSourceDir, 'code', 'STEPFile.h'), 'r') as f:
            unpatched = f.read()

        patched = unpatched.replace(
            'return Couple<T>(db).MustGetObject(To<EXPRESS::ENTITY>())->To<T>();',
            'return Couple<T>(db).MustGetObject(To<EXPRESS::ENTITY>())->template To<T>();'
        ).replace(
            'return e?Couple<T>(db).MustGetObject(*e)->ToPtr<T>():(const T*)0;',
            'return e?Couple<T>(db).MustGetObject(*e)->template ToPtr<T>():(const T*)0;')

        with open(os.path.join(assimpSourceDir, 'code', 'STEPFile.h'), 'w') as f:
            f.write(patched)

        # patch CMakeLists.txt
        unpatched = None
        with open(os.path.join(assimpSourceDir, 'CMakeLists.txt'), 'r') as f:
            unpatched = f.read()

        patched = unpatched.replace('set (ASSIMP_SV_REVISION 1264)', '')

        with open(os.path.join(assimpSourceDir, 'CMakeLists.txt'), 'w') as f:
            f.write(patched)

    if sys.platform.startswith('linux') or sys.platform == 'darwin':
        cmakeBuildDir = assimpSourceDir + '-build'
        if os.path.exists(cmakeBuildDir):
            shutil.rmtree(cmakeBuildDir)
        os.mkdir(cmakeBuildDir)
        os.chdir(cmakeBuildDir)

        subprocess.check_call(['cmake', '-DCMAKE_INSTALL_PREFIX={0}'.format(externDir), assimpSourceDir])
        subprocess.check_call(['make', '-j8'])
        subprocess.check_call(['make', 'install'])

        if sys.platform == 'darwin':
            os.chdir(externDir)
            subprocess.check_call(['install_name_tool', '-id', '@rpath/libassimp.3.dylib', 'lib/libassimp.3.0..dylib'])
    else:
        print('Unknown platform "{0}"'.format(sys.platform))
        return ['0']

    return ASSIMP_VERSION

def main():
    ok, externDir, srcDir = createExtern()
    if not ok:
        print('Unable to create extern/src')
        return

    versions = installedLibraryVersions(srcDir)
    reinstallNeeded = (
        versions['sdl2'] < SDL2_VERSION or
        versions['glm'] < GLM_VERSION or
        versions['assimp'] < ASSIMP_VERSION
    )

    if reinstallNeeded:
        removeOldArtifacts(externDir)
    if versions['sdl2'] < SDL2_VERSION or reinstallNeeded:
        versions['sdl2'] = installSdl2(externDir, srcDir)
    if versions['glm'] < GLM_VERSION or reinstallNeeded:
        versions['glm'] = installGlm(externDir, srcDir)
    if versions['assimp'] < ASSIMP_VERSION or reinstallNeeded:
        versions['assimp'] = installAssimp(externDir, srcDir)

    with open(os.path.join(srcDir, 'versions.json'), 'w') as f:
        outVersions = {
            'glm': '.'.join(versions['glm']),
            'sdl2': '.'.join(versions['sdl2']),
            'assimp': '.'.join(versions['assimp'])
        }
        f.write(json.dumps(outVersions))

    if versions['sdl2'] == ['0'] or versions['glm'] == ['0'] or versions['assimp'] == ['0']:
        sys.exit(-1)

if __name__ == '__main__':
    main()
