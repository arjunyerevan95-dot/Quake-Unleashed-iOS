"""Compile actual mobile behavior migration, preserving custom and opt-out state."""
import pathlib,re,subprocess,sys,tempfile
s=(pathlib.Path(sys.argv[1])/'Quake/vr/vr_main.cpp').read_text()
a=s.index('    if(Cvar_VariableValue("vr_ios_vanilla_version") == 0.f)')
b=s.index('    // One-time phone migration',a)
block=s[a:b]
rows=re.findall(r'\{"(vr_[a-z_]+)",([0-9.]+)f,"([0-9.]+)"\}',block)
assert len(rows)==22 and 'vr_push' not in dict((n,v) for n,o,v in rows)
source="""#include <map>
#include <string>
#include <cstdlib>
#include <cassert>
std::map<std::string,float> vars;
float Cvar_VariableValue(const char*n){return vars[n];}
void Cvar_Set(const char*n,const char*v){vars[n]=std::strtof(v,nullptr);}
void migrate(){"""+block+'}\nint main(){\n'
for n,o,v in rows:source+=f'vars["{n}"]={o}f;\n'
source+='migrate();\n'
for n,o,v in rows:source+=f'assert(vars["{n}"]=={float(v)}f);\n'
source+='vars["vr_kill_push"]=2.0f;migrate();assert(vars["vr_kill_push"]==2.0f);\nvars.clear();vars["vr_kill_push"]=2.0f;migrate();assert(vars["vr_kill_push"]==2.0f);\nvars.clear();vars["vr_ios_vanilla_version"]=-1;vars["vr_ai_enhanced"]=1;migrate();assert(vars["vr_ai_enhanced"]==1);\n}\n'
if len(sys.argv)>2:pathlib.Path(sys.argv[2]).write_text(source)
else:
 with tempfile.TemporaryDirectory() as d:
  p=pathlib.Path(d);(p/'test.cpp').write_text(source)
  subprocess.run(['c++','-std=c++17',str(p/'test.cpp'),'-o',str(p/'test')],check=True)
  subprocess.run([str(p/'test')],check=True)
print('Behavior preset source: 22 defaults, customized values, repeat launch and opt-out covered')
