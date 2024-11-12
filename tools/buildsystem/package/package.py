#!/usr/bin/env python3
import argparse
import datetime
import os
from platform import machine
from multiprocessing import cpu_count
import shutil
import stat
import subprocess
import sys
from sys import platform
from enum import Enum

sys.path.append(os.path.join(os.path.dirname(__file__), os.pardir, 'common'))
from nap_shared import get_cmake_path, get_nap_root, get_build_arch, BuildType, Platform, get_default_generator

WORKING_DIR = '.'
BUILD_DIR = 'packaging_build'
LIB_DIR = 'packaging_lib'
BIN_DIR = 'packaging_bin'
PACKAGING_DIR = 'packaging_staging'
ARCHIVING_DIR = 'archiving'
APPS_SOURCE_DIR = 'apps'
APPS_DEST_DIR = 'apps'
APPLY_PERMISSIONS_BATCHFILE = 'apply_executable_permissions.cmd'

ERROR_PACKAGE_EXISTS = 1
ERROR_INVALID_VERSION = 2
ERROR_COULD_NOT_REMOVE_DIRECTORY = 4
ERROR_BAD_INPUT = 5
ERROR_SOURCE_ARCHIVE_GIT_NOT_CLEAN = 6
ERROR_SOURCE_ARCHIVE_EXISTING = 7

