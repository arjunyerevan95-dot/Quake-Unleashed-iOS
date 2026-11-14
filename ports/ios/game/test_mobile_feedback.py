"""Compile production default migration and gyro mapping; check frame-owner contract."""
import pathlib,sys,tempfile,subprocess
root=pathlib.Path(__file__).parent
s=(pathlib.Path(sys.argv[1])/'Quake/vr/vr_main.cpp').read_text()
a=s.index('    if(Cvar_VariableValue("vr_ios_flat_ui_version") == 0.f)')
b=s.index('    // One-time mobile behavior preset',a)
block=s[a:b]
patch=(root/'engine.patch').read_text()
assert 'float right=scale*yaw*gyro_yawsensitivity.value;' in patch
assert 'float down=-scale*pitch*gyro_pitchsensitivity.value;' in patch
boot=(root/'ios_bootstrap.m').read_text()
bridge=(root/'sdl_zink_bridge.c').read_text()
assert 'NSRunLoopCommonModes' in boot and '[frameDriver.link invalidate]' in boot
assert 'SDL_iPhoneSetAnimationCallback' not in bridge
assert '!suspended && QVR_IOSApplicationActive()' in bridge
source='''#include <map>
#include <string>
#include <cstdlib>
#include <cassert>
std::map<std::string,float> vars;
float Cvar_VariableValue(const char*n){return vars[n];}
void Cvar_Set(const char*n,const char*v){vars[n]=std::strtof(v,nullptr);}
void migrate(){'''+block+'''}
int main(){
const char *names[]={"vr_flashlight","vr_menu_flat_vr_style","vr_menu_recolor"};
for(auto n:names)vars[n]=1;
migrate();for(auto n:names)assert(vars[n]==0);
vars["vr_flashlight"]=1;migrate();assert(vars["vr_flashlight"]==1);
vars.clear();vars["vr_flashlight"]=2;vars["vr_menu_recolor"]=0.5f;
migrate();assert(vars["vr_flashlight"]==2 && vars["vr_menu_recolor"]==0.5f);
vars.clear();vars["vr_ios_flat_ui_version"]=-1;vars["vr_flashlight"]=1;
migrate();assert(vars["vr_flashlight"]==1);
}'''
if len(sys.argv)>2:pathlib.Path(sys.argv[2]).write_text(source)
else:
 with tempfile.TemporaryDirectory() as d:
  p=pathlib.Path(d);(p/'test.cpp').write_text(source)
  subprocess.run(['c++','-std=c++17',str(p/'test.cpp'),'-o',str(p/'test')],check=True)
  subprocess.run([str(p/'test')],check=True)
print('PASS: actual mobile default migration; yaw-only inversion and stable frame owner contracts (not device tests)')
