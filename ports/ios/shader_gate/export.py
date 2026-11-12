#!/usr/bin/env python3
"""Export exact upstream shader strings using its C preprocessor, no game assets.
Generated shader text inherits upstream GPL-2.0-or-later licensing.
"""
import pathlib,re,subprocess,tempfile,json,hashlib,sys
root=pathlib.Path(sys.argv[2]).resolve() if len(sys.argv)>2 else pathlib.Path(__file__).resolve().parents[3]
out=pathlib.Path(sys.argv[1]).resolve();out.mkdir(parents=True,exist_ok=True)
header=root/'Quake/gl_shaders.h'
names=re.findall(r'static const char (\w+)\[\]\s*=',header.read_text())
needed=set(re.findall(r'QS_STRINGIFY\s*\((\w+)\)',header.read_text()))-{'output_name','LINEAR_SPACE_OIT','SHOW_ACTIVE_LIGHT_CLUSTERS','SHOW_WORLD_NORMALS','QVR_RETRO_LUT_UNIT_WORLD','QVR_RETRO_LUT_UNIT_SPRITE'}
defs={}
for h in (root/'Quake').glob('*.h'):
 for name,val in re.findall(r'^\s*#\s*define\s+(\w+)\s+([0-9]+)\b',h.read_text(errors='replace'),re.M):
  if name in needed: defs[name]=val
missing=needed-defs.keys()
if missing:raise SystemExit('Unresolved constants: '+str(missing))
source='#include <stdio.h>\n#define QS_STR1(x) #x\n#define QS_STRINGIFY(x) QS_STR1(x)\n'
source+=''.join('#define '+k+' '+v+'\n' for k,v in defs.items())
source+='#include "gl_shaders.h"\nint main(void){ FILE*f;\n'
for name in names:source+=f'f=fopen("{name}.glsl","wb");if(!f)return 1;fputs({name},f);fclose(f);\n'
source+='return 0;}\n'
with tempfile.TemporaryDirectory() as tmp:
 src=pathlib.Path(tmp)/'export.c';src.write_text(source);exe=pathlib.Path(tmp)/'export'
 subprocess.run(['cc','-I'+str(root/'Quake'),str(src),'-o',str(exe)],check=True)
 subprocess.run([str(exe)],cwd=out,check=True)
manifest={'upstream_commit':subprocess.check_output(['git','-C',str(root),'rev-parse','9192b037^{commit}'],text=True).strip(),'license':'GPL-2.0-or-later','constants':defs,'shaders':{n:hashlib.sha256((out/(n+'.glsl')).read_bytes()).hexdigest() for n in names}}
(out/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
print('Exported',len(names),'upstream shader stages')
