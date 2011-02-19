#include <string>
#include <cstdlib>
#include "buildgear/config.h"
#include "buildgear/configfile.h"
#include "buildgear/debug.h"
#include "buildgear/time.h"
#include "buildgear/options.h"
#include "buildgear/filesystem.h"
#include "buildgear/buildfiles.h"
#include "buildgear/dependency.h"
#include "buildgear/source.h"
#include "buildgear/download.h"

Debug debug(cout);

int main (int argc, char *argv[])
{
   COptions     Options;
   CConfigFile  ConfigFile;
   CTime        Time;
   CFileSystem  FileSystem;
   CBuildFiles  BuildFiles;
   CDependency  Dependency;
   CSource      Source;
   
   /* Disable cursor (terminfo:civis) */
   cout << TERMINFO_CIVIS;
   
   /* Parse command line options */
   Options.Parse(argc, argv);
   
   /* Start counting elapsed time */
   Time.Start();
   
   /* Debug stream option */
   debug.On() = false;

   /* Search for build root directory */
   FileSystem.FindRoot(ROOT_DIR);
   
   /* Parse configuration file(s) */
   ConfigFile.Parse(GLOBAL_CONFIG_FILE);
   ConfigFile.Parse(LOCAL_CONFIG_FILE);

   /* Verify buildgear required tools */
// Tools.Verify();
//   - fakeroot
//   - sed
//   - bin/bash (dash becomes a don't care)
//   - sha256sum
//   - tar
//   - gzip
//   - bzip2
//   - xz
//   - lzma
//   - unzip
//   - cp
//   - mv
//   - cat
//   - sort
//   - diff
//   - find
//   - grep
//   - rm
//   - file

   /* Future optimization
    * 
    * Parse command to find host/ or target/ type
    * if either not found assume target
    * skip to last /name so that it is possible 
    * do 
    *    buildgear build target/audio/audacity
    *    
    *    equivalent to:
    * 
    *    buildgear build target/audacity
    *    buildgear build audacity
    * 
    * flat host and target namespace:
    * 
    * Directories are an illusion - have no meaning, 
    * only 'name' is important!
    * 
    * Track and load dependent buildfiles
    *  -> does not load all files
    *  -> minimizes I/O load
    */

   /* Find host and target build files */
   cout << "Searching for build files...\n";
   FileSystem.FindFiles(BUILD_FILES_HOST_DIR,
                        BUILD_FILE,
                        &BuildFiles.host_buildfiles);
   FileSystem.FindFiles(BUILD_FILES_TARGET_DIR,
                        BUILD_FILE,
                        &BuildFiles.target_buildfiles);

   /* Print number of buildfiles found */
   cout << BuildFiles.host_buildfiles.size() +
           BuildFiles.target_buildfiles.size() 
        << " files found\n\n";

   /* Parse and verify buildfiles */
   cout << "Verifying build files...\n";
   BuildFiles.ParseAndVerify(&BuildFiles.host_buildfiles, HOST);
   BuildFiles.ParseAndVerify(&BuildFiles.target_buildfiles, TARGET);
   cout << "Done\n\n";
   
   /* Show buildfiles meta info (debug only) */
//   BuildFiles.ShowMeta(&BuildFiles.host_buildfiles);
//   BuildFiles.ShowMeta(&BuildFiles.target_buildfiles);
   
   /* Load dependencies */
   cout << "Loading dependencies...\n";
   BuildFiles.LoadDependency(&BuildFiles.target_buildfiles, TARGET);
   BuildFiles.LoadDependency(&BuildFiles.target_buildfiles, HOST);
   BuildFiles.LoadDependency(&BuildFiles.host_buildfiles, HOST);
   cout << "Done\n\n";

   /* Note: 
    * Allowed dependency relations: TARGET -> TARGET
    *                               TARGET -> HOST
    *                                 HOST -> HOST
    */

   /* Resolve dependencies (FIXME: handle also host/..)  */
   Dependency.Resolve(Options.name, &BuildFiles.target_buildfiles);

   /* Print resolved */
//   Dependency.ShowResolved();

   /* Create build directory */
   FileSystem.CreateDirectory(BUILD_DIR);

   /* Download source files */
   cout << "Downloading sources..." << endl;
   Source.Download(&Dependency.download_order);
   cout << "Done\n\n";

   goto time;

   /* Quit if download command */
   if (Options.download)
      exit(EXIT_SUCCESS);

   /* Start building */
   if (Options.build)
      Source.Build(&Dependency);

time:   
   /* Stop counting elapsed time */
   Time.Stop();
   
   /* Show elapsed time */
   Time.ShowElapsedTime();
   
   /* Enable cursor again (terminfo:cnorm) */
   cout << TERMINFO_CNORM;
}