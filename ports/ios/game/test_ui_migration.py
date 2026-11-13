"""Compile the actual patched migration block with a tiny cvar test adapter."""
import pathlib, subprocess, sys, tempfile
engine=pathlib.Path(sys.argv[1])
s=(engine/'Quake/vr/vr_main.cpp').read_text()
start=s.index('    if(Cvar_VariableValue("scr_ios_ui_version") < 1.f)')
end=s.index('    // On a phone,',start)
block=s[start:end]
source=r'''#include <map>
#include <string>
#include <cstdlib>
#include <cassert>
std::map<std::string,float> vars;
float Cvar_VariableValue(const char* n){return vars[n];}
void Cvar_Set(const char* n,const char* v){vars[n]=std::strtof(v,nullptr);}
void migrate(){'''+block+r'''}
int main(){
vars={{"scr_menuscale",1},{"scr_sbarscale",1},{"scr_conscale",1}};
migrate(); assert(vars["scr_menuscale"]==2&&vars["scr_sbarscale"]==2&&vars["scr_conscale"]==2);
vars["scr_menuscale"]=1; migrate(); assert(vars["scr_menuscale"]==1);
vars={{"scr_menuscale",1.25},{"scr_sbarscale",3},{"scr_conscale",1.5}};
migrate(); assert(vars["scr_menuscale"]==1.25&&vars["scr_sbarscale"]==3&&vars["scr_conscale"]==1.5);
}
'''
with tempfile.TemporaryDirectory() as d:
 p=pathlib.Path(d); (p/'test.cpp').write_text(source)
 subprocess.run(['c++','-std=c++17',str(p/'test.cpp'),'-o',str(p/'test')],check=True)
 subprocess.run([str(p/'test')],check=True)
print('Actual phone UI migration: defaults, custom values and persisted opt-out passed')
