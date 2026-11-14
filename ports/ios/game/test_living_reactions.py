from pathlib import Path
import sys,subprocess,tempfile
s=(Path(sys.argv[1])/'Quake/vr/vr_main.cpp').read_text();a=s.index('    if(Cvar_VariableValue("vr_ios_reactions_version") == 0.f)');b=s.index('    // One-time phone migration',a);block=s[a:b]
source='''#include <map>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#define MAX_OSPATH 512
#define q_snprintf snprintf
const char*com_gamedir=".";
std::map<std::string,float> vars;
float Cvar_VariableValue(const char*n){return vars[n];}
void Cvar_Set(const char*n,const char*v){vars[n]=strtof(v,nullptr);}
void restore(){'''+block+'''}
int main(){vars["vr_ai_enhanced"]=0;vars["vr_nav"]=0;vars["vr_kill_push"]=2.3f;vars["vr_hit_stagger"]=0.7f;restore();
assert(vars["vr_knockdown"]==1&&vars["vr_kdtrig"]==1&&vars["vr_hit_react"]==1);assert(vars["vr_hit_stagger"]==0.7f&&vars["vr_kill_push"]==2.3f&&vars["vr_ai_enhanced"]==0&&vars["vr_nav"]==0);
FILE*f=fopen("ios-living-reactions-before-0.cfg","rb");assert(f);char data[512]={0};fread(data,1,511,f);fclose(f);assert(std::string(data).find("vr_knockdown \\\"0\\\"")!=std::string::npos);
vars["vr_knockdown"]=0;restore();assert(vars["vr_knockdown"]==0);vars["vr_ios_reactions_version"]=-1;restore();assert(vars["vr_knockdown"]==0);
vars["vr_ios_reactions_version"]=0;com_gamedir="nonexistent-dir";restore();assert(vars["vr_knockdown"]==0&&vars["vr_ios_reactions_version"]==0);
remove("ios-living-reactions-before-0.cfg");}
'''
if len(sys.argv)>2:Path(sys.argv[2]).write_text(source)
else:
 with tempfile.TemporaryDirectory() as d:
  p=Path(d);(p/'t.cpp').write_text(source);subprocess.run(['c++','-std=c++17',str(p/'t.cpp'),'-o',str(p/'test')],check=True);subprocess.run([str(p/'test')],cwd=d,check=True)
print('PASS restore zeros only, preserve custom strengths/AI, backup original, repeat/opt-out, backup failure fail-closed')
