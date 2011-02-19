#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <list>
#include <stdio.h>
#include <errno.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "buildgear/config.h"
#include "buildgear/filesystem.h"
#include "buildgear/debug.h"
#include "buildgear/buildfiles.h"

CBuildFile::CBuildFile(string filename)
{
   CBuildFile::filename = filename;
}

void stripChar(string &str, char c)
{
   unsigned int i;

   for (i=0; i<str.length(); i++)
      if (str[i]==c)
      {
         str.erase(i,1);
         i--;
      }
}

void CBuildFiles::ParseAndVerify(list<CBuildFile*> *buildfiles, bool type)
{   
   list<CBuildFile*>::iterator it;
   
   /* Traverse through all buildfiles */
   for (it=buildfiles->begin();
        it!=buildfiles->end();
        it++)
   {
      FILE *fp;
      char line_buffer[PATH_MAX];
      size_t pos;
      string command =  "bash -c 'source " + 
                        (*it)->filename + 
                        "; echo name=$name \
                         ; echo version=$version \
                         ; echo release=$release \
                         ; echo source=${source[@]} \
                         ; echo depends=${depends[@]}'";
      
      fp = popen(command.c_str(), "r");
      if (fp == NULL)
         throw std::runtime_error(strerror(errno));

      // Assign name based on filename
      if (type == TARGET)
         pos = (*it)->filename.rfind("target/");
      else
         pos = (*it)->filename.rfind("host/");

      if (pos == (*it)->filename.npos)
      {
         cout << "Error: " << (*it)->filename << " is invalid." << endl;
         exit(EXIT_FAILURE);
      }
      (*it)->name = (*it)->filename.substr(pos);
      pos = (*it)->name.rfind("/Buildfile");
      if (pos == (*it)->filename.npos)
      {
         cout << "Error: " << (*it)->filename << " is invalid." << endl;
         exit(EXIT_FAILURE);
      }
      (*it)->name.erase(pos,10);

      while (fgets(line_buffer, PATH_MAX, fp) != NULL)
      {
         // Parse key=value pairs
         string line(line_buffer);
         string key, value;
         size_t pos = line.find_first_of('=');
	
         key=line.substr(0, pos);
         value=line.substr(pos+1);

         stripChar(value, '\n');

         // Required keys (FIXME: add check for "" values)
//         if (key == KEY_NAME)
//            (*it)->name = value;
         if (key == KEY_VERSION)
            (*it)->version = value;
         if (key == KEY_RELEASE)
            (*it)->release = value;
         
         // Optional keys
         if (key == KEY_SOURCE)
            (*it)->source = value;
         if (key == KEY_DEPENDS)
            (*it)->depends = value;
      }
      pclose(fp);
   }
}

void CBuildFiles::ShowMeta(list<CBuildFile*> *buildfiles)
{   
   list<CBuildFile*>::iterator it;

   cout << endl << "Buildfiles:" << endl << endl;
   
   /* Traverse through all buildfiles */
   for (it=buildfiles->begin(); 
        it!=buildfiles->end();
        it++)
   {
      cout << KEY_NAME << ":    " << (*it)->name << endl;
      cout << KEY_VERSION << ": " << (*it)->version << endl;
      cout << KEY_RELEASE << ": " << (*it)->release << endl;
      cout << KEY_SOURCE << ":  " << (*it)->source << endl;
      cout << KEY_DEPENDS ": " << (*it)->depends << endl << endl;
   }
}

void CBuildFiles::LoadDependency(list<CBuildFile*> *buildfiles, bool dep_type)
{
   list<CBuildFile*>::iterator it, itr;
   
   /* Traverse through buildfiles */
   for (it=buildfiles->begin(); 
        it!=buildfiles->end();
        it++)
   {
      int no_match, no_match_exit=false;
      string dep;
      istringstream iss((*it)->depends);
      
      // For each dependency element      
      while ( getline(iss, dep, ' ') )
      {         
         // Reset match state
         no_match = true;

         if (dep.find("target/") == 0)
         {
            // Find matching target buildfile
            for (itr=CBuildFiles::target_buildfiles.begin(); 
               itr!=CBuildFiles::target_buildfiles.end();
               itr++)
            {
               // If match found make target dependency relation
               if (dep == (*itr)->name)
               {
                  (*it)->target_dependency.push_back((*itr));
                  no_match = false;
               }
            }
         } else if (dep.find("host/") == 0)
         {
            // Find matching host buildfile
            for (itr=CBuildFiles::host_buildfiles.begin(); 
               itr!=CBuildFiles::host_buildfiles.end();
               itr++)
            {
               // If match found make host dependency relation
               if (dep == (*itr)->name)
               {
                  (*it)->host_dependency.push_back((*itr));
                  no_match = false;
               }
            }
         } else
         {
            cout << "Error: Invalid dependency element." << endl;
         }
         
         // Warn if missing buildfile(s)
         if (no_match)
         {
            cout << "Error: " << (*it)->name << " " << dep << " not found" << endl;
            no_match_exit = true;
         }
      }
      
      if (no_match_exit)
      {
         cout << "Error: " << (*it)->name << " is missing one or more dependencies" << endl;
         exit(EXIT_FAILURE);
      }
   }
}