def call(cwd, cmd, capture_output=False):
    """Execute command in provided working directory"""

    # print('dir: %s' % os.path.abspath(cwd))
    # print('cmd: %s' % cmd)
    if capture_output:
        proc = subprocess.Popen(cmd, cwd=cwd, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    else:
        proc = subprocess.Popen(cmd, cwd=cwd)
    (out, err) = proc.communicate()
    if type(out) is bytes:
        out = out.decode('ascii', 'ignore')
        err = err.decode('ascii', 'ignore')
    if proc.returncode != 0:
        raise Exception("Bailing for non zero returncode: %s", proc.returncode)
    return out, err

def package(zip_release,
            include_debug_symbols,
            include_docs,
            single_app_to_include,
            clean,
            include_timestamp_in_name,
            build_label,
            package_name,
            archive_source,
            archive_source_zipped,
            archive_source_only,
            overwrite,
            additional_dirs,
            enable_python):

    """Package a NAP platform release - main entry point"""
    nap_root = get_nap_root()
    os.chdir(nap_root)
    build_label_out = build_label if not build_label is None else ''

    # Add timestamp and git revision for build info
    timestamp = datetime.datetime.now().strftime('%Y.%m.%dT%H.%M')
    git_revision = None
    try:
        (git_revision, _) = call(WORKING_DIR, ['git', 'rev-parse', 'HEAD'], True)
        git_revision = git_revision.strip()
    except Exception as e:
        print("Warning: unable to get git revision")

    # Construct package name if name not given
    package_basename, source_archive_basename = package_name, package_name
    if package_name is None:
        (package_basename, source_archive_basename) = build_package_basename(timestamp if include_timestamp_in_name else None, build_label);

    # Ensure we won't overwrite any existing package
    if not archive_source_only and not overwrite:
        check_for_existing_package(package_basename, zip_release)

    if archive_source:
        # Verify our repo is clean if we want to do a source archive
        verify_clean_git_repo()

        # Check we're not going to overwrite an existing source archive
        if not overwrite:
            check_existing_source_archive(source_archive_basename, archive_source_zipped)

    if not archive_source_only:
        print("Packaging...")

        # Remove old packaging path if it exists
        if os.path.exists(PACKAGING_DIR):
            remove_directory_exit_on_failure(PACKAGING_DIR, 'old packaging staging area')
        os.makedirs(PACKAGING_DIR)

        # Clean build if requested
        if clean:
            clean_the_build()

        # Convert additional sub directories to CMake list type
        sub_dirs = ';'.join(additional_dirs)

        # Do the packaging
        if Platform.get() == Platform.Linux:
            package_path = package_for_linux(package_basename,
                                             timestamp,
                                             git_revision,
                                             build_label_out,
                                             overwrite,
                                             single_app_to_include,
                                             include_docs,
                                             zip_release,
                                             include_debug_symbols,
                                             sub_dirs,
                                             enable_python
                                             )
        elif Platform.get() == Platform.macOS:
            package_path = package_for_macos(package_basename,
                                             timestamp,
                                             git_revision,
                                             build_label_out,
                                             overwrite,
                                             single_app_to_include,
                                             include_docs,
                                             zip_release,
                                             include_debug_symbols,
                                             sub_dirs,
                                             enable_python
                                             )
        elif Platform.get() == Platform.Windows:
            package_path = package_for_win64(package_basename,
                                             timestamp,
                                             git_revision,
                                             build_label_out,
                                             overwrite,
                                             single_app_to_include,
                                             include_docs,
                                             zip_release,
                                             include_debug_symbols,
                                             sub_dirs,
                                             enable_python
                                             )
        else:
            print("Error: Unsupported platform")
            sys.exit(ERROR_BAD_INPUT)

    # Archive source
    if archive_source:
        create_source_archive(source_archive_basename, archive_source_zipped, build_label_out, timestamp, git_revision, overwrite)

        # Make main package path visible
        if not archive_source_only:
            print("Release packaged to %s" % package_path)

def remove_directory_exit_on_failure(path, use):
    try:
        shutil.rmtree(path)
    except OSError as e:
        print("Error: Could not remove directory '%s' (%s): %s" % (path, use, e))
        sys.exit(ERROR_COULD_NOT_REMOVE_DIRECTORY)

def clean_the_build():
    """Clean the build"""

    print("Cleaning...")
    if Platform.get() == Platform.Linux:
        for build_type in BuildType.to_list():
            build_dir_for_type = "%s_%s" % (BUILD_DIR, build_type.lower())
            if os.path.exists(build_dir_for_type):
                print("Clean removing %s" % os.path.abspath(build_dir_for_type))
                remove_directory_exit_on_failure(build_dir_for_type, 'build dir for %s' % build_type.lower())
    else:
        if os.path.exists(BUILD_DIR):
            print("Clean removing %s" % os.path.abspath(BUILD_DIR))
            remove_directory_exit_on_failure(BUILD_DIR, 'build dir')

    if os.path.exists(LIB_DIR):
        print("Clean removing %s" % os.path.abspath(LIB_DIR))
        remove_directory_exit_on_failure(LIB_DIR, 'lib dir')
    if os.path.exists(BIN_DIR):
        print("Clean removing %s" % os.path.abspath(BIN_DIR))
        remove_directory_exit_on_failure(BIN_DIR, 'bin dir')

def check_for_existing_package(package_path, zip_release, remove=False):
    """Ensure we aren't overwriting a previous package, remove if requested"""

    # Add extension if zipping
    if zip_release:
        if Platform.get() == Platform.Linux:
            package_path += '.tar.bz2'
        else:
            package_path += '.zip'

    # Check and fail, or remove if requested
    if os.path.exists(package_path):
        if remove:
            print("Overwriting %s" % os.path.abspath(package_path))
            if zip_release:
                os.remove(package_path)
            else:
                remove_directory_exit_on_failure(package_path, 'overwriting existing package')
        else:
            print("Error: Existing package found: %s" % os.path.abspath(package_path))
            sys.exit(ERROR_PACKAGE_EXISTS)


def package_for_linux(package_basename, timestamp, git_revision, build_label, overwrite, single_app_to_include, include_docs, zip_release, include_debug_symbols, additional_dirs, enable_python):
    """Package NAP platform release for Linux"""

    for build_type in BuildType.to_list():
        build_dir_for_type = "%s_%s" % (BUILD_DIR, build_type.lower())
        call(WORKING_DIR, [get_cmake_path(),
                           '-H.',
                           '-B%s' % build_dir_for_type,
                           '-G%s' % get_default_generator(),
                           '-DNAP_PACKAGED_BUILD=1',
                           '-DCMAKE_BUILD_TYPE=%s' % build_type,
                           '-DINCLUDE_DOCS=%s' % int(include_docs),
                           '-DBUILD_TIMESTAMP=%s' % timestamp,
                           '-DBUILD_GIT_REVISION=%s' % git_revision,
                           '-DBUILD_LABEL=%s' % build_label,
                           '-DINCLUDE_DEBUG_SYMBOLS=%s' % int(include_debug_symbols),
                           '-DADDITIONAL_SUB_DIRECTORIES=%s' % additional_dirs,
                           '-DNAP_ENABLE_PYTHON=%s' % int(enable_python)
                           ])

        d = '%s/%s' % (WORKING_DIR, build_dir_for_type)
        call(d, ['make', 'all', 'install', '-j%s' % cpu_count()])

    # Remove all Naivi apps but the requested one
    if single_app_to_include is not None:
        remove_all_apps_but_specified(single_app_to_include)

    # If requested, overwrite any existing package
    if overwrite:
        check_for_existing_package(package_basename, zip_release, True)

    # Create archive
    if zip_release:
        return archive_framework_to_linux_tar_bz2(package_basename)
    else:
        return archive_to_timestamped_dir(package_basename)

def package_for_macos(package_basename, timestamp, git_revision, build_label, overwrite, single_app_to_include, include_docs, zip_release, include_debug_symbols, additional_dirs, enable_python):
    """Package NAP platform release for macOS"""

    # Generate app
    call(WORKING_DIR, [get_cmake_path(),
                       '-H.',
                       '-B%s' % BUILD_DIR,
                       '-G%s' % get_default_generator(),
                       '-DNAP_PACKAGED_BUILD=1',
                       '-DINCLUDE_DOCS=%s' % int(include_docs),
                       '-DBUILD_TIMESTAMP=%s' % timestamp,
                       '-DBUILD_GIT_REVISION=%s' % git_revision,
                       '-DBUILD_LABEL=%s' % build_label,
                       '-DINCLUDE_DEBUG_SYMBOLS=%s' % int(include_debug_symbols),
                       '-DADDITIONAL_SUB_DIRECTORIES=%s' % additional_dirs,
                       '-DNAP_ENABLE_PYTHON=%s' % int(enable_python)
                       ])

    # Build & install to packaging dir
    d = '%s/%s' % (WORKING_DIR, BUILD_DIR)
    for build_type in BuildType.to_list():
        call(d, ['xcodebuild', '-configuration', build_type, '-target', 'install', '-jobs', str(cpu_count())])

    # Remove unwanted files (eg. .DS_Store)
    call(PACKAGING_DIR, ['find', '.', '-name', '.DS_Store', '-type', 'f', '-delete'])

    # Remove all Naivi apps but the requested one
    if single_app_to_include is not None:
        remove_all_apps_but_specified(single_app_to_include)

    # If requested, overwrite any existing package
    if overwrite:
        check_for_existing_package(package_basename, zip_release, True)

    # Create archive
    if zip_release:
        return archive_framework_to_macos_zip(package_basename)
    else:
        return archive_to_timestamped_dir(package_basename)

def package_for_win64(package_basename, timestamp, git_revision, build_label, overwrite, single_app_to_include, include_docs, zip_release, include_debug_symbols, additional_dirs, enable_python):
    """Package NAP platform release for Windows"""

    # Create build dir if it doesn't exist
    if not os.path.exists(BUILD_DIR):
        os.makedirs(BUILD_DIR)

    # Generate app
    call(WORKING_DIR, [get_cmake_path(),
                       '-H.',
                       '-B%s' % BUILD_DIR,
                       '-G%s' % get_default_generator(),
                       '-DNAP_PACKAGED_BUILD=1',
                       '-DINCLUDE_DOCS=%s' % int(include_docs),
                       '-DBUILD_TIMESTAMP=%s' % timestamp,
                       '-DBUILD_GIT_REVISION=%s' % git_revision,
                       '-DBUILD_LABEL=%s' % build_label,
                       '-DINCLUDE_DEBUG_SYMBOLS=%s' % int(include_debug_symbols),
                       '-DADDITIONAL_SUB_DIRECTORIES=%s' % additional_dirs,
                       '-DNAP_ENABLE_PYTHON=%s' % int(enable_python)
                       ])

    # Build & install to packaging dir
    for build_type in BuildType.to_list():
        call(WORKING_DIR, [get_cmake_path(), '--build', BUILD_DIR, '--target', 'install', '--config', build_type])

    # Remove all Naivi apps but the requested one
    if single_app_to_include is not None:
        remove_all_apps_but_specified(single_app_to_include)

    # If requested, overwrite any existing package
    if overwrite:
        check_for_existing_package(package_basename, zip_release, True)

    # Create archive
    if zip_release:
        return archive_framework_to_win64_zip(package_basename)
    else:
        return archive_to_timestamped_dir(package_basename)

def archive_framework_to_linux_tar_bz2(package_basename):
    """Create build archive to bzipped tarball on Linux"""

    shutil.move(PACKAGING_DIR, package_basename)

    package_filename_with_ext = create_linux_tar_bz2(package_basename)

    # Cleanup
    shutil.move(package_basename, PACKAGING_DIR)

    full_out_path = os.path.abspath(package_filename_with_ext)
    print("Packaged to %s" % full_out_path)
    return full_out_path

def create_linux_tar_bz2(source_directory):
    """Create a bzipped tarball for the provided directory on Linux"""

    archive_filename_with_ext = '%s.%s' % (source_directory, 'tar.bz2')
    print("Archiving to %s ..." % os.path.abspath(archive_filename_with_ext))
    call(WORKING_DIR, ['tar', '-cjvf', archive_filename_with_ext, source_directory])
    return archive_filename_with_ext

def archive_framework_to_macos_zip(package_basename):
    """Create build archive to zip on macOS"""

    shutil.move(PACKAGING_DIR, package_basename)

    # Archive
    package_filename_with_ext = create_macos_zip(package_basename)

    # Cleanup
    shutil.move(package_basename, PACKAGING_DIR)

    full_out_path = os.path.abspath(package_filename_with_ext)
    print("Packaged to %s" % full_out_path)
    return full_out_path

def create_macos_zip(source_directory):
    """Create a zip for the provided directory on macOS"""

    archive_filename_with_ext = '%s.%s' % (source_directory, 'zip')
    print("Archiving to %s ..." % os.path.abspath(archive_filename_with_ext))
    call(WORKING_DIR, ['zip', '-yr', archive_filename_with_ext, source_directory])
    return archive_filename_with_ext

def archive_framework_to_win64_zip(package_basename):
    """Create build archive to zip on Win64"""

    # Rename our packaging dir to match the release
    shutil.move(PACKAGING_DIR, package_basename)

    (archive_filename_with_ext, archive_path) = create_win64_zip(package_basename)

    # Cleanup
    shutil.move(archive_path, PACKAGING_DIR)
    shutil.rmtree(ARCHIVING_DIR)

    full_out_path = os.path.abspath(archive_filename_with_ext)
    print("Packaged to %s" % full_out_path)
    return(full_out_path)

def create_win64_zip(source_directory):
    """Create a zip for the provided directory on Win64"""

    archive_filename_with_ext = '%s.%s' % (source_directory, 'zip')

    # Create our archive dir, used to create a copy level folder within the archive
    if os.path.exists(ARCHIVING_DIR):
        shutil.rmtree(ARCHIVING_DIR, True)
    os.makedirs(ARCHIVING_DIR)
    archive_path = os.path.join(ARCHIVING_DIR, source_directory)
    shutil.move(source_directory, archive_path)

    # Create archive
    print("Archiving to %s ..." % os.path.abspath(archive_filename_with_ext))
    shutil.make_archive(source_directory, 'zip', ARCHIVING_DIR)

    # Cleanup
    return archive_filename_with_ext, archive_path

def archive_to_timestamped_dir(package_basename):
    """Copy our packaged dir to a timestamped dir"""

    shutil.move(PACKAGING_DIR, package_basename)

    full_out_path = os.path.abspath(package_basename)
    print("Packaged to directory %s" % full_out_path)
    return(full_out_path)

def remove_all_apps_but_specified(single_app_to_include):
    """Remove all internal Naivi apps but the specified one"""
    for app_name in os.listdir(os.path.join(PACKAGING_DIR, APPS_DEST_DIR)):
        if not app_name == single_app_to_include:
            path = os.path.join(PACKAGING_DIR, APPS_DEST_DIR, app_name)
            remove_directory_exit_on_failure(path, 'unwanted internal app')

def archive_source_archive_directory(source_directory):
    if Platform.get() == Platform.Linux:
        package_filename_with_ext = create_linux_tar_bz2(source_directory)
        shutil.rmtree(source_directory)
    elif Platform.get() == Platform.macOS:
        package_filename_with_ext = create_macos_zip(source_directory)
        shutil.rmtree(source_directory)
    elif Platform.get() == Platform.Windows:
        (package_filename_with_ext, _) = create_win64_zip(source_directory)
        shutil.rmtree(ARCHIVING_DIR)
    else:
        print("Error: unsupported platform")
        exit(ERROR_BAD_INPUT)

    print("Source archived to %s" % os.path.abspath(package_filename_with_ext))

def build_package_basename(timestamp, label):
    """Build the name of our package and populate our JSON build info file"""

    # Do the packaging
    if Platform.get() == Platform.Linux:
        platform_name = 'Linux'
    elif Platform.get() == Platform.macOS:
        platform_name = 'macOS'
    elif Platform.get() == Platform.Windows:
        platform_name = 'Win64'
    else:
        print("Error: Unsupported platform")
        sys.exit(ERROR_BAD_INPUT)

    # Fetch version from version.cmake
    (version_unparsed, _) = call(WORKING_DIR, [get_cmake_path(), '-P', 'cmake/version.cmake'], True)
    chunks = version_unparsed.split(':')
    if len(chunks) < 2:
        print("Error passing invalid output from version.cmake: %s" % version_unparsed)
        sys.exit(ERROR_INVALID_VERSION)
    version = chunks[1].strip()

    # Fetch architecture
    arch = get_architecture()

    # Create filename, including timestamp or not as requested
    package_filename = "NAP-%s-%s-%s" % (version, platform_name, arch)
    source_archive_basename = "NAP_SOURCE-%s" % version
    if not timestamp is None:
        package_filename += "-%s" % timestamp
        source_archive_basename += "-%s" % timestamp
    if not label is None:
        package_filename += "-%s" % label
        source_archive_basename += "-%s" % label
    return (package_filename, source_archive_basename)

def get_architecture():
    """Retrieve architecture identifier"""
    if Platform.get() == Platform.Linux:
        v = get_build_arch()
    else:
        v = 'x86_64'
    return v

def verify_clean_git_repo():
    """Verify that the git repository has no local changes"""

    (out, err) = call('.', ['git', 'status', '--porcelain'], True)
    clean = out.strip() == '' and err.strip() == ''
    if not clean:
        print("Error: Git repo needs to be clean to create a source archive")
        sys.exit(ERROR_SOURCE_ARCHIVE_GIT_NOT_CLEAN)

def check_existing_source_archive(source_archive_basename, zip_source_archive, overwrite=False):
    """Verify we're not going to overwrite an existing source archive"""

    # Check staging dir name
    staging_dir = os.path.join(os.path.pardir, source_archive_basename)
    if os.path.exists(staging_dir):
        if overwrite:
            shutil.rmtree(staging_dir)
        else:
            print("Source archive staging dir %s already exists" % staging_dir)
            sys.exit(ERROR_SOURCE_ARCHIVE_EXISTING)

    # Check cwd staging dir name
    if os.path.exists(source_archive_basename):
        if overwrite:
            shutil.rmtree(source_archive_basename)
        else:
            if zip_source_archive:
                print("Source staging dir %s already exists" % source_archive_basename)
            else:
                print("Source archive output dir %s already exists" % source_archive_basename)
            sys.exit(ERROR_SOURCE_ARCHIVE_EXISTING)

    # Check final zipped filename
    output_path = os.path.join(os.path.pardir, source_archive_basename)
    if zip_source_archive:
        if Platform.get() == Platform.Linux:
            output_path += '.tar.bz2'
        else:
            output_path += '.zip'
        if os.path.exists(output_path):
            if overwrite:
                shutil.rmtree(output_path)
            else:
                print("Source archive output %s already exists" % output_path)
                sys.exit(ERROR_SOURCE_ARCHIVE_EXISTING)

def strip_client_apps_for_source_archive(staging_dir):
    # Remove apps
    shutil.rmtree(os.path.join(staging_dir, 'apps'))

    # Remove client apps from CMakeLists.txt
    path = os.path.join(staging_dir, 'CMakeLists.txt')
    output = []
    with open(path, 'r') as f:
        lines = f.readlines()
        in_client_targets = False
        for line in lines:
            if 'START_SOURCE_ARCHIVE_REMOVED_SECTION' in line:
                in_client_targets = True
                output.append("# App targets\n")
                output.append("add_subdirectory(apps/example)\n")
            if not in_client_targets:
                output.append(line)
            if 'END_SOURCE_ARCHIVE_REMOVED_SECTION' in line:
                in_client_targets = False

    with open(path, 'w') as f:
        f.writelines(output)

def create_source_archive(source_archive_basename, zip_source_archive, build_label, timestamp, git_revision, overwrite):
    """Create a source archive of the repository with apps removed"""

    print("Creating source archive...")

    # Determine archive name
    staging_dir = os.path.join(os.path.pardir, source_archive_basename)

    # Remove any existing staging/output directories/files
    if overwrite:
        check_existing_source_archive(source_archive_basename, zip_source_archive, True)

    # Copy the repo
    shutil.copytree('.', staging_dir, ignore=shutil.ignore_patterns('packaging_bin', 'packaging_build', 'packaging_lib', 'msvc64'))

    # # Clean any git ignored content
    call(staging_dir, ['git', 'clean', '-dxf'], True)
    # Hard clean the repo, to be sure
    call(staging_dir, ['git', 'reset', '--hard'], True)

    # Remove .git, handling shutil readonly permissions issues on Win64
    if sys.platform == 'win32':
        def del_rw(action, name, exc):
            os.chmod(name, stat.S_IWRITE)
            os.remove(name)
        shutil.rmtree(os.path.join(staging_dir, '.git'), onerror=del_rw)
    else:
        shutil.rmtree(os.path.join(staging_dir, '.git'))

    # Misc. cleanup
    shutil.rmtree(os.path.join(staging_dir, 'test'))

    # Remove client apps
    os.rename(os.path.join(staging_dir, 'apps', 'example'), os.path.join(staging_dir, 'example-tmp'))
    strip_client_apps_for_source_archive(staging_dir)
    os.rename(os.path.join(staging_dir, 'example-tmp'), os.rename(os.path.join(staging_dir, 'apps', 'example')))

    # Create executable bit retaining script if on Windows
    if sys.platform == 'win32':
        create_source_archive_executable_bit_applicator(staging_dir)

    # Populate source_archive.json
    call(WORKING_DIR, [get_cmake_path(),
                       '-DOUT_PATH=%s' % staging_dir,
                       '-DNAP_ROOT=.',
                       '-DBUILD_TIMESTAMP=%s' % timestamp,
                       '-DBUILD_GIT_REVISION=%s' % git_revision,
                       '-DBUILD_LABEL=%s' % build_label,
                       '-P', 'cmake/source_archive_populate_info.cmake'
                       ])

    # Move alongside release
    local_archive_name = os.path.join('.', source_archive_basename)
    if overwrite and os.path.exists(local_archive_name):
        shutil.rmtree(local_archive_name)
    os.rename(staging_dir, local_archive_name)

    # Optionally: compress to file (and cleanup)
    if zip_source_archive:
        archive_source_archive_directory(source_archive_basename)
    else:
        print("Source archived to %s" % os.path.abspath(source_archive_basename))

def create_source_archive_executable_bit_applicator(staging_dir):
    """Create a batch file for retaining source archive file executable bits on Windows"""

    lines = subprocess.check_output('git ls-files --stage', shell=True)
    if type(lines) is bytes:
        lines = lines.decode('ascii', 'ignore')

    batch_filename = os.path.join(staging_dir, APPLY_PERMISSIONS_BATCHFILE)
    with open(batch_filename, 'w') as writer:
        writer.write('@echo off\n')
        for line in lines.split('\n'):
            chunks = line.split()
            if len(chunks) < 4:
                continue
            exe_bit_filepath = chunks[3]
            file_in_staging = os.path.join(staging_dir, *exe_bit_filepath.split('/'))
            if chunks[0] == '100755' and os.path.exists(file_in_staging):
                writer.write('if exist %s (\n' % exe_bit_filepath)
                writer.write('\tgit update-index --chmod=+x %s\n' % exe_bit_filepath)
                writer.write(')\n')

if __name__ == '__main__':
    parser = argparse.ArgumentParser()

    labelling_group = parser.add_argument_group('Labelling')
    labelling_group.add_argument("-nt", "--no-timestamp", action="store_true",
                        help="Don't include timestamp in the release archive and folder name, for final releases")
    labelling_group.add_argument("-l", "--label", type=str,
                        help="An optional suffix for the package")
    labelling_group.add_argument("-n", "--name", type=str,
                        help="Overrides the package name. NAP timestamp, version and label information is excluded")

    core_group = parser.add_argument_group('Core Behaviour')
    core_group.add_argument("-nz", "--no-zip", action="store_true",
                        help="Don't zip the release, package to a directory")
    core_group.add_argument("-ds", "--include-debug-symbols", action="store_true",
                        help="Include debug symbols")
    core_group.add_argument("-o", "--overwrite", action="store_true",
                        help="Overwrite any existing framework or source package")
    core_group.add_argument("-c", "--clean", action="store_true",
                        help="Clean build")
    core_group.add_argument('-p', '--enable-python', action="store_true",
                       help="Enable python integration using pybind (deprecated)")

    source_archive_group = parser.add_argument_group('Source Archiving')
    source_archive_group.add_argument("-as", "--archive-source", action="store_true",
                        help="Create a source archive")
    source_archive_group.add_argument("-asz", "--source-archive-zipped", action="store_true",
                        help="Zip the created source archive")
    source_archive_group.add_argument("-aso", "--source-archive-only", action="store_true",
                        help="Only create a source archive")

    nap_apps_group = parser.add_argument_group('Applications')
    nap_apps_group.add_argument("-sna", "--include-single-app", type=str,
                        help="Include only a single application with the given name.")
    nap_apps_group.add_argument("-d", "--additional_dirs", nargs='+', type=str, default=[],
                        help="List of additional sub directories to add to the build")

    unsupported_group = parser.add_argument_group('Unsupported')
    unsupported_group.add_argument("--include-docs", action="store_true",
                        help="Include documentation")
    args = parser.parse_args()

    # If we're packaging a single Naivi app make sure it exists
    if args.include_single_app:
        path = os.path.join(get_nap_root(), APPS_SOURCE_DIR, args.include_single_app)
        if not os.path.exists(path):
            print("Error: Can't package single Naivi app '%s' as it doesn't exist" % args.include_single_app)
            sys.exit(ERROR_BAD_INPUT)

    # It doesn't make sense to zip a source archive that we're not creating
    if args.source_archive_zipped and not args.archive_source:
        print("Error: Can't zip a source archive that you're not creating")
        sys.exit(ERROR_BAD_INPUT)

    if args.source_archive_only and (args.no_zip
                                     or args.include_docs
                                     or args.include_apps
                                     or args.include_single_app
                                     or args.clean
                                     or args.include_debug_symbols
                                     or args.additional_dirs
                                     ):
        print("Error: You have specified options that don't make sense if only creating a source archive")
        sys.exit(ERROR_BAD_INPUT)

    # Package our build
    package(not args.no_zip,
            args.include_debug_symbols,
            args.include_docs,
            args.include_single_app,
            args.clean,
            not args.no_timestamp,
            args.label,
            args.name,
            args.archive_source or args.source_archive_only,
            args.source_archive_zipped,
            args.source_archive_only,
            args.overwrite,
            args.additional_dirs,
            args.enable_python)
