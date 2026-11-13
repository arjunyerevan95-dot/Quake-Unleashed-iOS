"""Check adapter against the actual pinned upstream flat gameplay contract."""
from pathlib import Path
import re,sys
r=Path(__file__).parent;e=Path(sys.argv[1])
s=(r/'ios_bootstrap.m').read_text(encoding="utf-8");args=re.search(r'static char \*args\[\] = \{(.*?)NULL',s,re.S)[1];tokens=re.findall(r'"([^"]*)"',args)
assert len(tokens)==int(re.search(r'\*argc = (\d+)',s)[1])
assert '-flatscreenmode' in tokens
for key,value in [('+vr_enabled','0'),('+vr_flat_holsters','0'),('+vr_flat_autoreload','1')]:assert tokens[tokens.index(key)+1]==value
assert '-flatscreenmode' in (e/'docs/vr-port/FLATPLAY.md').read_text(encoding="utf-8")
s=(e/'Quake/in_sdl.c').read_text(encoding="utf-8");assert 'Key_SetBinding(K_YBUTTON,"impulse 15")' in s
s=(r/'sdl_zink_bridge.c').read_text(encoding="utf-8");gate=s.split('int QVR_IOSCanRender(void){')[1].split('\n}')[0]
assert '!suspended && QVR_IOSApplicationActive()' in gate and 'w>=h' not in gate
print('Flat profile launch count, native mappings and lifecycle gate source contracts passed')

