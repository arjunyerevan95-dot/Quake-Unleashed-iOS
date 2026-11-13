/* Versioned, standalone touch-layout persistence. No game config is modified. */
#ifndef QVR_TOUCH_LAYOUT_H
#define QVR_TOUCH_LAYOUT_H
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#ifdef _WIN32
#include <windows.h>
#endif
typedef struct { float x,y,scale; int role; } QVR_TouchItem;
/* Parsing is transactional: malformed or newer files leave caller defaults intact. */
static int QVR_LayoutLoad(const char *path,QVR_TouchItem *items,int count){
 FILE*f=fopen(path,"r");if(!f)return errno==ENOENT?0:-1;
 char header[64];int version=0,n=0;
 if(!fgets(header,sizeof header,f)||sscanf(header,"QVR_TOUCH_LAYOUT %d %d",&version,&n)!=2||version!=1||n!=count){fclose(f);return -1;}
 QVR_TouchItem tmp[32];int seen[32]={0};if(count>32){fclose(f);return -1;}memcpy(tmp,items,count*sizeof *items);
 for(int j=0;j<n;j++){
  int role;float x,y,z;
  if(fscanf(f,"%d %f %f %f",&role,&x,&y,&z)!=4||!isfinite(x)||!isfinite(y)||!isfinite(z)||x<0||x>1||y<0||y>1||z<.6f||z>1.8f){fclose(f);return -1;}
  int i;for(i=0;i<count;i++)if(tmp[i].role==role)break;
  if(i==count||seen[i]){fclose(f);return -1;}seen[i]=1;tmp[i].x=x;tmp[i].y=y;tmp[i].scale=z;
 }
 if(ferror(f)){fclose(f);return -1;}fclose(f);memcpy(items,tmp,count*sizeof *items);return 1;
}
/* Keep a recoverable copy of the previous custom layout, then replace atomically
 * on iOS/POSIX. Never delete the live layout to force a failed replacement. */
static int QVR_LayoutSave(const char*path,const QVR_TouchItem*items,int count){
 char temp[1024],backup[1024];if(strlen(path)>900)return -1;
 snprintf(temp,sizeof temp,"%s.tmp",path);FILE*f=fopen(temp,"w");if(!f)return -1;
 int ok=fprintf(f,"QVR_TOUCH_LAYOUT 1 %d\n",count)>0;
 for(int i=0;i<count;i++)if(fprintf(f,"%d %.7g %.7g %.7g\n",items[i].role,items[i].x,items[i].y,items[i].scale)<0)ok=0;
 if(fflush(f)!=0)ok=0;if(fclose(f)!=0)ok=0;if(!ok){remove(temp);return -1;}
 FILE*old=fopen(path,"rb");if(old){
  int found=0;for(unsigned i=0;i<10000;i++){snprintf(backup,sizeof backup,"%s.bak.%u",path,i);FILE*b=fopen(backup,"rb");if(b){fclose(b);continue;}found=1;break;}
  FILE*b=found?fopen(backup,"wb"):NULL;if(!b){fclose(old);remove(temp);return -1;}
  char chunk[4096];size_t n;while((n=fread(chunk,1,sizeof chunk,old)))if(fwrite(chunk,1,n,b)!=n)ok=0;
  if(ferror(old))ok=0;fclose(old);if(fclose(b)!=0)ok=0;
  if(!ok){remove(temp);return -1;}
 }
 #ifdef _WIN32
 if(!MoveFileExA(temp,path,MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH)){remove(temp);return -1;}
 #else
 if(rename(temp,path)!=0){remove(temp);return -1;}
 #endif
 return 0;
}
#endif
