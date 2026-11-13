import ctypes as C, sys
s=C.CDLL(sys.argv[1])
class D(C.Structure):
 _fields_=[(n,C.c_uint16) for n in ['version','type','naxes','nbuttons','nhats','vendor','product','padding']]+[('buttons',C.c_uint32),('axes',C.c_uint32),('name',C.c_char_p)]+[(n,C.c_void_p) for n in ['userdata','update','player','rumble','triggers','led','effect']]
class G(C.Structure):_fields_=[('data',C.c_uint8*16)]
s.SDL_Init.argtypes=[C.c_uint];assert s.SDL_Init(0x2000)==0
s.SDL_JoystickAttachVirtual.argtypes=[C.c_int,C.c_int,C.c_int,C.c_int]
base=s.SDL_NumJoysticks()
other=s.SDL_JoystickAttachVirtual(0,2,2,0);assert other==base
assert not s.SDL_IsGameController(other)
s.SDL_GameControllerOpen.restype=C.c_void_p;assert not s.SDL_GameControllerOpen(other)
s.SDL_JoystickAttachVirtualEx.argtypes=[C.POINTER(D)]
d=D(version=1,type=1,naxes=6,nbuttons=21,vendor=0x045e,product=0x028e,buttons=(1<<21)-1,axes=63,name=b'UNLEASHED Touch Xbox 360 Controller')
i=s.SDL_JoystickAttachVirtualEx(C.byref(d));assert i==base+1
s.SDL_JoystickGetDeviceGUID.argtypes=[C.c_int];s.SDL_JoystickGetDeviceGUID.restype=G
s.SDL_JoystickGetGUIDString.argtypes=[G,C.c_char_p,C.c_int];buf=C.create_string_buffer(64);s.SDL_JoystickGetGUIDString(s.SDL_JoystickGetDeviceGUID(i),buf,64)
s.SDL_GameControllerAddMapping.argtypes=[C.c_char_p]
m=buf.value+b',UNLEASHED Touch Xbox 360 Controller,a:b0,b:b1,x:b2,y:b3,back:b4,guide:b5,start:b6,leftstick:b7,rightstick:b8,leftshoulder:b9,rightshoulder:b10,dpup:b11,dpdown:b12,dpleft:b13,dpright:b14,leftx:a0,lefty:a1,rightx:a2,righty:a3,lefttrigger:a4,righttrigger:a5,type:Xbox360,'
assert s.SDL_GameControllerAddMapping(m)>=0
s.SDL_GameControllerOpen.restype=C.c_void_p;c=s.SDL_GameControllerOpen(i);assert c
s.SDL_GameControllerGetType.argtypes=[C.c_void_p];assert s.SDL_GameControllerGetType(c)==1
s.SDL_JoystickOpen.restype=C.c_void_p;j=s.SDL_JoystickOpen(i)
s.SDL_JoystickSetVirtualAxis.argtypes=[C.c_void_p,C.c_int,C.c_int16];s.SDL_GameControllerGetAxis.argtypes=[C.c_void_p,C.c_int];s.SDL_GameControllerGetAxis.restype=C.c_int16
for v in (-32767,32767):
 assert s.SDL_JoystickSetVirtualAxis(j,5,v)==0;s.SDL_JoystickUpdate();print('trigger',v,s.SDL_GameControllerGetAxis(c,5));assert s.SDL_GameControllerGetAxis(c,5)==(0 if v<0 else 32767)
s.SDL_JoystickInstanceID.argtypes=[C.c_void_p]
owned=s.SDL_JoystickInstanceID(j)
assert s.SDL_JoystickGetDeviceInstanceID(base+1)==owned
assert s.SDL_JoystickDetachVirtual(other)==0
assert s.SDL_JoystickGetDeviceInstanceID(base)==owned
print('PASS non-controller open failure, Xbox native trigger route, owned instance reindex; base=',base)
s.SDL_Quit()
