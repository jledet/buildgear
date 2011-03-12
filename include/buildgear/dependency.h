#ifndef DEPENDENCY_H
#define DEPENDENCY_H

#include "buildgear/buildfiles.h"
#include "buildgear/dependency.h"

using namespace std;

class CDependency
{
	public:
      void ShowDownloadOrder(void);
      void ShowBuildOrder(void);
      void Resolve(string name,
                   list<CBuildFile*> *buildfiles,
                   list<CBuildFile*> *build_order);
      void ResolveDepths(CBuildFile *buildfile);
      list<CBuildFile*> resolved;
      list<CBuildFile*> unresolved;
      list<CBuildFile*> build_order;
      list<CBuildFile*> download_order;
   private:
      void ResolveDep(CBuildFile *buildfile,
                   list<CBuildFile*> *resolved,
                   list<CBuildFile*> *unresolved);
      void SetDepths(CBuildFile *buildfile, int depth);
};

#endif
