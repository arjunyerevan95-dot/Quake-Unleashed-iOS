"""Compile the actual runloop-aware wait body with deterministic semaphore receipts."""
import pathlib,sys,tempfile,subprocess
s=(pathlib.Path(sys.argv[1])/'src/audio/coreaudio/SDL_coreaudio.m').read_text()
a=s.index('        int wait_status;');b=s.index('    } else',a)
body=s[a:b].replace('this->hidden->ready_semaphore','0')
source=r'''#include <cassert>
#include <cstdio>
int pending,pumps,fail;
#define SDL_MUTEX_TIMEDOUT 1
int SDL_SemWaitTimeout(int,int){if(fail)return -1;return pending-- >0 ? 1:0;}
void CFRunLoopRunInMode(int,double,bool){++pumps;}
const int kCFRunLoopDefaultMode=0;
int SDL_SetError(const char*){return -1;}
int waitReady(){'''+body+r'''return 0;}
int main(){pending=0;pumps=0;assert(waitReady()==0&&pumps==0);
pending=4;pumps=0;assert(waitReady()==0&&pumps==4);
fail=1;pumps=0;assert(waitReady()==-1&&pumps==0);}
'''
if len(sys.argv)>2:pathlib.Path(sys.argv[2]).write_text(source)
else:
 with tempfile.TemporaryDirectory() as d:
  p=pathlib.Path(d);(p/'test.cpp').write_text(source)
  subprocess.run(['c++','-std=c++17',str(p/'test.cpp'),'-o',str(p/'test')],check=True)
  subprocess.run([str(p/'test')],check=True)
print('PASS actual wait body: immediate ready, delayed ready/runloop servicing, error propagated; not iOS device proof')
